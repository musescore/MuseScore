//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: importxml.cpp 5653 2012-05-19 20:19:58Z lvinken $
//
//  Copyright (C) 2002-2013 Werner Schweer and others
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

// TODO LVI 2011-01-18: it seems brackets ending after the last note in a measure
// import OK but stop is not exported OK. Find out what causes this (import, export,
// structural issue).

// TODO LVI 2011-10-30: determine how to report import errors.
// Currently all output (both debug and error reports) are done using qDebug.

// TODO LVI 2012-04-26: read drumset definition from MusicXML
// Mapping Musescore DrumInstrument to MusicXML elements
// pitch         part-list/score-part/midi-instrument/midi-unpitched
// name          part-list/score-part/score-instrument/instrument-name
// notehead      part/measure/note/notehead (once for each individual note)
// line          part/measure/note/unpitched (once for each individual note) + current clef
// stemdirection part/measure/note/stem (once for each individual note)
// voice         N/A (leave empty ?)
// shortcut      N/A (leave empty ?)

/**
 MusicXML import.
 */

#include "config.h"
// #include "musescore.h"
#include "musicxml.h"
#include "file.h"
#include "libmscore/score.h"
#include "libmscore/rest.h"
#include "libmscore/chord.h"
#include "libmscore/sig.h"
#include "libmscore/key.h"
#include "libmscore/clef.h"
#include "libmscore/note.h"
#include "libmscore/element.h"
#include "libmscore/sym.h"
#include "libmscore/slur.h"
#include "libmscore/tie.h"
#include "libmscore/hairpin.h"
#include "libmscore/tuplet.h"
#include "libmscore/segment.h"
#include "libmscore/dynamic.h"
#include "libmscore/page.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/measure.h"
#include "libmscore/style.h"
#include "libmscore/bracket.h"
#include "libmscore/timesig.h"
#include "libmscore/xml.h"
#include "libmscore/barline.h"
#include "libmscore/lyrics.h"
#include "libmscore/volta.h"
#include "libmscore/textline.h"
#include "libmscore/keysig.h"
#include "libmscore/pitchspelling.h"
#include "libmscore/utils.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/tremolo.h"
#include "libmscore/box.h"
#include "libmscore/repeat.h"
#include "libmscore/ottava.h"
#include "libmscore/trill.h"
#include "libmscore/pedal.h"
#include "libmscore/harmony.h"
#include "libmscore/tempotext.h"
#include "libmscore/articulation.h"
#include "libmscore/arpeggio.h"
#include "libmscore/glissando.h"
#include "libmscore/breath.h"
#include "libmscore/tempo.h"
#include "libmscore/chordlist.h"
#include "libmscore/mscore.h"
#include "libmscore/accidental.h"
#include "libmscore/rehearsalmark.h"
#include "libmscore/fingering.h"
#include "preferences.h"
#include "musicxmlsupport.h"
#include "libmscore/chordline.h"
#include "libmscore/figuredbass.h"
#include "libmscore/fret.h"
#include "libmscore/qzipreader_p.h"
#include "libmscore/stafftype.h"
#include "libmscore/tablature.h"
#include "libmscore/drumset.h"
#include "libmscore/beam.h"
#include "libmscore/jump.h"
#include "libmscore/marker.h"
#include "importxmlfirstpass.h"

namespace Ms {

//---------------------------------------------------------
//   local defines for debug output
//---------------------------------------------------------

// #define DEBUG_VOICE_MAPPER true
// #define DEBUG_TICK true

//---------------------------------------------------------
//   MusicXMLStepAltOct2Pitch
//---------------------------------------------------------

/**
 Convert MusicXML \a step / \a alter / \a octave to midi pitch.
 Note: same code is also in libmscore/tablature.cpp.
 TODO: combine ?
 */

static int MusicXMLStepAltOct2Pitch(char step, int alter, int octave)
      {
      int istep = step - 'A';
      //                       a  b   c  d  e  f  g
      static int table[7]  = { 9, 11, 0, 2, 4, 5, 7 };
      if (istep < 0 || istep > 6) {
            qDebug("MusicXMLStepAltOct2Pitch: illegal step %d, <%c>", istep, step);
            return -1;
            }
      int pitch = table[istep] + alter + (octave+1) * 12;

      if (pitch < 0)
            pitch = -1;
      if (pitch > 127)
            pitch = -1;

      return pitch;
      }

//---------------------------------------------------------
//   xmlSetPitch
//---------------------------------------------------------

/**
 Convert MusicXML \a step / \a alter / \a octave to midi pitch,
 set pitch and tpc.
 */

static void xmlSetPitch(Note* n, char step, int alter, int octave, Ottava* ottava, int track)
      {
      // qDebug("xmlSetPitch(n=%p, st=%c, alter=%d, octave=%d)",
      //        n, step, alter, octave);

      int pitch = MusicXMLStepAltOct2Pitch(step, alter, octave);

      if (pitch < 0)
            pitch = 0;
      if (pitch > 127)
            pitch = 127;

      if (ottava != 0 && ottava->track() == track)
            pitch -= ottava->pitchShift();

      n->setPitch(pitch);

      int istep = step - 'A';
      //                        a  b  c  d  e  f  g
      static int table1[7]  = { 5, 6, 0, 1, 2, 3, 4 };
      int tpc  = step2tpc(table1[istep], AccidentalVal(alter));
      // alternativ: tpc = step2tpc((istep + 5) % 7, alter);      // rotate istep 5 steps
      n->setTpc(tpc);
      }

//---------------------------------------------------------
//   calcTicks
//---------------------------------------------------------

static int calcTicks(QString text, int divisions)
      {
      bool ok;
      int val = MxmlSupport::stringToInt(text, &ok);
      if (!ok) {
            qDebug("MusicXml-Import: bad duration value: <%s>",
                   qPrintable(text));
            }
      if (val == 0)     // neuratron scanner produces sometimes 0 !?
            val = 1;
      val *= MScore::division;
      val /= divisions;
      return val;
      }


//---------------------------------------------------------
//   noteDurationAsFraction
//---------------------------------------------------------

/**
 Determine note duration as fraction. Prefer note type over duration.
 Input e is the note element.
 If chord or grace, duration is 0.
 */

static Fraction noteDurationAsFraction(const int divisions, const QDomElement e)
      {
      int actualNotes = 0;
      bool chord = false;
      int dots  = 0;
      int duration = 0;
      bool grace = false;
      int normalNotes = 0;
      bool rest = false;
      QString type;
      for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
            if (ee.tagName() == "chord")
                  chord = true;
            else if (ee.tagName() == "dot")
                  dots++;
            else if (ee.tagName() == "duration") {
                  bool ok;
                  duration = MxmlSupport::stringToInt(ee.text(), &ok);
                  if (!ok)
                        qDebug("MusicXml-Import: bad duration value: <%s>",
                               qPrintable(ee.text()));
                  }
            else if (ee.tagName() == "grace")
                  grace = true;
            else if (ee.tagName() == "rest")
                  rest = true;
            else if (ee.tagName() == "time-modification") {
                  for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                        if (eee.tagName() == "actual-notes") {
                              bool ok;
                              actualNotes = MxmlSupport::stringToInt(eee.text(), &ok);
                              if (!ok || divisions <= 0)
                                    qDebug("MusicXml-Import: bad actual-notes value: <%s>",
                                           qPrintable(eee.text()));
                              }
                        if (eee.tagName() == "normal-notes") {
                              bool ok;
                              normalNotes = MxmlSupport::stringToInt(eee.text(), &ok);
                              if (!ok || divisions <= 0)
                                    qDebug("MusicXml-Import: bad normal-notes value: <%s>",
                                           qPrintable(eee.text()));
                              }
                        }
                  }
            else if (ee.tagName() == "type")
                  type = ee.text();
            }

      // if chord or grace, duration is 0
      if (chord || grace)
            return Fraction(0, 1);

      // calculate note duration as fraction based on type, dots, normal and actual notes
      // if that does not succeed, fallback to using the <duration> element
      // note divisions = ticks / quarter note
      Fraction f = MxmlSupport::calculateFraction(type, dots, normalNotes, actualNotes);
      if (!f.isValid()) {
            f = Fraction(duration, 4 * divisions);
            }

      // bug fix for rests in triplet
      if (f.isValid() && rest && normalNotes == 0 && actualNotes == 0) {
            if ((Fraction(duration, 4 * divisions) / f) == Fraction(2, 3)) {
                  // duration is exactly 2/3 of what is expected based on type
                  f = Fraction(duration, 4 * divisions);
                  }
            }

#ifdef DEBUG_TICK
      qDebug("time-in-fraction: note type %s dots %d norm %d act %d"
             " dur %d chord %d grace %d-> dt frac %s (ticks %d)",
             qPrintable(type), dots, normalNotes, actualNotes, duration,
             chord, grace, qPrintable(f.print()), f.ticks());
#endif

      return f;
      }


//---------------------------------------------------------
//   moveTick
//---------------------------------------------------------

/**
 Move tick and typFr by amount specified in the element e, which must be
 a forward, backup or note.
 */

static void moveTick(const int mtick, int& tick, int& maxtick, Fraction& typFr, const int divisions, const QDomElement e)
      {
      if (e.tagName() == "forward") {
            for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                  if (ee.tagName() == "duration") {
                        int val = calcTicks(ee.text(), divisions);
#ifdef DEBUG_TICK
                        qDebug("forward %d", val);
#endif
                        bool ok;
                        val = MxmlSupport::stringToInt(ee.text(), &ok);
                        if (!ok || divisions <= 0)
                              qDebug("MusicXml-Import: bad divisions value: <%s>",
                                     qPrintable(ee.text()));
                        Fraction f(val, 4 * divisions); // note divisions = ticks / quarter note
                        typFr += f;
                        typFr.reduce();
                        tick = mtick + typFr.ticks();
                        if (tick > maxtick)
                              maxtick = tick;
                        }
                  else if (ee.tagName() == "voice")
                        ;
                  else if (ee.tagName() == "staff")
                        ;
                  else
                        domError(ee);
                  }
            }
      else if (e.tagName() == "backup") {
            for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                  if (ee.tagName() == "duration") {
                        int val = calcTicks(ee.text(), divisions);
#ifdef DEBUG_TICK
                        qDebug("backup %d", val);
#endif
                        bool ok;
                        val = MxmlSupport::stringToInt(ee.text(), &ok);
                        if (!ok || divisions <= 0)
                              qDebug("MusicXml-Import: bad divisions value: <%s>",
                                     qPrintable(ee.text()));
                        Fraction f(val, 4 * divisions); // note divisions = ticks / quarter note
                        if (f < typFr) {
                              typFr -= f;
                              typFr.reduce();
                              }
                        else
                              typFr = Fraction(0, 1);
                        tick = mtick + typFr.ticks();
                        }
                  else
                        domError(ee);
                  }
            }
      else if (e.tagName() == "note") {
#ifdef DEBUG_TICK
            int ticks = 0;
            for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                  QString tag(ee.tagName());
                  if (tag == "duration") {
                        ticks = calcTicks(ee.text(), divisions);
                        }
                  }
            qDebug("note %d", ticks);
#endif
            Fraction f = noteDurationAsFraction(divisions, e);
            typFr += f;
            typFr.reduce();
            tick = mtick + typFr.ticks();
            if (tick > maxtick)
                  maxtick = tick;
            }
      }


//---------------------------------------------------------
//   determineMeasureStart
//---------------------------------------------------------

/**
 Determine the start ticks of each measure
 i.e. the sum of all previous measures length
 or start tick measure equals start tick previous measure plus length previous measure
 */

static void determineMeasureStart(const QVector<int>& ml, QVector<int>& ms)
      {
      ms.resize(ml.size());
      // first measure starts at tick = 0
      ms[0] = 0;
      // all others start at start tick previous measure plus length previous measure
      for (int i = 1; i < ml.size(); i++)
            ms[i] = ms.at(i - 1) + ml.at(i - 1);
#ifdef DEBUG_TICK
      for (int i = 0; i < ms.size(); i++)
            qDebug("measurelength ms[%d] %d", i + 1, ms.at(i));
#endif
      }


//---------------------------------------------------------
//   MusicXml
//---------------------------------------------------------

/**
 MusicXml constructor.
 */

MusicXml::MusicXml(QDomDocument* d, MxmlReaderFirstPass const& p1)
      :
      lastVolta(0),
      doc(d),
      pass1(p1),
      maxLyrics(0),
      beamMode(BeamMode::NONE),
      pageWidth(0),
      pageHeight(0)
      {
      // Determine the start tick of each measure in the part
      pass1.determineMeasureLength(measureLength);
      determineMeasureStart(measureLength, measureStart);
      }


//---------------------------------------------------------
//   initMusicXmlSchema
//    return false on error
//---------------------------------------------------------

static bool initMusicXmlSchema(QXmlSchema& schema)
      {
      // read the MusicXML schema from the application resources
      QFile schemaFile(":/schema/musicxml.xsd");
      if (!schemaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug("initMusicXmlSchema() could not open resource musicxml.xsd");
            MScore::lastError = QT_TRANSLATE_NOOP("file", "internal error: could not open resource musicxml.xsd\n");
            return false;
            }

      // copy the schema into a QByteArray and fixup xs:imports,
      // using a path to the application resources instead of to www.musicxml.org
      // to prevent downloading from the net
      QByteArray schemaBa;
      QTextStream schemaStream(&schemaFile);
      while (!schemaStream.atEnd()) {
            QString line = schemaStream.readLine();
            if (line.contains("xs:import"))
                  line.replace("http://www.musicxml.org/xsd", "qrc:///schema");
            schemaBa += line.toUtf8();
            schemaBa += "\n";
            }

      // load and validate the schema
      schema.load(schemaBa);
      if (!schema.isValid()) {
            qDebug("initMusicXmlSchema() internal error: MusicXML schema is invalid");
            MScore::lastError = QT_TRANSLATE_NOOP("file", "internal error: MusicXML schema is invalid\n");
            return false;
            }

      return true;
      }


//---------------------------------------------------------
//   musicXMLValidationErrorDialog
//---------------------------------------------------------

/**
 Show a dialog displaying the MusicXML validation error(s)
 and asks the user if he wants to try to load the file anyway.
 Return QMessageBox::Yes (try anyway) or QMessageBox::No (don't)
 */

static int musicXMLValidationErrorDialog(QString text, QString detailedText)
      {
      QMessageBox errorDialog;
      errorDialog.setIcon(QMessageBox::Question);
      errorDialog.setText(text);
      errorDialog.setInformativeText("Do you want to try to load this file anyway ?");
      errorDialog.setDetailedText(detailedText);
      errorDialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
      errorDialog.setDefaultButton(QMessageBox::No);
      return errorDialog.exec();
      }


//---------------------------------------------------------
//   extractRootfile
//---------------------------------------------------------

/**
Extract rootfile from compressed MusicXML file \a qf, return true if OK and false on error.
*/

static bool extractRootfile(QFile* qf, QByteArray& data)
      {
      MQZipReader f(qf->fileName());
      data = f.fileData("META-INF/container.xml");

      QDomDocument container;
      int line, column;
      QString err;
      if (!container.setContent(data, false, &err, &line, &column)) {
            QString s = QT_TRANSLATE_NOOP("file", "error reading container.xml at line %1 column %2: %3\n");
            MScore::lastError = s.arg(line).arg(column).arg(err);
            return false;
            }

      // extract first rootfile
      QString rootfile = "";
      for (QDomElement e = container.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "container") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "rootfiles") {
                              for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                                    if (eee.tagName() == "rootfile") {
                                          if (rootfile == "")
                                                rootfile = eee.attribute(QString("full-path"));
                                          }
                                    else
                                          domError(eee);
                                    }
                              }
                        else
                              domError(ee);
                        }
                  }
            else
                  domError(e);
            }

      if (rootfile == "") {
            qDebug("can't find rootfile in: %s", qPrintable(qf->fileName()));
            MScore::lastError = QT_TRANSLATE_NOOP("file", "can't find rootfile\n");
            return false;
            }

      // read the rootfile
      data = f.fileData(rootfile);
      return true;
      }


//---------------------------------------------------------
//   doValidate
//---------------------------------------------------------

/**
 Validate MusicXML data from file \a name contained in QIODevice \a dev.
 */

static Score::FileError doValidate(const QString& name, QIODevice* dev)
      {
      QTime t;
      t.start();

      // initialize the schema
      ValidatorMessageHandler messageHandler;
      QXmlSchema schema;
      schema.setMessageHandler(&messageHandler);
      if (!initMusicXmlSchema(schema))
            return Score::FILE_BAD_FORMAT;  // appropriate error message has been printed by initMusicXmlSchema

      // validate the data
      QXmlSchemaValidator validator(schema);
      bool valid = validator.validate(dev, QUrl::fromLocalFile(name));
      qDebug("Validation time elapsed: %d ms", t.elapsed());

      if (valid)
            qDebug("importMusicXml() file '%s' is a valid MusicXML file", qPrintable(name));
      else {
            qDebug("importMusicXml() file '%s' is not a valid MusicXML file", qPrintable(name));
            MScore::lastError = QT_TRANSLATE_NOOP("file", "this is not a valid MusicXML file\n");
            QString text = QString("File '%1' is not a valid MusicXML file").arg(name);
            if (musicXMLValidationErrorDialog(text, messageHandler.getErrors()) != QMessageBox::Yes)
                  return Score::FILE_USER_ABORT;
            }

      // return OK
      return Score::FILE_NO_ERROR;
      }

//---------------------------------------------------------
//   doImport
//---------------------------------------------------------

/**
 Import MusicXML data from file \a name contained in QIODevice \a dev into score \a score.
 */

static Score::FileError doImport(Score* score, const QString& name, QIODevice* dev, MxmlReaderFirstPass const& pass1)
      {
      QTime t;
      t.start();
      QDomDocument doc;
      int line;
      int column;
      QString err;
      if (!doc.setContent(dev, false, &err, &line, &column)) {
            QString s = QT_TRANSLATE_NOOP("file", "error at line %1 column %2: %3\n");
            MScore::lastError = s.arg(line).arg(column).arg(err);
            return Score::FILE_BAD_FORMAT;
            }
      docName = name; // set filename for domError
      MusicXml musicxml(&doc, pass1);
      musicxml.import(score);
      qDebug("Parsing time elapsed: %d ms", t.elapsed());
      return Score::FILE_NO_ERROR;
      }


//---------------------------------------------------------
//   doValidateAndImport
//---------------------------------------------------------

/**
 Validate and import MusicXML data from file \a name contained in QIODevice \a dev into score \a score.
 */

static Score::FileError doValidateAndImport(Score* score, const QString& name, QIODevice* dev)
      {
      // validate the file
      Score::FileError res;
      res = doValidate(name, dev);
      if (res != Score::FILE_NO_ERROR)
            return res;

      // pass 1
      dev->seek(0);
      MxmlReaderFirstPass pass1;
      res = pass1.setContent(dev);
      if (res != Score::FILE_NO_ERROR)
            return res;
      pass1.parseFile();

      // import the file
      dev->seek(0);
      res = doImport(score, name, dev, pass1);
      qDebug("importMusicXml() return %d", res);
      return res;
      }


//---------------------------------------------------------
//   importMusicXml
//    return Score::FILE_* errors
//---------------------------------------------------------

/**
 Import MusicXML file \a name into the Score.
 */

Score::FileError importMusicXml(Score* score, const QString& name)
      {
      qDebug("importMusicXml(%p, %s)", score, qPrintable(name));

      // open the MusicXML file
      QFile xmlFile(name);
      if (!xmlFile.exists())
            return Score::FILE_NOT_FOUND;
      if (!xmlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug("importMusicXml() could not open MusicXML file '%s'", qPrintable(name));
            MScore::lastError = QT_TRANSLATE_NOOP("file", "could not open MusicXML file\n");
            return Score::FILE_OPEN_ERROR;
            }

      // and import it
      return doValidateAndImport(score, name, &xmlFile);
      }


//---------------------------------------------------------
//   importCompressedMusicXml
//    return false on error
//---------------------------------------------------------

/**
 Import compressed MusicXML file \a name into the Score.
 */

Score::FileError importCompressedMusicXml(Score* score, const QString& name)
      {
      qDebug("importCompressedMusicXml(%p, %s)", score, qPrintable(name));

      // open the compressed MusicXML file
      QFile mxlFile(name);
      if (!mxlFile.exists())
            return Score::FILE_NOT_FOUND;
      if (!mxlFile.open(QIODevice::ReadOnly)) {
            qDebug("importCompressedMusicXml() could not open compressed MusicXML file '%s'", qPrintable(name));
            MScore::lastError = QT_TRANSLATE_NOOP("file", "could not open compressed MusicXML file\n");
            return Score::FILE_OPEN_ERROR;
            }

      // extract the root file
      QByteArray data;
      if (!extractRootfile(&mxlFile, data))
            return Score::FILE_BAD_FORMAT;  // appropriate error message has been printed by extractRootfile
      QBuffer buffer(&data);
      buffer.open(QIODevice::ReadOnly);

      // and import it
      return doValidateAndImport(score, name, &buffer);
      }


//---------------------------------------------------------
//   import
//      scorePartwise
//        part-list
//        part
//        work
//        identification
//---------------------------------------------------------

static void tupletAssert();

/**
 Parse the MusicXML file, which must be in score-partwise format.
 */

