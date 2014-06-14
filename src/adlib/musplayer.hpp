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

#ifndef ADLIB_MUSPLAYER_H
#define ADLIB_MUSPLAYER_H

#include <string>
#include <vector>

#include "adlib/adlib.hpp"

namespace Common {
	class SeekableReadStream;
}

namespace AdLib {

/** A VGM recording player for the AdLib MUS format, with the instrument information in SND files.
 *
 *  In the Gob engine, those files are usually named .MDY and .TBR instead.
 */
class MUSPlayer : public AdLib {
public:
	MUSPlayer(Common::SeekableReadStream &mus, Common::SeekableReadStream &snd);
	~MUSPlayer();

protected:
	// AdLib interface
	uint32 pollMusic(bool first);
	void rewind();

private:
	struct Timbre {
		std::string name;

		uint16 params[kOperatorsPerVoice * kParamCount];
	};

	std::vector<Timbre> _timbres;

	byte  *_songData;
	uint32 _songDataSize;

	const byte *_playPos;

	uint32 _songID;
	std::string _songName;

	uint8 _ticksPerBeat;
	uint8 _beatsPerMeasure;

	uint8 _soundMode;
	uint8 _pitchBendRange;

	uint16 _baseTempo;

	uint16 _tempo;

	byte _lastCommand;


	/** Load the instruments (.SND or .TBR) */
	void loadSND(Common::SeekableReadStream &snd);
	/** Load the melody (.MUS or .MDY) */
	void loadMUS(Common::SeekableReadStream &mus);

	void unload();

	void unloadSND();
	void unloadMUS();

	void readSNDHeader (Common::SeekableReadStream &snd, int &timbreCount, int &timbrePos);
	void readSNDTimbres(Common::SeekableReadStream &snd, int  timbreCount, int  timbrePos);

	void readMUSHeader(Common::SeekableReadStream &mus);
	void readMUSSong  (Common::SeekableReadStream &mus);

	uint32 getSampleDelay(uint16 delay) const;
	void setInstrument(uint8 voice, uint8 instrument);
	void skipToTiming();

	static void readString(Common::SeekableReadStream &stream, std::string &string, byte *buffer, uint size);
};

} // End of namespace AdLib

#endif // ADLIB_MUSPLAYER_H
