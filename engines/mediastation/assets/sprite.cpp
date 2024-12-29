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
#include "mediastation/assets/sprite.h"
#include "mediastation/debugchannels.h"
#include "mediastation/mediastation.h"

namespace MediaStation {

SpriteFrameHeader::SpriteFrameHeader(Chunk &chunk) : BitmapHeader(chunk) {
    _index = Datum(chunk).u.i;
    debugC(5, kDebugLoading, "SpriteFrameHeader::SpriteFrameHeader(): _index = 0x%x (@0x%lx)", _index, chunk.pos());
    _boundingBox = Datum(chunk, DatumType::POINT_2).u.point;
    debugC(5, kDebugLoading, "SpriteFrameHeader::SpriteFrameHeader(): _boundingBox (@0x%lx)", chunk.pos());
}

SpriteFrameHeader::~SpriteFrameHeader() {
    delete _boundingBox;
    _boundingBox = nullptr;
}

SpriteFrame::SpriteFrame(Chunk &chunk, SpriteFrameHeader *header) : Bitmap(chunk, header) {
    _bitmapHeader = header;
}

SpriteFrame::~SpriteFrame() {
    delete _bitmapHeader;
    _bitmapHeader = nullptr;
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

Sprite::~Sprite() {
    for (SpriteFrame *frame : _frames) {
        delete frame;
    }
}

Operand Sprite::callMethod(BuiltInFunction methodId, Common::Array<Operand> &args) {
    switch (methodId) {
        case BuiltInFunction::spatialShow: {
            assert(args.size() == 0);
            spatialShow();
            return Operand();
        }

        case BuiltInFunction::timePlay: {
            assert(args.size() == 0);
            timePlay();
            return Operand();
        }

        case BuiltInFunction::movieReset: {
            assert(args.size() == 0);
            debugC(5, kDebugScript, "Sprite::movieReset(): Sprite reset");
            _isPlaying = false;
            _startTime = 0;
            _lastProcessedTime = 0;
            // TODO: This won't add back to the playing assets array!
            return Operand();
        }

        default: {
            error("Got unimplemented method ID %d", methodId);
        }
    }
}

void Sprite::spatialShow() {
    debugC(5, kDebugScript, "Sprite::spatialShow(): Sprite now showing");
    _isPlaying = true;
    _isSpatialShowOnly = true;
    g_engine->addPlayingAsset(this);
}

void Sprite::timePlay() {
    //if (_isPlaying) {
    //    error("Sprite::timePlay(): Attempted to play a sprite that is already playing");
    //    return;
    //}

    // SET ANIMATION VARIABLES.
    debugC(5, kDebugScript, "Sprite::timePlay(): Sprite playback started");
    _isPlaying = true;
    _isSpatialShowOnly = false;
    _startTime = g_system->getMillis();
    _lastProcessedTime = 0;
    g_engine->addPlayingAsset(this);

    // GET THE DURATION OF THE SPRITE.
    if (_header->_frameRate == 0) {
        _header->_frameRate = 10;
    }
    _duration = ((double)_frames.size() / _header->_frameRate) * 1000; 

    // RUN THE MOVIE START EVENT HANDLER.
    EventHandler *startEvent = _header->_eventHandlers.getValOrDefault(EventHandler::Type::MovieBegin);
    if (startEvent != nullptr) {
        debugC(5, kDebugScript, "Sprite::timePlay(): Executing start event handler");
        startEvent->execute(_header->_id);
    } else {
        debugC(5, kDebugScript, "Sprite::timePlay(): No start event handler");
    }
}

void Sprite::process() {
    debugC(5, kDebugGraphics, "Sprite %d: Redrawing", _header->_id);
    if (_isSpatialShowOnly) { // This is really isSpatialShow
        drawFirstFrame();
    } else {
        drawNextFrame();
    }
    // Sprites arenʻt showing because weʻre not respecting z-indices, and the movie is being rendered AFTER the 
    // sprite, so it isnʻt being shown!

    // TODO: I don't think sprites support time-based event handlers. Because we
    // have a separate timer for restarting the sprite when it expires.
}

void Sprite::readChunk(Chunk &chunk) {
    // Reads one frame from the sprite.
    debugC(5, kDebugLoading, "Sprite::readFrame(): Reading sprite frame (@0x%lx)", chunk.pos());
    SpriteFrameHeader *header = new SpriteFrameHeader(chunk);
    SpriteFrame *frame = new SpriteFrame(chunk, header);
    _frames.push_back(frame);

    Common::sort(_frames.begin(), _frames.end(), [](SpriteFrame *a, SpriteFrame *b) {
        return a->index() < b->index();
    });
}

void Sprite::drawFirstFrame() {
    // GET THE FIRST FRAME.
    SpriteFrame *firstFrame = _frames[0];
    for (SpriteFrame *frame : _frames) {
        if (frame->index() < firstFrame->index()) {
            firstFrame = frame;
        }
    }

    SpriteFrame *frame = firstFrame;
    uint frameLeft = frame->left() + _header->_boundingBox->left;
    uint frameTop = frame->top() + _header->_boundingBox->top;
    debugC(7, kDebugGraphics, "SPRITE: Drawing still frame %d (%d x %d) @ (%d, %d)", frame->index(), frame->width(), frame->height(), frameLeft, frameTop);
    g_engine->_screen->transBlitFrom(frame->surface, Common::Point(frameLeft, frameTop), 0, false);
}

bool Sprite::drawNextFrame() {
    // DETERMINE WHICH FRAMES NEED TO BE DRAWN.
    uint currentTime = g_system->getMillis();
    uint movieTime = currentTime - _startTime;
    debugC(8, kDebugGraphics, "Sprite::drawNextFrame(): Starting frame blitting (sprite time: %d)", movieTime);
    if (movieTime > _duration) {
        // RESET ANIMATION VARIABLES.
        _isPlaying = false;
        _startTime = 0;
        _lastProcessedTime = 0;

        // RUN THE SPRITE END EVENT HANDLER.
        EventHandler *endEvent = _header->_eventHandlers.getValOrDefault(EventHandler::Type::MovieEnd);
        if (endEvent != nullptr) {
            debugC(5, kDebugScript, "Sprite::drawNextFrame(): Executing end event handler");
            endEvent->execute(_header->_id);
        } else {
            debugC(5, kDebugScript, "Sprite::drawNextFrame: No end event handler");
        }
        return false;
    }

    uint frameIndex = 0;
    Common::Array<SpriteFrame *> framesToDraw;
    for (SpriteFrame *frame : _frames) {
        // Unlike movies, sprite frames don't have individual durations. They
        // are guaranteed to be multiples of the given frame rate.
        uint frameStartInMilliseconds = frameIndex * _header->_frameRate * 10;
        uint frameEndInMilliseconds = (frameIndex + 1) * _header->_frameRate * 10;
        // Unlike movies, sprite frame coordinates are relative to the sprite
        // bounding box.
        uint frameLeft = frame->left() + _header->_boundingBox->left;
        uint frameTop = frame->top() + _header->_boundingBox->top;

        bool isAfterStart = (_startTime + frameStartInMilliseconds) <= currentTime;
        bool isBeforeEnd = (_startTime + frameEndInMilliseconds) >= currentTime;
        if (!isAfterStart || (isAfterStart && !isBeforeEnd)) {
            frameIndex++;
            continue;
        }
        debugC(7, kDebugGraphics, "SPRITE: (time: %d ms) Drawing frame %d (%d x %d) @ (%d, %d); start: %d ms, end: %d ms", movieTime, frame->index(), frame->width(), frame->height(), frameLeft, frameTop, frameStartInMilliseconds, frameEndInMilliseconds);
        framesToDraw.push_back(frame);
        frameIndex++;
    }

    // BLIT THE FRAMES.
    for (SpriteFrame *frame : framesToDraw) {
        uint frameLeft = frame->left() + _header->_boundingBox->left;
        uint frameTop = frame->top() + _header->_boundingBox->top;
        debugC(7, kDebugGraphics, "SPRITE: (time: %d ms) Drawing frame %d (%d x %d) @ (%d, %d)", movieTime, frame->index(), frame->width(), frame->height(), frameLeft, frameTop);
        g_engine->_screen->transBlitFrom(frame->surface, Common::Point(frameLeft, frameTop), 0, false);
    }
    // The main game loop takes care of updating the screen.

    uint frameBlitEnd = g_system->getMillis() - _startTime;
    uint elapsedTime = frameBlitEnd - movieTime;
    debugC(8, kDebugGraphics, "Sprite::drawNextFrame(): Finished frame blitting in %d ms (current sprite time: %d ms)", elapsedTime, frameBlitEnd);
    return true;
}

} // End of namespace MediaStation