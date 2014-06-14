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

#ifndef GOB_TOTFILE_HPP
#define GOB_TOTFILE_HPP

#include <string>

#include "common/types.hpp"

namespace Common {
class SeekableReadStream;
}

namespace Gob {

class GameDir;

class TOTFile {
public:
	TOTFile(GameDir &gameDir, const std::string &name);
	~TOTFile();

	const std::string &getName() const;

	uint16 getTOTResourceCount() const;
	uint16 getEXTResourceCount() const;

	Common::SeekableReadStream *getTOTResource(uint16 id) const;
	Common::SeekableReadStream *getEXTResource(uint16 id) const;

private:
	// Structure sizes in the files
	static const int kTOTResItemSize   = 4 + 2 + 2 + 2;
	static const int kTOTResTableSize  = 2 + 1;
	static const int kEXTResItemSize   = 4 + 2 + 2 + 2;
	static const int kEXTResTableSize  = 2 + 1;


	struct Properties {
		uint8  versionMajor;
		uint8  versionMinor;
		uint32 variablesCount;
		uint32 textsOffset;
		uint32 resourcesOffset;
		uint16 animDataSize;
		uint8  imFileNumber;
		uint8  exFileNumber;
		uint8  communHandling;
		uint16 functions[14];
		uint32 scriptEnd;
		uint32 textsSize;
		uint32 resourcesSize;
	};

	enum ResourceType {
		kResourceTOT = 0,
		kResourceIM,
		kResourceEXT,
		kResourceEX
	};

	struct TOTResourceItem {
		ResourceType type;
		uint16 size;
		int16 width;
		int16 height;
		union {
			int32 offset;
			int32 index;
		};
	};

	struct TOTResourceTable {
		int16 itemsCount;
		byte unknown;
		TOTResourceItem *items;
		uint32 dataOffset;

		TOTResourceTable();
		~TOTResourceTable();
	};

	struct EXTResourceItem {
		ResourceType type;
		int32 offset;
		uint16 size;
		int16 width;
		int16 height;
		bool packed;
	};

	struct EXTResourceTable {
		int16 itemsCount;
		byte unknown;
		EXTResourceItem *items;

		EXTResourceTable();
		~EXTResourceTable();
	};


	std::string _name;

	Properties _props;

	Common::SeekableReadStream *_totFile;
	Common::SeekableReadStream *_extFile;
	Common::SeekableReadStream *_exFile;
	Common::SeekableReadStream *_imFile;

	TOTResourceTable *_totResourceTable;
	EXTResourceTable *_extResourceTable;


	void load(GameDir &gameDir);
	void unload();

	void loadProperties();
	bool loadTOTResourceTable();
	bool loadEXTResourceTable();

	void loadEXTFile(GameDir &gameDir);
	void loadIMFile(GameDir &gameDir);
	void loadEXFile(GameDir &gameDir);

	Common::SeekableReadStream *getTOTData(TOTResourceItem &totItem) const;
	Common::SeekableReadStream *getIMData(TOTResourceItem &totItem) const;
	Common::SeekableReadStream *getEXTData(EXTResourceItem &extItem, uint32 &size) const;
	Common::SeekableReadStream *getEXData(EXTResourceItem &extItem, uint32 &size) const;
};

} // End of namespace Gob

#endif // GOB_TOTFILE_HPP
