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

	Asset *openingMovie = _assets.getValOrDefault(105);
	if (activeScreen->_palette == nullptr) {
		error("No palette");
	}
	Asset *palette = _assets.getValOrDefault(104);
	_screen->setPalette(*palette->header->_palette);
	// _screen->setPalette(*activeScreen->_palette);

    Common::Event e;
    uint32 currentTime = g_system->getMillis();
	uint32 animationStart = currentTime;
    uint32 nextFrameTime = currentTime;
    Graphics::ManagedSurface *previousFrameSurface = nullptr;
    int previousFrameLeft = 0;
    int previousFrameTop = 0;

	Movie *currentMovie = openingMovie->a.movie;
	while (true) {
		currentTime = g_system->getMillis();
		while (g_system->getEventManager()->pollEvent(e)) {
			debugC(9, kDebugEvents, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
			debugC(9, kDebugEvents, "@@@@   Processing events");
			debugC(9, kDebugEvents, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

			if (e.type == Common::EVENT_QUIT || e.type == Common::EVENT_KEYDOWN || e.type == Common::EVENT_LBUTTONDOWN) {
				return Common::kNoError;
			}
		}

		// DRAW THE FRAMES.
		Common::Array<MovieFrame *> framesToDraw;
		for (MovieFrame *frame : currentMovie->_frames) {
			bool isAfterStart = animationStart + frame->_footer->_startInMilliseconds <= currentTime;
			bool isBeforeEnd = animationStart + frame->_footer->_endInMilliseconds >= currentTime;
			if (!isAfterStart || (isAfterStart && !isBeforeEnd)) {
				frame->_showing = false;
				continue;
			}
			//if (frame->_showing) {
			//	continue;
			//}

			// Blit the new frame
			// frame->_showing = true;
			debugC(7, kDebugGraphics, "(time: %d ms) Drawing frame %d (%d x %d) @ (%d, %d); start: %d ms, end: %d ms, keyframeEnd: %d ms, _unk5 = %d", currentTime - animationStart, frame->_footer->_index, frame->width(), frame->height(), frame->_footer->_left, frame->_footer->_top, frame->_footer->_startInMilliseconds, frame->_footer->_endInMilliseconds, frame->_keyframeEndInMilliseconds, frame->_footer->_zIndex);
			framesToDraw.push_back(frame);
		}

		// BLIT THE FRAMES.
		Common::sort(framesToDraw.begin(), framesToDraw.end(), [](MovieFrame *a, MovieFrame *b) {
			return a->_footer->_zIndex > b->_footer->_zIndex;
		});
		for (MovieFrame *frame : framesToDraw) {
			debugC(7, kDebugGraphics, "(time: %d ms) Drawing frame %d (%d x %d) @ (%d, %d); start: %d ms, end: %d ms, keyframeEnd: %d ms, _unk5 = %d", currentTime - animationStart, frame->_footer->_index, frame->width(), frame->height(), frame->_footer->_left, frame->_footer->_top, frame->_footer->_startInMilliseconds, frame->_footer->_endInMilliseconds, frame->_keyframeEndInMilliseconds, frame->_footer->_zIndex);
			_screen->transBlitFrom(frame->surface, Common::Point(frame->_footer->_left, frame->_footer->_top), 0, false);
		}
		framesToDraw.clear();

		_screen->update();
		g_system->delayMillis(50);
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
