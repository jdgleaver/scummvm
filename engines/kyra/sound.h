/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * LPGL License
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef KYRA_SOUND_H
#define KYRA_SOUND_H


#include "common/scummsys.h"
#include "common/file.h"
#include "common/mutex.h"

#include "sound/midiparser.h"
#include "sound/mixer.h"
#include "sound/softsynth/ym2612.h"

#include "kyra/kyra.h"

namespace Audio {
class AudioStream;
} // end of namespace Audio

namespace Kyra {

/**
 * Analog audio output device API for Kyrandia games.
 * It countains functionallity to play music tracks,
 * sound effects and voices.
 */
class Sound {
public:
	Sound(KyraEngine *vm, Audio::Mixer *mixer);
	virtual ~Sound();

	enum kType {
		kAdlib,
		kMidiMT32,
		kMidiGM,
		kTowns
	};

	virtual kType getMusicType() const = 0;
	virtual kType getSfxType() const { return getMusicType(); }

	/**
	 * Initializes the output device.
	 *
	 * @return true on success, else false
	 */
	virtual bool init() = 0;

	/**
	 * Updates the device, this is needed for some devices.
	 */
	virtual void process() {}

	/**
	 * Updates internal volume settings according to ConfigManager
	 */
	virtual void updateVolumeSettings() {}

	/**
	 * Sets the soundfiles the output device will use
	 * when playing a track and/or sound effect.
	 *
	 * @param list	soundfile list
	 */
	virtual void setSoundList(const AudioDataStruct *list) { _soundDataList = list; }

	/**
	 * Load a specifc sound file for use of
	 * playing music and sound effects.
	 */
	virtual void loadSoundFile(uint file) = 0;

	/**
	 * Plays the specified track.
	 *
	 * @param track	track number
	 */
	virtual void playTrack(uint8 track) = 0;

	/**
	 * Stop playback of the current track
	 */
	virtual void haltTrack() = 0;

	/**
	 * Plays the specified sound effect
	 *
	 * @param track	sound effect id
	 */
	virtual void playSoundEffect(uint8 track) = 0;

	/**
	 * Starts fading out the volume.
	 *
	 * This keeps fading out the output until
	 * it is silenced, but does not change
	 * the volume set by setVolume! It will
	 * automatically reset the volume when
	 * playing a new track or sound effect.
	 */
	virtual void beginFadeOut() = 0;

	void enableMusic(int enable) { _musicEnabled = enable; }
	int musicEnabled() const { return _musicEnabled; }
	void enableSFX(bool enable) { _sfxEnabled = enable; }
	bool sfxEnabled() const { return _sfxEnabled; }

	virtual bool voiceFileIsPresent(const char *file);

	/**
	 * Plays the specified voice file.
	 *
	 * Also before starting to play the
	 * specified voice file, it stops the
	 * current voice.
	 *
	 * TODO: add support for queueing voice
	 * files
	 *
	 * @param file	file to be played
	 * @param isSfx marks file as sfx instead of voice
	 * @return channel the voice file is played on
	 */
	virtual bool voicePlay(const char *file, bool isSfx = false);

	/**
	 * Checks if a voice is being played.
	 *
	 * @return true when playing, else false
	 */
	bool voiceIsPlaying(const char *file = 0);

	/**
	 * Stops playback of the current voice.
	 */
	void voiceStop(const char *file = 0);
protected:
	const char *fileListEntry(int file) const { return (_soundDataList != 0 && file >= 0 && file < _soundDataList->_fileListLen) ? _soundDataList->_fileList[file] : ""; }
	const void *cdaData() const { return _soundDataList != 0 ? _soundDataList->_cdaTracks : 0; }
	const int cdaTrackNum() const { return _soundDataList != 0 ? _soundDataList->_cdaNumTracks : 0; }

	enum {
		kNumChannelHandles = 4
	};
	
	struct SoundChannel {
		Common::String file;
		Audio::SoundHandle channelHandle;
	};
	SoundChannel _soundChannels[kNumChannelHandles];

	int _musicEnabled;
	bool _sfxEnabled;

	int _currentTheme;

	KyraEngine *_vm;
	Audio::Mixer *_mixer;

private:
	const AudioDataStruct *_soundDataList;

	struct SpeechCodecs {
		const char *fileext;
		Audio::AudioStream *(*streamFunc)(
			Common::SeekableReadStream *stream,
			bool disposeAfterUse,
			uint32 startTime,
			uint32 duration,
			uint numLoops);
	};

