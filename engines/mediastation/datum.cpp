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
#include "mediastation/chunk.h"
#include "mediastation/datum.h"

namespace MediaStation {

Point::Point() : x(0), y(0) {}
Point::Point(Chunk &chunk) {
    x = Datum(chunk, DatumType::INT16_2).u.i;
    y = Datum(chunk, DatumType::INT16_2).u.i;
}

BoundingBox::BoundingBox(Chunk &chunk) {
    left_top_point = Datum(chunk, DatumType::POINT_2);
    dimensions = Datum(chunk, DatumType::POINT_1); 
}

Polygon::Polygon(Chunk &chunk) {
    int total_points = Datum(chunk, DatumType::UINT16_1).u.i;
    for (int i = 0; i < total_points; i++) {
        Datum point = Datum(chunk, DatumType::POINT_1);
        points.push_back(point);
    }
}

Reference::Reference() : chunk_id(0) {}
Reference::Reference(Chunk &chunk) {
    chunk_id = chunk.readUint32LE();
}

Datum::Datum() { 
    u.i = 0;
}

Datum::Datum(Chunk &chunk) {
    t = static_cast<DatumType>(chunk.readUint16LE());
    readWithType(chunk);
}

Datum::Datum(Chunk &chunk, DatumType expectedType) {
    t = static_cast<DatumType>(chunk.readUint16LE());
    if (t != expectedType) {
        error("Datum::Datum(): Expected datum type 0x%x, got 0x%x (@0x%lx)", expectedType, t, chunk.pos());
    }
    readWithType(chunk);
}

void Datum::readWithType(Chunk &chunk) {
    debugC(9, kDebugLoading, "Datum::Datum(): Type 0x%x (@0x%lx)", t, chunk.pos());
    if (DatumType::UINT8 == t) {
        u.i = chunk.readByte();
    } else if (DatumType::UINT16_1 == t || DatumType::UINT16_2 == t) {
        u.i = chunk.readUint16LE();
    } else if (DatumType::INT16_1 == t || DatumType::INT16_2 == t) {
        u.i = chunk.readSint16LE();
    } else if (DatumType::UINT32_1 == t || DatumType::UINT32_2 == t) {
        u.i = chunk.readUint32LE();
    } else if (DatumType::FLOAT64_1 == t || DatumType::FLOAT64_2 == t) {
        u.f = chunk.readDoubleLE();
    } else if (DatumType::STRING == t || DatumType::FILENAME == t) {
        // TODO: This copies the string. Can we read it directly from the chunk?
        int size = Datum(chunk, DatumType::UINT32_1).u.i;
        char* buffer = new char[size + 1];
        chunk.read(buffer, size);
        buffer[size] = '\0';
        u.string = new Common::String(buffer);
        delete[] buffer;
    } else if (DatumType::POINT_1 == t || DatumType::POINT_2 == t) {
        u.point = new Point(chunk);
    } else if (DatumType::BOUNDING_BOX == t) {
        u.bbox = new BoundingBox(chunk);
    } else if (DatumType::POLYGON == t) {
        u.polygon = new Polygon(chunk);
    } else if (DatumType::PALETTE == t) {
        u.palette = new unsigned char[0x300];
        chunk.read(u.palette, 0x300);
    } else if (DatumType::REFERENCE == t) {
        u.ref = new Reference(chunk);
    } else {
        error("Unknown datum type: 0x%x (@0x%lx)", t, chunk.pos());
    }
}

} // End of namespace MediaStation