void MusicXml::import(Score* s)
      {
      tupletAssert();
      score  = s;
      tie    = 0;
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            slur[i] = 0;
      for (int i = 0; i < MAX_BRACKETS; ++i)
            bracket[i] = 0;
      for (int i = 0; i < MAX_DASHES; ++i)
            dashes[i] = 0;
      ottava = 0;
      trill = 0;
      pedal = 0;
      harmony = 0;
      tremStart = 0;
      hairpin = 0;
      figBass = 0;
      figBassExtend = false;

      // TODO only if multi-measure rests used ???
      // score->style()->set(ST_createMultiMeasureRests, true);

      for (QDomElement e = doc->documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "score-partwise")
                  scorePartwise(e.firstChildElement());
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   addText
//---------------------------------------------------------

static void addText(VBox*& vbx, Score* s, QString strTxt, int stl)
      {
      if (!strTxt.isEmpty()) {
            Text* text = new Text(s);
            text->setTextStyleType(stl);
            text->setText(strTxt);
            if (vbx == 0)
                  vbx = new VBox(s);
            vbx->add(text);
            }
      }

//---------------------------------------------------------
//   doCredits
//---------------------------------------------------------

/**
 Create Text elements for the credits read from MusicXML credit-words elements.
 Apply simple heuristics using only default x and y to recognize the meaning of credit words
 If no credits are found, create credits from meta data.
 */

void MusicXml::doCredits()
      {
      // IMPORT_LAYOUT
      // qDebug("MusicXml::doCredits()");
      /*
      const PageFormat* pf = score->pageFormat();
      qDebug("page format w=%g h=%g spatium=%g DPMM=%g DPI=%g",
             pf->width(), pf->height(), score->spatium(), MScore::DPMM, DPI);
      // page width and height in tenths
      const double pw  = pf->width() * 10 * DPI / score->spatium();
      const double ph  = pf->height() * 10 * DPI / score->spatium();
      */
      const int pw1 = pageWidth / 3;
      const int pw2 = pageWidth * 2 / 3;
      const int ph2 = pageHeight / 2;
      // qDebug("page format w=%g h=%g", pw, ph);
      // qDebug("page format w=%d h=%d", pageWidth, pageHeight);
      // qDebug("page format pw1=%d pw2=%d ph2=%d", pw1, pw2, ph2);
      // dump the credits
      /*
      for (ciCreditWords ci = credits.begin(); ci != credits.end(); ++ci) {
            CreditWords* w = *ci;
            qDebug("credit-words defx=%g defy=%g just=%s hal=%s val=%s words=%s",
                   w->defaultX,
                   w->defaultY,
                   w->justify.toUtf8().data(),
                   w->hAlign.toUtf8().data(),
                   w->vAlign.toUtf8().data(),
                   w->words.toUtf8().data());
            }
       */

      CreditWords* crwTitle = 0;
      CreditWords* crwSubTitle = 0;
      CreditWords* crwComposer = 0;
      CreditWords* crwPoet = 0;
      CreditWords* crwCopyRight = 0;

      QMap<int, CreditWords*> creditMap;  // store credit-words sorted on y pos
      int nWordsHeader = 0;               // number of credit-words in the header
      int nWordsFooter = 0;               // number of credit-words in the footer

      for (ciCreditWords ci = credits.begin(); ci != credits.end(); ++ci) {
            CreditWords* w = *ci;
            double defx = w->defaultX;
            double defy = w->defaultY;
            // composer is in the right column
            if (pw2 < defx) {
                  // found composer
                  if (!crwComposer) crwComposer = w;
                  }
            // poet is in the left column
            else if (defx < pw1) {
                  // found poet
                  if (!crwPoet) crwPoet = w;
                  }
            // save others (in the middle column) to be handled later
            else {
                  creditMap.insert(defy, w);
                  // and count #words in header and footer
                  if (defy > ph2)
                        nWordsHeader++;
                  else
                        nWordsFooter++;
                  }
            } // end for (ciCreditWords ...

      /*
      qDebug("header %d footer %d", nWordsHeader, nWordsFooter);
      QMap<int, CreditWords*>::const_iterator ci = creditMap.constBegin();
      while (ci != creditMap.constEnd()) {
            CreditWords* w = ci.value();
            qDebug("creditMap %d credit-words defx=%g defy=%g just=%s hal=%s val=%s words=%s",
                   ci.key(),
                   w->defaultX,
                   w->defaultY,
                   w->justify.toUtf8().data(),
                   w->hAlign.toUtf8().data(),
                   w->vAlign.toUtf8().data(),
                   w->words.toUtf8().data());
            ++ci;
            }
       */

      // assign title, subtitle and copyright
      QList<int> keys = creditMap.uniqueKeys(); // note: ignoring credit-words at the same y pos

      // if any credit-words present, the highest is the title
      // note that the keys are sorted in ascending order
      // -> use the last key
      if (keys.size() >= 1)
            crwTitle = creditMap.value(keys.at(keys.size() - 1));

      // if two credit-words present and both are in the header or the footer,
      // the lowest one is the subtitle, else it is the copyright
      if (keys.size() == 2) {
            if (nWordsHeader == 2 || nWordsFooter == 2)
                  crwSubTitle = creditMap.value(keys.at(0));
            else
                  crwCopyRight = creditMap.value(keys.at(0));
            }

      // if three or more credit-words present
      // the second-highest is the subtitle
      // the lowest one is the copyright
      if (keys.size() >= 3) {
            crwSubTitle = creditMap.value(keys.at(keys.size() - 2));
            crwCopyRight = creditMap.value(keys.at(0));
            }

      /*
      if (crwTitle) qDebug("title='%s'", crwTitle->words.toUtf8().data());
      if (crwSubTitle) qDebug("subtitle='%s'", crwSubTitle->words.toUtf8().data());
      if (crwComposer) qDebug("composer='%s'", crwComposer->words.toUtf8().data());
      if (crwPoet) qDebug("poet='%s'", crwPoet->words.toUtf8().data());
      if (crwCopyRight) qDebug("copyright='%s'", crwCopyRight->words.toUtf8().data());
       */

      QString strTitle;
      QString strSubTitle;
      QString strComposer;
      QString strPoet;
      QString strTranslator;

      if (crwTitle || crwSubTitle || crwComposer || crwPoet || crwCopyRight) {
            // use credits
            if (crwTitle) strTitle = crwTitle->words;
            if (crwSubTitle) strSubTitle = crwSubTitle->words;
            if (crwComposer) strComposer = crwComposer->words;
            if (crwPoet) strPoet = crwPoet->words;
            }
      else {
            // use metadata
            if (!(score->metaTag("movementTitle").isEmpty() && score->metaTag("workTitle").isEmpty())) {
                  strTitle = score->metaTag("movementTitle");
                  if (strTitle.isEmpty())
                        strTitle = score->metaTag("workTitle");
                  }
            if (!(score->metaTag("movementNumber").isEmpty() && score->metaTag("workNumber").isEmpty())) {
                  strSubTitle = score->metaTag("movementNumber");
                  if (strSubTitle.isEmpty())
                        strSubTitle = score->metaTag("workNumber");
                  }
            QString metaComposer = score->metaTag("composer");
            QString metaPoet = score->metaTag("poet");
            QString metaTranslator = score->metaTag("translator");
            if (!metaComposer.isEmpty()) strComposer = metaComposer;
            if (!metaPoet.isEmpty()) strPoet = metaPoet;
            if (!metaTranslator.isEmpty()) strTranslator = metaTranslator;
            }

      VBox* vbox  = 0;
      addText(vbox, score, strTitle,      TEXT_STYLE_TITLE);
      addText(vbox, score, strSubTitle,   TEXT_STYLE_SUBTITLE);
      addText(vbox, score, strComposer,   TEXT_STYLE_COMPOSER);
      addText(vbox, score, strPoet,       TEXT_STYLE_POET);
      addText(vbox, score, strTranslator, TEXT_STYLE_TRANSLATOR);
      if (vbox) {
            vbox->setTick(0);
            score->measures()->add(vbox);
            }

      // if no <rights> element was read and a copyright was found in the credit-words
      // set the rights metadata to the value found
      // note that MusicXML files can contain at least two different copyright statements:
      // - in the <rights> element (metadata)
      // - in the <credit-words> (the printed version)
      // while MuseScore supports only the first one
      if (score->metaTag("copyright") == "" && crwCopyRight)
            score->setMetaTag("copyright", crwCopyRight->words);
      }


//---------------------------------------------------------
//   determineTimeSig
//---------------------------------------------------------

/**
 Determine the time signature based on \a beats, \a beatType and \a timeSymbol.
 Sets return parameters \a st, \a bts, \a btp.
 Return true if OK, false on error.
 */

static bool determineTimeSig(const QString beats, const QString beatType, const QString timeSymbol,
                             TimeSigType& st, int& bts, int& btp)
      {
      // initialize
      st  = TSIG_NORMAL;
      bts = 0;       // the beats (max 4 separated by "+") as integer
      btp = 0;       // beat-type as integer
      // determine if timesig is valid
      if (beats == "2" && beatType == "2" && timeSymbol == "cut") {
            st = TSIG_ALLA_BREVE;
            bts = 2;
            btp = 2;
            return true;
            }
      else if (beats == "4" && beatType == "4" && timeSymbol == "common") {
            st = TSIG_FOUR_FOUR;
            bts = 4;
            btp = 4;
            return true;
            }
      else {
            if (!timeSymbol.isEmpty()) {
                  qDebug("ImportMusicXml: time symbol <%s> not recognized with beats=%s and beat-type=%s",
                         qPrintable(timeSymbol), qPrintable(beats), qPrintable(beatType));
                  return false;
                  }

            btp = beatType.toInt();
            QStringList list = beats.split("+");
            for (int i = 0; i < list.size(); i++)
                  bts += list.at(i).toInt();
            }
      return true;
      }


//---------------------------------------------------------
//   readPageFormat
//---------------------------------------------------------

void MusicXml::readPageFormat(PageFormat* pf, QDomElement de, qreal conversion)
      {
      qreal _oddRightMargin  = 0.0;
      qreal _evenRightMargin = 0.0;
      QSizeF size;

      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            if (tag == "page-margins") {
                  QString type = e.attribute("type","both");
                  qreal lm = 0.0, rm = 0.0, tm = 0.0, bm = 0.0;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        const QString& tag(ee.tagName());
                        qreal val = ee.text().toDouble() * conversion;
                        if (tag == "left-margin")
                              lm = val;
                        else if (tag == "right-margin")
                              rm = val;
                        else if (tag == "top-margin")
                              tm = val;
                        else if (tag == "bottom-margin")
                              bm = val;
                        else
                              domError(ee);
                        }
                  pf->setTwosided(type == "odd" || type == "even");
                  if (type == "odd" || type == "both") {
                        pf->setOddLeftMargin(lm);
                        _oddRightMargin = rm;
                        pf->setOddTopMargin(tm);
                        pf->setOddBottomMargin(bm);
                        }
                  if (type == "even" || type == "both") {
                        pf->setEvenLeftMargin(lm);
                        _evenRightMargin = rm;
                        pf->setEvenTopMargin(tm);
                        pf->setEvenBottomMargin(bm);
                        }
                  }
            else if (tag == "page-height") {
                  size.rheight() = val.toDouble() * conversion;
                  // set pageHeight and pageWidth for use by doCredits()
                  pageHeight = static_cast<int>(val.toDouble() + 0.5);
                  }
            else if (tag == "page-width") {
                  size.rwidth() = val.toDouble() * conversion;
                  // set pageHeight and pageWidth for use by doCredits()
                  pageWidth = static_cast<int>(val.toDouble() + 0.5);
                  }
            else
                  domError(e);
            }
      pf->setSize(size);
      qreal w1 = size.width() - pf->oddLeftMargin() - _oddRightMargin;
      qreal w2 = size.width() - pf->evenLeftMargin() - _evenRightMargin;
      pf->setPrintableWidth(qMax(w1, w2));     // silently adjust right margins
      }

//---------------------------------------------------------
//   scorePartwise
//---------------------------------------------------------

/**
 Read the MusicXML score-partwise element.
 */

void MusicXml::scorePartwise(QDomElement ee)
      {
      // In a first pass collect all parts in case the part-list does not
      // list them all. Incomplete part-list's are generated by some versions
      // of Finale.
      // Furthermore, determine the length in ticks of each measure in the part
      for (QDomElement e = ee; !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "part") {
                  QString id = e.attribute(QString("id"));
                  if (id == "")
                        qDebug("MusicXML import: part without id");
                  else {
                        Part* part = new Part(score);
                        part->setId(id);
                        score->appendPart(part);
                        Staff* staff = new Staff(score, part, 0);
                        part->staves()->push_back(staff);
                        score->staves().push_back(staff);
                        tuplets.resize(VOICES); // part now contains one staff, thus VOICES voices
                        }
                  }
            }

      // Read the score
      for (QDomElement e = ee; !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "part-list")
                  xmlPartList(e.firstChildElement());
            else if (tag == "part")
                  xmlPart(e.firstChildElement(), e.attribute(QString("id")));
            else if (tag == "work") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "work-number")
                              score->setMetaTag("workNumber", ee.text());
                        else if (ee.tagName() == "work-title")
                              score->setMetaTag("workTitle", ee.text());
                        else
                              domError(ee);
                        }
                  }
            else if (tag == "identification") {
                  // read the metadata
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "creator") {
                              // type is an arbitrary label
                              score->setMetaTag(ee.attribute(QString("type")), ee.text());
                              }
                        else if (ee.tagName() == "rights")
                              score->setMetaTag("copyright", ee.text());
                        else if (ee.tagName() == "encoding")
                              score->setMetaTag("encoding", ee.text());
                        else if (ee.tagName() == "source")
                              score->setMetaTag("source", ee.text());
                        else if (ee.tagName() == "miscellaneous")
                              ;  // ignore
                        else
                              domError(ee);
                        }
                  }
            else if (tag == "defaults") {
                  // IMPORT_LAYOUT
                  double millimeter = score->spatium()/10.0;
                  double tenths = 1.0;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        if (tag == "scaling") {
                              for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                                    QString tag(eee.tagName());
                                    if (tag == "millimeters")
                                          millimeter = eee.text().toDouble();
                                    else if (tag == "tenths")
                                          tenths = eee.text().toDouble();
                                    else
                                          domError(eee);
                                    }
                              double _spatium = MScore::DPMM * (millimeter * 10.0 / tenths);
                              if (preferences.musicxmlImportLayout)
                                    score->setSpatium(_spatium);
                              }
                        else if (tag == "page-layout") {
                              PageFormat pf;
                              readPageFormat(&pf, ee, millimeter / (tenths * INCH));
                              if (preferences.musicxmlImportLayout)
                                    score->setPageFormat(pf);
                              }
                        else if (tag == "system-layout") {
                              for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                                    QString tag(eee.tagName());
                                    Spatium val(eee.text().toDouble() / 10.0);
                                    if (tag == "system-margins")
                                          ;
                                    else if (tag == "system-distance") {
                                          if (preferences.musicxmlImportLayout) {
                                                score->style()->set(ST_minSystemDistance, val);
                                                qDebug("system distance %f", val.val());
                                                }
                                          }
                                    else if (tag == "top-system-distance")
                                          ;
                                    else
                                          domError(eee);
                                    }
                              }
                        else if (tag == "staff-layout") {
                              for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                                    QString tag(eee.tagName());
                                    Spatium val(eee.text().toDouble() / 10.0);
                                    if (tag == "staff-distance") {
                                          if (preferences.musicxmlImportLayout)
                                                score->style()->set(ST_staffDistance, val);
                                          }
                                    else
                                          domError(eee);
                                    }
                              }
                        else if (tag == "music-font")
                              domNotImplemented(ee);
                        else if (tag == "word-font")
                              domNotImplemented(ee);
                        else if (tag == "lyric-font")
                              domNotImplemented(ee);
                        else if (tag == "appearance")
                              domNotImplemented(ee);
                        else if (tag == "lyric-language")
                              domNotImplemented(ee);
                        else
                              domError(ee);
                        }

                  score->setDefaultsRead(true); // TODO only if actually succeeded ?
                  // IMPORT_LAYOUT END
                  }
            else if (tag == "movement-number")
                  score->setMetaTag("movementNumber", e.text());
            else if (tag == "movement-title")
                  score->setMetaTag("movementTitle", e.text());
            else if (tag == "credit") {
                  QString page = e.attribute(QString("page"), "1");
                  // handle only page 1 credits (to extract title etc.)
                  if (page == "1") {
                        // multiple credit-words elements may be present,
                        // which are appended
                        // use the position info from the first one
                        // font information is ignored, credits will be styled
                        bool   creditWordsRead = false;
                        double defaultx = 0;
                        double defaulty = 0;
                        QString justify;
                        QString halign;
                        QString valign;
                        QString crwords;
                        for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                              QString tag(ee.tagName());
                              if (tag == "credit-words") {
                                    // IMPORT_LAYOUT
                                    if (!creditWordsRead) {
                                          defaultx = ee.attribute(QString("default-x")).toDouble();
                                          defaulty = ee.attribute(QString("default-y")).toDouble();
                                          justify  = ee.attribute(QString("justify"));
                                          halign   = ee.attribute(QString("halign"));
                                          valign   = ee.attribute(QString("valign"));
                                          creditWordsRead = true;
                                          }
                                    crwords += ee.text();
                                    }
                              else
                                    domError(ee);
                              }
                        if (crwords != "") {
                              CreditWords* cw = new CreditWords(defaultx, defaulty, justify, halign, valign, crwords);
                              credits.append(cw);
                              }
                        }
                  }
            else
                  domError(e);
            }

      // add brackets where required

      /*
      qDebug("partGroupList");
      for (int i = 0; i < (int) partGroupList.size(); i++) {
            MusicXmlPartGroup* pg = partGroupList[i];
            qDebug("part-group span %d start %d type %d barlinespan %d",
                   pg->span, pg->start, pg->type, pg->barlineSpan);
            }
       */

      // set of (typically multi-staff) parts containing one or more explicit brackets
      // spanning only that part: these won't get an implicit brace later
      // e.g. a two-staff piano part with an explicit brace
      QSet<Part const* const> partSet;

      // handle the explicit brackets
      const QList<Part*>& il = score->parts();
      for (int i = 0; i < (int) partGroupList.size(); i++) {
            MusicXmlPartGroup* pg = partGroupList[i];
            // add part to set
            if (pg->span == 1)
                  partSet << il.at(pg->start);
            // determine span in staves
            int stavesSpan = 0;
            for (int j = 0; j < pg->span; j++)
                  stavesSpan += il.at(pg->start + j)->nstaves();
            // add bracket and set the span
            // TODO: use group-symbol default-x to determine horizontal order of brackets
            if (pg->type == NO_BRACKET)
                  il.at(pg->start)->staff(0)->setBracket(0, NO_BRACKET);
            else
                  il.at(pg->start)->staff(0)->addBracket(BracketItem(pg->type, stavesSpan));
            if (pg->barlineSpan)
                  il.at(pg->start)->staff(0)->setBarLineSpan(pg->span);
            }

      // handle the implicit brackets:
      // multi-staff parts w/o explicit brackets get a brace
      foreach(Part const* const p, il) {
            if (p->nstaves() > 1 && !partSet.contains(p)) {
                  p->staff(0)->addBracket(BracketItem(BRACKET_BRACE, p->nstaves()));
                  p->staff(0)->setBarLineSpan(p->nstaves());
                  }
            }

      // having read all parts (meaning all segments have been created),
      // now attach all jumps and markers to segments
      // simply use the first SegChordRest in the measure
      for (int i = 0; i < jumpsMarkers.size(); i++) {
            Segment* seg = jumpsMarkers.at(i).meas()->first(Segment::SegChordRest);
            qDebug("jumpsMarkers jm %p meas %p ",
                   jumpsMarkers.at(i).el(), jumpsMarkers.at(i).meas());
            if (seg) {
                  qDebug("attach to seg %p", seg);
                  seg->add(jumpsMarkers.at(i).el());
                  }
            else {
                  qDebug("no segment found");
                  }
            }
      }



//---------------------------------------------------------
//   partGroupStart
//---------------------------------------------------------

/**
 Store part-group start with number \a n, first part \a p and symbol / \a s in the partGroups
 array for later reference, as at this time insufficient information is available to be able
 to generate the brackets.
 */

static void partGroupStart(MusicXmlPartGroup* (&pgs)[MAX_PART_GROUPS], int n, int p, QString s, bool barlineSpan)
      {
      // qDebug("partGroupStart number=%d part=%d symbol=%s\n", n, p, s.toLatin1().data());
      if (n < 0 || n >= MAX_PART_GROUPS) {
            qDebug("illegal part-group number: %d", n);
            return;
            }

      if (pgs[n]) {
            qDebug("part-group number=%d already active", n);
            return;
            }

      BracketType bracketType = NO_BRACKET;
      if (s == "")
            ;  // ignore (handle as NO_BRACKET)
      else if (s == "none")
            ;  // already set to NO_BRACKET
      else if (s == "brace")
            bracketType = BRACKET_BRACE;
      else if (s == "bracket")
            bracketType = BRACKET_NORMAL;
      else if (s == "line")
            bracketType = BRACKET_LINE;
      else if (s == "square")
            bracketType = BRACKET_SQUARE;
      else {
            qDebug("part-group symbol=%s not supported", s.toLatin1().data());
            return;
            }

      MusicXmlPartGroup* pg = new MusicXmlPartGroup;
      pg->span = 0;
      pg->start = p;
      pg->barlineSpan = barlineSpan,
      pg->type = bracketType;
      pgs[n] = pg;
      }

//---------------------------------------------------------
//   partGroupStop
//---------------------------------------------------------

/**
 Handle part-group stop with number \a n and part \a p.

 For part group n, the start part, span (in parts) and type are now known.
 To generate brackets, the span in staves must also be known.
 */

static void partGroupStop(MusicXmlPartGroup* (&pgs)[MAX_PART_GROUPS], int n, int p,
                          std::vector<MusicXmlPartGroup*>& pgl)
      {
      // qDebug("partGroupStop number=%d part=%d\n", n, p);
      if (n < 0 || n >= MAX_PART_GROUPS) {
            qDebug("illegal part-group number: %d", n);
            return;
            }

      if (!pgs[n]) {
            qDebug("part-group number=%d not active", n);
            return;
            }

      pgs[n]->span = p - pgs[n]->start;
      // qDebug("part-group number=%d start=%d span=%d type=%d",
      //        n, pgs[n]->start, pgs[n]->span, pgs[n]->type);
      pgl.push_back(pgs[n]);
      pgs[n] = 0;
      }

//---------------------------------------------------------
//   xmlPartList
//---------------------------------------------------------

/**
 Read the MusicXML part-list element.
 */

void MusicXml::xmlPartList(QDomElement e)
      {
      int scoreParts = 0;
      bool barlineSpan = false;
      MusicXmlPartGroup* partGroups[MAX_PART_GROUPS];
      for (int i = 0; i < MAX_PART_GROUPS; ++i)
            partGroups[i] = 0;

      for (; !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "score-part")
                  xmlScorePart(e.firstChildElement(), e.attribute(QString("id")), scoreParts);
            else if (e.tagName() == "part-group") {
                  int number = e.attribute(QString("number")).toInt() - 1;
                  QString symbol = "";
                  QString type = e.attribute(QString("type"));
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "group-symbol")
                              symbol = ee.text();
                        else if (ee.tagName() == "group-barline") {
                              if (ee.text() == "yes")
                                    barlineSpan = true;
                              }else
                              domError(ee);
                        }
                  if (type == "start")
                        partGroupStart(partGroups, number, scoreParts, symbol, barlineSpan);
                  else if (type == "stop")
                        partGroupStop(partGroups, number, scoreParts, partGroupList);
                  else
                        qDebug("Import MusicXml:xmlPartList: part-group type '%s' not supported",
                               type.toLatin1().data());
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   xmlScorePart
//---------------------------------------------------------

