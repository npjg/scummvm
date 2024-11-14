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

#include "mediastation/assets/sprite.h"
#include "mediastation/datum.h"

namespace MediaStation {

SpriteFrameHeader::SpriteFrameHeader(Chunk &chunk) : BitmapHeader(chunk) {
    _index = Datum(chunk).u.i;
    debugC(5, kDebugLoading, "SpriteFrameHeader::SpriteFrameHeader(): _index = 0x%x (@0x%lx)", _index, chunk.pos());
    _boundingBox = Datum(chunk, DatumType::POINT_2).u.point;
    debugC(5, kDebugLoading, "SpriteFrameHeader::SpriteFrameHeader(): _boundingBox (@0x%lx)", chunk.pos());
}

SpriteFrameHeader::~SpriteFrameHeader() {
    delete _boundingBox;
}

uint32 SpriteFrame::left() {
    return _bitmapHeader->_boundingBox->x;
}

uint32 SpriteFrame::top() {
    return _bitmapHeader->_boundingBox->y;
}

Common::Point SpriteFrame::topLeft() {
    return Common::Point(left(), top());
}

Common::Rect SpriteFrame::boundingBox() {
    return Common::Rect(topLeft(), width(), height());
}

uint32 SpriteFrame::index() {
    return _bitmapHeader->_index;
}

Sprite::Sprite(AssetHeader *header) : _header(header) {

}

Sprite::~Sprite() {
    // The Sprite doesn't own the header, so it doesn't need to be deleted here.
    // Just set it null.
    _header = nullptr;
    for (SpriteFrame *frame : _frames) {
        delete frame;
    }
}

void Sprite::readFrame(Chunk &chunk) {
    debugC(5, kDebugLoading, "Sprite::readFrame(): Reading sprite frame (@0x%lx)", chunk.pos());
    SpriteFrameHeader *header = new SpriteFrameHeader(chunk);
    SpriteFrame *frame = new SpriteFrame(chunk, header);
    _frames.push_back(frame);
}

} // End of namespace MediaStation