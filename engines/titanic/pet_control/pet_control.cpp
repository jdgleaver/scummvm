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

#include "titanic/pet_control/pet_control.h"
#include "titanic/core/project_item.h"
#include "titanic/game_manager.h"
#include "titanic/game_state.h"

namespace Titanic {

CPetControl::CPetControl() : CGameObject(), 
	_currentArea(PET_CONVERSATION), _fieldC0(0), _locked(0), _fieldC8(0),
	_treeItem1(nullptr), _treeItem2(nullptr), _hiddenRoom(nullptr) {
}

void CPetControl::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(0, indent);
	file->writeNumberLine(_currentArea, indent);
	file->writeQuotedLine(_string1, indent);
	file->writeQuotedLine(_string2, indent);

	saveAreas(file, indent);
	CGameObject::save(file, indent);
}

void CPetControl::load(SimpleFile *file) {
	int val = file->readNumber();
	isValid();
	
	if (!val) {
		_currentArea = (PetArea)file->readNumber();
		_string1 = file->readString();
		_string2 = file->readString();
		
		loadAreas(file, 0);
	}

	CGameObject::load(file);
}

bool CPetControl::isValid() {
	return _convSection.isValid(this) &&
		_roomsSection.isValid(this) && 
		_remoteSection.isValid(this) &&
		_invSection.isValid(this) &&
		_sub5.isValid(this) &&
		_saveSection.isValid(this) &&
		_sub7.isValid(this) &&
		_frame.isValid(this);
}

void CPetControl::loadAreas(SimpleFile *file, int param) {
	_convSection.load(file, param);
	_roomsSection.load(file, param);
	_remoteSection.load(file, param);
	_invSection.load(file, param);
	_sub5.load(file, param);
	_saveSection.load(file, param);
	_sub7.load(file, param);
	_frame.load(file, param);
}

void CPetControl::saveAreas(SimpleFile *file, int indent) const {
	_convSection.save(file, indent);
	_roomsSection.save(file, indent);
	_remoteSection.save(file, indent);
	_invSection.save(file, indent);
	_sub5.save(file, indent);
	_saveSection.save(file, indent);
	_sub7.save(file, indent);
	_frame.save(file, indent);
}

void CPetControl::draw(CScreenManager *screenManager) {
	CGameManager *gameManager = getGameManager();
	Rect bounds = _oldBounds;
	bounds.constrain(gameManager->_bounds);

	if (!bounds.isEmpty()) {
		if (_fieldC8 >= 0) {
			_invSection.proc5(_fieldC8);
			_fieldC8 = -1;
		}

		_frame.drawFrame(screenManager);

		// Draw the specific area that's currently active
		switch (_currentArea) {
		case PET_INVENTORY:
			_invSection.draw(screenManager);
			break;
		case PET_CONVERSATION:
			_convSection.draw(screenManager);
			break;
		case PET_REMOTE:
			_remoteSection.draw(screenManager);
			break;
		case PET_ROOMS:
			_roomsSection.draw(screenManager);
			break;
		case PET_SAVE:
			_saveSection.draw(screenManager);
			break;
		case PET_5:
			_sub5.draw(screenManager);
			break;
		case PET_6:
			_sub7.draw(screenManager);
			break;
		default:
			break;
		}
	}
}

void CPetControl::postLoad() {
	CProjectItem *root = getRoot();

	if (!_string1.empty() && root)
		_treeItem1 = root->findByName(_string1);
	if (!_string2.empty() && root)
		_treeItem2 = root->findByName(_string2);

	setArea(_currentArea);
	loaded();
}

void CPetControl::loaded() {
	_convSection.postLoad();
	_roomsSection.postLoad();
	_remoteSection.postLoad();
	_invSection.postLoad();
	_sub5.postLoad();
	_saveSection.postLoad();
	_sub7.postLoad();
	_frame.postLoad();
}

void CPetControl::enterNode(CNodeItem *node) {
	getGameManager()->_gameState.enterNode();
}

void CPetControl::enterRoom(CRoomItem *room) {
	_roomsSection.enterRoom(room);
	_remoteSection.enterRoom(room);
}

void CPetControl::clear() {
	_treeItem2 = nullptr;
	_string2.clear();
}

bool CPetControl::fn1(int val) {
	warning("TODO: CPetControl::fn1");
	return false;
}

void CPetControl::fn4() {
	warning("TODO: CPetControl::fn4");
}

PetArea CPetControl::setArea(PetArea newArea) {
	if (newArea == _currentArea || !canChangeArea())
		return _currentArea;

	// Signal the currently active area that it's being left
	switch (_currentArea) {
	case PET_INVENTORY:
		_invSection.leave();
		break;
	case PET_CONVERSATION:
		_convSection.leave();
		break;
	case PET_REMOTE:
		_remoteSection.leave();
		break;
	case PET_ROOMS:
		_roomsSection.leave();
		break;
	case PET_SAVE:
		_saveSection.leave();
		break;
	case PET_5:
		_sub5.leave();
		break;
	case PET_6:
		_sub7.leave();
		break;
	default:
		break;
	}

	// Change the current area
	PetArea oldArea = _currentArea;
	_frame.setArea(newArea);
	_currentArea = newArea;

	// Signal to the new view that it's been activated
	switch (newArea) {
	case PET_INVENTORY:
		_invSection.enter(oldArea);
		
		break;
	case PET_CONVERSATION:
		_convSection.enter(oldArea);
		break;
	case PET_REMOTE:
		_remoteSection.enter(oldArea);
		break;
	case PET_ROOMS:
		_roomsSection.enter(oldArea);
		break;
	case PET_SAVE:
		_saveSection.enter(oldArea);
		break;
	case PET_5:
		_sub5.enter(oldArea);
		break;
	case PET_6:
		_sub7.enter(oldArea);
		break;
	default:
		break;
	}

	makeDirty();
	return newArea;
}

void CPetControl::fn2(int val) {
	switch (_currentArea) {
	case PET_INVENTORY:
		_invSection.proc38(val);
		break;
	case PET_CONVERSATION:
		_convSection.proc38(val);
		break;
	case PET_REMOTE:
		_remoteSection.proc38(val);
		break;
	case PET_ROOMS:
		_roomsSection.proc38(val);
		break;
	case PET_SAVE:
		_saveSection.proc38(val);
		break;
	case PET_5:
		_sub5.proc38(val);
		break;
	case PET_6:
		_sub7.proc38(val);
		break;
	default:
		break;
	}
}

void CPetControl::fn3(CTreeItem *item) {
	_treeItem2 = item;
	if (item)
		_string2 = item->getName();
	else
		_string2.clear();
}

CRoomItem *CPetControl::getHiddenRoom() {
	if (!_hiddenRoom)
		_hiddenRoom = CTreeItem::getHiddenRoom();

	return _hiddenRoom;
}

CGameObject *CPetControl::findItemInRoom(CRoomItem *room, 
		const CString &name) const {
	if (!room)
		return nullptr;

	for (CTreeItem *treeItem = room->getFirstChild(); treeItem;
			treeItem = treeItem->scan(room)) {
		if (!treeItem->getName().compareTo(name)) {
			return dynamic_cast<CGameObject *>(treeItem);
		}
	}

	return nullptr;
}

CGameObject *CPetControl::getHiddenObject(const CString &name) {
	CRoomItem *room = getHiddenRoom();
	return room ? findItemInRoom(room, name) : nullptr;
}

} // End of namespace Titanic
