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

#ifndef ADLIB_HPP
#define ADLIB_HPP

#include "common/types.hpp"

/** Base class for a VGM recording player of an AdLib music format. */
class AdLib {
public:
	AdLib();
	virtual ~AdLib();

	void convert();

protected:
	enum kVoice {
		kVoiceMelody0   =  0,
		kVoiceMelody1   =  1,
		kVoiceMelody2   =  2,
		kVoiceMelody3   =  3,
		kVoiceMelody4   =  4,
		kVoiceMelody5   =  5,
		kVoiceMelody6   =  6, // Only available in melody mode.
		kVoiceMelody7   =  7, // Only available in melody mode.
		kVoiceMelody8   =  8, // Only available in melody mode.
		kVoiceBaseDrum  =  6, // Only available in percussion mode.
		kVoiceSnareDrum =  7, // Only available in percussion mode.
		kVoiceTom       =  8, // Only available in percussion mode.
		kVoiceCymbal    =  9, // Only available in percussion mode.
		kVoiceHihat     = 10  // Only available in percussion mode.
	};

	/** Operator parameters. */
	enum kParam {
		kParamKeyScaleLevel =  0,
		kParamFreqMulti     =  1,
		kParamFeedback      =  2,
		kParamAttack        =  3,
		kParamSustain       =  4,
		kParamSustaining    =  5,
		kParamDecay         =  6,
		kParamRelease       =  7,
		kParamLevel         =  8,
		kParamAM            =  9,
		kParamVib           = 10,
		kParamKeyScaleRate  = 11,
		kParamFM            = 12,
		kParamWaveSelect    = 13
	};

	static const int kOperatorCount  = 18; ///< Number of operators.
	static const int kParamCount     = 14; ///< Number of operator parameters.
	static const int kPitchStepCount = 25; ///< Number of pitch bend steps in a half tone.
	static const int kOctaveCount    =  8; ///< Number of octaves we can play.
	static const int kHalfToneCount  = 12; ///< Number of half tones in an octave.

	static const int kOperatorsPerVoice = 2; ///< Number of operators per voice.

	static const int kMelodyVoiceCount     =  9; ///< Number of melody voices.
	static const int kPercussionVoiceCount =  5; ///< Number of percussion voices.
	static const int kMaxVoiceCount        = 11; ///< Max number of voices.

	/** Number of notes we can play. */
	static const int kNoteCount = kHalfToneCount * kOctaveCount;

	static const int kMaxVolume = 0x007F;
	static const int kMaxPitch  = 0x3FFF;
	static const int kMidPitch  = 0x2000;

	static const int kStandardMidC = 60; ///< A mid C in standard MIDI.
	static const int kOPLMidC      = 48; ///< A mid C for the OPL.


	/** Return the number of samples per second. */
	uint32 getSamplesPerSecond() const;

	/** Write a value into an OPL register. */
	void writeOPL(byte reg, byte val);

	/** Signal that the playback ended.
	 *
	 *  @param killRepeat Explicitly request that the song is not to be looped.
	 */
	void end(bool killRepeat = false);

	/** The callback function that's called for polling more AdLib commands.
	 *
	 *  @param  first Is this the first poll since the start of the song?
	 *  @return The number of samples until the next poll.
	 */
	virtual uint32 pollMusic(bool first) = 0;

	/** Rewind the song. */
	virtual void rewind() = 0;

	/** Return whether we're in percussion mode. */
	bool isPercussionMode() const;

	/** Set percussion or melody mode. */
	void setPercussionMode(bool percussion);

	/** Enable/Disable the wave select operator parameters.
	 *
	 *  When disabled, all operators use the sine wave, regardless of the parameter.
	 */
	void enableWaveSelect(bool enable);

	/** Change the pitch bend range.
	 *
	 *  @param range The range in half tones from 1 to 12 inclusive.
	 *         See bendVoicePitch() for how this works in practice.
	 */
	void setPitchRange(uint8 range);

	/** Set the tremolo (amplitude vibrato) depth.
	 *
	 *  @param tremoloDepth false: 1.0dB, true: 4.8dB.
	 */
	void setTremoloDepth(bool tremoloDepth);

	/** Set the frequency vibrato depth.
	 *
	 *  @param vibratoDepth false: 7 cent, true: 14 cent. 1 cent = 1/100 half tone.
	 */
	void setVibratoDepth(bool vibratoDepth);

