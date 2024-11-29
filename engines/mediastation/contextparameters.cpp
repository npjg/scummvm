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
#include "mediastation/datum.h"
#include "mediastation/contextparameters.h"
#include "mediastation/mediascript/variabledeclaration.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

ContextParameters::ContextParameters(Chunk &chunk) : contextName(nullptr) {
    fileNumber = Datum(chunk, DatumType::UINT16_1).u.i;
    uint sectionType = Datum(chunk, DatumType::UINT16_1).u.i;
    while ((SectionType)sectionType != SectionType::EMPTY) {
        debugC(5, kDebugLoading, "ContextParameters::ContextParameters: sectionType = 0x%x (@0x%lx)", sectionType, chunk.pos());
        switch ((SectionType)sectionType) {
            case SectionType::NAME: {
                uint repeatedFileNumber = Datum(chunk, DatumType::UINT16_1).u.i;
                if (repeatedFileNumber != fileNumber) {
                    warning("ContextParameters::ContextParameters(): Repeated file number didn't match: %d != %d", repeatedFileNumber, fileNumber);
                }
                contextName = Datum(chunk, DatumType::STRING).u.string;
                // TODO: This is likely just an end flag.
                uint unk1 = Datum(chunk, DatumType::UINT16_1).u.i;
            }
            
            case SectionType::FILE_NUMBER: {
                error("ContextParameters::ContextParameters(): Section type FILE_NUMBER not implemented yet");
                break;
            }

            case SectionType::VARIABLE: {
                uint repeatedFileNumber = Datum(chunk, DatumType::UINT16_1).u.i;
                if (repeatedFileNumber != fileNumber) {
                    warning("ContextParameters::ContextParameters(): Repeated file number didn't match: %d != %d", repeatedFileNumber, fileNumber);
                }
                Variable *variable = new Variable(chunk);
                _variables.setVal(variable->id, variable);
                break;
            }

            case SectionType::BYTECODE: {
                Function *function = new Function(chunk);
                _functions.setVal(function->_id, function);
                break;
            }

            default: {
                error("ContextParameters::ContextParameters(): Unknown section type 0x%x", sectionType);
            }
        }
        sectionType = Datum(chunk, DatumType::UINT16_1).u.i;
    }
}

ContextParameters::~ContextParameters() {
    delete contextName;
    _variables.clear();
}

} // End of namespace MediaStation
