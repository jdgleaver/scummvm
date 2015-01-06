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

#include "xeen/interface.h"
#include "xeen/dialogs_error.h"
#include "xeen/resources.h"
#include "xeen/xeen.h"

namespace Xeen {

Interface::Interface(XeenEngine *vm) : ButtonContainer(), _vm(vm) {
	Common::fill(&_partyFaces[0], &_partyFaces[MAX_ACTIVE_PARTY], nullptr);
	_batUIFrame = 0;
	_spotDoorsUIFrame = 0;
	_dangerSenseUIFrame = 0;
	_face1UIFrame = 0;
	_face2UIFrame = 0;
	_blessedUIFrame = 0;
	_powerShieldUIFrame = 0;
	_holyBonusUIFrame = 0;
	_heroismUIFrame = 0;
	_isEarlyGame = false;
	_buttonsLoaded = false;
	_hiliteChar = -1;
	Common::fill(&_combatCharIds[0], &_combatCharIds[8], 0);
	_intrIndex1 = 0;

	initDrawStructs();
}

void Interface::initDrawStructs() {
	_faceDrawStructs[0] = DrawStruct(0, 0, 0);
	_faceDrawStructs[1] = DrawStruct(0, 101, 0);
	_faceDrawStructs[2] = DrawStruct(0, 0, 43);
	_faceDrawStructs[3] = DrawStruct(0, 101, 43);

	_mainList[0] = DrawStruct(7, 232, 74);
	_mainList[1] = DrawStruct(0, 235, 75);
	_mainList[2] = DrawStruct(2, 260, 75);
	_mainList[3] = DrawStruct(4, 286, 75);
	_mainList[4] = DrawStruct(6, 235, 96);
	_mainList[5] = DrawStruct(8, 260, 96);
	_mainList[6] = DrawStruct(10, 286, 96);
	_mainList[7] = DrawStruct(12, 235, 117);
	_mainList[8] = DrawStruct(14, 260, 117);
	_mainList[9] = DrawStruct(16, 286, 117);
	_mainList[10] = DrawStruct(20, 235, 148);
	_mainList[11] = DrawStruct(22, 260, 148);
	_mainList[12] = DrawStruct(24, 286, 148);
	_mainList[13] = DrawStruct(26, 235, 169);
	_mainList[14] = DrawStruct(28, 260, 169);
	_mainList[15] = DrawStruct(30, 286, 169);
}

void Interface::setup() {
	_globalSprites.load("global.icn");
	_borderSprites.load("border.icn");
	_spellFxSprites.load("spellfx.icn");
	_fecpSprites.load("fecp.brd");
	_blessSprites.load("bless.icn");
	_restoreSprites.load("restorex.icn");
	_hpSprites.load("hpbars.icn");
	_uiSprites.load("inn.icn");

	// Get mappings to the active characters in the party
	_vm->_party._activeParty.resize(_vm->_party._partyCount);
	for (int i = 0; i < _vm->_party._partyCount; ++i) {
		_vm->_party._activeParty[i] = _vm->_roster[_vm->_party._partyMembers[i]];
	}

	_isEarlyGame = _vm->_party._minutes >= 300;
}

void Interface::manageCharacters(bool soundPlayed) {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;
	bool flag = false;

start:
	if (_vm->_party._mazeId != 0) {
		_vm->_mode = MODE_0;
		_buttonsLoaded = true;
	} else {
		if (!soundPlayed) {
			warning("TODO: loadSound?");
		}

		if (!_partyFaces[0]) {
			// Xeen only uses 24 of possible 30 character slots
			loadCharIcons();

			for (int i = 0; i < _vm->_party._partyCount; ++i)
				_partyFaces[i] = &_charFaces[_vm->_party._partyMembers[i]];
		}

		_vm->_mode = MODE_1;
		Common::Array<int> xeenSideChars;

		// Build up a list of characters on the same Xeen side being loaded
		for (int i = 0; i < XEEN_TOTAL_CHARACTERS; ++i) {
			PlayerStruct &player = _vm->_roster[i];
			if (player._name.empty() || player._xeenSide != _vm->_loadDarkSide)
				continue;

			xeenSideChars.push_back(i);
		}

		// Add in buttons for the UI
		_interfaceText = "";
		_buttonsLoaded = true;
		addButton(Common::Rect(16, 100, 40, 120), 242, &_uiSprites, true);
		addButton(Common::Rect(52, 100, 76, 120), 243, &_uiSprites, true);
		addButton(Common::Rect(87, 100, 111, 120), 68, &_uiSprites, true);
		addButton(Common::Rect(122, 100, 146, 120), 82, &_uiSprites, true);
		addButton(Common::Rect(157, 100, 181, 120), 67, &_uiSprites, true);
		addButton(Common::Rect(192, 100, 216, 120), 88, &_uiSprites, true);
		addButton(Common::Rect(), 27, &_uiSprites, false);
		addButton(Common::Rect(16, 16, 48, 48), 49, &_uiSprites, false);
		addButton(Common::Rect(117, 16, 139, 48), 50, &_uiSprites, false);
		addButton(Common::Rect(16, 59, 48, 81), 51, &_uiSprites, false);
		addButton(Common::Rect(117, 59, 149, 81), 52, &_uiSprites, false);

		setupBackground();
		Window &w = screen._windows[11];
		w.open();
		setupFaces(0, xeenSideChars, false);
		w.writeString(_interfaceText);
		w.drawList(&_faceDrawStructs[0], 4);

		_uiSprites.draw(w, 0, Common::Point(16, 100));
		_uiSprites.draw(w, 2, Common::Point(52, 100));
		_uiSprites.draw(w, 4, Common::Point(87, 100));
		_uiSprites.draw(w, 6, Common::Point(122, 100));
		_uiSprites.draw(w, 8, Common::Point(157, 100));
		_uiSprites.draw(w, 10, Common::Point(192, 100));

		screen.loadPalette("mm4.pal");

		if (flag) {
			screen._windows[0].update();
			events.setCursor(0);
			screen.fadeIn(4);
		} else {
			if (_vm->getGameID() == GType_DarkSide) {
				screen.fadeOut(4);
				screen._windows[0].update();
			}

			doScroll(_vm, false, false);
			events.setCursor(0);

			if (_vm->getGameID() == GType_DarkSide) {
				screen.fadeIn(4);
			}
		}

		// TODO
		bool breakFlag = false;
		while (!_vm->shouldQuit() && !breakFlag) {
			events.pollEventsAndWait();
			checkEvents(_vm);

			switch (_buttonValue) {
			case Common::KEYCODE_ESCAPE:
			case Common::KEYCODE_SPACE:
			case Common::KEYCODE_e:
			case Common::KEYCODE_x:
				if (_vm->_party._partyCount == 0) {
					ErrorScroll::show(_vm, NO_ONE_TO_ADVENTURE_WITH);
				} else {
					if (_vm->_mode != MODE_0) {
						for (_intrIndex1 = 4; _intrIndex1 >= 0; --_intrIndex1) {
							events.updateGameCounter();
							drawViewBackground(_intrIndex1);
							w.update();

							while (events.timeElapsed() < 1)
								events.pollEventsAndWait();
						}
					}

					w.close();
					_vm->_party._realPartyCount = _vm->_party._partyCount;
					_vm->_party._mazeId = _vm->_party._priorMazeId;

					_vm->_party.copyPartyToRoster(_vm->_roster);
					_vm->_saves->writeCharFile();
					breakFlag = true;
					break;
				}
				break;
			case Common::KEYCODE_1:
				break;
			case Common::KEYCODE_2:
				break;
			case Common::KEYCODE_3:
				break;
			case Common::KEYCODE_4:
				break;
			case Common::KEYCODE_c:
				if (xeenSideChars.size() == 24) {
					ErrorScroll::show(_vm, YOUR_ROSTER_IS_FULL);
				} else {
					screen.fadeOut(4);
					w.close();
					addCharacterToRoster();
					_vm->_saves->writeCharFile();
					screen.fadeOut(4);
					flag = true;
					_buttonsLoaded = true;
					goto start;
				}
				break;
			case Common::KEYCODE_d:
				break;
			case Common::KEYCODE_r:
				if (_vm->_party._partyCount > 0) {
					// TODO
				}
				break;
			case 201:
				// TODO
				break;
			case 202:
				// TODO
				break;
			case 203:
				// TODO
				break;
			case 204:
				// TODO
				break;
			case 205:
				// TODO
				break;
			case 206:
				// TODO
				break;
			case 242:
				// TODO
				break;
			case 243:
				// TODO
				break;
			default:
				break;
			}
		}
	}

	for (int i = 0; i < TOTAL_CHARACTERS; ++i)
		_charFaces[i].clear();
	_globalSprites.clear();
	_borderSprites.clear();
	_spellFxSprites.clear();
	_fecpSprites.clear();
	_blessSprites.clear();
	_restoreSprites.clear();
	_hpSprites.clear();
	_uiSprites.clear();
}

void Interface::loadCharIcons() {
	for (int i = 0; i < XEEN_TOTAL_CHARACTERS; ++i) {
		// Load new character resource
		Common::String name = Common::String::format("char%02d.fac", i + 1);
		_charFaces[i].load(name);
	}

	_dseFace.load("dse.fac");
}

void Interface::loadPartyIcons() {
	for (int i = 0; i < _vm->_party._partyCount; ++i)
		_partyFaces[i] = &_charFaces[_vm->_party._partyMembers[i]];
}

void Interface::setupBackground() {
	_vm->_screen->loadBackground("back.raw");
	assembleBorder();
}

void Interface::assembleBorder() {
	Screen &screen = *_vm->_screen;
	Window &gameWindow = screen._windows[28];

	// Draw the outer frame
	_globalSprites.draw(gameWindow, 0);

	// Draw the animating bat character used to show when levitate is active
	_borderSprites.draw(screen, _vm->_party._levitateActive ? _batUIFrame + 16 : 16);
	_batUIFrame = (_batUIFrame + 1) % 12;

	// Draw UI element to indicate whether can spot hidden doors
	_borderSprites.draw(screen,
		(_vm->_spotDoorsAllowed && _vm->_party.checkSkill(SPOT_DOORS)) ? _spotDoorsUIFrame + 28 : 28,
		Common::Point(194, 91));
	_spotDoorsUIFrame = (_spotDoorsUIFrame + 1) % 12;

	// Draw UI element to indicate whether can sense danger
	_borderSprites.draw(screen,
		(_vm->_dangerSenseAllowed && _vm->_party.checkSkill(DANGER_SENSE)) ? _spotDoorsUIFrame + 40 : 40,
		Common::Point(107, 9));
	_dangerSenseUIFrame = (_dangerSenseUIFrame + 1) % 12;

	// Handle the face UI elements for indicating clairvoyance status
	_face1UIFrame = (_face1UIFrame + 1) % 4;
	if (_vm->_face1State == 0)
		_face1UIFrame += 4;
	else if (_vm->_face1State == 2)
		_face1UIFrame = 0;

	_face2UIFrame = (_face2UIFrame + 1) % 4 + 12;
	if (_vm->_face2State == 0)
		_face2UIFrame += 252;
	else if (_vm->_face2State == 2)
		_face2UIFrame = 0;

	if (!_vm->_party._clairvoyanceActive) {
		_face1UIFrame = 0;
		_face2UIFrame = 8;
	}

	_borderSprites.draw(screen, _face1UIFrame, Common::Point(0, 32));
	_borderSprites.draw(screen,
		screen._windows[10]._enabled || screen._windows[2]._enabled ?
		52 : _face2UIFrame,
		Common::Point(215, 32));

	// Draw resistence indicators
	if (!screen._windows[10]._enabled && !screen._windows[2]._enabled
		&& screen._windows[38]._enabled) {
		_fecpSprites.draw(screen, _vm->_party._fireResistence ? 1 : 0,
			Common::Point(2, 2));
		_fecpSprites.draw(screen, _vm->_party._electricityResistence ? 3 : 2,
			Common::Point(219, 2));
		_fecpSprites.draw(screen, _vm->_party._coldResistence ? 5 : 4,
			Common::Point(2, 134));
		_fecpSprites.draw(screen, _vm->_party._poisonResistence ? 7 : 6,
			Common::Point(219, 134));
	} else {
		_fecpSprites.draw(screen, _vm->_party._fireResistence ? 9 : 8,
			Common::Point(8, 8));
		_fecpSprites.draw(screen, _vm->_party._electricityResistence ? 10 : 11,
			Common::Point(219, 8));
		_fecpSprites.draw(screen, _vm->_party._coldResistence ? 12 : 13,
			Common::Point(8, 134));
		_fecpSprites.draw(screen, _vm->_party._poisonResistence ? 14 : 15,
			Common::Point(219, 134));
	}

	// Draw UI element for blessed
	_blessSprites.draw(screen, 16, Common::Point(33, 137));
	if (_vm->_party._blessedActive) {
		_blessedUIFrame = (_blessedUIFrame + 1) % 4;
		_blessSprites.draw(screen, _blessedUIFrame, Common::Point(33, 137));
	}

	// Draw UI element for power shield
	if (_vm->_party._powerShieldActive) {
		_powerShieldUIFrame = (_powerShieldUIFrame + 1) % 4;
		_blessSprites.draw(screen, _powerShieldUIFrame + 4,
			Common::Point(55, 137));
	}

	// Draw UI element for holy bonus
	if (_vm->_party._holyBonusActive) {
		_holyBonusUIFrame = (_holyBonusUIFrame + 1) % 4;
		_blessSprites.draw(screen, _holyBonusUIFrame + 8, Common::Point(160, 137));
	}

	// Draw UI element for heroism
	if (_vm->_party._heroismActive) {
		_heroismUIFrame = (_heroismUIFrame + 1) % 4;
		_blessSprites.draw(screen, _heroismUIFrame + 12, Common::Point(182, 137));
	}

	// Draw direction character if direction sense is active
	if (_vm->_party.checkSkill(DIRECTION_SENSE) && !_vm->_noDirectionSense) {
		const char *dirText = DIRECTION_TEXT[_vm->_party._mazeDirection];
		Common::String msg = Common::String::format(
			"\002""08\003""c\013""139\011""116%c\014""d\001", *dirText);
		screen._windows[0].writeString(msg);
	}

	// Draw view frame
	if (screen._windows[12]._enabled)
		screen._windows[12].frame();
}

void Interface::setupFaces(int charIndex, Common::Array<int> xeenSideChars, bool updateFlag) {
	Common::String playerNames[4];
	Common::String playerRaces[4];
	Common::String playerSex[4];
	Common::String playerClass[4];
	int posIndex;
	int charId;

	for (posIndex = 0; posIndex < 4; ++posIndex) {
		int charId = xeenSideChars[charIndex];
		bool isInParty = _vm->_party.isInParty(charId);

		if (charId == 0xff) {
			while ((int)_buttons.size() > (7 + posIndex))
				_buttons.remove_at(_buttons.size() - 1);
			break;
		}

		Common::Rect &b = _buttons[7 + posIndex]._bounds;
		b.moveTo((posIndex & 1) ? 117 : 16, b.top);
		PlayerStruct &ps = _vm->_roster[xeenSideChars[charIndex + posIndex]];
		playerNames[posIndex] = isInParty ? IN_PARTY : ps._name;
		playerRaces[posIndex] = RACE_NAMES[ps._race];
		playerSex[posIndex] = SEX_NAMES[ps._sex];
		playerClass[posIndex] = CLASS_NAMES[ps._class];
	}

	charIconsPrint(updateFlag);

	// Set up the sprite set to use for each face
	charId = xeenSideChars[charIndex];
	_faceDrawStructs[0]._sprites = (charId == 0xff) ? (SpriteResource *)nullptr : &_charFaces[charId];
	charId = xeenSideChars[charIndex + 1];
	_faceDrawStructs[1]._sprites = (charId == 0xff) ? (SpriteResource *)nullptr : &_charFaces[charId];
	charId = xeenSideChars[charIndex + 2];
	_faceDrawStructs[2]._sprites = (charId == 0xff) ? (SpriteResource *)nullptr : &_charFaces[charId];
	charId = xeenSideChars[charIndex + 3];
	_faceDrawStructs[3]._sprites = (charId == 0xff) ? (SpriteResource *)nullptr : &_charFaces[charId];

	_interfaceText = Common::String::format(PARTY_DETAILS,
		playerNames[0].c_str(), playerRaces[0].c_str(), playerSex[0].c_str(), playerClass[0].c_str(),
		playerNames[1].c_str(), playerRaces[1].c_str(), playerSex[1].c_str(), playerClass[1].c_str(),
		playerNames[2].c_str(), playerRaces[2].c_str(), playerSex[2].c_str(), playerClass[2].c_str(),
		playerNames[3].c_str(), playerRaces[3].c_str(), playerSex[3].c_str(), playerClass[3].c_str()
	);
}

void Interface::charIconsPrint(bool updateFlag) {
	Screen &screen = *_vm->_screen;
	bool stateFlag = _vm->_mode == MODE_2;
	_restoreSprites.draw(screen, 0, Common::Point(8, 149));

	// Handle drawing the party faces
	for (int idx = 0; idx < (stateFlag ? _vm->_party._combatPartyCount : 
			_vm->_party._partyCount); ++idx) {
		int charIndex = stateFlag ? _combatCharIds[idx] : idx;
		PlayerStruct &ps = _vm->_party._activeParty[charIndex];
		Condition charCondition = ps.findCondition();
		int charFrame = FACE_CONDITION_FRAMES[charCondition];
		
		SpriteResource *sprites = (charFrame > 4 && !_charFaces[0].empty()) ?
			&_dseFace : _partyFaces[charIndex];
		if (charFrame > 4)
			charFrame -= 5;

		sprites->draw(screen, charFrame, Common::Point(CHAR_FACES_X[idx], 150));
	}

	if (!_hpSprites.empty()) {
		for (int idx = 0; idx < (stateFlag ? _vm->_party._combatPartyCount :
			_vm->_party._partyCount); ++idx) {
			int charIndex = stateFlag ? _combatCharIds[idx] : idx;
			PlayerStruct &ps = _vm->_party._activeParty[charIndex];

			// Draw the Hp bar
			int maxHp = ps.getMaxHp();
			int frame;
			if (ps._currentHp < 1)
				frame = 4;
			else if (ps._currentHp > maxHp)
				frame = 3;
			else if (ps._currentHp == maxHp)
				frame = 0;
			else if (ps._currentHp < (maxHp / 4))
				frame = 2;
			else
				frame = 1;

			_hpSprites.draw(screen, frame, Common::Point(HP_BARS_X[idx], 182));
		}
	}

	if (_hiliteChar != -1)
		_globalSprites.draw(screen, 8, Common::Point(CHAR_FACES_X[_hiliteChar] - 1, 149));

	if (updateFlag)
		screen._windows[33].update();
}

void Interface::drawViewBackground(int bgType) {
	if (bgType >= 4)
		return;

	if (bgType == 0) {
		// Totally black background
		_vm->_screen->fillRect(Common::Rect(8, 8, 224, 140), 0);
	} else {
		const byte *lookup = BACKGROUND_XLAT + bgType;
		for (int yp = 8; yp < 140; ++yp) {
			byte *destP = (byte *)_vm->_screen->getBasePtr(8, yp);
			for (int xp = 8; xp < 224; ++xp, ++destP)
				*destP = lookup[*destP];
		}
	}
}

void Interface::addCharacterToRoster() {
	error("TODO");
}

void Interface::draw3d(bool flag) {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;

	if (!screen._windows[11]._enabled)
		return;

	warning("TODO");
}

void Interface::animate3d() {

}

void Interface::setMonsters() {

}

void Interface::setObjects() {

}

void Interface::setOutdoorsMonsters() {

}

void Interface::setOutdoorsObjects() {

}

void Interface::startup() {
	Screen &screen = *_vm->_screen;
	loadCharIcons();
	_iconSprites.load("main.icn");

	animate3d();
	if (_vm->_map->_isOutdoors) {
		setMonsters();
		setObjects();
	} else {
		setOutdoorsMonsters();
		setOutdoorsObjects();
	}
	draw3d(false);

	_globalSprites.draw(screen._windows[1], 5, Common::Point(232, 9));
	charIconsPrint(false);

	_mainList[0]._sprites = &_globalSprites;
	for (int i = 1; i < 16; ++i)
		_mainList[i]._sprites = &_iconSprites;
}

void Interface::mainIconsPrint() {
	Screen &screen = *_vm->_screen;
	screen._windows[38].close();
	screen._windows[12].close();
	screen._windows[0].drawList(_mainList, 16);
	screen._windows[34].update();
}

} // End of namespace Xeen
