//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011-2012 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "config.h"
#include "score.h"
#include "xml.h"
#include "element.h"
#include "measure.h"
#include "segment.h"
#include "slur.h"
#include "chordrest.h"
#include "chord.h"
#include "tuplet.h"
#include "beam.h"
#include "revisions.h"
#include "page.h"
#include "part.h"
#include "staff.h"
#include "system.h"
#include "keysig.h"
#include "clef.h"
#include "text.h"
#include "ottava.h"
#include "volta.h"
#include "excerpt.h"
#include "mscore.h"
#include "stafftype.h"
#ifdef OMR
#include "omr/omr.h"
#include "omr/omrpage.h"
#endif
#include "sig.h"
#include "undo.h"
#include "imageStore.h"
#include "audio.h"
#include "barline.h"
#include "libmscore/qzipreader_p.h"
#include "libmscore/qzipwriter_p.h"
#ifdef Q_OS_WIN
#include <windows.h>
#include <stdio.h>
#endif

namespace Ms {

//---------------------------------------------------------
//   writeMeasure
//---------------------------------------------------------

static void writeMeasure(Xml& xml, MeasureBase* m, int staffIdx, bool writeSystemElements)
      {
      if (m->type() == Element::MEASURE || staffIdx == 0)
           m->write(xml, staffIdx, writeSystemElements);
      if (m->type() == Element::MEASURE)
            xml.curTick = m->tick() + m->ticks();
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Score::write(Xml& xml, bool selectionOnly)
      {
      xml.stag("Score");

#ifdef OMR
      if (_omr && xml.writeOmr)
            _omr->write(xml);
#endif
      if (_showOmr && xml.writeOmr)
            xml.tag("showOmr", _showOmr);
      if (_audio && xml.writeOmr) {
            xml.tag("playMode", int(_playMode));
            _audio->write(xml);
            }

      for (int i = 0; i < 32; ++i) {
            if (!_layerTags[i].isEmpty()) {
                  xml.tag(QString("LayerTag id=\"%1\" tag=\"%2\"").arg(i).arg(_layerTags[i]),
                     _layerTagComments[i]);
                  }
            }
      int n = _layer.size();
      for (int i = 1; i < n; ++i) {       // dont save default variant
            const Layer& l = _layer[i];
            xml.tagE(QString("Layer name=\"%1\" mask=\"%2\"").arg(l.name).arg(l.tags));
            }
      xml.tag("currentLayer", _currentLayer);

      if (!MScore::testMode)
            _synthesizerState.write(xml);

      if (pageNumberOffset())
            xml.tag("page-offset", pageNumberOffset());
      xml.tag("Division", MScore::division);
      xml.curTrack = -1;

      _style.save(xml, true);      // save only differences to buildin style

      if (!parentScore()) {
            int idx = 0;
            foreach(StaffType** st, _staffTypes) {
                  if ((idx >= STAFF_TYPES) || !(*st)->isEqual(*StaffType::preset(idx)))
                        (*st)->write(xml, idx);
                  ++idx;
                  }
            }
      xml.tag("showInvisible", _showInvisible);
      xml.tag("showUnprintable", _showUnprintable);
      xml.tag("showFrames", _showFrames);
      xml.tag("showMargins", _showPageborders);

      QMapIterator<QString, QString> i(_metaTags);
      while (i.hasNext()) {
            i.next();
            if (!MScore::testMode  || (i.key() != "platform" && i.key() != "creationDate"))
                  xml.tag(QString("metaTag name=\"%1\"").arg(i.key()), i.value());
            }

      foreach(KeySig* ks, customKeysigs)
            ks->write(xml);

      if (!selectionOnly) {
            xml.stag("PageList");
            foreach(Page* page, _pages)
                  page->write(xml);
            xml.etag();
            }

      foreach(const Part* part, _parts)
            part->write(xml);

      xml.curTrack = 0;
      int staffStart;
      int staffEnd;
      MeasureBase* measureStart;
      MeasureBase* measureEnd;

      if (selectionOnly) {
            staffStart   = _selection.staffStart();
            staffEnd     = _selection.staffEnd();
            measureStart = _selection.startSegment()->measure();
            // include title frames:
            while (measureStart->prev() && !measureStart->prev()->sectionBreak())
                  measureStart = measureStart->prev();
            if (_selection.endSegment())
                  measureEnd   = _selection.endSegment()->measure()->next();
            else
                  measureEnd   = 0;
            }
      else {
            staffStart   = 0;
            staffEnd     = nstaves();
            measureStart = first();
            measureEnd   = 0;
            }

      xml.trackDiff = -staffStart * VOICES;
      if (measureStart) {
            for (int staffIdx = staffStart; staffIdx < staffEnd; ++staffIdx) {
                  xml.stag(QString("Staff id=\"%1\"").arg(staffIdx + 1));
                  xml.curTick  = measureStart->tick();
                  xml.tickDiff = xml.curTick;
                  xml.curTrack = staffIdx * VOICES;
                  bool writeSystemElements = staffIdx == staffStart;
                  for (MeasureBase* m = measureStart; m != measureEnd; m = m->next())
                        writeMeasure(xml, m, staffIdx, writeSystemElements);
                  xml.etag();
                  }
            }
      xml.curTrack = -1;
      if (!selectionOnly) {
            foreach(Excerpt* excerpt, _excerpts) {
                  if (excerpt->score() != this)
                        excerpt->score()->write(xml, false);       // recursion
                  }
            }
      if (parentScore())
            xml.tag("name", name());
      xml.etag();
      }

//---------------------------------------------------------
//   readStaff
//---------------------------------------------------------

void Score::readStaff(XmlReader& e)
      {
      MeasureBase* mb = first();
      int staff       = e.intAttribute("id", 1) - 1;
      e.setTick(0);
      e.setTrack(staff * VOICES);

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "Measure") {
                  Measure* measure = 0;
                  if (staff == 0) {
                        measure = new Measure(this);
                        measure->setTick(e.tick());
                        add(measure);
                        if (_mscVersion < 115) {
                              const SigEvent& ev = sigmap()->timesig(measure->tick());
                              measure->setLen(ev.timesig());
                              measure->setTimesig(ev.nominal());
                              }
                        else {
                              //
                              // inherit timesig from previous measure
                              //
                              Measure* m = measure->prevMeasure();
                              Fraction f(m ? m->timesig() : Fraction(4,4));
                              measure->setLen(f);
                              measure->setTimesig(f);
                              }
                        }
                  else {
                        while (mb) {
                              if (mb->type() != Element::MEASURE) {
                                    mb = mb->next();
                                    }
                              else {
                                    measure = (Measure*)mb;
                                    mb      = mb->next();
                                    break;
                                    }
                              }
                        if (measure == 0) {
                              qDebug("Score::readStaff(): missing measure!\n");
                              measure = new Measure(this);
                              measure->setTick(e.tick());
                              add(measure);
                              }
                        }
                  measure->read(e, staff);
                  e.setTick(measure->tick() + measure->ticks());
                  }
            else if (tag == "HBox" || tag == "VBox" || tag == "TBox" || tag == "FBox") {
                  MeasureBase* mb = static_cast<MeasureBase*>(Element::name2Element(tag, this));
                  mb->read(e);
                  mb->setTick(e.tick());
                  add(mb);
                  }
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   saveFile
///   If file has generated name, create a modal file save dialog
///   and ask filename.
///   Rename old file to backup file (.xxxx.msc?,).
///   Default is to save score in .mscz format,
///   Return true if OK and false on error.
//---------------------------------------------------------

bool Score::saveFile()
      {
      QString suffix = info.suffix();
      if (info.exists() && !info.isWritable()) {
            QString s = QT_TRANSLATE_NOOP("file", "The following file is locked: \n%1 \n\nTry saving to a different location.");
            MScore::lastError = s.arg(info.filePath());
            return false;
            }

      // if file was already saved in this session
      // save but don't overwrite backup again

      if (saved()) {
            try {
                  if (suffix == "mscx")
                        saveFile(info);
                  else
                        saveCompressedFile(info, false);
                  }
            catch (QString s) {
                  MScore::lastError = s;
                  return false;
                  }
            undo()->setClean();
            setDirty(false);
            return true;
            }
      //
      // step 1
      // save into temporary file to prevent partially overwriting
      // the original file in case of "disc full"
      //

      QString tempName = info.filePath() + QString(".temp");
      QFile temp(tempName);
      if (!temp.open(QIODevice::WriteOnly)) {
            MScore::lastError = QT_TRANSLATE_NOOP("file", "Open Temp File\n")
               + tempName + QT_TRANSLATE_NOOP("file", "\nfailed: ") + QString(strerror(errno));
            return false;
            }
      try {
            if (suffix == "mscx")
                  saveFile(&temp, false);
            else
                  saveCompressedFile(&temp, info, false);
            }
      catch (QString s) {
            MScore::lastError = s;
            return false;
            }
      if (temp.error() != QFile::NoError) {
            MScore::lastError = QT_TRANSLATE_NOOP("file",
               "MuseScore: Save File failed: ") + temp.errorString();
            temp.close();
            return false;
            }
      temp.close();

      //
      // step 2
      // remove old backup file if exists
      //
      QDir dir(info.path());
      QString backupName = QString(".") + info.fileName() + QString(",");
      if (dir.exists(backupName)) {
            if (!dir.remove(backupName)) {
//                  QMessageBox::critical(mscore, tr("MuseScore: Save File"),
//                     tr("removing old backup file ") + backupName + tr(" failed"));
                  }
            }

      //
      // step 3
      // rename old file into backup
      //
      QString name(info.filePath());
      if (dir.exists(name)) {
            if (!dir.rename(name, backupName)) {
//                  QMessageBox::critical(mscore, tr("MuseScore: Save File"),
//                     tr("renaming old file <")
//                      + name + tr("> to backup <") + backupName + tr("> failed"));
                  }
            }
#ifdef Q_OS_WIN
      QFileInfo fileBackup(dir, backupName);
      QString backupNativePath = QDir::toNativeSeparators(fileBackup.absoluteFilePath());
      SetFileAttributes((LPCTSTR)backupNativePath.toLocal8Bit(), FILE_ATTRIBUTE_HIDDEN);
#endif
      //
      // step 4
      // rename temp name into file name
      //
      if (!QFile::rename(tempName, name)) {
            MScore::lastError = QT_TRANSLATE_NOOP("file", "renaming temp. file <")
               + tempName + QT_TRANSLATE_NOOP("file", "> to <") + name
               + QT_TRANSLATE_NOOP("file", "> failed:\n")
               + QString(strerror(errno));
            return false;
            }
      // make file readable by all
      QFile::setPermissions(name, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser
         | QFile::ReadGroup | QFile::ReadOther);

      undo()->setClean();
      setDirty(false);
      setSaved(true);
      return true;
      }

//---------------------------------------------------------
//   saveCompressedFile
//---------------------------------------------------------

void Score::saveCompressedFile(QFileInfo& info, bool onlySelection)
      {
      QFile fp(info.filePath());
      if (!fp.open(QIODevice::WriteOnly)) {
            QString s = QString("Open File\n") + info.filePath() + QString("\nfailed: ")
               + QString(strerror(errno));
            throw(s);
            }
      saveCompressedFile(&fp, info, onlySelection);
      fp.close();
      }

//---------------------------------------------------------
//   saveCompressedFile
//    file is already opened
//---------------------------------------------------------

void Score::saveCompressedFile(QIODevice* f, QFileInfo& info, bool onlySelection)
      {
      MQZipWriter uz(f);

      QString fn = info.completeBaseName() + ".mscx";
      QBuffer cbuf;
      cbuf.open(QIODevice::ReadWrite);
      Xml xml(&cbuf);
      xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      xml.stag("container");
      xml.stag("rootfiles");
      xml.stag(QString("rootfile full-path=\"%1\"").arg(Xml::xmlString(fn)));
      xml.etag();
      foreach(ImageStoreItem* ip, imageStore) {
            if (!ip->isUsed(this))
                  continue;
            QString path = QString("Pictures/") + ip->hashName();
            xml.tag("file", path);
            }

      xml.etag();
      xml.etag();
      cbuf.seek(0);
      //uz.addDirectory("META-INF");
      uz.addFile("META-INF/container.xml", cbuf.data());

      // save images
      //uz.addDirectory("Pictures");
      foreach(ImageStoreItem* ip, imageStore) {
            if (!ip->isUsed(this))
                  continue;
            QString path = QString("Pictures/") + ip->hashName();
            uz.addFile(path, ip->buffer());
            }
#ifdef OMR
      //
      // save OMR page images
      //
      if (_omr) {
            int n = _omr->numPages();
            for (int i = 0; i < n; ++i) {
                  QString path = QString("OmrPages/page%1.png").arg(i+1);
                  QBuffer cbuf;
                  OmrPage* page = _omr->page(i);
                  const QImage& image = page->image();
                  if (!image.save(&cbuf, "PNG"))
                        throw(QString("save file: cannot save image (%1x%2)").arg(image.width()).arg(image.height()));
                  uz.addFile(path, cbuf.data());
                  cbuf.close();
                  }
            }
#endif
      //
      // save audio
      //
      if (_audio)
            uz.addFile("audio.ogg", _audio->data());

      QBuffer dbuf;
      dbuf.open(QIODevice::ReadWrite);
      saveFile(&dbuf, true, onlySelection);
      dbuf.seek(0);
      uz.addFile(fn, dbuf.data());
      uz.close();
      }

//---------------------------------------------------------
//   saveFile
//    return true on success
//---------------------------------------------------------

bool Score::saveFile(QFileInfo& info)
      {
      if (info.suffix().isEmpty())
            info.setFile(info.filePath() + ".mscx");
      QFile fp(info.filePath());
      if (!fp.open(QIODevice::WriteOnly)) {
            QString s = QString("Open File\n") + info.filePath() + QString("\nfailed: ")
               + QString(strerror(errno));
            return false;
            }
      saveFile(&fp, false);
      fp.close();
      return true;
      }

//---------------------------------------------------------
//   loadStyle
//---------------------------------------------------------

bool Score::loadStyle(const QString& fn)
      {
      QFile f(fn);
      if (f.open(QIODevice::ReadOnly)) {
            MStyle st = _style;
            if (st.load(&f)) {
                  undo(new ChangeStyle(this, st));
                  return true;
                  }
             else {
                  MScore::lastError = tr("The style file is not compatible with this version of MuseScore.");
                  return false;
                  }
            }
      MScore::lastError = strerror(errno);
      return false;
      }

//---------------------------------------------------------
//   saveStyle
//---------------------------------------------------------

bool Score::saveStyle(const QString& name)
      {
      QString ext(".mss");
      QFileInfo info(name);

      if (info.suffix().isEmpty())
            info.setFile(info.filePath() + ext);
      QFile f(info.filePath());
      if (!f.open(QIODevice::WriteOnly)) {
            MScore::lastError = QT_TRANSLATE_NOOP("file", "Open Style File\n")
               + f.fileName() + QT_TRANSLATE_NOOP("file", "\nfailed: ")
               + QString(strerror(errno));
            return false;
            }

      Xml xml(&f);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      _style.save(xml, false);     // save complete style
      xml.etag();
      if (f.error() != QFile::NoError) {
            MScore::lastError = QT_TRANSLATE_NOOP("file", "Write Style failed: ")
               + f.errorString();
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   saveFile
//    return true on success
//---------------------------------------------------------

extern QString revision;
extern bool enableTestMode;

void Score::saveFile(QIODevice* f, bool msczFormat, bool onlySelection)
      {
      if(!MScore::testMode)
            MScore::testMode = enableTestMode;
      Xml xml(f);
      xml.writeOmr = msczFormat;
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      if (!MScore::testMode) {
            xml.tag("programVersion", VERSION);
            xml.tag("programRevision", revision);
            }
      write(xml, onlySelection);
      xml.etag();
      if (!parentScore())
            _revisions->write(xml);
      if(!onlySelection) {
            //update version values for i.e. plugin access
            _mscoreVersion = VERSION;
            _mscoreRevision = revision.toInt();
            _mscVersion = MSCVERSION;
            }
      }

//---------------------------------------------------------
//   loadCompressedMsc
//    return false on error
//---------------------------------------------------------

Score::FileError Score::loadCompressedMsc(QString name, bool ignoreVersionError)
      {
      MQZipReader uz(name);
      if (!uz.exists()) {
            qDebug("loadCompressedMsc: <%s> not found\n", qPrintable(name));
            MScore::lastError = QT_TRANSLATE_NOOP("file", "file not found");
            return FILE_NOT_FOUND;
            }
      QByteArray cbuf = uz.fileData("META-INF/container.xml");

      QString rootfile;
      XmlReader e(cbuf);
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "rootfile") {
                  rootfile = e.attribute("full-path");
                  e.skipCurrentElement();
                  }
            else if (tag == "file") {
                  QString image(e.readElementText());
                  QByteArray dbuf = uz.fileData(image);
                  imageStore.add(image, dbuf);
                  }
            }
      if (rootfile.isEmpty())
            return FILE_NO_ROOTFILE;

      QByteArray dbuf = uz.fileData(rootfile);
      if (dbuf.isEmpty()) {
//            qDebug("root file <%s> is empty", qPrintable(rootfile));
            QList<MQZipReader::FileInfo> fil = uz.fileInfoList();
            foreach(const MQZipReader::FileInfo& fi, fil) {
                  if (fi.filePath.endsWith(".mscx")) {
                        dbuf = uz.fileData(fi.filePath);
                        break;
                        }
                  }
            }
      e.clear();
      e.addData(dbuf);
      e.setDocName(info.completeBaseName());

      FileError retval = read1(e, ignoreVersionError);
      _noteHeadWidth = symbols[_symIdx][quartheadSym].width(spatium() / (MScore::DPI * SPATIUM20));

#ifdef OMR
      //
      // load OMR page images
      //
      if (_omr) {
            int n = _omr->numPages();
            for (int i = 0; i < n; ++i) {
                  QString path = QString("OmrPages/page%1.png").arg(i+1);
                  QByteArray dbuf = uz.fileData(path);
                  OmrPage* page = _omr->page(i);
                  QImage image;
                  if (image.loadFromData(dbuf, "PNG")) {
                        page->setImage(image);
                        }
                  else
                        qDebug("load image failed");
                  }
            }
#endif
      //
      //  read audio
      //
      if (_audio) {
            QByteArray dbuf = uz.fileData("audio.ogg");
            _audio->setData(dbuf);
            }
      return retval;
      }

//---------------------------------------------------------
//   loadMsc
//    return true on success
//---------------------------------------------------------

Score::FileError Score::loadMsc(QString name, bool ignoreVersionError)
      {
      info.setFile(name);

      if (name.endsWith(".mscz"))
            return loadCompressedMsc(name, ignoreVersionError);

      QFile f(name);
      if (!f.open(QIODevice::ReadOnly)) {
            MScore::lastError = f.errorString();
            return FILE_OPEN_ERROR;
            }

      XmlReader xml(&f);
      FileError retval = read1(xml, ignoreVersionError);
      _noteHeadWidth = symbols[_symIdx][quartheadSym].width(spatium() / (MScore::DPI * SPATIUM20));
      return retval;
      }

//---------------------------------------------------------
//   parseVersion
//---------------------------------------------------------

void Score::parseVersion(const QString& val)
      {
      QRegExp re("(\\d+)\\.(\\d+)\\.(\\d+)");
      int v1, v2, v3, rv1, rv2, rv3;
      if (re.indexIn(VERSION) != -1) {
            QStringList sl = re.capturedTexts();
            if (sl.size() == 4) {
                  v1 = sl[1].toInt();
                  v2 = sl[2].toInt();
                  v3 = sl[3].toInt();
                  if (re.indexIn(val) != -1) {
                        sl = re.capturedTexts();
                        if (sl.size() == 4) {
                              rv1 = sl[1].toInt();
                              rv2 = sl[2].toInt();
                              rv3 = sl[3].toInt();

                              int currentVersion = v1 * 10000 + v2 * 100 + v3;
                              int readVersion = rv1 * 10000 + rv2 * 100 + rv3;
                              if (readVersion > currentVersion) {
                                    qDebug("read future version\n");
                                    }
                              }
                        }
                  else {
                        QRegExp re1("(\\d+)\\.(\\d+)");
                        if (re1.indexIn(val) != -1) {
                              sl = re.capturedTexts();
                              if (sl.size() == 3) {
                                    rv1 = sl[1].toInt();
                                    rv2 = sl[2].toInt();

                                    int currentVersion = v1 * 10000 + v2 * 100 + v3;
                                    int readVersion = rv1 * 10000 + rv2 * 100;
                                    if (readVersion > currentVersion) {
                                          qDebug("read future version\n");
                                          }
                                    }
                              }
                        else
                              qDebug("1cannot parse <%s>\n", qPrintable(val));
                        }
                  }
            }
      else
            qDebug("2cannot parse <%s>\n", VERSION);
      }

//---------------------------------------------------------
//   read1
//    return true on success
//---------------------------------------------------------

Score::FileError Score::read1(XmlReader& e, bool ignoreVersionError)
      {
      _elinks.clear();

      while (e.readNextStartElement()) {
            if (e.name() == "museScore") {
                  const QString& version = e.attribute("version");
                  QStringList sl = version.split('.');
                  _mscVersion = sl[0].toInt() * 100 + sl[1].toInt();

                  if (!ignoreVersionError) {
                        QString message;
                        if (_mscVersion > MSCVERSION)
                              return FILE_TOO_NEW;
                        if (_mscVersion < 114)
                              return FILE_TOO_OLD;
                        }

                  if (_mscVersion <= 114)
                        return read114(e);
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "programVersion") {
                              _mscoreVersion = e.readElementText();
                              parseVersion(_mscoreVersion);
                              }
                        else if (tag == "programRevision")
                              _mscoreRevision = e.readInt();
                        else if (tag == "Score") {
                              if (!read(e))
                                    return FILE_BAD_FORMAT;
                              }
                        else if (tag == "Revision") {
                              Revision* revision = new Revision;
                              revision->read(e);
                              _revisions->add(revision);
                              }
                        else
                              e.unknown();
                        }
                  }
            else
                  e.unknown();
            }
      int id = 1;
      foreach (LinkedElements* le, _elinks)
            le->setLid(this, id++);
      _elinks.clear();

// _mscVersion is needed used during layout
//      _mscVersion = MSCVERSION;     // for later drag & drop usage