/**
 Read the MusicXML score-part element.
 */

void MusicXml::xmlScorePart(QDomElement e, QString id, int& parts)
      {
      Part* part = 0;
      foreach(Part* p, score->parts()) {
            if (p->id() == id) {
                  part = p;
                  parts++;
                  break;
                  }
            }
      if (part == 0) {
            // Some versions of Rosegarden (at least v11.02) mention parts
            // in the <part-list>, but don't contain the corresponding <part>s.
            // (i.e. the part-list is overcomplete).
            // These parts are reported but can safely be ignored.
            qDebug("Import MusicXml::xmlScorePart: cannot find part %s", qPrintable(id));
            return;
            }

      qDebug("MusicXml::xmlScorePart: instruments part %s", qPrintable(id));
      drumsets.insert(id, MusicXMLDrumset());

      for (; !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "part-name") {
                  // OK? (ws) Yes it should be ok.part-name is display in front of staff in finale. (la)
                  part->setLongName(e.text());
                  // part->setTrackName(e.text());
                  }
            else if (e.tagName() == "part-abbreviation") {
                  part->setShortName(e.text());
                  }
            else if (e.tagName() == "score-instrument") {
                  QString instrId = e.attribute("id");
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "instrument-name") {
                              qDebug("MusicXml::xmlScorePart: instrument id %s name %s",
                                     qPrintable(instrId), qPrintable(ee.text()));
                              drumsets[id].insert(instrId, MusicXMLDrumInstrument(ee.text()));
                              // part-name or instrument-name?
                              if (part->longName().isEmpty())
                                    part->setLongName(ee.text());
                              }
                        else if (ee.tagName() == "instrument-sound")
                              domNotImplemented(e);
                        else if (ee.tagName() == "solo")
                              domNotImplemented(e);
                        else if (ee.tagName() == "virtual-instrument")
                              domNotImplemented(e);
                        else
                              domError(ee);
                        }
                  }
            else if (e.tagName() == "midi-instrument") {
                  QString instrId = e.attribute("id");
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "midi-channel")
                              part->setMidiChannel(ee.text().toInt() - 1);
                        else if (ee.tagName() == "midi-program")
                              part->setMidiProgram(ee.text().toInt() - 1);
                        else if (ee.tagName() == "midi-unpitched") {
                              qDebug("MusicXml::xmlScorePart: instrument id %s midi-unpitched %s",
                                     qPrintable(instrId), qPrintable(ee.text()));
                              if (drumsets[id].contains(instrId))
                                    drumsets[id][instrId].pitch = ee.text().toInt() - 1;
                              }
                        else if (ee.tagName() == "volume") {
                              double vol = ee.text().toDouble();
                              if (vol >= 0 && vol <= 100)
                                    part->setVolume(( vol / 100) * 127);
                              }
                        else if (ee.tagName() == "pan") {
                              double pan = ee.text().toDouble();
                              if (pan >= -90 && pan <= 90)
                                    part->setPan( ((pan + 90) / 180) * 127 );
                              }
                        else
                              domError(ee);
                        }
                  }
            else
                  domError(e);
            }
      /*
      score->parts().push_back(part);
      Staff* staff = new Staff(score, part, 0);
      part->staves()->push_back(staff);
      score->staves().push_back(staff);
      */

      // dump drumset for this part
      /*
      MusicXMLDrumsetIterator i(drumsets[id]);
      while (i.hasNext()) {
            i.next();
            qDebug("%s %s", qPrintable(i.key()), qPrintable(i.value().toString()));
            }
       */
      }


//---------------------------------------------------------
//   VoiceDesc
//---------------------------------------------------------

VoiceDesc::VoiceDesc() : _staff(-1), _voice(-1), _overlaps(false)
      {
      for (int i = 0; i < MAX_STAVES; ++i) {
            _chordRests[i] =  0;
            _staffAlloc[i] = -1;
            _voices[i]     = -1;
            }
      }

void VoiceDesc::incrChordRests(int s)
      {
      if (0 <= s && s < MAX_STAVES)
            _chordRests[s]++;
      }

int VoiceDesc::numberChordRests() const
      {
      int res = 0;
      for (int i = 0; i < MAX_STAVES; ++i)
            res += _chordRests[i];
      return res;
      }

int VoiceDesc::preferredStaff() const
      {
      int max = 0;
      int res = 0;
      for (int i = 0; i < MAX_STAVES; ++i)
            if (_chordRests[i] > max) {
                  max = _chordRests[i];
                  res = i;
                  }
      return res;
      }

QString VoiceDesc::toString() const
      {
      QString res = "[";
      for (int i = 0; i < MAX_STAVES; ++i)
            res += QString(" %1").arg(_chordRests[i]);
      res += QString(" ] overlaps %1").arg(_overlaps);
      if (_overlaps) {
            res += " staffAlloc [";
            for (int i = 0; i < MAX_STAVES; ++i)
                  res += QString(" %1").arg(_staffAlloc[i]);
            res += " ] voices [";
            for (int i = 0; i < MAX_STAVES; ++i)
                  res += QString(" %1").arg(_voices[i]);
            res += " ]";
            }
      else
            res += QString(" staff %1 voice %2").arg(_staff + 1).arg(_voice + 1);
      return res;
      }


//---------------------------------------------------------
//   fillGap -- fill one gap (tstart - tend) in this track in this measure with rest(s)
//---------------------------------------------------------

static void fillGap(Measure* measure, int track, int tstart, int tend)
      {
      int ctick = tstart;
      int restLen = tend - tstart;
      // qDebug("\nfillGIFV     fillGap(measure %p track %d tstart %d tend %d) restLen %d len",
      //        measure, track, tstart, tend, restLen);
      // note: as MScore::division (#ticks in a quarter note) equals 480
      // MScore::division / 64 (#ticks in a 256th note) uequals 7.5 but is rounded down to 7
      while (restLen > MScore::division / 64) {
            int len = restLen;
            TDuration d(TDuration::V_INVALID);
            if (measure->ticks() == restLen)
                  d.setType(TDuration::V_MEASURE);
            else
                  d.setVal(len);
            Rest* rest = new Rest(measure->score(), d);
            rest->setDuration(Fraction::fromTicks(len));
            rest->setTrack(track);
            rest->setVisible(false);
            Segment* s = measure->getSegment(rest, tstart);
            s->add(rest);
            len = rest->globalDuration().ticks();
            // qDebug(" %d", len);
            ctick   += len;
            restLen -= len;
            }
      }

//---------------------------------------------------------
//   fillGapsInFirstVoices -- fill gaps in first voice of every staff in this measure for this part with rest(s)
//---------------------------------------------------------

static void fillGapsInFirstVoices(Measure* measure, Part* part)
      {
      int measTick     = measure->tick();
      int measLen      = measure->ticks();
      int nextMeasTick = measTick + measLen;
      int staffIdx = part->score()->staffIdx(part);
      /*
      qDebug("fillGIFV measure %p part %p idx %d nstaves %d tick %d - %d (len %d)",
             measure, part, staffIdx, part->nstaves(),
             measTick, nextMeasTick, measLen);
      */
      for (int st = 0; st < part->nstaves(); ++st) {
            int track = (staffIdx + st) * VOICES;
            int endOfLastCR = measTick;
            for (Segment* s = measure->first(); s; s = s->next()) {
                  // qDebug("fillGIFV   segment %p tp %s", s, s->subTypeName());
                  Element* el = s->element(track);
                  if (el) {
                        // qDebug(" el[%d] %p", track, el);
                        if (s->isChordRest()) {
                              ChordRest* cr  = static_cast<ChordRest*>(el);
                              int crTick     = cr->tick();
                              int crLen      = cr->globalDuration().ticks();
                              int nextCrTick = crTick + crLen;
                              /*
                              qDebug(" chord/rest tick %d - %d (len %d)",
                                     crTick, nextCrTick, crLen);
                              */
                              if (crTick > endOfLastCR) {
                                    /*
                                    qDebug(" GAP: track %d tick %d - %d",
                                           track, endOfLastCR, crTick);
                                    */
                                    fillGap(measure, track, endOfLastCR, crTick);
                                    }
                              endOfLastCR = nextCrTick;
                              }
                        }
                  }
            if (nextMeasTick > endOfLastCR) {
                  /*
                  qDebug("fillGIFV   measure end GAP: track %d tick %d - %d",
                         track, endOfLastCR, nextMeasTick);
                  */
                  fillGap(measure, track, endOfLastCR, nextMeasTick);
                  }
            }
      }

//---------------------------------------------------------
//   xmlPart
//---------------------------------------------------------

/**
 Read the MusicXML part element.
 */

void MusicXml::xmlPart(QDomElement e, QString id)
      {
      // qDebug("xmlPart(id='%s')", qPrintable(id));
      if (id == "") {
            qDebug("MusicXML import: part without id");
            return;
            }
      Part* part = 0;
      foreach(Part* p, score->parts()) {
            if (p->id() == id) {
                  part = p;
                  break;
                  }
            }
      if (part == 0) {
            qDebug("Import MusicXml:xmlPart: cannot find part %s", id.toLatin1().data());
            return;
            }
      fractionTSig          = Fraction(0, 1);
      tick                  = 0;
      maxtick               = 0;
      prevtick              = 0;
      lastMeasureLen        = 0;
      multiMeasureRestCount = -1;
      startMultiMeasureRest = false;

      // initVoiceMapperAndMapVoices(e);
      voicelist = pass1.getVoiceList(id);
#ifdef DEBUG_VOICE_MAPPER
      // debug: print voice mapper contents
      qDebug("voiceMapperStats: new staff");
      for (QMap<int, VoiceDesc>::const_iterator i = voicelist.constBegin(); i != voicelist.constEnd(); ++i) {
            qDebug("voiceMapperStats: voice %d staff data %s",
                   i.key() + 1, qPrintable(i.value().toString()));
            }
#endif

      if (!score->measures()->first()) {
            doCredits();
            }

      for (int measureNr = 0; !e.isNull(); e = e.nextSiblingElement(), measureNr++) {
            if (e.tagName() == "measure") {
                  // set the correct start tick for the measure
                  tick = measureStart.at(measureNr);
                  Measure* measure = xmlMeasure(part, e, e.attribute(QString("number")).toInt()-1, measureLength.at(measureNr));
                  if (measure)
                        fillGapsInFirstVoices(measure, part);
                  }
            else
                  domError(e);
            }

      // qDebug("wedge list:");
      auto i = spanners.constBegin();
      while (i != spanners.constEnd()) {
            Spanner* sp = i.key();
            int tick1 = i.value().first;
            int tick2 = i.value().second;
            // qDebug("wedge %p tick1 %d tick2 %d", sp, tick1, tick2);
            sp->setTick(tick1);
            sp->setTick2(tick2);
            sp->score()->addElement(sp);
            ++i;
            }
      spanners.clear();

      // determine if part contains a drumset
      // this is the case if any instrument has a midi-unpitched element,
      // (which stored in the MusicXMLDrumInstrument pitch field)
      // debug: also dump the drumset for this part
      Drumset* drumset = new Drumset;
      bool hasDrumset = false;
      drumset->clear();

      MusicXMLDrumsetIterator ii(drumsets[id]);
      while (ii.hasNext()) {
            ii.next();
            // qDebug("%s %s", qPrintable(ii.key()), qPrintable(ii.value().toString()));
            int pitch = ii.value().pitch;
            if (0 <= pitch && pitch <= 127) {
                  hasDrumset = true;
                  drumset->drum(ii.value().pitch)
                        = DrumInstrument(ii.value().name.toLatin1().constData(),
                                         ii.value().notehead, ii.value().line, ii.value().stemDirection);
                  }
            }
      // qDebug("hasDrumset %d", hasDrumset);
      if (hasDrumset) {
            // set staff type to percussion if incorrectly imported as pitched staff
            // Note: part has been read, staff type already set based on clef type and staff-details
            // but may be incorrect for a percussion staff that does not use a percussion clef
            for (int j = 0; j < part->nstaves(); ++j)
                  if (part->staff(j)->lines() == 5 && !part->staff(j)->isDrumStaff())
                        part->staff(j)->setStaffType(score->staffType(PERC_DEFAULT_STAFF_TYPE));
            // set drumset for instrument
            part->instr()->setUseDrumset(true);
            part->instr()->setDrumset(drumset);
            part->instr()->channel(0).bank = 128;
            part->instr()->channel(0).updateInitList();
            }
      else
            delete drumset;
      }

//---------------------------------------------------------
//   readFiguredBassItem
//---------------------------------------------------------

static void readFiguredBassItem(FiguredBassItem* fgi, const QDomElement& de,
                                bool paren, bool& extend)
      {
      // read the <figure> node de
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            int iVal = val.toInt();
            if (tag == "extend")
                  extend = true;
            else if (tag == "figure-number") {
                  // MusicXML spec states figure-number is a number
                  // MuseScore can only handle single digit
                  if (1 <= iVal && iVal <= 9)
                        fgi->setDigit(iVal);
                  }
            else if (tag == "prefix")
                  fgi->setPrefix(fgi->MusicXML2Modifier(val));
            else if (tag == "suffix")
                  fgi->setSuffix(fgi->MusicXML2Modifier(val));
            else
                  domError(e);
            }
      // set parentheses
      if (paren) {
            // parenthesis open
            if (fgi->prefix() != FiguredBassItem::ModifierNone)
                  fgi->setParenth1(FiguredBassItem::ParenthesisRoundOpen);  // before prefix
            else if (fgi->digit() != FBIDigitNone)
                  fgi->setParenth2(FiguredBassItem::ParenthesisRoundOpen);  // before digit
            else if (fgi->suffix() != FiguredBassItem::ModifierNone)
                  fgi->setParenth3(FiguredBassItem::ParenthesisRoundOpen);  // before suffix
            // parenthesis close
            if (fgi->suffix() != FiguredBassItem::ModifierNone)
                  fgi->setParenth4(FiguredBassItem::ParenthesisRoundClosed);  // after suffix
            else if (fgi->digit() != FBIDigitNone)
                  fgi->setParenth3(FiguredBassItem::ParenthesisRoundClosed);  // after digit
            else if (fgi->prefix() != FiguredBassItem::ModifierNone)
                  fgi->setParenth2(FiguredBassItem::ParenthesisRoundClosed);  // after prefix
            }
      }

//---------------------------------------------------------
//   Read MusicXML
//
// Set the FiguredBass state based on the MusicXML <figured-bass> node de.
// Note that onNote and ticks must be set by the MusicXML importer,
// as the required context is not present in the items DOM tree.
// Exception: if a <duration> element is present, tick can be set.
// Return true if valid, non-empty figure(s) are found
// Set extend to true if extend elements were found
//---------------------------------------------------------

static bool readFigBass(FiguredBass* fb, const QDomElement& de, int divisions, bool& extend)
      {
      extend = false;
      bool parentheses = (de.attribute("parentheses") == "yes");
      QString normalizedText;
      int idx = 0;
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            if (tag == "duration") {
                  bool ok = true;
                  int duration = val.toInt(&ok);
                  if (ok) {
                        duration *= MScore::division;
                        duration /= divisions;
                        fb->setTicks(duration);
                        }
                  else
                        qDebug("MusicXml-Import: bad duration value: <%s>",
                               qPrintable(val));
                  }
            else if (tag == "figure") {
                  bool figureExtend = false;
                  FiguredBassItem* pItem = new FiguredBassItem(fb->score(), idx++);
                  pItem->setTrack(fb->track());
                  pItem->setParent(fb);
                  readFiguredBassItem(pItem, e, parentheses, figureExtend);
                  if (figureExtend)
                        extend = true;
                  fb->appendItem(*pItem);
                  // add item normalized text
                  if (!normalizedText.isEmpty())
                        normalizedText.append('\n');
                  normalizedText.append(pItem->normalizedText());
                  }
            else {
                  domError(e);
                  return false;
                  }
            }
      fb->setText(normalizedText);                  // this is the text to show while editing
      bool res = !normalizedText.isEmpty();
      return res;
      }


//---------------------------------------------------------
//   removeBeam -- // beam mode for all elements and remove the beam
//---------------------------------------------------------

static void removeBeam(Beam*& beam)
      {
      for (int i = 0; i < beam->elements().size(); ++i)
            beam->elements().at(i)->setBeamMode(BeamMode::NONE);
      delete beam;
      beam = 0;
      }

//---------------------------------------------------------
//   handleBeamAndStemDir
//---------------------------------------------------------

static void handleBeamAndStemDir(ChordRest* cr, const BeamMode bm, const MScore::Direction sd, Beam*& beam)
      {
      if (!cr) return;
      // create a new beam
      if (bm == BeamMode::BEGIN) {
            // if currently in a beam, delete it
            if (beam) {
                  qDebug("handleBeamAndStemDir() new beam, removing previous incomplete beam %p", beam);
                  removeBeam(beam);
                  }
            // create a new beam
            beam = new Beam(cr->score());
            beam->setTrack(cr->track());
            beam->setBeamDirection(sd);
            }
      // add ChordRest to beam
      if (beam) {
            // verify still in the same track (switching voices in the middle of a beam is not supported)
            // and in a beam ...
            // (note no check is done on correct order of beam begin/continue/end)
            if (cr->track() == beam->track()
                && (bm == BeamMode::BEGIN || bm == BeamMode::MID || bm == BeamMode::END)) {
                  // ... and actually add cr to the beam
                  beam->add(cr);
                  }
            else {
                  qDebug("handleBeamAndStemDir() from track %d to track %d bm %d -> abort beam",
                         beam->track(), cr->track(), bm);
                  // ... or reset beam mode for all elements and remove the beam
                  removeBeam(beam);
                  }
            }
      // if no beam, set stem direction on chord itself
      if (!beam) {
            static_cast<Chord*>(cr)->setStemDirection(sd);
            cr->setBeamMode(BeamMode::NONE);
            }
      // terminate the currect beam and add to the score
      if (beam && bm == BeamMode::END)
            beam = 0;
      }

//---------------------------------------------------------
//   xmlMeasure
//---------------------------------------------------------

/**
 Read the MusicXML measure element.
 */

