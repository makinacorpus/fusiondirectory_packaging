<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2011-2015  FusionDirectory

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

/*!
 * \file class_supportDaemon.inc
 * Source code for class supportDaemon
 */

require_once('jsonRPCClient.php');

/*!
 * \brief This class contains all the function needed by the argonaut
 * deployment system
 */
class supportDaemon
{
  private $s_host       = "";
  private $host_error   = "";
  private $http_options = array();
  private $ssl_options  = array();
  private $config;

  private $s_error      = "";
  private $b_error      = FALSE;

  private $logdir       = "";

  /* Duration in seconds while we do not try to contact server if previous task has failed */
  private $stateReloadDelay = 0; // Disabled

  /*!
   * \brief constructor
   */
  public function __construct()
  {
    $this->config = session::global_get('config');
    /* This should only be the case if we call this from setup.
    __autoload()
    */
    if (!is_object($this->config)) {
      return;
    }
    $argonautHost = $this->get_argonaut_host();

    if ($argonautHost != "") {
      // load from config, store statically
      $this->s_host = $argonautHost;
      $this->logdir = $this->config->get_cfg_value("faiLogsDir");
      if ($this->logdir == "") {
        $this->logdir = FAI_LOG_DIR;
      }
    }
  }

  /*!
   * \brief Load argonaut server config from argonaut service in ldap
   */
  public function get_argonaut_host()
  {
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search(
      "(objectClass=argonautServer)",
      array(
        'cn','ipHostNumber','argonautProtocol','argonautPort',
        'argonautTimeout','argonautCertCN',
        'argonautServerToken'
      )
    );
    if ($ldap->count() == 1) {
      $ldap_infos = $ldap->fetch();
      if (isset($ldap_infos['argonautTimeout'][0])) {
        $this->http_options = array(
          'timeout' => $ldap_infos['argonautTimeout'][0]
        );
        if ($ldap_infos['argonautProtocol'][0] == 'https') {
          $this->ssl_options = array(
            'cafile'            => $this->config->get_cfg_value('SslCaCertPath'),
            'peer_name'         => $ldap_infos['argonautCertCN'][0],
            'verify_peer'       => TRUE,
            'verify_peer_name'  => TRUE,
          );
          $sshaMethod = new passwordMethodssha($this->config);
          $this->http_options['header'] = 'Authorization: Basic '.base64_encode('fd:'.$sshaMethod->generate_hash($ldap_infos['argonautServerToken'][0]));
        };
      }
      return $ldap_infos['argonautProtocol'][0]."://".$ldap_infos['ipHostNumber'][0].":".$ldap_infos['argonautPort'][0];
    } elseif ($ldap->count() > 1) {
      $this->host_error = _("Multiple argonaut server found. Only one is supported");
    } else {
      $this->host_error = _("There is no argonaut server configured");
    }
    return "";
  }

  /*!
   * \brief Test if the argonaut server is available
   *
   * \return boolean TRUE if the server pings, FALSE otherwise
   */
  public function is_available()
  {
    if ($this->s_host == "") {
      $this->set_error($this->host_error);
      return FALSE;
    }
    if (session::global_is_set('argonaut_state')) {
      $state = session::global_get('argonaut_state');
      if (time() - $state['date'] < $this->stateReloadDelay) {
        if ($state['error'] != "") {
          $this->set_error($state['error']);
        }
        return $state['value'];
      }
    }
    try {
      $client = new jsonRPCClient($this->s_host, $this->http_options, $this->ssl_options);
      $ok = "OK";
      $test = $client->echo($ok);
      if ($test == $ok) {
        $this->reset_error();
        return $this->new_state(TRUE);
      } else {
        $this->set_error("Received $test, expected $ok");
        return $this->new_state(FALSE);
      }
    } catch (Exception $e) {
      $this->set_error(nl2br($e->getMessage()));
      return $this->new_state(FALSE);
    }
  }

  private function saved_state ()
  {
    if (session::global_is_set('argonaut_state')) {
      $state = session::global_get('argonaut_state');
      if (time() - $state['date'] < $this->stateReloadDelay) {
        if ($state['value'] == FALSE) {
          $this->set_error($state['error']);
        }
        return $state['value'];
      } else {
        return TRUE;
      }
    }
    return TRUE;
  }