      return FILE_NO_ERROR;
      }

//---------------------------------------------------------
//   read
//    return false on error
//---------------------------------------------------------

bool Score::read(XmlReader& e)
      {
      if (parentScore())
            setMscVersion(parentScore()->mscVersion());

      while (e.readNextStartElement()) {
            e.setTrack(-1);
            const QStringRef& tag(e.name());
            if (tag == "Staff")
                  readStaff(e);
            else if (tag == "KeySig") {
                  KeySig* ks = new KeySig(this);
                  ks->read(e);
                  customKeysigs.append(ks);
                  }
            else if (tag == "StaffType") {
                  int idx           = e.intAttribute("idx");
                  QString groupName = e.attribute("group", "pitched");
                  int group;
                  // staff type numbering did change!
                  // attempt to keep some compatibility with existing 2.0 scores
                  if (groupName == "percussion")
                        group = PERCUSSION_STAFF_GROUP;
                  else if (groupName == "tablature")
                        group = TAB_STAFF_GROUP;
                  else group = STANDARD_STAFF_GROUP;
                  StaffType* ost = staffType(idx);
                  StaffType* st;
                  if (ost && ost->group() == group)
                        st = ost->clone();
                  else {
                        idx = -1;
                        switch (group)
                        {
                        case PERCUSSION_STAFF_GROUP:
                              st  = new StaffTypePercussion();
                              break;
                        case TAB_STAFF_GROUP:
                              st  = new StaffTypeTablature();
                              break;
                        default:
                              st  = new StaffTypePitched();
                        }
                        }
                  st->read(e);
                  st->setBuiltin(false);
                  addStaffType(idx, st);
                  }
            else if (tag == "siglist")
                  _sigmap->read(e, _fileDivision);
            else if (tag == "programVersion") {
                  _mscoreVersion = e.readElementText();
                  parseVersion(_mscoreVersion);
                  }
            else if (tag == "programRevision")
                  _mscoreRevision = e.readInt();
            else if (tag == "Omr") {
#ifdef OMR
                  _omr = new Omr(this);
                  _omr->read(e);
#else
                  e.skipCurrentElement();
#endif
                  }
            else if (tag == "Audio") {
                  _audio = new Audio;
                  _audio->read(e);
                  }
            else if (tag == "showOmr")
                  _showOmr = e.readInt();
            else if (tag == "playMode")
                  _playMode = PlayMode(e.readInt());
            else if (tag == "LayerTag") {
                  int id = e.intAttribute("id");
                  const QString& tag = e.attribute("tag");
                  QString val(e.readElementText());
                  if (id >= 0 && id < 32) {
                        _layerTags[id] = tag;
                        _layerTagComments[id] = val;
                        }
                  }
            else if (tag == "Layer") {
                  Layer layer;
                  layer.name = e.attribute("name");
                  layer.tags = e.attribute("mask").toUInt();
                  _layer.append(layer);
                  }
            else if (tag == "currentLayer")
                  _currentLayer = e.readInt();
            else if (tag == "SyntiSettings")    // obsolete
                  _synthesizerState.read(e);
            else if (tag == "Synthesizer")
                  _synthesizerState.read(e);
            else if (tag == "Spatium")
                  _style.setSpatium (e.readDouble() * MScore::DPMM); // obsolete, moved to Style
            else if (tag == "page-offset")            // obsolete, moved to Score
                  setPageNumberOffset(e.readInt());
            else if (tag == "Division")
                  _fileDivision = e.readInt();
            else if (tag == "showInvisible")
                  _showInvisible = e.readInt();
            else if (tag == "showUnprintable")
                  _showUnprintable = e.readInt();
            else if (tag == "showFrames")
                  _showFrames = e.readInt();
            else if (tag == "showMargins")
                  _showPageborders = e.readInt();
            else if (tag == "Style") {
                  qreal sp = _style.spatium();
                  _style.load(e);
                  // if (_layoutMode == LayoutFloat || _layoutMode == LayoutSystem) {
                  if (_layoutMode == LayoutFloat) {
                        // style should not change spatium in
                        // float mode
                        _style.setSpatium(sp);
                        }
                  }
            else if (tag == "copyright" || tag == "rights") {
                  Text* text = new Text(this);
                  text->read(e);
                  setMetaTag("copyright", text->text());
                  delete text;
                  }
            else if (tag == "movement-number")
                  setMetaTag("movementNumber", e.readElementText());
            else if (tag == "movement-title")
                  setMetaTag("movementTitle", e.readElementText());
            else if (tag == "work-number")
                  setMetaTag("workNumber", e.readElementText());
            else if (tag == "work-title")
                  setMetaTag("workTitle", e.readElementText());
            else if (tag == "source")
                  setMetaTag("source", e.readElementText());
            else if (tag == "metaTag") {
                  QString name = e.attribute("name");
                  setMetaTag(name, e.readElementText());
                  }
            else if (tag == "Part") {
                  Part* part = new Part(this);
                  part->read(e);
                  _parts.push_back(part);
                  }
            else if ((tag == "HairPin")
                || (tag == "Ottava")
                || (tag == "TextLine")
                || (tag == "Volta")
                || (tag == "Trill")
                || (tag == "Slur")
                || (tag == "Pedal")) {
                  Spanner* s = static_cast<Spanner*>(Element::name2Element(tag, this));
                  s->read(e);
                  addSpanner(s);
                  }
            else if (tag == "Excerpt") {
                  Excerpt* ex = new Excerpt(this);
                  ex->read(e);
                  _excerpts.append(ex);
                  }
            else if (tag == "Beam") {
                  Beam* beam = new Beam(this);
                  beam->read(e);
                  beam->setParent(0);
                  // _beams.append(beam);
                  }
            else if (tag == "Score") {          // recursion
                  Score* s = new Score(style());
                  s->setParentScore(this);
                  s->read(e);
                  addExcerpt(s);
                  }
            else if (tag == "PageList") {
                  while (e.readNextStartElement()) {
                        if (e.name() == "Page") {
                              Page* page = new Page(this);
                              _pages.append(page);
                              page->read(e);
                              }
                        else
                              e.unknown();
                        }
                  }
            else if (tag == "name")
                  setName(e.readElementText());
            else if (tag == "page-layout") {    // obsolete
                  if (_layoutMode != LayoutFloat && _layoutMode != LayoutSystem) {
                        PageFormat pf;
                        pf.copy(*pageFormat());
                        pf.read(e);
                        setPageFormat(pf);
                        }
                  else
                        e.skipCurrentElement();
                  }
            else if (tag == "cursorTrack")
                  e.skipCurrentElement();
            else
                  e.unknown();
            }
      if (e.error() != QXmlStreamReader::NoError)
            return false;

      connectTies();

      _fileDivision = MScore::division;

      //
      //    sanity check for barLineSpan
      //
      foreach(Staff* st, _staves) {
            int barLineSpan = st->barLineSpan();
            int idx = staffIdx(st);
            int n = nstaves();
            if (idx + barLineSpan > n) {
                  qDebug("bad span: idx %d  span %d staves %d\n", idx, barLineSpan, n);
                  st->setBarLineSpan(n - idx);
                  }
            // check spanFrom
            if(st->barLineFrom() < MIN_BARLINE_SPAN_FROMTO)
                  st->setBarLineFrom(MIN_BARLINE_SPAN_FROMTO);
            if(st->barLineFrom() > st->lines()*2)
                  st->setBarLineFrom(st->lines()*2);
            // check spanTo
            Staff* stTo = st->barLineSpan() <= 1 ? st : staff(idx + st->barLineSpan() - 1);
            // 1-line staves have special bar line spans
            int maxBarLineTo        = stTo->lines() == 1 ? BARLINE_SPAN_1LINESTAFF_TO : stTo->lines()*2;
            int defaultBarLineTo    = stTo->lines() == 1 ? BARLINE_SPAN_1LINESTAFF_TO : (stTo->lines() - 1) * 2;
            if (st->barLineTo() == UNKNOWN_BARLINE_TO)
                  st->setBarLineTo(defaultBarLineTo);
            if (st->barLineTo() < MIN_BARLINE_SPAN_FROMTO)
                  st->setBarLineTo(MIN_BARLINE_SPAN_FROMTO);
            if (st->barLineTo() > maxBarLineTo)
                  st->setBarLineTo(maxBarLineTo);
            // on single staff span, check spanFrom and spanTo are distant enough
            if (st->barLineSpan() == 1) {
                  if(st->barLineTo() - st->barLineFrom() < MIN_BARLINE_FROMTO_DIST) {
                        st->setBarLineFrom(0);
                        st->setBarLineTo(defaultBarLineTo);
                        }
                  }
            }

      if (_omr == 0)
            _showOmr = false;

      fixTicks();
      rebuildMidiMapping();
      updateChannel();
      updateNotes();          // only for parts needed?
      createPlayEvents();
      setExcerptsChanged(false);
      return true;
      }

