/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef STARTREK_OBJECT_H
#define STARTREK_OBJECT_H

#include "startrek/sprite.h"

#include "common/scummsys.h"

namespace StarTrek {

class StarTrekEngine;
class FileStream;


struct Object {
	uint16 spriteDrawn;
	char animationString3[16];
	uint16 animType;
	Sprite sprite;
	char animationString4[10];
	uint16 scale;
	SharedPtr<FileStream> animFile;
	uint16 numAnimFrames;
	uint16 animFrame;
	uint32 frameToStartNextAnim;
	uint16 field5e;
	uint16 field60;
	uint16 field62;
	uint16 field64;
	uint16 field66;
	char animationString2[8];
	uint16 field70;
	uint16 field72;
	uint16 field74;
	uint16 field76;
	uint16 iwSrcPosition;
	uint16 iwDestPosition;
	uint16 field7c;
	uint16 field7e;
	uint16 field80;
	uint16 field82;
	uint16 field84;
	uint16 field86;
	uint16 field88;
	uint16 field8a;
	uint16 field8c;
	uint16 field8e;
	uint16 field90;
	uint16 field92;
	uint16 field94;
	uint16 field96;
	char animationString[9];
	uint8  fielda1;
	uint16 fielda2;
	uint16 fielda4;
	uint16 fielda6;
};

}

#endif

