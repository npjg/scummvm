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

#include "mediastation/mediascript/eventhandler.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

EventHandler::EventHandler(Chunk &chunk): _code(nullptr) {
    _type = (EventHandler::Type)(Datum(chunk).u.i);
    debugC(5, kDebugLoading, "EventHandler::EventHandler(): Type 0x%x (@0x%lx)", _type, chunk.pos());
    _argumentType = (EventHandler::ArgumentType)(Datum(chunk).u.i);
    debugC(5, kDebugLoading, "EventHandler::EventHandler(): Argument type 0x%x (@0x%lx)", _argumentType, chunk.pos());
    _argumentValue = Datum(chunk);

    if (_argumentType != EventHandler::ArgumentType::Null) {
        uint lengthInBytes = Datum(chunk, DatumType::UINT32_1).u.i;
        debugC(5, kDebugLoading, "EventHandler::EventHandler(): Null argument type, length = 0x%x (@0x%lx)", lengthInBytes, chunk.pos());
    }

    _code = new CodeChunk(chunk);
}

Operand EventHandler::execute() {
    // The only argument that can be provided to an event handler is the
    // _argumentValue.
    // So if we are here, we can execute it directly.
    return _code->execute();
}

EventHandler::~EventHandler() {
    delete _code;
}

} // End of namespace MediaStation