//---------------------------------------------------------
//   print
//---------------------------------------------------------

void Score::print(QPainter* painter, int pageNo)
      {
      _printing  = true;
      Page* page = pages().at(pageNo);
      QRectF fr  = page->abbox();

      QList<Element*> ell = page->items(fr);
      qStableSort(ell.begin(), ell.end(), elementLessThan);
      foreach(const Element* e, ell) {
            e->itemDiscovered = 0;
            if (!e->visible())
                  continue;
            painter->save();
            painter->translate(e->pagePos());
            e->draw(painter);
            painter->restore();
            }
      _printing = false;
      }

//---------------------------------------------------------
//   readCompressedToBuffer
//---------------------------------------------------------

QByteArray Score::readCompressedToBuffer()
      {
      MQZipReader uz(filePath());

      QByteArray cbuf = uz.fileData("META-INF/container.xml");

      XmlReader e(cbuf);
      QString rootfile;
      QList<QString> images;

      while (e.readNextStartElement()) {
            if (e.name() != "container") {
                  e.unknown();
                  continue;
                  }
            while (e.readNextStartElement()) {
                  if (e.name() != "rootfiles") {
                        e.unknown();
                        continue;
                        }
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());

                        if (tag == "rootfile") {
                              if (rootfile.isEmpty())
                                    rootfile = e.attribute("full-path");
                              }
                        else if (tag == "file")
                              images.append(e.readElementText());
                        else
                              e.unknown();
                        }
                  }
            }
      //
      // load images
      //
      foreach(const QString& s, images) {
            QByteArray dbuf = uz.fileData(s);
            imageStore.add(s, dbuf);
            }

      if (rootfile.isEmpty()) {
            qDebug("can't find rootfile in: %s\n", qPrintable(filePath()));
            return QByteArray();
            }
      return uz.fileData(rootfile);
      }

