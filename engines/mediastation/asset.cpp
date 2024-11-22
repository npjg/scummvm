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

#include "mediastation/asset.h"

#include "mediastation/assetheader.h"
#include "mediastation/assets/bitmap.h"
#include "mediastation/assets/sound.h"
#include "mediastation/assets/movie.h"
#include "mediastation/assets/sprite.h"

namespace MediaStation {

Asset::Asset(AssetHeader *header) : header(header) {
    if (AssetType::IMAGE == header->_type) {
        a.bitmap = nullptr;
    } else if (AssetType::SOUND == header->_type) {
        a.sound = new Sound(header);
    } else if (AssetType::MOVIE == header->_type) {
        a.movie = new Movie(header);
    } else if (AssetType::SPRITE == header->_type) {
        a.sprite = new Sprite(header);
    }
}

Asset::~Asset() {
    if (header->_type == AssetType::MOVIE) {
        delete a.movie;
    } else if (header->_type == AssetType::SOUND) {
        delete a.sound;
    } else if (header->_type == AssetType::IMAGE) {
        delete a.bitmap;
    } else if (header->_type == AssetType::SPRITE) {
        delete a.sprite;
    }
    delete header;
    header = nullptr;
}

} // End of namespace MediaStation
