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

#include "zarchive.h"

#include <zlib.h>

//---------------------------------------------------------
//   ZipEntryP
//---------------------------------------------------------

class ZipEntryP
      {
   public:
      ZipEntryP();

	quint32 lhOffset;		// Offset of the local header record for this entry
	quint32 dataOffset;	// Offset of the file data for this entry
	uchar   gpFlag[2];	// General purpose flag
	quint16 compMethod;	// Compression method
	uchar   modTime[2];	// Last modified time
	uchar   modDate[2];	// Last modified date
	quint32 crc;		// CRC32
	quint32 szComp;		// Compressed file size
	quint32 szUncomp;		// Uncompressed file size
	QString comment;		// File comment

	bool lhEntryChecked;	// Is true if the local header record for this entry has been parsed
	bool hasDataDescriptor() const { return gpFlag[0] & 0x08; }
      };

#define ZIP_LOCAL_HEADER_SIZE     30  // Local header size (including signature, excluding variable length fields)
#define ZIP_DD_SIZE_WS            16  // Data descriptor size (signature included)
#define ZIP_CD_SIZE               46  // Central Directory record size (signature included)
#define ZIP_EOCD_SIZE             22  // End of Central Directory record size (signature included)

// Some offsets inside a local header record (signature included)
#define ZIP_LH_OFF_VERS       4
#define ZIP_LH_OFF_GPFLAG     6
#define ZIP_LH_OFF_CMET       8
#define ZIP_LH_OFF_MODT       10
#define ZIP_LH_OFF_MODD       12
#define ZIP_LH_OFF_CRC        14
#define ZIP_LH_OFF_CSIZE      18
#define ZIP_LH_OFF_USIZE      22
#define ZIP_LH_OFF_NAMELEN    26
#define ZIP_LH_OFF_XLEN       28

// Some offsets inside a data descriptor record (including signature)
#define ZIP_DD_OFF_CRC32      4
#define ZIP_DD_OFF_CSIZE      8
#define ZIP_DD_OFF_USIZE      12

// Some offsets inside a Central Directory record (including signature)
#define ZIP_CD_OFF_MADEBY     4
#define ZIP_CD_OFF_VERSION    6
#define ZIP_CD_OFF_GPFLAG     8
#define ZIP_CD_OFF_CMET       10
#define ZIP_CD_OFF_MODT       12
#define ZIP_CD_OFF_MODD       14
#define ZIP_CD_OFF_CRC        16
#define ZIP_CD_OFF_CSIZE      20
#define ZIP_CD_OFF_USIZE      24
#define ZIP_CD_OFF_NAMELEN    28
#define ZIP_CD_OFF_XLEN       30
#define ZIP_CD_OFF_COMMLEN    32
#define ZIP_CD_OFF_DISKSTART  34
#define ZIP_CD_OFF_IATTR      36
#define ZIP_CD_OFF_EATTR      38
#define ZIP_CD_OFF_LHOFF      42

// Some offsets inside a EOCD record (including signature)
#define ZIP_EOCD_OFF_DISKNUM        4
#define ZIP_EOCD_OFF_CDDISKNUM      6
#define ZIP_EOCD_OFF_ENTRIES        8
#define ZIP_EOCD_OFF_CDENTRIES      10
#define ZIP_EOCD_OFF_CDSIZE         12
#define ZIP_EOCD_OFF_CDOFF          16
#define ZIP_EOCD_OFF_COMMLEN        20

//! PKZip version for archives created by this API
#define ZIP_VERSION                 0x14

//! This macro updates a one-char-only CRC; it's the Info-Zip macro re-adapted
#define CRC32(c, b) crcTable[((int)c^b) & 0xff] ^ (c >> 8)

//---------------------------------------------------------
//   errorString
//---------------------------------------------------------

#define TR(x)  QCoreApplication::translate("ZArchive", x)

QString ZArchive::errorString() const
      {
	switch (lastError) {
	      case Ok:                     return TR("ZIP operation completed successfully."); break;
	      case ZlibInit:               return TR("Failed to initialize or load zlib library."); break;
	      case ZlibError:              return TR("zlib library error."); break;
	      case OpenFailed:             return TR("Unable to create or open file."); break;
	      case NoOpenArchive:          return TR("No archive has been created yet."); break;
	      case FileNotFound:           return TR("File or directory does not exist."); break;
	      case ReadFailed:             return TR("File read error."); break;
	      case WriteFailed:            return TR("File write error."); break;
	      case SeekFailed:             return TR("File seek error."); break;
	      case PartiallyCorrupted:     return TR("Partially corrupted archive. Some files might be extracted."); break;
	      case Corrupted:              return TR("Corrupted archive."); break;
	      case CreateDirFailed:        return TR("Unable to create a directory."); break;
	      case InvalidDevice:          return TR("Invalid device."); break;
	      case InvalidArchive:         return TR("Invalid or incompatible zip archive."); break;
	      case HeaderConsistencyError: return TR("Inconsistent headers. Archive might be corrupted."); break;
	      default:
                  break;
	      }
      return QCoreApplication::translate("Unzip", "Unknown error.");
      }
#undef TR

//---------------------------------------------------------
//   ZipEntryP
//---------------------------------------------------------

ZipEntryP::ZipEntryP()
      {
      lhOffset       = 0;
      dataOffset     = 0;
	gpFlag[0]      = gpFlag[1] = 0;
	compMethod     = 0;
	modTime[0]     = modTime[1] = 0;
	modDate[0]     = modDate[1] = 0;
	crc            = 0;
	szComp         = 0;
	szUncomp       = 0;
	lhEntryChecked = false;
	}

//---------------------------------------------------------
//   Zip
//---------------------------------------------------------

Zip::Zip()
      {
	headers = 0;
	device  = 0;

	// keep an unsigned pointer so we avoid to over bloat the code with casts
	uBuffer  = (uchar*) buffer1;
	crcTable = (quint32*) get_crc_table();
	memset(buffer1, 0, sizeof(buffer1));
	memset(buffer2, 0, sizeof(buffer2));
      }

