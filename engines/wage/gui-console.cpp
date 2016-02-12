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
 * MIT License:
 *
 * Copyright (c) 2009 Alexei Svitkine, Eugene Sandulenko
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "common/timer.h"
#include "common/unzip.h"
#include "graphics/cursorman.h"
#include "graphics/fonts/bdf.h"
#include "graphics/palette.h"

#include "wage/wage.h"
#include "wage/design.h"
#include "wage/entities.h"
#include "wage/menu.h"
#include "wage/gui.h"
#include "wage/world.h"

namespace Wage {

const Graphics::Font *Gui::getConsoleFont() {
	char fontName[128];
	Scene *scene = _engine->_world->_player->_currentScene;

	snprintf(fontName, 128, "%s-%d", scene->getFontName(), scene->_fontSize);

	return getFont(fontName, Graphics::FontManager::kConsoleFont);
}

void Gui::clearOutput() {
	_out.clear();
	_lines.clear();
	_consoleFullRedraw = true;
}

void Gui::appendText(const char *s) {
	Common::String str(s);
	_consoleDirty = true;

	if (!str.contains('\n')) {
		_out.push_back(str);
		flowText(str);
		return;
	}

	// Okay, we got new lines, need to split it
	// and push substrings individually
	Common::String tmp = "";

	for (uint i = 0; i < str.size(); i++) {
		if (str[i] == '\n') {
			_out.push_back(tmp);
			flowText(tmp);
			tmp = "";
			continue;
		}

		tmp += str[i];
	}

	_out.push_back(tmp);
	flowText(tmp);
}

enum {
	kConWOverlap = 20,
	kConHOverlap = 20,
	kConWPadding = 3,
	kConHPadding = 4,
	kConOverscan = 3
};

void Gui::flowText(Common::String &str) {
	Common::StringArray wrappedLines;
	int textW = _consoleTextArea.width() - kConWPadding * 2;
	const Graphics::Font *font = getConsoleFont();

	font->wordWrapText(str, textW, wrappedLines);

	if (wrappedLines.size() == 0) // Sometimes we have empty lines
		_lines.push_back("");

	for (Common::StringArray::const_iterator j = wrappedLines.begin(); j != wrappedLines.end(); ++j)
		_lines.push_back(*j);

	uint pos = _scrollPos;
	_scrollPos = MAX<int>(0, (_lines.size() - 1 - _consoleNumLines) * _consoleLineHeight);

	_cursorX = kConWPadding;

	if (_scrollPos)
		_cursorY = (_consoleNumLines) * _consoleLineHeight + kConHPadding;
	else
		_cursorY = (_lines.size() - 1) * _consoleLineHeight + kConHPadding;

	if (pos != _scrollPos)
		_consoleFullRedraw = true;

	if (!_engine->_temporarilyHidden)
		draw();
}

void Gui::renderConsole(Graphics::Surface *g, Common::Rect &r) {
	bool fullRedraw = _consoleFullRedraw;
	bool textReflow = false;
	int surfW = r.width() + kConWOverlap * 2;
	int surfH = r.height() + kConHOverlap * 2;

	Common::Rect boundsR(kConWOverlap - kConOverscan, kConHOverlap - kConOverscan,
					r.width() + kConWOverlap + kConOverscan, r.height() + kConHOverlap + kConOverscan);
	Common::Rect fullR(0, 0, surfW, surfH);

	if (_console.w != surfW || _console.h != surfH) {
		if (_console.w != surfW)
			textReflow = true;

		_console.free();

		_console.create(surfW, surfH, Graphics::PixelFormat::createFormatCLUT8());
		fullRedraw = true;
	}

	if (fullRedraw)
		_console.fillRect(fullR, kColorWhite);

	const Graphics::Font *font = getConsoleFont();

	_consoleLineHeight = font->getFontHeight();
	int textW = r.width() - kConWPadding * 2;
	int textH = r.height() - kConHPadding * 2;

	if (textReflow) {
		_lines.clear();

		for (uint i = 0; i < _out.size(); i++)
			flowText(_out[i]);
	}

	const int firstLine = _scrollPos / _consoleLineHeight;
	const int lastLine = MIN((_scrollPos + textH) / _consoleLineHeight + 1, _lines.size());
	const int xOff = kConWOverlap;
	const int yOff = kConHOverlap;
	int x1 = xOff + kConWPadding;
	int y1 = yOff - (_scrollPos % _consoleLineHeight) + kConHPadding;

	if (fullRedraw)
		_consoleNumLines = (r.height() - 2 * kConWPadding) / _consoleLineHeight - 2;

	for (int line = firstLine; line < lastLine; line++) {
		const char *str = _lines[line].c_str();
		int color = kColorBlack;

		if ((line > _selectionStartY && line < _selectionEndY) ||
			(line > _selectionEndY && line < _selectionStartY)) {
			color = kColorWhite;
			Common::Rect trect(0, y1, _console.w, y1 + _consoleLineHeight);

			Design::drawFilledRect(&_console, trect, kColorBlack, _patterns, kPatternSolid);
		}

		if (line == _selectionStartY || line == _selectionEndY) {
			if (_selectionStartY != _selectionEndY) {
				int color1 = kColorBlack;
				int color2 = kColorWhite;
				int midpoint = _selectionStartX;

				if (_selectionStartY > _selectionEndY)
					SWAP(color1, color2);

				if (line == _selectionEndY) {
					SWAP(color1, color2);
					midpoint = _selectionEndX;
				}

				Common::String beg(_lines[line].c_str(), &_lines[line].c_str()[midpoint]);
				Common::String end(&_lines[line].c_str()[midpoint]);

				int rectW = font->getStringWidth(beg) + kConWPadding + kConWOverlap;
				Common::Rect trect(0, y1, _console.w, y1 + _consoleLineHeight);
				if (color1 == kColorWhite)
					trect.right = rectW;
				else
					trect.left = rectW;

				Design::drawFilledRect(&_console, trect, kColorBlack, _patterns, kPatternSolid);

				font->drawString(&_console, beg, x1, y1, textW, color1);
				font->drawString(&_console, end, x1 + rectW - kConWPadding - kConWOverlap, y1, textW, color2);
			} else {
				int startPos = _selectionStartX;
				int endPos = _selectionEndX;

				if (startPos > endPos)
					SWAP(startPos, endPos);

				Common::String beg(_lines[line].c_str(), &_lines[line].c_str()[startPos]);
				Common::String mid(&_lines[line].c_str()[startPos], &_lines[line].c_str()[endPos]);
				Common::String end(&_lines[line].c_str()[endPos]);

				int rectW1 = font->getStringWidth(beg) + kConWPadding + kConWOverlap;
				int rectW2 = rectW1 + font->getStringWidth(mid);
				Common::Rect trect(rectW1, y1, rectW2, y1 + _consoleLineHeight);

				Design::drawFilledRect(&_console, trect, kColorBlack, _patterns, kPatternSolid);

				font->drawString(&_console, beg, x1, y1, textW, kColorBlack);
				font->drawString(&_console, mid, x1 + rectW1 - kConWPadding - kConWOverlap, y1, textW, kColorWhite);
				font->drawString(&_console, end, x1 + rectW2 - kConWPadding - kConWOverlap, y1, textW, kColorBlack);
			}
		} else {
			if (*str)
				font->drawString(&_console, _lines[line], x1, y1, textW, color);
		}

		y1 += _consoleLineHeight;
	}

	g->copyRectToSurface(_console, r.left - kConOverscan, r.top - kConOverscan, boundsR);
	g_system->copyRectToScreen(g->getBasePtr(r.left, r.top), g->pitch, r.left, r.top, r.width(), r.height());
}

void Gui::drawInput() {
	if (!_screen.getPixels())
		return;

	if (_sceneIsActive) {
		_sceneIsActive = false;
		_bordersDirty = true;
	}

	_out.pop_back();
	_lines.pop_back();
	appendText(_engine->_inputText.c_str());
	_inputTextLineNum = _out.size() - 1;

	const Graphics::Font *font = getConsoleFont();

	if (_engine->_inputText.contains('\n')) {
		_consoleDirty = true;
	} else {
		int x = kConWPadding + _consoleTextArea.left;
		int y = _cursorY + _consoleTextArea.top;

		Common::Rect r(x, y, x + _consoleTextArea.width() - kConWPadding, y + font->getFontHeight());
		_screen.fillRect(r, kColorWhite);

		undrawCursor();

		font->drawString(&_screen, _out[_inputTextLineNum], x, y, _screen.w, kColorBlack);

		g_system->copyRectToScreen(_screen.getBasePtr(x, y), _screen.pitch, x, y, _consoleTextArea.width(), font->getFontHeight());
	}

	_cursorX = font->getStringWidth(_out[_inputTextLineNum]) + kConHPadding;
}

void Gui::actionCopy() {
	if (_selectionStartX == -1)
		return;

	int startX = _selectionStartX;
	int startY = _selectionStartY;
	int endX = _selectionEndX;
	int endY = _selectionEndY;

	if (startY > endY) {
		SWAP(startX, endX);
		SWAP(endX, endY);
	}

	_clipboard.clear();

	for (int i = startY; i <= endY; i++) {
		if (startY == endY) {
			_clipboard = Common::String(&_lines[i].c_str()[startX], &_lines[i].c_str()[endX]);
			break;
		}

		if (i == startY) {
			_clipboard += &_lines[i].c_str()[startX];
			_clipboard += '\n';
		} else if (i == endY) {
			_clipboard += Common::String(_lines[i].c_str(), &_lines[i].c_str()[endX]);
		} else {
			_clipboard += _lines[i];
			_clipboard += '\n';
		}
	}

	_menu->enableCommand(kMenuEdit, kMenuActionPaste, true);
}

void Gui::actionPaste() {
	_undobuffer = _engine->_inputText;
	_engine->_inputText += _clipboard;
	drawInput();
	_engine->_inputText = _out.back();	// Set last part of the multiline text

	_menu->enableCommand(kMenuEdit, kMenuActionUndo, true);
}

void Gui::actionUndo() {
	_engine->_inputText = _undobuffer;
	drawInput();

	_menu->enableCommand(kMenuEdit, kMenuActionUndo, false);
}

void Gui::disableUndo() {
	_menu->enableCommand(kMenuEdit, kMenuActionUndo, false);
}

} // End of namespace Wage