Measure* MusicXml::xmlMeasure(Part* part, QDomElement e, int number, int measureLen)
      {
#ifdef DEBUG_TICK
      qDebug("xmlMeasure %d begin", number);
#endif
      int staves = score->nstaves();
      int staff = score->staffIdx(part);
      // current "tick" within this measure as fraction
      // calculated using note type, backup and forward
      Fraction noteTypeTickFr;
      // maximum "tick" within this measure as fraction
      Fraction maxNoteTypeTickFr;
      // current beam
      Beam* beam = 0;

      if (staves == 0) {
            qDebug("no staves!");
            return 0;
            }

      // search measure for tick
      Measure* measure = 0;
      Measure* lastMeasure = 0;
      for (MeasureBase* mb = score->measures()->first(); mb; mb = mb->next()) {
            if (mb->type() != Element::MEASURE)
                  continue;
            Measure* m = (Measure*)mb;
            lastMeasure = m;
            if (m->tick() == tick) {
                  measure = m;
                  break;
                  }
            }
      if (!measure) {
            //
            // DEBUG:
            if (lastMeasure && lastMeasure->tick() > tick) {
                  qDebug("Measure at position %d not found!", tick);
                  }
            measure  = new Measure(score);
            measure->setTick(tick);
            measure->setLen(Fraction::fromTicks(measureLen));
            measure->setNo(number);
            score->measures()->add(measure);
            } else {
            // ws:
            // int pstaves = part->nstaves();
            // for (int i = 0; i < pstaves; ++i) {
            //    Staff* reals = score->staff(staff+i);
            // measure->mstaff(staff+i)->lines->setLines(reals->lines());
            // }
            }

      QString implicit = e.attribute("implicit", "no");
      if (implicit == "yes")
            measure->setIrregular(true);

      int cv = 0; // current voice for chords, default is 0
      QList<GraceNoteInfo> graceNotesInfos;
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "attributes")
                  xmlAttributes(measure, staff, e.firstChildElement());
            else if (e.tagName() == "note") {
                  xmlNote(measure, staff, part->id(), beam, cv, e, graceNotesInfos);
                  moveTick(measure->tick(), tick, maxtick, noteTypeTickFr, divisions, e);
#ifdef DEBUG_TICK
                  qDebug(" after inserting note tick=%d", tick);
#endif
                  }
            else if (e.tagName() == "backup") {
                  // moveTick(tick, maxtick, divisions, e);
                  moveTick(measure->tick(), tick, maxtick, noteTypeTickFr, divisions, e);
                  }
            else if (e.tagName() == "direction") {
                  direction(measure, staff, e);
                  }
            else if (e.tagName() == "print") {
                  // IMPORT_LAYOUT
                  QString newSystem = e.attribute("new-system", "no");
                  QString newPage   = e.attribute("new-page", "no");
                  //
                  // in MScore the break happens _after_ the marked measure:
                  //
                  Measure* pm = (Measure*)(measure->prev());      // TODO: MeasureBase
                  if (pm == 0)
                        qDebug("ImportXml: warning: break on first measure");
                  else {
                        if (preferences.musicxmlImportBreaks
                            && (newSystem == "yes" || newPage == "yes")) {
                              LayoutBreak* lb = new LayoutBreak(score);
                              lb->setTrack(staff * VOICES);
                              lb->setLayoutBreakType(
                                    newSystem == "yes" ? LayoutBreak::LINE : LayoutBreak::PAGE
                                    );
                              pm->add(lb);
                              }
                        }
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "system-layout") {
                              }
                        else if (ee.tagName() == "staff-layout") {
                              }
                        else if (ee.tagName() == "measure-numbering") {
                              }
                        else
                              domError(ee);
                        }
                  // IMPORT_LAYOUT END
                  }
            else if (e.tagName() == "forward") {
                  // moveTick(tick, maxtick, divisions, e);
                  moveTick(measure->tick(), tick, maxtick, noteTypeTickFr, divisions, e);
                  }
            else if (e.tagName() == "barline") {
                  QString loc = e.attribute("location", "right");
                  QString barStyle;
                  QString endingNumber;
                  QString endingType;
                  QString endingText;
                  QString repeat;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "bar-style")
                              barStyle = ee.text();
                        else if (ee.tagName() == "ending") {
                              endingNumber = ee.attribute("number");
                              endingType   = ee.attribute("type");
                              endingText = ee.text();
                              }
                        else if (ee.tagName() == "repeat")
                              repeat = ee.attribute("direction");
                        else
                              domError(ee);
                        }
                  if ((barStyle != "") || (repeat != "")) {
                        BarLine* barLine = new BarLine(score);
                        bool visible = true;
                        if (barStyle == "light-heavy" && repeat == "backward") {
                              barLine->setBarLineType(END_REPEAT);
                              }
                        else if (barStyle == "heavy-light" && repeat == "forward") {
                              barLine->setBarLineType(START_REPEAT);
                              }
                        else if (barStyle == "light-heavy" && repeat.isEmpty())
                              barLine->setBarLineType(END_BAR);
                        else if (barStyle == "regular")
                              barLine->setBarLineType(NORMAL_BAR);
                        else if (barStyle == "dashed")
                              barLine->setBarLineType(BROKEN_BAR);
                        else if (barStyle == "dotted")
                              barLine->setBarLineType(DOTTED_BAR);
                        else if (barStyle == "light-light")
                              barLine->setBarLineType(DOUBLE_BAR);
                        /*
                        else if (barStyle == "heavy-light")
                              ;
                        else if (barStyle == "heavy-heavy")
                              ;
                        */
                        else if (barStyle == "none") {
                              barLine->setBarLineType(NORMAL_BAR);
                              visible = false;
                              }
                        else if (barStyle == "") {
                              if (repeat == "backward")
                                    barLine->setBarLineType(END_REPEAT);
                              else if (repeat == "forward")
                                    barLine->setBarLineType(START_REPEAT);
                              else
                                    qDebug("ImportXml: warning: empty bar type");
                              }
                        else
                              qDebug("unsupported bar type <%s>", barStyle.toLatin1().data());
                        barLine->setTrack(staff * VOICES);
                        if (barLine->barLineType() == START_REPEAT) {
                              measure->setRepeatFlags(RepeatStart);
                              }
                        else if (barLine->barLineType() == END_REPEAT) {
                              measure->setRepeatFlags(RepeatEnd);
                              }
                        else {
                              if (loc == "right")
                                    measure->setEndBarLineType(barLine->barLineType(), false, visible);
                              else if (measure->prevMeasure())
                                    measure->prevMeasure()->setEndBarLineType(barLine->barLineType(), false, visible);
                              }
                        }
                  if (!(endingNumber.isEmpty() && endingType.isEmpty())) {
                        if (endingNumber.isEmpty())
                              qDebug("ImportXml: warning: empty ending number");
                        else if (endingType.isEmpty())
                              qDebug("ImportXml: warning: empty ending type");
                        else {
                              QStringList sl = endingNumber.split(",", QString::SkipEmptyParts);
                              QList<int> iEndingNumbers;
                              bool unsupported = false;
                              foreach(const QString &s, sl) {
                                    int iEndingNumber = s.toInt();
                                    if (iEndingNumber <= 0) {
                                          unsupported = true;
                                          break;
                                          }
                                    iEndingNumbers.append(iEndingNumber);
                                    }

                              if (unsupported)
                                    qDebug("ImportXml: warning: unsupported ending number <%s>",
                                           endingNumber.toLatin1().data());
                              else {
                                    if (endingType == "start") {
                                          Volta* volta = new Volta(score);
                                          volta->setTrack(staff * VOICES);
                                          volta->setText(endingText.isEmpty() ? endingNumber : endingText);
                                          // LVIFIX TODO also support endings "1 - 3"
                                          volta->endings().clear();
                                          volta->endings().append(iEndingNumbers);
                                          volta->setTick(measure->tick());
                                          score->addElement(volta);
                                          lastVolta = volta;
                                          }
                                    else if (endingType == "stop") {
                                          if (lastVolta) {
                                                lastVolta->setVoltaType(VoltaType::CLOSED);
                                                lastVolta->setTick2(measure->tick() + measure->ticks());
                                                lastVolta = 0;
                                                }
                                          else {
                                                qDebug("lastVolta == 0 on stop");
                                                }
                                          }
                                    else if (endingType == "discontinue") {
                                          if (lastVolta) {
                                                lastVolta->setVoltaType(VoltaType::OPEN);
                                                lastVolta->setTick2(measure->tick() + measure->ticks());
                                                lastVolta = 0;
                                                }
                                          else {
                                                qDebug("lastVolta == 0 on discontinue");
                                                }
                                          }
                                    else
                                          qDebug("ImportXml: warning: unsupported ending type <%s>",
                                                 endingType.toLatin1().data());
                                    }
                              }
                        }
                  }
            else if (e.tagName() == "sound")
                  domNotImplemented(e);
            else if (e.tagName() == "harmony")
                  xmlHarmony(e, tick, measure, staff);
            else if (e.tagName() == "figured-bass") {
                  if (figBass) {
                        qDebug("more than one figured bass element on note not supported");
                        }
                  else {
                        // read figured bass element to attach to next note
                        figBassExtend = false;
                        bool mustkeep = false;
                        figBass = new FiguredBass(score);
                        // mustkeep = figBass->readMusicXML(e, divisions, figBassExtend);
                        mustkeep = readFigBass(figBass, e, divisions, figBassExtend);
                        // qDebug("xmlMeaure: fb mustkeep %d extend %d", mustkeep, figBassExtend);
                        if (!mustkeep) {
                              delete figBass;
                              figBass = 0;
                              }
                        }
                  }
            else
                  domError(e);
            }

#ifdef DEBUG_TICK
      qDebug("end_of_measure measure->tick()=%d maxtick=%d lastMeasureLen=%d measureLen=%d tsig=%d(%s)",
             measure->tick(), maxtick, lastMeasureLen, measureLen,
             fractionTSig.ticks(), qPrintable(fractionTSig.print()));
#endif

      // can't have beams extending into the next measure
      if (beam)
            removeBeam(beam);

      // TODO:
      // - how to handle fractionTSig.isZero (shouldn't happen ?)
      // - how to handle unmetered music
      if (fractionTSig.isValid() && !fractionTSig.isZero())
            measure->setTimesig(fractionTSig);
#if 1 // previous code
      // measure->setLen(Fraction::fromTicks(measureLen));
      lastMeasureLen = measureLen;
      tick = maxtick;
#endif

      // multi-measure rest handling:
      // the first measure in a multi-measure rest gets setBreakMultiMeasureRest(true)
      // and count down the remaining number of measures
      // the first measure after a multi-measure rest gets setBreakMultiMeasureRest(true)
      // for all other measures breakMultiMeasureRest is unchanged (stays default (false))
      if (startMultiMeasureRest) {
            measure->setBreakMultiMeasureRest(true);
            startMultiMeasureRest = false;
            }
      else {
            if (multiMeasureRestCount > 0) {
                  // measure is continuation of a multi-measure rest
                  --multiMeasureRestCount;
                  }
            else if (multiMeasureRestCount == 0) {
                  // measure is first measure after a multi-measure rest
                  measure->setBreakMultiMeasureRest(true);
                  --multiMeasureRestCount;
                  }
            }

      return measure;
      }

//---------------------------------------------------------
//   setSLinePlacement -- helper for direction
//---------------------------------------------------------

static void setSLinePlacement(SLine* sli, float s, const QString pl, bool hasYoff, qreal yoff)
      {
      qDebug("setSLinePlacement s=%g pl='%s' hasy=%d yoff=%g",
             s, qPrintable(pl), hasYoff, yoff
             );
      float offs = 0.0;
      if (hasYoff) offs = yoff;
      else {
            // MuseScore 1.x compatible offsets
            if (pl == "above")
                  sli->setPlacement(Element::ABOVE);
            else if (pl == "below")
                  sli->setPlacement(Element::BELOW);
            else
                  qDebug("setSLinePlacement invalid placement '%s'", qPrintable(pl));
            }
      sli->setYoff(offs);
      qDebug(" -> offs*s=%g", offs * s);
      }

//---------------------------------------------------------
//   metronome
//---------------------------------------------------------

static QString ucs4ToString(int uc)
      {
      QString s;
      if (uc & 0xffff0000) {
            s = QChar(QChar::highSurrogate(uc));
            s += QChar(QChar::lowSurrogate(uc));
            }
      else
            s = QChar(uc);
      return s;
      }
/**
 Read the MusicXML metronome element.
 */

/*
          <metronome parentheses="yes">
            <beat-unit>quarter</beat-unit>
            <beat-unit-dot/>
            <per-minute>50</per-minute>
            </metronome>
          <metronome parentheses="yes">
            <beat-unit>quarter</beat-unit>
            <beat-unit-dot/>
            <beat-unit>half</beat-unit>
            <beat-unit-dot/>
            </metronome>
*/

static void metronome(QDomElement e, Text* t)
      {
      if (!t) return;
      bool textAdded = false;
      QString tempoText = t->text();

      QString parenth = e.attribute("parentheses");
      if (parenth == "yes")
            tempoText += "(";
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString txt = e.text();
            if (e.tagName() == "beat-unit") {
                  if (textAdded) tempoText += " = ";
                  if (txt == "breve") tempoText += ucs4ToString(0x1d15c);
                  else if (txt == "whole") tempoText += ucs4ToString(0x1d15d);
                  else if (txt == "half") tempoText += ucs4ToString(0x1d15e);
                  else if (txt == "quarter") tempoText += ucs4ToString(0x1d15f);
                  else if (txt == "eighth") tempoText += ucs4ToString(0x1d160);
                  else if (txt == "16th") tempoText += ucs4ToString(0x1d161);
                  else if (txt == "32nd") tempoText += ucs4ToString(0x1d162);
                  else if (txt == "64th") tempoText += ucs4ToString(0x1d163);
                  else tempoText += txt;
                  textAdded = true;
                  }
            else if (e.tagName() == "beat-unit-dot") {
                  tempoText += ucs4ToString(0x1d16d);
                  textAdded = true;
                  }
            else if (e.tagName() == "per-minute") {
                  if (textAdded) tempoText += " = ";
                  tempoText += txt;
                  textAdded = true;
                  }
            else
                  domError(e);
            } // for (e = e.firstChildElement(); ...
      if (parenth == "yes")
            tempoText += ")";
      t->setText(tempoText);
      }

//---------------------------------------------------------
//   addElement
//---------------------------------------------------------

static void addElement(Element* el, bool hasYoffset, int staff, int rstaff, Score* /* score */, QString& placement,
                       qreal rx, qreal ry, int /* offset */, Measure* measure, int tick)
      {
      if (hasYoffset) /* el->setYoff(yoffset) */;              // TODO is this still necessary ? Some element types do ot support this
      else {
            el->setPlacement(placement == "above" ? Element::ABOVE : Element::BELOW);
            // double y = (staff + rstaff) * (score->styleD(ST_staffDistance) + 4);             // TODO 4 = #lines/staff - 1
            // y += (placement == "above" ? -3 : 5);
            // y *= score->spatium();
            // el->setReadPos(QPoint(0, y));
            }
      el->setUserOff(QPointF(rx, ry));
      // el->setMxmlOff(offset);
      el->setTrack((staff + rstaff) * VOICES);
      Segment* s = measure->getSegment(Segment::SegChordRest, tick);
      s->add(el);
      }

//---------------------------------------------------------
//   direction
//---------------------------------------------------------

/**
 Read the MusicXML direction element.
 */

// LVI FIXME: introduce offset concept to mscore.
// offset changes only the print position (not the tick), but unlike relative-x
// it is expressed in terms of divisions (MusicXML direction.dtd)
// even though the DTD does not mention it, practically speaking
// offset and relative-x are mutually exclusive

// Typical example:
// <direction placement="above">
//   <direction-type>
//      <words>Fine</words>
//      </direction-type>
//   <sound fine="yes"/>
//   </direction>

// Note: multiple direction-type (and multiple words) elements may be present,
// and at least one direction-type must be present but sound is optional and
// at most one can be present.

