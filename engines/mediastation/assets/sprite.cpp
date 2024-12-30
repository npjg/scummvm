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
	debugC(5, kDebugLoading, "SpriteFrameHeader::SpriteFrameHeader(): _index = 0x%x (@0x%llx)", _index, chunk.pos());
	_boundingBox = Datum(chunk, DatumType::POINT_2).u.point;
	debugC(5, kDebugLoading, "SpriteFrameHeader::SpriteFrameHeader(): _boundingBox (@0x%llx)", chunk.pos());
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

Operand Sprite::callMethod(BuiltInMethod methodId, Common::Array<Operand> &args) {
	switch (methodId) {
	case BuiltInMethod::spatialShow: {
		assert(args.size() == 0);
		spatialShow();
		return Operand();
	}

	case BuiltInMethod::timePlay: {
		assert(args.size() == 0);
		timePlay();
		return Operand();
	}

	case BuiltInMethod::movieReset: {
		assert(args.size() == 0);
		debugC(5, kDebugScript, "Sprite::movieReset(): Sprite reset");
		_isPlaying = true;
		_startTime = 0;
		_currentFrameIndex = 0;
		_nextFrameTime = 0;
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
	g_engine->addPlayingAsset(this);

	// Persist the first frame.
	// TODO: Is there anything that says what the persisted frame should be?
	SpriteFrame *firstFrame = _frames[0];
	for (SpriteFrame *frame : _frames) {
		if (frame->index() < firstFrame->index()) {
			firstFrame = frame;
		}
	}
	_persistFrame = firstFrame;
}

void Sprite::timePlay() {
	// SET ANIMATION VARIABLES.
	debugC(5, kDebugScript, "Sprite::timePlay(): Sprite playback started");
	_isPlaying = true;
	_persistFrame = nullptr;
	_startTime = g_system->getMillis();
	_lastProcessedTime = 0;
	_nextFrameTime = 0;
	g_engine->addPlayingAsset(this);

	if (_header->_frameRate == 0) {
		_header->_frameRate = 10;
	}

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
	drawNextFrame();

	// TODO: I don't think sprites support time-based event handlers. Because we
	// have a separate timer for restarting the sprite when it expires.
}

void Sprite::readChunk(Chunk &chunk) {
	// Reads one frame from the sprite.
	debugC(5, kDebugLoading, "Sprite::readFrame(): Reading sprite frame (@0x%llx)", chunk.pos());
	SpriteFrameHeader *header = new SpriteFrameHeader(chunk);
	SpriteFrame *frame = new SpriteFrame(chunk, header);
	_frames.push_back(frame);

	Common::sort(_frames.begin(), _frames.end(), [](SpriteFrame * a, SpriteFrame * b) {
		return a->index() < b->index();
	});
}

bool Sprite::drawNextFrame() {
	if (_persistFrame != nullptr) {
		drawFrame(_persistFrame);
		return true;
	}

	uint currentTime = g_system->getMillis() - _startTime;
	if (currentTime <= _nextFrameTime) {
		// Just redraw the current frame in case it was covered over.
		// This will change when the rendering is reworked.
		drawFrame(_frames[_currentFrameIndex]);
		return true;
	}

	_nextFrameTime = _currentFrameIndex * (1000 / _header->_frameRate);
	warning("Next frame (%d * (1000 / %d)) : %d ms", _currentFrameIndex, _header->_frameRate, _nextFrameTime);
	drawFrame(_frames[_currentFrameIndex]);

	bool spriteFinishedPlaying = (++_currentFrameIndex == _frames.size());
	if (spriteFinishedPlaying) {
		// RESET ANIMATION VARIABLES.
		// Sprites always keep their last frame showing until they are hidden again.
		_isPlaying = true;
		_persistFrame = _frames[_currentFrameIndex - 1];
		_startTime = 0;
		_lastProcessedTime = 0;
		_currentFrameIndex = 0;
		_nextFrameTime = 0;

		// RUN THE SPRITE END EVENT HANDLER.
		EventHandler *endEvent = _header->_eventHandlers.getValOrDefault(EventHandler::Type::MovieEnd);
		if (endEvent != nullptr) {
			debugC(5, kDebugScript, "Sprite::drawNextFrame(): Executing end event handler");
			endEvent->execute(_header->_id);
		} else {
			debugC(5, kDebugScript, "Sprite::drawNextFrame(): No stopped event handler");
		}
		return false;
	}

	return true;
}

void Sprite::drawFrame(SpriteFrame *frame) {
	uint frameLeft = frame->left() + _header->_boundingBox->left;
	uint frameTop = frame->top() + _header->_boundingBox->top;
	//debugC(7, kDebugGraphics, "SPRITE: (time: %d ms) Drawing frame %d (%d x %d) @ (%d, %d)", movieTime, frame->index(), frame->width(), frame->height(), frameLeft, frameTop);
	g_engine->_screen->transBlitFrom(frame->surface, Common::Point(frameLeft, frameTop), 0, false);
}

} // End of namespace MediaStation