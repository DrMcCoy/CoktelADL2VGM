/* CoktelADL2VGM - Tool to convert Coktel Vision's AdLib music to VGM
 *
 * CoktelADL2VGM is the legal property of its developers, whose names
 * can be found in the AUTHORS file distributed with this source
 * distribution.
 *
 * CoktelADL2VGM is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with CoktelADL2VGM. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file common/version.cpp
 *  Basic CoktelADL2VGM version information
 */

#include "common/version.hpp"

#if defined(HAVE_CONFIG_H)
	#include "config.h"
#endif

// Define default values if the real ones can't be determined

#ifndef PACKAGE_NAME
	#define PACKAGE_NAME "CoktelADL2VGM"
#endif

#ifndef PACKAGE_VERSION
	#define PACKAGE_VERSION "0.1.0"
#endif

const char *ADL2VGM_NAME            = PACKAGE_NAME;
const char *ADL2VGM_VERSION         = PACKAGE_VERSION;
const char *ADL2VGM_NAMEVERSION     = PACKAGE_NAME " " PACKAGE_VERSION;
