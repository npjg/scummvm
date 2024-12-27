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

#ifndef MEDIASTATION_HOTSPOT_H
#define MEDIASTATION_HOTSPOT_H

#include "mediastation/assetheader.h"

namespace MediaStation {

class Hotspot : public Asset {
public:
    Hotspot(AssetHeader *header) : Asset(header) {};
    virtual ~Hotspot() override = default;

    virtual void play() override;
    virtual void stop() override;
    virtual void process() override;
};

} // End of namespace MediaStation

#endif