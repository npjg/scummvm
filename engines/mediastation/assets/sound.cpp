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

#include "audio/decoders/adpcm.h"

#include "mediastation/debugchannels.h"
#include "mediastation/assets/sound.h"

namespace MediaStation {

Sound::Sound(AssetHeader *header) : _header(header) {
    if (_header != nullptr) {
        _encoding = _header->_soundEncoding;
    }
    _mixer = g_system->getMixer();
}

Sound::Sound(AssetHeader::SoundEncoding encoding) {
    _encoding = encoding;
    _mixer = g_system->getMixer();
}

Sound::~Sound() {
    // TODO: Will this actually destroy the underlying buffers that we copied
    // the data into?
    // The sound doesn't own the header, so we don't need to delete it here.
    _header = nullptr;
    for (Audio::SeekableAudioStream *stream : _streams) {
        delete stream;
    }
}

void Sound::readSubfile(Subfile &subfile, Chunk &chunk, uint totalChunks) {
    if (_streams.size() != 0) {
        warning("Sound::readSubfile(): Some audio has already been read.");
    }
    uint32 expectedChunkId = chunk.id;

    readChunk(chunk);
    for (uint i = 0; i < totalChunks; i++) {
        chunk = subfile.nextChunk();
        if (chunk.id != expectedChunkId) {
            // TODO: Make this show the chunk IDs as strings, not numbers.
            error("Sound::readSubfile(): Expected chunk %s, got %s", tag2str(expectedChunkId), tag2str(chunk.id));
        }
        readChunk(chunk);
    }
}

void Sound::readChunk(Chunk &chunk) {
    // TODO: Can we read the chunk directly into the audio stream?
    debugC(5, kDebugLoading, "Sound::readChunk(): (encoding = 0x%x) Reading audio chunk (@0x%lx)", (uint)_encoding, chunk.pos());
    byte *buffer = (byte *)malloc(chunk.length);
    chunk.read((void *)buffer, chunk.length);

    switch(_encoding) {
        case AssetHeader::SoundEncoding::PCM_S16LE_MONO_22050: {
            Audio::SeekableAudioStream *stream = Audio::makeRawStream(buffer, chunk.length, Sound::RATE, Sound::FLAGS, DisposeAfterUse::NO);
            _streams.push_back(stream);
            break;
        }

        case AssetHeader::SoundEncoding::IMA_ADPCM_S16LE_MONO_22050: {
            // TODO: Support ADPCM decoding.
            Audio::SeekableAudioStream *stream = nullptr; // Audio::makeADPCMStream(buffer, chunk.length, DisposeAfterUse::NO, Audio::ADPCMType::kADPCMMSIma, Sound::RATE, 1, 4);
            _streams.push_back(stream);
            break;
        }

        default: {
            error("Sound::readChunk(): Unknown audio encoding 0x%x", (uint)_encoding);
            break;
        }
    }
    debugC(5, kDebugLoading, "Sound::readChunk(): Finished reading audio chunk (@0x%lx)", chunk.pos());
}

void Sound::play() {
    if (_streams.size() == 0) {
        warning("Sound::play(): No audio streams to play");
    }

    _queue = Audio::makeQueuingAudioStream(Sound::RATE, false);
    for (Audio::SeekableAudioStream *stream : _streams) {
        _queue->queueAudioStream(stream, DisposeAfterUse::NO);
    }

    _mixer->stopHandle(_soundHandle);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, _queue, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, false, false);
}

}