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

#undef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include "soloud.h"
#include "soloud_file.h"

namespace SoLoud
{
	unsigned int File::read8()
	{
		unsigned char d = 0;
		read((unsigned char*)&d, 1);
		return d;
	}

	unsigned int File::read16()
	{
		unsigned short d = 0;
		read((unsigned char*)&d, 2);
		return d;
	}

	unsigned int File::read32()
	{
		unsigned int d = 0;
		read((unsigned char*)&d, 4);
		return d;
	}

DiskFile::DiskFile(FILE *fp):
mFileHandle(fp)
{

}

	unsigned int DiskFile::read(unsigned char *aDst, unsigned int aBytes)
	{
		return (unsigned int)fread(aDst, 1, aBytes, mFileHandle);
	}

	unsigned int DiskFile::length()
	{
		if (!mFileHandle)
			return 0;
		unsigned int pos = (unsigned int)ftell(mFileHandle);
		fseek(mFileHandle, 0, SEEK_END);
		unsigned int len = (unsigned int)ftell(mFileHandle);
		fseek(mFileHandle, pos, SEEK_SET);
		return len;
	}

	void DiskFile::seek(int aOffset)
	{
		fseek(mFileHandle, aOffset, SEEK_SET);
	}

	unsigned int DiskFile::pos()
	{
		return (unsigned int)ftell(mFileHandle);
	}

	FILE *DiskFile::getFilePtr()
	{
		return mFileHandle;
	}

	DiskFile::~DiskFile()
	{
		if (mFileHandle)
			fclose(mFileHandle);
	}

	DiskFile::DiskFile()
	{
		mFileHandle = 0;
	}

	result DiskFile::open(const char *aFilename)
	{
		if (!aFilename)
			return INVALID_PARAMETER;
		mFileHandle = fopen(aFilename, "rb");
		if (!mFileHandle)
			return FILE_NOT_FOUND;
		return SO_NO_ERROR;
	}

	int DiskFile::eof()
	{
		return feof(mFileHandle);
	}



	unsigned int MemoryFile::read(unsigned char *aDst, unsigned int aBytes)
	{
		if (mOffset + aBytes >= mDataLength)
			aBytes = mDataLength - mOffset;

		memcpy(aDst, mDataPtr + mOffset, aBytes);
		mOffset += aBytes;

		return aBytes;
	}

	unsigned int MemoryFile::length()
	{
		return mDataLength;
	}

	void MemoryFile::seek(int aOffset)
	{
		if (aOffset >= 0)
			mOffset = aOffset;
		else
			mOffset = mDataLength + aOffset;
		if (mOffset > mDataLength-1)
			mOffset = mDataLength-1;
	}

	unsigned int MemoryFile::pos()
	{
		return mOffset;
	}

	const unsigned char * MemoryFile::getMemPtr()
	{
		return mDataPtr;
	}

	MemoryFile::~MemoryFile()
	{
		if (mDataOwned)
			delete[] mDataPtr;
	}

	MemoryFile::MemoryFile()
	{
		mDataPtr = 0;
		mDataLength = 0;
		mOffset = 0;
		mDataOwned = false;
	}

	result MemoryFile::openMem(const unsigned char *aData, unsigned int aDataLength, bool aCopy, bool aTakeOwnership)
	{
		if (aData == NULL || aDataLength == 0)
			return INVALID_PARAMETER;

		if (mDataOwned)
			delete[] mDataPtr;
		mDataPtr = 0;
		mOffset = 0;

		mDataLength = aDataLength;

		if (aCopy)
		{
			mDataOwned = true;
			mDataPtr = new unsigned char[aDataLength];
			if (mDataPtr == NULL)
				return OUT_OF_MEMORY;
			memcpy((void *)mDataPtr, aData, aDataLength);
			return SO_NO_ERROR;
		}

		mDataPtr = aData;
		mDataOwned = aTakeOwnership;
		return SO_NO_ERROR;
	}

	result MemoryFile::openToMem(const char *aFile)
	{
		if (!aFile)
			return INVALID_PARAMETER;
		if (mDataOwned)
			delete[] mDataPtr;
		mDataPtr = 0;
		mOffset = 0;

		DiskFile df;
		int res = df.open(aFile);
		if (res != SO_NO_ERROR)
			return res;

		mDataLength = df.length();
		mDataPtr = new unsigned char[mDataLength];
		if (mDataPtr == NULL)
			return OUT_OF_MEMORY;
		df.read((unsigned char*)mDataPtr, mDataLength);
		mDataOwned = true;
		return SO_NO_ERROR;
	}

	result MemoryFile::openFileToMem(File *aFile)
	{
		if (!aFile)
			return INVALID_PARAMETER;
		if (mDataOwned)
			delete[] mDataPtr;
		mDataPtr = 0;
		mOffset = 0;

		mDataLength = aFile->length();
		mDataPtr = new unsigned char[mDataLength];
		if (mDataPtr == NULL)
			return OUT_OF_MEMORY;
		aFile->read((unsigned char*)mDataPtr, mDataLength);
		mDataOwned = true;
		return SO_NO_ERROR;
	}

	int MemoryFile::eof()
	{
		if (mOffset >= mDataLength)
			return 1;
		return 0;
	}
}

extern "C"
{
	int Soloud_Filehack_fgetc(Soloud_Filehack *f)
	{
		SoLoud::File *fp = (SoLoud::File *)f;
		if (fp->eof())
			return EOF;
		return fp->read8();
	}

	int Soloud_Filehack_fread(void *dst, int s, int c, Soloud_Filehack *f)
	{
		SoLoud::File *fp = (SoLoud::File *)f;
		return fp->read((unsigned char*)dst, s*c) / s;

	}

	int Soloud_Filehack_fseek(Soloud_Filehack *f, int idx, int base)
	{
		SoLoud::File *fp = (SoLoud::File *)f;
		switch (base)
		{
		case SEEK_CUR:
			fp->seek(fp->pos() + idx);
			break;
		case SEEK_END:
			fp->seek(fp->length() + idx);
			break;
		default:
			fp->seek(idx);
		}
		return 0;
	}

	int Soloud_Filehack_ftell(Soloud_Filehack *f)
	{
		SoLoud::File *fp = (SoLoud::File *)f;
		return fp->pos();
	}

	int Soloud_Filehack_fclose(Soloud_Filehack *f)
	{
		SoLoud::File *fp = (SoLoud::File *)f;
		delete fp;
		return 0;
	}

	Soloud_Filehack * Soloud_Filehack_fopen(const char *aFilename, char * /*aMode*/)
	{
		SoLoud::DiskFile *df = new SoLoud::DiskFile();
		int res = df->open(aFilename);
		if (res != SoLoud::SO_NO_ERROR)
		{
			delete df;
			df = 0;
		}
		return (Soloud_Filehack*)df;
	}

	int Soloud_Filehack_fopen_s(Soloud_Filehack** f, const char* aFilename, char* /*aMode*/)
	{
		*f = Soloud_Filehack_fopen(aFilename, 0);
		return 1;
	}
}
