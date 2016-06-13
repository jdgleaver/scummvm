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

#ifndef MACVENTURE_OBJECT_H
#define MACVENTURE_OBJECT_H

#include "macventure/macventure.h"

namespace MacVenture {

struct ContainerHeader {
	uint32 header;
};

struct ObjectGroup {
	uint32 bitOffset; // Actually uint24, but we don't have that
	uint32 offset;
};

struct ContainerSubHeader {
	uint16 numObjects;
	uint16 *huff;
	uint8 *lengths;
	ObjectGroup *groups;
};

class Object {
public:
	Object();
	~Object();
};

} // End of namespace MacVenture

#endif