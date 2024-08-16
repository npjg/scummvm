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

#include "common/file.h"

#ifndef MEDIASTATION_CHUNK_H
#define MEDIASTATION_CHUNK_H

namespace MediaStation {

class Chunk : public Common::ReadStream {
private:
    Common::SeekableReadStream *_input;
	uint32 _bytesRead;

public:
    uint32 id;
    uint32 size;

    Chunk();
    Chunk(Common::SeekableReadStream *stream);

    bool hasReadAll() const {
        return (size - _bytesRead) == 0;
    }

    void incBytesRead(uint32 inc) {
        _bytesRead += inc;
        if (_bytesRead > size) {
            error("Chunk overread");
        }
    }

    // ReadStream implementation
    bool eos() const { return _input->eos(); }
    bool err() const { return _input->err(); }
    void clearErr() { _input->clearErr(); }
    uint32 read(void *dataPtr, uint32 dataSize) {
        incBytesRead(dataSize);
        return _input->read(dataPtr, dataSize);
    }

};

} // End of namespace MediaStation

#endif