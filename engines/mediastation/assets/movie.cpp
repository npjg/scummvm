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

#include "mediastation/mediastation.h"
#include "mediastation/assets/movie.h"
#include "mediastation/datum.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

MovieFrameHeader::MovieFrameHeader(Chunk &chunk) : BitmapHeader(chunk) {
    _index = Datum(chunk).u.i;
    debugC(5, kDebugLoading, "MovieFrameHeader::MovieFrameHeader(): _index = 0x%x (@0x%lx)", _index, chunk.pos());
    _keyframeEndInMilliseconds = Datum(chunk).u.i;
}

MovieFrameFooter::MovieFrameFooter(Chunk &chunk) {
    _unk1 = Datum(chunk).u.i;
    _unk2 = Datum(chunk).u.i;
    if (g_engine->isFirstGenerationEngine()) { // TODO: Add the version number check. 
        _startInMilliseconds = Datum(chunk).u.i;
        _endInMilliseconds = Datum(chunk).u.i;
        _left = Datum(chunk).u.i;
        _top = Datum(chunk).u.i;
        _unk3 = Datum(chunk).u.i;
        _unk4 = Datum(chunk).u.i;
        _index = Datum(chunk).u.i;
    } else {
        _unk4 = Datum(chunk).u.i;
        _startInMilliseconds = Datum(chunk).u.i;
        _endInMilliseconds = Datum(chunk).u.i;
        _left = Datum(chunk).u.i;
        _top = Datum(chunk).u.i;
        _zIndex = Datum(chunk).u.i;
        _unk6 = Datum(chunk).u.i;
        _unk7 = Datum(chunk).u.i;
        _index = Datum(chunk).u.i;
        _unk8 = Datum(chunk).u.i;
        _unk9 = Datum(chunk).u.i;
        debugC(5, kDebugLoading, "MovieFrameFooter::MovieFrameFooter(): _startInMilliseconds = 0x%x, _endInMilliseconds = 0x%x, _left = 0x%x, _top = 0x%x, _index = 0x%x (@0x%lx)", _startInMilliseconds, _endInMilliseconds, _left, _top, _index, chunk.pos());
        debugC(5, kDebugLoading, "MovieFrameFooter::MovieFrameFooter(): _unk4 = 0x%x, _unk5 = 0x%x, _unk6 = 0x%x, _unk7 = 0x%x, _unk8 = 0x%x, _unk9 = 0x%x", _unk4, _zIndex, _unk6, _unk7, _unk8, _unk9);
    }
}

MovieFrame::MovieFrame(Chunk &chunk, MovieFrameHeader *header) :
    Bitmap(chunk, header), 
    _footer(nullptr),
    _showing(false) 
    {
        _bitmapHeader = header;
    }

void MovieFrame::setFooter(MovieFrameFooter *footer) {
    if (footer != nullptr) {
        assert(footer->_index == _bitmapHeader->_index);
    }
    _footer = footer;
}

uint32 MovieFrame::left() {
    if (_footer != nullptr) {
        return _footer->_left;
    } else {
        error("MovieFrame::left(): Cannot get the left coordinate of a keyframe");
    }
}

uint32 MovieFrame::top() {
    if (_footer != nullptr) {
        return _footer->_top;
    } else {
        error("MovieFrame::left(): Cannot get the top coordinate of a keyframe");
    }
}

Common::Point MovieFrame::topLeft() {
    if (_footer != nullptr) {
        return Common::Point(_footer->_left, _footer->_top);
    } else {
        error("MovieFrame::topLeft(): Cannot get the top-left coordinate of a keyframe");
    }
}

Common::Rect MovieFrame::boundingBox() {
    if (_footer != nullptr) {
        return Common::Rect(Common::Point(_footer->_left, _footer->_top), width(), height());
    } else {
        error("MovieFrame::boundingBox(): Cannot get the bounding box of a keyframe");
    }
}

uint32 MovieFrame::index() {
    if (_footer != nullptr) {
        return _footer->_index;
    } else {
        error("MovieFrame::index(): Cannot get the index of a keyframe");
    }
}

uint32 MovieFrame::startInMilliseconds() {
    if (_footer != nullptr) {
        return _footer->_startInMilliseconds;
    } else {
        error("MovieFrame::startInMilliseconds(): Cannot get the start time of a keyframe");
    }
}

uint32 MovieFrame::endInMilliseconds() {
    if (_footer != nullptr) {
        return _footer->_endInMilliseconds;
    } else {
        error("MovieFrame::endInMilliseconds(): Cannot get the end time of a keyframe");
    }
}