	static const SpeechCodecs _supportedCodes[];
};

class AdlibDriver;

/**
 * AdLib implementation of the sound output device.
 *
 * It uses a special sound file format special to
 * Dune II, Kyrandia 1 and 2. While Dune II and
 * Kyrandia 1 are using exact the same format, the
 * one of Kyrandia 2 slightly differs.
 *
 * See AdlibDriver for more information.
 * @see AdlibDriver
 */
class SoundAdlibPC : public Sound {
public:
	SoundAdlibPC(KyraEngine *vm, Audio::Mixer *mixer);
	~SoundAdlibPC();

	kType getMusicType() const { return kAdlib; }

	bool init();
	void process();

	void loadSoundFile(uint file);

	void playTrack(uint8 track);
	void haltTrack();

	void playSoundEffect(uint8 track);

	void beginFadeOut();
private:
	void play(uint8 track);

	void unk1();
	void unk2();

	AdlibDriver *_driver;

	bool _v2;
	uint8 _trackEntries[500];
	uint8 *_soundDataPtr;
	int _sfxPlayingSound;
	uint _soundFileLoaded;

	uint8 _sfxPriority;
	uint8 _sfxFourthByteOfSong;

	int _numSoundTriggers;
	const int *_soundTriggers;

	static const int _kyra1NumSoundTriggers;
	static const int _kyra1SoundTriggers[];
};

/**
 * MIDI output device.
 *
 * This device supports both MT-32 MIDI, as used in
 * Kyrandia 1 and 2, and GM MIDI, as used in Kyrandia 2.
 *
 * Currently it does not initialize the MT-32 output properly,
 * so MT-32 output does sound a bit odd in some cases.
 *
 * TODO: this code needs some serious cleanup and rework
 * to support MT-32 and GM properly.
 */
class SoundMidiPC : public MidiDriver, public Sound {
public:
	SoundMidiPC(KyraEngine *vm, Audio::Mixer *mixer, MidiDriver *driver);
	~SoundMidiPC();

	kType getMusicType() const { return isMT32() ? kMidiMT32 : kMidiGM; }

	bool init() { return true; }

	void updateVolumeSettings() { /*XXX*/ }

	void loadSoundFile(uint file);

	void playTrack(uint8 track);
	void haltTrack();

	void playSoundEffect(uint8 track);

	void beginFadeOut();

	//MidiDriver interface implementation
	int open();
	void close();
	void send(uint32 b);
	void metaEvent(byte type, byte *data, uint16 length);

	void setTimerCallback(void *timerParam, void (*timerProc)(void *)) { }
	uint32 getBaseTempo(void) { return _driver ? _driver->getBaseTempo() : 0; }

	//Channel allocation functions
	MidiChannel *allocateChannel()		{ return 0; }
	MidiChannel *getPercussionChannel()	{ return 0; }

	void setPassThrough(bool b)	{ _passThrough = b; }

	void hasNativeMT32(bool nativeMT32);
	bool isMT32() const { return _nativeMT32; }

private:
	void setVolume(int vol);

	void playMusic(uint8 *data, uint32 size);
	void stopMusic();
	void loadSoundEffectFile(uint file);
	void loadSoundEffectFile(uint8 *data, uint32 size);

	void stopSoundEffect();

	static void onTimer(void *data);

	MidiChannel *_channel[32];
	int _virChannel[16];
	uint8 _channelVolume[16];
	MidiDriver *_driver;
	bool _nativeMT32;
	bool _useC55;
	bool _passThrough;
	uint8 _volume;
	bool _isPlaying;
	bool _sfxIsPlaying;
	uint32 _fadeStartTime;
	bool _fadeMusicOut;
	bool _eventFromMusic;
	MidiParser *_parser;
	byte *_parserSource;
	MidiParser *_soundEffect;
	byte *_soundEffectSource;

	Common::Mutex _mutex;
};

class SoundTowns_EuphonyDriver;
class SoundTowns : public MidiDriver, public Sound {
public:
	SoundTowns(KyraEngine *vm, Audio::Mixer *mixer);
	~SoundTowns();

	kType getMusicType() const { return kTowns; }

	bool init();
	void process();

	void loadSoundFile(uint file);

	void playTrack(uint8 track);
	void haltTrack();

	void playSoundEffect(uint8);

	void beginFadeOut();

	//MidiDriver interface implementation
	int open();
	void close();
	void send(uint32 b);
	void metaEvent(byte type, byte *data, uint16 length) {}