//---------------------------------------------------------
//   readToBuffer
//---------------------------------------------------------

QByteArray Score::readToBuffer()
      {
      QByteArray ba;
      QString cs  = info.suffix();

      if (cs == "mscz") {
            ba = readCompressedToBuffer();
            }
      if (cs.toLower() == "msc" || cs.toLower() == "mscx") {
            QFile f(filePath());
            if (f.open(QIODevice::ReadOnly)) {
                  ba = f.readAll();
                  f.close();
                  }
            }
      return ba;
      }

//---------------------------------------------------------
//   createRevision
//---------------------------------------------------------

void Score::createRevision()
      {
#if 0
qDebug("createRevision\n");
      QBuffer dbuf;
      dbuf.open(QIODevice::ReadWrite);
      saveFile(&dbuf, false, false);
      dbuf.close();

      QByteArray ba1 = readToBuffer();

      QString os = QString::fromUtf8(ba1.data(), ba1.size());
      QString ns = QString::fromUtf8(dbuf.buffer().data(), dbuf.buffer().size());

      diff_match_patch dmp;
      Revision* r = new Revision();
      r->setDiff(dmp.patch_toText(dmp.patch_make(ns, os)));
      r->setId("1");
      _revisions->add(r);

//      qDebug("patch:\n%s\n==========\n", qPrintable(patch));
      //
#endif
      }

