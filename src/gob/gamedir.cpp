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

static std::string makeLower(const std::string &str) {
	std::string l;

	l.reserve(str.size());
	for (std::string::const_iterator c = str.begin(); c != str.end(); ++c)
		l += tolower(*c);

	return l;
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

		file.name = makeLower(fileName);

		// Geisha use 0ot files, which are compressed TOT files without the packed byte set.
		if (hasExtension(file.name.c_str(), "0ot")) {
			file.name[file.name.size() - 3] = 't';
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

Common::SeekableReadStream *GameDir::getFile(const std::string &name) {
	Common::SeekableReadStream *stream = openDirectFile(name);
	if (stream)
		return stream;

	File *file = findArchiveFile(name);
	if (!file)
		throw Common::kOpenError;

	return openArchiveFile(*file);
}

Common::SeekableReadStream *GameDir::openDirectFile(const std::string &name) {
	for (std::list<std::string>::const_iterator f = _files.begin(); f != _files.end(); ++f) {
		if (adl2vgm_stricmp(f->c_str(), name.c_str()))
			continue;

		Common::File *file = 0;
		try {
			file = new Common::File(_path + "/" + *f);
		} catch (Common::Exception &e) {
			delete file;
			throw;
		}

		return file;
	}

	return 0;
}

GameDir::File *GameDir::findArchiveFile(std::string name) {
	name = makeLower(name);

	for (std::list<Archive *>::iterator a = _archives.begin(); a != _archives.end(); ++a) {
		FileMap::iterator file = (*a)->files.find(name);
		if (file != (*a)->files.end())
			return &file->second;
	}

	return 0;
}

Common::SeekableReadStream *GameDir::openArchiveFile(File &file) {
	if (!file.archive)
		throw Common::Exception("File has no archive");

	if (!file.archive->file.isOpen())
		throw Common::Exception("File's archive is not open");

	if (!file.archive->file.seek(file.offset))
		throw Common::kSeekError;

	Common::SeekableReadStream *rawData = file.archive->file.readStream(file.size);

	if (file.compression == 0)
		return rawData;

	Common::SeekableReadStream *unpackedData = unpack(*rawData, file.compression);

	delete rawData;

	return unpackedData;
}

Common::SeekableReadStream *GameDir::unpack(Common::SeekableReadStream &src, uint8 compression) {
	int32 size;

	byte *data = unpack(src, size, compression);

	return new Common::MemoryReadStream(data, size, true);
}

uint32 GameDir::getSizeChunks(Common::SeekableReadStream &src) {
	uint32 size = 0;

	uint32 chunkSize = 2, realSize;
	while (chunkSize != 0xFFFF) {
		src.skip(chunkSize - 2);

		chunkSize = src.readUint16LE();
		realSize  = src.readUint16LE();

		if (chunkSize < 4)
			throw Common::Exception("Invalid chunk size (%d)", chunkSize);

		size += realSize;
	}

	if (src.eos())
		throw Common::Exception("End of stream while reading chunks size");

	src.seek(0);

	return size;
}

byte *GameDir::unpack(Common::SeekableReadStream &src, int32 &size, uint8 compression) {
	if ((compression != 1) && (compression != 2))
		throw Common::Exception("Invalid compression (%d)", compression);

	if      (compression == 1)
		size = src.readUint32LE();
	else if (compression == 2)
		size = getSizeChunks(src);

	if (size <= 0)
		throw Common::Exception("Invalid data size (%d)", size);

	byte *data = new byte[size];

	if      (compression == 1)
		unpackChunk(src, data, size);
	else if (compression == 2)
		unpackChunks(src, data, size);

	return data;
}

void GameDir::unpackChunks(Common::SeekableReadStream &src, byte *dest, uint32 size) {
	uint32 chunkSize = 0, realSize;
	while (chunkSize != 0xFFFF) {
		uint32 pos = src.pos();

		chunkSize = src.readUint16LE();
		realSize  = src.readUint16LE();

		if ((chunkSize < 4) || (size < realSize))
			throw Common::Exception("Invalid data size (%d, %d, %d)", chunkSize, size, realSize);

		src.skip(2);

		unpackChunk(src, dest, realSize);

		if (chunkSize != 0xFFFF)
			src.seek(pos + chunkSize + 2);

		size -= realSize;
		dest += realSize;
	}
}

void GameDir::unpackChunk(Common::SeekableReadStream &src, byte *dest, uint32 size) {
	byte *tmpBuf = new byte[4114];

	uint32 counter = size;

	for (int i = 0; i < 4078; i++)
		tmpBuf[i] = 0x20;
	uint16 tmpIndex = 4078;

	uint16 cmd = 0;
	while (1) {
		cmd >>= 1;
		if ((cmd & 0x0100) == 0)
			cmd = src.readByte() | 0xFF00;

		if ((cmd & 1) != 0) { /* copy */
			byte tmp = src.readByte();

			*dest++ = tmp;
			tmpBuf[tmpIndex] = tmp;

			tmpIndex++;
			tmpIndex %= 4096;
			counter--;
			if (counter == 0)
				break;
		} else { /* copy string */
			byte tmp1 = src.readByte();
			byte tmp2 = src.readByte();

			int16 off = tmp1 | ((tmp2 & 0xF0) << 4);
			byte  len =         (tmp2 & 0x0F) + 3;

			for (int i = 0; i < len; i++) {
				*dest++ = tmpBuf[(off + i) % 4096];
				counter--;
				if (counter == 0) {
					delete[] tmpBuf;
					return;
				}
				tmpBuf[tmpIndex] = tmpBuf[(off + i) % 4096];
				tmpIndex++;
				tmpIndex %= 4096;
			}

		}
	}

	delete[] tmpBuf;
}

} // End of namespace Gob

