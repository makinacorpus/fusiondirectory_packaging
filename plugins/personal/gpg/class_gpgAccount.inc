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

class gpgAccount extends simplePlugin
{
  var $displayHeader = TRUE;
  var $objectclasses = array('fdGpgAccount');

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('GPG keys'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new PgpKeyAttribute(
            '', _('GPG keys of this user'),
            'fdUserKeyDN', TRUE
          )
        )
      )
    );
  }

  static function plInfo()
  {
    return array(
      'plShortName'     => _('GPG'),
      'plDescription'   => _('Edit user\'s GPG IDs'),
      'plSelfModify'    => TRUE,
      'plObjectType'    => array('user'),
      'plIcon'          => 'plugins/gpg/images/icon.png',
      'plSmallIcon'     => 'plugins/gpg/images/iconMini.png',

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
  }
}
?>
