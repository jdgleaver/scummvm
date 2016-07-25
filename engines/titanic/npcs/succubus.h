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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TITANIC_SUCCUBUS_H
#define TITANIC_SUCCUBUS_H

#include "titanic/npcs/true_talk_npc.h"

namespace Titanic {

class CSuccUBus : public CTrueTalkNPC {
private:
	static int _v0;
	static int _v1;
	static int _v2;
	static int _v3;
	static int _v4;
private:
	int _field108;
	int _field10C;
	int _field110;
	int _field114;
	int _field118;
	int _field11C;
	int _field120;
	int _field124;
	int _field128;
	int _field12C;
	int _field130;
	int _field134;
	int _field138;
	int _field13C;
	int _field140;
	int _field144;
	int _field148;
	int _field14C;
	int _field150;
	int _field154;
	int _field158;
	int _field15C;
	CString _string2;
	int _field16C;
	int _field170;
	int _field174;
	int _field178;
	int _field17C;
	int _field180;
	int _field184;
	int _field188;
	int _field18C;
	int _field190;
	int _field194;
	int _field198;
	int _field19C;
	int _field1A0;
	int _field1A4;
	int _field1A8;
	int _field1AC;
	int _field1B0;
	int _field1B4;
	int _field1B8;
	int _field1BC;
	int _field1C0;
	int _field1C4;
	int _field1C8;
	int _field1CC;
	int _field1D0;
	int _field1D4;
	int _field1D8;
public:
	CLASSDEF;
	CSuccUBus();

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent);

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file);
};

} // End of namespace Titanic

#endif /* TITANIC_SUCCUBUS_H */
