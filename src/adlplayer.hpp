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

#ifndef ADLPLAYER_HPP
#define ADLPLAYER_HPP

#include <vector>

#include "adlib.hpp"

namespace Common {
	class SeekableReadStream;
}

/** A VGM recording player for Coktel Vision's ADL music format. */
class ADLPlayer : public AdLib {
public:
	ADLPlayer();
	~ADLPlayer();

	bool load(Common::SeekableReadStream &adl);
	bool load(const byte *data, uint32 dataSize);
	void unload();

protected:
	// AdLib interface
	uint32 pollMusic(bool first);
	void rewind();

private:
	struct Timbre {
		uint16 startParams[kOperatorsPerVoice * kParamCount];
		uint16 params[kOperatorsPerVoice * kParamCount];
	};

	uint8 _soundMode;

	std::vector<Timbre> _timbres;

	byte  *_songData;
	uint32 _songDataSize;

	const byte *_playPos;

	uint8  _modifyInstrument;
	uint16 _currentInstruments[kMaxVoiceCount];


	void setInstrument(int voice, int instrument);

	bool readHeader  (Common::SeekableReadStream &adl, int &timbreCount);
	bool readTimbres (Common::SeekableReadStream &adl, int  timbreCount);
	bool readSongData(Common::SeekableReadStream &adl);

	uint32 getSampleDelay(uint16 delay) const;
};

#endif // ADLPLAYER_HPP
