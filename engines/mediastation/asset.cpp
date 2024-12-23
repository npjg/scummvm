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
#include "mediastation/assets/timer.h"
#include "mediastation/assets/movie.h"
#include "mediastation/assets/sprite.h"
#include "mediastation/assets/path.h"

namespace MediaStation {

Asset::Asset(AssetHeader *header) : header(header) {
    switch (header->_type) {
        case AssetType::IMAGE:
            a.bitmap = nullptr;
            break;

        case AssetType::SOUND:
            a.sound = new Sound(header);
            break;

        case AssetType::TIMER:
            a.timer = new Timer(header);
            break;

        case AssetType::MOVIE:
            a.movie = new Movie(header);
            break;

        case AssetType::SPRITE:
            a.sprite = new Sprite(header);
            break;

        case AssetType::PATH:
            a.path = new Path(header);
            break;

        default:
            error("Asset::Asset(): Unexpected asset type %d", header->_type);
            break;
    }
}

bool Asset::isPlaying() {
    switch (header->_type) {
        case AssetType::SOUND:
            //return a.sound->isPlaying();
            return false;

        case AssetType::MOVIE:
            return a.movie->isPlaying();

        case AssetType::TIMER:
            return a.timer->isPlaying();

        case AssetType::SPRITE:
            return false;

        case AssetType::PATH:
            return false;
    }
}

void Asset::play() {
    switch (header->_type) {
        case AssetType::SOUND:
            break;

        case AssetType::MOVIE:
            a.movie->play();
            break;

        case AssetType::TIMER:
            a.timer->play();
            break;

        case AssetType::SPRITE:
            break;

        case AssetType::PATH:
            break;
    }
}

void Asset::process() {
    switch (header->_type) {
        case AssetType::SOUND: {
            // a.sound->process();
            break;
        }

        case AssetType::MOVIE: {
            a.movie->process(); // TODO: This now returns a bool that says when itʻs finished playing or not.
            break;
        }

        case AssetType::TIMER: {
            a.timer->process();
        }

        case AssetType::SPRITE: {
            // a.sprite->process();
            break;
        }

        case AssetType::PATH: {
            // a.path->process();
            break;
        }

        default: 
            break;
    }
}

Asset::~Asset() {
    switch (header->_type) {
        case AssetType::MOVIE:
            delete a.movie;
            a.movie = nullptr;
            break;

        case AssetType::SOUND:
            delete a.sound;
            a.sound = nullptr;
            break;

        case AssetType::IMAGE:
            delete a.bitmap;
            a.bitmap = nullptr;
            break;

        case AssetType::SPRITE:
            delete a.sprite;
            a.sprite = nullptr;
            break;

        case AssetType::PATH:
            delete a.path;
            a.path = nullptr;
            break;

        default:
            // Handle unexpected asset type if necessary
            break;
    }
    delete header;
    header = nullptr;
}

} // End of namespace MediaStation
