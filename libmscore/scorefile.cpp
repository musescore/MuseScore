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
#include "sym.h"

#ifdef OMR
#include "omr/omr.h"
#include "omr/omrpage.h"
#endif

#include "sig.h"
#include "undo.h"
#include "imageStore.h"
#include "audio.h"
#include "barline.h"
#include "thirdparty/qzip/qzipreader_p.h"
#include "thirdparty/qzip/qzipwriter_p.h"
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
      //
      // special case multi measure rest
      //
      Measure* mm = 0;
      if (m->score()->styleB(StyleIdx::createMultiMeasureRests) && m->type() == Element::Type::MEASURE) {
            mm = static_cast<Measure*>(m);
            Segment* s = mm->findSegment(Segment::Type::EndBarLine, mm->endTick());
            if (s == 0)
                  mm->createEndBarLines();
            }

      if (m->type() == Element::Type::MEASURE || staffIdx == 0)
            m->write(xml, staffIdx, writeSystemElements);

      if (mm && mm->mmRest()) {
            mm->mmRest()->write(xml, staffIdx, writeSystemElements);
            xml.tag("tick", mm->tick() + mm->ticks());         // rewind tick
            }

      if (m->type() == Element::Type::MEASURE)
            xml.curTick = m->tick() + m->ticks();
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Score::write(Xml& xml, bool selectionOnly)
      {
      // if we have multi measure rests and some parts are hidden,
      // then some layout information is missing:
      // relayout with all parts set visible

      QList<Part*> hiddenParts;
      bool unhide = false;
      if (styleB(StyleIdx::createMultiMeasureRests)) {
            for (Part* part : _parts) {
                  if (!part->show()) {
                        if (!unhide) {
                              startCmd();
                              unhide = true;
                              }
                        part->undoChangeProperty(P_ID::VISIBLE, true);
                        hiddenParts.append(part);
                        }
                  }
            }
      if (unhide) {
            doLayout();
            for (Part* p : hiddenParts)
                  p->setShow(false);
            }

      xml.stag("Score");
      switch(_layoutMode) {
            case LayoutMode::PAGE:
            case LayoutMode::FLOAT:
            case LayoutMode::SYSTEM:
                  break;
            case LayoutMode::LINE:
                  xml.tag("layoutMode", "line");
                  break;
            }

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

      xml.tag("showInvisible", _showInvisible);
      xml.tag("showUnprintable", _showUnprintable);
      xml.tag("showFrames", _showFrames);
      xml.tag("showMargins", _showPageborders);

      QMapIterator<QString, QString> i(_metaTags);
      while (i.hasNext()) {
            i.next();
            if ((!MScore::testMode  && !MScore::saveTemplateMode) || (i.key() != "platform" && i.key() != "creationDate"))
                  xml.tag(QString("metaTag name=\"%1\"").arg(i.key().toHtmlEscaped()), i.value());
            }

      if (!selectionOnly) {
            xml.stag("PageList");
            foreach(Page* page, _pages)
                  page->write(xml);
            xml.etag();
            }

      xml.curTrack = 0;
      int staffStart;
      int staffEnd;
      MeasureBase* measureStart;
      MeasureBase* measureEnd;

      if (selectionOnly) {
            staffStart   = _selection.staffStart();
            staffEnd     = _selection.staffEnd();
            // make sure we select full parts
            Staff* sStaff = staff(staffStart);
            Part* sPart = sStaff->part();
            Staff* eStaff = staff(staffEnd - 1);
            Part* ePart = eStaff->part();
            staffStart = staffIdx(sPart);
            staffEnd = staffIdx(ePart) + ePart->nstaves();
            measureStart = _selection.startSegment()->measure();
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

      // Let's decide: write midi mapping to a file or not
      checkMidiMapping();
      foreach(const Part* part, _parts) {
            if (!selectionOnly || ((staffIdx(part) >= staffStart) && (staffEnd >= staffIdx(part) + part->nstaves())))
                  part->write(xml);
            }

      xml.curTrack = 0;
      xml.trackDiff = -staffStart * VOICES;
      if (measureStart) {
            for (int staffIdx = staffStart; staffIdx < staffEnd; ++staffIdx) {
                  xml.stag(QString("Staff id=\"%1\"").arg(staffIdx + 1 - staffStart));
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
            for (const Excerpt* excerpt : _excerpts) {
                  if (excerpt->partScore() != this)
                        excerpt->partScore()->write(xml, false);       // recursion
                  }
            }
      if (parentScore())
            xml.tag("name", name());
      xml.etag();

      if (unhide) {
            endCmd();
            undo()->undo();
            endUndoRedo();
            }
      }

//---------------------------------------------------------
//   readStaff
//---------------------------------------------------------

void Score::readStaff(XmlReader& e)
      {
      int staff = e.intAttribute("id", 1) - 1;
      e.initTick(0);
      e.setTrack(staff * VOICES);

      if (staff == 0) {
            while (e.readNextStartElement()) {
                  const QStringRef& tag(e.name());

                  if (tag == "Measure") {
                        Measure* measure = 0;
                        measure = new Measure(this);
                        measure->setTick(e.tick());
                        if (_mscVersion < 115) {
                              const SigEvent& ev = sigmap()->timesig(measure->tick());
                              measure->setLen(ev.timesig());
                              measure->setTimesig(ev.nominal());
                              }
                        else {
                              //
                              // inherit timesig from previous measure
                              //
                              Measure* m = e.lastMeasure(); // measure->prevMeasure();
                              Fraction f(m ? m->timesig() : Fraction(4,4));
                              measure->setLen(f);
                              measure->setTimesig(f);
                              }
                        measure->read(e, staff);
                        if (!measure->isMMRest()) {
                              measures()->add(measure);
                              e.setLastMeasure(measure);
                              e.initTick(measure->tick() + measure->ticks());
                              }
                        else {
                              // this is a multi measure rest
                              // always preceded by the first measure it replaces
                              Measure* m = e.lastMeasure();

                              if (m) {
                                    m->setMMRest(measure);
                                    measure->setTick(m->tick());
                                    }
                              }
                        }
                  else if (tag == "HBox" || tag == "VBox" || tag == "TBox" || tag == "FBox") {
                        MeasureBase* mb = static_cast<MeasureBase*>(Element::name2Element(tag, this));
                        mb->read(e);
                        mb->setTick(e.tick());
                        measures()->add(mb);
                        }
                  else if (tag == "tick")
                        e.initTick(fileDivision(e.readInt()));
                  else
                        e.unknown();
                  }
            }
      else {
            Measure* measure = firstMeasure();
            while (e.readNextStartElement()) {
                  const QStringRef& tag(e.name());

                  if (tag == "Measure") {
                        if (measure == 0) {
                              qDebug("Score::readStaff(): missing measure!");
                              measure = new Measure(this);
                              measure->setTick(e.tick());
                              measures()->add(measure);
                              }
                        e.initTick(measure->tick());
                        measure->read(e, staff);
                        if (measure->isMMRest())
                              measure = e.lastMeasure()->nextMeasure();
                        else {
                              e.setLastMeasure(measure);
                              if (measure->mmRest())
                                    measure = measure->mmRest();
                              else
                                    measure = measure->nextMeasure();
                              }
                        }
                  else if (tag == "tick")
                        e.initTick(fileDivision(e.readInt()));
                  else
                        e.unknown();
                  }
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
            MScore::lastError = tr("The following file is locked: \n%1 \n\nTry saving to a different location.").arg(info.filePath());
            return false;
            }
      //
      // step 1
      // save into temporary file to prevent partially overwriting
      // the original file in case of "disc full"
      //

      QString tempName = info.filePath() + QString(".temp");
      QFile temp(tempName);
      if (!temp.open(QIODevice::WriteOnly)) {
            MScore::lastError = tr("Open Temp File\n%1\nfailed: %2").arg(tempName).arg(QString(strerror(errno)));
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
            MScore::lastError = tr("MuseScore: Save File failed: %1").arg(temp.errorString());
            temp.close();
            return false;
            }
      temp.close();

      QString name(info.filePath());
      QDir dir(info.path());
      if (!saved()) {
            // if file was already saved in this session
            // save but don't overwrite backup again

            //
            // step 2
            // remove old backup file if exists
            //
            QString backupName = QString(".") + info.fileName() + QString(",");
            if (dir.exists(backupName)) {
                  if (!dir.remove(backupName)) {
//                      if (!MScore::noGui)
//                            QMessageBox::critical(0, tr("MuseScore: Save File"),
//                               tr("Removing old backup file ") + backupName + tr(" failed"));
                        }
                  }

            //
            // step 3
            // rename old file into backup
            //
            if (dir.exists(name)) {
                  if (!dir.rename(name, backupName)) {
//                      if (!MScore::noGui)
//                            QMessageBox::critical(0, tr("MuseScore: Save File"),
//                               tr("Renaming old file <")
//                               + name + tr("> to backup <") + backupName + tr("> failed"));
                        }
                  }
#ifdef Q_OS_WIN
            QFileInfo fileBackup(dir, backupName);
            QString backupNativePath = QDir::toNativeSeparators(fileBackup.absoluteFilePath());
            SetFileAttributes((LPCTSTR)backupNativePath.toLocal8Bit(), FILE_ATTRIBUTE_HIDDEN);
#endif
            }
      else {
            // file has previously been saved - remove the old file
            if (dir.exists(name)) {
                  if (!dir.remove(name)) {
//                      if (!MScore::noGui)
//                            QMessageBox::critical(0, tr("MuseScore: Save File"),
//                               tr("Removing old file") + name + tr(" failed"));
                        }
                  }
            }

      //
      // step 4
      // rename temp name into file name
      //
      if (!QFile::rename(tempName, name)) {
            MScore::lastError = tr("Renaming temp. file <%1> to <%2> failed:\n%3").arg(tempName).arg(name).arg(QString(strerror(errno)));
            return false;
            }
      // make file readable by all
      QFile::setPermissions(name, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser
         | QFile::ReadGroup | QFile::ReadOther);

      undo()->setClean();
      setSaved(true);
      info.refresh();
      update();
      return true;
      }

//---------------------------------------------------------
//   saveCompressedFile
//---------------------------------------------------------

void Score::saveCompressedFile(QFileInfo& info, bool onlySelection)
      {
      QFile fp(info.filePath());
      if (!fp.open(QIODevice::WriteOnly)) {
            QString s = tr("Open File\n%1\nfailed: ")
               + QString(strerror(errno));
            throw(s.arg(info.filePath()));
            }
      saveCompressedFile(&fp, info, onlySelection);
      fp.close();
      }

//---------------------------------------------------------
//   createThumbnail
//---------------------------------------------------------

QImage Score::createThumbnail()
      {
      LayoutMode layoutMode = _layoutMode;
      switchToPageMode();
      Page* page = pages().at(0);
      QRectF fr  = page->abbox();
      qreal mag  = 256.0 / qMax(fr.width(), fr.height());
      int w      = int(fr.width() * mag);
      int h      = int(fr.height() * mag);

      QImage pm(w, h, QImage::Format_ARGB32_Premultiplied);
      pm.setDotsPerMeterX(lrint((mag * 1000) / INCH));
      pm.setDotsPerMeterY(lrint((mag * 1000) / INCH));
      pm.fill(0xffffffff);
      QPainter p(&pm);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::TextAntialiasing, true);
      p.scale(mag, mag);
      print(&p, 0);
      p.end();
      if (layoutMode != _layoutMode)
            endCmd(true);       // rollback
      return pm;
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
      foreach (ImageStoreItem* ip, imageStore) {
            if (!ip->isUsed(this))
                  continue;
            QString path = QString("Pictures/") + ip->hashName();
            uz.addFile(path, ip->buffer());
            }

      // create thumbnail
      QImage pm = createThumbnail();

      QByteArray ba;
      QBuffer b(&ba);
      if (!b.open(QIODevice::WriteOnly))
            qDebug("open buffer failed");
      if (!pm.save(&b, "PNG"))
            qDebug("save failed");
      uz.addFile("Thumbnails/thumbnail.png", ba);

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
            MScore::lastError = tr("Open File\n%1\nfailed: %2").arg(info.filePath()).arg(QString(strerror(errno)));
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
            MScore::lastError = tr("Open Style File\n%1\nfailed: %2").arg(f.fileName().arg(QString(strerror(errno))));
            return false;
            }

      Xml xml(&f);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      _style.save(xml, false);     // save complete style
      xml.etag();
      if (f.error() != QFile::NoError) {
            MScore::lastError = tr("Write Style failed: %1").arg(f.errorString());
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
      if (!MScore::testMode) {
            xml.stag("museScore version=\"" MSC_VERSION "\"");
            xml.tag("programVersion", VERSION);
            xml.tag("programRevision", revision);
            }
      else {
            xml.stag("museScore version=\"2.00\"");
            }
      write(xml, onlySelection);
      xml.etag();
      if (!parentScore())
            _revisions->write(xml);
      if (!onlySelection) {
            //update version values for i.e. plugin access
            _mscoreVersion = VERSION;
            _mscoreRevision = revision.toInt();
            _mscVersion = MSCVERSION;
            }
      }

//---------------------------------------------------------
//   readRootFile
//---------------------------------------------------------

QString readRootFile(MQZipReader* uz, QList<QString>& images)
      {
      QString rootfile;

      QByteArray cbuf = uz->fileData("META-INF/container.xml");
      if (cbuf.isEmpty()) {
            qDebug("can't find container.xml");
            return rootfile;
            }

      XmlReader e(cbuf);

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
                              if (rootfile.isEmpty()) {
                                    rootfile = e.attribute("full-path");
                                    e.skipCurrentElement();
                                    }
                              }
                        else if (tag == "file")
                              images.append(e.readElementText());
                        else
                              e.unknown();
                        }
                  }
            }
      return rootfile;
      }

