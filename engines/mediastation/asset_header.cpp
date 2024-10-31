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

#include "mediastation/datum.h"
#include "mediastation/asset_header.h"

namespace MediaStation {

AssetHeader::AssetHeader(Chunk &chunk) {
    _fileNumber = Datum(chunk).u.i;
    // TODO: Cast to an asset type.
    _type = AssetType(Datum(chunk).u.i);
    _id = Datum(chunk).u.i;
    debugC(4, kDebugLoading, "AssetHeader::AssetHeader(): _type = 0x%x, _id = 0x%x (@0x%lx)", _type, _id, chunk.pos());

    AssetHeader::SectionType sectionType = getSectionType(chunk);
    bool moreSectionsToRead = (AssetHeader::SectionType::EMPTY != sectionType);
    while (moreSectionsToRead) {
        debugC(5, kDebugLoading, "AssetHeader::AssetHeader(): sectionType = 0x%x (@0x%lx)", sectionType, chunk.pos());
        readSection(sectionType, chunk);
        sectionType = getSectionType(chunk);
        moreSectionsToRead = (AssetHeader::SectionType::EMPTY != sectionType);
    }
}

void AssetHeader::readSection(AssetHeader::SectionType sectionType, Chunk& chunk) {
    switch (sectionType) {
        case AssetHeader::SectionType::EMPTY: {
            break;
        }

        case AssetHeader::SectionType::EVENT_HANDLER: {
            error("Event handler Not implemented");
            break;
        }

        case AssetHeader::SectionType::ASSET_ID: {
            // We already have this asset's ID, so we will just verify it is the same
            // as the ID we have already read.
            uint32 duplicateAssetId = Datum(chunk).u.i;
            if (duplicateAssetId != _id) {
                warning("AssetHeader::readSection(): Asset ID %d does not match original asset ID %d", duplicateAssetId, _id);
            }
            break;
        }

        case AssetHeader::SectionType::CHUNK_REFERENCE: {
            // These are references to the chunk(s) that hold the data for this asset.
            // The references and the chunks have the following format "a501".
            // There is no guarantee where these chunk(s) might actually be located:
            //  - They might be in the same RIFF subfile as this header,
            //  - They might be in a different RIFF subfile in the same CXT file,
            //  - They might be in a different CXT file entirely.
            if (_type == AssetType::MOVIE) {
                _chunkReference.m = MovieChunkReference();
                _chunkReference.m.headerChunkId = Datum(chunk).u.i;
            } else {
                // TODO: Need to get the chunk reference out of here. It is its
                // own datum type.
                _chunkReference.c = Datum(chunk).u.i;
            }
            break;
        }

        case AssetHeader::SectionType::MOVIE_AUDIO_CHUNK_REFERENCE: {
            _chunkReference.m.audioChunkId = Datum(chunk).u.i;
            break;
        }

        case AssetHeader::SectionType::MOVIE_VIDEO_CHUNK_REFERENCE: {
            _chunkReference.m.videoChunkId = Datum(chunk).u.i;
            break;
        }

        case AssetHeader::SectionType::BOUNDING_BOX: {
            _boundingBox = Datum(chunk, DatumType::BOUNDING_BOX).u.bbox;
            break;
        }

        case AssetHeader::SectionType::MOUSE_ACTIVE_AREA: {
            _mouseActiveArea = Datum(chunk, DatumType::POLYGON).u.polygon;
            break;
        }

        case AssetHeader::SectionType::Z_INDEX: {
            _zIndex = Datum(chunk).u.i;
            break;
        }

        case AssetHeader::SectionType::ASSET_REFERENCE: {
            _assetReference = Datum(chunk).u.i;
            break;
        }

        case AssetHeader::SectionType::STARTUP: {
            _startup = Datum(chunk).u.i;
            break;
        }

        case AssetHeader::SectionType::TRANSPARENCY: {
            _transparency = Datum(chunk).u.i;
            break;
        }

        case AssetHeader::SectionType::HAS_OWN_SUBFILE: {
            _hasOwnSubfile = Datum(chunk).u.i;
            break;
        }

        case AssetHeader::SectionType::CURSOR_RESOURCE_ID: {
            _cursorResourceId = Datum(chunk).u.i;
            break;
        }

        case AssetHeader::SectionType::FRAME_RATE: {
            _frameRate = Datum(chunk).u.i;
            break;
        }

        case AssetHeader::SectionType::LOAD_TYPE: {
            _loadType = Datum(chunk).u.i;
            break;
        }

        case AssetHeader::SectionType::SOUND_INFO: {
            _totalChunks = Datum(chunk).u.i;
            _rate = Datum(chunk).u.i;
            break;
        }

        case AssetHeader::SectionType::MOVIE_LOAD_TYPE: {
            _loadType = Datum(chunk).u.i;
            break;
        }

        default: {
            error("AssetHeader::readSection(): Unknown section type 0x%x (@0x%lx)", sectionType, chunk.pos());
            break;
        }
    }
}

AssetHeader::SectionType AssetHeader::getSectionType(Chunk &chunk) {
    Datum datum = Datum(chunk, DatumType::UINT16_1);
    AssetHeader::SectionType sectionType = static_cast<AssetHeader::SectionType>(datum.u.i);
    return sectionType;
}

} // end of namespace MediaStation