  private function new_state($state_value)
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->s_error, date("H.i.s")." Setting state ($state_value) to ");
    session::global_set('argonaut_state', array(
      'date'  => time(),
      'error' => $this->s_error,
      'value' => $state_value
    ));
    return $state_value;
  }

  /*!
   * \brief Allows simply appending a new DaemonEvent
   *
   * \param String $event The event to append.
   *
   * \return TRUE if correctly appended FALSE otherwise
   */
  public function append($event)
  {
    if (!($event instanceof DaemonEvent)) {
      return FALSE;
    }

    /* Add to queue if new
     */
    if ($event->is_new()) {
      $action = $event->get_action();

      /* Get event informations, like targets..
      */
      $targets  = $event->get_targets();
      $data     = $event->save();

      /* Put timestamp back to UTC time
       */
      timezone::get_default_timezone();
      if ($event->get_timestamp(FALSE) != 0) {
        $data_json['timestamp'] = $event->get_timestamp(FALSE);
        if (isset($data['periodic'])) {
          $data_json['periodic'] = $data['periodic'];
        }
      }
      $data_json['args'] = $data;
      unset($data_json['args']['timestamp']);
      unset($data_json['args']['periodic']);

      /* Append an entry
      */
      return $this->append_call($action, $targets, $data_json);
    } else {
      /* Updated edited entry. */
      trigger_error("Event is not new. This should not happen.");
    }

    return FALSE;
  }

  /*!
   * \brief Allows simply append a call to a function
   *
   * \param string $action    The action to call
   *
   * \param Array  $targets   The concerned mac addresses
   *
   * \param Array  $data      The data to send (timestamp, periodic and args, see Argonaut protocol)
   *
   * \return the returned value if correctly executed and FALSE otherwise
   */
  public function append_call($action, $targets, $data)
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $action, date("H.i.s")." Appending call to");
    if ($this->s_host == "") {
      $this->set_error($this->host_error);
      return FALSE;
    }
    if ($this->saved_state() == FALSE) {
      return FALSE;
    }

    /* Append an entry */
    try {
      $client = new jsonRPCClient($this->s_host, $this->http_options, $this->ssl_options);
      $status = $client->action($action, $targets, $data);
      $this->reset_error();
      $this->new_state(TRUE);
      return $status[0];
    } catch (jsonRPCClient_RequestErrorException $e) {
      $this->set_error(nl2br($e->getMessage()));
      $this->new_state(TRUE); // connection is OK
      return FALSE;
    } catch (jsonRPCClient_NetworkErrorException $e) {
      $this->set_error(nl2br($e->getMessage()));
      return $this->new_state(FALSE);
    }
  }

  /*!
   * \brief  Sets an error message, which can be returned with get_error().
   *
   * \param  String $str The Error message,
   */
  private function set_error($str)
  {
    $this->b_error = TRUE;
    $this->s_error = $str;
  }


  /*!
   * \brief  Resets the error message.
   */
  private function reset_error()
  {
    $this->b_error = FALSE;
    $this->s_error = "";
  }


  /*!
   * \brief  Checks if an error occured.
   *
   * \return boolean returns TRUE or FALSE, whether there is an error or not.
   */
  public function is_error()
  {
    return $this->b_error;
  }


  /*!
   * \brief  Returns the last error.
   *
   * \return string Returns the last error.
   */
  public function get_error()
  {
    $str = $this->s_error;
    $ret = "";
    if (is_string($str)) {
      $ret = $str;
    } else {
      foreach ($str as $msg) {
        $ret .= $msg." ";
      }
    }
    $ret = str_replace(" ", "&nbsp;", $ret);
    return $ret;
  }

  /*!
   * \brief format queue entries
   *
   * \param array $entries The entries to format
   *
   * \return array The entries formatted
   */
  private function format_entries($entries)
  {
    timezone::get_default_timezone();
    $entries_r = array();
    foreach ($entries as &$entry) {
      $entry_r['TIMESTAMP'] = DaemonEvent::_timestamp_to_event($entry['data']['timestamp']);
      $entry_r['MACADDRESS'] = $entry['target'];

      // TODO : do an ldap search only if we haven't done it for the same mac before
      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->config->current['BASE']);
      $ldap->search("(macAddress=".$entry_r['MACADDRESS'].")", array('cn'));
      $ldap_infos = $ldap->fetch();
      $ldap_infos['cn'][0];
      $entry_r['PLAINNAME'] = $ldap_infos['cn'][0];

      $entry_r['ERROR']     = $entry['error'];
      $entry_r['PROGRESS']  = $entry['progress'];

      $entry_r['PERIODIC']  = (isset($entry['data']['periodic'])?$entry['data']['periodic']:NULL);
      $entry_r['STATUS']    = $entry['status'];
      $entry_r['SUBSTATUS'] = $entry['substatus'];
      $entry_r['ID']        = $entry['id'];
      $entry_r['HEADERTAG'] = $entry['action'];

      $entries_r[] = $entry_r;
    }
    unset($entries);
    return $entries_r;
  }


  /*!
   * \brief Returns an array containing all queued entries.
   *
   * \return array All queued entries as an array or FALSE if there is an error.
   */
  public function get_queued_entries() //$event_types = array("*"),$from=-1,$to=-1,$sort="timestamp DESC"
  {
    if ($this->s_host == "") {
      $this->set_error($this->host_error);
      return FALSE;
    }
    if ($this->saved_state() == FALSE) {
      return FALSE;
    }
    timezone::get_default_timezone();
    try {
      $client = new jsonRPCClient($this->s_host, $this->http_options, $this->ssl_options);
      $entries = $client->get_entries();
    } catch (Exception $e) {
      $this->set_error(nl2br($e->getMessage()));
      return $this->new_state(FALSE);
    }
    $entries = $this->format_entries($entries);
    $this->reset_error();
    $this->new_state(TRUE);
    return $entries;
  }

  /*!
   * \brief  Returns an entry containing all requested ids.
   *
   * \param array $ids The IDs of the entries we want to return.
   *
   * \return array  Array of the requested entries.
  */
  public function get_entries_by_id($ids)
  {
    return $this->get_entries("get_entries_by_id", $ids);
  }

  /*!
   * \brief  Removes a set of entries from the argonaut queue.
   *
   * \param array $ids The IDs to remove.
   *
   * \return boolean TRUE on success FALSE otherwise.
   */
  public function remove_entries($ids)
  {
    if (!is_array($ids)) {
      trigger_error("Requires an array as parameter.");
      return;
    }
    if ($this->s_host == "") {
      $this->set_error($this->host_error);
      return FALSE;
    }
    if ($this->saved_state() == FALSE) {
      return FALSE;
    }

    try {
      $client = new jsonRPCClient($this->s_host, $this->http_options, $this->ssl_options);
      $client->remove_entries($ids);
      $this->reset_error();
      return $this->new_state(TRUE);
    } catch (jsonRPCClient_RequestErrorException $e) {
      $this->set_error(nl2br($e->getMessage()));
      $this->new_state(TRUE); // connection is OK
      return FALSE;
    } catch (jsonRPCClient_NetworkErrorException $e) {
      $this->set_error(nl2br($e->getMessage()));
      return $this->new_state(FALSE);
    }
  }

  /*!
   * \brief Removes an entry from the argonaut queue.
   *
   * \param integer $id The ID of the entry we want to remove.
   *
   * \return Boolean TRUE on success.
  */
  public function remove_entry($id)
  {
    return $this->remove_entries(array($id));
  }

  /*!
   * \brief Returns an entry from the argonaut queue
   *
   * \param integer $id The ID of the entry we want to return.
   *
   * \return array  array of the requested entry.
  */
  public function get_entry_by_id($id)
  {
    return $this->get_entries_by_id(array($id));
  }

  /*!
   * \brief  Ask argonaut to process a set of entries now
   *
   * \param  array $ids  The ids of the entries to process immediatly
   *
   * \return boolean Returns TRUE on success FALSE otherwise.
  */
  public function process_entries_now($ids)
  {
    if (!is_array($ids)) {
      trigger_error("Requires an array as first parameter.");
      return;
    }
    if ($this->s_host == "") {
      $this->set_error($this->host_error);
      return FALSE;
    }
    if ($this->saved_state() == FALSE) {
      return FALSE;
    }

    try {
      $client = new jsonRPCClient($this->s_host, $this->http_options, $this->ssl_options);
      $client->process_entries_now($ids);
      $this->reset_error();
      return $this->new_state(TRUE);
    } catch (jsonRPCClient_RequestErrorException $e) {
      $this->set_error(nl2br($e->getMessage()));
      $this->new_state(TRUE); // connection is OK
      return FALSE;
    } catch (jsonRPCClient_NetworkErrorException $e) {
      $this->set_error(nl2br($e->getMessage()));
      return $this->new_state(FALSE);
    }
  }

  /*!
   * \brief Check if a workstation is currently installing
   *
   * \param  string $mac The mac address of the workstation
   *
   * \return boolean Returns TRUE if currently installing FALSE otherwise.
   */
  public function is_currently_installing($mac)
  {
    $evts = $this->get_entries_by_mac(array($mac));
    if ($evts) {
      foreach ($evts as $evt) {
        if (($evt['STATUS'] == "processing") &&
            in_array($evt['HEADERTAG'], array("reinstall","update"))) {

          return TRUE;
        }
      }
    }
    return FALSE;
  }

  /*!
   * \brief  Returns an array containing all entries concerning the macs passed.
   *
   * \param array $macs  The mac addresses of the workstations for which we want entries.
   *
   * \return array  The entries for the requested mac addresses.
   */
  public function get_entries_by_mac($macs)
  {
    return $this->get_entries("get_entries_by_mac", $macs);
  }

  /*!
   * \brief  Get entries from argonaut server with given function and format entries before to return them
   *
   * \param function $func  The JSONRPC method to use for retrieving addresses
   *
   * \param array $address  The mac addresses or ids of the entries we want.
   *
   * \return Array   The entries for the requested addresses.
   */
  private function get_entries($func, $address)
  {
    if (!is_array($address)) {
      trigger_error("Requires an array as parameter.");
      return;
    }
    if ($this->s_host == "") {
      $this->set_error($this->host_error);
      return FALSE;
    }
    if ($this->saved_state() == FALSE) {
      return FALSE;
    }

    try {
      $client = new jsonRPCClient($this->s_host, $this->http_options, $this->ssl_options);
      $entries = $client->$func($address);
      $entries = $this->format_entries($entries);
      $this->reset_error();
      $this->new_state(TRUE);
      return $entries;
    } catch (jsonRPCClient_RequestErrorException $e) {
      $this->set_error(nl2br($e->getMessage()));
      $this->new_state(TRUE); // connection is OK
      return FALSE;
    } catch (jsonRPCClient_NetworkErrorException $e) {
      $this->set_error(nl2br($e->getMessage()));
      return $this->new_state(FALSE);
    }
  }

  /*!
   * \brief  Returns log file informations for a given mac address
   *
   * \param  string $mac The mac address to fetch logs for.
   *
   * \return Array A Multidimensional array containing log infos.
   *
   * \code
   *  MAC_00_01_6C_9D_B9_FA['install_20080311_090900']['FILES'][0]=debconf.log
   *  MAC_00_01_6C_9D_B9_FA['install_20080311_090900']['FILES'][1]=syslog.log
   * \endcode
   */
  public function get_log_info_for_mac($mac)
  {
    if (empty($mac)) {
      return FALSE;
    }
    /* read mac directory */
    $logfiledir = $this->logdir.strtolower("/$mac/");
    if (!is_dir($logfiledir)) {
      return FALSE;
    }
    $dates = scandir($logfiledir);
    if (!$dates) {
      /* directory does not exists */
      return FALSE;
    }
    $ret = array();
    foreach ($dates as $date) {
      if ($date == "." || $date == "..") {
          continue;
      }
      $ret[$date]['DATE_STR']  = $date;
      $ret[$date]['REAL_DATE'] = strtotime(preg_replace("/[^0-9]*/", "", $date));
      foreach (scandir("$logfiledir/$date") as $file) {
        if ($file == "." || $file == "..") {
          continue;
        }
        $ret[$date]['FILES'][] = $file;
      }
    }
    return array($mac => $ret);
  }

  /*!
   * \brief  Get a precise log file on a certain date for a mac address
   *
   * \param string $mac The mac address to fetch logs for.
   *
   * \param string $date The date for the log to fetch.
   *
   * \param string $file The log filename.
   *
   */
  public function get_log_file($mac, $date, $file)
  {
    $path = $this->logdir.strtolower("/$mac")."/$date/$file";
    if (is_file($path)) {
      $this->reset_error();
      $lines = file_get_contents($path);
      if ($lines === FALSE) {
        $this->set_error(sprintf(_('Could not read "%s"!'), $path));
      }
      return $lines;
    } else {
      $this->set_error(sprintf(_('"%s" is not a file!'), $path));
      return FALSE;
    }
  }

  /*!
   * \brief  Returns true if an argonaut daemon is running on the pc
   *
   * \param string $mac The mac address to ping
   *
   * \return boolean TRUE if the workstation is running an argonaut daemon
  */
  public function ping($mac)
  {
    if ($this->s_host == "") {
      $this->set_error($this->host_error);
      return FALSE;
    }
    if ($this->saved_state() == FALSE) {
      return FALSE;
    }
    try {
      $client = new jsonRPCClient($this->s_host, $this->http_options, $this->ssl_options);
      $this->reset_error();
      $this->new_state(TRUE);
      return $client->ping($mac);
    } catch (jsonRPCClient_RequestErrorException $e) {
      $this->set_error(nl2br($e->getMessage()));
      $this->new_state(TRUE); // connection is OK
      return FALSE;
    } catch (jsonRPCClient_NetworkErrorException $e) {
      $this->set_error(nl2br($e->getMessage()));
      return $this->new_state(FALSE);
    }
  }

  /*!
   * \brief  Get packages list for the given release
   *
   * \param string  $release The release we want the packages of. If empty all releases are used.
   *
   * \param array   $attrs   The attributes we want for each packages.
   * (all attributes from Packages file can put, plus version, distribution, has_template and templace)
   *
   * \param array   $filters The filters we want to use. An or is performed between array cases.
   *
   * \param integer $from    The indice of the package we want to start from
   *
   * \param integer $to    The indice of the package we want to stop to.
   *
   * \param boolean $package_list    True if filters is an array of package names that needs exact matching, false if it is an array of perl regexp
   *
   * \return Array   List of packages that fit the given parameters.
  */
  public function FAI_get_packages($release, $attrs, $filters, $from = -1, $to = -1, $package_list = FALSE)
  {
    if ($this->s_host == "") {
      $this->set_error($this->host_error);
      return array();
    }
    if ($this->saved_state() == FALSE) {
      return array();
    }
    if ($package_list) { // filters is a package name list
      foreach ($filters as &$filter) {
        $filter = "^".$this->escape_perl_chars($filter)."$";
      }
      unset($filter);
    }
    try {
      $client = new jsonRPCClient($this->s_host, $this->http_options, $this->ssl_options);
      $res = $client->get_packages($release, $attrs, $filters, $from, $to);
      $to_return = array();
      $attrs = array_map('strtoupper', $attrs);
      foreach ($res as $distribution => $packages) {
        $words = preg_split("/\//", $distribution);
        $distribution = $words[0];
        $section = $words[1];

        if (in_array("DISTRIBUTION", $attrs)) {
          foreach ($packages as &$package) {
            $package['DISTRIBUTION'] = $distribution;
          }
          unset($package);
        }
        if (in_array("SECTION", $attrs)) {
          foreach ($packages as &$package) {
            $package['SECTION'] = $section; // FIXME : do we want to do this? It will replace "contrib/misc by contrib, for instance
          }
          unset($package);
        }
        $to_return = array_merge($to_return, $packages);
      }
      $this->reset_error();
      $this->new_state(TRUE);
      return $to_return;
    } catch (jsonRPCClient_RequestErrorException $e) {
      $this->set_error(nl2br($e->getMessage()));
      $this->new_state(TRUE); // connection is OK
      return array();
    } catch (jsonRPCClient_NetworkErrorException $e) {
      $this->set_error(nl2br($e->getMessage()));
      $this->new_state(FALSE);
      return array();
    }
  }

  /*!
   * \brief Escape perl regexp special chars
   *
   * \param string $str The string that needs escaping
   *
   * \return string Escaped string
  */
  public function escape_perl_chars($str)
  {
    $patterns = array("/\+/", "/\*/", "/\./");
    $escaped  = array("\+",   "\*",   "\."  );
    return preg_replace($patterns, $escaped, $str);
  }

  /*!
   * \brief Get all configured repository servers
   *
   * \return array All configured repository servers
  */
  public function FAI_get_server()
  {
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(FAIrepository=*)(objectClass=FAIrepositoryServer))", array("FAIrepository"));

    $repos = array();
    while ($entry = $ldap->fetch()) {
      if (isset($entry['FAIrepository'])) {
        // Add an entry for each Repository configured for server
        unset($entry['FAIrepository']['count']);
        foreach ($entry['FAIrepository'] as $repo) {
          $tmp = explode("|", $repo);
          $infos['URL']           = $tmp[0];
          $infos['PARENT_SERVER'] = $tmp[1];
          $infos['FAI_RELEASE']   = $tmp[2];
          $infos['SECTIONS']      = (empty($tmp[3])?array():explode(",", $tmp[3]));
          $infos['INSTALL']       = $tmp[4];
          $infos['LOCAL']         = $tmp[5];
          $infos['ARCHS']         = (empty($tmp[6])?array():explode(",", $tmp[6]));
          $infos['DIST']          = (isset($tmp[7])?$tmp[7]:'debian');
          $repos[] = $infos;
        }
      }
    }
    return $repos;
  }

  /*!
   * \brief Get all configured customs repository servers for a given release
   *
   * \param string $release The release concerned
   *
   * \return array All configured customs repository servers for the given release
  */
  public function FAI_get_customs ($release)
  {
    $list     = array();
    $servers  = $this->FAI_get_server();

    foreach ($servers as $server) {
      if (strtolower($server['INSTALL']) == strtolower("custom:$release")) {
        $list[$server['FAI_RELEASE']] = $server['FAI_RELEASE'];
      }
    }

    return array_values($list);
  }

  /*! \brief  Get available kernels packages
   *
   * \param string $release The release we want kernels of
   *
   * \return array  All kernel packages for the given release
  */
  public function FAI_get_kernels($release)
  {
    $kernels = $this->FAI_get_packages($release, array("package"), array("linux-image", '^kernel$'));
    foreach ($kernels as &$kernel) {
      $kernel = $kernel['PACKAGE'];
    }
    unset($kernel);
    return $kernels;
  }

  /*!
   * \brief  Get available profiles
   *
   * \param string $release The release we want profiles for
   *
   * \return array All FAI profiles for the given release
  */
  public function FAI_get_profiles($release)
  {
    $ldap = $this->config->get_ldap_link();
    $ldap->cd(get_ou("faiBaseRDN").$this->config->current['BASE']);
    $ldap->search("(&(objectClass=FAIprofile)(objectClass=FAIclass)(ou:dn:=$release))", array('cn'));

    $res = array();
    while ($entry = $ldap->fetch()) {
      $res[] = $entry['cn'][0];
    }

    return $res;
  }

  /*!
   * \brief Clean queue
   *
   * \param array $mac The macs that have to be removed from the queue
   *
   * \return TRUE if success or FALSE otherwise
  */
  public function clean_queue_from_mac($mac)
  {
    if ($this->s_host == "") {
      $this->set_error($this->host_error);
      return FALSE;
    }
    if ($this->saved_state() == FALSE) {
      return FALSE;
    }

    try {
      $client = new jsonRPCClient($this->s_host, $this->http_options, $this->ssl_options);
      $this->reset_error();
      $taskids = $client->get_entries_by_mac(array($mac));
      $client->remove_entries($taskids);
      return $this->new_state(TRUE);
    } catch (jsonRPCClient_RequestErrorException $e) {
      $this->set_error(nl2br($e->getMessage()));
      $this->new_state(TRUE); // connection is OK
      return FALSE;
    } catch (jsonRPCClient_NetworkErrorException $e) {
      $this->set_error(nl2br($e->getMessage()));
      return $this->new_state(FALSE);
    }
  }
};

?>