uint32 MovieFrame::zCoordinate() {
    if (_footer != nullptr) {
        return _footer->_zIndex;
    } else {
        error("MovieFrame::zCoordinate(): Cannot get the z-coordinate of a keyframe");
    }
}

uint32 MovieFrame::keyframeEndInMilliseconds() {
    return _bitmapHeader->_keyframeEndInMilliseconds;
}

MovieFrame::~MovieFrame() {
    delete _footer;
    _footer = nullptr;
}

Movie::~Movie() {
    for (MovieFrame *frame : _stills) {
        delete frame;
    }
    _stills.clear();
    for (MovieFrame *frame : _frames) {
        delete frame;
    }
    _frames.clear();
    //for (Sound *sound : _sounds) {
    //    delete sound;
    //}
    //_sounds.clear();
    for (MovieFrameFooter *footer : _footers) {
        delete footer;
    }
    _footers.clear();
}

Operand Movie::callMethod(BuiltInFunction methodId, Common::Array<Operand> &args) {
    switch (methodId) {
        case BuiltInFunction::timePlay: {
            assert(args.empty());
            timePlay();

            return Operand();
        }

        default: {
            error("Got unimplemented method ID %d", methodId);
        }
    }
}

void Movie::timePlay() {
    if (_isPlaying) {
        error("Movie::play(): Attempted to play a movie that is already playing");
        return;
    }

    // SET ANIMATION VARIABLES.
    _isPlaying = true;
    _startTime = g_system->getMillis();
    _lastProcessedTime = 0;
    g_engine->addPlayingAsset(this);

    // GET THE DURATION OF THE MOVIE.
    _duration = 0;
    for (MovieFrame *frame : _frames) {
        if (frame->endInMilliseconds() > _duration) {
            _duration = frame->endInMilliseconds();
        }
    }

    // RUN THE MOVIE START EVENT HANDLER.
    EventHandler *startEvent = _header->_eventHandlers.getValOrDefault(EventHandler::Type::MovieBegin);
    if (startEvent != nullptr) {
        debugC(5, kDebugScript, "Movie::play(): Executing movie start event handler");
        startEvent->execute(_header->_id);
    }
}

void Movie::timeStop() {
    // RESET ANIMATION VARIABLES.
    _isPlaying = false;
    _startTime = 0;
    _lastProcessedTime = 0;

    // RUN THE MOVIE STOPPED EVENT HANDLER.
    EventHandler *endEvent = _header->_eventHandlers.getValOrDefault(EventHandler::Type::MovieStopped);
    if (endEvent != nullptr) {
        debugC(5, kDebugScript, "Movie::play(): Executing movie stopped event handler");
        endEvent->execute(_header->_id);
    }
}

void Movie::process() {
    debugC(5, kDebugGraphics, "Movie %d: Redrawing", _header->_id);
    processTimeEventHandlers();
    drawNextFrame();
}

void Movie::processTimeEventHandlers() {
    if (!_isPlaying) {
        warning("Movie::processTimeEventHandlers(): Attempted to process time event handlers while movie is not playing");
        return;
    }

    uint currentTime = g_system->getMillis();
    for (EventHandler *timeEvent : _header->_timeHandlers) {
        double timeEventInFractionalSeconds = timeEvent->_argumentValue.u.f;
        uint timeEventInMilliseconds = timeEventInFractionalSeconds * 1000;
        bool timeEventAlreadyProcessed = timeEventInMilliseconds < _lastProcessedTime;
        bool timeEventNeedsToBeProcessed = timeEventInMilliseconds <= currentTime - _startTime;
        if (!timeEventAlreadyProcessed && timeEventNeedsToBeProcessed) {
            debugC(5, kDebugScript, "Movie::processTimeEventHandlers(): Running On Time handler for movie time %d ms (real movie time: %d ms)", timeEventInMilliseconds, currentTime - _startTime);
            timeEvent->execute(_header->_id);
        }
    }
    _lastProcessedTime = currentTime - _startTime;
}