	void setTimerCallback(void *timerParam, void (*timerProc)(void *)) { }
	uint32 getBaseTempo(void);

	//Channel allocation functions
	MidiChannel *allocateChannel()		{ return 0; }
	MidiChannel *getPercussionChannel()	{ return 0; }

	static float semitoneAndSampleRate_to_sampleStep(int8 semiTone, int8 semiToneRootkey,
		uint32 sampleRate, uint32 outputRate, int32 pitchWheel);
private:
	bool loadInstruments();
	void playEuphonyTrack(uint32 offset, int loop);

	static void onTimer(void *data);

	int _lastTrack;
	Audio::AudioStream *_currentSFX;
	Audio::SoundHandle _sfxHandle;

	uint _sfxFileIndex;
	uint8 *_sfxFileData;

	SoundTowns_EuphonyDriver * _driver;
	MidiParser * _parser;

	Common::Mutex _mutex;

	const uint8 *_sfxBTTable;
	const uint8 *_sfxWDTable;
};

//class SoundTowns_v2_TwnDriver;
class SoundTowns_v2 : public Sound {
public:
	SoundTowns_v2(KyraEngine *vm, Audio::Mixer *mixer);
	~SoundTowns_v2();

	kType getMusicType() const { return kTowns; }

	bool init();
	void process();

	void loadSoundFile(uint file) {}

	void playTrack(uint8 track);
	void haltTrack();
	void beginFadeOut();

	bool voicePlay(const char *file, bool isSfx = false);
	void playSoundEffect(uint8) {}

private:
	int _lastTrack;

	Audio::AudioStream *_currentSFX;

	//SoundTowns_v2_TwnDriver *_driver;
	uint8 *_twnTrackData;
};

class MixedSoundDriver : public Sound {
public:
	MixedSoundDriver(KyraEngine *vm, Audio::Mixer *mixer, Sound *music, Sound *sfx) : Sound(vm, mixer), _music(music), _sfx(sfx) {}
	~MixedSoundDriver() { delete _music; delete _sfx; }

	kType getMusicType() const { return _music->getMusicType(); }
	kType getSfxType() const { return _sfx->getSfxType(); }

	bool init() { return (_music->init() && _sfx->init()); }
	void process() { _music->process(); _sfx->process(); }

	void updateVolumeSettings() { _music->updateVolumeSettings(); _sfx->updateVolumeSettings(); }

	void setSoundList(const AudioDataStruct * list) { _music->setSoundList(list); _sfx->setSoundList(list); }
	void loadSoundFile(uint file) { _music->loadSoundFile(file); _sfx->loadSoundFile(file); }

	void playTrack(uint8 track) { _music->playTrack(track); }
	void haltTrack() { _music->haltTrack(); }

	void playSoundEffect(uint8 track) { _sfx->playSoundEffect(track); }

	void beginFadeOut() { _music->beginFadeOut(); }
private:
	Sound *_music, *_sfx;
};

// Digital Audio

#define SOUND_STREAMS 4

class AUDStream;

/**
 * Digital audio output device.
 *
 * This is just used for Kyrandia 3.
 */
class SoundDigital {
public:
	SoundDigital(KyraEngine *vm, Audio::Mixer *mixer);
	~SoundDigital();

	bool init() { return true; }

	/**
	 * Plays a sound.
	 *
	 * @param stream		Data stream used for playback
	 *                      It will be deleted when playback is finished
	 * @param loop			true if the sound should loop (endlessly)
	 * @param fadeIn		true if the sound should be faded in volume wise
	 * @param channel		tell the sound player to use a specific channel for playback
	 *
	 * @return channel playing the sound
	 */
	int playSound(Common::SeekableReadStream *stream, bool loop = false, bool fadeIn = false, int channel = -1);

	/**
	 * Checks if a given channel is playing a sound.
	 *
	 * @param channel	channel number to check
	 * @return true if playing, else false
	 */
	bool isPlaying(int channel);

	/**
	 * Stop the playback of a sound in the given
	 * channel.
	 *
	 * @param channel	channel number
	 */
	void stopSound(int channel);

	/**
	 * Makes the sound in a given channel
	 * fading out.
	 *
	 * @param channel	channel number
	 */
	void beginFadeOut(int channel);
private:
	KyraEngine *_vm;
	Audio::Mixer *_mixer;

	struct Sound {
		Audio::SoundHandle handle;
		AUDStream *stream;
	} _sounds[SOUND_STREAMS];
};

} // end of namespace Kyra

#endif
