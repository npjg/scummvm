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

#include "mediastation/assets/path.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {


Path::Path(AssetHeader *header) : _header(header) {

}

Path::~Path() {
    // We don't own the header so we don't need to delete it.
    _header = nullptr;
}

void Path::play() {
    // TODO: Check that itʻs zero before we reset it, since this function isn't re-entrant!
    _percentComplete = 0;

    if (_header->_duration == 0) {
        warning("Path::play(): Got zero duration");
    } else if (_header->_stepRate == 0) {
        error("Path::play(): Got zero step rate");
    }
    uint totalSteps = (_header->_duration * _header->_stepRate) / 1000;
    uint stepDurationInMilliseconds = 1000 / _header->_stepRate;
    debugC(5, kDebugGraphics, "Path::play(): durationInMilliseconds = %d, totalSteps = %d, stepDurationInMilliseconds = %d", _header->_duration, totalSteps, stepDurationInMilliseconds);

    // RUN THE START EVENT HANDLER.
    EventHandler *startEventHandler = nullptr; // TODO: Haven't seen a path start event in the wild yet, don't know its ID.
    if (startEventHandler != nullptr) {
        debugC(5, kDebugScript, "Path::play(): Running PathStart event handler");
        startEventHandler->execute();
    }

    // STEP THE PATH.
    EventHandler *pathStepHandler = _header->_eventHandlers[EventHandler::Type::Step];
    for (uint i = 0; i < totalSteps; i++) {
        _percentComplete = (double)(i + 1) / totalSteps;
        debugC(5, kDebugGraphics, "Path::play(): Step %d of %d (%d%% complete)", i, totalSteps, _percentComplete);
        // TODO: Actually step the path. It seems they mostly just use this for
        // palette animation in the On Step event handler, so nothing is actually drawn on the screen now.

        // RUN THE ON STEP EVENT HANDLER.
        // TODO: Is this supposed to come after or before we step the path?
        if (pathStepHandler != nullptr) {
            debugC(5, kDebugScript, "Path::play(): Running PathStep event handler");
            pathStepHandler->execute();
        }
    }

    // RUN THE END EVENT HANDLER.
    EventHandler *endEventHandler = _header->_eventHandlers[EventHandler::Type::PathEnd];
    if (endEventHandler != nullptr) {
        debugC(5, kDebugScript, "Path::play(): Running PathEnd event handler");
        endEventHandler->execute();
    }

    // CLEAN UP.
    _percentComplete = 0;
}

void Path::setDuration(uint durationInMilliseconds) {
    // TODO: Do we need to save the original duration?
    debugC(5, kDebugScript, "Path::setDuration(): Setting duration to %d ms", durationInMilliseconds);
    _header->_duration = durationInMilliseconds;
}


double Path::percentComplete() {
    debugC(5, kDebugScript, "Path::percentComplete(): Returning percent complete %d%%", _percentComplete);
    return _percentComplete;
}

} // End of namespace MediaStation