//---------------------------------------------------------
//   writeSegments
//    ls  - write upto this segment (excluding)
//          can be zero
//---------------------------------------------------------

void Score::writeSegments(Xml& xml, const Measure* m, int strack, int etrack,
   Segment* fs, Segment* ls, bool writeSystemElements, bool clip, bool needFirstTick)
      {
      for (int track = strack; track < etrack; ++track) {
            for (Segment* segment = fs; segment && segment != ls; segment = segment->next1()) {
                  if (track == 0)
                        segment->setWritten(false);
                  Element* e = segment->element(track);
                  //
                  // special case: - barline span > 1
                  //               - part (excerpt) staff starts after
                  //                 barline element
                  bool needTick = (needFirstTick && segment == fs) || (segment->tick() != xml.curTick);
                  if ((segment->segmentType() == Segment::SegEndBarLine)
                     && (e == 0)
                     && writeSystemElements
                     && ((track % VOICES) == 0)) {
                        // search barline:
                        for (int idx = track - VOICES; idx >= 0; idx -= VOICES) {
                              if (segment->element(idx)) {
                                    int oDiff = xml.trackDiff;
                                    xml.trackDiff = idx;          // staffIdx should be zero
                                    segment->element(idx)->write(xml);
                                    xml.trackDiff = oDiff;
                                    break;
                                    }
                              }
                        }
                  foreach (Element* e, segment->annotations()) {
                        if (e->track() != track || e->generated())
                              continue;
                        if (needTick) {
                              xml.tag("tick", segment->tick() - xml.tickDiff);
                              xml.curTick = segment->tick();
                              needTick = false;
                              }
                        e->write(xml);
                        }
                  if (segment->segmentType() & (Segment::SegChordRest)) {
                        for (auto i : _spanner.map()) {     // TODO: dont search whole list
                              Spanner* s = i.second;
                              if (s->track() != track || s->generated())
                                    continue;

                              int endTick = ls == 0 ? lastMeasure()->endTick() : ls->tick();
                              if (s->tick() == segment->tick() && (!clip || s->tick2() < endTick)) {
                                    if (needTick) {
                                          xml.tag("tick", segment->tick() - xml.tickDiff);
                                          xml.curTick = segment->tick();
                                          needTick = false;
                                          }
                                    s->setId(++xml.spannerId);
                                    s->write(xml);
                                    }
                              if (s->tick2() == segment->tick() && (!clip || s->tick() >= fs->tick())) {
                                    if (needTick) {
                                          xml.tag("tick", segment->tick() - xml.tickDiff);
                                          xml.curTick = segment->tick();
                                          needTick = false;
                                          }
                                    Q_ASSERT(s->id() != -1);
                                    xml.tagE(QString("endSpanner id=\"%1\"").arg(s->id()));
                                    }
                              }
                        }

                  if (!e)
                        continue;

                  if (e->generated()) {
                        if ((xml.curTick - xml.tickDiff) == 0) {
                              if (e->type() == Element::CLEF) {
                                    if (needTick) {
                                          xml.tag("tick", segment->tick() - xml.tickDiff);
                                          xml.curTick = segment->tick();
                                          needTick = false;
                                          }
                                    e->write(xml);
                                    }
                              }
                        continue;
                        }
                  if (needTick) {
                        xml.tag("tick", segment->tick() - xml.tickDiff);
                        xml.curTick = segment->tick();
                        needTick = false;
                        }
                  if (e->isChordRest()) {
                        ChordRest* cr = static_cast<ChordRest*>(e);
                        cr->writeBeam(xml);
                        cr->writeTuplet(xml);
                        }
#if 0 // TODO MM
                  if ((segment->segmentType() == Segment::SegEndBarLine) && m && (m->multiMeasure() > 0)) {
                        xml.stag("BarLine");
                        xml.tag("subtype", m->endBarLineType());
                        xml.tag("visible", m->endBarLineVisible());
                        xml.etag();
                        }
                  else
#endif
                  e->write(xml);
                  segment->write(xml);    // write only once
                  }
            }
      }

