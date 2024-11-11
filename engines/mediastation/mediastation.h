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

#ifndef MEDIASTATION_H
#define MEDIASTATION_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/util.h"
#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/screen.h"

#include "mediastation/detection.h"
#include "mediastation/datafile.h"
#include "mediastation/boot.h"
#include "mediastation/context.h"
#include "mediastation/assetheader.h"

#include "mediastation/assets/bitmap.h"
#include "mediastation/assets/sound.h"
#include "mediastation/assets/movie.h"
#include "mediastation/assets/sprite.h"

namespace MediaStation {

enum DebugChannels {
	kDebugGraphics = 1,
	kDebugPath,
	kDebugScan,
	kDebugScript,
	kDebugEvents,
	kDebugLoading
};

struct MediaStationGameDescription;

struct FunctionDeclaration {
	// ID
	// Human readable name
	// Number of params (-1 = 1+ PARAMS)
	// Function pointer
};

struct Asset {
	AssetHeader *header;

	Asset(AssetHeader *header) : header(header) {
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

    ~Asset() {
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

    union {
        Bitmap *bitmap;
        Sound *sound;
        Sprite *sprite;
        // Font *font;
        Movie *movie;
    } a;
};

class MediaStationEngine : public Engine {
private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	Boot *_boot;
	// map the list of contexts here.

	Context *loadContext(uint32 contextId);
	Common::HashMap<uint, FunctionDeclaration> _functionDeclarations;

protected:
	// Engine APIs
	Common::Error run() override;

public:
	Graphics::Screen *_screen = nullptr;

public:
	MediaStationEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~MediaStationEngine() override;

	uint32 getFeatures() const;

	Common::String getGameId() const;

	uint32 getRandomNumber(uint maxNum) {
		return _randomSource.getRandomNumber(maxNum);
	}

	bool hasFeature(EngineFeature f) const override {
		return
		    (f == kSupportsReturnToLauncher);
	};

	bool isFirstGenerationEngine() {
		if (_boot == nullptr) {
			error("Attempted to get engine version before BOOT.STM was read");
		} else { 
			return (_boot->_versionInfo == nullptr);
		}
	}

    Common::HashMap<uint, Function *> _functions;
    Common::HashMap<uint, Asset *> _assets;
    Common::HashMap<uint, Asset *> _assetsByChunkReference;
};

extern MediaStationEngine *g_engine;
#define SHOULD_QUIT ::MediaStation::g_engine->shouldQuit()

} // End of namespace MediaStation

#endif // MEDIASTATION_H