void MusicXml::direction(Measure* measure, int staff, QDomElement e)
      {
      QString placement = e.attribute("placement");

      QString dirType;
      QString type;
      QString txt;
      QString lang;
      QString fontWeight = "";
      QString fontStyle = "";
      QString fontSize = "";
      int offset = 0; // not supported yet
      int rstaff = 0;
      QStringList dynamics;
      // int spread;
      qreal rx = 0.0;
      qreal ry = 0.0;
      qreal yoffset = 0.0; // actually this is default-y
      // qreal xoffset;
      bool hasYoffset = false;
      QString dynaVelocity = "";
      QString tempo = "";
      QString rehearsal = "";
      QString sndCapo = "";
      QString sndCoda = "";
      QString sndDacapo = "";
      QString sndDalsegno = "";
      QString sndSegno = "";
      QString sndFine = "";
      bool coda = false;
      bool segno = false;
      int ottavasize = 0;
      bool pedalLine = false;
      int number = 1;
      QString lineEnd;
      // qreal endLength;
      QString lineType;
      QDomElement metrEl;

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "direction-type") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        // IMPORT_LAYOUT
                        dirType = ee.tagName();
                        if (preferences.musicxmlImportLayout) {
                              ry      = ee.attribute(QString("relative-y"), "0").toDouble() * -.1;
                              rx      = ee.attribute(QString("relative-x"), "0").toDouble() * .1;
                              yoffset = ee.attribute("default-y").toDouble(&hasYoffset) * -0.1;
                              // xoffset = ee.attribute("default-x", "0.0").toDouble() * 0.1;
                              }
                        if (dirType == "words") {
                              txt        = ee.text();
                              lang       = ee.attribute(QString("xml:lang"), "it");
                              fontWeight = ee.attribute(QString("font-weight"));
                              fontSize   = ee.attribute(QString("font-size"));
                              fontStyle  = ee.attribute(QString("font-style"));
                              }
                        else if (dirType == "rehearsal") {
                              rehearsal = ee.text();
                              }
                        else if (dirType == "pedal") {
                              type = ee.attribute(QString("type"));
                              pedalLine = ee.attribute(QString("line")) == "yes";
                              }
                        else if (dirType == "dynamics") {
                              QDomElement eee = ee.firstChildElement();
                              if (!eee.isNull()) {
                                    if (eee.tagName() == "other-dynamics")
                                          dynamics.push_back(eee.text());
                                    else
                                          dynamics.push_back(eee.tagName());
                                    }
                              }
                        else if (dirType == "wedge") {
                              type   = ee.attribute(QString("type"));
                              // spread = ee.attribute(QString("spread"), "0").toInt();
                              }
                        else if (dirType == "dashes") {
                              type      = ee.attribute(QString("type"));
                              number    = ee.attribute(QString("number"), "1").toInt();
                              }
                        else if (dirType == "bracket") {
                              type      = ee.attribute(QString("type"));
                              number    = ee.attribute(QString("number"), "1").toInt();
                              lineEnd   = ee.attribute(QString("line-end"), "none");
                              // endLength = ee.attribute(QString("end-length"), "0").toDouble() * 0.1;
                              lineType  = ee.attribute(QString("line-type"), "solid");
                              }
                        else if (dirType == "metronome")
                              metrEl = ee;
                        else if (dirType == "octave-shift") {
                              type       = ee.attribute(QString("type"));
                              ottavasize = ee.attribute(QString("size"), "0").toInt();
                              }
                        else if (dirType == "coda")
                              coda = true;
                        else if (dirType == "segno")
                              segno = true;
                        else
                              domError(ee);
                        }
                  }
            else if (e.tagName() == "sound") {
                  // attr: dynamics, tempo
                  // LVIFIX: TODO coda and segno should be numbered uniquely
                  sndCapo = e.attribute("capo");
                  sndCoda = e.attribute("coda");
                  sndDacapo = e.attribute("dacapo");
                  sndDalsegno = e.attribute("dalsegno");
                  sndFine = e.attribute("fine");
                  sndSegno = e.attribute("segno");
                  tempo = e.attribute("tempo");
                  dynaVelocity = e.attribute("dynamics");
                  }
            else if (e.tagName() == "offset")
                  offset = (e.text().toInt() * MScore::division)/divisions;
            else if (e.tagName() == "staff") {
                  // DEBUG: <staff>0</staff>
                  rstaff = e.text().toInt() - 1;
                  if (rstaff < 0)         // ???
                        rstaff = 0;
                  }
            else if (e.tagName() == "voice")
                  domNotImplemented(e);
            else
                  domError(e);
            } // for (e = e.firstChildElement(); ...

      /*
      qDebug(" tempo=%s txt=%s metrEl=%s coda=%d segno=%d sndCapo=%s sndCoda=%s"
             " sndDacapo=%s sndDalsegno=%s sndFine=%s sndSegno=%s",
             tempo.toLatin1().data(),
             txt.toLatin1().data(),
             qPrintable(metrEl.tagName()),
             coda,
             segno,
             sndCapo.toLatin1().data(),
             sndCoda.toLatin1().data(),
             sndDacapo.toLatin1().data(),
             sndDalsegno.toLatin1().data(),
             sndFine.toLatin1().data(),
             sndSegno.toLatin1().data()
            );
      */

      // Try to recognize the various repeats
      QString repeat = "";
      // Easy cases first
      if (coda) repeat = "coda";
      if (segno) repeat = "segno";
      // As sound may be missing, next do a wild-card match with known repeat texts
      QString lowerTxt = txt.toLower();
      QRegExp daCapo("d\\.? *c\\.?|da *capo");
      QRegExp daCapoAlFine("d\\.? *c\\.? *al *fine|da *capo *al *fine");
      QRegExp daCapoAlCoda("d\\.? *c\\.? *al *coda|da *capo *al *coda");
      QRegExp dalSegno("d\\.? *s\\.?|d[ae]l *segno");
      QRegExp dalSegnoAlFine("d\\.? *s\\.? *al *fine|d[ae]l *segno *al *fine");
      QRegExp dalSegnoAlCoda("d\\.? *s\\.? *al *coda|d[ae]l *segno *al *coda");
      QRegExp fine("fine");
      QRegExp toCoda("to *coda");
      if (daCapo.exactMatch(lowerTxt)) repeat = "daCapo";
      if (daCapoAlFine.exactMatch(lowerTxt)) repeat = "daCapoAlFine";
      if (daCapoAlCoda.exactMatch(lowerTxt)) repeat = "daCapoAlCoda";
      if (dalSegno.exactMatch(lowerTxt)) repeat = "dalSegno";
      if (dalSegnoAlFine.exactMatch(lowerTxt)) repeat = "dalSegnoAlFine";
      if (dalSegnoAlCoda.exactMatch(lowerTxt)) repeat = "dalSegnoAlCoda";
      if (fine.exactMatch(lowerTxt)) repeat = "fine";
      if (toCoda.exactMatch(lowerTxt)) repeat = "toCoda";
      // If that did not work, try to recognize a sound attribute
      if (repeat == "" && sndCoda != "") repeat = "coda";
      if (repeat == "" && sndDacapo != "") repeat = "daCapo";
      if (repeat == "" && sndDalsegno != "") repeat = "dalSegno";
      if (repeat == "" && sndFine != "") repeat = "fine";
      if (repeat == "" && sndSegno != "") repeat = "segno";
      // If a repeat was found, assume words is no longer needed
      if (repeat != "") txt = "";

      /*
      qDebug(" txt=%s repeat=%s",
             txt.toLatin1().data(),
             repeat.toLatin1().data()
            );
      */

      if (repeat != "") {
            Jump* jp = 0;
            Marker* m = 0;
            if (repeat == "segno") {
                  m = new Marker(score);
                  // note: Marker::read() also contains code to set text style based on type
                  // avoid duplicated code
                  m->setTextStyleType(TEXT_STYLE_REPEAT_LEFT);
                  // apparently this MUST be after setTextStyle
                  m->setMarkerType(MarkerType::SEGNO);
                  }
            else if (repeat == "coda") {
                  m = new Marker(score);
                  m->setTextStyleType(TEXT_STYLE_REPEAT_LEFT);
                  m->setMarkerType(MarkerType::CODA);
                  }
            else if (repeat == "fine") {
                  m = new Marker(score);
                  m->setTextStyleType(TEXT_STYLE_REPEAT_RIGHT);
                  m->setMarkerType(MarkerType::FINE);
                  }
            else if (repeat == "toCoda") {
                  m = new Marker(score);
                  m->setTextStyleType(TEXT_STYLE_REPEAT_RIGHT);
                  m->setMarkerType(MarkerType::TOCODA);
                  }
            else if (repeat == "daCapo") {
                  jp = new Jump(score);
                  jp->setTextStyleType(TEXT_STYLE_REPEAT_RIGHT);
                  jp->setJumpType(JumpType::DC);
                  }
            else if (repeat == "daCapoAlCoda") {
                  jp = new Jump(score);
                  jp->setTextStyleType(TEXT_STYLE_REPEAT_RIGHT);
                  jp->setJumpType(JumpType::DC_AL_CODA);
                  }
            else if (repeat == "daCapoAlFine") {
                  jp = new Jump(score);
                  jp->setTextStyleType(TEXT_STYLE_REPEAT_RIGHT);
                  jp->setJumpType(JumpType::DC_AL_FINE);
                  }
            else if (repeat == "dalSegno") {
                  jp = new Jump(score);
                  jp->setTextStyleType(TEXT_STYLE_REPEAT_RIGHT);
                  jp->setJumpType(JumpType::DS);
                  }
            else if (repeat == "dalSegnoAlCoda") {
                  jp = new Jump(score);
                  jp->setTextStyleType(TEXT_STYLE_REPEAT_RIGHT);
                  jp->setJumpType(JumpType::DS_AL_CODA);
                  }
            else if (repeat == "dalSegnoAlFine") {
                  jp = new Jump(score);
                  jp->setTextStyleType(TEXT_STYLE_REPEAT_RIGHT);
                  jp->setJumpType(JumpType::DS_AL_FINE);
                  }
            if (jp) {
                  jp->setTrack((staff + rstaff) * VOICES);
                  qDebug("jumpsMarkers adding jm %p meas %p",jp, measure);
                  jumpsMarkers.append(JumpMarkerDesc(jp, measure));
                  }
            if (m) {
                  m->setTrack((staff + rstaff) * VOICES);
                  qDebug("jumpsMarkers adding jm %p meas %p",m, measure);
                  jumpsMarkers.append(JumpMarkerDesc(m, measure));
                  }
            }

      if ((dirType == "words" && repeat == "") || dirType == "metronome") {
            /*
            qDebug("words txt='%s' metrEl='%s' tempo='%s' pl='%s' hasyoffs=%d fsz='%s' fst='%s' fw='%s'",
                   txt.toUtf8().data(),
                   qPrintable(metrEl.tagName()),
                   tempo.toUtf8().data(),
                   placement.toUtf8().data(),
                   hasYoffset,
                   fontSize.toUtf8().data(),
                   fontStyle.toUtf8().data(),
                   fontWeight.toUtf8().data()
                   );
            */
            Text* t;
            if (tempo != "" && tempo.toDouble() > 0) {
                  t = new TempoText(score);
                  double tpo = tempo.toDouble()/60.0;
                  ((TempoText*) t)->setTempo(tpo);
                  score->setTempo(tick, tpo);
                  }
            else {
                  t = new Text(score);
                  t->setTextStyleType(TEXT_STYLE_TECHNIK);
                  }
            if (!fontSize.isEmpty() || !fontStyle.isEmpty() || !fontWeight.isEmpty()) {
                  if (!fontSize.isEmpty()) {
                        bool ok = true;
                        float size = fontSize.toFloat(&ok);
                        if (ok)
                              t->setSize(size);
                        }
                  t->setItalic(fontStyle == "italic");
                  t->setBold(fontWeight == "bold");
                  }
            t->setText(txt);
            if (metrEl.tagName() != "") metronome(metrEl, t);
            if (hasYoffset) t->setYoff(yoffset);
            addElement(t, hasYoffset, staff, rstaff, score, placement,
                       rx, ry, offset, measure, tick);
            /*
            if (hasYoffset) t->setYoff(yoffset);
            else t->setAbove(placement == "above");
            t->setUserOff(QPointF(rx, ry));
            t->setMxmlOff(offset);
            t->setTrack((staff + rstaff) * VOICES);
            Segment* s = measure->getSegment(Segment::SegChordRest, tick);
            s->add(t);
            */
            }
      else if (dirType == "rehearsal") {
            Text* t = new RehearsalMark(score);
            t->setText(rehearsal);
            if (hasYoffset) t->setYoff(yoffset);
            else t->setPlacement(placement == "above" ? Element::ABOVE : Element::BELOW);
            if (hasYoffset) t->setYoff(yoffset);
            addElement(t, hasYoffset, staff, rstaff, score, placement,
                       rx, ry, offset, measure, tick);
            /*
            t->setUserOff(QPointF(rx, ry));
            t->setMxmlOff(offset);
            t->setTrack((staff + rstaff) * VOICES);
            Segment* s = measure->getSegment(Segment::SegChordRest, tick);
            s->add(t);
            */
            }
      else if (dirType == "pedal") {
            if (pedalLine) {
                  if (type == "start") {
                        if (pedal) {
                              qDebug("overlapping pedal lines not supported");
                              delete pedal;
                              pedal = 0;
                              }
                        else {
                              pedal = new Pedal(score);
                              pedal->setTrack((staff + rstaff) * VOICES);
                              if (placement == "") placement = "below";
                              setSLinePlacement(pedal,
                                                score->spatium(), placement,
                                                hasYoffset, yoffset);
                              spanners[pedal] = QPair<int, int>(tick, -1);
                              // qDebug("wedge pedal=%p inserted at first tick %d", pedal, tick);
                              }
                        }
                  else if (type == "stop") {
                        if (!pedal) {
                              qDebug("pedal line stop without start");
                              }
                        else {
                              spanners[pedal].second = tick;
                              // qDebug("wedge pedal=%p second tick %d", pedal, tick);
                              pedal = 0;
                              }
                        }
                  else
                        qDebug("unknown pedal %s", type.toLatin1().data());
                  }
            else {
                  Symbol* s = new Symbol(score);
                  s->setAlign(ALIGN_LEFT | ALIGN_BASELINE);
                  s->setOffsetType(OFFSET_SPATIUM);
                  if (type == "start")
                        s->setSym(pedalPedSym);
                  else if (type == "stop")
                        s->setSym(pedalasteriskSym);
                  else
                        qDebug("unknown pedal %s", type.toLatin1().data());
                  if (hasYoffset) s->setYoff(yoffset);
                  else s->setPlacement(placement == "above" ? Element::ABOVE : Element::BELOW);
                  s->setUserOff(QPointF(rx, ry));
                  // s->setMxmlOff(offset);
                  s->setTrack((staff + rstaff) * VOICES);
                  Segment* seg = measure->getSegment(Segment::SegChordRest, tick);
                  seg->add(s);
                  }
            }
      else if (dirType == "dynamics") {
            // more than one dynamic ???
            // LVIFIX: check import/export of <other-dynamics>unknown_text</...>
            for (QStringList::Iterator it = dynamics.begin(); it != dynamics.end(); ++it ) {
                  Dynamic* dyn = new Dynamic(score);
                  dyn->setDynamicType(*it);
                  if (!dynaVelocity.isEmpty()) {
                        int dynaValue = round(dynaVelocity.toDouble() * 0.9);
                        if (dynaValue > 127)
                              dynaValue = 127;
                        else if (dynaValue < 0)
                              dynaValue = 0;
                        dyn->setVelocity( dynaValue );
                        }
                  if (hasYoffset) dyn->setYoff(yoffset);
                  addElement(dyn, hasYoffset, staff, rstaff, score, placement,
                             rx, ry, offset, measure, tick);
                  }
            }
      else if (dirType == "wedge") {
            // qDebug("wedge type='%s' hairpin=%p", qPrintable(type), hairpin);
            // bool above = (placement == "above");
            if (type == "crescendo" || type == "diminuendo") {
                  if (hairpin) {
                        qDebug("overlapping wedge not supported");
                        spanners.remove(hairpin);
                        delete hairpin;
                        hairpin = 0;
                        }
                  else {
                        hairpin = new Hairpin(score);
                        hairpin->setHairpinType(type == "crescendo"
                                                ? Hairpin::CRESCENDO : Hairpin::DECRESCENDO);
                        hairpin->setPlacement(placement == "above" ? Element::ABOVE : Element::BELOW);

                        if (hasYoffset)
                              hairpin->setYoff(yoffset);
                        // else
                        //      hairpin->setYoff(above ? -3 : 8);
                        // hairpin->setUserOff(rx, ry));
                        hairpin->setTrack((staff + rstaff) * VOICES);
                        spanners[hairpin] = QPair<int, int>(tick, -1);
                        // qDebug("wedge hairpin=%p inserted at first tick %d", hairpin, tick);
                        }
                  }
            else if (type == "stop") {
                  if (!hairpin) {
                        qDebug("wedge stop without start");
                        }
                  else {
                        spanners[hairpin].second = tick;
                        // qDebug("wedge hairpin=%p second tick %d", hairpin, tick);
                        hairpin = 0;
                        }
                  }
            else
                  qDebug("unknown wedge type: %s", qPrintable(type));
            }
      else if (dirType == "bracket") {
            int n = number-1;
            TextLine* b = bracket[n];
            if (type == "start") {
                  if (b) {
                        qDebug("overlapping bracket with same number?");
                        delete b;
                        bracket[n] = 0;
                        }
                  else {
                        b = new TextLine(score);

                        // what does placement affect?
                        //yoffset += (placement == "above" ? 0.0 : 5.0);
                        // store for later to set in segment
                        // b->setUserOff(QPointF(rx + xoffset, ry + yoffset));
                        // b->setMxmlOff(offset);
                        if (placement == "") placement = "above";  // set default
                        setSLinePlacement(b,
                                          score->spatium(), placement,
                                          hasYoffset, yoffset);

                        b->setBeginHook(lineEnd != "none");
                        if (lineEnd == "up")
                              b->setBeginHookHeight(-1 * b->beginHookHeight());

                        // hack: assume there was a words element before the bracket
                        if (!txt.isEmpty()) {
                              b->setBeginText(txt, score->textStyle(TEXT_STYLE_TEXTLINE));
                              }

                        if (lineType == "solid")
                              b->setLineStyle(Qt::SolidLine);
                        else if (lineType == "dashed")
                              b->setLineStyle(Qt::DashLine);
                        else if (lineType == "dotted")
                              b->setLineStyle(Qt::DotLine);
                        else
                              qDebug("unsupported line-type: %s", lineType.toLatin1().data());

                        b->setTrack((staff + rstaff) * VOICES);
                        spanners[b] = QPair<int, int>(tick, -1);
                        qDebug("bracket=%p inserted at first tick %d", b, tick);
                        bracket[n] = b;
                        }
                  }
            else if (type == "stop") {
                  if (!b)
                        qDebug("bracket stop without start, number %d", number);
                  else {
                        // TODO: MuseScore doesn't support lines which start and end on different staves
                        /*
                        QPointF userOff = b->userOff();
                        b->add(b->createLineSegment());

                        b->setUserOff(QPointF()); // restore the offset
                        b->setMxmlOff2(offset);
                        LineSegment* ls1 = b->lineSegments().front();
                        LineSegment* ls2 = b->lineSegments().back();
                        // what does placement affect?
                        //yoffset += (placement == "above" ? 0.0 : 5.0);
                        ls1->setUserOff(userOff);
                        ls2->setUserOff2(QPointF(rx + xoffset, ry + yoffset));
                        */
                        b->setEndHook(lineEnd != "none");
                        if (lineEnd == "up")
                              b->setEndHookHeight(-1 * b->endHookHeight());
                        spanners[b].second = tick;
                        qDebug("bracket=%p second tick %d", b, tick);
                        bracket[n] = 0;
                        }
                  }
            }
      else if (dirType == "dashes") {
            int n = number-1;
            TextLine* b = dashes[n];
            if (type == "start") {
                  if (b) {
                        printf("overlapping dashes with same number?\n");
                        delete b;
                        dashes[n] = 0;
                        }
                  else {
                        b = new TextLine(score);

                        // what does placement affect?
                        //yoffset += (placement == "above" ? 0.0 : 5.0);
                        // store for later to set in segment
                        // b->setUserOff(QPointF(rx + xoffset, ry + yoffset));
                        // b->setMxmlOff(offset);
                        if (placement == "") placement = "above";  // set default
                        setSLinePlacement(b,
                                          score->spatium(), placement,
                                          hasYoffset, yoffset);

                        // hack: assume there was a words element before the dashes
                        if (!txt.isEmpty()) {
                              b->setBeginText(txt, score->textStyle(TEXT_STYLE_TEXTLINE));
                              }

                        b->setBeginHook(false);
                        b->setLineStyle(Qt::DashLine);
                        b->setTrack((staff + rstaff) * VOICES);
                        spanners[b] = QPair<int, int>(tick, -1);
                        qDebug("bracket=%p inserted at first tick %d", b, tick);
                        // b->setTick(tick);
                        dashes[n] = b;
                        }
                  }
            else if (type == "stop") {
                  if (!b) {
                        printf("dashes stop without start, number %d\n", number);
                        }
                  else {
                        // b->setTick2(tick);
                        // TODO: MuseScore doesn't support lines which start and end on different staves
                        /*
                        QPointF userOff = b->userOff();
                        b->add(b->createLineSegment());

                        b->setUserOff(QPointF()); // restore the offset
                        b->setMxmlOff2(offset);
                        LineSegment* ls1 = b->lineSegments().front();
                        LineSegment* ls2 = b->lineSegments().back();
                        // what does placement affect?
                        //yoffset += (placement == "above" ? 0.0 : 5.0);
                        ls1->setUserOff(userOff);
                        ls2->setUserOff2(QPointF(rx + xoffset, ry + yoffset));
                        */
                        b->setEndHook(false);
                        // score->add(b);
                        spanners[b].second = tick;
                        qDebug("bracket=%p second tick %d", b, tick);
                        dashes[n] = 0;
                        }
                  }
            }
      else if (dirType == "octave-shift") {
            if (type == "up" || type == "down") {
                  if (ottava) {
                        qDebug("overlapping octave-shift not supported");
                        delete ottava;
                        ottava = 0;
                        }
                  else {
                        if (!(ottavasize == 8 || ottavasize == 15)) {
                              qDebug("unknown octave-shift size %d", ottavasize);
                              delete ottava;
                              ottava = 0;
                              }
                        else {
                              ottava = new Ottava(score);
                              ottava->setTrack((staff + rstaff) * VOICES);
                              if (type == "down" && ottavasize ==  8) ottava->setOttavaType(OttavaType::OTTAVA_8VA);
                              if (type == "down" && ottavasize == 15) ottava->setOttavaType(OttavaType::OTTAVA_15MA);
                              if (type ==   "up" && ottavasize ==  8) ottava->setOttavaType(OttavaType::OTTAVA_8VB);
                              if (type ==   "up" && ottavasize == 15) ottava->setOttavaType(OttavaType::OTTAVA_15MB);
                              if (placement == "") placement = "above";  // set default
                              setSLinePlacement(ottava,
                                                score->spatium(), placement,
                                                hasYoffset, yoffset);
                              spanners[ottava] = QPair<int, int>(tick, -1);
                              // qDebug("wedge ottava=%p inserted at first tick %d", ottava, tick);
                              }
                        }
                  }
            else if (type == "stop") {
                  if (!ottava) {
                        qDebug("octave-shift stop without start");
                        }
                  else {
                        spanners[ottava].second = tick;
                        // qDebug("wedge ottava=%p second tick %d", ottava, tick);
                        ottava = 0;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   setStaffLines - set stafflines and barline span for a single staff
//---------------------------------------------------------

static void setStaffLines(Score* score, int staffIdx, int stafflines)
      {
      score->staff(staffIdx)->setLines(stafflines);
      score->staff(staffIdx)->setBarLineTo((stafflines-1) * 2);
      }

//---------------------------------------------------------
//   readStringData
//---------------------------------------------------------

static void readStringData(StringData* t, QDomElement de)
      {
      t->setFrets(25);

      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            int val = e.text().toInt();
            if (tag == "staff-lines") {
                  if (val > 0) {
                        // resize the string table and init with zeroes
                        t->stringList() = QVector<int>(val).toList();
                        }
                  else
                        qDebug("Tablature::readMusicXML: illegal staff-lines %d", val);
                  }
            else if (tag == "staff-tuning") {
                  int line   = e.attribute("line").toInt();
                  QString step;
                  int alter  = 0;
                  int octave = 0;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        const QString& tag(ee.tagName());
                        int val = ee.text().toInt();
                        if (tag == "tuning-alter")
                              alter = val;
                        else if (tag == "tuning-octave")
                              octave = val;
                        else if (tag == "tuning-step")
                              step = ee.text();
                        else
                              domError(ee);
                        }
                  if (0 < line && line <= t->stringList().size()) {
                        int pitch = MusicXMLStepAltOct2Pitch(step[0].toLatin1(), alter, octave);
                        if (pitch >= 0)
                              t->stringList()[line - 1] = pitch;
                        else
                              qDebug("Tablature::readMusicXML invalid string %d tuning step/alter/oct %s/%d/%d",
                                     line, qPrintable(step), alter, octave);
                        }
                  }
            else if (tag == "capo") {
                  ; // not supported: silently ignored
                  }
            else {
                  ; // others silently ignored
                  }
            }
      }

//---------------------------------------------------------
//   xmlStaffDetails
//---------------------------------------------------------

static void xmlStaffDetails(Score* score, int staff, StringData* t, QDomElement e)
      {
      int number  = e.attribute(QString("number"), "-1").toInt();
      int staffIdx = staff;
      if (number != -1)
            staffIdx += number - 1;
      bool hasStafflines = false;
      int stafflines = 5;
      for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
            if (ee.tagName() == "staff-lines") {
                  hasStafflines = true;
                  stafflines = ee.text().toInt();
                  }
            else if (ee.tagName() == "staff-tuning")
                  ;  // ignore here (but prevent error message), handled by Tablature::readMusicXML
            else
                  domNotImplemented(ee);
            }

      if (hasStafflines) {
            if (number == -1) {
                  int staves = score->staff(staff)->part()->nstaves();
                  for (int i = 0; i < staves; ++i)
                        setStaffLines(score, staffIdx+i, stafflines);
                  }
            else
                  setStaffLines(score, staffIdx, stafflines);
            }

      if (t) {
            readStringData(t, e);
            Instrument* i = score->staff(staff)->part()->instr();
            i->setStringData(t);
            }
      }

//---------------------------------------------------------
//   xmlAttributes
//---------------------------------------------------------

/**
 Read the MusicXML attributes element.
 */

// Standard order of attributes as written by Dolet for Finale is divisions,
// key, time, staves and clef(s). For the first measure this means number of
// staves must be read first, as it determines how many key and time signatures
// must be inserted.

