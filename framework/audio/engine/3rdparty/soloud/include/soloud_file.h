/*
SoLoud audio engine
Copyright (c) 2013-2015 Jari Komppa

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.
*/

#ifndef SOLOUD_FILE_H
#define SOLOUD_FILE_H

#include <stdio.h>
#include "soloud.h"

typedef void* Soloud_Filehack;

namespace SoLoud
{
	class File
	{
	public:
		virtual ~File() {}
		unsigned int read8();
		unsigned int read16();
		unsigned int read32();
		virtual int eof() = 0;
		virtual unsigned int read(unsigned char *aDst, unsigned int aBytes) = 0;
		virtual unsigned int length() = 0;
		virtual void seek(int aOffset) = 0;
		virtual unsigned int pos() = 0;
		virtual FILE * getFilePtr() { return 0; }
		virtual const unsigned char * getMemPtr() { return 0; }
	};

	class DiskFile : public File
	{
	public:
		FILE *mFileHandle;

		virtual int eof();
		virtual unsigned int read(unsigned char *aDst, unsigned int aBytes);
		virtual unsigned int length();
		virtual void seek(int aOffset);
		virtual unsigned int pos();
		virtual ~DiskFile();
		DiskFile();
		DiskFile(FILE *fp);
		result open(const char *aFilename);
		virtual FILE * getFilePtr();
	};

	class MemoryFile : public File
	{
	public:
		const unsigned char *mDataPtr;
		unsigned int mDataLength;
		unsigned int mOffset;
		bool mDataOwned;

		virtual int eof();
		virtual unsigned int read(unsigned char *aDst, unsigned int aBytes);
		virtual unsigned int length();
		virtual void seek(int aOffset);
		virtual unsigned int pos();
		virtual const unsigned char * getMemPtr();
		virtual ~MemoryFile();
		MemoryFile();
		result openMem(const unsigned char *aData, unsigned int aDataLength, bool aCopy=false, bool aTakeOwnership=true);
		result openToMem(const char *aFilename);
		result openFileToMem(File *aFile);
	};
};

#endif
