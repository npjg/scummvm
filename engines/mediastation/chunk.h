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

class Chunk : public Common::SeekableReadStream {
private:
    Common::SeekableReadStream *_input;
    uint32 _dataStartOffset;
    uint32 _dataEndOffset;

public:
    uint32 id;
    uint32 length;

    Chunk();
    Chunk(Common::SeekableReadStream *stream);

    uint32 bytesRemaining() { return _dataEndOffset - pos(); }

    // ReadStream implementation
    bool eos() const { return _input->eos(); }
    bool err() const { return _input->err(); }
    void clearErr() { _input->clearErr(); }
    uint32 read(void *dataPtr, uint32 dataSize) {
        if (pos() > _dataEndOffset) {
            error("Attempted to read past end of chunk");
        }
        return _input->read(dataPtr, dataSize);
    }
    int64 pos() const { return _input->pos(); }
    int64 size() const { return _input->size(); }
    bool seek(int64 offset, int whence = SEEK_SET) {
        if (offset < _dataStartOffset) {
            error("Attempted to seek past start of chunk");
        } else if (offset > _dataEndOffset) {
            error("Attempted to seek past end of chunk");
        } else {
            return _input->seek(offset, whence);
        }
    }
};

} // End of namespace MediaStation

#endif