void MusicXml::xmlAttributes(Measure* measure, int staff, QDomElement e)
      {
      QString beats = "";
      QString beatType = "";
      QString timeSymbol = "";

      int staves = 1; // default is one staff
      for (QDomElement e2 = e; !e2.isNull(); e2 = e2.nextSiblingElement()) {
            if (e2.tagName() == "staves") {
                  staves = e2.text().toInt();
                  Part* part = score->staff(staff)->part();
                  part->setStaves(staves);
                  // grow tuplets size, do not shrink to prevent losing info
                  if (staves * VOICES > tuplets.size())
                        tuplets.resize(staves * VOICES);
                  }
            }

      for (; !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "divisions") {
                  bool ok;
                  divisions = MxmlSupport::stringToInt(e.text(), &ok);
                  if (!ok) {
                        qDebug("MusicXml-Import: bad divisions value: <%s>",
                               qPrintable(e.text()));
                        divisions = 4;
                        }
                  }
            else if (e.tagName() == "key") {
                  int number = e.attribute(QString("number"), "-1").toInt();
                  QString printObject(e.attribute("print-object", "yes"));
                  int staffIdx = staff;
                  if (number != -1)
                        staffIdx += number - 1;
                  KeySigEvent key;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "fifths")
                              key.setAccidentalType(ee.text().toInt());
                        else if (ee.tagName() == "mode")
                              domNotImplemented(ee);
                        else
                              domError(ee);
                        }
                  if (number == -1) {
                        //
                        // apply key to all staves in the part
                        //
                        int staves = score->staff(staff)->part()->nstaves();
                        // apply to all staves in part
                        for (int i = 0; i < staves; ++i) {
                              KeySigEvent oldkey = score->staff(staffIdx+i)->keymap()->key(tick);
                              if (oldkey != key) {
                                    // new key differs from key in effect at this tick
                                    KeySig* keysig = new KeySig(score);
                                    keysig->setTrack((staffIdx + i) * VOICES);
                                    keysig->setKeySigEvent(key);
                                    keysig->setVisible(printObject == "yes");
                                    Segment* s = measure->getSegment(keysig, tick);
                                    s->add(keysig);
                                    }
                              }
                        }
                  else {
                        //
                        // apply key to staff(staffIdx) only
                        //
                        KeySigEvent oldkey = score->staff(staffIdx)->keymap()->key(tick);
                        if (oldkey != key) {
                              // new key differs from key in effect at this tick
                              KeySig* keysig = new KeySig(score);
                              keysig->setTrack(staffIdx * VOICES);
                              keysig->setKeySigEvent(key);
                              keysig->setVisible(printObject == "yes");
                              Segment* s = measure->getSegment(keysig, tick);
                              s->add(keysig);
                              }
                        }
                  }
            else if (e.tagName() == "time") {
                  timeSymbol = e.attribute("symbol");
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "beats")
                              beats = ee.text();
                        else if (ee.tagName() == "beat-type") {
                              beatType = ee.text();
                              }
                        else if (ee.tagName() == "senza-misura")
                              ;
                        else
                              domError(ee);
                        }
                  }
            else if (e.tagName() == "clef") {
                  int st = xmlClef(e, staff, measure);
                  int number = e.attribute(QString("number"), "-1").toInt();
                  int staffIdx = staff;
                  if (number != -1)
                        staffIdx += number - 1;
                  // qDebug("xmlAttributes clef score->staff(0) %p staffIdx %d score->staff(%d) %p",
                  //       score->staff(0), staffIdx, staffIdx, score->staff(staffIdx));
                  if (st != STANDARD_STAFF_TYPE)
                        score->staff(staffIdx)->setStaffType(score->staffType(st));
                  }
            else if (e.tagName() == "staves")
                  ;  // ignore, already handled
            else if (e.tagName() == "staff-details") {
                  int number = e.attribute(QString("number"), "-1").toInt();
                  int staffIdx = staff;
                  if (number != -1)
                        staffIdx += number - 1;
                  StringData* t = 0;
                  if (score->staff(staffIdx)->isTabStaff())
                        t = new StringData;
                  xmlStaffDetails(score, staff, t, e);
                  }
            else if (e.tagName() == "instruments")
                  domNotImplemented(e);
            else if (e.tagName() == "transpose") {
                  Interval interval;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        int i = ee.text().toInt();
                        if (ee.tagName() == "diatonic")
                              interval.diatonic = i;
                        else if (ee.tagName() == "chromatic")
                              interval.chromatic = i;
                        else
                              domError(ee);
                        }
                  score->staff(staff)->part()->instr()->setTranspose(interval);
                  }
            else if (e.tagName() == "measure-style")
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "multiple-rest") {
                              int multipleRest = ee.text().toInt();
                              if (multipleRest > 1) {
                                    multiMeasureRestCount = multipleRest - 1;
                                    startMultiMeasureRest = true;
                                    }
                              else
                                    qDebug("ImportMusicXml: multiple-rest %d not supported",
                                           multipleRest);
                              }
                        else
                              domError(ee);
                        }
            else
                  domError(e);
            }
      if (beats != "" && beatType != "") {
            // determine if timesig is valid
            TimeSigType st  = TSIG_NORMAL;
            int bts = 0; // total beats as integer (beats may contain multiple numbers, separated by "+")
            int btp = 0; // beat-type as integer
            if (determineTimeSig(beats, beatType, timeSymbol, st, bts, btp)) {
                  fractionTSig = Fraction(bts, btp);
                  // add timesig to all staves
                  //ws score->sigmap()->add(tick, TimeSig::getSig(st));
                  Part* part = score->staff(staff)->part();
                  int staves = part->nstaves();
                  for (int i = 0; i < staves; ++i) {
                        TimeSig* timesig = new TimeSig(score);
                        timesig->setTrack((staff + i) * VOICES);
                        timesig->setSig(fractionTSig, st);
                        // handle simple compound time signature
                        if (beats.contains(QChar('+'))) {
                              timesig->setNumeratorString(beats);
                              timesig->setDenominatorString(beatType);
                              }
                        Segment* s = measure->getSegment(timesig, tick);
                        s->add(timesig);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   addLyrics -- add a single lyric to the score
//                or delete it (if number too high)
//---------------------------------------------------------

static void addLyric(ChordRest* cr, Lyrics* l, int lyricNo)
      {
      if (lyricNo > MAX_LYRICS) {
            qDebug("too much lyrics (>%d)", MAX_LYRICS);
            delete l;
            }
      else {
            l->setNo(lyricNo);
            cr->add(l);
            }
      }

//---------------------------------------------------------
//   addLyrics -- add a notes lyrics to the score
//---------------------------------------------------------

static void addLyrics(ChordRest* cr,
                      QMap<int, Lyrics*>& numbrdLyrics,
                      QMap<int, Lyrics*>& defyLyrics,
                      QList<Lyrics*>& unNumbrdLyrics)
      {
      // first the lyrics with valid number
      int lyricNo = -1;
      for (QMap<int, Lyrics*>::const_iterator i = numbrdLyrics.constBegin(); i != numbrdLyrics.constEnd(); ++i) {
            lyricNo = i.key(); // use number obtained from MusicXML file
            Lyrics* l = i.value();
            addLyric(cr, l, lyricNo);
            }

      // then the lyrics without valid number but with valid default-y
      for (QMap<int, Lyrics*>::const_iterator i = defyLyrics.constBegin(); i != defyLyrics.constEnd(); ++i) {
            lyricNo++; // use sequence number
            Lyrics* l = i.value();
            addLyric(cr, l, lyricNo);
            }

      // finally the remaining lyrics, which are simply added in order they appear in the MusicXML file
      for (QList<Lyrics*>::const_iterator i = unNumbrdLyrics.constBegin(); i != unNumbrdLyrics.constEnd(); ++i) {
            lyricNo++; // use sequence number
            Lyrics* l = *i;
            addLyric(cr, l, lyricNo);
            }
      }

//---------------------------------------------------------
//   xmlLyric -- parse a MusicXML lyric element
//---------------------------------------------------------

void MusicXml::xmlLyric(int trk, QDomElement e,
                        QMap<int, Lyrics*>& numbrdLyrics,
                        QMap<int, Lyrics*>& defyLyrics,
                        QList<Lyrics*>& unNumbrdLyrics)
      {
      Lyrics* l = new Lyrics(score);
      //TODO-WS l->setTick(tick);
      l->setTrack(trk);

      bool ok = true;
      int lyricNo = e.attribute(QString("number")).toInt(&ok) - 1;
      if (ok) {
            if (lyricNo < 0) {
                  qDebug("invalid lyrics number (<0)");
                  delete l;
                  }
            else if (lyricNo > MAX_LYRICS) {
                  qDebug("too much lyrics (>%d)", MAX_LYRICS);
                  delete l;
                  }
            else {
                  numbrdLyrics[lyricNo] = l;
                  }
            }
      else {
            int defaultY = e.attribute(QString("default-y")).toInt(&ok);
            if (ok)
                  // invert default-y as it decreases with increasing lyric number
                  defyLyrics[-defaultY] = l;
            else
                  unNumbrdLyrics.append(l);
            }

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "syllabic") {
                  if (e.text() == "single")
                        l->setSyllabic(Lyrics::SINGLE);
                  else if (e.text() == "begin")
                        l->setSyllabic(Lyrics::BEGIN);
                  else if (e.text() == "end")
                        l->setSyllabic(Lyrics::END);
                  else if (e.text() == "middle")
                        l->setSyllabic(Lyrics::MIDDLE);
                  else
                        qDebug("unknown syllabic %s", qPrintable(e.text()));
                  }
            else if (e.tagName() == "text")
                  l->setText(l->text()+e.text());
            else if (e.tagName() == "elision")
                  if (e.text().isEmpty()) {
                        l->setText(l->text()+" ");
                        }
                  else {
                        l->setText(l->text()+e.text());
                        }
            else if (e.tagName() == "extend")
                  ;
            else if (e.tagName() == "end-line")
                  ;
            else if (e.tagName() == "end-paragraph")
                  ;
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   hasElem
//---------------------------------------------------------

/**
 Determine if \a e has a child \a tagname.
 */

static bool hasElem(const QDomElement e, const QString& tagname)
      {
      return !e.elementsByTagName(tagname).isEmpty();
      }

//---------------------------------------------------------
//   nrOfGraceSegsReq
//---------------------------------------------------------

/**
 Determine the number of grace segments required for the grace note represented by \a n.
 */

static int nrOfGraceSegsReq(QDomNode n)
      {
      if (!n.isElement() || n.nodeName() != "note") return 0;
      QDomElement e = n.toElement();
      if (!hasElem(e, "grace")) return 0;
      int nsegs = 0;
      // when counting starts in a grace chord but not at the first note,
      // compensate for the missed first note
      if (hasElem(e, "chord")) nsegs = 1;
      // count the number of grace chords
      // i.e the number of notes with grace but without chord child elements
      for (; !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag != "note")
                  qDebug("nrOfGraceSegsReq: found non-note tag '%s'",
                         qPrintable(tag));
            if (!hasElem(e, "grace"))
                  // non-grace note found, done
                  return nsegs;
            if (!hasElem(e, "chord"))
                  // first note of grace chord found
                  ++nsegs;
            }
      return 0;
      }

//---------------------------------------------------------
//   tupletAssert -- check assertions for tuplet handling
//---------------------------------------------------------

/**
 Check assertions for tuplet handling. If this fails, MusicXML
 import will almost certainly break in non-obvious ways.
 Should never happen, thus it is OK to quit the application.
 */

static void tupletAssert()
      {
      if (!(TDuration::V_BREVE      == TDuration::V_LONG + 1
            && TDuration::V_WHOLE   == TDuration::V_BREVE + 1
            && TDuration::V_HALF    == TDuration::V_WHOLE + 1
            && TDuration::V_QUARTER == TDuration::V_HALF + 1
            && TDuration::V_EIGHT   == TDuration::V_QUARTER + 1
            && TDuration::V_16TH    == TDuration::V_EIGHT + 1
            && TDuration::V_32ND    == TDuration::V_16TH + 1
            && TDuration::V_64TH    == TDuration::V_32ND + 1
            && TDuration::V_128TH   == TDuration::V_64TH + 1
            && TDuration::V_256TH   == TDuration::V_128TH + 1
            )) {
            qFatal("tupletAssert() failed");
            }
      }

//---------------------------------------------------------
//   smallestTypeAndCount
//---------------------------------------------------------

/**
 Determine the smallest note type and the number of those
 present in a ChordRest.
 For a note without dots the type equals the note type
 and count is one.
 For a single dotted note the type equals half the note type
 and count is three.
 A double dotted note is similar.
 Note: code assumes when duration().type() is incremented,
 the note length is divided by two, checked by tupletAssert().
 */

static void smallestTypeAndCount(ChordRest const* const cr, int& type, int& count)
      {
      type = cr->durationType().type();
      count = 1;
      switch (cr->durationType().dots()) {
            case 0:
                  // nothing to do
                  break;
            case 1:
                  type += 1; // next-smaller type
                  count = 3;
                  break;
            case 2:
                  type += 2; // next-next-smaller type
                  count = 7;
                  break;
            default:
                  qDebug("smallestTypeAndCount() does not support more than 2 dots");
            }
      }

//---------------------------------------------------------
//   matchTypeAndCount
//---------------------------------------------------------

/**
 Given two note types and counts, if the types are not equal,
 make them equal by successively doubling the count of the
 largest type.
 */

static void matchTypeAndCount(int& type1, int& count1, int& type2, int& count2)
      {
      while (type1 < type2) {
            type1++;
            count1 *= 2;
            }
      while (type2 < type1) {
            type2++;
            count2 *= 2;
            }
      }

//---------------------------------------------------------
//   determineTupletTypeAndCount
//---------------------------------------------------------

/**
 Determine type and number of smallest notes in the tuplet
 */

static void determineTupletTypeAndCount(Tuplet* t, int& tupletType, int& tupletCount)
      {
      int elemCount   = 0; // number of tuplet elements handled

      foreach (DurationElement* de, t->elements()) {
            if (de->type() == Element::CHORD || de->type() == Element::REST) {
                  ChordRest* cr = static_cast<ChordRest*>(de);
                  if (elemCount == 0) {
                        // first note: init variables
                        smallestTypeAndCount(cr, tupletType, tupletCount);
                        }
                  else {
                        int noteType = 0;
                        int noteCount = 0;
                        smallestTypeAndCount(cr, noteType, noteCount);
                        // match the types
                        matchTypeAndCount(tupletType, tupletCount, noteType, noteCount);
                        tupletCount += noteCount;
                        }
                  }
            elemCount++;
            }
      }

//---------------------------------------------------------
//   determineTupletBaseLen
//---------------------------------------------------------

/**
 Determine tuplet baseLen as determined by the tuplet ratio,
 and type and number of smallest notes in the tuplet.

 Example: baselen of a 3:2 tuplet with 1/16, 1/8, 1/8 and 1/16
 is 1/8. For this tuplet smalles note is 1/16, count is 6.
 */

TDuration determineTupletBaseLen(Tuplet* t)
      {
      int tupletType  = 0; // smallest note type in the tuplet
      int tupletCount = 0; // number of smallest notes in the tuplet

      // first determine type and number of smallest notes in the tuplet
      determineTupletTypeAndCount(t, tupletType, tupletCount);

      // sanity check:
      // for a 3:2 tuplet, count must be a multiple of 3
      if (tupletCount % t->ratio().numerator()) {
            qDebug("determineTupletBaseLen(%p) cannot divide count %d by %d", t, tupletCount, t->ratio().numerator());
            return TDuration();
            }

      // calculate baselen in smallest notes
      tupletCount /= t->ratio().numerator();

      // normalize
      while (tupletCount > 1 && (tupletCount % 2) == 0) {
            tupletCount /= 2;
            tupletType  -= 1;
            }

      return TDuration(TDuration::DurationType(tupletType));
      }

//---------------------------------------------------------
//   isTupletFilled
//---------------------------------------------------------

/**
 Determine if the tuplet contains the required number of notes,
 either (1) of the specified normal type
 or (2) the amount of the smallest notes in the tuplet equals
 actual notes.

 Example (1): a 3:2 tuplet with a 1/4 and a 1/8 note is filled
              if normal type is 1/8, it is not filled if normal
              type is 1/4.

 Example (2): a 3:2 tuplet with a 1/4 and a 1/8 note is filled.

 Use note types instead of duration to prevent errors due to rounding.
 */

bool isTupletFilled(Tuplet* t, TDuration normalType)
      {
      if (!t) return false;

      int tupletType  = 0; // smallest note type in the tuplet
      int tupletCount = 0; // number of smallest notes in the tuplet

      // first determine type and number of smallest notes in the tuplet
      determineTupletTypeAndCount(t, tupletType, tupletCount);

      // then compare ...
      if (normalType.isValid()) {
            int matchedNormalType  = normalType.type();
            int matchedNormalCount = t->ratio().numerator();
            // match the types
            matchTypeAndCount(tupletType, tupletCount, matchedNormalType, matchedNormalCount);
            // ... result scenario (1)
            return tupletCount >= matchedNormalCount;
            }
      else {
            // ... result scenario (2)
            return tupletCount >= t->ratio().numerator();
            }
      }

//---------------------------------------------------------
//   xmlTuplet
//---------------------------------------------------------

/**
 Parse and handle tuplet(s)
 Tuplets with <actual-notes> and <normal-notes> but without <tuplet>
 are handled correctly.
 TODO Nested tuplets are not (yet) supported.

 Note that cr must be initialized: fields measure, score, tick
 and track are used.
 */

void xmlTuplet(Tuplet*& tuplet, ChordRest* cr, int ticks, QDomElement e)
      {
      int actualNotes = 1;
      int normalNotes = 1;
      TDuration normalType;
      bool rest = false;
      QString tupletType;
      QString tupletPlacement;
      QString tupletBracket;
      QString tupletShowNumber;

      // parse the elements required for tuplet handling
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString s(e.text());
            if (tag == "notations") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "tuplet") {
                              tupletType       = ee.attribute("type");
                              tupletPlacement  = ee.attribute("placement");
                              tupletBracket    = ee.attribute("bracket");
                              tupletShowNumber = ee.attribute("show-number");
                              }
                        }
                  }
            else if (tag == "rest") {
                  rest = true;
                  }
            else if (tag == "time-modification") {  // tuplets
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "actual-notes")
                              actualNotes = ee.text().toInt();
                        else if (ee.tagName() == "normal-notes")
                              normalNotes = ee.text().toInt();
                        else if (ee.tagName() == "normal-type") {
                              // "measure" is not a valid normal-type,
                              // but would be accepted by setType()
                              if (ee.text() != "measure")
                                    normalType.setType(ee.text());
                              }
                        else
                              domError(ee);
                        }
                  }
            }

      // detect tremolo
      if (actualNotes == 2 && normalNotes == 1)
            return;

      // Special case:
      // Encore generates rests in tuplets w/o <tuplet> or <time-modification>.
      // Detect this by comparing the actual duration with the expected duration
      // based on note type. If actual is 2/3 of expected, the rest is part
      // of a tuplet.
      if (rest && actualNotes == 1 && normalNotes == 1 && cr->duration().ticks() != ticks) {
            if (2 * cr->duration().ticks() == 3 * ticks) {
                  actualNotes = 3;
                  normalNotes = 2;
                  }
            }

      // check for obvious errors
      if (tupletType == "start" && tuplet) {
            qDebug("tuplet already started");
            // TODO: how to recover ?
            }
      if (tupletType == "stop" && !tuplet) {
            qDebug("tuplet stop but no tuplet started");
            // TODO: how to recover ?
            }
      if (tupletType != "" && tupletType != "start" && tupletType != "stop") {
            qDebug("unknown tuplet type %s", qPrintable(tupletType));
            }

      // Tuplet are either started by the tuplet start
      // or when the time modification is first found.
      if (!tuplet) {
            if (tupletType == "start"
                || (!tuplet && (actualNotes != 1 || normalNotes != 1))) {
                  tuplet = new Tuplet(cr->score());
                  tuplet->setTrack(cr->track());
                  tuplet->setRatio(Fraction(actualNotes, normalNotes));
                  tuplet->setTick(cr->tick());
                  // set bracket, leave at default if unspecified
                  if (tupletBracket == "yes")
                        tuplet->setBracketType(Tuplet::SHOW_BRACKET);
                  else if (tupletBracket == "no")
                        tuplet->setBracketType(Tuplet::SHOW_NO_BRACKET);
                  // set number, default is "actual" (=SHOW_NUMBER)
                  if (tupletShowNumber == "both")
                        tuplet->setNumberType(Tuplet::SHOW_RELATION);
                  else if (tupletShowNumber == "none")
                        tuplet->setNumberType(Tuplet::NO_TEXT);
                  else
                        tuplet->setNumberType(Tuplet::SHOW_NUMBER);
                  // TODO type, placement, bracket
                  tuplet->setParent(cr->measure());
                  }
            }

      // Add chord to the current tuplet.
      // Must also check for actual/normal notes to prevent
      // adding one chord too much if tuplet stop is missing.
      if (tuplet && !(actualNotes == 1 && normalNotes == 1)) {
            cr->setTuplet(tuplet);
            tuplet->add(cr);
            }

      // Tuplets are stopped by the tuplet stop
      // or when the tuplet is filled completely
      // (either with knowledge of the normal type
      // or as a last resort calculated based on
      // actual and normal notes plus total duration)
      // or when the time-modification is not found.
      if (tuplet) {
            if (tupletType == "stop"
                || isTupletFilled(tuplet, normalType)
                || (actualNotes == 1 && normalNotes == 1)) {
                  // set baselen
                  TDuration td = determineTupletBaseLen(tuplet);
                  td.print();
                  // qDebug("stop tuplet %p basetype %d", tuplet, tupletType);
                  tuplet->setBaseLen(td);
                  // TODO determine usefulness of following check
                  int totalDuration = 0;
                  foreach (DurationElement* de, tuplet->elements()) {
                        if (de->type() == Element::CHORD || de->type() == Element::REST) {
                              totalDuration+=de->globalDuration().ticks();
                              }
                        }
                  if (!(totalDuration && normalNotes)) {
                        qDebug("MusicXML::import: tuplet stop but bad duration");
                        }
                  tuplet = 0;
                  }
            }
      }

//---------------------------------------------------------
//   addArticulationToChord
//---------------------------------------------------------

/**
 Add Articulation to Chord.
 */

static void addArticulationToChord(ChordRest* cr, ArticulationType articSym, QString dir)
      {
      Articulation* na = new Articulation(cr->score());
      na->setArticulationType(articSym);
      if (dir == "up") {
            na->setUp(true);
            na->setAnchor(A_TOP_STAFF);
            }
      else if (dir == "down") {
            na->setUp(false);
            na->setAnchor(A_BOTTOM_STAFF);
            }
      cr->add(na);
      }

//---------------------------------------------------------
//   addMordentToChord
//---------------------------------------------------------

/**
 Add Mordent to Chord.
 */

static void addMordentToChord(ChordRest* cr, QString name, QString attrLong, QString attrAppr, QString attrDep)
      {
      ArticulationType articSym = ARTICULATIONS; // legal but impossible ArticulationType value here indicating "not found"
      if (name == "inverted-mordent") {
            if ((attrLong == "" || attrLong == "no") && attrAppr == "" && attrDep == "") articSym = Articulation_Prall;
            else if (attrLong == "yes" && attrAppr == "" && attrDep == "") articSym = Articulation_PrallPrall;
            else if (attrLong == "yes" && attrAppr == "below" && attrDep == "") articSym = Articulation_UpPrall;
            else if (attrLong == "yes" && attrAppr == "above" && attrDep == "") articSym = Articulation_DownPrall;
            else if (attrLong == "yes" && attrAppr == "" && attrDep == "below") articSym = Articulation_PrallDown;
            else if (attrLong == "yes" && attrAppr == "" && attrDep == "above") articSym = Articulation_PrallUp;
            }
      else if (name == "mordent") {
            if ((attrLong == "" || attrLong == "no") && attrAppr == "" && attrDep == "") articSym = Articulation_Mordent;
            else if (attrLong == "yes" && attrAppr == "" && attrDep == "") articSym = Articulation_PrallMordent;
            else if (attrLong == "yes" && attrAppr == "below" && attrDep == "") articSym = Articulation_UpMordent;
            else if (attrLong == "yes" && attrAppr == "above" && attrDep == "") articSym = Articulation_DownMordent;
            }
      if (articSym != ARTICULATIONS) {
            Articulation* na = new Articulation(cr->score());
            na->setArticulationType(articSym);
            cr->add(na);
            }
      else
            qDebug("unknown ornament: name '%s' long '%s' approach '%s' departure '%s'",
                   qPrintable(name), qPrintable(attrLong), qPrintable(attrAppr), qPrintable(attrDep));
      }

//---------------------------------------------------------
//   readArticulations
//---------------------------------------------------------

/**
 Read a "simple" MuseScore articulation.
 These are the articulations that can be
 - represented by an enum ArticulationType
 - added to a ChordRest
 Return true (articulation recognized and handled)
 or false (articulation not recognized).
 Note simple implementation: MusicXML syntax is not strictly
 checked, the articulations parent element does not matter.
 */

static bool readArticulations(ChordRest* cr, QString mxmlName)
      {
      QMap<QString, ArticulationType> map; // map MusicXML articulation name to MuseScore symbol
      map["accent"]           = Articulation_Sforzatoaccent;
      map["staccatissimo"]    = Articulation_Staccatissimo;
      map["staccato"]         = Articulation_Staccato;
      map["tenuto"]           = Articulation_Tenuto;
      map["turn"]             = Articulation_Turn;
      map["inverted-turn"]    = Articulation_Reverseturn;
      map["stopped"]          = Articulation_Plusstop;
      map["up-bow"]           = Articulation_Upbow;
      map["down-bow"]         = Articulation_Downbow;
      map["detached-legato"]  = Articulation_Portato;
      map["spiccato"]         = Articulation_Staccatissimo;
      map["snap-pizzicato"]   = Articulation_Snappizzicato;
      map["schleifer"]        = Articulation_Schleifer;
      map["open-string"]      = Articulation_Ouvert;
      map["thumb-position"]   = Articulation_Thumb;

      if (map.contains(mxmlName)) {
            addArticulationToChord(cr, map.value(mxmlName), "");
            return true;
            }
      else
            return false;
      }

//---------------------------------------------------------
//   convertAccidental
//---------------------------------------------------------

/**
 Convert a MusicXML accidental name to a MuseScore enum Accidental::Type.
 */

static Accidental::AccidentalType convertAccidental(QString mxmlName)
      {
      QMap<QString, Accidental::AccidentalType> map; // map MusicXML accidental name to MuseScore enum Accidental::Type
      map["natural"] = Accidental::ACC_NATURAL;
      map["flat"] = Accidental::ACC_FLAT;
      map["sharp"] = Accidental::ACC_SHARP;
      map["double-sharp"] = Accidental::ACC_SHARP2;
      map["sharp-sharp"] = Accidental::ACC_SHARP2;
      map["flat-flat"] = Accidental::ACC_FLAT2;
      map["double-flat"] = Accidental::ACC_FLAT2;
      map["natural-flat"] = Accidental::ACC_NONE;

      map["quarter-flat"] = Accidental::ACC_MIRRORED_FLAT;
      map["quarter-sharp"] = Accidental::ACC_SHARP_SLASH;
      map["three-quarters-flat"] = Accidental::ACC_MIRRORED_FLAT2;
      map["three-quarters-sharp"] = Accidental::ACC_SHARP_SLASH4;

      map["sharp-down"] = Accidental::ACC_SHARP_ARROW_DOWN;
      map["sharp-up"] = Accidental::ACC_SHARP_ARROW_UP;
      map["natural-down"] = Accidental::ACC_NATURAL_ARROW_DOWN;
      map["natural-up"] = Accidental::ACC_NATURAL_ARROW_UP;
      map["flat-down"] = Accidental::ACC_FLAT_ARROW_DOWN;
      map["flat-up"] = Accidental::ACC_FLAT_ARROW_UP;

      map["slash-quarter-sharp"] = Accidental::ACC_MIRRIRED_FLAT_SLASH;
      map["slash-sharp"] = Accidental::ACC_SHARP_SLASH;
      map["slash-flat"] = Accidental::ACC_FLAT_SLASH;
      map["double-slash-flat"] = Accidental::ACC_FLAT_SLASH2;

      map["sori"] = Accidental::ACC_SORI;
      map["koron"] = Accidental::ACC_KORON;

      map["natural-sharp"] = Accidental::ACC_NONE;

      if (map.contains(mxmlName))
            return map.value(mxmlName);
      else
            qDebug("unknown accidental %s", qPrintable(mxmlName));
      // default: return Accidental::ACC_NONE
      return Accidental::ACC_NONE;
      }

//---------------------------------------------------------
//   convertNotehead
//---------------------------------------------------------

/**
 Convert a MusicXML notehead name to a MuseScore headgroup.
 */

static Note::NoteHeadGroup convertNotehead(QString mxmlName)
      {
      QMap<QString, int> map; // map MusicXML notehead name to a MuseScore headgroup
      map["slash"] = 5;
      map["triangle"] = 3;
      map["diamond"] = 2;
      map["x"] = 1;
      map["circle-x"] = 6;
      map["do"] = 7;
      map["re"] = 8;
      map["mi"] = 4;
      map["fa"] = 9;
      map["so"] = 12;
      map["la"] = 10;
      map["ti"] = 11;
      map["normal"] = 0;

      if (map.contains(mxmlName))
            return Note::NoteHeadGroup(map.value(mxmlName));
      else
            qDebug("unknown notehead %s", qPrintable(mxmlName));
      // default: return 0
      return Note::HEAD_NORMAL;
      }

