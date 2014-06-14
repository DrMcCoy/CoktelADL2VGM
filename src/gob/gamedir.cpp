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

#include <sys/types.h>
#include <dirent.h>

#include <cctype>
#include <cstring>
#include <cerrno>

#include "common/error.hpp"
#include "common/util.hpp"

#include "gob/gamedir.hpp"

namespace Gob {

GameDir::File::File() : size(0), offset(0), compression(0), archive(0) {
}

GameDir::File::File(const std::string &n, uint32 s, uint32 o, uint8 c, Archive &a) :
	name(n), size(s), offset(o), compression(c), archive(&a) {
}


GameDir::Archive::Archive(const std::string &n) : name(n) {
}


GameDir::GameDir(const std::string &path) : _path(path) {
	openDir();
	openArchives();
}

GameDir::~GameDir() {
	closeArchives();
}

static bool hasExtension(const char *name, const char *ext) {
	const char *p = strrchr(name, '.');
	if (!p)
		return false;

	return !adl2vgm_stricmp(p + 1, ext);
}

void GameDir::openDir() {
	DIR *dir = opendir(_path.c_str());
	if (!dir)
		throw Common::Exception("Can't open \"%s\": %s", _path.c_str(), strerror(errno));

	struct dirent *entry = 0;
	while ((entry = readdir(dir))) {
		_files.push_back(entry->d_name);

		if      (hasExtension(entry->d_name, "stk"))
			_stk.push_back(entry->d_name);
		else if (hasExtension(entry->d_name, "itk"))
			_stk.push_back(entry->d_name);
		else if (hasExtension(entry->d_name, "adl"))
			_adl.push_back(entry->d_name);
		else if (hasExtension(entry->d_name, "mid"))
			_adl.push_back(entry->d_name);
		else if (hasExtension(entry->d_name, "mdy"))
			_mdy.push_back(entry->d_name);
		else if (hasExtension(entry->d_name, "mus"))
			_mdy.push_back(entry->d_name);
		else if (hasExtension(entry->d_name, "tot"))
			_tot.push_back(entry->d_name);
	}

	closedir(dir);
}

void GameDir::openArchives() {
	for (std::list<std::string>::const_iterator s = _stk.begin(); s != _stk.end(); ++s) {
		status("Opening archive \"%s\"", s->c_str());

		try {
			Archive *archive = openArchive(_path + "/" + *s);
			_archives.push_back(archive);
		} catch (Common::Exception &e) {
			Common::printException(e, "WARNING: ");
		}

	}
}

void GameDir::closeArchives() {
	for (std::list<Archive *>::iterator a = _archives.begin(); a != _archives.end(); ++a)
		delete *a;

	_archives.clear();
}

GameDir::Archive *GameDir::openArchive(const std::string &name) {
	Archive *archive = new Archive(name);
	if (!archive->file.open(archive->name))
		throw Common::kOpenError;

	uint16 fileCount = archive->file.readUint16LE();
	for (uint16 i = 0; i < fileCount; i++) {
		File file;

		char fileName[14];

		archive->file.read(fileName, 13);
		fileName[13] = '\0';

		file.size        = archive->file.readUint32LE();
		file.offset      = archive->file.readUint32LE();
		file.compression = archive->file.readByte() != 0;

		file.name = fileName;

		// Geisha use 0ot files, which are compressed TOT files without the packed byte set.
		if (hasExtension(file.name.c_str(), "0OT")) {
			file.name[file.name.size() - 3] = 'T';
			file.compression = 2;
		}

		file.archive = archive;
		archive->files[file.name] = file;

		if      (hasExtension(file.name.c_str(), "adl"))
			_adl.push_back(file.name);
		else if (hasExtension(file.name.c_str(), "mid"))
			_adl.push_back(file.name);
		else if (hasExtension(file.name.c_str(), "mdy"))
			_mdy.push_back(file.name);
		else if (hasExtension(file.name.c_str(), "mus"))
			_mdy.push_back(file.name);
		else if (hasExtension(file.name.c_str(), "tot"))
			_tot.push_back(file.name);
	}

	return archive;
}

const std::list<std::string> &GameDir::getADL() const {
	return _adl;
}

const std::list<std::string> &GameDir::getMDY() const {
	return _mdy;
}

const std::list<std::string> &GameDir::getTOT() const {
	return _tot;
}

} // End of namespace Gob

