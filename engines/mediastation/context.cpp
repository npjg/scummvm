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
#include "mediastation/context.h"
#include "mediastation/datum.h"
#include "mediastation/debugchannels.h"

#include "mediastation/bitmap.h"
#include "mediastation/assets/canvas.h"
#include "mediastation/assets/palette.h"
#include "mediastation/assets/image.h"
#include "mediastation/assets/path.h"
#include "mediastation/assets/sound.h"
#include "mediastation/assets/movie.h"
#include "mediastation/assets/sprite.h"
#include "mediastation/assets/hotspot.h"
#include "mediastation/assets/timer.h"

namespace MediaStation {

Context::Context(const Common::Path &path) :
	Datafile(path) {
	// This stuff isn't part of any graphics palette.
	readPreamble();

	// READ THE FIRST SUBFILE.
	Subfile subfile = Subfile(_stream);
	Chunk chunk = subfile.nextChunk();
	// First, read the header sections.
	if (g_engine->isFirstGenerationEngine()) {
		readOldStyleHeaderSections(subfile, chunk);
	} else {
		readNewStyleHeaderSections(subfile, chunk);
	}
	// Then, read any asset data.
	chunk = subfile.currentChunk;
	while (!subfile.atEnd()) {
		readAssetInFirstSubfile(chunk);
		if (!subfile.atEnd()) {
			chunk = subfile.nextChunk();
		}
	}

	// Then, assets in the rest of the subfiles.
	for (uint i = 1; i < subfile_count; i++) {
		subfile = Subfile(_stream);
		readAssetFromLaterSubfile(subfile);
	}
}

Context::~Context() {
	delete _palette;
	_palette = nullptr;
	delete _parameters;
	_parameters = nullptr;
}

bool Context::readPreamble() {
	uint16 signature = _stream->readUint16LE();
	if (signature != 0x4949) { // "II"
		warning("Datafile::openFile(): Wrong signature for file %s. Got 0x%04X", _path.toString(Common::Path::kNativeSeparator).c_str(), signature);
		close();
		return false;
	}
	_stream->skip(2); // 0x00 0x00

	unk1 = _stream->readUint32LE();
	debugC(5, kDebugLoading, "Context::openFile(): unk1 = 0x%x", unk1);

	subfile_count = _stream->readUint32LE();
	// The total size of this file, including this header.
	// (Basically the true file size shown on the filesystem.)
	file_size = _stream->readUint32LE();
	return true;
}

void Context::readOldStyleHeaderSections(Subfile &subfile, Chunk &chunk) {
	error("Context::readOldStyleHeaderSections(): Not implemented yet");
}

void Context::readNewStyleHeaderSections(Subfile &subfile, Chunk &chunk) {
	// READ THE PALETTE.
	bool moreSectionsToRead = (chunk.id == MKTAG('i', 'g', 'o', 'd'));
	if (!moreSectionsToRead) {
		warning("Context::readNewStyleHeaderSections(): Got no header sections (@0x%llx)", chunk.pos());
	}

	while (moreSectionsToRead) {
		// VERIFY THIS CHUNK IS A HEADER.
		// TODO: What are the situations when it's not?
		uint16 sectionType = Datum(chunk, DatumType::UINT16_1).u.i;
		debugC(5, kDebugLoading, "Context::readNewStyleHeaderSections(): sectionType = 0x%x (@0x%llx)", sectionType, chunk.pos());
		bool chunkIsHeader = (sectionType == 0x000d);
		if (!chunkIsHeader) {
			error("Context::readNewStyleHeaderSections(): Expected header chunk, got %s (@0x%llx)", tag2str(chunk.id), chunk.pos());
		}

		// READ THIS HEADER SECTION.
		moreSectionsToRead = readHeaderSection(subfile, chunk);
		if (subfile.atEnd()) {
			break;
		} else {
			debugC(5, kDebugLoading, "\nContext::readNewStyleHeaderSections(): Getting next chunk (@0x%llx)", chunk.pos());
			chunk = subfile.nextChunk();
			moreSectionsToRead = (chunk.id == MKTAG('i', 'g', 'o', 'd'));
		}
	}
	debugC(5, kDebugLoading, "Context::readNewStyleHeaderSections(): Finished reading sections (@0x%llx)", chunk.pos());
}

void Context::readAssetInFirstSubfile(Chunk &chunk) {
	if (chunk.id == MKTAG('i', 'g', 'o', 'd')) {
		warning("Context::readAssetInFirstSubfile(): Skippping \"igod\" asset link chunk");
		chunk.skip(chunk.bytesRemaining());
		return;
	}

	// TODO: Make sure this is not an asset link.
	Asset *asset = g_engine->_assetsByChunkReference.getValOrDefault(chunk.id);
	if (asset == nullptr) {
		error("Context::readAssetInFirstSubfile(): Asset for chunk \"%s\" (0x%x) does not exist or has not been read yet in this title. (@0x%llx)", tag2str(chunk.id), chunk.id, chunk.pos());
	}
	debugC(5, kDebugLoading, "\nContext::readAssetInFirstSubfile(): Got asset with chunk ID %s in first subfile (type: 0x%x) (@0x%llx)", tag2str(chunk.id), asset->type(), chunk.pos());
	asset->readChunk(chunk);
}

void Context::readAssetFromLaterSubfile(Subfile &subfile) {
	Chunk chunk = subfile.nextChunk();
	Asset *asset = g_engine->_assetsByChunkReference.getValOrDefault(chunk.id);
	if (asset == nullptr) {
		error("Context::readAssetFromLaterSubfile(): Asset for chunk \"%s\" (0x%x) does not exist or has not been read yet in this title. (@0x%llx)", tag2str(chunk.id), chunk.id, chunk.pos());
	}
	debugC(5, kDebugLoading, "\nContext::readAssetFromLaterSubfile(): Got asset with chunk ID %s in later subfile (type: 0x%x) (@0x%llx)", tag2str(chunk.id), asset->type(), chunk.pos());
	asset->readSubfile(subfile, chunk);
}

bool Context::readHeaderSection(Subfile &subfile, Chunk &chunk) {
	uint16 sectionType = Datum(chunk, DatumType::UINT16_1).u.i;
	debugC(5, kDebugLoading, "Context::readHeaderSection(): sectionType = 0x%x (@0x%llx)", sectionType, chunk.pos());
	switch ((SectionType)sectionType) {
	case SectionType::PARAMETERS: {
		if (_parameters != nullptr) {
			error("Context::readHeaderSection(): Got multiple parameters (@0x%llx)", chunk.pos());
		}
		_parameters = new ContextParameters(chunk);
		break;
	}

	case SectionType::ASSET_LINK: {
		warning("Context::readHeaderSection(): ASSET_LINK not implemented yet");
		break;
	}

	case SectionType::PALETTE: {
		if (_palette != nullptr) {
			error("Context::readHeaderSection(): Got multiple palettes (@0x%llx)", chunk.pos());
		}
		// TODO: Avoid the copying here!
		const uint PALETTE_ENTRIES = 256;
		const uint PALETTE_BYTES = PALETTE_ENTRIES * 3;
		byte* buffer = new byte[PALETTE_BYTES];
		chunk.read(buffer, PALETTE_BYTES);
		_palette = new Graphics::Palette(buffer, PALETTE_ENTRIES);
		delete[] buffer;
		debugC(5, kDebugLoading, "Context::readHeaderSection(): Read palette");
		// This is likely just an ending flag that we expect to be zero.
		Datum(chunk, DatumType::UINT16_1).u.i;
		break;
	}

	case SectionType::ASSET_HEADER: {
		Asset *asset = nullptr;
		AssetHeader *header = new AssetHeader(chunk);
		switch (header->_type) {
		case AssetType::IMAGE:
			asset = new Image(header);
			break;

		case AssetType::MOVIE:
			asset = new Movie(header);
			break;

		case AssetType::SOUND:
			asset = new Sound(header);
			break;

		case AssetType::PALETTE:
			asset = new Palette(header);
			break;

		case AssetType::PATH:
			asset = new Path(header);
			break;

		case AssetType::TIMER:
			asset = new Timer(header);
			break;

		case AssetType::HOTSPOT:
			asset = new Hotspot(header);
			break;

		case AssetType::SPRITE:
			asset = new Sprite(header);
			break;

		case AssetType::CANVAS:
			asset = new Canvas(header);
			break;

		case AssetType::SCREEN:
			if (_screenAsset != nullptr) {
				error("Context::readHeaderSection(): Got multiple screen assets in the same context");
			}
			_screenAsset = header;
			break;

		default:
			error("Context::readHeaderSection(): No class for asset type 0x%x (@0x%llx)", header->_type, chunk.pos());
		}

		if (g_engine->_assets.contains(header->_id)) {
			error("Context::readHeaderSection(): Asset with ID 0x%d was already defined in this title", header->_id);
		}
		g_engine->_assets.setVal(header->_id, asset);
		if (header->_chunkReference != 0) {
			debugC(5, kDebugLoading, "Context::readHeaderSection(): Storing asset with chunk ID \"%s\" (0x%x)", tag2str(header->_chunkReference), header->_chunkReference);
			g_engine->_assetsByChunkReference.setVal(header->_chunkReference, asset);
		}
		// TODO: Store the movie chunk references better.
		if (header->_audioChunkReference != 0) {
			g_engine->_assetsByChunkReference.setVal(header->_audioChunkReference, asset);
		}
		if (header->_animationChunkReference != 0) {
			g_engine->_assetsByChunkReference.setVal(header->_animationChunkReference, asset);
		}
		// TODO: This datum only appears sometimes.
		Datum(chunk).u.i;
		break;
	}

	case SectionType::FUNCTION: {
		Function *function = new Function(chunk);
		g_engine->_functions.setVal(function->_id, function);
		if (!g_engine->isFirstGenerationEngine()) {
			Datum(chunk).u.i; // Should be zero.
		}
		break;
	}

	case SectionType::END: {
		error("Context::readHeaderSection(): END Not implemented yet");
		return false;
	}

	case SectionType::EMPTY: {
		error("Context::readHeaderSection(): EMPTY Not implemented yet");
		break;
	}

	case SectionType::POOH: {
		error("Context::readHeaderSection(): POOH Not implemented yet");
		break;
	}

	default: {
		error("Context::readHeaderSection(): Unknown section type 0x%x (@0x%llx)", sectionType, chunk.pos());
	}
	}

	return true;
}

void Context::play() {
	// FIND AND EXECUTE THE ENTRY SCRIPT.
	// The entry script is stored in the asset with the same ID as the context.
	// It's the asset that has a SCREEN asset type.
	if (_screenAsset == nullptr) {
		error("Context::play(): No entry script exists for this context, cannot play it");
	}
	EventHandler *entryHandler = nullptr; //_screenAsset->_eventHandlers.getVal(uint32(EventHandler::Type::Entry));
	// So how can we actually execute this script?

	// FIND AND EXECUTE THE EXIT SCRIPT.
}

} // End of namespace MediaStation