	/** Set the keyboard split point. */
	void setKeySplit(bool keySplit);

	/** Set the timbre of a voice.
	 *
	 *  Layout of the operator parameters is as follows:
	 *  - First 13 parameter for the first operator
	 *  - First 13 parameter for the second operator
	 *  - 14th parameter (wave select) for the first operator
	 *  - 14th parameter (wave select) for the second operator
	 */
	void setVoiceTimbre(uint8 voice, const uint16 *params);

	/** Set a voice's volume. */
	void setVoiceVolume(uint8 voice, uint8 volume);

	/** Bend a voice's pitch.
	 *
	 *  The pitchBend parameter is a value between 0 (full down) and kMaxPitch (full up).
	 *  The actual frequency depends on the pitch range set previously by setPitchRange(),
	 *  with full down being -range half tones and full up range half tones.
	 */
	void bendVoicePitch(uint8 voice, uint16 pitchBend);

	/** Switch a voice on.
	 *
	 *  Plays one of the kNoteCount notes. However, the valid range of a note is between
	 *  0 and 127, of which only 12 to 107 are audible.
	 */
	void noteOn(uint8 voice, uint8 note);

	/** Switch a voice off. */
	void noteOff(uint8 voice);

private:
	static const uint32 kRate = 44100;

	static const uint8 kOperatorType  [kOperatorCount];
	static const uint8 kOperatorOffset[kOperatorCount];
	static const uint8 kOperatorVoice [kOperatorCount];

	static const uint8 kVoiceMelodyOperator    [kOperatorsPerVoice][kMelodyVoiceCount];
	static const uint8 kVoicePercussionOperator[kOperatorsPerVoice][kPercussionVoiceCount];

	static const byte kPercussionMasks[kPercussionVoiceCount];

	static const uint16 kPianoParams    [kOperatorsPerVoice][kParamCount];
	static const uint16 kBaseDrumParams [kOperatorsPerVoice][kParamCount];

	static const uint16 kSnareDrumParams[kParamCount];
	static const uint16 kTomParams      [kParamCount];
	static const uint16 kCymbalParams   [kParamCount];
	static const uint16 kHihatParams    [kParamCount];


	bool _first;
	bool _ended;

	bool _tremoloDepth;
	bool _vibratoDepth;
	bool _keySplit;

	bool _enableWaveSelect;

	bool _percussionMode;
	byte _percussionBits;

	uint8  _pitchRange;
	uint16 _pitchRangeStep;

	uint8 _voiceNote[kMaxVoiceCount]; // Last note of each voice
	uint8 _voiceOn  [kMaxVoiceCount]; // Whether each voice is currently on

	uint8 _operatorVolume[kOperatorCount]; // Volume of each operator

	byte _operatorParams[kOperatorCount][kParamCount]; // All operator parameters

	uint16  _freqs[kPitchStepCount][kHalfToneCount];
	uint16 *_freqPtr[kMaxVoiceCount];

	int _halfToneOffset[kMaxVoiceCount];


	void initOPL();

	void reset();
	void allOff();

	// Write global parameters into the OPL
	void writeTremoloVibratoDepthPercMode();
	void writeKeySplit();

	// Write operator parameters into the OPL
	void writeWaveSelect(uint8 oper);
	void writeTremoloVibratoSustainingKeyScaleRateFreqMulti(uint8 oper);
	void writeSustainRelease(uint8 oper);
	void writeAttackDecay(uint8 oper);
	void writeFeedbackFM(uint8 oper);
	void writeKeyScaleLevelVolume(uint8 oper);
	void writeAllParams(uint8 oper);

	void initOperatorParams();
	void initOperatorVolumes();
	void setOperatorParams(uint8 oper, const uint16 *params, uint8 wave);

	void voiceOff(uint8 voice);

	void initFreqs();
	void setFreqs(uint16 *freqs, int32 num, int32 denom);
	int32 calcFreq(int32 deltaDemiToneNum, int32 deltaDemiToneDenom);
	void resetFreqs();

	void changePitch(uint8 voice, uint16 pitchBend);

	void setFreq(uint8 voice, uint16 note, bool on);
};

#endif // ADLIB_HPP
