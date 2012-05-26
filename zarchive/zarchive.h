//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __ZARCHIVE_H__
#define __ZARCHIVE_H__

class QIODevice;
class QFile;
class QDir;
class QStringList;
class QString;

#define BUFFER_SIZE (256*1024)

class ZipEntryP;

//---------------------------------------------------------
//   ZArchive
//---------------------------------------------------------

class ZArchive
      {
   public:
      enum ErrorCode {
            Ok,
            ZlibInit,
            ZlibError,
            FileExists,
            OpenFailed,
            NoOpenArchive,
            FileNotFound,
            ReadFailed,
            WriteFailed,
            SeekFailed,
            PartiallyCorrupted,
            Corrupted,
            CreateDirFailed,
            InvalidDevice,
            InvalidArchive,
            HeaderConsistencyError,
            Skip, SkipAll // internal use only
            };
   protected:
      ErrorCode lastError;

   public:
	QString errorString() const;
      };

//---------------------------------------------------------
//   Zip
//---------------------------------------------------------

class Zip : public ZArchive
      {
	QIODevice* device;
	QMap<QString, ZipEntryP*>* headers;

	char buffer1[BUFFER_SIZE];
	char buffer2[BUFFER_SIZE];

	uchar* uBuffer;
	const quint32* crcTable;

	void reset();
	bool zLibInit();
	void setULong(quint32 v, char* buffer, uint offset);

   public:
	Zip();
	~Zip();

	bool createArchive(const QString& file);
	bool createArchive(QIODevice* device);
	bool createEntry(const QString& entryName, QIODevice& actualFile, QDateTime dt, bool dirOnly = false, bool isPNGFile = false, int level = 9);
	bool closeArchive();
      };

//---------------------------------------------------------
//   Unzip
//---------------------------------------------------------

class Unzip : public ZArchive
      {
   public:
	enum CompressionMethod {
		NoCompression, Deflated, UnknownCompression
            };

	enum FileType {
		File, Directory
            };
	struct ZipEntry {
		QString filename;

		quint32 compressedSize;
		quint32 uncompressedSize;
		quint32 crc32;

		QDateTime lastModified;

		CompressionMethod compression;
		FileType type;

		ZipEntry();
	      };
   private:
	QMap<QString, ZipEntryP*>* headers;
	QIODevice* device;

	char buffer1[BUFFER_SIZE];
	char buffer2[BUFFER_SIZE];

	uchar* uBuffer;
	const quint32* crcTable;

	quint32 cdOffset;       // Central Directory (CD) offset
	quint32 eocdOffset;     // End of Central Directory (EOCD) offset

	// Number of entries in the Central Directory (as to the EOCD record)
	quint16 cdEntryCount;

	// The number of detected entries that have been skipped because of a non compatible format
	quint16 unsupportedEntryCount;

	QStringList fileList() const;
	QList<ZipEntry> entryList() const;

	ErrorCode seekToCentralDirectory();
	ErrorCode parseCentralDirectoryRecord();
	ErrorCode parseLocalHeaderRecord(const QString& path, ZipEntryP& entry);

	bool extractFile(const QString& path, ZipEntryP& entry, QIODevice* device);
	bool createDirectory(const QString& path);

	quint32 getULong(const uchar* data, quint32 offset) const;
	quint64 getULLong(const uchar* data, quint32 offset) const;
	quint16 getUShort(const uchar* data, quint32 offset) const;

	QDateTime convertDateTime(const uchar date[2], const uchar time[2]) const;

   public:
	Unzip();
	~Unzip();

	bool openArchive(const QString& filename);
	bool openArchive(QIODevice* device);
	void closeArchive();
	bool contains(const QString& file) const;
	bool extractFile(const QString& filename, QIODevice* device);
      };

#endif

