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

static void writeMeasure(XmlWriter& xml, MeasureBase* m, int staffIdx, bool writeSystemElements)
      {
      //
      // special case multi measure rest
      //
      if (m->isMeasure() || staffIdx == 0)
            m->write(xml, staffIdx, writeSystemElements);

      if (m->score()->styleB(StyleIdx::createMultiMeasureRests) && m->isMeasure() && toMeasure(m)->mmRest())
            toMeasure(m)->mmRest()->write(xml, staffIdx, writeSystemElements);

      xml.setCurTick(m->endTick());
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

bool Score::write(XmlWriter& xml, bool selectionOnly)
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
      if (excerpt()) {
            Excerpt* e = excerpt();
            QMultiMap<int, int> trackList = e->tracks();
            QMapIterator<int, int> i(trackList);
            if (!(trackList.size() == e->parts().size() * VOICES) && !trackList.isEmpty()) {
                  while (i.hasNext()) {
                      i.next();
                      xml.tagE(QString("Tracklist sTrack=\"%1\" dstTrack=\"%2\"").arg(i.key()).arg(i.value()));
                      }
                  }
            }

      switch (_layoutMode) {
            case LayoutMode::PAGE:
            case LayoutMode::FLOAT:
            case LayoutMode::SYSTEM:
                  break;
            case LayoutMode::LINE:
                  xml.tag("layoutMode", "line");
                  break;
            }

#ifdef OMR
      if (masterScore()->omr() && xml.writeOmr())
            masterScore()->omr()->write(xml);
#endif
      if (isMaster() && masterScore()->showOmr() && xml.writeOmr())
            xml.tag("showOmr", masterScore()->showOmr());
      if (_audio && xml.writeOmr()) {
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
      xml.setCurTrack(-1);

      _style.save(xml, true);      // save only differences to buildin style

      xml.tag("showInvisible",   _showInvisible);
      xml.tag("showUnprintable", _showUnprintable);
      xml.tag("showFrames",      _showFrames);
      xml.tag("showMargins",     _showPageborders);

      QMapIterator<QString, QString> i(_metaTags);
      while (i.hasNext()) {
            i.next();
            // do not output "platform" and "creationDate" in test mode
            if ((!MScore::testMode  && !MScore::saveTemplateMode) || (i.key() != "platform" && i.key() != "creationDate"))
                  xml.tag(QString("metaTag name=\"%1\"").arg(i.key().toHtmlEscaped()), i.value());
            }

      if (!selectionOnly) {
            xml.stag("PageList");
            for (Page* page : _pages)
                  page->write(xml);
            xml.etag();
            }

      xml.setCurTrack(0);
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
      masterScore()->checkMidiMapping();
      for (const Part* part : _parts) {
            if (!selectionOnly || ((staffIdx(part) >= staffStart) && (staffEnd >= staffIdx(part) + part->nstaves())))
                  part->write(xml);
            }

      xml.setCurTrack(0);
      xml.setTrackDiff(-staffStart * VOICES);
      if (measureStart) {
            for (int staffIdx = staffStart; staffIdx < staffEnd; ++staffIdx) {
                  xml.stag(QString("Staff id=\"%1\"").arg(staffIdx + 1 - staffStart));
                  xml.setCurTick(measureStart->tick());
                  xml.setTickDiff(xml.curTick());
                  xml.setCurTrack(staffIdx * VOICES);
                  bool writeSystemElements = (staffIdx == staffStart);
                  for (MeasureBase* m = measureStart; m != measureEnd; m = m->next())
                        writeMeasure(xml, m, staffIdx, writeSystemElements);
                  xml.etag();
                  }
            }
      xml.setCurTrack(-1);
      if (isMaster()) {
            if (!selectionOnly) {
                  for (const Excerpt* excerpt : excerpts()) {
                        if (excerpt->partScore() != this)
                              excerpt->partScore()->write(xml, false);       // recursion
                        }
                  }
            }
      else
            xml.tag("name", excerpt()->title());
      xml.etag();

      if (unhide) {
            endCmd();
            undoRedo(true);   // undo
            }
      return true;
      }

//---------------------------------------------------------
//   Staff
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
                        //
                        // inherit timesig from previous measure
                        //
                        Measure* m = e.lastMeasure(); // measure->prevMeasure();
                        Fraction f(m ? m->timesig() : Fraction(4,4));
                        measure->setLen(f);
                        measure->setTimesig(f);

                        measure->read(e, staff);
                        measure->checkMeasure(staff);
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
                        measure->checkMeasure(staff);
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

bool MasterScore::saveFile()
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
      bool rv = suffix == "mscx" ? Score::saveFile(&temp, false) : Score::saveCompressedFile(&temp, info, false);
      if (!rv) {
            return false;
            }

      if (temp.error() != QFile::NoError) {
            MScore::lastError = tr("MuseScore: Save File failed: %1").arg(temp.errorString());
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

      undoStack()->setClean();
      setSaved(true);
      info.refresh();
      update();
      return true;
      }

//---------------------------------------------------------
//   saveCompressedFile
//---------------------------------------------------------

bool Score::saveCompressedFile(QFileInfo& info, bool onlySelection)
      {
      QFile fp(info.filePath());
      if (!fp.open(QIODevice::WriteOnly)) {
            MScore::lastError = tr("Open File\n%1\nfailed: ")
               + QString(strerror(errno));
            return false;
            }
      return saveCompressedFile(&fp, info, onlySelection);
      }

//---------------------------------------------------------
//   createThumbnail
//---------------------------------------------------------

QImage Score::createThumbnail()
      {
      LayoutMode mode = layoutMode();
      setLayoutMode(LayoutMode::PAGE);
      doLayout();

      Page* page = pages().at(0);
      QRectF fr  = page->abbox();
      qreal mag  = 256.0 / qMax(fr.width(), fr.height());
      int w      = int(fr.width() * mag);
      int h      = int(fr.height() * mag);

      QImage pm(w, h, QImage::Format_ARGB32_Premultiplied);

      int dpm = lrint(DPMM * 1000.0);
      pm.setDotsPerMeterX(dpm);
      pm.setDotsPerMeterY(dpm);
      pm.fill(0xffffffff);

      double pr = MScore::pixelRatio;
      MScore::pixelRatio = 1.0;

      QPainter p(&pm);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::TextAntialiasing, true);
      p.scale(mag, mag);
      print(&p, 0);
      p.end();

      MScore::pixelRatio = pr;

      if (layoutMode() != mode) {
            setLayoutMode(mode);
            doLayout();
            }
      return pm;
      }