//---------------------------------------------------------
//   addTextToNote
//---------------------------------------------------------

/**
 Add Text to Note.
 */

static void addTextToNote(QString txt, int style, Score* score, Note* note)
      {
      if (!txt.isEmpty()) {
            Text* t = new Fingering(score);
            t->setTextStyleType(style);
            t->setText(txt);
            note->add(t);
            }
      }

//---------------------------------------------------------
//   addFermata
//---------------------------------------------------------

/**
 Add a MusicXML fermata.
 Note: MusicXML common.mod: "The fermata type is upright if not specified."
 */

static void addFermata(ChordRest* cr, const QString type, const ArticulationType articSym)
      {
      if (type == "upright" || type == "")
            addArticulationToChord(cr, articSym, "up");
      else if (type == "inverted")
            addArticulationToChord(cr, articSym, "down");
      else
            qDebug("unknown fermata type '%s'", qPrintable(type));
      }

//---------------------------------------------------------
//   xmlFermata
//---------------------------------------------------------

/**
 Read a MusicXML fermata.
 Note: MusicXML common.mod: "An empty fermata element represents a normal fermata."
 */

static void xmlFermata(ChordRest* cr, QDomElement e)
      {
      QString fermata     = e.text();
      QString fermataType = e.attribute(QString("type"));

      if (fermata == "normal" || fermata == "")
            addFermata(cr, fermataType, Articulation_Fermata);
      else if (fermata == "angled")
            addFermata(cr, fermataType, Articulation_Shortfermata);
      else if (fermata == "square")
            addFermata(cr, fermataType, Articulation_Longfermata);
      else
            qDebug("unknown fermata '%s'\n", qPrintable(fermata));
      }

//---------------------------------------------------------
//   xmlNotations
//---------------------------------------------------------

/**
 Read MusicXML notations.
 */

void MusicXml::xmlNotations(Note* note, ChordRest* cr, int trk, int ticks, QDomElement e)
      {
      Measure* measure = cr->measure();
      int track = cr->track();

      QString wavyLineType;
      QString arpeggioType;
      QString glissandoType;
      int breath = -1;
      int tremolo = 0;
      QString tremoloType;
      QString placement;
      QStringList dynamics;
      qreal rx = 0.0;
      qreal ry = 0.0;
      qreal yoffset = 0.0; // actually this is default-y
      // qreal xoffset = 0.0; // not used
      bool hasYoffset = false;
      QSet<QString> slurIds;             // combination start/stop and number must be unique within a note
      QString chordLineType;

      for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
            if (ee.tagName() == "slur") {
                  int slurNo   = ee.attribute(QString("number"), "1").toInt() - 1;
                  QString slurType = ee.attribute(QString("type"));
                  QString lineType  = ee.attribute(QString("line-type"), "solid");
                  // PriMus Music-Notation by Columbussoft (build 10093) generates overlapping
                  // slurs that do not have a number attribute to distinguish them.
                  // The duplicates must be ignored, to prevent memory allocation issues,
                  // which caused a MuseScore crash

                  QString slurId = QString("slur %1").arg(slurType) + QString(" %1").arg(slurNo);
                  bool unique = !slurIds.contains(slurId);

                  if (unique) {
                        slurIds.insert(slurId);
                        if (slurType == "start") {
                              bool endSlur = false;
                              if (slur[slurNo] == 0) {
                                    slur[slurNo] = new Slur(score);
                                    if (lineType == "dotted")
                                          slur[slurNo]->setLineType(1);
                                    else if (lineType == "dashed")
                                          slur[slurNo]->setLineType(2);
                                    slur[slurNo]->setTick(cr->tick());
                                    slur[slurNo]->setStartElement(cr);
                                    }
                              else
                                    endSlur = true;
                              QString pl = ee.attribute(QString("placement"));
                              if (pl == "above")
                                    slur[slurNo]->setSlurDirection(MScore::UP);
                              else if (pl == "below")
                                    slur[slurNo]->setSlurDirection(MScore::DOWN);
                              //slur[slurNo]->setStart(tick, trk + voice);
                              //slur[slurNo]->setTrack((staff + relStaff) * VOICES);
                              slur[slurNo]->setTrack(track);
                              slur[slurNo]->setTrack2(track);
                              score->addElement(slur[slurNo]);
                              if (endSlur) {
                                    slur[slurNo]->setTick(cr->tick());
                                    slur[slurNo]->setStartElement(cr);
                                    slur[slurNo] = 0;
                                    }
                              }
                        else if (slurType == "stop") {
                              if (slur[slurNo] == 0) {
                                    slur[slurNo] = new Slur(score);
                                    slur[slurNo]->setTick2(cr->tick());
                                    slur[slurNo]->setTrack2(track);
                                    slur[slurNo]->setEndElement(cr);
                                    }
                              else {
                                    slur[slurNo]->setTick2(cr->tick());
                                    slur[slurNo]->setTrack2(track);
                                    slur[slurNo]->setEndElement(cr);
                                    slur[slurNo] = 0;
                                    }
                              }
                        else
                              qDebug("unknown slur type %s", qPrintable(slurType));
                        }
                  else
                        qDebug("ignoring duplicate '%s'", qPrintable(slurId));
                  }

            else if (ee.tagName() == "tied") {
                  QString tiedType = ee.attribute(QString("type"));
                  if (tiedType == "start") {
                        if (tie) {
                              qDebug("Tie already active");
                              }
                        else if (note) {
                              tie = new Tie(score);
                              // qDebug("use Tie %p", tie);
                              note->setTieFor(tie);
                              tie->setStartNote(note);
                              tie->setTrack(track);
                              tie = 0;
                              }
                        QString tiedOrientation = e.attribute("orientation", "auto");
                        if (tiedOrientation == "over")
                              tie->setSlurDirection(MScore::UP);
                        else if (tiedOrientation == "under")
                              tie->setSlurDirection(MScore::DOWN);
                        else if (tiedOrientation == "auto")
                              ;  // ignore
                        else
                              qDebug("unknown tied orientation: %s", tiedOrientation.toLatin1().data());
                        }
                  else if (tiedType == "stop")
                        ;  // ignore
                  else
                        qDebug("unknown tied type %s", tiedType.toLatin1().data());
                  }
            else if (ee.tagName() == "tuplet") {
                  // needed for tuplets, handled in xmlTuplet
                  }
            else if (ee.tagName() == "dynamics") {
                  // IMPORT_LAYOUT
                  placement = ee.attribute("placement");
                  if (preferences.musicxmlImportLayout) {
                        ry        = ee.attribute(QString("relative-y"), "0").toDouble() * -.1;
                        rx        = ee.attribute(QString("relative-x"), "0").toDouble() * .1;
                        yoffset   = ee.attribute("default-y").toDouble(&hasYoffset) * -0.1;
                        // xoffset   = ee.attribute("default-x", "0.0").toDouble() * 0.1;
                        }
                  QDomElement eee = ee.firstChildElement();
                  if (!eee.isNull()) {
                        if (eee.tagName() == "other-dynamics")
                              dynamics.push_back(eee.text());
                        else
                              dynamics.push_back(eee.tagName());
                        }
                  }
            else if (ee.tagName() == "articulations") {
                  for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                        if (readArticulations(cr, eee.tagName()))
                              continue;
                        else if (eee.tagName() == "breath-mark")
                              breath = 0;
                        else if (eee.tagName() == "caesura")
                              breath = 3;
                        else if (eee.tagName() == "doit"
                                 || eee.tagName() == "falloff"
                                 || eee.tagName() == "plop"
                                 || eee.tagName() == "scoop")
                              chordLineType = eee.tagName();
                        else if (eee.tagName() == "strong-accent") {
                              QString strongAccentType = eee.attribute(QString("type"));
                              if (strongAccentType == "up" || strongAccentType == "")
                                    addArticulationToChord(cr, Articulation_Marcato, "up");
                              else if (strongAccentType == "down")
                                    addArticulationToChord(cr, Articulation_Marcato, "down");
                              else
                                    qDebug("unknown mercato type %s", strongAccentType.toLatin1().data());
                              }
                        else
                              domError(eee);
                        }
                  }
            else if (ee.tagName() == "fermata")
                  xmlFermata(cr, ee);
            else if (ee.tagName() == "ornaments") {
                  bool trillMark = false;
                  // <trill-mark placement="above"/>
                  for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                        if (readArticulations(cr, eee.tagName()))
                              continue;
                        else if (eee.tagName() == "trill-mark")
                              trillMark = true;
                        else if (eee.tagName() == "wavy-line")
                              wavyLineType = eee.attribute(QString("type"));
                        else if (eee.tagName() == "tremolo") {
                              tremolo = eee.text().toInt();
                              tremoloType = eee.attribute(QString("type"));
                              }
                        else if (eee.tagName() == "accidental-mark")
                              domNotImplemented(eee);
                        else if (eee.tagName() == "delayed-turn")
                              // TODO: actually this should be offset a bit to the right
                              addArticulationToChord(cr, Articulation_Turn, "");
                        else if (eee.tagName() == "inverted-mordent"
                                 || eee.tagName() == "mordent")
                              addMordentToChord(cr, eee.tagName(), eee.attribute("long"), eee.attribute("approach"), eee.attribute("departure"));
                        else
                              domError(eee);
                        }
                  // note that mscore wavy line already implicitly includes a trillsym
                  // so don't add an additional one
                  if (trillMark && wavyLineType != "start")
                        addArticulationToChord(cr, Articulation_Trill, "");
                  }
            else if (ee.tagName() == "technical") {
                  for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                        if (readArticulations(cr, eee.tagName()))
                              continue;
                        else if (eee.tagName() == "fingering")
                              addTextToNote(eee.text(), TEXT_STYLE_FINGERING, score, note);
                        else if (eee.tagName() == "fret") {
                              if (note->staff()->isTabStaff())
                                    note->setFret(eee.text().toInt());
                              }
                        else if (eee.tagName() == "pluck")
                              addTextToNote(eee.text(), TEXT_STYLE_FINGERING, score, note);
                        else if (eee.tagName() == "string") {
                              if (note->staff()->isTabStaff())
                                    note->setString(eee.text().toInt() - 1);
                              else
                                    addTextToNote(eee.text(), TEXT_STYLE_STRING_NUMBER, score, note);
                              }
                        else if (eee.tagName() == "pull-off")
                              domNotImplemented(eee);
                        else
                              domError(eee);
                        }
                  }
            else if (ee.tagName() == "arpeggiate") {
                  arpeggioType = ee.attribute(QString("direction"));
                  if (arpeggioType == "") arpeggioType = "none";
                  }
            else if (ee.tagName() == "non-arpeggiate")
                  arpeggioType = "non-arpeggiate";
            // glissando and slide are added to the "stop" chord only
            else if (ee.tagName() == "glissando") {
                  if (ee.attribute("type") == "stop") glissandoType = "glissando";
                  }
            else if (ee.tagName() == "slide") {
                  if (ee.attribute("type") == "stop") glissandoType = "slide";
                  }
            else
                  domError(ee);
            }

      if (!arpeggioType.isEmpty()) {
            Arpeggio* a = new Arpeggio(score);
            if (arpeggioType == "none")
                  a->setArpeggioType(ArpeggioType::NORMAL);
            else if (arpeggioType == "up")
                  a->setArpeggioType(ArpeggioType::UP);
            else if (arpeggioType == "down")
                  a->setArpeggioType(ArpeggioType::DOWN);
            else if (arpeggioType == "non-arpeggiate")
                  a->setArpeggioType(ArpeggioType::BRACKET);
            else {
                  qDebug("unknown arpeggio type %s", qPrintable(arpeggioType));
                  delete a;
                  a = 0;
                  }
            if ((static_cast<Chord*>(cr))->arpeggio()) {
                  // there can be only one
                  delete a;
                  a = 0;
                  }
            else
                  cr->add(a);
            }

      if (!glissandoType.isEmpty()) {
            Glissando* g = new Glissando(score);
            if (glissandoType == "slide")
                  g->setGlissandoType(GlissandoType::STRAIGHT);
            else if (glissandoType == "glissando")
                  g->setGlissandoType(GlissandoType::WAVY);
            else {
                  qDebug("unknown glissando type %s", glissandoType.toLatin1().data());
                  delete g;
                  g = 0;
                  }
            if ((static_cast<Chord*>(cr))->glissando()) {
                  // there can be only one
                  delete g;
                  g = 0;
                  }
            else
                  cr->add(g);
            }

      if (!wavyLineType.isEmpty()) {
            if (wavyLineType == "start") {
                  if (trill) {
                        qDebug("overlapping wavy-line not supported");
                        delete trill;
                        trill = 0;
                        }
                  else {
                        trill = new Trill(score);
                        trill->setTrack(trk);
                        spanners[trill] = QPair<int, int>(tick, -1);
                        // qDebug("wedge trill=%p inserted at first tick %d", trill, tick);
                        }
                  }
            else if (wavyLineType == "stop") {
                  if (!trill) {
                        qDebug("wavy-line stop without start");
                        }
                  else {
                        spanners[trill].second = tick + ticks;
                        // qDebug("wedge trill=%p second tick %d", trill, tick);
                        trill = 0;
                        }
                  }
            else
                  qDebug("unknown wavy-line type %s", wavyLineType.toLatin1().data());
            }

      if (breath >= 0) {
            Breath* b = new Breath(score);
            // b->setTrack(trk + voice); TODO check next line
            b->setTrack(track);
            b->setBreathType(breath);
            Segment* seg = measure->getSegment(Segment::SegBreath, tick);
            seg->add(b);
            }

      if (tremolo) {
            if (tremolo == 1 || tremolo == 2 || tremolo == 3 || tremolo == 4) {
                  if (tremoloType == "" || tremoloType == "single") {
                        Tremolo* t = new Tremolo(score);
                        switch (tremolo) {
                              case 1: t->setTremoloType(TREMOLO_R8); break;
                              case 2: t->setTremoloType(TREMOLO_R16); break;
                              case 3: t->setTremoloType(TREMOLO_R32); break;
                              case 4: t->setTremoloType(TREMOLO_R64); break;
                              }
                        cr->add(t);
                        }
                  else if (tremoloType == "start") {
                        if (tremStart) qDebug("MusicXML::import: double tremolo start");
                        tremStart = static_cast<Chord*>(cr);
                        }
                  else if (tremoloType == "stop") {
                        if (tremStart) {
                              Tremolo* t = new Tremolo(score);
                              switch (tremolo) {
                                    case 1: t->setTremoloType(TREMOLO_C8); break;
                                    case 2: t->setTremoloType(TREMOLO_C16); break;
                                    case 3: t->setTremoloType(TREMOLO_C32); break;
                                    case 4: t->setTremoloType(TREMOLO_C64); break;
                                    }
                              t->setChords(tremStart, static_cast<Chord*>(cr));
                              // fixup chord duration and type
                              t->chord1()->setDurationType(ticks);
                              t->chord1()->setDuration(ticks);
                              t->chord2()->setDurationType(ticks);
                              t->chord2()->setDuration(ticks);
                              // add tremolo to first chord (only)
                              tremStart->add(t);
                              }
                        else qDebug("MusicXML::import: double tremolo stop w/o start");
                        tremStart = 0;
                        }
                  }
            else
                  qDebug("unknown tremolo type %d", tremolo);
            }

      if (chordLineType != "") {
            ChordLine* cl = new ChordLine(score);
            if (chordLineType == "falloff")
                  cl->setChordLineType(CHORDLINE_FALL);
            if (chordLineType == "doit")
                  cl->setChordLineType(CHORDLINE_DOIT);
            if (chordLineType == "plop")
                  cl->setChordLineType(CHORDLINE_PLOP);
            if (chordLineType == "scoop")
                  cl->setChordLineType(CHORDLINE_SCOOP);
            note->chord()->add(cl);
            }

      // more than one dynamic ???
      // LVIFIX: check import/export of <other-dynamics>unknown_text</...>
      // TODO remove duplicate code (see MusicXml::direction)
      for (QStringList::Iterator it = dynamics.begin(); it != dynamics.end(); ++it ) {
            Dynamic* dyn = new Dynamic(score);
            dyn->setDynamicType(*it);
            if (hasYoffset) dyn->setYoff(yoffset);
            addElement(dyn, hasYoffset, track / VOICES /* staff */, 0 /* rstaff */, score, placement,
                       rx, ry, 0 /*offset */, measure, tick);
            }

      }


//---------------------------------------------------------
//   findLastFiguredBass
//---------------------------------------------------------

/**
 * Find last figured bass on \a track before \a seg
 */

static FiguredBass* findLastFiguredBass(int track, Segment* seg)
      {
      // qDebug("findLastFiguredBass(track %d seg %p)", track, seg);
      while ((seg = seg->prev1(Segment::SegChordRest))) {
            // qDebug("findLastFiguredBass seg %p", seg);
            foreach(Element* e, seg->annotations()) {
                  if (e->track() == track && e->type() == Element::FIGURED_BASS) {
                        FiguredBass* fb = static_cast<FiguredBass*>(e);
                        // qDebug("findLastFiguredBass found fb %p at seg %p", fb, seg);
                        return fb;
                        }
                  }
            }
      return 0;
      }


//---------------------------------------------------------
//   xmlNote
//---------------------------------------------------------

/**
 Read a MusicXML note.

 \a Staff is the number of first staff of the part this note belongs to.
 */

