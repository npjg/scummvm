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

#include "mediastation/mediascript/variabledeclaration.h"
#include "mediastation/chunk.h"
#include "mediastation/datum.h"
#include "mediastation/datafile.h"

namespace MediaStation {

VariableDeclaration::VariableDeclaration(Chunk &chunk) {
    id = Datum(chunk, DatumType::UINT16_1).u.i;
    type = Datum(chunk, DatumType::UINT8).u.i;
    debugC(5, kDebugLoading, "VariableDeclaration::VariableDeclaration(): id = 0x%x, type 0x%x (@0x%lx)", id, type, chunk.pos());
    switch ((Type)type) {
        case Type::COLLECTION: {
            uint totalItems = Datum(chunk).u.i;
            value.collection = new Common::Array<VariableDeclaration *>;
            for (uint i = 0; i < totalItems; i++) {
                VariableDeclaration *variableDeclaration = new VariableDeclaration(chunk);
                value.collection->push_back(variableDeclaration);
            }
            break;
        }

        case Type::STRING: {
            // TODO: This copies the string. Can we read it directly from the chunk?
            int size = Datum(chunk).u.i;
            char* buffer = new char[size + 1];
            chunk.read(buffer, size);
            buffer[size] = '\0';
            value.string = new Common::String(buffer);
            delete[] buffer;
            break;
        }

        case Type::ASSET_ID: {
            value.assetId = Datum(chunk, DatumType::UINT16_1).u.i;
            break;
        }

        case Type::BOOLEAN: {
            uint rawValue = Datum(chunk, DatumType::UINT16_1).u.i;
            value.b = (rawValue == 1);
            break;
        }

        case Type::LITERAL: {
            // Client code can worry about extracting the value.
            value.datum = new Datum(chunk);
        }

        default: {
            warning("VariableDeclaration::VariableDeclaration(): Got unknown variable value type 0x%x", type);
            value.datum = new Datum(chunk);
        }
    }
}

VariableDeclaration::~VariableDeclaration() {
    switch ((Type)type) {
        case Type::ASSET_ID: 
        case Type::BOOLEAN: {
            break;
        }

        case Type::COLLECTION: {
            delete value.collection;
            break;
        }

        case Type::STRING: {
            delete value.string;
            break;
        }

        case Type::LITERAL: {
            delete value.datum;
            break;
        }

        default: {
            delete value.datum;
            break;
        }
    }
}

} // End of namespace MediaStation