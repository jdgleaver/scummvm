/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "common/savefile.h"
#include "palm.h"

// SaveFile class

class OldPalmSaveFile : public Common::SaveFile {
private:
	FILE *fh;
public:
	OldPalmSaveFile(const char *filename, bool saveOrLoad) {
		fh = ::fopen(filename, (saveOrLoad? "wb" : "rb"));
	}
	~OldPalmSaveFile() {
		if (fh) ::fclose(fh);
	}
	
	bool eos() const { return feof(fh) != 0; }
	bool ioFailed() const { return ferror(fh) != 0; }
	void clearIOFailed() { clearerr(fh); }

	bool isOpen() const { return fh != NULL; }

	uint32 read(void *buf, uint32 size);
	uint32 write(const void *buf, uint32 size);
	
	void skip(uint32 offset) {
		::fseek(fh, offset, SEEK_CUR);
	}
};

uint32 OldPalmSaveFile::read(void *buf, uint32 size) {
	// we must return the size, where fread return nitems upon success ( 1 <=> size)
	if (fh) return (::fread(buf, 1, size, fh));
	return 0;
}

uint32 OldPalmSaveFile::write(const void *buf, uint32 size) {
	// we must return the size, where fwrite return nitems upon success ( 1 <=> size)
	if (fh) return ::fwrite(buf, 1, size, fh);
	return 0;
}





// SaveFileManager class

class OldPalmSaveFileManager : public Common::SaveFileManager {
public:
	virtual Common::OutSaveFile *openForSaving(const char *filename) {
		return openSavefile(filename, true);
	}
	virtual Common::InSaveFile *openForLoading(const char *filename) {
		return openSavefile(filename, false);
	}

	Common::SaveFile *openSavefile(const char *filename, bool saveOrLoad);
	void listSavefiles(const char *prefix, bool *marks, int num);

protected:
	Common::SaveFile *makeSaveFile(const char *filename, bool saveOrLoad);
};

Common::SaveFile *OldPalmSaveFileManager::openSavefile(const char *filename, bool saveOrLoad) {
	char buf[256];

	strncpy(buf, getSavePath(), sizeof(buf));
	strncat(buf, filename, sizeof(buf));

	return makeSaveFile(buf, saveOrLoad);
}

void OldPalmSaveFileManager::listSavefiles(const char *prefix, bool *marks, int num) {
	FileRef fileRef;
	// try to open the dir
	Err e = VFSFileOpen(gVars->VFS.volRefNum, getSavePath(), vfsModeRead, &fileRef);
	memset(marks, false, num*sizeof(bool));

	if (e != errNone)
		return;

	// enumerate all files
	UInt32 dirEntryIterator = vfsIteratorStart;
	Char filename[32];
	FileInfoType info = {0, filename, 32};
	UInt16 length = StrLen(prefix);
	int slot = 0;

	while (dirEntryIterator != vfsIteratorStop) {
		e = VFSDirEntryEnumerate (fileRef, &dirEntryIterator, &info);

		if (e != expErrEnumerationEmpty) {										// there is something

			if (StrLen(info.nameP) == (length + 2)) {						// consider max 99, filename length is ok
				if (StrNCaselessCompare(prefix, info.nameP, length) == 0) { // this seems to be a save file
					if (isdigit(info.nameP[length]) && isdigit(info.nameP[length+1])) {

						slot = StrAToI(filename + length);
						if (slot >= 0 && slot < num)
							*(marks+slot) = true;

					}
				}
			}

		}
	}

	VFSFileClose(fileRef);
}

Common::SaveFile *OldPalmSaveFileManager::makeSaveFile(const char *filename, bool saveOrLoad) {
	OldPalmSaveFile *sf = new OldPalmSaveFile(filename, saveOrLoad);

	if (!sf->isOpen()) {
		delete sf;
		sf = 0;
	}
	return sf;
}

// OSystem
Common::SaveFileManager *OSystem_PALMOS::getSavefileManager() {
	return new OldPalmSaveFileManager();
}