bool Movie::drawNextFrame() {
    // DETERMINE WHICH FRAMES NEED TO BE DRAWN.
    uint currentTime = g_system->getMillis();
    uint movieTime = currentTime - _startTime;
    debugC(8, kDebugGraphics, "Movie::drawNextFrame(): Starting frame blitting (movie time: %d)", movieTime);
    if (movieTime > _duration) {
        // RESET ANIMATION VARIABLES.
        _isPlaying = false;
        _startTime = 0;
        _lastProcessedTime = 0;

        // RUN THE MOVIE STOPPED EVENT HANDLER.
        EventHandler *endEvent = _header->_eventHandlers.getValOrDefault(EventHandler::Type::MovieEnd);
        if (endEvent != nullptr) {
            debugC(5, kDebugScript, "Movie::drawNextFrame(): Executing movie end event handler");
            endEvent->execute(_header->_id);
        }
        return false;
    }

    Common::Array<MovieFrame *> framesToDraw;
    for (MovieFrame *frame : _frames) {
        bool isAfterStart = _startTime + frame->startInMilliseconds() <= currentTime;
        bool isBeforeEnd = _startTime + frame->endInMilliseconds() >= currentTime;
        if (!isAfterStart || (isAfterStart && !isBeforeEnd)) {
            continue;
        }
        debugC(7, kDebugGraphics, "(time: %d ms) Drawing frame %d (%d x %d) @ (%d, %d); start: %d ms, end: %d ms, keyframeEnd: %d ms, _unk5 = %d", movieTime, frame->index(), frame->width(), frame->height(), frame->left(), frame->top(), frame->startInMilliseconds(), frame->endInMilliseconds(), frame->keyframeEndInMilliseconds(), frame->zCoordinate());
        framesToDraw.push_back(frame);
    }

    // BLIT THE FRAMES.
    Common::sort(framesToDraw.begin(), framesToDraw.end(), [](MovieFrame *a, MovieFrame *b) {
        return a->zCoordinate() > b->zCoordinate();
    });
    for (MovieFrame *frame : framesToDraw) {
        debugC(7, kDebugGraphics, "(time: %d ms) Drawing frame %d (%d x %d) @ (%d, %d); start: %d ms, end: %d ms, keyframeEnd: %d ms, _unk5 = %d", movieTime, frame->index(), frame->width(), frame->height(), frame->left(), frame->top(), frame->startInMilliseconds(), frame->endInMilliseconds(), frame->keyframeEndInMilliseconds(), frame->zCoordinate());
        g_engine->_screen->transBlitFrom(frame->surface, Common::Point(frame->left(), frame->top()), 0, false);
    }
    // The main game loop takes care of updating the screen.

    uint frameBlitEnd = g_system->getMillis() - _startTime;
    uint elapsedTime = frameBlitEnd - movieTime;
    debugC(8, kDebugGraphics, "Movie::drawNextFrame(): Finished frame blitting in %d ms (current movie time: %d ms)", elapsedTime, frameBlitEnd);
    return true;
}

void Movie::readChunk(Chunk &chunk) {
    // Individual chunks are "stills" and are stored in the first subfile.
    uint sectionType = Datum(chunk).u.i;
    switch ((SectionType)sectionType) {
        case SectionType::FRAME: {
            debugC(5, kDebugLoading, "Movie::readStill(): Reading frame");
            MovieFrameHeader *header = new MovieFrameHeader(chunk);
            MovieFrame *frame = new MovieFrame(chunk, header);
            _stills.push_back(frame);
            break;
        }

        case SectionType::FOOTER: {
            debugC(5, kDebugLoading, "Movie::readStill(): Reading footer");
            MovieFrameFooter *footer = new MovieFrameFooter(chunk);
            _footers.push_back(footer);
            break;
        }

        default: {
            error("Unknown movie still section type");
        }
    }
}

