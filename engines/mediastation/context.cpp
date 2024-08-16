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

#include "mediastation/context.h"
#include "mediastation/mediastation.h"

namespace MediaStation {

Context::Context(const Common::Path &path) : Datafile(path) {

}

bool Context::readPreamble() {
    uint16 signature = _stream->readUint16LE();
    if (signature != 0x4949) { // "II"
        warning("Datafile::openFile(): Wrong signature for file %s. Got 0x%04X", _path.toString(Common::Path::kNativeSeparator).c_str(), signature);
		close();
		return false;
    }

    unk1 = _stream->readUint32LE();
    debugC(5, kDebugLoading, "Contextt::openFile(): unk1 = %d", unk1);

    subfile_count = _stream->readUint32LE();
    // The total size of this file, including this header.
    // (Basically the true file size shown on the filesystem.)
    file_size = _stream->readUint32LE();
}

} // End of namespace MediaStation
