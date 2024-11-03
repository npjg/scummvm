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

namespace MediaStation {

Context::Context(const Common::Path &path) : Datafile(path), _palette(nullptr), _parameters(nullptr) {
    // This stuff isn't part of any graphics palette.
    readPreamble();

    // READ THE FIRST SUBFILE.
    Subfile subfile = Subfile(_stream);
    Chunk chunk = subfile.nextChunk();

    if (g_engine->isFirstGenerationEngine()) {
        readOldStyleHeaderSections(subfile, chunk);
    } else {
        readNewStyleHeaderSections(subfile, chunk);
    }
}

Context::~Context() {
    delete _palette;
    delete _parameters;
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
        warning("Context::readNewStyleHeaderSections(): Got no header sections (@0x%lx)", chunk.pos());
    }

    while (moreSectionsToRead) {
        // VERIFY THIS CHUNK IS A HEADER.
        // TODO: What are the situations when it's not?
        uint16 sectionType = Datum(chunk, DatumType::UINT16_1).u.i;
        debugC(5, kDebugLoading, "Context::readNewStyleHeaderSections(): sectionType = 0x%x (@0x%lx)", sectionType, chunk.pos());
        bool chunkIsHeader = (sectionType == 0x000d);
        if (!chunkIsHeader) {
            error("Context::readNewStyleHeaderSections(): Expected header chunk, got %s (@0x%lx)", tag2str(chunk.id), chunk.pos());
        }

        // READ THIS HEADER SECTION.
        moreSectionsToRead = readHeaderSection(subfile, chunk);
        if (subfile.atEnd()) {
            break;
        } else {
            debugC(5, kDebugLoading, "\nContext::readNewStyleHeaderSections(): Getting next chunk (@0x%lx)", chunk.pos());
            chunk = subfile.nextChunk();
            moreSectionsToRead = (chunk.id == MKTAG('i', 'g', 'o', 'd'));
        }
    }
    debugC(5, kDebugLoading, "Context::readNewStyleHeaderSections(): Finished reading sections (@0x%lx)", chunk.pos());
}

bool Context::readHeaderSection(Subfile &subfile, Chunk &chunk) {
    uint16 sectionType = Datum(chunk, DatumType::UINT16_1).u.i;
    debugC(5, kDebugLoading, "Context::readHeaderSection(): sectionType = 0x%x (@0x%lx)", sectionType, chunk.pos());
    switch ((SectionType)sectionType) {
        case SectionType::PARAMETERS: {
            if (_parameters != nullptr) {
                error("Context::readHeaderSection(): Got multiple parameters (@0x%lx)", chunk.pos());
            }
            _parameters = new ContextParameters(chunk);
        }

        case SectionType::ASSET_LINK: {
            break;
        }

        case SectionType::PALETTE: {
            if (_palette != nullptr) {
                error("Context::readHeaderSection(): Got multiple palettes (@0x%lx)", chunk.pos());
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
            AssetHeader *header = new AssetHeader(chunk);
            _assetHeaders.setVal(header->_id, header);
            // TODO: This only appears sometimes.
            Datum(chunk).u.i;
            break;
        }

        case SectionType::FUNCTION: {
            error("Context::readHeaderSection(): FUNCTION Not implemented yet");
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
            error("Context::readHeaderSection(): Unknown section type 0x%x (@0x%lx)", sectionType, chunk.pos());
        }
    }

    return true;
}

} // End of namespace MediaStation
