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

#include "graphics/framelimiter.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"	
#include "common/events.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/paletteman.h"

#include "mediastation/mediastation.h"
#include "mediastation/debugchannels.h"
#include "mediastation/detection.h"
#include "mediastation/boot.h"
#include "mediastation/context.h"
#include "mediastation/asset.h"
#include "mediastation/assets/movie.h"

namespace MediaStation {

MediaStationEngine *g_engine;

MediaStationEngine::MediaStationEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), 
	_randomSource("MediaStation"),
	_boot(nullptr) {
	g_engine = this;
	_mixer = g_system->getMixer();
}

MediaStationEngine::~MediaStationEngine() {
	_mixer = nullptr;

	delete _screen;
	_screen = nullptr;

	delete _boot;
	_boot = nullptr;

    for (auto it = _assets.begin(); it != _assets.end(); ++it) {
        delete it->_value;
    }
    _assets.clear();
	_assetsByChunkReference.clear();

	for (auto it = _functions.begin(); it != _functions.end(); ++it) {
		delete it->_value;
	}
	_functions.clear();

	for (auto it = _variables.begin(); it != _variables.end(); ++it) {
		delete it->_value;
	}
	_variables.clear();
}

uint32 MediaStationEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String MediaStationEngine::getGameId() const {
	return _gameDescription->gameId;
}

bool MediaStationEngine::isFirstGenerationEngine() {
	if (_boot == nullptr) {
		error("Attempted to get engine version before BOOT.STM was read");
	} else { 
		return (_boot->_versionInfo == nullptr);
	}
}

Common::Error MediaStationEngine::run() {
	// INITIALIZE SUBSYSTEMS.
	// All Media Station games run at 640x480.
	initGraphics(640, 480);
	_screen = new Graphics::Screen();
	_screen->fillRect(Common::Rect(640, 480), 255);

	// LOAD BOOT.STM.
	Common::Path bootStmFilepath = Common::Path("BOOT.STM");
	_boot = new Boot(bootStmFilepath);

	// LOAD THE ROOT CONTEXT.
	// This is because we might have assets that always need to be loaded.
	Context *root = nullptr; 
	uint32 rootContextId = _boot->getRootContextId();
	if (rootContextId != 0) {
		root = loadContext(rootContextId);
	} else {
		warning("MediaStation::run(): Title has no root context");
	}

	Context *activeScreen = loadContext(_boot->_entryContextId);
	if (activeScreen->_screenAsset != nullptr) {
		// GET THE PALETTE.
		setPaletteFromHeader(activeScreen->_screenAsset);
		
		// PROCESS THE OPENING EVENT HANDLER.
		EventHandler *entryEvent = activeScreen->_screenAsset->_eventHandlers.getValOrDefault(EventHandler::Type::Entry);
		if (entryEvent != nullptr) {
			debugC(5, kDebugScript, "Executing context entry event handler");
			entryEvent->execute(activeScreen->_screenAsset->_id);
		} else {
			debugC(5, kDebugScript, "No context entry event handler");
		}
	}

    uint32 currentTime = g_system->getMillis();
	while (true) {
		// PROCESS EVENTS.
		Common::ErrorCode status = processEvents();
		if (status == Common::kNoError) {
			return status;
		}

		// PROCESS ANY ASSETS CURRENTLY PLAYING.
		// First, they all need to be sorted by z-coordinate.
		Common::sort(_assetsPlaying.begin(), _assetsPlaying.end(), [](Asset *a, Asset *b) {
			return a->zIndex() > b->zIndex();
		});
		//warning("START RENDER CYCLE");
		for (auto it = _assetsPlaying.begin(); it != _assetsPlaying.end(); ) {
			(*it)->process();
			if (!(*it)->isPlaying()) {
				it = _assetsPlaying.erase(it);
			} else {
				++it;
			}
		}
		//warning("END RENDER CYCLE");

    	g_engine->_screen->update();
		g_system->delayMillis(10);
	}

	return Common::kNoError;
}

Common::ErrorCode MediaStationEngine::processEvents() {
	while (g_system->getEventManager()->pollEvent(e)) {
		debugC(9, kDebugEvents, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
		debugC(9, kDebugEvents, "@@@@   Processing events");
		debugC(9, kDebugEvents, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

		switch (e.type) {
			case Common::EVENT_QUIT: {
				error("Quitting");
			}

			case Common::EVENT_KEYDOWN: {
				error("Quitting");
			}

			case Common::EVENT_LBUTTONDOWN: {
				error("Quitting");
			}

			default: {
				break;
			}
		}
	}
	return Common::kUserCanceled; //Common::kNoError;
}

Context *MediaStationEngine::loadContext(uint32 contextId) {
	if (_boot == nullptr) {
		error("Cannot load contexts before BOOT.STM is read");
	}

    // GET THE FILE ID.
    SubfileDeclaration *subfileDeclaration = _boot->_subfileDeclarations.getValOrDefault(contextId);
    if (subfileDeclaration == nullptr) {
		warning("MediaStationEngine::loadContext(): Couldn't find subfile declaration with ID 0x%x", contextId);
        return nullptr;
    }
	// The subfile declarations have other assets too, so we need to make sure 
	if (subfileDeclaration->_startOffsetInFile != 16) {
		warning("MediaStationEngine::loadContext(): Requested ID wasn't for a context.");
		return nullptr;
	}
    uint32 fileId = subfileDeclaration->_fileId;

    // GET THE FILENAME.
    FileDeclaration *fileDeclaration = _boot->_fileDeclarations.getValOrDefault(fileId);
    if (fileDeclaration == nullptr) {
		warning("MediaStationEngine::loadContext(): Couldn't find file declaration with ID 0x%x", fileId);
        return nullptr;
    }
    Common::String *fileName = fileDeclaration->_name;

	// LOAD THE CONTEXT.
	Common::Path entryCxtFilepath = Common::Path(*fileName);
	Context *context = new Context(entryCxtFilepath);

	// SET THE VARIABLES.
	return context;
}

void MediaStationEngine::setPalette(Asset *palette) {
	assert(palette != nullptr);
	setPaletteFromHeader(palette->getHeader());
}

void MediaStationEngine::setPaletteFromHeader(AssetHeader *header) {
	assert(header != nullptr);
	if (header->_palette != nullptr) {
		_screen->setPalette(*header->_palette);
	} else {
		warning("MediaStationEngine::setPaletteFromHeader(): Asset %d does not have a palette. Current palette will be unchanged.", header->_id);
	}
}


void MediaStationEngine::addPlayingAsset(Asset *assetToAdd) {
	// If we're already marking the asset as played, we don't need to mark it
	// played again.
	for (Asset *asset : g_engine->_assetsPlaying) {
		if (asset == assetToAdd) {
			return;
		}
	}
	g_engine->_assetsPlaying.push_back(assetToAdd);
}

} // End of namespace MediaStation
