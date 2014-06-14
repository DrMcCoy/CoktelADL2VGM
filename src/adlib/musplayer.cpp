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

#include "common/error.hpp"
#include "common/stream.hpp"

#include "adlib/musplayer.hpp"

namespace AdLib {

MUSPlayer::MUSPlayer(Common::SeekableReadStream &mus, Common::SeekableReadStream &snd) :
	_songData(0), _songDataSize(0), _playPos(0), _songID(0) {

	try {
		loadSND(snd);
	} catch (Common::Exception &e) {
		e.add("Failed to load SND");
		throw;
	}

	try {
		loadMUS(mus);
	} catch (Common::Exception &e) {
		e.add("Failed to load MUS");
		throw;
	}
}

MUSPlayer::~MUSPlayer() {
	unload();
}

void MUSPlayer::unload() {
	unloadSND();
	unloadMUS();
}

uint32 MUSPlayer::getSampleDelay(uint16 delay) const {
	if (delay == 0)
		return 0;

	uint32 freq = (_ticksPerBeat * _tempo) / 60;

	return ((uint32)delay * getSamplesPerSecond()) / freq;
}

void MUSPlayer::skipToTiming() {
	while (*_playPos < 0x80)
		_playPos++;

	if (*_playPos != 0xF8)
		_playPos--;
}

uint32 MUSPlayer::pollMusic(bool first) {
	if (_timbres.empty() || !_songData || !_playPos || (_playPos >= (_songData + _songDataSize))) {
		end();
		return 0;
	}

	if (first)
		return getSampleDelay(*_playPos++);

	uint16 delay = 0;
	while (delay == 0) {
		byte cmd = *_playPos;

		// Delay overflow
		if (cmd == 0xF8) {
			_playPos++;
			delay = 0xF8;
			break;
		}

		// Song end marker
		if (cmd == 0xFC) {
			end();
			return 0;
		}

		// Global command
		if (cmd == 0xF0) {
			_playPos++;

			byte type1 = *_playPos++;
			byte type2 = *_playPos++;

			if ((type1 == 0x7F) && (type2 == 0)) {
				// Tempo change, as a fraction of the base tempo

				uint32 num   = *_playPos++;
				uint32 denom = *_playPos++;

				_tempo = _baseTempo * num + ((_baseTempo * denom) >> 7);

				_playPos++;
			} else {

				// Unsupported global command, skip it
				_playPos -= 2;
				while(*_playPos++ != 0xF7)
					;
			}

			delay = *_playPos++;
			break;
		}

		// Voice command

		if (cmd >= 0x80) {
			_playPos++;

			_lastCommand = cmd;
		} else
			cmd = _lastCommand;

		uint8 voice = cmd & 0x0F;
		uint8 note, volume;
		uint16 pitch;

		switch (cmd & 0xF0) {
		case 0x80: // Note off
			_playPos += 2;
			noteOff(voice);
			break;

		case 0x90: // Note on
			note   = *_playPos++;
			volume = *_playPos++;

			if (volume) {
				setVoiceVolume(voice, volume);
				noteOn(voice, note);
			} else
				noteOff(voice);
			break;

		case 0xA0: // Set volume
			setVoiceVolume(voice, *_playPos++);
			break;

		case 0xB0:
			_playPos += 2;
			break;

		case 0xC0: // Set instrument
			setInstrument(voice, *_playPos++);
			break;

		case 0xD0:
			_playPos++;
			break;

		case 0xE0: // Pitch bend
			pitch  = *_playPos++;
			pitch += *_playPos++ << 7;
			bendVoicePitch(voice, pitch);
			break;

		default:
			throw Common::Exception("Unsupported command: 0x%02X", cmd);
		}

		delay = *_playPos++;
	}

	if (delay == 0xF8) {
		delay = 240;

		if (*_playPos != 0xF8)
			delay += *_playPos++;
	}

	return getSampleDelay(delay);
}

void MUSPlayer::rewind() {
	_playPos = _songData;
	_tempo   = _baseTempo;

	_lastCommand = 0;

	setPercussionMode(_soundMode != 0);
	setPitchRange(_pitchBendRange);
}

void MUSPlayer::loadSND(Common::SeekableReadStream &snd) {
	int timbreCount, timbrePos;

	readSNDHeader(snd, timbreCount, timbrePos);
	readSNDTimbres(snd, timbreCount, timbrePos);

	if (snd.err())
		throw Common::kReadError;
}

void MUSPlayer::readString(Common::SeekableReadStream &stream, std::string &string, byte *buffer, uint size) {
	if (stream.read(buffer, size) != size)
		throw Common::kReadError;

	buffer[size] = '\0';

	string = (char *) buffer;
}

void MUSPlayer::readSNDHeader(Common::SeekableReadStream &snd, int &timbreCount, int &timbrePos) {
	// Sanity check
	if (snd.size() <= 6)
		throw Common::Exception("File too small (%d)", snd.size());

	// Version
	const uint8 versionMajor = snd.readByte();
	const uint8 versionMinor = snd.readByte();

	if ((versionMajor != 1) && (versionMinor != 0))
		throw Common::Exception("Unsupported version %d.%d", versionMajor, versionMinor);

	// Number of timbres and where they start
	timbreCount = snd.readUint16LE();
	timbrePos   = snd.readUint16LE();

	const uint16 minTimbrePos = 6 + timbreCount * 9;

	// Sanity check
	if (timbrePos < minTimbrePos)
		throw Common::Exception("Timbre offset too small: %d < %d", timbrePos, minTimbrePos);

	const uint32 timbreParametersSize = snd.size() - timbrePos;
	const uint32 paramSize            = kOperatorsPerVoice * kParamCount * sizeof(uint16);

	// Sanity check
	if (timbreParametersSize != (timbreCount * paramSize))
		throw Common::Exception("Timbre parameters size mismatch: %d != %d",
		                        timbreParametersSize, timbreCount * paramSize);
}

void MUSPlayer::readSNDTimbres(Common::SeekableReadStream &snd, int timbreCount, int timbrePos) {
	_timbres.resize(timbreCount);

	// Read names
	byte nameBuffer[10];
	for (std::vector<Timbre>::iterator t = _timbres.begin(); t != _timbres.end(); ++t)
		readString(snd, t->name, nameBuffer, 9);

	if (!snd.seek(timbrePos))
		throw Common::kSeekError;

	// Read parameters
	for (std::vector<Timbre>::iterator t = _timbres.begin(); t != _timbres.end(); ++t) {
		for (int i = 0; i < (kOperatorsPerVoice * kParamCount); i++)
			t->params[i] = snd.readUint16LE();
	}
}

void MUSPlayer::loadMUS(Common::SeekableReadStream &mus) {
	readMUSHeader(mus);
	readMUSSong(mus);

	if (mus.err())
		throw Common::kReadError;
}

void MUSPlayer::readMUSHeader(Common::SeekableReadStream &mus) {
	// Sanity check
	if (mus.size() <= 6)
		throw Common::Exception("File too small (%d)", mus.size());

	// Version
	const uint8 versionMajor = mus.readByte();
	const uint8 versionMinor = mus.readByte();

	if ((versionMajor != 1) && (versionMinor != 0))
		throw Common::Exception("Unsupported version %d.%d", versionMajor, versionMinor);

	_songID = mus.readUint32LE();

	byte nameBuffer[31];
	readString(mus, _songName, nameBuffer, 30);

	_ticksPerBeat    = mus.readByte();
	_beatsPerMeasure = mus.readByte();

	mus.skip(4); // Length of song in ticks

	_songDataSize = mus.readUint32LE();

	mus.skip(4); // Number of commands
	mus.skip(8); // Unused

	_soundMode      = mus.readByte();
	_pitchBendRange = mus.readByte();
	_baseTempo      = mus.readUint16LE();

	mus.skip(8); // Unused
}

void MUSPlayer::readMUSSong(Common::SeekableReadStream &mus) {
	const uint32 realSongDataSize = mus.size() - mus.pos();

	if (realSongDataSize < _songDataSize)
		throw Common::Exception("File too small for the song data: %d < %d", realSongDataSize, _songDataSize);

	_songData = new byte[_songDataSize];

	if (mus.read(_songData, _songDataSize) != _songDataSize)
		throw Common::kReadError;
}

void MUSPlayer::unloadSND() {
	_timbres.clear();
}

void MUSPlayer::unloadMUS() {
	delete[] _songData;

	_songData     = 0;
	_songDataSize = 0;

	_playPos = 0;
}

void MUSPlayer::setInstrument(uint8 voice, uint8 instrument) {
	if (instrument >= _timbres.size())
		return;

	setVoiceTimbre(voice, _timbres[instrument].params);
}

} // End of namespace AdLib
