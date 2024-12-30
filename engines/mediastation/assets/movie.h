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

#include "audio/audiostream.h"

#include "mediastation/assetheader.h"
#include "mediastation/bitmap.h"
#include "mediastation/mediascript/builtins.h"

#ifndef MEDIASTATION_MOVIE_H
#define MEDIASTATION_MOVIE_H

namespace MediaStation {

class MovieFrameHeader : public BitmapHeader {
public:
	MovieFrameHeader(Chunk &chunk);

	uint _index;
	uint _keyframeEndInMilliseconds;
};

class MovieFrameFooter {
public:
	MovieFrameFooter(Chunk &chunk);

	uint _unk1;
	uint _unk2;
	uint _startInMilliseconds;
	uint _endInMilliseconds;
	uint _left;
	uint _top;
	uint _unk3;
	uint _unk4;
	uint _zIndex; // TODO: This is still unconfirmed but seems likely.
	uint _unk6;
	uint _unk7;
	uint _unk8;
	uint _unk9;
	uint _index;
};

class MovieFrame : public Bitmap {
public:
	MovieFrame(Chunk &chunk, MovieFrameHeader *header);
	~MovieFrame();

	void setFooter(MovieFrameFooter *footer);
	uint32 left();
	uint32 top();
	Common::Point topLeft();
	Common::Rect boundingBox();
	uint32 index();
	uint32 startInMilliseconds();
	uint32 endInMilliseconds();
	uint32 keyframeEndInMilliseconds();
	// This is called zCoordinate because zIndex is too close to "index" and
	// that could be confusing.
	uint32 zCoordinate();

	bool _showing;

private:
	MovieFrameHeader *_bitmapHeader;
	MovieFrameFooter *_footer;
};

class Movie : public Asset {
private:
	enum class SectionType {
		ROOT = 0x06a8,
		FRAME = 0x06a9,
		FOOTER = 0x06aa
	};

public:
	Movie(AssetHeader *header) : Asset(header) {};
	virtual ~Movie() override;

	virtual void readChunk(Chunk &chunk) override;
	virtual void readSubfile(Subfile &subfile, Chunk &chunk) override;

	virtual Operand callMethod(BuiltInMethod methodId, Common::Array<Operand> &args) override;
	virtual void process() override;

private:
	Common::Array<MovieFrame *> _frames;
	Common::Array<MovieFrame *> _stills;
	Common::Array<MovieFrameFooter *> _footers;
	Common::Array<Audio::SeekableAudioStream *> _audioStreams;

	// Method implementations. These should be called from callMethod.
	void timePlay();
	void timeStop();

	// Internal helper functions.
	bool drawNextFrame();
	void processTimeEventHandlers();
};

} // End of namespace MediaStation

#endif