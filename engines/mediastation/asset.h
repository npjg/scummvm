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

#ifndef MEDIASTATION_ASSET_H
#define MEDIASTATION_ASSET_H

#include "common/func.h"

#include "mediastation/subfile.h"
#include "mediastation/chunk.h"

namespace MediaStation {

enum class AssetType;
class AssetHeader;

class Asset {
public:
	Asset(AssetHeader *header) : _header(header) {};
    virtual ~Asset();

    virtual void play() = 0;
    virtual void stop() = 0;
    // Called to have the asset do any processing, like drawing new frames,
    // handling time-based event handlers, and such. Some assets don't have any 
    // processing to do.
    virtual void process() { return; };
    virtual bool isPlaying() const { return _isPlaying; }

    //typedef Operand (*Method)(const Common::Array<Operand> &);
    //virtual Common::HashMap<uint32, Method> getMethodMap() const;

    virtual void readChunk(Chunk &chunk);
    virtual void readSubfile(Subfile &subfile, Chunk &chunk);
    AssetType type() const;

protected:
	AssetHeader *_header = nullptr;
    bool _isPlaying = false;
    uint _startTime = 0;
    uint _lastProcessedTime = 0;
    uint _duration = 0;
};

} // End of namespace MediaStation

#endif