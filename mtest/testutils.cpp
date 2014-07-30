//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012-2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>
#include "config.h"
#include "libmscore/score.h"
#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "libmscore/instrtemplate.h"
#include "omr/omr.h"
#include "testutils.h"
#include "mscore/preferences.h"
#include "libmscore/page.h"
#include "synthesizer/msynthesizer.h"
#include "mscore/musescoreCore.h"
#include "mscore/shortcut.h"
#include "mscore/importmidi_operations.h"
#include "libmscore/xml.h"
#include "libmscore/excerpt.h"

namespace Ms {

#ifdef OMR
extern Score::FileError importPdf(Score*, const QString&);
#endif

extern Score::FileError importBB(Score*, const QString&);
extern Score::FileError importCapella(Score*, const QString&);
extern Score::FileError importCapXml(Score*, const QString&);
extern Score::FileError importCompressedMusicXml(Score*, const QString&);
extern Score::FileError importMusicXml(Score*, const QString&);
extern Score::FileError importGTP(Score*, const QString&);
extern bool saveXml(Score*, const QString&);
bool debugMode = false;
QString revision;
bool enableTestMode;

Score* score;
MuseScoreCore* mscoreCore;
MasterSynthesizer* synti;
QString dataPath;
QIcon* icons[0];
Shortcut Shortcut::sc[1] = { Shortcut() };
QString mscoreGlobalShare;

//---------------------------------------------------------
//   Preferences
//---------------------------------------------------------

Preferences preferences;
Preferences::Preferences()
      {
      }

//---------------------------------------------------------
//   writeReadElement
//    writes and element and reads it back
//---------------------------------------------------------

Element* MTest::writeReadElement(Element* element)
      {
      //
      // write element
      //
      QBuffer buffer;
      buffer.open(QIODevice::WriteOnly);
      Xml xml(&buffer);
      xml.header();
      element->write(xml);
      buffer.close();

      //
      // read element
      //
// printf("===read <%s>===\n", element->name());
// printf("%s\n", buffer.buffer().data());

      XmlReader e(buffer.buffer());
      e.readNextStartElement();
      QString tag(e.name().toString());
// printf("read tag %s\n", qPrintable(tag));
      element = Element::name2Element(e.name(), score);
      element->read(e);
      return element;
      }

//---------------------------------------------------------
//   MTest
//---------------------------------------------------------

MTest::MTest()
      {
      MScore::testMode = true;
      }

//---------------------------------------------------------
//   readScore
//---------------------------------------------------------

Score* MTest::readScore(const QString& name)
      {
      QString path = root + "/" + name;
      return readCreatedScore(path);
      }

//---------------------------------------------------------
//   readCreatedScore
//---------------------------------------------------------

Score* MTest::readCreatedScore(const QString& name)
      {
      Score* score = new Score(mscore->baseStyle());
      score->setName(name);
//      MScore::testMode = true;
      QString csl  = score->fileInfo()->suffix().toLower();

      Score::FileError rv;
      if (csl == "cap")
            rv = importCapella(score, name);
      else if (csl == "capx")
            rv = importCapXml(score, name);
      else if (csl == "sgu")
            rv = importBB(score, name);
      else if (csl == "mscz" || csl == "mscx")
            rv = score->loadMsc(name, false);
      else if (csl == "mxl")
            rv = importCompressedMusicXml(score, name);
#ifdef OMR
      else if (csl == "pdf")
            rv = importPdf(score, name);
#endif
      else if (csl == "xml")
            rv = importMusicXml(score, name);
      else if (csl == "gp3" || csl == "gp4" || csl == "gp5" || csl == "gpx")
            rv = importGTP(score, name);
      else
            rv = Score::FileError::FILE_UNKNOWN_TYPE;


      if (rv != Score::FileError::FILE_NO_ERROR) {
            QWARN(qPrintable(QString("readScore: cannot load <%1> type <%2>\n").arg(name).arg(csl)));
            delete score;
            return 0;
            }
      score->updateNotes();
      for (Excerpt* e : score->excerpts())
            e->score()->updateNotes();
      return score;
      }

//---------------------------------------------------------
//   saveScore
//---------------------------------------------------------

bool MTest::saveScore(Score* score, const QString& name) const
      {
      QFileInfo fi(name);
//      MScore::testMode = true;
      return score->saveFile(fi);
      }

//---------------------------------------------------------
//   compareFiles
//---------------------------------------------------------

bool MTest::compareFiles(const QString& saveName, const QString& compareWith) const
      {
      QString cmd = "diff";
      QStringList args;
      args.append("-u");
      args.append(saveName);
      args.append(root + "/" + compareWith);
      QProcess p;
      p.start(cmd, args);
      if (!p.waitForFinished())
            return false;
      if (p.exitCode()) {
            QByteArray ba = p.readAll();
            qDebug("%s", qPrintable(ba));
            qDebug("   <diff -u %s %s failed", qPrintable(saveName),
               qPrintable(QString(root + "/" + compareWith)));
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   saveCompareScore
//---------------------------------------------------------

bool MTest::saveCompareScore(Score* score, const QString& saveName, const QString& compareWith) const
      {
      saveScore(score, saveName);
      return compareFiles(saveName, compareWith);
      }

//---------------------------------------------------------
//   saveCompareMusicXMLScore
//---------------------------------------------------------

bool MTest::saveCompareMusicXmlScore(Score* score, const QString& saveName, const QString& compareWith)
      {
      saveMusicXml(score, saveName);
      return compareFiles(saveName, compareWith);
      }

//---------------------------------------------------------
//   savePdf
//---------------------------------------------------------

bool MTest::savePdf(Score* cs, const QString& saveName)
      {
      QPrinter printerDev(QPrinter::HighResolution);
      const PageFormat* pf = cs->pageFormat();
      printerDev.setPaperSize(pf->size(), QPrinter::Inch);

      printerDev.setCreator("MuseScore Version: " VERSION);
      printerDev.setFullPage(true);
      printerDev.setColorMode(QPrinter::Color);
      printerDev.setDocName(cs->name());
      printerDev.setDoubleSidedPrinting(pf->twosided());
      printerDev.setOutputFormat(QPrinter::PdfFormat);

      printerDev.setOutputFileName(saveName);
      QPainter p(&printerDev);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::TextAntialiasing, true);
      double mag = printerDev.logicalDpiX() / MScore::DPI;
            p.scale(mag, mag);

      const QList<Page*> pl = cs->pages();
      int pages    = pl.size();
      int offset   = cs->pageNumberOffset();
      int fromPage = printerDev.fromPage() - 1 - offset;
      int toPage   = printerDev.toPage() - 1 - offset;
      if (fromPage < 0)
                  fromPage = 0;
      if ((toPage < 0) || (toPage >= pages))
                  toPage = pages - 1;

      for (int copy = 0; copy < printerDev.numCopies(); ++copy) {
                  bool firstPage = true;
                  for (int n = fromPage; n <= toPage; ++n) {
                              if (!firstPage)
                                          printerDev.newPage();
                              firstPage = false;

                              cs->print(&p, n);
                              if ((copy + 1) < printerDev.numCopies())
                                          printerDev.newPage();
                              }
                  }
      p.end();
      return true;
      }

//---------------------------------------------------------
//   saveMusicXml
//---------------------------------------------------------

bool MTest::saveMusicXml(Score* score, const QString& saveName)
      {
      return saveXml(score, saveName);
      }

//---------------------------------------------------------
//   saveMimeData
//---------------------------------------------------------

bool MTest::saveMimeData(QByteArray mimeData, const QString& saveName)
      {
      QFile f(saveName);
      if (!f.open(QIODevice::WriteOnly))
            return false;

      f.write(mimeData);
      return f.error() == QFile::NoError;
      }

//---------------------------------------------------------
//   saveCompareMimeData
//---------------------------------------------------------

bool MTest::saveCompareMimeData(QByteArray mimeData, const QString& saveName, const QString& compareWith)
      {
      saveMimeData(mimeData, saveName);
      return compareFiles(saveName, compareWith);
      }

//---------------------------------------------------------
//   initMTest
//---------------------------------------------------------

void MTest::initMTest()
      {
      MScore::DPI  = 120;
      MScore::PDPI = 120;
      MScore::DPMM = MScore::DPI / INCH;
      MScore::noGui = true;

      synti  = new MasterSynthesizer();
      mscore = new MScore;
      mscore->init();
      preferences.shortestNote = MScore::division / 4; // midi quantization: 1/16

      root = TESTROOT "/mtest";
      loadInstrumentTemplates(":/instruments.xml");
      score = readScore("/test.mscx");
      }
}

