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

#ifndef MEDIASTATION_DATUM_H
#define MEDIASTATION_DATUM_H

namespace MediaStation {

enum class DatumType {
    UINT8 = 0x0002,
    // TODO: Understand why there are different (u)int16 type codes.
    UINT16_1 = 0x0003,
    UINT16_2 = 0x0013,
    INT16_1 = 0x0006,
    INT16_2 = 0x0010,
    // TODO: Understand why there are two different uint32 type codes.
    UINT32_1 = 0x0004,
    UINT32_2 = 0x0007,
    // TODO: Understand why there are two different float64 type codes.
    FLOAT64_1 = 0x0011,
    FLOAT64_2 = 0x0009,
    STRING = 0x0012,
    FILENAME = 0x000a,
    POINT_1 = 0x000f,
    POINT_2 = 0x000e,
    BOUNDING_BOX = 0x000d,
    POLYGON = 0x001d,
    // These are other types.
    PALETTE = 0x05aa,
    REFERENCE = 0x001b
};

class Point;
class BoundingBox;
class Polygon;
class Reference;

class Datum {
public:
    DatumType t;
    union {
        int i;
        double f;
        Common::String *string;
        Polygon *polygon;
        Point *point;
        BoundingBox *bbox;
        unsigned char *palette;
        Reference *ref;
    } u;

    Datum();
    Datum(Chunk& chunk);
    Datum(Chunk& chunk, DatumType expectedType);
    ~Datum();

private:
    void readWithType(Chunk& chunk);
};

class Point {
public:
    int x;
    int y;

    Point();
    Point(Chunk& chunk);
};

class BoundingBox {
public:
    Datum left_top_point;
    Datum dimensions;

    BoundingBox();
    BoundingBox(Chunk& chunk);
};

class Polygon {
public:
    Common::Array<Datum> points;

    Polygon();
    Polygon(Chunk& chunk);
};

class Reference {
public:
    uint32 chunk_id;

    Reference();
    Reference(Chunk& chunk);
};

} // End of namespace MediaStation

#endif
