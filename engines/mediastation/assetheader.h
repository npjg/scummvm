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

#include "graphics/palette.h"

#include "mediastation/chunk.h"
#include "mediastation/datum.h"

#include "mediastation/mediascript/eventhandler.h"

#ifndef MEDIASTATION_ASSET_HEADER_H
#define MEDIASTATION_ASSET_HEADER_H

namespace MediaStation {

struct MovieChunkReference {
    uint32 headerChunkId;
    uint32 audioChunkId;
    uint32 videoChunkId;
};

typedef uint32 ChunkReference;

enum class AssetType {
        SCREEN  = 0x0001, // SCR
        STAGE  = 0x0002, // STG
        PATH  = 0x0004, // PTH
        SOUND  = 0x0005, // SND
        TIMER  = 0x0006, // TMR
        IMAGE  = 0x0007, // IMG
        HOTSPOT  = 0x000b, // HSP
        SPRITE  = 0x000e, // SPR
        LK_ZAZU = 0x000f,
        LK_CONSTELLATIONS = 0x0010,
        IMAGE_SET = 0x001d,
        CURSOR  = 0x000c, // CSR
        PRINTER  = 0x0019, // PRT
        MOVIE  = 0x0016, // MOV
        PALETTE  = 0x0017,
        TEXT  = 0x001a, // TXT
        FONT  = 0x001b, // FON
        CAMERA  = 0x001c, // CAM
        CANVAS  = 0x001e, // CVS
        // TODO: Discover how the XSND differs from regular sounds.
        // Only appears in Ariel.
        XSND = 0x001f,
        XSND_MIDI = 0x0020,
        // TODO: Figure out what this is. Only appears in Ariel.
        RECORDER = 0x0021,
        FUNCTION  = 0x0069 // FUN
};

typedef uint32 AssetId;

class AssetHeader {
public:
    enum class SectionType {
        EMPTY = 0x0000,
        SOUND_ENCODING_1 = 0x0001,
        SOUND_ENCODING_2 = 0x0002,
        EVENT_HANDLER = 0x0017,
        STAGE_ID = 0x0019,
        ASSET_ID = 0x001a,
        CHUNK_REFERENCE = 0x001b,
        MOVIE_ANIMATION_CHUNK_REFERENCE = 0x06a4,
        MOVIE_AUDIO_CHUNK_REFERENCE = 0x06a5,
        ASSET_REFERENCE = 0x077b,
        BOUNDING_BOX = 0x001c,
        MOUSE_ACTIVE_AREA = 0x001d,
        Z_INDEX = 0x001e,
        STARTUP = 0x001f,
        TRANSPARENCY = 0x0020,
        HAS_OWN_SUBFILE = 0x0021,
        CURSOR_RESOURCE_ID = 0x0022,
        FRAME_RATE = 0x0024,
        LOAD_TYPE = 0x0032,
        SOUND_INFO = 0x0033,
        MOVIE_LOAD_TYPE = 0x0037,
        SPRITE_CHUNK_COUNT = 0x03e8,

        PALETTE = 0x05aa,
        DISSOLVE_FACTOR = 0x05dc,
        GET_OFFSTAGE_EVENTS = 0x05dd,
        X = 0x05de,
        Y = 0x05df,

        // PATH FIELDS.
        START_POINT = 0x060e,
        END_POINT = 0x060f,
        PATH_UNK1 = 0x0610,
        STEP_RATE = 0x0611,
        DURATION = 0x0612,

        // CAMERA FIELDS.
        VIEWPORT_ORIGIN = 0x076f,
        LENS_OPEN = 0x770,

        // STAGE FIELDS.
        STAGE_UNK1 = 0x771,
        CYLINDRICAL_X = 0x772,
        CYLINDRICAL_Y = 0x773,

        ASSET_NAME = 0x0bb8,
    };

    enum class SoundEncoding {
        PCM_S16LE_MONO_22050 = 0x0010, // Uncompressed linear PCM
        IMA_ADPCM_S16LE_MONO_22050 = 0x0004 // IMA ADPCM encoding, must be decoded
    };

    AssetHeader(Chunk &chunk);
    ~AssetHeader();

    uint32 _fileNumber;
    AssetType _type;
    AssetId _id;

    ChunkReference _chunkReference;
    // These two are only used in movies.
    ChunkReference _audioChunkReference;
    ChunkReference _animationChunkReference;
    Common::Rect *_boundingBox;
    Common::Array<Common::Point *> *_mouseActiveArea;
    uint32 _zIndex;
    uint32 _assetReference;
    uint32 _startup;
    bool _transparency;
    bool _hasOwnSubfile;
    uint32 _cursorResourceId;
    uint32 _frameRate;
    uint32 _loadType;
    uint32 _totalChunks;
    uint32 _rate;
    bool _editable;
    Graphics::Palette *_palette;
    bool _getOffstageEvents;
    uint32 _x; // Image only.
    uint32 _y; // Image only.
    Common::String *_name;
    Common::HashMap<uint, EventHandler *> _eventHandlers;
    uint32 _stageId;
    SoundEncoding _soundEncoding;
    uint32 _chunkCount;

    // PATH FIELDS.
    uint32 _dissolveFactor;
    Common::Point *_startPoint;
    Common::Point *_endPoint;
    uint32 _stepRate;
    uint32 _duration;

private:
    void readSection(AssetHeader::SectionType sectionType, Chunk &chunk);
    AssetHeader::SectionType getSectionType(Chunk &chunk);
};

} // End of namespace MediaStation

#endif