Zip::~Zip()
      {
	closeArchive();
      }

//---------------------------------------------------------
//   createArchive
//---------------------------------------------------------

bool Zip::createArchive(const QString& filename)
      {
	QFile* file = new QFile(filename);

	if (!file->open(QIODevice::WriteOnly)) {
		delete file;
		lastError = OpenFailed;
            return false;
	      }
	if (!createArchive(file)) {
		file->remove();
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   createArchive
//---------------------------------------------------------

bool Zip::createArchive(QIODevice* dev)
      {
	if (device != 0)
		closeArchive();

	device = dev;

	if (!device->isOpen()) {
		if (!device->open(QIODevice::ReadOnly)) {
			delete device;
			device = 0;
			lastError = OpenFailed;
                  return false;
		      }
	      }
	headers = new QMap<QString,ZipEntryP*>;
	return true;
      }

//---------------------------------------------------------
//   createEntry
//    Add a new entry \p entryName to the archive, reading
//    data from \p actualFile, with timestamp \p dt.
//	\p ActualFile must be open for read and and at current
//    position 0.
//	For a directory, \p dirOnly must be true and
//    \p actualFile is ignored.
//	For a PNG file, \p isPNGFile must be true and zlib
//    strategy Z_RLE is used.
//	Compression level is determined by \p level.
//---------------------------------------------------------

bool Zip::createEntry(const QString& entryName,
   QIODevice& actualFile, QDateTime dt, bool dirOnly, bool isPNGFile,
   int level)
      {
	if (device == 0) {
		lastError = NoOpenArchive;
            return false;
            }

	if (!actualFile.isReadable()) {
		lastError = ReadFailed;
            return false;
            }

	// create header and store it to write a central directory later
	ZipEntryP* h = new ZipEntryP;

	h->compMethod = (level == 0) ? 0 : 0x0008;

	QDate d = dt.date();
	h->modDate[1] = ((d.year() - 1980) << 1) & 254;
	h->modDate[1] |= ((d.month() >> 3) & 1);
	h->modDate[0] = ((d.month() & 7) << 5) & 224;
	h->modDate[0] |= d.day();

	QTime t = dt.time();
	h->modTime[1] = (t.hour() << 3) & 248;
	h->modTime[1] |= ((t.minute() >> 3) & 7);
	h->modTime[0] = ((t.minute() & 7) << 5) & 224;
	h->modTime[0] |= t.second() / 2;

	h->szUncomp = dirOnly ? 0 : actualFile.size();

	// **** Write local file header ****

	// signature
	buffer1[0] = 'P'; buffer1[1] = 'K';
	buffer1[2] = 0x3; buffer1[3] = 0x4;

	// version needed to extract
	buffer1[ZIP_LH_OFF_VERS] = ZIP_VERSION;
	buffer1[ZIP_LH_OFF_VERS + 1] = 0;

	// general purpose flag
	buffer1[ZIP_LH_OFF_GPFLAG] = h->gpFlag[0];
	buffer1[ZIP_LH_OFF_GPFLAG + 1] = h->gpFlag[1];

	// compression method
	buffer1[ZIP_LH_OFF_CMET] = h->compMethod & 0xFF;
	buffer1[ZIP_LH_OFF_CMET + 1] = (h->compMethod>>8) & 0xFF;

	// last mod file time
	buffer1[ZIP_LH_OFF_MODT] = h->modTime[0];
	buffer1[ZIP_LH_OFF_MODT + 1] = h->modTime[1];

	// last mod file date
	buffer1[ZIP_LH_OFF_MODD] = h->modDate[0];
	buffer1[ZIP_LH_OFF_MODD + 1] = h->modDate[1];

	// skip crc (4bytes) [14,15,16,17]

	// skip compressed size but include evtl. encryption header (4bytes: [18,19,20,21])
	buffer1[ZIP_LH_OFF_CSIZE] =
	buffer1[ZIP_LH_OFF_CSIZE + 1] =
	buffer1[ZIP_LH_OFF_CSIZE + 2] =
	buffer1[ZIP_LH_OFF_CSIZE + 3] = 0;

	h->szComp = 0;

	// uncompressed size [22,23,24,25]
	setULong(h->szUncomp, buffer1, ZIP_LH_OFF_USIZE);

	// filename length
	QByteArray entryNameBytes = entryName.toUtf8();
	int sz = entryNameBytes.size();

	buffer1[ZIP_LH_OFF_NAMELEN] = sz & 0xFF;
	buffer1[ZIP_LH_OFF_NAMELEN + 1] = (sz >> 8) & 0xFF;

	// extra field length
	buffer1[ZIP_LH_OFF_XLEN] = buffer1[ZIP_LH_OFF_XLEN + 1] = 0;

	// Store offset to write crc and compressed size
	h->lhOffset = device->pos();
	quint32 crcOffset = h->lhOffset + ZIP_LH_OFF_CRC;

	if (device->write(buffer1, ZIP_LOCAL_HEADER_SIZE) != ZIP_LOCAL_HEADER_SIZE) {
		delete h;
		lastError = WriteFailed;
            return false;
	      }

	// Write out filename
	if (device->write(entryNameBytes) != sz) {
		delete h;
		lastError = WriteFailed;
            return false;
	      }

	qint64 written = 0;
	quint32 crc = crc32(0L, Z_NULL, 0);

	if (!dirOnly) {
		// Write file data
		qint64 read = 0;
		qint64 totRead = 0;
		qint64 toRead = actualFile.size();

		if (level == 0) {
			while ( (read = actualFile.read(buffer1, BUFFER_SIZE)) > 0 ) {
				crc = crc32(crc, uBuffer, read);

				if ( (written = device->write(buffer1, read)) != read ) {
					actualFile.close();
					delete h;
					lastError = WriteFailed;
                              return false;
				      }
			      }
		      }
		else {
			z_stream zstr;

			// Initialize zalloc, zfree and opaque before calling the init function
			zstr.zalloc = Z_NULL;
			zstr.zfree = Z_NULL;
			zstr.opaque = Z_NULL;

			int zret;

			// Use deflateInit2 with negative windowBits to get raw compression
			if ((zret = deflateInit2_(
					&zstr,
					(int)level,
					Z_DEFLATED,
					-MAX_WBITS,
					8,
					isPNGFile ? Z_RLE : Z_DEFAULT_STRATEGY,
					ZLIB_VERSION,
					sizeof(z_stream)
				)) != Z_OK ) {
				actualFile.close();
				delete h;
				lastError = ZlibError;
                        return false;
			      }

			qint64 compressed;

			int flush = Z_NO_FLUSH;

			do {
				read = actualFile.read(buffer1, BUFFER_SIZE);
				totRead += read;

				if (read == 0)
					break;
				if (read < 0) {
					actualFile.close();
					deflateEnd(&zstr);
					delete h;
					lastError = ReadFailed;
                              return false;
				      }

				crc = crc32(crc, uBuffer, read);

				zstr.next_in = (Bytef*) buffer1;
				zstr.avail_in = (uInt)read;

				// Tell zlib if this is the last chunk we want to encode
				// by setting the flush parameter to Z_FINISH
				flush = (totRead == toRead) ? Z_FINISH : Z_NO_FLUSH;

				// Run deflate() on input until output buffer not full
				// finish compression if all of source has been read in
        		do
        		{
					zstr.next_out = (Bytef*) buffer2;
					zstr.avail_out = BUFFER_SIZE;

					zret = deflate(&zstr, flush);
					// State not clobbered

					// Write compressed data to file and empty buffer
					compressed = BUFFER_SIZE - zstr.avail_out;

					if (device->write(buffer2, compressed) != compressed)
					{
						deflateEnd(&zstr);
						actualFile.close();
						delete h;
                                    lastError = WriteFailed;
                                    return false;
					}

					written += compressed;

				} while (zstr.avail_out == 0);

			} while (flush != Z_FINISH);

			deflateEnd(&zstr);

		} // if (level != STORE)

		actualFile.close();
	}

	// Store end of entry offset
	quint32 current = device->pos();

	// Update crc and compressed size in local header
	if (!device->seek(crcOffset)) {
		delete h;
		lastError = SeekFailed;
            return false;
	      }

	h->crc = dirOnly ? 0 : crc;
	h->szComp += written;

	setULong(h->crc, buffer1, 0);
	setULong(h->szComp, buffer1, 4);
	if ( device->write(buffer1, 8) != 8) {
		delete h;
		lastError = WriteFailed;
            return false;
	      }

	// Seek to end of entry
	if (!device->seek(current)) {
		delete h;
		lastError = SeekFailed;
            return false;
	      }

	if ((h->gpFlag[0] & 8) == 8) {
		// Write data descriptor

		// Signature: PK\7\8
		buffer1[0] = 'P';
		buffer1[1] = 'K';
		buffer1[2] = 0x07;
		buffer1[3] = 0x08;

		// CRC
		setULong(h->crc, buffer1, ZIP_DD_OFF_CRC32);

		// Compressed size
		setULong(h->szComp, buffer1, ZIP_DD_OFF_CSIZE);

		// Uncompressed size
		setULong(h->szUncomp, buffer1, ZIP_DD_OFF_USIZE);

		if (device->write(buffer1, ZIP_DD_SIZE_WS) != ZIP_DD_SIZE_WS) {
			delete h;
			lastError = WriteFailed;
                  return false;
		      }
	      }

	headers->insert(entryName, h);
	return true;
      }

//---------------------------------------------------------
//   setULong
//---------------------------------------------------------

void Zip::setULong(quint32 v, char* buffer, unsigned int offset)
      {
	buffer[offset+3] = ((v >> 24) & 0xFF);
	buffer[offset+2] = ((v >> 16) & 0xFF);
	buffer[offset+1] = ((v >> 8) & 0xFF);
	buffer[offset] = (v & 0xFF);
      }

//---------------------------------------------------------
//   closeArchive
//---------------------------------------------------------

bool Zip::closeArchive()
      {
	// Close current archive by writing out central directory
	// and free up resources

	if (device == 0 || headers == 0)
		return true;

	const ZipEntryP* h;

	unsigned int sz;
	quint32 szCentralDir = 0;
	quint32 offCentralDir = device->pos();

	for (QMap<QString,ZipEntryP*>::ConstIterator itr = headers->constBegin(); itr != headers->constEnd(); ++itr) {
		h = itr.value();

		// signature
		buffer1[0] = 'P';
		buffer1[1] = 'K';
		buffer1[2] = 0x01;
		buffer1[3] = 0x02;

		// version made by  (currently only MS-DOS/FAT - no symlinks or other stuff supported)
		buffer1[ZIP_CD_OFF_MADEBY] = buffer1[ZIP_CD_OFF_MADEBY + 1] = 0;

		// version needed to extract
		buffer1[ZIP_CD_OFF_VERSION] = ZIP_VERSION;
		buffer1[ZIP_CD_OFF_VERSION + 1] = 0;

		// general purpose flag
		buffer1[ZIP_CD_OFF_GPFLAG] = h->gpFlag[0];
		buffer1[ZIP_CD_OFF_GPFLAG + 1] = h->gpFlag[1];

		// compression method
		buffer1[ZIP_CD_OFF_CMET] = h->compMethod & 0xFF;
		buffer1[ZIP_CD_OFF_CMET + 1] = (h->compMethod >> 8) & 0xFF;

		// last mod file time
		buffer1[ZIP_CD_OFF_MODT] = h->modTime[0];
		buffer1[ZIP_CD_OFF_MODT + 1] = h->modTime[1];

		// last mod file date
		buffer1[ZIP_CD_OFF_MODD] = h->modDate[0];
		buffer1[ZIP_CD_OFF_MODD + 1] = h->modDate[1];

		// crc (4bytes) [16,17,18,19]
		setULong(h->crc, buffer1, ZIP_CD_OFF_CRC);

		// compressed size (4bytes: [20,21,22,23])
		setULong(h->szComp, buffer1, ZIP_CD_OFF_CSIZE);

		// uncompressed size [24,25,26,27]
		setULong(h->szUncomp, buffer1, ZIP_CD_OFF_USIZE);

		// filename
		QByteArray fileNameBytes = itr.key().toUtf8();
		sz = fileNameBytes.size();
		buffer1[ZIP_CD_OFF_NAMELEN] = sz & 0xFF;
		buffer1[ZIP_CD_OFF_NAMELEN + 1] = (sz >> 8) & 0xFF;

		// extra field length
		buffer1[ZIP_CD_OFF_XLEN] = buffer1[ZIP_CD_OFF_XLEN + 1] = 0;

		// file comment length
		buffer1[ZIP_CD_OFF_COMMLEN] = buffer1[ZIP_CD_OFF_COMMLEN + 1] = 0;

		// disk number start
		buffer1[ZIP_CD_OFF_DISKSTART] = buffer1[ZIP_CD_OFF_DISKSTART + 1] = 0;

		// internal file attributes
		buffer1[ZIP_CD_OFF_IATTR] = buffer1[ZIP_CD_OFF_IATTR + 1] = 0;

		// external file attributes
		buffer1[ZIP_CD_OFF_EATTR] =
		buffer1[ZIP_CD_OFF_EATTR + 1] =
		buffer1[ZIP_CD_OFF_EATTR + 2] =
		buffer1[ZIP_CD_OFF_EATTR + 3] = 0;

		// relative offset of local header [42->45]
		setULong(h->lhOffset, buffer1, ZIP_CD_OFF_LHOFF);

		if (device->write(buffer1, ZIP_CD_SIZE) != ZIP_CD_SIZE) {
			//! \todo See if we can detect QFile objects using the Qt Meta Object System
			/*
			if (!device->remove())
				qDebug() << tr("Unable to delete corrupted archive: %1").arg(device->fileName());
			*/
			lastError = WriteFailed;
                  return false;
		      }

		// Write out filename
		if ((unsigned int)device->write(fileNameBytes) != sz) {
			//! \todo SAME AS ABOVE: See if we can detect QFile objects using the Qt Meta Object System
			/*
			if (!device->remove())
				qDebug() << tr("Unable to delete corrupted archive: %1").arg(device->fileName());
				*/
			lastError = WriteFailed;
                  return false;
		      }

		szCentralDir += (ZIP_CD_SIZE + sz);
	      }

	// Write end of central directory

	// signature
	buffer1[0] = 'P';
	buffer1[1] = 'K';
	buffer1[2] = 0x05;
	buffer1[3] = 0x06;

	// number of this disk
	buffer1[ZIP_EOCD_OFF_DISKNUM] = buffer1[ZIP_EOCD_OFF_DISKNUM + 1] = 0;

	// number of disk with central directory
	buffer1[ZIP_EOCD_OFF_CDDISKNUM] = buffer1[ZIP_EOCD_OFF_CDDISKNUM + 1] = 0;

	// number of entries in this disk
	sz = headers->count();
      buffer1[ZIP_EOCD_OFF_ENTRIES] = sz & 0xFF;
	buffer1[ZIP_EOCD_OFF_ENTRIES + 1] = (sz >> 8) & 0xFF;

	// total number of entries
	buffer1[ZIP_EOCD_OFF_CDENTRIES] = buffer1[ZIP_EOCD_OFF_ENTRIES];
	buffer1[ZIP_EOCD_OFF_CDENTRIES + 1] = buffer1[ZIP_EOCD_OFF_ENTRIES + 1];

	// size of central directory [12->15]
	setULong(szCentralDir, buffer1, ZIP_EOCD_OFF_CDSIZE);

	// central dir offset [16->19]
	setULong(offCentralDir, buffer1, ZIP_EOCD_OFF_CDOFF);

	buffer1[ZIP_EOCD_OFF_COMMLEN] = buffer1[ZIP_EOCD_OFF_COMMLEN + 1] = 0;

	if (device->write(buffer1, ZIP_EOCD_SIZE) != ZIP_EOCD_SIZE) {
		//! \todo SAME AS ABOVE: See if we can detect QFile objects using the Qt Meta Object System
		/*
		if (!device->remove())
			qDebug() << tr("Unable to delete corrupted archive: %1").arg(device->fileName());
			*/
		lastError = WriteFailed;
            return false;
	      }

      reset();
	return true;
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Zip::reset()
      {
	if (headers) {
		qDeleteAll(*headers);
		delete headers;
		headers = 0;
	      }

      if (device) {
            device->close();
            device = 0;
            }
      }

//! Local header size (excluding signature, excluding variable length fields)
#define UNZIP_LOCAL_HEADER_SIZE 26
//! Central Directory file entry size (excluding signature, excluding variable length fields)
#define UNZIP_CD_ENTRY_SIZE_NS 42
//! Data descriptor size (excluding signature)
#define UNZIP_DD_SIZE 12
//! End Of Central Directory size (including signature, excluding variable length fields)
#define UNZIP_EOCD_SIZE 22

// Some offsets inside a CD record (excluding signature)
#define UNZIP_CD_OFF_VERSION 0
#define UNZIP_CD_OFF_GPFLAG 4
#define UNZIP_CD_OFF_CMETHOD 6
#define UNZIP_CD_OFF_MODT 8
#define UNZIP_CD_OFF_MODD 10
#define UNZIP_CD_OFF_CRC32 12
#define UNZIP_CD_OFF_CSIZE 16
#define UNZIP_CD_OFF_USIZE 20
#define UNZIP_CD_OFF_NAMELEN 24
#define UNZIP_CD_OFF_XLEN 26
#define UNZIP_CD_OFF_COMMLEN 28
#define UNZIP_CD_OFF_LHOFFSET 38

// Some offsets inside a local header record (excluding signature)
#define UNZIP_LH_OFF_VERSION 0
#define UNZIP_LH_OFF_GPFLAG 2
#define UNZIP_LH_OFF_CMETHOD 4
#define UNZIP_LH_OFF_MODT 6
#define UNZIP_LH_OFF_MODD 8
#define UNZIP_LH_OFF_CRC32 10
#define UNZIP_LH_OFF_CSIZE 14
#define UNZIP_LH_OFF_USIZE 18
#define UNZIP_LH_OFF_NAMELEN 22
#define UNZIP_LH_OFF_XLEN 24

// Some offsets inside a data descriptor record (excluding signature)
#define UNZIP_DD_OFF_CRC32 0
#define UNZIP_DD_OFF_CSIZE 4
#define UNZIP_DD_OFF_USIZE 8

// Some offsets inside a EOCD record
#define UNZIP_EOCD_OFF_ENTRIES 6
#define UNZIP_EOCD_OFF_CDOFF 12
#define UNZIP_EOCD_OFF_COMMLEN 16

/*!
	Max version handled by this API.
	0x1B = 2.7 --> full compatibility only up to version 2.0 (0x14)
	versions from 2.1 to 2.7 may use unsupported compression methods
	versions after 2.7 may have an incompatible header format
*/
#define UNZIP_VERSION 0x1B
//! Full compatibility granted until this version
#define UNZIP_VERSION_STRICT 0x14

//! CRC32 routine
#define CRC32(c, b) crcTable[((int)c^b) & 0xff] ^ (c >> 8)

//! Checks if some file has been already extracted.
#define UNZIP_CHECK_FOR_VALID_DATA \
	{\
		if (headers != 0)\
		{\
			lastError = headers->size() != 0 ? Unzip::PartiallyCorrupted : Unzip::Corrupted;\
			break;\
		}\
		else\
		{\
			delete device;\
			device = 0;\
			lastError = Unzip::Corrupted;\
			break;\
		}\
	}


//---------------------------------------------------------
//   Unzip
//---------------------------------------------------------

Unzip::Unzip()
      {
	headers               = 0;
	device                = 0;
	uBuffer               = (uchar*) buffer1;
	crcTable              = (quint32*) get_crc_table();
	cdOffset              = 0;
	eocdOffset            = 0;
	cdEntryCount          = 0;
	unsupportedEntryCount = 0;
      }

Unzip::~Unzip()
      {
	closeArchive();
      }

//---------------------------------------------------------
//   openArchive
//    Opens a zip archive and reads the files list. Closes
//    any previously opened archive.
//---------------------------------------------------------

bool Unzip::openArchive(const QString& filename)
      {
	QFile* file = new QFile(filename);

	if (!file->exists()) {
		delete file;
            lastError = FileNotFound;
		return false;
            }
	if (!file->open(QIODevice::ReadOnly)) {
		delete file;
		lastError = OpenFailed;
            return false;
	      }
	return openArchive(file);
      }

//---------------------------------------------------------
//   contains
//---------------------------------------------------------

bool Unzip::contains(const QString& file) const
      {
	if (headers == 0)
		return false;
	return headers->contains(file);
      }

//---------------------------------------------------------
//   fileList
//    Returns complete paths of files and directories in
//    this archive.
//---------------------------------------------------------

QStringList Unzip::fileList() const
      {
      return headers == 0 ? QStringList() : headers->keys();
      }

//---------------------------------------------------------
//   entryList
//    Returns information for each (correctly parsed)
//    entry of this archive.
//---------------------------------------------------------

QList<Unzip::ZipEntry> Unzip::entryList() const
      {
	QList<Unzip::ZipEntry> list;

	if (headers != 0) {
		for (QMap<QString,ZipEntryP*>::ConstIterator it = headers->constBegin(); it != headers->constEnd(); ++it) {
			const ZipEntryP* entry = it.value();

			ZipEntry z;
			z.filename = it.key();
			z.compressedSize = entry->szComp;
			z.uncompressedSize = entry->szUncomp;
			z.crc32 = entry->crc;
			z.lastModified = convertDateTime(entry->modDate, entry->modTime);
			z.compression = entry->compMethod == 0 ? NoCompression : entry->compMethod == 8 ? Deflated : UnknownCompression;
			z.type = z.filename.endsWith("/") ? Directory : File;

			list.append(z);
		      }
	      }
	return list;
      }

//---------------------------------------------------------
//   extractFile
//---------------------------------------------------------

bool Unzip::extractFile(const QString& filename, QIODevice* dev)
      {
	QMap<QString,ZipEntryP*>::Iterator itr = headers->find(filename);
	if (itr != headers->end()) {
		ZipEntryP* entry = itr.value();
		return extractFile(itr.key(), *entry, dev);
	      }
      lastError = FileNotFound;
      return false;
      }

//---------------------------------------------------------
//   ZipEntry
//---------------------------------------------------------

Unzip::ZipEntry::ZipEntry()
      {
	compressedSize   = 0;
	uncompressedSize = 0;
	crc32            = 0;
	compression      = NoCompression;
	type             = File;
      }

//---------------------------------------------------------
//   openArchive
//---------------------------------------------------------

bool Unzip::openArchive(QIODevice* dev)
      {
	if (device)
		closeArchive();

	device = dev;

	if (!(device->isOpen() || device->open(QIODevice::ReadOnly))) {
		delete device;
		device = 0;
		lastError = OpenFailed;
            return false;
	      }

	lastError = seekToCentralDirectory();
	if (lastError != Ok) {
		closeArchive();
		return false;
	      }

	//! \todo Ignore CD entry count? CD may be corrupted.
	if (cdEntryCount == 0)
		return true;

	bool continueParsing = true;

	while (continueParsing) {
		if (device->read(buffer1, 4) != 4)
			UNZIP_CHECK_FOR_VALID_DATA

		if (! (buffer1[0] == 'P' && buffer1[1] == 'K' && buffer1[2] == 0x01  && buffer1[3] == 0x02) )
			break;

		if ( (lastError = parseCentralDirectoryRecord()) != Unzip::Ok )
			break;
	      }

	if (lastError != Unzip::Ok) {
		closeArchive();
            return false;
            }

	return true;
      }

//---------------------------------------------------------
//   parseLocalHeaderRecord
//---------------------------------------------------------

Unzip::ErrorCode Unzip::parseLocalHeaderRecord(const QString& path, ZipEntryP& entry)
      {
	if (!device->seek(entry.lhOffset))
		return Unzip::SeekFailed;

	// Test signature
	if (device->read(buffer1, 4) != 4)
		return Unzip::ReadFailed;

	if ((buffer1[0] != 'P') || (buffer1[1] != 'K') || (buffer1[2] != 0x03) || (buffer1[3] != 0x04))
		return Unzip::InvalidArchive;

	if (device->read(buffer1, UNZIP_LOCAL_HEADER_SIZE) != UNZIP_LOCAL_HEADER_SIZE)
		return Unzip::ReadFailed;

	/*
		Check 3rd general purpose bit flag.

		"bit 3: If this bit is set, the fields crc-32, compressed size
		and uncompressed size are set to zero in the local
		header.  The correct values are put in the data descriptor
		immediately following the compressed data."
	*/
	bool hasDataDescriptor = entry.hasDataDescriptor();

	if ((entry.compMethod != getUShort(uBuffer, UNZIP_LH_OFF_CMETHOD))
	   || (entry.gpFlag[0] != uBuffer[UNZIP_LH_OFF_GPFLAG])
	   || (entry.gpFlag[1] != uBuffer[UNZIP_LH_OFF_GPFLAG + 1])
	   || (entry.modTime[0] != uBuffer[UNZIP_LH_OFF_MODT])
	   || (entry.modTime[1] != uBuffer[UNZIP_LH_OFF_MODT + 1])
	   || (entry.modDate[0] != uBuffer[UNZIP_LH_OFF_MODD])
	   || (entry.modDate[1] != uBuffer[UNZIP_LH_OFF_MODD + 1])
         || (!hasDataDescriptor &&
            ((entry.crc != getULong(uBuffer, UNZIP_LH_OFF_CRC32))
		  || (entry.szComp != getULong(uBuffer, UNZIP_LH_OFF_CSIZE))
		  || (entry.szUncomp != getULong(uBuffer, UNZIP_LH_OFF_USIZE)))))
            {
		return Unzip::HeaderConsistencyError;
            }

	// Check filename
	quint16 szName = getUShort(uBuffer, UNZIP_LH_OFF_NAMELEN);
	if (szName == 0)
		return Unzip::HeaderConsistencyError;

	if (device->read(buffer2, szName) != szName)
		return Unzip::ReadFailed;

	QString filename = QString::fromUtf8(buffer2, szName);
	if (filename != path) {
		qDebug() << "Filename in local header mismatches.";
		return Unzip::HeaderConsistencyError;
	      }

	// Skip extra field
	quint16 szExtra = getUShort(uBuffer, UNZIP_LH_OFF_XLEN);
	if (szExtra != 0) {
		if (!device->seek(device->pos() + szExtra))
			return Unzip::SeekFailed;
	      }

	entry.dataOffset = device->pos();

	if (hasDataDescriptor) {
		/*
			The data descriptor has this OPTIONAL signature: PK\7\8
			We try to skip the compressed data relying on the size set in the
			Central Directory record.
		*/
		if (!device->seek(device->pos() + entry.szComp))
			return Unzip::SeekFailed;

		// Read 4 bytes and check if there is a data descriptor signature
		if (device->read(buffer2, 4) != 4)
			return Unzip::ReadFailed;

		bool hasSignature = buffer2[0] == 'P' && buffer2[1] == 'K' && buffer2[2] == 0x07 && buffer2[3] == 0x08;
		if (hasSignature) {
			if (device->read(buffer2, UNZIP_DD_SIZE) != UNZIP_DD_SIZE)
				return Unzip::ReadFailed;
		      }
		else {
			if (device->read(buffer2 + 4, UNZIP_DD_SIZE - 4) != UNZIP_DD_SIZE - 4)
				return Unzip::ReadFailed;
		      }

		// DD: crc, compressed size, uncompressed size
		if (
		   entry.crc != getULong((uchar*)buffer2, UNZIP_DD_OFF_CRC32) ||
		   entry.szComp != getULong((uchar*)buffer2, UNZIP_DD_OFF_CSIZE) ||
		   entry.szUncomp != getULong((uchar*)buffer2, UNZIP_DD_OFF_USIZE)
		   )
			return Unzip::HeaderConsistencyError;
	      }

      return Unzip::Ok;
      }

//---------------------------------------------------------
//   seekToCentralDirectory
//---------------------------------------------------------

Unzip::ErrorCode Unzip::seekToCentralDirectory()
      {
	qint64 length = device->size();
	qint64 offset = length - UNZIP_EOCD_SIZE;

	if (length < UNZIP_EOCD_SIZE)
		return Unzip::InvalidArchive;

	if (!device->seek( offset ))
		return Unzip::SeekFailed;

	if (device->read(buffer1, UNZIP_EOCD_SIZE) != UNZIP_EOCD_SIZE)
		return Unzip::ReadFailed;

	bool eocdFound = (buffer1[0] == 'P' && buffer1[1] == 'K' && buffer1[2] == 0x05 && buffer1[3] == 0x06);

	if (eocdFound) {
		// Zip file has no comment (the only variable length field in the EOCD record)
		eocdOffset = offset;
	      }
	else {
		qint64 read;
		char* p = 0;

		offset -= UNZIP_EOCD_SIZE;

		if (offset <= 0)
			return Unzip::InvalidArchive;

		if (!device->seek( offset ))
			return Unzip::SeekFailed;

		while ((read = device->read(buffer1, UNZIP_EOCD_SIZE)) >= 0) {
			if ( (p = strstr(buffer1, "PK\5\6")) != 0) {
				// Seek to the start of the EOCD record so we can read it fully
				// Yes... we could simply read the missing bytes and append them to the buffer
				// but this is far easier so heck it!
				device->seek( offset + (p - buffer1) );
				eocdFound = true;
				eocdOffset = offset + (p - buffer1);

				// Read EOCD record
				if (device->read(buffer1, UNZIP_EOCD_SIZE) != UNZIP_EOCD_SIZE)
					return Unzip::ReadFailed;

				break;
			      }

			// TODO: This is very slow and only a temporary bug fix. Need some pattern matching algorithm here.
			offset -= 1;      // UNZIP_EOCD_SIZE;
			if (offset <= 0)
				return Unzip::InvalidArchive;

			if (!device->seek( offset ))
				return Unzip::SeekFailed;
		      }
	      }

	if (!eocdFound)
		return Unzip::InvalidArchive;

	// Parse EOCD to locate CD offset
	offset = getULong((const uchar*)buffer1, UNZIP_EOCD_OFF_CDOFF + 4);

	cdOffset = offset;

	cdEntryCount = getUShort((const uchar*)buffer1, UNZIP_EOCD_OFF_ENTRIES + 4);

	quint16 commentLength = getUShort((const uchar*)buffer1, UNZIP_EOCD_OFF_COMMLEN + 4);
	if (commentLength != 0) {
		QByteArray c = device->read(commentLength);
		if (c.count() != commentLength)
			return Unzip::ReadFailed;
	      }

	// Seek to the start of the CD record
	if (!device->seek( cdOffset ))
		return Unzip::SeekFailed;

	return Unzip::Ok;
      }

//---------------------------------------------------------
//   parseCentralDirectoryRecord
//---------------------------------------------------------

Unzip::ErrorCode Unzip::parseCentralDirectoryRecord()
      {
	// Read CD record
	if (device->read(buffer1, UNZIP_CD_ENTRY_SIZE_NS) != UNZIP_CD_ENTRY_SIZE_NS)
		return Unzip::ReadFailed;

	bool skipEntry = false;

	// Get compression type so we can skip non compatible algorithms
	quint16 compMethod = getUShort(uBuffer, UNZIP_CD_OFF_CMETHOD);

	// Get variable size fields length so we can skip the whole record
	// if necessary
	quint16 szName    = getUShort(uBuffer, UNZIP_CD_OFF_NAMELEN);
	quint16 szExtra   = getUShort(uBuffer, UNZIP_CD_OFF_XLEN);
	quint16 szComment = getUShort(uBuffer, UNZIP_CD_OFF_COMMLEN);

	quint32 skipLength = szName + szExtra + szComment;

	Unzip::ErrorCode ec = Unzip::Ok;

	if ((compMethod != 0) && (compMethod != 8)) {
		qDebug() << "Unsupported compression method. Skipping file.";
		skipEntry = true;
	      }

	// Header parsing may be a problem if version is bigger than UNZIP_VERSION
	if (!skipEntry && buffer1[UNZIP_CD_OFF_VERSION] > UNZIP_VERSION) {
		qDebug() << "Unsupported PKZip version. Skipping file.";
		skipEntry = true;
	      }

	if (!skipEntry && szName == 0) {
		qDebug() << "Skipping file with no name.";
		skipEntry = true;
	      }

	if (!skipEntry && device->read(buffer2, szName) != szName) {
		ec = Unzip::ReadFailed;
		skipEntry = true;
	      }

	if (skipEntry) {
		if (ec == Unzip::Ok) {
			if (!device->seek( device->pos() + skipLength ))
				ec = Unzip::SeekFailed;
			unsupportedEntryCount++;
		      }
		return ec;
	      }

	QString filename = QString::fromUtf8(buffer2, szName);

	ZipEntryP* h = new ZipEntryP;
	h->compMethod = compMethod;

	h->gpFlag[0] = buffer1[UNZIP_CD_OFF_GPFLAG];
	h->gpFlag[1] = buffer1[UNZIP_CD_OFF_GPFLAG + 1];

	h->modTime[0] = buffer1[UNZIP_CD_OFF_MODT];
	h->modTime[1] = buffer1[UNZIP_CD_OFF_MODT + 1];

	h->modDate[0] = buffer1[UNZIP_CD_OFF_MODD];
	h->modDate[1] = buffer1[UNZIP_CD_OFF_MODD + 1];

	h->crc = getULong(uBuffer, UNZIP_CD_OFF_CRC32);
	h->szComp = getULong(uBuffer, UNZIP_CD_OFF_CSIZE);
	h->szUncomp = getULong(uBuffer, UNZIP_CD_OFF_USIZE);

	// Skip extra field (if any)
	if (szExtra != 0) {
		if (!device->seek(device->pos() + szExtra)) {
			delete h;
			return Unzip::SeekFailed;
		      }
	      }

	// Read comment field (if any)
	if (szComment != 0) {
		if (device->read(buffer2, szComment) != szComment) {
			delete h;
			return Unzip::ReadFailed;
		      }
	      }

	h->lhOffset = getULong(uBuffer, UNZIP_CD_OFF_LHOFFSET);

	if (headers == 0)
		headers = new QMap<QString, ZipEntryP*>();
	headers->insert(filename, h);
	return Unzip::Ok;
      }

//---------------------------------------------------------
//   closeArchive
//---------------------------------------------------------

void Unzip::closeArchive()
      {
	if (device == 0)
		return;

	if (headers != 0) {
		qDeleteAll(*headers);
		delete headers;
		headers = 0;
	      }

	delete device;
      device                = 0;
	cdOffset              = 0;
	eocdOffset            = 0;
	cdEntryCount          = 0;
	unsupportedEntryCount = 0;
      }

//---------------------------------------------------------
//   extractFile
//---------------------------------------------------------

bool Unzip::extractFile(const QString& path, ZipEntryP& entry, QIODevice* dev)
      {
	if (!entry.lhEntryChecked) {
	      lastError = parseLocalHeaderRecord(path, entry);
		entry.lhEntryChecked = true;

		if (lastError != Ok)
			return false;
	      }

	if (!device->seek(entry.dataOffset)) {
            lastError = SeekFailed;
		return false;
            }

	if (entry.szComp == 0) {
		if (entry.crc != 0) {
                  lastError = Corrupted;
			return false;
                  }
		return true;
	      }

	uInt rep = entry.szComp / BUFFER_SIZE;
	uInt rem = entry.szComp % BUFFER_SIZE;
	uInt cur = 0;

	// extract data
	qint64 read;
	quint64 tot = 0;

	quint32 myCRC = crc32(0L, Z_NULL, 0);

	if (entry.compMethod == 0) {
		while ( (read = device->read(buffer1, cur < rep ? BUFFER_SIZE : rem)) > 0 ) {
			myCRC = crc32(myCRC, uBuffer, read);

			if (dev->write(buffer1, read) != read) {
                        lastError = WriteFailed;
				return false;
                        }

			cur++;
			tot += read;

			if (tot == entry.szComp)
				break;
		      }

		if (read < 0)
			return Unzip::ReadFailed;
	      }
	else if (entry.compMethod == 8) {
		/* Allocate inflate state */
		z_stream zstr;
		zstr.zalloc = Z_NULL;
		zstr.zfree = Z_NULL;
		zstr.opaque = Z_NULL;
		zstr.next_in = Z_NULL;
		zstr.avail_in = 0;

		int zret;

		// Use inflateInit2 with negative windowBits to get raw decompression
		if ((zret = inflateInit2_(&zstr, -MAX_WBITS, ZLIB_VERSION, sizeof(z_stream))) != Z_OK) {
			lastError = ZlibError;
                  return false;
                  }

		int szDecomp;

		// Decompress until deflate stream ends or end of file
		do {
			read = device->read(buffer1, cur < rep ? BUFFER_SIZE : rem);
			if (read == 0)
				break;
			if (read < 0) {
				(void)inflateEnd(&zstr);
                        lastError = ReadFailed;
				return false;
			      }
			cur++;
			tot += read;

			zstr.avail_in = (uInt) read;
			zstr.next_in = (Bytef*) buffer1;


			// Run inflate() on input until output buffer not full
			do {
				zstr.avail_out = BUFFER_SIZE;
				zstr.next_out = (Bytef*) buffer2;;

				zret = inflate(&zstr, Z_NO_FLUSH);

				switch (zret) {
					case Z_NEED_DICT:
					case Z_DATA_ERROR:
					case Z_MEM_ERROR:
						inflateEnd(&zstr);
						lastError = WriteFailed;
                                    return false;

					default:
						;
				      }

				szDecomp = BUFFER_SIZE - zstr.avail_out;
				if (dev->write(buffer2, szDecomp) != szDecomp) {
					inflateEnd(&zstr);
					lastError = ZlibError;
                              return false;
				      }

				myCRC = crc32(myCRC, (const Bytef*) buffer2, szDecomp);

			      } while (zstr.avail_out == 0);

		      } while (zret != Z_STREAM_END);

		inflateEnd(&zstr);
	      }

	if (myCRC != entry.crc) {
		lastError = Corrupted;
            return false;
            }
	return true;
      }

//---------------------------------------------------------
//   createDirectory
//    Creates a new directory and all the needed parent
//    directories.
//---------------------------------------------------------

bool Unzip::createDirectory(const QString& path)
      {
	QDir d(path);
	if (!d.exists()) {
            int sep = path.lastIndexOf("/");
		if (sep <= 0)
                  return true;

		if (!createDirectory(path.left(sep)))
			return false;

		if (!d.mkdir(path)) {
			qDebug() << QString("Unable to create directory: %1").arg(path);
			return false;
		      }
	      }
      return true;
      }

//---------------------------------------------------------
//   getULong
//    Reads an quint32 (4 bytes) from a byte array starting
//    at given offset.
//---------------------------------------------------------

quint32 Unzip::getULong(const uchar* data, quint32 offset) const
      {
	quint32 res = (quint32) data[offset];
	res |= (((quint32)data[offset+1]) << 8);
	res |= (((quint32)data[offset+2]) << 16);
	res |= (((quint32)data[offset+3]) << 24);
	return res;
      }

//---------------------------------------------------------
//   getULLong
//    Reads an quint64 (8 bytes) from a byte array starting
//    at given offset.
//---------------------------------------------------------

quint64 Unzip::getULLong(const uchar* data, quint32 offset) const
      {
	quint64 res = (quint64) data[offset];
	res |= (((quint64)data[offset+1]) << 8);
	res |= (((quint64)data[offset+2]) << 16);
	res |= (((quint64)data[offset+3]) << 24);
	res |= (((quint64)data[offset+1]) << 32);
	res |= (((quint64)data[offset+2]) << 40);
	res |= (((quint64)data[offset+3]) << 48);
	res |= (((quint64)data[offset+3]) << 56);
	return res;
      }

//---------------------------------------------------------
//   getUShort
//    Reads an quint16 (2 bytes) from a byte array starting
//    at given offset.
//---------------------------------------------------------

quint16 Unzip::getUShort(const uchar* data, quint32 offset) const
      {
	return (quint16) data[offset] | (((quint16)data[offset+1]) << 8);
      }

//---------------------------------------------------------
//   convertDateTime
//    Converts date and time values from ZIP format to a
//    QDateTime object.
//---------------------------------------------------------

QDateTime Unzip::convertDateTime(const uchar date[2], const uchar time[2]) const
      {
	QDateTime dt;

	// Usual PKZip low-byte to high-byte order

	// Date: 7 bits = years from 1980, 4 bits = month, 5 bits = day
	quint16 year  = (date[1] >> 1) & 127;
	quint16 month = ((date[1] << 3) & 14) | ((date[0] >> 5) & 7);
	quint16 day   = date[0] & 31;

	// Time: 5 bits hour, 6 bits minutes, 5 bits seconds with a 2sec precision
	quint16 hour    = (time[1] >> 3) & 31;
	quint16 minutes = ((time[1] << 3) & 56) | ((time[0] >> 5) & 7);
	quint16 seconds = (time[0] & 31) * 2;

	dt.setDate(QDate(1980 + year, month, day));
	dt.setTime(QTime(hour, minutes, seconds));
	return dt;
      }