//---------------------------------------------------------
//   loadCompressedMsc
//    return false on error
//---------------------------------------------------------

Score::FileError Score::loadCompressedMsc(QIODevice* io, bool ignoreVersionError)
      {
      MQZipReader uz(io);

      QList<QString> sl;
      QString rootfile = readRootFile(&uz, sl);
      if (rootfile.isEmpty())
            return FileError::FILE_NO_ROOTFILE;

      //
      // load images
      //
      if (!MScore::noImages) {
            foreach(const QString& s, sl) {
                  QByteArray dbuf = uz.fileData(s);
                  imageStore.add(s, dbuf);
                  }
            }

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
      XmlReader e(dbuf);
      e.setDocName(info.completeBaseName());

      FileError retval = read1(e, ignoreVersionError);

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

      QFile f(name);
      if (!f.open(QIODevice::ReadOnly)) {
            MScore::lastError = f.errorString();
            return FileError::FILE_OPEN_ERROR;
            }

      if (name.endsWith(".mscz"))
            return loadCompressedMsc(&f, ignoreVersionError);
      else {
            XmlReader r(&f);
            return read1(r, ignoreVersionError);
            }
      }

Score::FileError Score::loadMsc(QString name, QIODevice* io, bool ignoreVersionError)
      {
      info.setFile(name);

      if (name.endsWith(".mscz"))
            return loadCompressedMsc(io, ignoreVersionError);
      else {
            XmlReader r(io);
            return read1(r, ignoreVersionError);
            }
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
                                    qDebug("read future version");
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
                                          qDebug("read future version");
                                          }
                                    }
                              }
                        else
                              qDebug("1cannot parse <%s>", qPrintable(val));
                        }
                  }
            }
      else
            qDebug("2cannot parse <%s>", VERSION);
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
                              return FileError::FILE_TOO_NEW;
                        if (_mscVersion < 114)
                              return FileError::FILE_TOO_OLD;
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
                                    return FileError::FILE_BAD_FORMAT;
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
#if 0
      // check all spanners for missing end
      QList<Spanner*> sl;
      for (auto i = _spanner.cbegin(); i != _spanner.cend(); ++i) {
            if (i->second->ticks() == 0)
                  sl.append(i->second);
            }
      int lastTick = lastMeasure()->endTick();
      for (Spanner* s : sl) {
            s->setTick2(lastTick);
            _spanner.removeSpanner(s);
            _spanner.addSpanner(s);
            }
