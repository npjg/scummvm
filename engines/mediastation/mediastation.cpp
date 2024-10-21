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
#include "mediastation/detection.h"
#include "mediastation/boot.h"
#include "mediastation/context.h"

namespace MediaStation {

MediaStationEngine *g_engine;

MediaStationEngine::MediaStationEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), 
	_randomSource("MediaStation"),
	_boot(nullptr) {
	g_engine = this;
}

MediaStationEngine::~MediaStationEngine() {
	delete _screen;
}

uint32 MediaStationEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String MediaStationEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error MediaStationEngine::run() {
	// LOAD BOOT.STM.
	Common::Path bootStmFilepath = Common::Path("BOOT.STM");
	_boot = new Boot(bootStmFilepath);

	// INITIALIZE GRAPHICS.
	// All Media Station games run at 640x480.
	initGraphics(640, 480);
	_screen = new Graphics::Screen();

	Context *activeScreen = loadContext(_boot->_entryContextId);

	// Draw a series of boxes on screen as a sample
	for (int i = 0; i < 100; ++i)
		_screen->frameRect(Common::Rect(i, i, 640 - i, 480 - i), i);
	_screen->update();

	// Simple event handling loop
	byte pal[256 * 3] = { 0 };
	Common::Event e;
	int offset = 0;

	Graphics::FrameLimiter limiter(g_system, 60);
	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
			debugC(9, kDebugEvents, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
			debugC(9, kDebugEvents, "@@@@   Processing events");
			debugC(9, kDebugEvents, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
		}

		// Cycle through a simple palette
		++offset;
		for (int i = 0; i < 256; ++i)
			pal[i * 3 + 1] = (i + offset) % 256;
		g_system->getPaletteManager()->setPalette(pal, 0, 256);
		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		limiter.delayBeforeSwap();
		// _screen->update();
		limiter.startFrame();
	}

	return Common::kNoError;
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
	Context *context= new Context(entryCxtFilepath);
	return context;
}

} // End of namespace MediaStation