void Movie::readSubfile(Subfile &subfile, Chunk &chunk) {
    // READ THE METADATA FOR THE WHOLE MOVIE.
    uint expectedRootSectionType = Datum(chunk).u.i;
    debugC(5, kDebugLoading, "Movie::readSubfile(): sectionType = 0x%x (@0x%lx)", expectedRootSectionType, chunk.pos());
    if (Movie::SectionType::ROOT != (Movie::SectionType)expectedRootSectionType) {
        error("Expected ROOT section type, got 0x%x", expectedRootSectionType);
    }
    uint chunkCount = Datum(chunk).u.i;
    debugC(5, kDebugLoading, "Movie::readSubfile(): chunkCount = 0x%x (@0x%lx)", chunkCount, chunk.pos());

    uint dataStartOffset = Datum(chunk).u.i;
    debugC(5, kDebugLoading, "Movie::readSubfile(): dataStartOffset = 0x%x (@0x%lx)", dataStartOffset, chunk.pos());

    Common::Array<uint> chunkLengths;
    for (uint i = 0; i < chunkCount; i++) {
        uint chunkLength = Datum(chunk).u.i;
        debugC(5, kDebugLoading, "Movie::readSubfile(): chunkLength = 0x%x (@0x%lx)", chunkLength, chunk.pos());
        chunkLengths.push_back(chunkLength);
    }

    // RAD THE INTERLEAVED AUDIO AND ANIMATION DATA.
    for (uint i = 0; i < chunkCount; i++) {
        debugC(5, kDebugLoading, "\nMovie::readSubfile(): Reading frameset %d of %d in subfile (@0x%lx)", i, chunkCount, chunk.pos());
        chunk = subfile.nextChunk();

        // READ ALL THE FRAMES IN THIS CHUNK.
        debugC(5, kDebugLoading, "Movie::readSubfile(): (Frameset %d of %d) Reading animation chunks... (@0x%lx)", i, chunkCount, chunk.pos());
        bool isAnimationChunk = (chunk.id == _header->_animationChunkReference);
        if (!isAnimationChunk) {
            warning("Movie::readSubfile(): (Frameset %d of %d) No animation chunks found (0x%lx)", i, chunkCount, chunk.pos());
        }
        MovieFrameHeader *header = nullptr;
        MovieFrame *frame = nullptr;
        while (isAnimationChunk) {
            uint sectionType = Datum(chunk).u.i;
            debugC(5, kDebugLoading, "Movie::readSubfile(): sectionType = 0x%x (@0x%lx)", sectionType, chunk.pos());
            switch (Movie::SectionType(sectionType)) {
                case Movie::SectionType::FRAME: {
                    header = new MovieFrameHeader(chunk);
                    frame = new MovieFrame(chunk, header);
                    _frames.push_back(frame);
                    break;
                }

                case Movie::SectionType::FOOTER: {
                    MovieFrameFooter *footer = new MovieFrameFooter(chunk);
                    // _footers.push_back(footer);
                    // TODO: This does NOT handle the case where there are
                    // keyframes. We need to match the footer to an arbitrary
                    // frame, since some keyframes don't have footers, sigh.
                    if (header == nullptr) {
                        error("Movie::readSubfile(): No frame to match footer to");
                    }
                    if (header->_index == footer->_index) {
                        frame->setFooter(footer);
                    } else {
                        error("Movie::readSubfile(): Footer index does not match frame index: %d != %d", header->_index, footer->_index);
                    }
                    break;
                }

                default: {
                    error("Movie::readSubfile(): Unknown movie animation section type 0x%x (@0x%lx)", sectionType, chunk.pos());
                }
            }

            // READ THE NEXT CHUNK.
            chunk = subfile.nextChunk();
            isAnimationChunk = (chunk.id == _header->_animationChunkReference);
        }

        // READ THE AUDIO.
        debugC(5, kDebugLoading, "Movie::readSubfile(): (Frameset %d of %d) Reading audio chunk... (0x%lx)", i, chunkCount, chunk.pos());
        bool isAudioChunk = (chunk.id = _header->_audioChunkReference);
        if (isAudioChunk) {
            // TODO: Actually read the audio chunk.
            //Sound *sound = new Sound(_header->_soundEncoding);
            //sound->readChunk(chunk);
            //_sounds.push_back(sound);
            chunk.skip(chunk.length);
            chunk = subfile.nextChunk();
        } else {
            debugC(5, kDebugLoading, "Movie::readSubfile(): (Frameset %d of %d) No audio chunk to read. (0x%lx)", i, chunkCount, chunk.pos());
        }

        // READ THE FOOTER FOR THIS SUBFILE.
        debugC(5, kDebugLoading, "Movie::readSubfile(): (Frameset %d of %d) Reading header chunk... (@0x%lx)", i, chunkCount, chunk.pos());
        bool isHeaderChunk = (chunk.id == _header->_chunkReference);
        if (isHeaderChunk) {
            if (chunk.length != 0x04) {
                error("Movie::readSubfile(): Expected movie header chunk of size 0x04, got 0x%x (@0x%lx)", chunk.length, chunk.pos());
            }
            chunk.skip(chunk.length);
        } else {
            error("Movie::readSubfile(): Expected header chunk, got %s (@0x%lx)", tag2str(chunk.id), chunk.pos());
        }
    }

    // SET THE MOVIE FRAME FOOTERS.
    // TODO: We donʻt do anything with this yet!
}

} // End of namespace MediaStation