#endif
      for (Staff* s : _staves)
            s->updateOttava();

      setCreated(false);
      return FileError::FILE_NO_ERROR;
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
            else if (tag == "KeySig")           // obsolete
                  e.skipCurrentElement();
            else if (tag == "StaffType") {      // obsolete
#if 0
                  int idx           = e.intAttribute("idx");
                  QString groupName = e.attribute("group", "pitched");

                  int group;
                  // staff type numbering did change!
                  // attempt to keep some compatibility with existing 2.0 scores
                  if (groupName == "percussion")
                        group = StaffGroup::PERCUSSION;
                  else if (groupName == "tablature")
                        group = StaffGroup::TAB;
                  else
                        group = StaffGroup::STANDARD;

                  StaffType* ost = staffType(idx);
                  StaffType* st;
                  if (ost && ost->group() == group)
                        st = new StaffType(*ost);
                  else {
                        idx = -1;
                        st = new StaffType;
                        }
#endif
                  StaffType st;
                  st.read(e);
                  e.staffType().append(st);
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
                  e.readNext();
                  }
            else if (tag == "currentLayer")
                  _currentLayer = e.readInt();
            else if (tag == "SyntiSettings")    // obsolete
                  _synthesizerState.read(e);
            else if (tag == "Synthesizer")
                  _synthesizerState.read(e);
            else if (tag == "Spatium")
                  _style.setSpatium (e.readDouble() * DPMM); // obsolete, moved to Style
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
                  // if (_layoutMode == LayoutMode::FLOAT || _layoutMode == LayoutMode::SYSTEM) {
                  if (_layoutMode == LayoutMode::FLOAT) {
                        // style should not change spatium in
                        // float mode
                        _style.setSpatium(sp);
                        }
                  _scoreFont = ScoreFont::fontFactory(_style.value(StyleIdx::MusicalSymbolFont).toString());
                  }
            else if (tag == "copyright" || tag == "rights") {
                  Text* text = new Text(this);
                  text->read(e);
                  setMetaTag("copyright", text->xmlText());
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
                  if (MScore::noExcerpts)
                        e.skipCurrentElement();
                  else {
                        Excerpt* ex = new Excerpt(this);
                        ex->read(e);
                        _excerpts.append(ex);
                        }
                  }
            else if (tag == "Beam") {
                  Beam* beam = new Beam(this);
                  beam->read(e);
                  beam->setParent(0);
                  // _beams.append(beam);
                  }
            else if (tag == "Score") {          // recursion
                   if (MScore::noExcerpts)
                        e.skipCurrentElement();
                  else {
                        Score* s = new Score(this, MScore::baseStyle());
                        s->read(e);
                        addExcerpt(s);
                        }
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
            else if (tag == "name") {
                  QString n = e.readElementText();
                  if (parentScore()) //ignore the name if it's not a child score
                        setName(n);
                  }
            else if (tag == "page-layout") {    // obsolete
                  if (_layoutMode != LayoutMode::FLOAT && _layoutMode != LayoutMode::SYSTEM) {
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
            else if (tag == "layoutMode") {
                  QString s = e.readElementText();
                  if (s == "line")
                        _layoutMode = LayoutMode::LINE;
                  else if (s == "system")
                        _layoutMode = LayoutMode::SYSTEM;
                  else
                        qDebug("layoutMode: %s", qPrintable(s));
                  }
            else
                  e.unknown();
            }
      if (e.error() != XmlStreamReader::NoError) {
            qDebug("%s: xml read error at line %lld col %lld: %s",
               qPrintable(e.getDocName()), e.lineNumber(), e.columnNumber(),
               e.name().toUtf8().data());
            MScore::lastError = tr("XML read error at line %1 column %2: %3").arg(e.lineNumber()).arg(e.columnNumber()).arg(e.name().toString());
            return false;
            }

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
                  qDebug("bad span: idx %d  span %d staves %d", idx, barLineSpan, n);
                  // span until last staff
                  barLineSpan = n - idx;
                  st->setBarLineSpan(barLineSpan);
                  }
            else if (idx == 0 && barLineSpan == 0) {
                  qDebug("bad span: idx %d  span %d staves %d", idx, barLineSpan, n);
                  // span from the first staff until the start of the next span
                  barLineSpan = 1;
                  for (int i = 1; i < n; ++i) {
                        if (staff(i)->barLineSpan() == 0)
                              ++barLineSpan;
                        else
                              break;
                        }
                  st->setBarLineSpan(barLineSpan);
                  }
            // check spanFrom
            int minBarLineFrom = st->lines() == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : MIN_BARLINE_SPAN_FROMTO;
            if (st->barLineFrom() < minBarLineFrom)
                  st->setBarLineFrom(minBarLineFrom);
            if (st->barLineFrom() > st->lines() * 2)
                  st->setBarLineFrom(st->lines() * 2);
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
                  if (st->barLineTo() - st->barLineFrom() < MIN_BARLINE_FROMTO_DIST) {
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
      MScore::pdfPrinting = true;
      Page* page = pages().at(pageNo);
      QRectF fr  = page->abbox();

      QList<Element*> ell = page->items(fr);
      qStableSort(ell.begin(), ell.end(), elementLessThan);
      foreach(const Element* e, ell) {
            if (!e->visible())
                  continue;
            painter->save();
            painter->translate(e->pagePos());
            e->draw(painter);
            painter->restore();
            }
      MScore::pdfPrinting = false;
      _printing = false;
      }

//---------------------------------------------------------
//   readCompressedToBuffer
//---------------------------------------------------------

QByteArray Score::readCompressedToBuffer()
      {
      MQZipReader uz(info.filePath());
      if (!uz.exists()) {
            qDebug("Score::readCompressedToBuffer: cannot read zip file");
            return QByteArray();
            }
      QList<QString> images;
      QString rootfile = readRootFile(&uz, images);

      //
      // load images
      //
      foreach(const QString& s, images) {
            QByteArray dbuf = uz.fileData(s);
            imageStore.add(s, dbuf);
            }

      if (rootfile.isEmpty()) {
            qDebug("=can't find rootfile in: %s", qPrintable(info.filePath()));
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
            QFile f(info.filePath());
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
qDebug("createRevision");
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

//      qDebug("patch:\n%s\n==========", qPrintable(patch));
      //
#endif
      }

//---------------------------------------------------------
//   writeSegments
//    ls  - write upto this segment (excluding)
//          can be zero
//---------------------------------------------------------

void Score::writeSegments(Xml& xml, int strack, int etrack,
   Segment* fs, Segment* ls, bool writeSystemElements, bool clip, bool needFirstTick)
      {
      int endTick = ls == 0 ? lastMeasure()->endTick() : ls->tick();
      // in clipboard mode, ls might be in an mmrest
      // since we are traversing regular measures,
      // force ls to last segment of the corresponding regular measure
      // if it is not in same measure as fs
      Measure* lm = ls ? ls->measure() : 0;
      if (clip && lm && lm->isMMRest() && lm != fs->measure()) {
            lm = tick2measure(ls->measure()->tick());
            if (lm)
                  ls = lm->last();
            else
                  qDebug("writeSegments: no measure for end segment in mmrest");
            }
      for (int track = strack; track < etrack; ++track) {
            if (!xml.canWriteVoice(track))
                  continue;
            for (Segment* segment = fs; segment && segment != ls; segment = segment->next1()) {
                  if (track == 0)
                        segment->setWritten(false);
                  Element* e = segment->element(track);
                  //
                  // special case: - barline span > 1
                  //               - part (excerpt) staff starts after
                  //                 barline element
                  bool needTick = (needFirstTick && segment == fs) || (segment->tick() != xml.curTick);
                  if ((segment->segmentType() == Segment::Type::EndBarLine)
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
                  for (Element* e : segment->annotations()) {
                        if (e->track() != track || e->generated()
                           || (e->systemFlag() && !writeSystemElements)) {
                              continue;
                              }
                        if (needTick) {
                              xml.tag("tick", segment->tick() - xml.tickDiff);
                              xml.curTick = segment->tick();
                              needTick = false;
                              }
                        e->write(xml);
                        }
                  Measure* m = segment->measure();
                  // don't write spanners for multi measure rests
                  if ((!(m && m->isMMRest())) && (segment->segmentType() & Segment::Type::ChordRest)) {
                        auto endIt = spanner().upper_bound(endTick);
                        for (auto i = spanner().begin(); i != endIt; ++i) {
                              Spanner* s = i->second;
                              if (s->generated() || !xml.canWrite(s))
                                    continue;

                              // don't write voltas to clipboard
                              if (clip && s->type() == Element::Type::VOLTA)
                                    continue;

                              if (s->track() == track) {
                                    bool end = false;
                                    if (s->anchor() == Spanner::Anchor::CHORD || s->anchor() == Spanner::Anchor::NOTE)
                                          end = s->tick2() < endTick;
                                    else
                                          end = s->tick2() <= endTick;
                                    if (s->tick() == segment->tick() && (!clip || end)) {
                                          if (needTick) {
                                                xml.tag("tick", segment->tick() - xml.tickDiff);
                                                xml.curTick = segment->tick();
                                                needTick = false;
                                                }
                                          s->write(xml);
                                          }
                                    }
                              if ((s->tick2() == segment->tick())
                                 && s->type() != Element::Type::SLUR
                                 && (s->track2() == track || (s->track2() == -1 && s->track() == track))
                                 && (!clip || s->tick() >= fs->tick())
                                 ) {
                                    if (needTick) {
                                          xml.tag("tick", segment->tick() - xml.tickDiff);
                                          xml.curTick = segment->tick();
                                          needTick = false;
                                          }
                                    xml.tagE(QString("endSpanner id=\"%1\"").arg(xml.spannerId(s)));
                                    }
                              }
                        }

                  if (!e || !xml.canWrite(e))
                        continue;

                  if (e->generated()) {
#if 0
                        if ((xml.curTick - xml.tickDiff) == 0) {
                              if (e->type() == Element::Type::CLEF) {
                                    if (needTick) {
                                          xml.tag("tick", segment->tick() - xml.tickDiff);
                                          xml.curTick = segment->tick();
                                          needTick = false;
                                          }
                                    e->write(xml);
                                    }
                              }
#endif
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
                  if ((segment->segmentType() == Segment::Type::EndBarLine) && (m->mmRestCount() < 0 || m->mmRest())) {
                        BarLine* bl = static_cast<BarLine*>(e);
                        if (styleB(StyleIdx::createMultiMeasureRests))
                              bl->setBarLineType(m->endBarLineType());
                        bl->setVisible(m->endBarLineVisible());
                        }
                  e->write(xml);
                  segment->write(xml);    // write only once
                  }
            //write spanner ending after the last segment, on the last tick
            if (clip || ls == 0) {
                  auto endIt = spanner().upper_bound(endTick);
                  for (auto i = spanner().begin(); i != endIt; ++i) {
                        Spanner* s = i->second;
                        if (s->generated() || !xml.canWrite(s))
                              continue;
                        if ((s->tick2() == endTick)
                          && s->type() != Element::Type::SLUR
                          && (s->track2() == track || (s->track2() == -1 && s->track() == track))
                          && (!clip || s->tick() >= fs->tick())
                          ) {
                              xml.tagE(QString("endSpanner id=\"%1\"").arg(xml.spannerId(s)));
                              }
                        }
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