void MusicXml::xmlNote(Measure* measure, int staff, const QString& partId, Beam*& beam, int& currentVoice, QDomElement e, QList<GraceNoteInfo>& gni)
      {
      int ticks = 0;
#ifdef DEBUG_TICK
      qDebug("xmlNote start tick=%d (%d div) divisions=%d", tick, tick * divisions / MScore::division, divisions);
#endif
      QDomNode pn = e; // TODO remove pn
      QDomElement org_e = e; // save e for later
      QDomElement domElemNotations;
      int voice = 0;
      int move  = 0;

      bool rest    = false;
      int relStaff = 0;
      BeamMode bm  = BeamMode::NONE;
      MScore::Direction sd = MScore::AUTO;
      int dots     = 0;
      bool grace   = false;
      QString graceSlash;
      QString step;
      int alter  = 0;
      int octave = 4;
      Accidental::AccidentalType accidental = Accidental::ACC_NONE;
      bool parentheses = false;
      bool editorial = false;
      bool cautionary = false;
      TDuration durationType(TDuration::V_INVALID);
      Note::NoteHeadGroup headGroup = Note::HEAD_NORMAL;
      bool noStem = false;
      QColor noteheadColor = QColor::Invalid;
      bool chord = false;
      int velocity = -1;
      bool unpitched = false;
      QString instrId;

      // first read all elements required for voice mapping
      QDomElement e2 = e.firstChildElement();
      for (; !e2.isNull(); e2 = e2.nextSiblingElement()) {
            QString tag(e2.tagName());
            QString s(e2.text());
            if (tag == "voice")
                  voice = s.toInt() - 1;
            else if (tag == "staff")
                  relStaff = s.toInt() - 1;
            else if (tag == "grace") {
                  grace = true;
                  }
            else if (tag == "duration") {
                  ticks = calcTicks(s, divisions);
                  }
            else if (tag == "chord") {
                  chord = true;
                  }
            // silently ignore others (will be handled later)
            }

      // Bug fix for Sibelius 7.1.3 which does not write <voice> for notes with <chord>
      if (!chord)
            // remember voice
            currentVoice = voice;
      else
            // use voice from last note w/o <chord>
            voice = currentVoice;

      // Musicxml voices are counted for all staffs of an
      // instrument. They are not limited. In mscore voices are associated
      // with a staff. Every staff can have at most VOICES voices.

      // The following lines map musicXml voices to mscore voices.
      // If a voice crosses two staffs, this is expressed with the
      // "move" parameter in mscore.

      // Musicxml voices are unique within a part, but not across parts.

      // qDebug("voice mapper before: relStaff=%d voice=%d staff=%d\n", relStaff, voice, staff);
      int s; // staff mapped by voice mapper
      int v; // voice mapped by voice mapper
      if (voicelist.value(voice).overlaps()) {
            // for overlapping voices, the staff does not change
            // and the voice is mapped and staff-dependent
            s = relStaff;
            v = voicelist.value(voice).voice(s);
            }
      else {
            // for non-overlapping voices, both staff and voice are
            // set by the voice mapper
            s = voicelist.value(voice).staff();
            v = voicelist.value(voice).voice();
            }

      // qDebug("voice mapper before: relStaff=%d voice=%d s=%d v=%d", relStaff, voice, s, v);
      if (s < 0 || v < 0) {
            qDebug("ImportMusicXml: too many voices (staff %d, relStaff %d, voice %d at line %d col %d)",
                   staff + 1, relStaff, voice + 1, e.lineNumber(), e.columnNumber());
            return;
            }
      else {
            int d = relStaff - s;
            relStaff = s;
            move += d;
            voice = v;
            }

      // qDebug("voice mapper after: relStaff=%d move=%d voice=%d", relStaff, move, voice);
      // note: relStaff is the staff number relative to the parts first staff
      //       voice is the voice number in the staff

      // determine tick for note
      int loc_tick = chord ? prevtick : tick;
      // qDebug("chord %d prevtick %d tick %d loc_tick %d", chord, prevtick, tick, loc_tick);

      // trk is first track of staff
      int trk = (staff + relStaff) * VOICES;
      // track is current track in staff
      int track = trk + voice;

      QString printObject = "yes";
      if (pn.isElement() && pn.nodeName() == "note") {
            QDomElement pne = pn.toElement();
            printObject = pne.attribute("print-object", "yes");
            }

      velocity = round(e.attribute("dynamics", "-1").toDouble() * 0.9);

      // storage for xmlLyric
      QMap<int, Lyrics*> numberedLyrics; // lyrics with valid number
      QMap<int, Lyrics*> defaultyLyrics; // lyrics with valid default-y
      QList<Lyrics*> unNumberedLyrics;   // lyrics with neither

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString s(e.text());

            if (tag == "pitch") {
                  step   = "C";
                  alter  = 0;
                  octave = 4;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "step")          // A-G
                              step = ee.text();
                        else if (ee.tagName() == "alter") {    // -1=flat 1=sharp (0.5=quarter sharp)
                              bool ok;
                              alter = MxmlSupport::stringToInt(ee.text(), &ok); // fractions not supported by mscore
                              if (!ok || alter < -2 || alter > 2) {
                                    qDebug("ImportXml: bad 'alter' value: %s at line %d col %d",
                                           qPrintable(ee.text()), ee.lineNumber(), ee.columnNumber());
                                    alter = 0;
                                    }
                              }
                        else if (ee.tagName() == "octave")   // 0-9 4=middle C
                              octave = ee.text().toInt();
                        else
                              domError(ee);
                        }
                  }
            else if (tag == "unpitched") {
                  //
                  // TODO: check semantic
                  //
                  unpitched = true;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "display-step")          // A-G
                              step = ee.text();
                        else if (ee.tagName() == "display-octave")   // 0-9 4=middle C
                              octave = ee.text().toInt();
                        else
                              domError(ee);
                        }
                  }
            else if (tag == "type")
                  durationType = TDuration(s);
            else if (tag == "chord" || tag == "duration" || tag == "staff" || tag == "voice")
                  // already handled by voice mapper, ignore here but prevent
                  // spurious "Unknown Node <staff>" or "... <voice>" messages
                  ;
            else if (tag == "stem") {
                  if (s == "up")
                        sd = MScore::UP;
                  else if (s == "down")
                        sd = MScore::DOWN;
                  else if (s == "none")
                        noStem = true;
                  else if (s == "double")
                        ;
                  else
                        qDebug("unknown stem direction %s", e.text().toLatin1().data());
                  }
            else if (tag == "beam") {
                  int beamNo = e.attribute(QString("number"), "1").toInt();
                  if (beamNo == 1) {
                        if (s == "begin")
                              bm = BeamMode::BEGIN;
                        else if (s == "end")
                              bm = BeamMode::END;
                        else if (s == "continue")
                              bm = BeamMode::MID;
                        else if (s == "backward hook")
                              ;
                        else if (s == "forward hook")
                              ;
                        else
                              qDebug("unknown beam keyword <%s>", s.toLatin1().data());
                        }
                  }
            else if (tag == "rest") {
                  rest = true;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "display-step")          // A-G
                              step = ee.text();
                        else if (ee.tagName() == "display-octave")   // 0-9 4=middle C
                              octave = ee.text().toInt();
                        else
                              domError(ee);
                        }
                  }
            else if (tag == "lyric")
                  xmlLyric(track, e, numberedLyrics, defaultyLyrics, unNumberedLyrics);
            else if (tag == "dot")
                  ++dots;
            else if (tag == "accidental") {
                  accidental = convertAccidental(s);
                  if (e.attribute(QString("cautionary")) == "yes")
                        cautionary = true;
                  if (e.attribute(QString("editorial")) == "yes")
                        editorial = true;
                  if (e.attribute(QString("parentheses")) == "yes")
                        parentheses = true;
                  }
            else if (tag == "notations") {
                  // save the QDomElement representing <notations> for later
                  domElemNotations = e;
                  }
            else if (tag == "tie") {
                  QString tieType = e.attribute(QString("type"));
                  if (tieType == "start")
                        ;
                  else if (tieType == "stop")
                        ;
                  else
                        qDebug("unknown tie type %s", tieType.toLatin1().data());
                  }
            else if (tag == "grace") {
                  graceSlash = e.attribute(QString("slash"));
                  }
            else if (tag == "time-modification") {
                  // needed for tuplets, handled in xmlTuplet
                  }
            else if (tag == "notehead") {
                  headGroup = convertNotehead(s);
                  QString color = e.attribute(QString("color"), 0);
                  if (color != 0)
                        noteheadColor = QColor(color);
                  }
            else if (tag == "instrument") {
                  instrId = e.attribute("id");
                  }
            else if (tag == "cue")
                  domNotImplemented(e);
            else
                  domError(e);
            }

      // qDebug("staff=%d relStaff=%d VOICES=%d voice=%d track=%d",
      //        staff, relStaff, VOICES, voice, track);

      // qDebug("%s at %d voice %d dur = %d, beat %d/%d div %d pitch %d ticks %d",
      //         rest ? "Rest" : "Note", loc_tick, voice, duration, beats, beatType,
      //         divisions, 0 /* pitch */, ticks);

      ChordRest* cr = 0;
      Note* note = 0;

      if (rest) {
            cr = new Rest(score);
            // By convention, whole measure rests do not have a "type" element
            // As of MusicXML 3.0, this can be indicated by an attribute "measure",
            // but for backwards compatibility the "old" convention still has to be supported.
            if (durationType.type() == TDuration::V_INVALID) {
                  // Verify the rest fits exactly in the measure, as some programs
                  // (e.g. Cakewalk SONAR X2 Studio [Version: 19.0.0.306]) leave out
                  // the type for all rests.
                  if (tick == measure->tick() && ticks == measure->ticks())
                        durationType.setType(TDuration::V_MEASURE);
                  else
                        durationType.setVal(ticks);
                  cr->setDurationType(durationType);
                  cr->setDuration(Fraction::fromTicks(ticks));
                  }
            else {
                  cr->setDurationType(durationType);
                  cr->setDots(dots);
                  cr->setDuration(cr->durationType().fraction());
                  }
            if (beam) {
                  if (beam->track() == track) {
                        cr->setBeamMode(BeamMode::MID);
                        beam->add(cr);
                        }
                  else
                        removeBeam(beam);
                  }
            else
                  cr->setBeamMode(BeamMode::NONE);
            cr->setTrack(track);
            static_cast<Rest*>(cr)->setStaffMove(move);
            Segment* s = measure->getSegment(cr, loc_tick);
            // Sibelius might export two rests at the same place, ignore the 2nd one
            // <?DoletSibelius Two NoteRests in same voice at same position may be an error?>
            if (!s->element(cr->track()))
                  s->add(cr);
            cr->setVisible(printObject == "yes");
            if (step != "" && 0 <= octave && octave <= 9) {
                  // qDebug("rest step=%s oct=%d", qPrintable(step), octave);
                  ClefType clef = cr->staff()->clef(loc_tick);
                  int po = ClefInfo::pitchOffset(clef);
                  int istep = step[0].toLatin1() - 'A';
                  // qDebug(" clef=%d po=%d istep=%d", clef, po, istep);
                  if (istep < 0 || istep > 6) {
                        qDebug("rest: illegal display-step %d, <%s>", istep, qPrintable(step));
                        }
                  else {
                        //                        a  b  c  d  e  f  g
                        static int table2[7]  = { 5, 6, 0, 1, 2, 3, 4 };
                        int dp = 7 * (octave + 2) + table2[istep];
                        // qDebug(" dp=%d po-dp=%d", dp, po-dp);
                        cr->setUserYoffset((po - dp + 3) * score->spatium() / 2);
                        }
                  }
            }
      else {
            char c = step[0].toLatin1();
            note = new Note(score);
            note->setHeadGroup(headGroup);
            if (noteheadColor != QColor::Invalid)
                  note->setColor(noteheadColor);

            if (velocity > 0) {
                  note->setVeloType(MScore::USER_VAL);
                  note->setVeloOffset(velocity);
                  }

            cr = measure->findChord(loc_tick, track);
            if (cr == 0) {
                  if(!grace) {
                        cr = new Chord(score);
                        cr->setBeamMode(bm);
                        cr->setTrack(track);

                        if (durationType.type() == TDuration::V_INVALID)
                              durationType.setType(TDuration::V_QUARTER);
                        cr->setDurationType(durationType);
                        cr->setDots(dots);
                        cr->setDuration(cr->durationType().fraction());
                        Segment* s = measure->getSegment(cr, loc_tick);
                        s->add(cr);
                        }
                  }
            if(cr)
                  cr->setStaffMove(move);

            // pitch must be set before adding note to chord as note
            // is inserted into pitch sorted list (ws)

            if (unpitched
                && drumsets.contains(partId)
                && drumsets[partId].contains(instrId)) {
                  // step and oct are display-step and ...-oct
                  // get pitch from instrument definition in drumset instead
                  int pitch = drumsets[partId][instrId].pitch;
                  note->setPitch(pitch);
                  // TODO - does this need to be key-aware?
                  note->setTpc(pitch2tpc(pitch, KEY_C, PREFER_NEAREST)); // TODO: necessary ?
                  }
            else
                  xmlSetPitch(note, c, alter, octave, ottava, track);

            if(grace) {
                  NoteType nt = NOTE_APPOGGIATURA;
                  int len = MScore::division/2;
                  if (graceSlash == "yes")
                         nt = NOTE_ACCIACCATURA;
                  if (durationType.type() == TDuration::V_QUARTER) {
                        nt = NOTE_GRACE4;
                        len = MScore::division;
                        }
                  else if (durationType.type() == TDuration::V_16TH) {
                        nt = NOTE_GRACE16;
                        len = MScore::division/4;
                        }
                  else if (durationType.type() == TDuration::V_32ND) {
                        nt = NOTE_GRACE32;
                        len = MScore::division/8;
                        }
                  gni.append({nt, note->pitch(), note->tpc(), len});
                  delete note;
                  return;
                  }

            cr->add(note);

            if(!grace && gni.size() > 0) {
                  for (int i = gni.size() - 1; i >= 0; --i) {
                        GraceNoteInfo g = gni.at(i);
                        score->setGraceNote(static_cast<Chord*>(cr), g.pitch, g.type, false, g.len, g.tpc);
                        }
                  gni.clear();
                  }

            static_cast<Chord*>(cr)->setNoStem(noStem);

            // qDebug("staff for new note: %p (staff=%d, relStaff=%d)",
            //        score->staff(staff + relStaff), staff, relStaff);

            if (editorial || cautionary || parentheses) {
                  Accidental* a = new Accidental(score);
                  a->setAccidentalType(accidental);
                  a->setHasBracket(cautionary || parentheses);
                  a->setRole(Accidental::ACC_USER);
                  note->add(a);
                  }

            // LVIFIX: quarter tone accidentals support is "drawing only"
            //WS-TODO if (accidental == 18
            // || accidental == 19
            // || accidental == 22
            // || accidental == 25)
            // note->setAccidentalType(accidental);

            // remember beam mode last non-grace note
            // bm == BeamMode::NONE means no <beam> was found
            if (!grace && bm != BeamMode::NONE)
                  beamMode = bm;

            // handle beam
            if (!chord && !grace)
                  handleBeamAndStemDir(cr, bm, sd, beam);

            note->setVisible(printObject == "yes");

            // set drumset information
            // note that in MuseScore, the drumset contains defaults for notehead,
            // line and stem direction, while a MusicXML file contains actuals.
            // the MusicXML values for each note are simply copied to the defaults

            if (unpitched) {
                  // determine staff line based on display-step / -octave and clef type
                  ClefType clef = cr->staff()->clef(loc_tick);
                  int po = ClefInfo::pitchOffset(clef);
                  int pitch = MusicXMLStepAltOct2Pitch(step[0].toLatin1(), 0, octave);
                  int line = po - absStep(pitch);

                  // correct for number of staff lines
                  // see ExportMusicXml::unpitch2xml for explanation
                  // TODO handle other # staff lines ?
                  int staffLines = cr->staff()->lines();
                  if (staffLines == 1) line -= 8;
                  if (staffLines == 3) line -= 2;

                  // the drum palette cannot handle stem direction AUTO,
                  // overrule if necessary
                  if (sd == MScore::AUTO) {
                        if (line > 4)
                              sd = MScore::DOWN;
                        else
                              sd = MScore::UP;
                        }

                  if (drumsets.contains(partId)
                      && drumsets[partId].contains(instrId)) {
                        drumsets[partId][instrId].notehead = headGroup;
                        drumsets[partId][instrId].line = line;
                        drumsets[partId][instrId].stemDirection = sd;
                        }
                  }
            }

      if (!chord && !grace) {
            xmlTuplet(tuplets[voice + relStaff * VOICES], cr, ticks, org_e);
            }

      if (!domElemNotations.isNull())
            xmlNotations(note, cr, trk, ticks, domElemNotations);

      // add lyrics found by xmlLyric
      addLyrics(cr, numberedLyrics, defaultyLyrics, unNumberedLyrics);

      // add figured bass element
      if (figBass) {
            // qDebug("add figured bass %p at tick %d ticks %d trk %d", figBass, tick, ticks, trk);
            figBass->setTrack(trk);
            figBass->setTicks(ticks);
            // TODO: set correct onNote value
            Segment* s = measure->getSegment(Segment::SegChordRest, tick);
            // TODO: use addelement() instead of Segment::add() ?
            s->add(figBass);
            figBass = 0;
            }
      else if (figBassExtend) {
            // extend last figured bass to end of this chord
            // qDebug("extend figured bass at tick %d ticks %d trk %d end %d", tick, ticks, trk, tick + ticks);
            FiguredBass* fb = findLastFiguredBass((trk / VOICES) * VOICES, cr->segment());
            if (fb)
                  fb->setTicks(tick + ticks - fb->segment()->tick());
            }
      figBassExtend = false;

      if (!chord)
            prevtick = tick;  // remember tick where last chordrest was inserted

#ifdef DEBUG_TICK
      qDebug(" after inserting note tick=%d prevtick=%d", tick, prevtick);
#endif
      }

//---------------------------------------------------------
//   readFretDiagram
//---------------------------------------------------------

static void readFretDiagram(FretDiagram* fd, QDomElement de)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            int val = e.text().toInt();
            if (tag == "frame-frets") {
                  if (val > 0)
                        fd->setFrets(val);
                  else
                        qDebug("FretDiagram::readMusicXML: illegal frame-fret %d", val);
                  }
            else if (tag == "frame-note") {
                  int fret   = -1;
                  int string = -1;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        const QString& tag(ee.tagName());
                        int val = ee.text().toInt();
                        if (tag == "fret")
                              fret = val;
                        else if (tag == "string")
                              string = val;
                        else
                              domError(ee);
                        }
                  qDebug("FretDiagram::readMusicXML string %d fret %d", string, fret);
                  if (string > 0) {
                        if (fret == 0)
                              fd->setMarker(fd->strings() - string, 79 /* ??? */);
                        else if (fret > 0)
                              fd->setDot(fd->strings() - string, fret);
                        }
                  }
            else if (tag == "frame-strings") {
                  if (val > 0) {
                        fd->setStrings(val);
                        for (int i = 0; i < val; ++i)
                              fd->setMarker(i, 88 /* ??? */);
                        }
                  else
                        qDebug("FretDiagram::readMusicXML: illegal frame-strings %d", val);
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   xmlHarmony
//---------------------------------------------------------

void MusicXml::xmlHarmony(QDomElement e, int tick, Measure* measure, int staff)
      {
      // type:

      // placement:
      double rx = 0.1 * e.attribute("relative-x", "0").toDouble();
      double ry = -0.1 * e.attribute("relative-y", "0").toDouble();

      double styleYOff = score->textStyle(TEXT_STYLE_HARMONY).offset().y();
      OffsetType offsetType = score->textStyle(TEXT_STYLE_HARMONY).offsetType();
      if (offsetType == OFFSET_ABS) {
            styleYOff = styleYOff * MScore::DPMM / score->spatium();
            }

      double dy = -0.1 * e.attribute("default-y", QString::number(styleYOff* -10)).toDouble();

      QString printObject(e.attribute("print-object", "yes"));
      QString printFrame(e.attribute("print-frame"));
      QString printStyle(e.attribute("print-style"));

      QString kind, kindText, symbols, parens;
      QList<HDegree> degreeList;

      if (harmony) {
            qDebug("MusicXML::import: more than one harmony");
            return;
            };

      Harmony* ha = new Harmony(score);
      ha->setUserOff(QPointF(rx, ry + dy - styleYOff));
      int offset = 0;
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "root") {
                  QString step;
                  int alter = 0;
                  bool invalidRoot = false;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        if (tag == "root-step") {
                              // attributes: print-style
                              step = ee.text();
                              if (ee.hasAttribute("text")) {
                                    QString rtext = ee.attribute("text");
                                    if (rtext == "") {
                                          invalidRoot = true;
                                          }
                                    }
                              }
                        else if (tag == "root-alter") {
                              // attributes: print-object, print-style
                              //             location (left-right)
                              alter = ee.text().toInt();
                              }
                        else
                              domError(ee);
                        }
                  if (invalidRoot)
                        ha->setRootTpc(INVALID_TPC);
                  else
                        ha->setRootTpc(step2tpc(step, AccidentalVal(alter)));
                  }
            else if (tag == "function") {
                  // attributes: print-style
                  domNotImplemented(e);
                  }
            else if (tag == "kind") {
                  // attributes: use-symbols  yes-no
                  //             text, stack-degrees, parentheses-degree, bracket-degrees,
                  //             print-style, halign, valign

                  kindText = e.attribute("text");
                  symbols = e.attribute("use-symbols");
                  parens = e.attribute("parentheses-degrees");
                  kind = e.text();
                  }
            else if (tag == "inversion") {
                  // attributes: print-style
                  }
            else if (tag == "bass") {
                  QString step;
                  int alter = 0;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        if (tag == "bass-step") {
                              // attributes: print-style
                              step = ee.text();
                              }
                        else if (tag == "bass-alter") {
                              // attributes: print-object, print-style
                              //             location (left-right)
                              alter = ee.text().toInt();
                              }
                        else
                              domError(ee);
                        }
                  ha->setBaseTpc(step2tpc(step, AccidentalVal(alter)));
                  }
            else if (tag == "degree") {
                  int degreeValue = 0;
                  int degreeAlter = 0;
                  QString degreeType = "";
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        if (tag == "degree-value") {
                              degreeValue = ee.text().toInt();
                              }
                        else if (tag == "degree-alter") {
                              degreeAlter = ee.text().toInt();
                              }
                        else if (tag == "degree-type") {
                              degreeType = ee.text();
                              }
                        else
                              domError(ee);
                        }
                  if (degreeValue <= 0 || degreeValue > 13
                      || degreeAlter < -2 || degreeAlter > 2
                      || (degreeType != "add" && degreeType != "alter" && degreeType != "subtract")) {
                        qDebug("incorrect degree: degreeValue=%d degreeAlter=%d degreeType=%s",
                               degreeValue, degreeAlter, qPrintable(degreeType));
                        }
                  else {
                        if (degreeType == "add")
                              degreeList << HDegree(degreeValue, degreeAlter, ADD);
                        else if (degreeType == "alter")
                              degreeList << HDegree(degreeValue, degreeAlter, ALTER);
                        else if (degreeType == "subtract")
                              degreeList << HDegree(degreeValue, degreeAlter, SUBTRACT);
                        }
                  }
            else if (tag == "frame") {
                  qDebug("xmlHarmony: found harmony frame");
                  FretDiagram* fd = new FretDiagram(score);
                  fd->setTrack(staff * VOICES);
                  // read frame into FretDiagram
                  readFretDiagram(fd, e);
                  Segment* s = measure->getSegment(Segment::SegChordRest, tick);
                  s->add(fd);
                  }
            else if (tag == "level")
                  domNotImplemented(e);
            else if (tag == "offset")
                  offset = calcTicks(e.text(), divisions);
            else
                  domError(e);
            }

      const ChordDescription* d = 0;
      if (ha->rootTpc() != INVALID_TPC)
            d = ha->fromXml(kind, kindText, symbols, parens, degreeList);
      if (d) {
            ha->setId(d->id);
            ha->setTextName(d->names.front());
            }
      else {
            ha->setId(-1);
            ha->setTextName(kindText);
            }
      ha->render();

      ha->setVisible(printObject == "yes");

      // TODO-LV: do this only if ha points to a valid harmony
      // harmony = ha;
      ha->setTrack(staff * VOICES);
      Segment* s = measure->getSegment(Segment::SegChordRest, tick + offset);
      s->add(ha);
      }

//---------------------------------------------------------
//   xmlClef
//---------------------------------------------------------

int MusicXml::xmlClef(QDomElement e, int staffIdx, Measure* measure)
      {
      ClefType clef   = ClefType::G;
      int res = STANDARD_STAFF_TYPE;
      int clefno = e.attribute(QString("number"), "1").toInt() - 1;
      QString c;
      int i = 0;
      int line = -1;
      for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
            if (ee.tagName() == "sign")
                  c = ee.text();
            else if (ee.tagName() == "line")
                  line = ee.text().toInt();
            else if (ee.tagName() == "clef-octave-change") {
                  i = ee.text().toInt();
                  if (i && !(c == "F" || c == "G"))
                        qDebug("clef-octave-change only implemented for F and G key");
                  }
            else
                  domError(ee);
            }

      //some software (Primus) don't include line and assume some default
      // it's permitted by MusicXML 2.0 XSD
      if (line == -1) {
            if (c == "G")
                  line = 2;
            else if (c == "F")
                  line = 4;
            else if (c == "C")
                  line = 3;
            }

      if (c == "G" && i == 0 && line == 2)
            clef = ClefType::G;
      else if (c == "G" && i == 1 && line == 2)
            clef = ClefType::G1;
      else if (c == "G" && i == 2 && line == 2)
            clef = ClefType::G2;
      else if (c == "G" && i == -1 && line == 2)
            clef = ClefType::G3;
      else if (c == "G" && i == 0 && line == 1)
            clef = ClefType::G4;
      else if (c == "F" && i == 0 && line == 3)
            clef = ClefType::F_B;
      else if (c == "F" && i == 0 && line == 4)
            clef = ClefType::F;
      else if (c == "F" && i == 1 && line == 4)
            clef = ClefType::F_8VA;
      else if (c == "F" && i == 2 && line == 4)
            clef = ClefType::F_15MA;
      else if (c == "F" && i == -1 && line == 4)
            clef = ClefType::F8;
      else if (c == "F" && i == -2 && line == 4)
            clef = ClefType::F15;
      else if (c == "F" && i == 0 && line == 5)
            clef = ClefType::F_C;
      else if (c == "C") {
            if (line == 5)
                  clef = ClefType::C5;
            else if (line == 4)
                  clef = ClefType::C4;
            else if (line == 3)
                  clef = ClefType::C3;
            else if (line == 2)
                  clef = ClefType::C2;
            else if (line == 1)
                  clef = ClefType::C1;
            }
      else if (c == "percussion") {
            clef = ClefType::PERC2;
            res = PERC_DEFAULT_STAFF_TYPE;
            }
      else if (c == "TAB") {
            clef = ClefType::TAB2;
            res = TAB_DEFAULT_STAFF_TYPE;
            }
      else
            qDebug("ImportMusicXML: unknown clef <sign=%s line=%d oct ch=%d>", qPrintable(c), line, i);
      // note: also generate symbol for tick 0
      // was not necessary before 0.9.6
      Clef* clefs = new Clef(score);
      clefs->setClefType(clef);
      clefs->setTrack((staffIdx + clefno) * VOICES);
      Segment* s = measure->getSegment(clefs, tick);
      s->add(clefs);
      return res;
      }
}

