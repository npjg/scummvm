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
#include "mediastation/debugchannels.h"

#include "mediastation/assets/timer.h"

namespace MediaStation {

void Timer::play() {
    if (_isPlaying) {
        warning("Timer::play(): Attempted to play a timer that is already playing");
        return;
    }

    // SET TIMER VARIABLES.
    _isPlaying = true;
    _startTime = g_system->getMillis();
    _lastProcessedTime = 0;

    // GET THE DURATION OF THE TIMER.
    // TODO: Is there a better way to find out what the max time is? Do we have to look
    // through each of the timer event handlers to figure it out?
    _duration = 0;
    for (EventHandler *timeEvent : _header->_timeHandlers) {
        // TODO: Centralize this converstion to milliseconds, as the same logic
        // is being used in several places.
        double timeEventInFractionalSeconds = timeEvent->_argumentValue.u.f;
        uint timeEventInMilliseconds = timeEventInFractionalSeconds * 1000;
        if (timeEventInMilliseconds > _duration) {
            _duration = timeEventInMilliseconds;
        }
    }    
}

void Timer::stop() {
    if (!_isPlaying) {
        warning("Timer::stop(): Attempted to stop a timer that is not playing");
        return;
    }

    _isPlaying = false;
    _startTime = 0;
    _lastProcessedTime = 0;
}

void Timer::process() {
    if (!_isPlaying) {
        warning("Timer::processTimeEventHandlers(): Attempted to process time event handlers while not playing");
        return;
    }

    uint currentTime = g_system->getMillis();
    uint movieTime = currentTime - _startTime;
    if (movieTime > _duration) {
        // We are done processing the timer.
        _isPlaying = false;
        _startTime = 0;
        _lastProcessedTime = 0;
        return;
    }
    for (EventHandler *timeEvent : _header->_timeHandlers) {
        double timeEventInFractionalSeconds = timeEvent->_argumentValue.u.f;
        uint timeEventInMilliseconds = timeEventInFractionalSeconds * 1000;
        bool timeEventAlreadyProcessed = timeEventInMilliseconds < _lastProcessedTime;
        bool timeEventNeedsToBeProcessed = timeEventInMilliseconds <= currentTime - _startTime;
        if (!timeEventAlreadyProcessed && timeEventNeedsToBeProcessed) {
            debugC(5, kDebugScript, "Timer::processTimeEventHandlers(): Running On Time handler for time %d ms (movie time: %d ms)", timeEventInMilliseconds, currentTime - _startTime);
            timeEvent->execute();
        }
    }
    _lastProcessedTime = currentTime - _startTime;
}

} // End of namespace MediaStation