//---------------------------------------------------------
//   saveCompressedFile
//    file is already opened
//---------------------------------------------------------

bool Score::saveCompressedFile(QIODevice* f, QFileInfo& info, bool onlySelection)
      {
      MQZipWriter uz(f);

      QString fn = info.completeBaseName() + ".mscx";
      QBuffer cbuf;
      cbuf.open(QIODevice::ReadWrite);
      XmlWriter xml(this, &cbuf);
      xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      xml.stag("container");
      xml.stag("rootfiles");
      xml.stag(QString("rootfile full-path=\"%1\"").arg(XmlWriter::xmlString(fn)));
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
      if (masterScore()->omr()) {
            int n = masterScore()->omr()->numPages();
            for (int i = 0; i < n; ++i) {
                  QString path = QString("OmrPages/page%1.png").arg(i+1);
                  QBuffer cbuf;
                  OmrPage* page = masterScore()->omr()->page(i);
                  const QImage& image = page->image();
                  if (!image.save(&cbuf, "PNG")) {
                        MScore::lastError = tr("save file: cannot save image (%1x%2)").arg(image.width()).arg(image.height());
                        return false;
                        }
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
      return true;
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

      XmlWriter xml(this, &f);
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

bool Score::saveFile(QIODevice* f, bool msczFormat, bool onlySelection)
      {
      if (!MScore::testMode)
            MScore::testMode = enableTestMode;
      XmlWriter xml(this, f);
      xml.setWriteOmr(msczFormat);
      xml.header();
      if (!MScore::testMode) {
            xml.stag("museScore version=\"" MSC_VERSION "\"");
            xml.tag("programVersion", VERSION);
            xml.tag("programRevision", revision);
            }
      else {
            xml.stag("museScore version=\"3.00\"");
            }
      if (!write(xml, onlySelection))
            return false;
      xml.etag();
      if (isMaster())
            masterScore()->revisions()->write(xml);
      if (!onlySelection) {
            //update version values for i.e. plugin access
            _mscoreVersion = VERSION;
            _mscoreRevision = revision.toInt();
            _mscVersion = MSCVERSION;
            }
      return true;
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

      XmlReader e(0, cbuf);

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

Score::FileError MasterScore::loadCompressedMsc(QIODevice* io, bool ignoreVersionError)
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
            QList<MQZipReader::FileInfo> fil = uz.fileInfoList();
            foreach(const MQZipReader::FileInfo& fi, fil) {
                  if (fi.filePath.endsWith(".mscx")) {
                        dbuf = uz.fileData(fi.filePath);
                        break;
                        }
                  }
            }
      XmlReader e(this, dbuf);
      e.setDocName(masterScore()->fileInfo()->completeBaseName());

      FileError retval = read1(e, ignoreVersionError);

#ifdef OMR
      //
      // load OMR page images
      //
      if (masterScore()->omr()) {
            int n = masterScore()->omr()->numPages();
            for (int i = 0; i < n; ++i) {
                  QString path = QString("OmrPages/page%1.png").arg(i+1);
                  QByteArray dbuf = uz.fileData(path);
                  OmrPage* page = masterScore()->omr()->page(i);
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
      if (audio()) {
            QByteArray dbuf = uz.fileData("audio.ogg");
            audio()->setData(dbuf);
            }
      return retval;
      }

//---------------------------------------------------------
//   loadMsc
//    return true on success
//---------------------------------------------------------

Score::FileError MasterScore::loadMsc(QString name, bool ignoreVersionError)
      {
      fileInfo()->setFile(name);

      QFile f(name);
      if (!f.open(QIODevice::ReadOnly)) {
            MScore::lastError = f.errorString();
            return FileError::FILE_OPEN_ERROR;
            }

      if (name.endsWith(".mscz"))
            return loadCompressedMsc(&f, ignoreVersionError);
      else {
            XmlReader r(this, &f);
            return read1(r, ignoreVersionError);
            }
      }

Score::FileError MasterScore::loadMsc(QString name, QIODevice* io, bool ignoreVersionError)
      {
      fileInfo()->setFile(name);

      if (name.endsWith(".mscz"))
            return loadCompressedMsc(io, ignoreVersionError);
      else {
            XmlReader r(this, io);
            return read1(r, ignoreVersionError);
            }
      }

//---------------------------------------------------------
//   parseVersion
//---------------------------------------------------------

void MasterScore::parseVersion(const QString& val)
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

Score::FileError MasterScore::read1(XmlReader& e, bool ignoreVersionError)
      {
      while (e.readNextStartElement()) {
            if (e.name() == "museScore") {
                  const QString& version = e.attribute("version");
                  QStringList sl = version.split('.');
                  setMscVersion(sl[0].toInt() * 100 + sl[1].toInt());

                  if (!ignoreVersionError) {
                        QString message;
                        if (mscVersion() > MSCVERSION)
                              return FileError::FILE_TOO_NEW;
                        if (mscVersion() < 114)
                              return FileError::FILE_TOO_OLD;
                        }
                  Score::FileError error;
                  if (mscVersion() == 114)
                        error = read114(e);
                  else if (mscVersion() <= 206)
                        error = read206(e);
                  else
                        error = read300(e);
                  setExcerptsChanged(false);
                  return error;
                  }
            else
                  e.unknown();
            }
      return FileError::FILE_CORRUPTED;
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
      for (const Element* e : ell) {
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

QByteArray MasterScore::readCompressedToBuffer()
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

QByteArray MasterScore::readToBuffer()
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

void Score::writeSegments(XmlWriter& xml, int strack, int etrack,
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
                  if (!segment->enabled())
                        continue;
                  if (track == 0)
                        segment->setWritten(false);
                  Element* e = segment->element(track);
                  //
                  // special case: - barline span > 1
                  //               - part (excerpt) staff starts after
                  //                 barline element
                  bool needTick = (needFirstTick && segment == fs) || (segment->tick() != xml.curTick());
                  if ((segment->isEndBarLineType()) && !e && writeSystemElements && ((track % VOICES) == 0)) {
                        // search barline:
                        for (int idx = track - VOICES; idx >= 0; idx -= VOICES) {
                              if (segment->element(idx)) {
                                    int oDiff = xml.trackDiff();
                                    xml.setTrackDiff(idx);          // staffIdx should be zero
                                    segment->element(idx)->write(xml);
                                    xml.setTrackDiff(oDiff);
                                    break;
                                    }
                              }
                        }
                  for (Element* e : segment->annotations()) {
                        if (e->track() != track || e->generated() || (e->systemFlag() && !writeSystemElements))
                              continue;
                        if (needTick) {
                              // xml.tag("tick", segment->tick() - xml.tickDiff);
                              int tick = xml.clipboardmode() ? segment->tick() : segment->rtick();
                              xml.tag("move", Fraction::fromTicks(tick + xml.tickDiff()));
                              xml.setCurTick(segment->tick());
                              needTick = false;
                              }
                        e->write(xml);
                        }
                  Measure* m = segment->measure();
                  // don't write spanners for multi measure rests
                  if ((!(m && m->isMMRest())) && segment->isChordRestType()) {
                        auto endIt = spanner().upper_bound(endTick);
                        for (auto i = spanner().begin(); i != endIt; ++i) {
                              Spanner* s = i->second;
                              if (s->generated() || !xml.canWrite(s))
                                    continue;

                              // don't write voltas to clipboard
                              if (clip && s->isVolta())
                                    continue;

                              if (s->track() == track) {
                                    bool end = false;
                                    if (s->anchor() == Spanner::Anchor::CHORD || s->anchor() == Spanner::Anchor::NOTE)
                                          end = s->tick2() < endTick;
                                    else
                                          end = s->tick2() <= endTick;
                                    if (s->tick() == segment->tick() && (!clip || end)) {
                                          if (needTick) {
                                                // xml.tag("tick", segment->tick() - xml.tickDiff);
                                                int tick = xml.clipboardmode() ? segment->tick() : segment->rtick();
                                                xml.tag("move", Fraction::fromTicks(tick + xml.tickDiff()));
                                                xml.setCurTick(segment->tick());
                                                needTick = false;
                                                }
                                          s->write(xml);
                                          }
                                    }
                              if ((s->tick2() == segment->tick())
                                 && !s->isSlur()
                                 && (s->track2() == track || (s->track2() == -1 && s->track() == track))
                                 && (!clip || s->tick() >= fs->tick())
                                 ) {
                                    if (needTick) {
                                          // xml.tag("tick", segment->tick() - xml.tickDiff);
                                          int tick = xml.clipboardmode() ? segment->tick() : segment->rtick();
                                          xml.tag("move", Fraction::fromTicks(tick + xml.tickDiff()));
                                          xml.setCurTick(segment->tick());
                                          needTick = false;
                                          }
                                    xml.tagE(QString("endSpanner id=\"%1\"").arg(xml.spannerId(s)));
                                    }
                              }
                        }

                  if (!e || !xml.canWrite(e))
                        continue;
                  if (e->generated())
                        continue;
                  if (needTick) {
                        // xml.tag("tick", segment->tick() - xml.tickDiff);
                        int tick = xml.clipboardmode() ? segment->tick() : segment->rtick();
                        xml.tag("move", Fraction::fromTicks(tick + xml.tickDiff()));
                        xml.setCurTick(segment->tick());
                        needTick = false;
                        }
                  if (e->isChordRest()) {
                        ChordRest* cr = toChordRest(e);
                        cr->writeBeam(xml);
                        cr->writeTuplet(xml);
                        }
//                  if (segment->isEndBarLine() && (m->mmRestCount() < 0 || m->mmRest())) {
//                        BarLine* bl = toBarLine(e);
//TODO                        bl->setBarLineType(m->endBarLineType());
//                        bl->setVisible(m->endBarLineVisible());
//                        }
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
                          && s->isSlur()
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

