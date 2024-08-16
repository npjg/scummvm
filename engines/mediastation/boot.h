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
#include "mediastation/subfile.h"

#ifndef MEDIASTATION_BOOT_H
#define MEDIASTATION_BOOT_H

namespace MediaStation {

class VersionInfo {
public:
    VersionInfo();
    ~VersionInfo();

    uint32 majorVersion;
    uint32 minorVersion;
    uint32 revision;
    Common::String string;
};

class Boot : Datafile {
private:
    Subfile subfile;

private:
    enum class SectionType {
        EMPTY = 0x0000,
        CONTEXT_DECLARATION = 0x0002,
        VERSION_INFORMATION = 0x0190,
        ENGINE_RESOURCE_NAME = 0x0bba,
        ENGINE_RESOURCE_ID = 0x0bbb,
        UNKNOWN_DECLARATION = 0x0007,
        FILE_DECLARATION = 0x000a,
        RIFF_DECLARATION = 0x000b,
        CURSOR_DECLARATION = 0x0015
    };

public:
    Boot(const Common::Path &path);

    Common::String game_title;

};



} // End of namespace MediaStation

#endif