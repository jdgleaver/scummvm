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

#include "director/lingo/lingo.h"
#include "director/lingo/lingo-gr.h"

namespace Director {

Lingo *g_lingo;

struct EventHandlerType {
	LEvent handler;
	const char *name;
} static const eventHanlerDescs[] = {
	{ kEventPrepareMovie,		"prepareMovie" },
	{ kEventStartMovie,			"startMovie" },
	{ kEventStopMovie,			"stopMovie" },

	{ kEventNew,				"newSprite" },
	{ kEventBeginSprite,		"beginSprite" },
	{ kEventEndSprite,			"endSprite" },

	{ kEventEnterFrame, 		"enterFrame" },
	{ kEventPrepareFrame, 		"prepareFrame" },
	{ kEventIdle,				"idle" },
	{ kEventStepFrame,			"stepFrame"},
	{ kEventExitFrame, 			"exitFrame" },

	{ kEventActivateWindow,		"activateWindow" },
	{ kEventDeactivateWindow,	"deactivateWindow" },
	{ kEventMoveWindow,			"moveWindow" },
	{ kEventResizeWindow,		"resizeWindow" },
	{ kEventOpenWindow,			"openWindow" },
	{ kEventCloseWindow,		"closeWindow" },
	{ kEventStart,				"start" },

	{ kEventKeyUp,				"keyUp" },
	{ kEventKeyDown,			"keyDown" },
	{ kEventMouseUp,			"mouseUp" },
	{ kEventMouseDown,			"mouseDown" },
	{ kEventRightMouseDown,		"rightMouseDown" },
	{ kEventRightMouseUp,		"rightMouseUp" },
	{ kEventMouseEnter,			"mouseEnter" },
	{ kEventMouseLeave,			"mouseLeave" },
	{ kEventMouseUpOutSide,		"mouseUpOutSide" },
	{ kEventMouseWithin,		"mouseWithin" },

	{ kEventNone,				0 },
};

Symbol::Symbol() {
	name = NULL;
	type = VOID;
	u.s = NULL;
	nargs = 0;
	global = false;
}

Lingo::Lingo(DirectorEngine *vm) : _vm(vm) {
	g_lingo = this;

	for (const EventHandlerType *t = &eventHanlerDescs[0]; t->handler != kEventNone; ++t)
		_eventHandlerTypes[t->handler] = t->name;

	initBuiltIns();

	_currentScript = 0;
	_currentScriptType = kMovieScript;
	_pc = 0;
	_returning = false;
	_indef = false;

	_linenumber = _colnumber = 0;

	warning("Lingo Inited");
}

Lingo::~Lingo() {
}

void Lingo::addCode(Common::String code, ScriptType type, uint16 id) {
	code += '\n';

	debug(2, "Add code \"%s\" for type %d with id %d", code.c_str(), type, id);

	if (_scripts[type].contains(id)) {
		delete _scripts[type][id];
	}

	_currentScript = new ScriptData;
	_currentScriptType = type;
	_scripts[type][id] = _currentScript;

	_linenumber = _colnumber = 1;

	// macros have conflicting grammar. Thus we ease life for the parser.
	if (code.contains("\nmacro ")) {
		const char *begin = strstr(code.c_str(), "\nmacro ") + 1;
		const char *end;
		bool first = true;

		while ((end = strstr(begin, "\nmacro "))) {
			if (first) {
				begin = code.c_str();
				first = false;
			}
			Common::String chunk(begin, end);

			parse(chunk.c_str());

			_currentScript->clear();

			begin = end + 1;
		}

		parse(begin);
	} else {
		parse(code.c_str());

		code1(STOP);
	}

	Common::hexdump((byte *)&_currentScript->front(), _currentScript->size() * sizeof(inst));
}

void Lingo::executeScript(ScriptType type, uint16 id) {
	if (!_scripts[type].contains(id)) {
		warning("Request to execute non-existant script type %d id %d", type, id);
		return;
	}

	_currentScript = _scripts[type][id];
	_pc = 0;
	_returning = false;

	_localvars = new SymbolHash;

	execute(_pc);

	cleanLocalVars();
}

void Lingo::processEvent(LEvent event, int entityId) {
	if (!_eventHandlerTypes.contains(event))
		error("processEvent: Unknown event %d for entity %d", event, entityId);

	debug(2, "processEvent(%s) for %d", _eventHandlerTypes[event], entityId);
}

int Lingo::alignTypes(Datum &d1, Datum &d2) {
	int opType = INT;

	if (d1.type == FLOAT || d2.type == FLOAT) {
		opType = FLOAT;
		d1.toFloat();
		d2.toFloat();
	}

	return opType;
}

int Datum::toInt() {
	switch (type) {
	case INT:
		// no-op
		break;
	case FLOAT:
		u.i = (int)u.f;
		type = INT;
		break;
	default:
		warning("Incorrect operation toInt() for type: %s", type2str());
	}

	return u.i;
}

float Datum::toFloat() {
	switch (type) {
	case INT:
		u.f = (float)u.i;
		type = FLOAT;
		break;
	case FLOAT:
		// no-op
		break;
	default:
		warning("Incorrect operation toFloat() for type: %s", type2str());
	}

	return u.f;
}

Common::String *Datum::toString() {
	Common::String *s = new Common::String;
	switch (type) {
	case INT:
		s->format("%d", u.i);
		break;
	case FLOAT:
		s->format("%f", u.f);
		break;
	case STRING:
		delete s;
		s = u.s;
		break;
	default:
		warning("Incorrect operation toString() for type: %s", type2str());
	}

	u.s = s;
	type = STRING;

	return u.s;
}

const char *Datum::type2str() {
	static char res[20];

	switch (type) {
	case INT:
		return "INT";
	case FLOAT:
		return "FLOAT";
	case STRING:
		return "STRING";
	case CASTREF:
		return "CASTREF";
	default:
		snprintf(res, 20, "-- (%d) --", type);
		return res;
	}
}

// This is table for built-in Macintosh font lowercasing.
// '.' means that the symbol should be not changed, rest
// of the symbols are stripping the diacritics
// The table starts from 0x80
//
// TODO: Check it for correctness.
static char lowerCaseConvert[] =
"aacenoua" // 80
"aaaaacee" // 88
"eeiiiino" // 90
"oooouuuu" // 98
"........" // a0
".......o" // a8
"........" // b0
".......o" // b8
"........" // c0
".. aao.." // c8
"--.....y";// d0-d8

Common::String *Lingo::toLowercaseMac(Common::String *s) {
	Common::String *res = new Common::String;
	const unsigned char *p = (const unsigned char *)s->c_str();

	while (*p) {
		if (*p >= 0x80 && *p <= 0xd8) {
			if (lowerCaseConvert[*p - 0x80] != '.')
				*res += lowerCaseConvert[*p - 0x80];
			else
				*res += *p;
		} else if (*p < 0x80) {
			*res += tolower(*p);
		} else {
			error("Unacceptable symbol in toLowercaseMac: %c", *p);
		}
		p++;
	}

	return res;
}

} // End of namespace Director
