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

#ifndef GOB_GAMEDIR_HPP
#define GOB_GAMEDIR_HPP

#include <string>
#include <list>
#include <map>

#include "common/types.hpp"
#include "common/file.hpp"

namespace Gob {

class GameDir {
public:
	GameDir(const std::string &path);
	~GameDir();

	const std::list<std::string> &getADL() const;
	const std::list<std::string> &getMDY() const;
	const std::list<std::string> &getTOT() const;

private:
	struct Archive;

	struct File {
		std::string name;
		uint32 size;
		uint32 offset;
		uint8  compression;

		Archive *archive;

		File();
		File(const std::string &n, uint32 s, uint32 o, uint8 c, Archive &a);
	};

	typedef std::map<std::string, File> FileMap;

	struct Archive {
		std::string  name;
		Common::File file;

		FileMap files;

		Archive(const std::string &n = "");
	};


	std::string _path;

	std::list<std::string> _files;

	std::list<std::string> _stk;
	std::list<std::string> _adl;
	std::list<std::string> _mdy;
	std::list<std::string> _tot;

	std::list<Archive *> _archives;


	void openDir();

	void openArchives();
	void closeArchives();

	Archive *openArchive(const std::string &name);
};

} // End of namespace Gob

#endif // GOB_GAMEDIR_HPP
