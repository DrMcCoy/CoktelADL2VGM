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

#include "common/util.hpp"
#include "common/error.hpp"

#include "adlib/adlplayer.hpp"
#include "adlib/musplayer.hpp"

#include "gob/gamedir.hpp"
#include "gob/totfile.hpp"

#include "convert.hpp"

/** Return the filename from a full path. */
static std::string findFilename(const std::string &path) {
	size_t sep = path.find_last_of("\\/");
	if (sep == std::string::npos)
		return path;

	return std::string(path, sep + 1);
}

static std::string changeExtension(const std::string &file, const std::string &ext) {
	size_t sep = file.find_last_of('.');
	if (sep == std::string::npos)
		return file + "." + ext;

	return std::string(file, 0, sep) + "." + ext;
}

static void convertADL(Gob::GameDir &gameDir, const std::string &adlFile) {
	status("Converting ADL \"%s\" to VGM...", adlFile.c_str());

	Common::SeekableReadStream *adl = 0;
	try {
		adl = gameDir.getFile(adlFile);

		AdLib::ADLPlayer adlPlayer(*adl);

		adlPlayer.convert(adlFile + ".vgm");

	} catch (Common::Exception &e) {
		delete adl;

		throw;
	}
}

static void convertADL(Gob::GameDir &gameDir) {
	const std::list<std::string> &adl = gameDir.getADL();
	for (std::list<std::string>::const_iterator f = adl.begin(); f != adl.end(); ++f) {
		try {
			convertADL(gameDir, *f);
		} catch (Common::Exception &e) {
			Common::printException(e, "WARNING: ");
		}
	}
}

static void convertTOTADL(const Gob::TOTFile &tot) {
	for (uint i = 0; i < tot.getTOTResourceCount(); i++) {
		char name[256];
		snprintf(name, sizeof(name), "%s.tot.%u", tot.getName().c_str(), i);

		status("Trying to convert ADL \"%s\" to VGM...", name);

		Common::SeekableReadStream *adl = 0;
		try {

			adl = tot.getTOTResource(i);

			AdLib::ADLPlayer adlPlayer(*adl);

			adlPlayer.convert(std::string(name) + ".vgm");

		} catch (Common::Exception &e) {
			delete adl;
			adl = 0;

			Common::printException(e, "WARNING: ");
		}

		delete adl;
	}

	for (uint i = 0; i < tot.getEXTResourceCount(); i++) {
		char name[256];
		snprintf(name, sizeof(name), "%s.ext.%u", tot.getName().c_str(), i);

		status("Trying to convert ADL \"%s\" to VGM...", name);

		Common::SeekableReadStream *adl = 0;
		try {

			adl = tot.getEXTResource(i);

			AdLib::ADLPlayer adlPlayer(*adl);

			adlPlayer.convert(std::string(name) + ".vgm");

		} catch (Common::Exception &e) {
			delete adl;
			adl = 0;

			Common::printException(e, "WARNING: ");
		}

		delete adl;
	}
}

static void convertMDY(Gob::GameDir &gameDir, const std::string &mdyFile, const std::string &tbrFile) {
	status("Converting MDY \"%s\" with TBR \"%s\" to VGM...", mdyFile.c_str(), tbrFile.c_str());

	Common::SeekableReadStream *mdy = 0;
	Common::SeekableReadStream *tbr = 0;
	try {
		mdy = gameDir.getFile(mdyFile);
		tbr = gameDir.getFile(tbrFile);

		AdLib::MUSPlayer musPlayer(*mdy, *tbr);

		musPlayer.convert(mdyFile + ".vgm");

	} catch (Common::Exception &e) {
		delete mdy;
		delete tbr;

		throw;
	}
}

static void convertMDY(Gob::GameDir &gameDir) {
	const std::list<std::string> &mdy = gameDir.getMDY();
	for (std::list<std::string>::const_iterator f = mdy.begin(); f != mdy.end(); ++f) {
		std::string tbr = changeExtension(*f, "tbr");

		try {
			convertMDY(gameDir, *f, tbr);
		} catch (Common::Exception &e) {
			Common::printException(e, "WARNING: ");
		}
	}
}


/** Convert an ADL file into VGM. */
void convertADL(const std::string &adlFile) {
	status("Converting ADL \"%s\" to VGM...", adlFile.c_str());

	// Open the input file
	Common::File adl(adlFile);
	AdLib::ADLPlayer adlPlayer(adl);

	adlPlayer.convert(findFilename(adlFile) + ".vgm");
}

/** Convert a MDY+TBR file into VGM. */
void convertMDY(const std::string &mdyFile, const std::string &tbrFile) {
	status("Converting MDY \"%s\" with TBR \"%s\" to VGM...", mdyFile.c_str(), tbrFile.c_str());

	// Open the input files
	Common::File mdy(mdyFile);
	Common::File tbr(tbrFile);
	AdLib::MUSPlayer musPlayer(mdy, tbr);

	musPlayer.convert(findFilename(mdyFile) + ".vgm");
}

void crawlDirectory(const std::string &directory) {
	status("Crawling through game directory \"%s\"", directory.c_str());

	Gob::GameDir gameDir(directory);

	convertADL(gameDir);
	convertMDY(gameDir);

	const std::list<std::string> &tot = gameDir.getTOT();
	for (std::list<std::string>::const_iterator f = tot.begin(); f != tot.end(); ++f) {
		status("Loading TOT \"%s\"", f->c_str());

		try {
			Gob::TOTFile totFile(gameDir, *f);

			convertTOTADL(totFile);

		} catch (Common::Exception &e) {
			Common::printException(e, "WARNING: ");
		}
	}
}
