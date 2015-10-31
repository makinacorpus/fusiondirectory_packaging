<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
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
 * \file variables.inc
 * Define common locations and variables
 * Generated by fusiondirectory-setup
 */

require_once('variables_common.inc');

if (!defined("CONFIG_DIR")) {
  define ("CONFIG_DIR", "/etc/fusiondirectory/"); /* FusionDirectory etc path */
}

/*!
 * \brief Allow setting the config file in the apache configuration
 *   e.g.  SetEnv CONFIG_FILE fusiondirectory.conf 1.0
 */
if (!defined("CONFIG_FILE")) {
  define ("CONFIG_FILE", "fusiondirectory.conf"); /* FusionDirectory filename */
}

/*!
 * \brief Path for smarty3 libraries
 */
define("SMARTY", "/usr/share/php/smarty3/Smarty.class.php");

/*!
 * \brief Smarty compile dir
 */
define ("SPOOL_DIR", "/var/spool/fusiondirectory/"); /* FusionDirectory spool directory */

/*!
 * \brief Global cache dir
 */
define ("CACHE_DIR", "/var/cache/fusiondirectory/"); /* FusionDirectory var directory */

/*!
 * \brief Global locale cache dir
 */
define ("LOCALE_DIR", "/var/cache/fusiondirectory/locale/"); /* FusionDirectory locale directory */

/*!
 * \brief Global tmp dir
 */
define ("TEMP_DIR", "/var/cache/fusiondirectory/tmp/"); /* FusionDirectory tmp directory */

/*!
 * \brief Directory containing the configuration template
 */
define ("CONFIG_TEMPLATE_DIR", "/var/cache/fusiondirectory/template/"); /* FusionDirectory template directory */

/*!
 * \brief Directory containing the fai logs
 */
define ("FAI_LOG_DIR", "/var/cache/fusiondirectory/fai/"); /* FusionDirectory fai directory */

/*!
 * \brief Directory containing the vacation files
 */
define ("SUPANN_DIR", "/etc/fusiondirectory/supann/"); /* FusionDirectory supann template directory */

/*!
 * \brief name of the class.cache file
 */
define("CLASS_CACHE", "class.cache"); /* name of the class cache */

?>