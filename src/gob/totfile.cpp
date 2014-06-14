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
#include "common/stream.hpp"

#include "gob/totfile.hpp"
#include "gob/gamedir.hpp"

namespace Gob {

TOTFile::TOTResourceTable::TOTResourceTable() {
	items = 0;
}

TOTFile::TOTResourceTable::~TOTResourceTable() {
	delete[] items;
}


TOTFile::EXTResourceTable::EXTResourceTable() {
	itemsCount = 0;
	unknown    = 0;
	items      = 0;
}

TOTFile::EXTResourceTable::~EXTResourceTable() {
	delete[] items;
}


TOTFile::TOTFile(GameDir &gameDir, const std::string &name) :
	_totFile(0), _extFile(0), _exFile(0), _imFile(0), _totResourceTable(0), _extResourceTable(0) {

	_name = std::string(name, 0, name.find_last_of('.'));

	load(gameDir);
}

TOTFile::~TOTFile() {
	unload();
}

void TOTFile::load(GameDir &gameDir) {
	_totFile = gameDir.getFile(_name + ".tot");

	loadProperties();
	loadEXTFile(gameDir);

	bool hasTOTRes = loadTOTResourceTable();
	bool hasEXTRes = loadEXTResourceTable();

	if (!hasTOTRes) {
		delete _totResourceTable;
		_totResourceTable = 0;
	}

	if (!hasEXTRes) {
		delete _extResourceTable;
		_extResourceTable = 0;
	}

	if (hasTOTRes)
		loadIMFile(gameDir);

	if (hasEXTRes)
		loadEXFile(gameDir);
}

void TOTFile::loadProperties() {
	byte header[128];

	_totFile->seek(0);
	if (_totFile->read(header, 128) != 128)
		throw Common::kReadError;
	_totFile->seek(0);

	// Offset 39-41: Version in "Major.Minor" string form
	if (header[40] != '.')
		throw Common::Exception("Invalid version");

	_props.versionMajor = header[39] - '0';
	_props.versionMinor = header[41] - '0';

	_props.variablesCount = READ_LE_UINT32(header + 44);

	_props.textsOffset     = READ_LE_UINT32(header + 48);
	_props.resourcesOffset = READ_LE_UINT32(header + 52);

	_props.animDataSize = READ_LE_UINT16(header + 56);

	_props.imFileNumber   = header[59];
	_props.exFileNumber   = header[60];
	_props.communHandling = header[61];

	for (int i = 0; i < 14; i++)
		_props.functions[i] = READ_LE_UINT16(header + 100 + i * 2);

	uint32 fileSize        = _totFile->size();
	uint32 textsOffset     = _props.textsOffset;
	uint32 resourcesOffset = _props.resourcesOffset;

	if (textsOffset == 0xFFFFFFFF)
		textsOffset = 0;
	if (resourcesOffset == 0xFFFFFFFF)
		resourcesOffset = 0;

	_props.scriptEnd = fileSize;
	if (textsOffset > 0)
		_props.scriptEnd = MIN(_props.scriptEnd, textsOffset);
	if (resourcesOffset > 0)
		_props.scriptEnd = MIN(_props.scriptEnd, resourcesOffset);

	// Calculate the sizes of the texts and resources tables for every possible order
	if ((textsOffset > 0) && (resourcesOffset > 0)) {
		// Both exists

		if (_props.textsOffset > resourcesOffset) {
			// First resources, then texts
			_props.textsSize     = fileSize - textsOffset;
			_props.resourcesSize = textsOffset - resourcesOffset;
		} else {
			// First texts, then resources
			_props.textsSize     = resourcesOffset - textsOffset;
			_props.resourcesSize = fileSize - resourcesOffset;
		}
	} else if (textsOffset     > 0) {
		// Only the texts table exists

		_props.textsSize     = fileSize - textsOffset;
		_props.resourcesSize = 0;
	} else if (resourcesOffset > 0) {
		// Only the resources table exists

		_props.textsSize     = 0;
		_props.resourcesSize = fileSize - resourcesOffset;
	} else {
		// Both don't exists

		_props.textsSize     = 0;
		_props.resourcesSize = 0;
	}
}

void TOTFile::unload() {
	delete _totFile;
	delete _extFile;
	delete _exFile;
	delete _imFile;

	delete _totResourceTable;
	delete _extResourceTable;
}

bool TOTFile::loadTOTResourceTable() {
	if ((_props.resourcesOffset == 0xFFFFFFFF) || (_props.resourcesOffset == 0))
		// No resources here
		return false;

	_totResourceTable = new TOTResourceTable;

	_totFile->seek(_props.resourcesOffset);
	_totResourceTable->itemsCount = _totFile->readSint16LE();

	uint32 resSize = _totResourceTable->itemsCount * kTOTResItemSize + kTOTResTableSize;

	_totResourceTable->dataOffset = _props.resourcesOffset + resSize;


	// Would the table actually fit into the TOT?
	if ((_props.resourcesOffset + resSize) > ((uint32) _totFile->size()))
		return false;

	_totResourceTable->unknown = _totFile->readByte();
	_totResourceTable->items = new TOTResourceItem[_totResourceTable->itemsCount];

	for (int i = 0; i < _totResourceTable->itemsCount; ++i) {
		TOTResourceItem &item = _totResourceTable->items[i];

		item.offset = _totFile->readSint32LE();
		item.size   = _totFile->readUint16LE();
		item.width  = _totFile->readSint16LE();
		item.height = _totFile->readSint16LE();

		if (item.offset < 0) {
			item.type = kResourceIM;
			item.index = -item.offset - 1;
		} else
			item.type = kResourceTOT;
	}

	return true;
}

bool TOTFile::loadEXTResourceTable() {
	_extResourceTable = new EXTResourceTable;
	if (!_extFile)
		return false;

	_extFile->seek(0);

	_extResourceTable->itemsCount = _extFile->readSint16LE();
	_extResourceTable->unknown    = _extFile->readByte();

	if (_extResourceTable->itemsCount > 0)
		_extResourceTable->items = new EXTResourceItem[_extResourceTable->itemsCount];

	for (int i = 0; i < _extResourceTable->itemsCount; i++) {
		EXTResourceItem &item = _extResourceTable->items[i];

		item.offset = _extFile->readUint32LE();
		item.size   = _extFile->readUint16LE();
		item.width  = _extFile->readUint16LE();
		item.height = _extFile->readUint16LE();

		if (item.offset < 0) {
			item.type = kResourceEX;
			item.offset = -item.offset - 1;
		} else {
			item.type = kResourceEXT;
			item.offset += kEXTResTableSize +
			               kEXTResItemSize * _extResourceTable->itemsCount;
		}

		item.packed = (item.width & 0x8000) != 0;

		item.width &= 0x7FFF;
	}

	return true;
}

void TOTFile::loadEXTFile(GameDir &gameDir) {
	try {
		_extFile = gameDir.getFile(_name + ".ext");
	} catch (...) {
	}
}

void TOTFile::loadIMFile(GameDir &gameDir) {
	char num = _props.imFileNumber + '0';
	if (num == '0')
		num = '1';

	std::string imFile = std::string("commun.im") + num;

	try {
		_imFile = gameDir.getFile(imFile);
	} catch (...) {
	}
}

void TOTFile::loadEXFile(GameDir &gameDir) {
	std::string exFile = std::string("commun.ex") + (char)(_props.exFileNumber + '0');

	try {
		_exFile = gameDir.getFile(exFile);
	} catch (...) {
	}
}

const std::string &TOTFile::getName() const {
	return _name;
}

uint16 TOTFile::getTOTResourceCount() const {
	if (!_totResourceTable || (_totResourceTable->itemsCount < 0))
		return 0;

	return (uint16)_totResourceTable->itemsCount;
}

uint16 TOTFile::getEXTResourceCount() const {
	if (!_extResourceTable || (_extResourceTable->itemsCount < 0))
		return 0;

	return (uint16)_extResourceTable->itemsCount;
}

Common::SeekableReadStream *TOTFile::getTOTResource(uint16 id) const {
	if (!_totResourceTable || (id >= _totResourceTable->itemsCount))
		throw Common::Exception("Trying to load non-existent TOT resource (%s, %d/%d)",
				_name.c_str(), id, _totResourceTable ? (_totResourceTable->itemsCount - 1) : -1);

	assert(_totResourceTable->items);

	TOTResourceItem &totItem = _totResourceTable->items[id];

	if (totItem.type == kResourceIM)
		return getIMData(totItem);
	if (totItem.type == kResourceTOT)
		return getTOTData(totItem);

	throw Common::Exception("Invalid TOT resource type %d", totItem.type);
}

Common::SeekableReadStream *TOTFile::getEXTResource(uint16 id) const {
	if (!_extResourceTable || (id > _extResourceTable->itemsCount))
		throw Common::Exception("Trying to load non-existent EXT resource (%s, %d/%d)",
				_name.c_str(), id, _extResourceTable ? (_extResourceTable->itemsCount - 1) : -1);

	assert(_extResourceTable->items);

	EXTResourceItem &extItem = _extResourceTable->items[id];

	uint32 size = extItem.size;

	if (extItem.width & 0x4000)
		size += 1 << 16;
	if (extItem.width & 0x2000)
		size += 2 << 16;
	if (extItem.width & 0x1000)
		size += 4 << 16;
	if (extItem.height == 0)
		size += extItem.width << 16;

	Common::SeekableReadStream *data = 0;
	if (extItem.type == kResourceEXT)
		data = getEXTData(extItem, size);
	if (extItem.type == kResourceEX)
		data = getEXData(extItem, size);

	if (!data)
		throw Common::Exception("Invalid EXT resource type %d", extItem.type);

	if (!extItem.packed)
		return data;

	Common::SeekableReadStream *unpackData = GameDir::unpack(*data, 1);

	delete data;

	return unpackData;
}

Common::SeekableReadStream *TOTFile::getTOTData(TOTResourceItem &totItem) const {
	if (!_totFile)
		throw Common::Exception("No TOT file");

	if (totItem.size == 0)
		throw Common::Exception("TOT item has size 0");

	int32 offset = _totResourceTable->dataOffset + totItem.offset;

	if (!_totFile->seek(offset))
		throw Common::kSeekError;

	return _totFile->readStream(totItem.size);
}

Common::SeekableReadStream *TOTFile::getIMData(TOTResourceItem &totItem) const {
	if (!_imFile)
		throw Common::Exception("No IM file");

	if (totItem.size == 0)
		throw Common::Exception("TOT item has size 0");

	int32 indexOffset = totItem.index * 4;
	if (!_imFile->seek(indexOffset))
		throw Common::kSeekError;

	uint32 offset = _imFile->readUint32LE();
	if (!_imFile->seek(offset))
		throw Common::kSeekError;

	return _imFile->readStream(totItem.size);
}

Common::SeekableReadStream *TOTFile::getEXTData(EXTResourceItem &extItem, uint32 &size) const {
	if (!_extFile)
		throw Common::Exception("No EXT file");

	if (!_extFile->seek(extItem.offset))
		throw Common::kSeekError;

	size = MIN<int>(size, _extFile->size() - extItem.offset);

	return _extFile->readStream(extItem.packed ? (size + 2) : size);
}

Common::SeekableReadStream *TOTFile::getEXData(EXTResourceItem &extItem, uint32 &size) const {
	if (!_exFile)
		throw Common::Exception("No EX file");

	if (!_exFile->seek(extItem.offset))
		throw Common::kSeekError;

	size = MIN<int>(size, _exFile->size() - extItem.offset);

	return _exFile->readStream(extItem.packed ? (size + 2) : size);
}

} // End of namespace Gob
