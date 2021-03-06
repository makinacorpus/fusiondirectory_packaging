<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2015  FusionDirectory

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

class pgpServerInfo extends simplePlugin
{
  var $mainTab = TRUE;
  /* This plugin only writes its objectClass */
  var $objectclasses = array('pgpServerInfo');

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    global $config;
    return array(
      // Attributes are grouped by section
      'main' => array(
        'name'  => _('GPG server info'),
        'attrs' => array(
          new HiddenAttribute ('cn', TRUE, 'pgpServerInfo'),
          new StringAttribute (
            _('Software'), _('Which software this GPG key server is running'),
            'pgpSoftware', FALSE,
            'OpenLDAP 2.4.33'
          ),
          new StringAttribute (
            _('GPG version'), _('Which version of GPG this key server is running'),
            'pgpVersion', FALSE,
            'gpg (GnuPG) 1.4.13'
          ),
          new CompositeAttribute (
            _('Branch in which keys are stored'),
            'pgpBaseKeySpaceDN',
            array(
              new StringAttribute(
                _('Keys RDN'), _('Branch under base in which keys are stored'),
                'BaseKeyBranchDN', TRUE,
                'ou=PGP Keys,'
              )
            ),
            '/^(.*)'.$config->current['BASE'].'$/',
            '%s'.$config->current['BASE']
          ),
        )
      ),
    );
  }

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('GPG server info'),
      'plDescription' => _('GPG server info'),
      'plSection'     => 'conf',
      'plPriority'    => 5,
      'plIcon'        => 'plugins/gpg/images/icon.png',
      'plCategory'    => array('configuration'),

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  protected function ldap_save ($cleanup = TRUE)
  {
    parent::ldap_save($cleanup);

    /* Create the new branch if non-existing */
    $ldap = $this->config->get_ldap_link();
    $ldap->cat($this->pgpBaseKeySpaceDN);
    if (!$ldap->count()) {
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->pgpBaseKeySpaceDN, "Creating branch");
      $ldap->cd($this->config->current['BASE']);
      $ldap->create_missing_trees($this->pgpBaseKeySpaceDN);
      if (!$ldap->success()) {
        msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(),
                            $this->pgpBaseKeySpaceDN, LDAP_MOD, get_class()));
      }
    }
    /* Delete the old branch if empty */
    if ($this->attributesAccess['pgpBaseKeySpaceDN']->hasChanged()) {
      $dn = $this->attributesAccess['pgpBaseKeySpaceDN']->getInitialValue();
      $ldap->cat($dn);
      if ($ldap->count()) {
        /* Old branch exists */
        $ldap->ls('(objectClass=*)', $dn);
        if (!$ldap->count()) {
          /* Old branch is empty */
          @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, "Deleting old empty branch");
          $ldap->rmdir($dn);
        } else {
          @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, "Old branch is not empty");
        }
      }
    }
  }
}
?>
