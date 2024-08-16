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

#include "mediastation/datafile.h"

#ifndef MEDIASTATION_CONTEXT_H
#define MEDIASTATION_CONTEXT_H

namespace MediaStation {

class Context : Datafile {
    public:
        Context(const Common::Path &path);
        ~Context();

        bool readPreamble();

    private:    
        uint32 unk1;
        uint32 subfile_count;
        uint32 file_size;
};

} // End of namespace MediaStation

#endif