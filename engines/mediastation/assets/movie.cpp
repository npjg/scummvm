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

#include "mediastation/assets/movie.h"
#include "mediastation/datum.h"

namespace MediaStation {

MovieFrameHeader::MovieFrameHeader(Chunk &chunk) : BitmapHeader(chunk) {
    _index = Datum(chunk).u.i;
    _keyframeEndInMilliseconds = Datum(chunk).u.i;
}

MovieFrameFooter::MovieFrameFooter(Chunk &chunk) {
    _unk1 = Datum(chunk).u.i;
    _unk2 = Datum(chunk).u.i;
    if (g_engine->isFirstGenerationEngine()) { // TODO: Add the version number check. 
        _startInMilliseconds = Datum(chunk).u.i;
        _endInMilliseconds = Datum(chunk).u.i;
        _left = Datum(chunk).u.i;
        _top = Datum(chunk).u.i;
        _unk3 = Datum(chunk).u.i;
        _unk4 = Datum(chunk).u.i;
        _index = Datum(chunk).u.i;
    } else {
        _unk4 = Datum(chunk).u.i;
        _startInMilliseconds = Datum(chunk).u.i;
        _endInMilliseconds = Datum(chunk).u.i;
        _left = Datum(chunk).u.i;
        _top = Datum(chunk).u.i;
        _unk5 = Datum(chunk).u.i;
        _unk6 = Datum(chunk).u.i;
        _unk7 = Datum(chunk).u.i;
        _index = Datum(chunk).u.i;
        _unk8 = Datum(chunk).u.i;
        _unk9 = Datum(chunk).u.i;
        debugC(5, kDebugLoading, "MovieFrameFooter::MovieFrameFooter(): _startInMilliseconds = 0x%x, _endInMilliseconds = 0x%x, _left = 0x%x, _top = 0x%x, _index = 0x%x (@0x%x)", _startInMilliseconds, _endInMilliseconds, _left, _top, _index, chunk.pos());
        debugC(5, kDebugLoading, "MovieFrameFooter::MovieFrameFooter(): _unk4 = 0x%x, _unk5 = 0x%x, _unk6 = 0x%x, _unk7 = 0x%x, _unk8 = 0x%x, _unk9 = 0x%x", _unk4, _unk5, _unk6, _unk7, _unk8, _unk9);
    }
}

Movie::Movie(AssetHeader *header) : _header(header) {

}

Movie::~Movie() {
    // The Movie doesn't own the header, so it doesn't need to be deleted here.
    // Just set it null.
    _header = nullptr;
    for (MovieFrame *frame : _stills) {
        delete frame;
    }
    _stills.clear();
    for (MovieFrame *frame : _frames) {
        delete frame;
    }
    _frames.clear();
    for (Sound *sound : _sounds) {
        delete sound;
    }
    _sounds.clear();
    for (MovieFrameFooter *footer : _footers) {
        delete footer;
    }
    _footers.clear();
}

void Movie::readStill(Chunk &chunk) {
    uint sectionType = Datum(chunk).u.i;
    switch ((SectionType)sectionType) {
        case SectionType::FRAME: {
            debugC(5, kDebugLoading, "Movie::readStill(): Reading frame");
            MovieFrameHeader *header = new MovieFrameHeader(chunk);
            MovieFrame *frame = new MovieFrame(chunk, header);
            _stills.push_back(frame);
            break;
        }

        case SectionType::FOOTER: {
            debugC(5, kDebugLoading, "Movie::readStill(): Reading footer");
            MovieFrameFooter *footer = new MovieFrameFooter(chunk);
            _footers.push_back(footer);
            break;
        }

        default: {
            error("Unknown movie still section type");
        }
    }
}

void Movie::readSubfile(Subfile &subfile, Chunk &chunk) {
    // READ THE METADATA FOR THE WHOLE MOVIE.
    uint expectedRootSectionType = Datum(chunk).u.i;
    debugC(5, kDebugLoading, "Movie::readSubfile(): sectionType = 0x%x (@0x%lx)", expectedRootSectionType, chunk.pos());
    if (Movie::SectionType::ROOT != (Movie::SectionType)expectedRootSectionType) {
        error("Expected ROOT section type, got 0x%x", expectedRootSectionType);
    }
    uint chunkCount = Datum(chunk).u.i;
    debugC(5, kDebugLoading, "Movie::readSubfile(): chunkCount = 0x%x (@0x%lx)", chunkCount, chunk.pos());

    uint dataStartOffset = Datum(chunk).u.i;
    debugC(5, kDebugLoading, "Movie::readSubfile(): dataStartOffset = 0x%x (@0x%lx)", dataStartOffset, chunk.pos());

    Common::Array<uint> chunkLengths;
    for (uint i = 0; i < chunkCount; i++) {
        uint chunkLength = Datum(chunk).u.i;
        debugC(5, kDebugLoading, "Movie::readSubfile(): chunkLength = 0x%x (@0x%lx)", chunkLength, chunk.pos());
        chunkLengths.push_back(chunkLength);
    }

    // RAD THE INTERLEAVED AUDIO AND ANIMATION DATA.
    for (uint i = 0; i < chunkCount; i++) {
        debugC(5, kDebugLoading, "\nMovie::readSubfile(): Reading frameset %d of %d in subfile (@0x%lx)", i, chunkCount, chunk.pos());
        chunk = subfile.nextChunk();

        // READ ALL THE FRAMES IN THIS CHUNK.
        debugC(5, kDebugLoading, "Movie::readSubfile(): (Frameset %d of %d) Reading animation chunks... (@0x%lx)", i, chunkCount, chunk.pos());
        bool isAnimationChunk = (chunk.id == _header->_animationChunkReference);
        if (!isAnimationChunk) {
            warning("Movie::readSubfile(): (Frameset %d of %d) No animation chunks found (0x%x)", i, chunkCount, chunk.pos());
        }
        while (isAnimationChunk) {
            uint sectionType = Datum(chunk).u.i;
            debugC(5, kDebugLoading, "Movie::readSubfile(): sectionType = 0x%x (@0x%lx)", sectionType, chunk.pos());
            switch (Movie::SectionType(sectionType)) {
                case Movie::SectionType::FRAME: {
                    MovieFrameHeader *header = new MovieFrameHeader(chunk);
                    MovieFrame *frame = new MovieFrame(chunk, header);
                    _frames.push_back(frame);
                    break;
                }

                case Movie::SectionType::FOOTER: {
                    MovieFrameFooter *footer = new MovieFrameFooter(chunk);
                    _footers.push_back(footer);
                    // TODO: We donʻt do anything with this yet!
                    break;
                }

                default: {
                    error("Movie::readSubfile(): Unknown movie animation section type 0x%x (@0x%lx)", sectionType, chunk.pos());
                }
            }

            // READ THE NEXT CHUNK.
            chunk = subfile.nextChunk();
            isAnimationChunk = (chunk.id == _header->_animationChunkReference);
        }

        // READ THE AUDIO.
        debugC(5, kDebugLoading, "Movie::readSubfile(): (Frameset %d of %d) Reading audio chunk... (0x%lx)", i, chunkCount, chunk.pos());
        bool isAudioChunk = (chunk.id = _header->_audioChunkReference);
        if (isAudioChunk) {
            Sound *sound = new Sound(_header->_soundEncoding);
            sound->readChunk(chunk);
            _sounds.push_back(sound);
            chunk = subfile.nextChunk();
        } else {
            debugC(5, kDebugLoading, "Movie::readSubfile(): (Frameset %d of %d) No audio chunk to read. (0x%lx)", i, chunkCount, chunk.pos());
        }

        // READ THE FOOTER FOR THIS SUBFILE.
        debugC(5, kDebugLoading, "Movie::readSubfile(): (Frameset %d of %d) Reading header chunk... (@0x%lx)", i, chunkCount, chunk.pos());
        bool isHeaderChunk = (chunk.id == _header->_chunkReference);
        if (isHeaderChunk) {
            if (chunk.length != 0x04) {
                error("Movie::readSubfile(): Expected movie header chunk of size 0x04, got 0x%x", chunk.length);
            }
            chunk.skip(chunk.length);
        } else {
            error("Movie::readSubfile(): Expected header chunk, got %s", tag2str(chunk.id));
        }
    }

    // SET THE MOVIE FRAME FOOTERS.
    // TODO: We donʻt do anything with this yet!
}

} // End of namespace MediaStation