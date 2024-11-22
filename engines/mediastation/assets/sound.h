/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MEDIASTATION_SOUND_H
#define MEDIASTATION_SOUND_H

#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

#include "mediastation/chunk.h"
#include "mediastation/subfile.h"
#include "mediastation/assetheader.h"

namespace MediaStation {

class Sound {
public:
    // For standalone Sound assets.
    Sound(AssetHeader *header);
    // For sounds that are part of a movie.
    Sound(AssetHeader::SoundEncoding encoding);
    ~Sound();

    void readSubfile(Subfile &subFile, Chunk &chunk, uint totalChunks);
    void readChunk(Chunk& chunk);

    AssetHeader *_header;
    AssetHeader::SoundEncoding _encoding;

    void play();

    // All Media Station audio is signed 16-bit little-endian mono at 22050 Hz.
    // Some defaults must be overridden in the flags.
    static const uint RATE = 22050;
    static const byte FLAGS = Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;

private:
    void decodeImaAdpcm();
    Audio::SoundHandle _soundHandle;
    Common::Array<Audio::SeekableAudioStream *> _streams;

    Audio::QueuingAudioStream *_queue;
	Audio::Mixer *_mixer;
};

} // End of namespace MediaStation

#endif