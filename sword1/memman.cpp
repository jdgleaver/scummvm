/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "memman.h"
#include "common/util.h"

MemMan::MemMan(void) {
	_alloced = 0;
	_memListFree = _memListFreeEnd = NULL;
}

MemMan::~MemMan(void) {
}

void MemMan::alloc(BsMemHandle *bsMem, uint32 pSize, uint16 pCond) {
	_alloced += pSize;
	bsMem->data = (void*)malloc(pSize);
	if (!bsMem->data)
		error("MemMan::alloc(): Can't alloc %d bytes of memory.", pSize);
	bsMem->cond = pCond;
	bsMem->size = pSize;
	if (pCond == MEM_CAN_FREE) {
		warning("%d Bytes alloced as FREEABLE.", pSize); // why should one want to alloc mem if it can be freed?
		addToFreeList(bsMem);
	} else if (bsMem->next || bsMem->prev) // it's in our _freeAble list, remove it from there
        removeFromFreeList(bsMem);
	checkMemoryUsage();
}

void MemMan::freeNow(BsMemHandle *bsMem) {
	if (bsMem->cond != MEM_FREED) {
		_alloced -= bsMem->size;
		removeFromFreeList(bsMem);
		free(bsMem->data);
		bsMem->cond = MEM_FREED;
	}
}

void MemMan::setCondition(BsMemHandle *bsMem, uint16 pCond) {
	if ((pCond == MEM_FREED) || (pCond > MEM_DONT_FREE))
		error("MemMan::setCondition: program tried to set illegal memory condition");
	if (bsMem->cond != pCond) {
		bsMem->cond = pCond;
		if (pCond == MEM_DONT_FREE)
			removeFromFreeList(bsMem);
		else if (pCond == MEM_CAN_FREE)
			addToFreeList(bsMem);
	}
}

void MemMan::checkMemoryUsage(void) {
	while ((_alloced > MAX_ALLOC) && _memListFree) {
		free(_memListFreeEnd->data);
		_memListFreeEnd->data = NULL;
		_memListFreeEnd->cond = MEM_FREED;
		_alloced -= _memListFreeEnd->size;
		removeFromFreeList(_memListFreeEnd);
	}
}

void MemMan::addToFreeList(BsMemHandle *bsMem) {
	if (bsMem->next || bsMem->prev) {
		warning("addToFreeList: mem block is already in freeList");
		return;
	}
	bsMem->prev = NULL;
	bsMem->next = _memListFree;
	if (bsMem->next)
		bsMem->next->prev = bsMem;
	_memListFree = bsMem;
	if (!_memListFreeEnd)
		_memListFreeEnd = _memListFree;
}

void MemMan::removeFromFreeList(BsMemHandle *bsMem) {
	/*BsMemHandle *forw = _memListFree;
	BsMemHandle *rev = _memListFreeEnd;

	while (forw || rev) {
		if (!(forw && rev))
			error("mem list is completely fubared");
		printf("%p <-> %p\n", forw, rev);
		forw = forw->next;
		rev = rev->prev;
	}
	printf("\n");*/
	if (!(bsMem->prev || bsMem->next))
		warning("removeFromFreeList: memory block wasn't in list");
	if (_memListFree == bsMem)
		_memListFree = bsMem->next;
	if (_memListFreeEnd == bsMem)
		_memListFreeEnd = bsMem->prev;

	if (bsMem->next)
		bsMem->next->prev = bsMem->prev;
	if (bsMem->prev)
		bsMem->prev->next = bsMem->next;
	bsMem->next = bsMem->prev = NULL;
}

void MemMan::initHandle(BsMemHandle *bsMem) {
	memset(bsMem, 0, sizeof(BsMemHandle));
}

