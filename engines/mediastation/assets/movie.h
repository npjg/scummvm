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

#include "mediastation/datum.h"
#include "mediastation/assetheader.h"
#include "mediastation/assets/bitmap.h"
#include "mediastation/assets/sound.h"

#ifndef MEDIASTATION_MOVIE_H
#define MEDIASTATION_MOVIE_H

namespace MediaStation {

class MovieFrameHeader : public BitmapHeader {
public:
    MovieFrameHeader(Chunk &chunk);

    uint _index;
    uint _keyframeEndInMilliseconds;
};

class MovieFrameFooter {
public:
    MovieFrameFooter(Chunk &chunk);

    uint _unk1;
    uint _unk2;
    uint _startInMilliseconds;
    uint _endInMilliseconds;
    uint _left;
    uint _top;
    uint _unk3;
    uint _unk4;
    uint _zIndex; // TODO: This is still unconfirmed but seems likely.
    uint _unk6;
    uint _unk7;
    uint _unk8;
    uint _unk9;
    uint _index;
};

class MovieFrame : public Bitmap {
public:
    MovieFrame(Chunk &chunk, MovieFrameHeader *header);
    ~MovieFrame();

    MovieFrameFooter *_footer;
    uint _keyframeEndInMilliseconds;
    bool _showing;
};

class Movie {
private:
    enum class SectionType {
        ROOT = 0x06a8,
        FRAME = 0x06a9,
        FOOTER = 0x06aa
    };

public:
    Movie(AssetHeader *asset);
    ~Movie();

    void readStill(Chunk &chunk);
    void readSubfile(Subfile &subfile, Chunk &chunk);

    Common::Array<MovieFrame *> _frames;
    Common::Array<MovieFrame *> _stills;
private:
    Common::Array<MovieFrameFooter *> _footers;
    Common::Array<Sound *> _sounds;
    AssetHeader::SoundEncoding _soundEncoding;
    AssetHeader *_header;
};

} // End of namespace MediaStation

#endif