//---------------------------------------------------------
//   searchTuplet
//    search complete Dom for tuplet id
//    last resort in case of error
//---------------------------------------------------------

Tuplet* Score::searchTuplet(XmlReader& /*e*/, int /*id*/)
      {
#if 0 // TODOx
      QDomElement e = de;
      QDomDocument doc = e.ownerDocument();

      QString tag;
      for (e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            tag = e.tagName();
            if (tag == "museScore")
                  break;
            }
      if (tag != "museScore") {
            qDebug("Score::searchTuplet():  no museScore found");
            return 0;
            }

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            tag = e.tagName();
            if (tag == "Score" || tag == "Part")
                  break;
            }
      if (tag != "Score" && tag != "Part") {
            qDebug("Score::searchTuplet():  no Score/Part found");
            return 0;
            }
      if (tag == "Score")
            e = e.firstChildElement();
      else
            e = e.nextSiblingElement();
      for (; !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "Staff") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "Measure") {
                              for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                                    if (eee.tagName() == "Tuplet") {
                                          Tuplet* tuplet = new Tuplet(this);
                                          QList<Spanner*> spannerList;
                                          QList<Tuplet*> tuplets;
                                          tuplet->read(eee);
                                          if (tuplet->id() == id)
                                                return tuplet;
                                          delete tuplet;
                                          }
                                    }
                              }
                        }
                  }
            }
#endif
      return 0;
      }

}

