//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: importxml.cpp 5653 2012-05-19 20:19:58Z lvinken $
//
//  Copyright (C) 2002-2015 Werner Schweer and others
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
#include "libmscore/stafftext.h"
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
#include "thirdparty/qzip/qzipreader_p.h"
#include "libmscore/stafftype.h"
#include "libmscore/stringdata.h"
#include "libmscore/drumset.h"
#include "libmscore/beam.h"
#include "libmscore/jump.h"
#include "libmscore/marker.h"
#include "importxmlfirstpass.h"
#include "libmscore/instrchange.h"

namespace Ms {

//---------------------------------------------------------
//   local defines for debug output
//---------------------------------------------------------

//#define DEBUG_VOICE_MAPPER true
//#define DEBUG_TICK true

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
 Note that n's staff and track have not been set yet
 */

static void xmlSetPitch(Note* n, char step, int alter, int octave, Ottava* (&ottavas)[MAX_NUMBER_LEVEL], int track)
      {
      //qDebug("xmlSetPitch(n=%p, st=%c, alter=%d, octave=%d)",
      //       n, step, alter, octave);

      const Staff* staff = n->score()->staff(track / VOICES);
      const Instrument* instr = staff->part()->instrument();
      const Interval intval = instr->transpose();
      //qDebug("  staff=%p instr=%p dia=%d chro=%d",
      //       staff, instr, (int) intval.diatonic, (int) intval.chromatic);

      int pitch = MusicXMLStepAltOct2Pitch(step, alter, octave);
      pitch += intval.chromatic; // assume not in concert pitch
      // ensure sane values
      pitch = limit(pitch, 0, 127);

      for(int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
            if (ottavas[i] != 0 && ottavas[i]->track() == track)
                  pitch -= ottavas[i]->pitchShift();
            }

      //                        a  b  c  d  e  f  g
      static int table1[7]  = { 5, 6, 0, 1, 2, 3, 4 };
      int istep = step - 'A';
      int tpc2 = step2tpc(table1[istep], AccidentalVal(alter));
      int tpc1 = Ms::transposeTpc(tpc2, intval, true);
      n->setPitch(pitch, tpc1, tpc2);
      //qDebug("  pitch=%d tpc1=%d tpc2=%d", n->pitch(), n->tpc1(), n->tpc2());
      }

//---------------------------------------------------------
//   calcTicks
//---------------------------------------------------------

static int calcTicks(QString text, int divisions)
      {
      if (divisions <= 0) {
            qDebug("MusicXml-Import: bad divisions value: <%d>", divisions);
            return 0;
            }
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
      if (divisions <= 0) {
            qDebug("moveTick: invalid divisions %d", divisions);
            return;
            }

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

static void determineMeasureStart(const QVector<Fraction>& ml, QVector<int>& ms)
      {
      ms.resize(ml.size());
      // first measure starts at tick = 0
      ms[0] = 0;
      // all others start at start tick previous measure plus length previous measure
      for (int i = 1; i < ml.size(); i++)
            ms[i] = ms.at(i - 1) + ml.at(i - 1).ticks();
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
      divisions(-1),    // set to impossible value to detect usage without initialization
      beamMode(Beam::Mode::NONE),
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
            MScore::lastError = QObject::tr("Internal error: Could not open resource musicxml.xsd\n");
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
            MScore::lastError = QObject::tr("Internal error: MusicXML schema is invalid\n");
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
      errorDialog.setInformativeText("Do you want to try to load this file anyway?");
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
            MScore::lastError = QObject::tr("Error reading container.xml at line %1 column %2: %3\n").arg(line).arg(column).arg(err);
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
            MScore::lastError = QObject::tr("Can't find rootfile\n%1").arg(qf->fileName());
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
            return Score::FileError::FILE_BAD_FORMAT;  // appropriate error message has been printed by initMusicXmlSchema

      // validate the data
      QXmlSchemaValidator validator(schema);
      bool valid = validator.validate(dev, QUrl::fromLocalFile(name));
      qDebug("Validation time elapsed: %d ms", t.elapsed());

      if (valid)
            qDebug("importMusicXml() file '%s' is a valid MusicXML file", qPrintable(name));
      else {
            qDebug("importMusicXml() file '%s' is not a valid MusicXML file", qPrintable(name));
            MScore::lastError = QObject::tr("File '%1' is not a valid MusicXML file").arg(name);
            if (MScore::noGui)
                  return Score::FileError::FILE_NO_ERROR;   // might as well try anyhow in converter mode
            if (musicXMLValidationErrorDialog(MScore::lastError, messageHandler.getErrors()) != QMessageBox::Yes)
                  return Score::FileError::FILE_USER_ABORT;
            }

      // return OK
      return Score::FileError::FILE_NO_ERROR;
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
            MScore::lastError = QObject::tr("Error at line %1 column %2: %3\n").arg(line).arg(column).arg(err);
            return Score::FileError::FILE_BAD_FORMAT;
            }
      docName = name; // set filename for domError
      MusicXml musicxml(&doc, pass1);
      musicxml.import(score);
      score->fixTicks();
      qDebug("Parsing time elapsed: %d ms", t.elapsed());
      return Score::FileError::FILE_NO_ERROR;
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
      if (res != Score::FileError::FILE_NO_ERROR)
            return res;

      // pass 1
      dev->seek(0);
      MxmlReaderFirstPass pass1;
      res = pass1.setContent(dev);
      if (res != Score::FileError::FILE_NO_ERROR)
            return res;
      pass1.parseFile();

      // import the file
      dev->seek(0);
      res = doImport(score, name, dev, pass1);
      qDebug("importMusicXml() return %hhd", res);
      return res;
      }


//---------------------------------------------------------
//   importMusicXml
//    return Score::FileError::FILE_* errors
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
            return Score::FileError::FILE_NOT_FOUND;
      if (!xmlFile.open(QIODevice::ReadOnly)) {
            qDebug("importMusicXml() could not open MusicXML file '%s'", qPrintable(name));
            MScore::lastError = QObject::tr("Could not open MusicXML file\n%1").arg(name);
            return Score::FileError::FILE_OPEN_ERROR;
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
            return Score::FileError::FILE_NOT_FOUND;
      if (!mxlFile.open(QIODevice::ReadOnly)) {
            qDebug("importCompressedMusicXml() could not open compressed MusicXML file '%s'", qPrintable(name));
            MScore::lastError = QObject::tr("Could not open compressed MusicXML file\n%1").arg(name);
            return Score::FileError::FILE_OPEN_ERROR;
            }

      // extract the root file
      QByteArray data;
      if (!extractRootfile(&mxlFile, data))
            return Score::FileError::FILE_BAD_FORMAT;  // appropriate error message has been printed by extractRootfile
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

      // assume no multi-measure rests, will be set to true when encountering a multi-measure rest
      // required as multi-measure rest is a meaure attribute in MusicXML instead of a style setting
      score->style()->set(StyleIdx::createMultiMeasureRests, false);

      for (QDomElement e = doc->documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "score-partwise")
                  scorePartwise(e.firstChildElement());
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   initPartState
//---------------------------------------------------------

/**
 Initialize members as required for reading the MusicXML part element.
 TODO: factor out part reading into a separate class
 */

void MusicXml::initPartState()
      {
      fractionTSig          = Fraction(0, 1);
      tick                  = 0;
      maxtick               = 0;
      prevtick              = 0;
      lastMeasureLen        = 0;
      multiMeasureRestCount = -1;
      startMultiMeasureRest = false;
      hasDrumset = false;
      tie    = 0;
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            slur[i] = SlurDesc();
      for (int i = 0; i < MAX_BRACKETS; ++i)
            bracket[i] = 0;
      for (int i = 0; i < MAX_DASHES; ++i)
            dashes[i] = 0;
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            ottavas[i] = 0;
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            hairpins[i] = 0;
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            trills[i] = 0;
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            glissandi[i][0] = glissandi[i][1] = 0;
      pedal = 0;
      pedalContinue = 0;
      harmony = 0;
      tremStart = 0;
      figBass = 0;
//      glissandoText = "";
//      glissandoColor = "";
      }

//---------------------------------------------------------
//   addText
//---------------------------------------------------------

static void addText(VBox* vbx, Score* s, QString strTxt, TextStyleType stl)
      {
      if (!strTxt.isEmpty()) {
            Text* text = new Text(s);
            text->setTextStyleType(stl);
            text->setXmlText(strTxt);
            vbx->add(text);
            }
      }

static void addText2(VBox* vbx, Score* s, QString strTxt, TextStyleType stl, Align v, double yoffs)
{
      if (!strTxt.isEmpty()) {
            Text* text = new Text(s);
            text->setTextStyleType(stl);
            text->setXmlText(strTxt);
            text->textStyle().setAlign(v);
            text->textStyle().setYoff(yoffs);
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
      //qDebug("MusicXml::doCredits()");
      const PageFormat* pf = score->pageFormat();
            /*
      qDebug("page format set (inch) w=%g h=%g tm=%g spatium=%g DPMM=%g DPI=%g",
             pf->width(), pf->height(), pf->oddTopMargin(), score->spatium(), MScore::DPMM, MScore::DPI);
       */
      // page width, height and odd top margin in tenths
      //const double pw  = pf->width() * 10 * MScore::DPI / score->spatium();
      const double ph  = pf->height() * 10 * MScore::DPI / score->spatium();
      //const double tm  = pf->oddTopMargin() * 10 * MScore::DPI / score->spatium();
      //const double tov = ph - tm;
      const int pw1 = pageWidth / 3;
      const int pw2 = pageWidth * 2 / 3;
      const int ph2 = pageHeight / 2;
      /*
      qDebug("page format set (tenths) w=%g h=%g tm=%g tov=%g", pw, ph, tm, tov);
      qDebug("page format (xml, tenths) w=%d h=%d", pageWidth, pageHeight);
      qDebug("page format pw1=%d pw2=%d ph2=%d", pw1, pw2, ph2);
       */
      // dump the credits
      /*
      for (ciCreditWords ci = credits.begin(); ci != credits.end(); ++ci) {
            CreditWords* w = *ci;
            qDebug("credit-words defx=%g defy=%g just=%s hal=%s val=%s words='%s'",
                   w->defaultX,
                   w->defaultY,
                   qPrintable(w->justify),
                   qPrintable(w->hAlign),
                   qPrintable(w->vAlign),
                   qPrintable(w->words));
            }
       */

      int nWordsHeader = 0;               // number of credit-words in the header
      int nWordsFooter = 0;               // number of credit-words in the footer
      for (ciCreditWords ci = credits.begin(); ci != credits.end(); ++ci) {
            CreditWords* w = *ci;
            double defy = w->defaultY;
                  // and count #words in header and footer
                  if (defy > ph2)
                        nWordsHeader++;
                  else
                        nWordsFooter++;
            } // end for (ciCreditWords ...

      // if there are any credit words in the header, use these
      // else use the credit words in the footer (if any)
      bool useHeader = nWordsHeader > 0;
      bool useFooter = nWordsHeader == 0 && nWordsFooter > 0;
      //qDebug("header %d footer %d useHeader %d useFooter %d",
      //       nWordsHeader, nWordsFooter, useHeader, useFooter);

      // determine credits height and create vbox to contain them
      qreal vboxHeight = 10;            // default height in spatium
      double miny = pageHeight;
      double maxy = 0;
      if (pageWidth > 1 && pageHeight > 1) {
            for (ciCreditWords ci = credits.begin(); ci != credits.end(); ++ci) {
                  CreditWords* w = *ci;
                  double defy = w->defaultY;
                  if ((useHeader && defy > ph2) || (useFooter && defy < ph2)) {
                        if (defy > maxy) maxy = defy;
                        if (defy < miny) miny = defy;
                        }
                  }
            //qDebug("miny=%g maxy=%g", miny, maxy);
            if (miny < (ph - 1) && maxy > 1) {  // if both miny and maxy set
                  double diff = maxy - miny;    // calculate height in tenths
                  if (diff > 1 && diff < ph2) { // and size is reasonable
                        vboxHeight = diff;
                        vboxHeight /= 10;       // height in spatium
                        vboxHeight += 2.5;      // guesstimated correction for last line
                        }
                  }
            }
      //qDebug("vbox height %g sp", vboxHeight);
      VBox* vbox = new VBox(score);
      vbox->setBoxHeight(Spatium(vboxHeight));

      QString remainingFooterText;
      QMap<int, CreditWords*> creditMap;  // store credit-words sorted on y pos
      bool creditWordsUsed = false;

      for (ciCreditWords ci = credits.begin(); ci != credits.end(); ++ci) {
            CreditWords* w = *ci;
            double defx = w->defaultX;
            double defy = w->defaultY;
            // handle all credit words in the box
            if ((useHeader && defy > ph2) || (useFooter && defy < ph2)) {
                  creditWordsUsed = true;
                  // composer is in the right column
                  if (pw2 < defx) {
                        // found composer
                        addText2(vbox, score, w->words,
                                 TextStyleType::COMPOSER, AlignmentFlags::RIGHT | AlignmentFlags::BOTTOM,
                                 (miny - w->defaultY) * score->spatium() / (10 * MScore::DPI));
                        }
                  // poet is in the left column
                  else if (defx < pw1) {
                        // found poet
                        addText2(vbox, score, w->words,
                                 TextStyleType::POET, AlignmentFlags::LEFT | AlignmentFlags::BOTTOM,
                                 (miny - w->defaultY) * score->spatium() / (10 * MScore::DPI));
                        }
                  // save others (in the middle column) to be handled later
                  else {
                        creditMap.insert(defy, w);
                        }
                  }
            // keep remaining footer text for possible use as copyright
            else if (useHeader && defy < ph2) {
                  //qDebug("add to copyright: '%s'", qPrintable(w->words));
                  remainingFooterText += w->words;
                  }
            } // end for (ciCreditWords ...

      /*
      QMap<int, CreditWords*>::const_iterator ci = creditMap.constBegin();
      while (ci != creditMap.constEnd()) {
            CreditWords* w = ci.value();
            qDebug("creditMap %d credit-words defx=%g defy=%g just=%s hal=%s val=%s words=%s",
                   ci.key(),
                   w->defaultX,
                   w->defaultY,
                   qPrintable(w->justify),
                   qPrintable(w->hAlign),
                   qPrintable(w->vAlign),
                   qPrintable(w->words));
            ++ci;
            }
       */

      // assign title, subtitle and copyright
      QList<int> keys = creditMap.uniqueKeys(); // note: ignoring credit-words at the same y pos

      // if any credit-words present, the highest is the title
      // note that the keys are sorted in ascending order
      // -> use the last key
      if (keys.size() >= 1) {
            CreditWords* w = creditMap.value(keys.at(keys.size() - 1));
            //qDebug("title='%s'", qPrintable(w->words));
            addText2(vbox, score, w->words,
                     TextStyleType::TITLE, AlignmentFlags::HCENTER | AlignmentFlags::TOP,
                     (maxy - w->defaultY) * score->spatium() / (10 * MScore::DPI));
            }

      // add remaining credit-words as subtitles
      for (int i = 0; i < (keys.size() - 1); i++) {
            CreditWords* w = creditMap.value(keys.at(i));
            //qDebug("subtitle='%s'", qPrintable(w->words));
            addText2(vbox, score, w->words,
                     TextStyleType::SUBTITLE, AlignmentFlags::HCENTER | AlignmentFlags::TOP,
                     (maxy - w->defaultY) * score->spatium() / (10 * MScore::DPI));
            }

      // use metadata if no workable credit-words found
      if (!creditWordsUsed) {

            QString strTitle;
            QString strSubTitle;
            QString strComposer;
            QString strPoet;
            QString strTranslator;

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

            addText(vbox, score, strTitle.toHtmlEscaped(),      TextStyleType::TITLE);
            addText(vbox, score, strSubTitle.toHtmlEscaped(),   TextStyleType::SUBTITLE);
            addText(vbox, score, strComposer.toHtmlEscaped(),   TextStyleType::COMPOSER);
            addText(vbox, score, strPoet.toHtmlEscaped(),       TextStyleType::POET);
            addText(vbox, score, strTranslator.toHtmlEscaped(), TextStyleType::TRANSLATOR);
            }

      if (vbox) {
            vbox->setTick(0);
            score->measures()->add(vbox);
            }

      // if no <rights> element was read and some text was found in the footer
      // set the rights metadata to the value found
      // TODO: remove formatting
      // note that MusicXML files can contain at least two different copyright statements:
      // - in the <rights> element (metadata)
      // - in the <credit-words> (the printed version)
      // while MuseScore supports only the first one
      if (score->metaTag("copyright") == "" && remainingFooterText != "")
            score->setMetaTag("copyright", remainingFooterText);
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
      st  = TimeSigType::NORMAL;
      bts = 0;       // the beats (max 4 separated by "+") as integer
      btp = 0;       // beat-type as integer
      // determine if timesig is valid
      if (beats == "2" && beatType == "2" && timeSymbol == "cut") {
            st = TimeSigType::ALLA_BREVE;
            bts = 2;
            btp = 2;
            return true;
            }
      else if (beats == "4" && beatType == "4" && timeSymbol == "common") {
            st = TimeSigType::FOUR_FOUR;
            bts = 4;
            btp = 4;
            return true;
            }
      else {
            if (!timeSymbol.isEmpty() && timeSymbol != "normal") {
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
//   text2syms
//---------------------------------------------------------

/**
 Convert SMuFL code points to MuseScore <sym>...</sym>
 */

static QString text2syms(const QString& t)
      {
      //QTime time;
      //time.start();

      // first create a map from symbol (Unicode) text to symId
      // note that this takes about 1 msec on a Core i5,
      // caching does not gain much

      ScoreFont* sf = ScoreFont::fallbackFont();
      QMap<QString, SymId> map;
      int maxStringSize = 0;        // maximum string size found

      for (int i = int(SymId::noSym); i < int(SymId::lastSym); ++i) {
            SymId id((SymId(i)));
            QString string(sf->toString(id));
            // insert all syms except space to prevent matching all regular spaces
            if (id != SymId::space)
                  map.insert(string, id);
            if (string.size() > maxStringSize)
                  maxStringSize = string.size();
            }
      //qDebug("text2syms map count %d maxsz %d filling time elapsed: %d ms",
      //       map.size(), maxStringSize, time.elapsed());

      // then look for matches
      QString in = t;
      QString res;

      while (in != "") {
            // try to find the largest match possible
            int maxMatch = qMin(in.size(), maxStringSize);
            QString sym;
            while (maxMatch > 0) {
                  QString toBeMatched = in.left(maxMatch);
                  if (map.contains(toBeMatched)) {
                        sym = Sym::id2name(map.value(toBeMatched));
                        break;
                        }
                  maxMatch--;
                  }
            if (maxMatch > 0) {
                  // found a match, add sym to res and remove match from string in
                  res += "<sym>";
                  res += sym;
                  res += "</sym>";
                  in.remove(0, maxMatch);
                  }
            else {
                  // not found, move one char from res to in
                  res += in.left(1);
                  in.remove(0, 1);
                  }
            }

      //qDebug("text2syms total time elapsed: %d ms, res '%s'", time.elapsed(), qPrintable(res));
      return res;
      }

//---------------------------------------------------------
//   decodeEntities
///  Allows decode &#...; into UNICODE (utf8) character.
//---------------------------------------------------------

static QString decodeEntities( const QString& src )
      {
      QString ret(src);
      QRegExp re("&#([0-9]+);");
      re.setMinimal(true);

      int pos = 0;
      while( (pos = re.indexIn(src, pos)) != -1 ) {
            ret = ret.replace(re.cap(0), QChar(re.cap(1).toInt(0,10)));
            pos += re.matchedLength();
            }
      return ret;
      }

//---------------------------------------------------------
//   nextPartOfFormattedString
//---------------------------------------------------------

/**
 Read the next part of a MusicXML formatted string and convert to MuseScore internal encoding.
 */

static QString nextPartOfFormattedString(QDomElement e)
      {
      QString txt        = e.text();
      // replace HTML entities
      txt = decodeEntities(txt);
      QString syms       = text2syms(txt);
      QString lang       = e.attribute(QString("xml:lang"), "it");
      QString fontWeight = e.attribute(QString("font-weight"));
      QString fontSize   = e.attribute(QString("font-size"));
      QString fontStyle  = e.attribute(QString("font-style"));
      QString underline  = e.attribute(QString("underline"));
      QString fontFamily = e.attribute(QString("font-family"));
      // TODO: color, enclosure, yoffset in only part of the text, ...
      QString importedtext;
      if (!fontSize.isEmpty()) {
            bool ok = true;
            float size = fontSize.toFloat(&ok);
            if (ok)
                  importedtext += QString("<font size=\"%1\"/>").arg(size);
      }
      if (!fontFamily.isEmpty() && txt == syms) {
            // add font family only if no <sym> replacement made
            importedtext += QString("<font face=\"%1\"/>").arg(fontFamily);
            }
      if (fontWeight == "bold")
            importedtext += "<b>";
      if (fontStyle == "italic")
            importedtext += "<i>";
      if (!underline.isEmpty()) {
            bool ok = true;
            int lines = underline.toInt(&ok);
            if (ok && (lines > 0))  // 1,2, or 3 underlines are imported as single underline
                  importedtext += "<u>";
            else
                  underline = "";
      }
      if (txt == syms) {
            txt.replace(QString("\r"), QString("")); // convert Windows line break \r\n -> \n
            importedtext += txt.toHtmlEscaped();
            }
      else {
            // <sym> replacement made, should be no need for line break or other conversions
            importedtext += syms;
            }
      if (underline != "")
            importedtext += "</u>";
      if (fontStyle == "italic")
            importedtext += "</i>";
      if (fontWeight == "bold")
            importedtext += "</b>";
      //qDebug("importedtext '%s'", qPrintable(importedtext));
      return importedtext;
      }

//---------------------------------------------------------
//   updateStyles
//---------------------------------------------------------

/**
 Determine if i is a style type for which the default size must be set
 */

// The MusicXML specification does not specify to which kinds of text
// the word-font setting applies. Setting all sizes to the size specified
// gives bad results, e.g. for measure numbers, so a selection is made.
// Some tweaking may still be required.

static bool mustSetSize(const int i)
      {
      return
            i == int(TextStyleType::TITLE)
            || i == int(TextStyleType::SUBTITLE)
            || i == int(TextStyleType::COMPOSER)
            || i == int(TextStyleType::POET)
            || i == int(TextStyleType::INSTRUMENT_LONG)
            || i == int(TextStyleType::INSTRUMENT_SHORT)
            || i == int(TextStyleType::INSTRUMENT_EXCERPT)
            || i == int(TextStyleType::TEMPO)
            || i == int(TextStyleType::METRONOME)
            || i == int(TextStyleType::TRANSLATOR)
            || i == int(TextStyleType::SYSTEM)
            || i == int(TextStyleType::STAFF)
            || i == int(TextStyleType::REPEAT_LEFT)
            || i == int(TextStyleType::REPEAT_RIGHT)
            || i == int(TextStyleType::TEXTLINE)
            || i == int(TextStyleType::GLISSANDO)
            || i == int(TextStyleType::INSTRUMENT_CHANGE);
      }

/**
 Update the style definitions to match the MusicXML word-font and lyric-font.
 */

static void updateStyles(Score* score,
                         const QString& wordFamily, const QString& wordSize,
                         const QString& lyricFamily, const QString& lyricSize)
      {
      const float fWordSize = wordSize.toFloat();   // note conversion error results in value 0.0
      const float fLyricSize = lyricSize.toFloat(); // but avoid comparing float with exact value later

      // loop over all text styles (except the empty, always hidden, first one)
      // set all text styles to the MusicXML defaults
      for (int i = int(TextStyleType::DEFAULT) + 1; i < int(TextStyleType::TEXT_STYLES); ++i) {
            TextStyle ts = score->style()->textStyle(TextStyleType(i));
            if (i == int(TextStyleType::LYRIC1) || i == int(TextStyleType::LYRIC2)) {
                  if (lyricFamily != "") ts.setFamily(lyricFamily);
                  if (fLyricSize > 0.001) ts.setSize(fLyricSize);
                  }
            else {
                  if (wordFamily != "") ts.setFamily(wordFamily);
                  if (fWordSize > 0.001 && mustSetSize(i)) ts.setSize(fWordSize);
                  }
            score->style()->setTextStyle(ts);
            }
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
                        Staff* staff = new Staff(score);
                        staff->setPart(part);
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
                  QString lyricFontFamily;
                  QString lyricFontSize;
                  QString wordFontFamily;
                  QString wordFontSize;
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
                                                score->style()->set(StyleIdx::minSystemDistance, val);
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
                                                score->style()->set(StyleIdx::staffDistance, val);
                                          }
                                    else
                                          domError(eee);
                                    }
                              }
                        else if (tag == "music-font")
                              domNotImplemented(ee);
                        else if (tag == "word-font") {
                              wordFontFamily = ee.attribute("font-family");
                              wordFontSize = ee.attribute("font-size");
                              }
                        else if (tag == "lyric-font") {
                              lyricFontFamily = ee.attribute("font-family");
                              lyricFontSize = ee.attribute("font-size");
                              }
                        else if (tag == "appearance")
                              domNotImplemented(ee);
                        else if (tag == "lyric-language")
                              domNotImplemented(ee);
                        else
                              domError(ee);
                        }

                  /*
                  qDebug("word font family '%s' size '%s' lyric font family '%s' size '%s'",
                         qPrintable(wordFontFamily), qPrintable(wordFontSize),
                         qPrintable(lyricFontFamily), qPrintable(lyricFontSize));
                   */
                  updateStyles(score, wordFontFamily, wordFontSize, lyricFontFamily, lyricFontSize);

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
                                    crwords += nextPartOfFormattedString(ee);
                                    }
                              else if (tag == "credit-type")
                                    domNotImplemented(ee); // TODO
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
            if (pg->type == BracketType::NO_BRACKET)
                  il.at(pg->start)->staff(0)->setBracket(0, BracketType::NO_BRACKET);
            else
                  il.at(pg->start)->staff(0)->addBracket(BracketItem(pg->type, stavesSpan));
            if (pg->barlineSpan)
                  il.at(pg->start)->staff(0)->setBarLineSpan(pg->span);
            }

      // handle the implicit brackets:
      // multi-staff parts w/o explicit brackets get a brace
      foreach(Part const* const p, il) {
            if (p->nstaves() > 1 && !partSet.contains(p)) {
                  p->staff(0)->addBracket(BracketItem(BracketType::BRACE, p->nstaves()));
                  p->staff(0)->setBarLineSpan(p->nstaves());
                  }
            }

      // having read all parts (meaning all segments have been created),
      // now attach all jumps and markers to segments
      // simply use the first SegChordRest in the measure
      for (int i = 0; i < jumpsMarkers.size(); i++) {
            Measure* meas = jumpsMarkers.at(i).meas();
            qDebug("jumpsMarkers jm %p meas %p ",
            jumpsMarkers.at(i).el(), meas);
            qDebug("attach to measure %p", meas);
            meas->add(jumpsMarkers.at(i).el());
            }
      }



//---------------------------------------------------------
//   partGroupStart
//---------------------------------------------------------

typedef std::map<int,MusicXmlPartGroup*> MusicXmlPartGroupMap;

/**
 Store part-group start with number \a n, first part \a p and symbol / \a s in the partGroups
 map \a pgs for later reference, as at this time insufficient information is available to be able
 to generate the brackets.
 */

static void partGroupStart(MusicXmlPartGroupMap& pgs, int n, int p, QString s, bool barlineSpan)
      {
      // qDebug("partGroupStart number=%d part=%d symbol=%s", n, p, s.toLatin1().data());

      if (pgs.count(n) > 0) {
            qDebug("part-group number=%d already active", n);
            return;
            }

      BracketType bracketType = BracketType::NO_BRACKET;
      if (s == "")
            ;  // ignore (handle as NO_BRACKET)
      else if (s == "none")
            ;  // already set to NO_BRACKET
      else if (s == "brace")
            bracketType = BracketType::BRACE;
      else if (s == "bracket")
            bracketType = BracketType::NORMAL;
      else if (s == "line")
            bracketType = BracketType::LINE;
      else if (s == "square")
            bracketType = BracketType::SQUARE;
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

static void partGroupStop(MusicXmlPartGroupMap& pgs, int n, int p,
                          MusicXmlPartGroupList& pgl)
      {
      if (pgs.count(n) == 0) {
            qDebug("part-group number=%d not active", n);
            return;
            }

      pgs[n]->span = p - pgs[n]->start;
      // qDebug("part-group number=%d start=%d span=%d type=%d",
      //        n, pgs[n]->start, pgs[n]->span, pgs[n]->type);
      pgl.push_back(pgs[n]);
      pgs.erase(n);
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
      MusicXmlPartGroupMap partGroups;

      for (; !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "score-part")
                  xmlScorePart(e.firstChildElement(), e.attribute(QString("id")), scoreParts);
            else if (e.tagName() == "part-group") {
                  bool barlineSpan = true;
                  int number = e.attribute(QString("number")).toInt() - 1;
                  QString symbol = "";
                  QString type = e.attribute(QString("type"));
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "group-symbol")
                              symbol = ee.text();
                        else if (ee.tagName() == "group-barline") {
                              if (ee.text() == "no")
                                    barlineSpan = false;
                              }
                        else
                              domError(ee);
                        }
                  if (type == "start")
                        partGroupStart(partGroups, number, scoreParts, symbol, barlineSpan);
                  else if (type == "stop")
                        partGroupStop(partGroups, number, scoreParts, partGroupList);
                  else
                        qDebug("Import MusicXml:xmlPartList: part-group type '%s' not supported",
                               qPrintable(type));
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

      drumsets.insert(id, MusicXMLDrumset());

      for (; !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "part-name") {
                  // Element part-name contains the displayed (full) part name
                  // It is displayed by default, but can be suppressed (print-object=”no”)
                  // As of MusicXML 3.0, formatting is deprecated, with part-name in plain text
                  // and the formatted version in the part-name-display element
                  if (!(e.attribute("print-object") == "no"))
                        part->setPlainLongName(e.text());
                  part->setPartName(e.text());
                  }
            else if (e.tagName() == "part-name-display") {
                  // TODO
                  domNotImplemented(e);
            }
            else if (e.tagName() == "part-abbreviation") {
                  // Element part-name contains the displayed (abbreviated) part name
                  // It is displayed by default, but can be suppressed (print-object=”no”)
                  // As of MusicXML 3.0, formatting is deprecated, with part-name in plain text
                  // and the formatted version in the part-abbreviation-display element
                  if (!(e.attribute("print-object") == "no"))
                        part->setPlainShortName(e.text());
                  }
            else if (e.tagName() == "part-abbreviation-display") {
                  // TODO
                  domNotImplemented(e);
            }
            else if (e.tagName() == "score-instrument") {
                  QString instrId = e.attribute("id");
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "ensemble")
                              domNotImplemented(e);
                        else if (ee.tagName() == "instrument-name") {
                              drumsets[id].insert(instrId, MusicXMLDrumInstrument(ee.text()));
                              // Element instrument-name is typically not displayed in the score,
                              // but used only internally
                              if (drumsets[id].contains(instrId))
                                    drumsets[id][instrId].name = ee.text();
                              // try to prevent an empty track name
                              if (part->partName() == "")
                                    part->setPartName(ee.text());
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
                        if (ee.tagName() == "midi-bank")
                              domNotImplemented(e);
                        else if (ee.tagName() == "midi-channel") {
                              int channel = ee.text().toInt();
                              if (channel < 1) {
                                    qDebug("MusicXml::xmlScorePart: incorrect midi-channel: %d", channel);
                                    channel = 1;
                                    }
                              else if (channel > 16) {
                                    qDebug("MusicXml::xmlScorePart: incorrect midi-channel: %d", channel);
                                    channel = 16;
                                    }
                              if (drumsets[id].contains(instrId))
                                    drumsets[id][instrId].midiChannel = channel - 1;
                              }
                        else if (ee.tagName() == "midi-program") {
                              int program = ee.text().toInt();
                              // Bug fix for Cubase 6.5.5 which generates <midi-program>0</midi-program>
                              // Check program number range
                              if (program < 1) {
                                    qDebug("MusicXml::xmlScorePart: incorrect midi-program: %d", program);
                                    program = 1;
                                    }
                              else if (program > 128) {
                                    qDebug("MusicXml::xmlScorePart: incorrect midi-program: %d", program);
                                    program = 128;
                                    }
                              if (drumsets[id].contains(instrId))
                                    drumsets[id][instrId].midiProgram = program - 1;
                              }
                        else if (ee.tagName() == "midi-unpitched") {
                              if (drumsets[id].contains(instrId))
                                    drumsets[id][instrId].pitch = ee.text().toInt() - 1;
                              }
                        else if (ee.tagName() == "volume") {
                              double vol = ee.text().toDouble();
                              if (vol >= 0 && vol <= 100) {
                                    if (drumsets[id].contains(instrId))
                                          drumsets[id][instrId].midiVolume = static_cast<int>((vol / 100) * 127);
                                    }
                              else
                                    qDebug("MusicXml::xmlScorePart: incorrect midi-volume: %g", vol);
                              }
                        else if (ee.tagName() == "pan") {
                              double pan = ee.text().toDouble();
                              if (pan >= -90 && pan <= 90) {
                                    if (drumsets[id].contains(instrId))
                                          drumsets[id][instrId].midiPan = static_cast<int>(((pan + 90) / 180) * 127);
                                    }
                              else
                                    qDebug("MusicXml::xmlScorePart: incorrect midi-volume: %g", pan);
                              }
                        else
                              domError(ee);
                        }
                  }
            else if (e.tagName() == "midi-device") {
                  // TODO
                  domNotImplemented(e);
                  }
            else
                  domError(e);
            }
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
            TDuration d(TDuration::DurationType::V_INVALID);
            if (measure->ticks() == restLen)
                  d.setType(TDuration::DurationType::V_MEASURE);
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
//   findDeleteWords
//---------------------------------------------------------

/**
 Find a non-empty staff text in \a s at \a track (which originates as MusicXML <words>).
 If found, delete it and return its text.
 */

static QString findDeleteStaffText(Segment* s, int track)
      {
      //qDebug("findDeleteWords(s %p track %d)", s, track);
      foreach (Element* e, s->annotations()) {
            //qDebug("findDeleteWords e %p type %hhd track %d", e, e->type(), e->track());
            if (e->type() != Element::Type::STAFF_TEXT || e->track() < track || e->track() >= track+VOICES)
                  continue;
            Text* t = static_cast<Text*>(e);
            //qDebug("findDeleteWords t %p text '%s'", t, qPrintable(t->text()));
            QString res = t->xmlText();
            if (res != "") {
                  s->remove(t);
                  return res;
                  }
            }
      return "";
      }

//---------------------------------------------------------
//   xmlPart
//---------------------------------------------------------

/**
 Read the MusicXML part element.
 */

void MusicXml::xmlPart(QDomElement e, QString id)
      {
      //qDebug("xmlPart(id='%s')", qPrintable(id));
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

      initPartState();

      // determine if the part contains a drumset
      // this is the case if any instrument has a midi-unpitched element,
      // (which stored in the MusicXMLDrumInstrument pitch field)
      // debug: also dump the drumset for this part

      hasDrumset = false;
      MusicXMLDrumsetIterator iii(drumsets[id]);
      while (iii.hasNext()) {
            iii.next();
            //qDebug("xmlPart: instrument: %s %s", qPrintable(iii.key()), qPrintable(iii.value().toString()));
            int pitch = iii.value().pitch;
            if (0 <= pitch && pitch <= 127) {
                  hasDrumset = true;
            }
      }
      //qDebug("xmlPart: hasDrumset %d", hasDrumset);

      KeySigEvent ev;
      KeySig currKeySig;
      currKeySig.setKeySigEvent(ev);

      // initVoiceMapperAndMapVoices(e);
      voicelist = pass1.getVoiceList(id);
#ifdef DEBUG_VOICE_MAPPER
      // debug: print voice mapper contents
      qDebug("voiceMapperStats: new staff");
      for (QMap<QString, Ms::VoiceDesc>::const_iterator i = voicelist.constBegin(); i != voicelist.constEnd(); ++i) {
            qDebug("voiceMapperStats: voice %s staff data %s",
                   qPrintable(i.key()), qPrintable(i.value().toString()));
            }
#endif

      if (!score->measures()->first()) {
            doCredits();
            }

      for (int measureNr = 0; !e.isNull(); e = e.nextSiblingElement(), measureNr++) {
            if (e.tagName() == "measure") {
                  // set the correct start tick for the measure
                  tick = measureStart.at(measureNr);
                  Measure* measure = xmlMeasure(part, e, e.attribute(QString("number")).toInt()-1, measureLength.at(measureNr), &currKeySig);
                  if (measure)
                        fillGapsInFirstVoices(measure, part);
                  }
            else
                  domError(e);
            }

      //qDebug("spanner list:");
      auto i = spanners.constBegin();
      while (i != spanners.constEnd()) {
            Spanner* sp = i.key();
            int tick1 = i.value().first;
            int tick2 = i.value().second;
            //qDebug("spanner %p tp %hhd tick1 %d tick2 %d track %d track2 %d",
            //       sp, sp->type(), tick1, tick2, sp->track(), sp->track2());
            sp->setTick(tick1);
            sp->setTick2(tick2);
            sp->score()->addElement(sp);
            ++i;
            }
      spanners.clear();

      // intialize Drumset drumset
      // debug: also dump the drumset for this part

      Drumset* drumset = new Drumset;
      drumset->clear();
      MusicXMLDrumsetIterator ii(drumsets[id]);
      while (ii.hasNext()) {
            ii.next();
            //qDebug("xmlPart: instrument: %s %s", qPrintable(ii.key()), qPrintable(ii.value().toString()));
            int pitch = ii.value().pitch;
            if (0 <= pitch && pitch <= 127) {
                  drumset->drum(ii.value().pitch)
                        = DrumInstrument(ii.value().name.toLatin1().constData(),
                                         ii.value().notehead, ii.value().line, ii.value().stemDirection);
                  }
            }

      // debug: dump the instrument map
      /*
      {
      auto il = pass1.getInstrList(id);
      for (auto it = il.cbegin(); it != il.cend(); ++it) {
            Fraction f = (*it).first;
            qDebug("xmlPart: instrument map: tick %s (%d) instr '%s'", qPrintable(f.print()), f.ticks(), qPrintable((*it).second));
            }
      }
       */

      // set the parts first instrument
      if (drumsets[id].size() > 0) {
            QString instrId = pass1.getInstrList(id).instrument(Fraction(0, 1));
            //qDebug("xmlPart: initial instrument '%s'", qPrintable(instrId));
            MusicXMLDrumInstrument instr;
            if (instrId == "")
                  instr = drumsets[id].first();
            else if (drumsets[id].contains(instrId))
                  instr = drumsets[id].value(instrId);
            else {
                  qDebug("xmlPart: initial instrument '%s' not found in part '%s'", qPrintable(instrId), qPrintable(id));
                  instr = drumsets[id].first();
                  }
            // part->setMidiChannel(instr.midiChannel); not required (is a NOP anyway)
            part->setMidiProgram(instr.midiProgram);
            part->setPan(instr.midiPan);
            part->setVolume(instr.midiVolume);
            part->instrument()->setTrackName(instr.name);
            }
      else
            qDebug("xmlPart: no instrument found for part '%s'", qPrintable(id));

      if (hasDrumset) {
            // set staff type to percussion if incorrectly imported as pitched staff
            // Note: part has been read, staff type already set based on clef type and staff-details
            // but may be incorrect for a percussion staff that does not use a percussion clef
            for (int j = 0; j < part->nstaves(); ++j)
                  if (part->staff(j)->lines() == 5 && !part->staff(j)->isDrumStaff())
                        part->staff(j)->setStaffType(StaffType::preset(StaffTypes::PERC_DEFAULT));
            // set drumset for instrument
            part->instrument()->setDrumset(drumset);
            part->instrument()->channel(0)->bank = 128;
            part->instrument()->channel(0)->updateInitList();
            }
      else {
            // drumset is not needed
            delete drumset;
            // set the instruments for this part
            MusicXmlInstrList il = pass1.getInstrList(id);
            for (auto it = il.cbegin(); it != il.cend(); ++it) {
                  Fraction f = (*it).first;
                  if (f > Fraction(0, 1)) {
                        auto instrId = (*it).second;
                        int staff = score->staffIdx(part);
                        int track = staff * VOICES;
                        //qDebug("xmlPart: instrument change: tick %s (%d) track %d instr '%s'",
                        //       qPrintable(f.print()), f.ticks(), track, qPrintable(instrId));
                        Segment* segment = score->tick2segment(f.ticks(), true, Segment::Type::ChordRest, true);
                        if (!segment)
                              qDebug("xmlPart: segment for instrument change at tick %d not found", f.ticks());
                        else if (!drumsets[id].contains(instrId))
                              qDebug("xmlPart: changed instrument '%s' at tick %d not found in part '%s'",
                                     qPrintable(instrId), f.ticks(), qPrintable(id));
                        else {
                              MusicXMLDrumInstrument mxmlInstr = drumsets[id].value(instrId);
                              Instrument instr;
                              // part->setMidiChannel(instr.midiChannel); not required (is a NOP anyway)
                              instr.channel(0)->program = mxmlInstr.midiProgram;
                              instr.channel(0)->pan = mxmlInstr.midiPan;
                              instr.channel(0)->volume = mxmlInstr.midiVolume;
                              instr.setTrackName(mxmlInstr.name);
                              InstrumentChange* ic = new InstrumentChange(instr, score);
                              ic->setTrack(track);
                              // if there is already a staff text at this tick / track,
                              // delete it and use its text here instead of "Instrument"
                              QString text = findDeleteStaffText(segment, track);
                              if (text == "")
                                    ic->setXmlText("Instrument");
                              else
                                    ic->setXmlText(text);
                              segment->add(ic);
                              }
                        }
                  }
            }

            //qDebug("xmlPart: end");
      }

//---------------------------------------------------------
//   readFiguredBassItem
//---------------------------------------------------------

static void readFiguredBassItem(FiguredBassItem* fgi, const QDomElement& de, bool paren)
      {
      // read the <figure> node de
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            int iVal = val.toInt();
            if (tag == "extend") {
                  if (e.attribute("type") == "start")
                        fgi->setContLine(FiguredBassItem::ContLine::EXTENDED);
                  else if (e.attribute("type") == "continue")
                        fgi->setContLine(FiguredBassItem::ContLine::EXTENDED);
                  else if (e.attribute("type") == "stop")
                        fgi->setContLine(FiguredBassItem::ContLine::SIMPLE);
                  }
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
            if (fgi->prefix() != FiguredBassItem::Modifier::NONE)
                  fgi->setParenth1(FiguredBassItem::Parenthesis::ROUNDOPEN);  // before prefix
            else if (fgi->digit() != FBIDigitNone)
                  fgi->setParenth2(FiguredBassItem::Parenthesis::ROUNDOPEN);  // before digit
            else if (fgi->suffix() != FiguredBassItem::Modifier::NONE)
                  fgi->setParenth3(FiguredBassItem::Parenthesis::ROUNDOPEN);  // before suffix
            // parenthesis close
            if (fgi->suffix() != FiguredBassItem::Modifier::NONE)
                  fgi->setParenth4(FiguredBassItem::Parenthesis::ROUNDCLOSED);  // after suffix
            else if (fgi->digit() != FBIDigitNone)
                  fgi->setParenth3(FiguredBassItem::Parenthesis::ROUNDCLOSED);  // after digit
            else if (fgi->prefix() != FiguredBassItem::Modifier::NONE)
                  fgi->setParenth2(FiguredBassItem::Parenthesis::ROUNDCLOSED);  // after prefix
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
//---------------------------------------------------------

static bool readFigBass(FiguredBass* fb, const QDomElement& de, int divisions)
      {
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
                  FiguredBassItem* pItem = new FiguredBassItem(fb->score(), idx++);
                  pItem->setTrack(fb->track());
                  pItem->setParent(fb);
                  readFiguredBassItem(pItem, e, parentheses);
                  fb->appendItem(pItem);
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
      fb->setXmlText(normalizedText);                  // this is the text to show while editing
      bool res = !normalizedText.isEmpty();
      return res;
      }


//---------------------------------------------------------
//   removeBeam -- // beam mode for all elements and remove the beam
//---------------------------------------------------------

static void removeBeam(Beam*& beam)
      {
      for (int i = 0; i < beam->elements().size(); ++i)
            beam->elements().at(i)->setBeamMode(Beam::Mode::NONE);
      delete beam;
      beam = 0;
      }

//---------------------------------------------------------
//   handleBeamAndStemDir
//---------------------------------------------------------

static void handleBeamAndStemDir(ChordRest* cr, const Beam::Mode bm, const MScore::Direction sd, Beam*& beam)
      {
      if (!cr) return;
      // create a new beam
      if (bm == Beam::Mode::BEGIN) {
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
                && (bm == Beam::Mode::BEGIN || bm == Beam::Mode::MID || bm == Beam::Mode::END)) {
                  // ... and actually add cr to the beam
                  beam->add(cr);
                  }
            else {
                  qDebug("handleBeamAndStemDir() from track %d to track %d bm %hhd -> abort beam",
                         beam->track(), cr->track(), bm);
                  // ... or reset beam mode for all elements and remove the beam
                  removeBeam(beam);
                  }
            }
      // if no beam, set stem direction on chord itself
      if (!beam) {
            static_cast<Chord*>(cr)->setStemDirection(sd);
            cr->setBeamMode(Beam::Mode::NONE);
            }
      // terminate the currect beam and add to the score
      if (beam && bm == Beam::Mode::END)
            beam = 0;
      }

//---------------------------------------------------------
//   xmlMeasure
//---------------------------------------------------------

/**
 Read the MusicXML measure element.
 */

Measure* MusicXml::xmlMeasure(Part* part, QDomElement e, int number, Fraction measureLen, KeySig* currKeySig)
      {
#ifdef DEBUG_TICK
      qDebug("xmlMeasure %d begin", number);
#endif
      int staves = score->nstaves();
      int staff = score->staffIdx(part);

      // collect candidates for courtesy accidentals to work out at measure end
      QList<Note*> courtAccNotes;
      QList<int> alterList;
      int alt = -10;                    // any number outside range of xml-tag "alter"
      QList<bool> accTmp;
      int i = 0;
      while(i < 74){                    // number of lines per stave rsp. length of array AccidentalState (no constant found)
          accTmp.append(false);
          i++;
      }

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
            if (mb->type() != Element::Type::MEASURE)
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
            measure->setLen(measureLen);
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

      QString cv = "1"; // current voice for chords, default is 1
      QList<Chord*> graceNotes;
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "attributes")
                  xmlAttributes(measure, staff, e.firstChildElement(), currKeySig);
            else if (e.tagName() == "note") {
                  Note* note = xmlNote(measure, staff, part->id(), beam, cv, e, graceNotes, alt);
                  if(note) {
                        if(note->accidental()){
                              if(note->accidental()->accidentalType() != Accidental::Type::NONE){
                                    courtAccNotes.append(note);
                                    alterList.append(alt);
                                    }
                              }
                        }
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
                  int blankPage = e.attribute("blank-page", "0").toInt();
                  //
                  // in MScore the break happens _after_ the marked measure:
                  //
                  MeasureBase* pm = measure->prevMeasure();  // We insert VBox only for title, no HBox for the moment
                  if (pm == 0) {
                        qDebug("ImportXml: warning: break on first measure");
                        if (blankPage == 1) { // blank title page, insert a VBOX if needed
                              pm = measure->prev();
                              if (pm == 0) {
                                    pm = score->insertMeasure(Element::Type::VBOX, measure);
                                    }
                              }
                        }
                  if (pm) {
                        if (preferences.musicxmlImportBreaks
                            && (newSystem == "yes" || newPage == "yes")) {
                              LayoutBreak* lb = new LayoutBreak(score);
                              lb->setLayoutBreakType(
                                    newSystem == "yes" ? LayoutBreak::Type::LINE : LayoutBreak::Type::PAGE
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
                  QString count;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "bar-style")
                              barStyle = ee.text();
                        else if (ee.tagName() == "ending") {
                              endingNumber = ee.attribute("number");
                              endingType   = ee.attribute("type");
                              endingText = ee.text();
                              }
                        else if (ee.tagName() == "repeat")
                        {
                              repeat = ee.attribute("direction");
                              count = ee.attribute("times");
                            if (count.isEmpty()) {
                                count = "2";
                            }
                              measure->setRepeatCount(count.toInt());
                        }
                        else
                              domError(ee);
                        }
                  if ((barStyle != "") || (repeat != "")) {
                        BarLine* barLine = new BarLine(score);
                        bool visible = true;
                        if (barStyle == "light-heavy" && repeat == "backward") {
                              barLine->setBarLineType(BarLineType::END_REPEAT);
                              }
                        else if (barStyle == "heavy-light" && repeat == "forward") {
                              barLine->setBarLineType(BarLineType::START_REPEAT);
                              }
                        else if (barStyle == "light-heavy" && repeat.isEmpty())
                              barLine->setBarLineType(BarLineType::END);
                        else if (barStyle == "regular")
                              barLine->setBarLineType(BarLineType::NORMAL);
                        else if (barStyle == "dashed")
                              barLine->setBarLineType(BarLineType::BROKEN);
                        else if (barStyle == "dotted")
                              barLine->setBarLineType(BarLineType::DOTTED);
                        else if (barStyle == "light-light")
                              barLine->setBarLineType(BarLineType::DOUBLE);
                        /*
                        else if (barStyle == "heavy-light")
                              ;
                        else if (barStyle == "heavy-heavy")
                              ;
                        */
                        else if (barStyle == "none") {
                              barLine->setBarLineType(BarLineType::NORMAL);
                              visible = false;
                              }
                        else if (barStyle == "") {
                              if (repeat == "backward")
                                    barLine->setBarLineType(BarLineType::END_REPEAT);
                              else if (repeat == "forward")
                                    barLine->setBarLineType(BarLineType::START_REPEAT);
                              else
                                    qDebug("ImportXml: warning: empty bar type");
                              }
                        else
                              qDebug("unsupported bar type <%s>", barStyle.toLatin1().data());
                        barLine->setTrack(staff * VOICES);
                        if (barLine->barLineType() == BarLineType::START_REPEAT) {
                              measure->setRepeatFlags(Repeat::START);
                              }
                        else if (barLine->barLineType() == BarLineType::END_REPEAT) {
                              measure->setRepeatFlags(Repeat::END);
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
                                                lastVolta->setVoltaType(Volta::Type::CLOSED);
                                                lastVolta->setTick2(measure->tick() + measure->ticks());
                                                lastVolta = 0;
                                                }
                                          else {
                                                qDebug("lastVolta == 0 on stop");
                                                }
                                          }
                                    else if (endingType == "discontinue") {
                                          if (lastVolta) {
                                                lastVolta->setVoltaType(Volta::Type::OPEN);
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
                  // read figured bass element to attach to next note
                  bool mustkeep = false;
                  figBass = new FiguredBass(score);
                  // mustkeep = figBass->readMusicXML(e, divisions);
                  mustkeep = readFigBass(figBass, e, divisions);
                  // qDebug("xmlMeaure: fb mustkeep %d", mustkeep);
                  if (mustkeep) {
                        figBassList.append(figBass);
                        }
                  else {
                        delete figBass;
                        figBass = 0;
                        }
                  }
            else
                  domError(e);
            }
       // grace notes appearing at the end of a measure are added as grace notes after to the last Chord
      foreach (Chord* gc, graceNotes){
           addGraceNoteAfter(gc, measure->last());
           graceNotes.clear();
           }
#ifdef DEBUG_TICK
      qDebug("end_of_measure measure->tick()=%d maxtick=%d lastMeasureLen=%d measureLen=%d tsig=%d(%s)",
             measure->tick(), maxtick, lastMeasureLen, measureLen.ticks(),
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
      lastMeasureLen = measureLen.ticks();
      tick = maxtick;
#endif

      // Check for "superfluous" accidentals to mark them as USER accidentals.
      // The candiadates list courtAccNotes is ordered voice after voice. Check it here segment after segment.
      AccidentalState currAcc;
      currAcc.init(currKeySig->keySigEvent().key());
      Segment::Type st = Segment::Type::ChordRest;
      for (Ms::Segment* segment = measure->first(st); segment; segment = segment->next(st)) {
            for (int track = 0; track < staves * VOICES; ++track) {
                   Element* e = segment->element(track);
                   if (!e || e->type() != Ms::Element::Type::CHORD)
                         continue;
                   Chord* chord = static_cast<Chord*>(e);
                   foreach (Note* nt, chord->notes()){
                         int i = courtAccNotes.indexOf(nt);
                         if(i > -1){
                               int alter = alterList.value(i);
                               int ln  = absStep(nt->tpc(), nt->pitch());
                               AccidentalVal currAccVal = currAcc.accidentalVal(ln);
                               if ((alter == -1 && currAccVal == AccidentalVal::FLAT && nt->accidental()->accidentalType() == Accidental::Type::FLAT    && !accTmp.value(ln))
                                     || (alter ==  0 && currAccVal == AccidentalVal::NATURAL && nt->accidental()->accidentalType() == Accidental::Type::NATURAL && !accTmp.value(ln))
                                     || (alter ==  1 && currAccVal == AccidentalVal::SHARP   && nt->accidental()->accidentalType() == Accidental::Type::SHARP   && !accTmp.value(ln))) {
                                     nt->accidental()->setRole(Accidental::Role::USER);
                                     }
                               else if  ((nt->accidental()->accidentalType() > Accidental::Type::NATURAL) && (nt->accidental()->accidentalType() < Accidental::Type::END)) { // microtonal accidental
                                     alter = 0;
                                     nt->accidental()->setRole(Accidental::Role::USER);
                                     accTmp.replace(ln, false);
                                     }
                               else {
                                     accTmp.replace(ln, true);
                                     }
                               }
                         }
                  }
            }

      // multi-measure rest handling:
      // if any multi-measure rest is found, the "create multi-measure rest" style setting
      // is enabled
      // the first measure in a multi-measure rest gets setBreakMultiMeasureRest(true)
      // and count down the remaining number of measures
      // the first measure after a multi-measure rest gets setBreakMultiMeasureRest(true)
      // for all other measures breakMultiMeasureRest is unchanged (stays default (false))
      if (startMultiMeasureRest) {
            score->style()->set(StyleIdx::createMultiMeasureRests, true);
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

// SLine placement is modified by changing the first segments user offset
// As the SLine has just been created, it does not have any segment yet

static void setSLinePlacement(SLine* sli, const QString placement)
      {
      /*
      qDebug("setSLinePlacement sli %p type %d s=%g pl='%s'",
             sli, sli->type(), sli->score()->spatium(), qPrintable(placement));
       */

      // calc y offset assuming five line staff and default style
      // note that required y offset is element type dependent
      const qreal stafflines = 5; // assume five line staff, but works OK-ish for other sizes too
      qreal offsAbove = 0;
      qreal offsBelow = 0;
      if (sli->type() == Element::Type::PEDAL || sli->type() == Element::Type::HAIRPIN) {
            offsAbove = -6 - (stafflines - 1);
            offsBelow = -1;
            }
      else if (sli->type() == Element::Type::TEXTLINE) {
            offsAbove = 0;
            offsBelow =  5 + 3 + (stafflines - 1);
            }
      else if (sli->type() == Element::Type::OTTAVA) {
            // ignore
            }
      else
            qDebug("setSLinePlacement sli %p unsupported type %hhd",
                   sli, sli->type());

      // move to correct position
      qreal y = 0;
      if (placement == "above") y += offsAbove;
      if (placement == "below") y += offsBelow;
      // add linesegment containing the user offset
      LineSegment* tls= sli->createLineSegment();
      //qDebug("   y = %g", y);
      y *= sli->score()->spatium();
      tls->setUserOff(QPointF(0, y));
      sli->add(tls);
      }

//---------------------------------------------------------
//   addElem
//---------------------------------------------------------

static void addElem(Element* el, int track, QString& placement, Measure* measure, int tick)
      {
      /*
      qDebug("addElem el %p track %d placement %s tick %d",
             el, track, qPrintable(placement), tick);
       */

      // calc y offset assuming five line staff and default style
      // note that required y offset is element type dependent
      const qreal stafflines = 5; // assume five line staff, but works OK-ish for other sizes too
      qreal offsAbove = 0;
      qreal offsBelow = 0;
      if (el->type() == Element::Type::TEMPO_TEXT || el->type() == Element::Type::REHEARSAL_MARK) {
            offsAbove = 0;
            offsBelow = 8 + (stafflines - 1);
            }
      else if (el->type() == Element::Type::TEXT || el->type() == Element::Type::STAFF_TEXT) {
            offsAbove = 0;
            offsBelow = 6 + (stafflines - 1);
            }
      else if (el->type() == Element::Type::SYMBOL) {
            offsAbove = -2;
            offsBelow =  4 + (stafflines - 1);
            }
      else if (el->type() == Element::Type::DYNAMIC) {
            offsAbove = -5.75 - (stafflines - 1);
            offsBelow = -0.75;
            }
      else
            qDebug("addElem el %p unsupported type %hhd",
                   el, el->type());

      // move to correct position
      // TODO: handle rx, ry
      qreal y = 0;
      if (placement == "above") y += offsAbove;
      if (placement == "below") y += offsBelow;
      //qDebug("   y = %g", y);
      y *= el->score()->spatium();
      el->setUserOff(QPoint(0, y));
      el->setTrack(track);
      Segment* s = measure->getSegment(Segment::Type::ChordRest, tick);
      s->add(el);
      }

//---------------------------------------------------------
//   metronome
//---------------------------------------------------------

/**
 Read the MusicXML metronome element, convert to text and set r to calculated tempo.
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

static QString metronome(QDomElement e, double& r)
      {
      r = 0;
      QString tempoText;
      QString perMinute;

      QString parenth = e.attribute("parentheses");
      if (parenth == "yes")
            tempoText += "(";
      TDuration dur1;
      TDuration dur2;
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString txt = e.text();
            if (e.tagName() == "beat-unit") {
                  // set first dur that is still invalid
                  if (!dur1.isValid()) dur1.setType(txt);
                  else if (!dur2.isValid()) dur2.setType(txt);
            }
            else if (e.tagName() == "beat-unit-dot") {
                  if (dur2.isValid()) dur2.setDots(1);
                  else if (dur1.isValid()) dur1.setDots(1);
            }
            else if (e.tagName() == "per-minute") {
                  perMinute = txt;
            }
            else
                  domError(e);
            } // for (e = e.firstChildElement(); ...

      if (dur1.isValid())
            tempoText += TempoText::duration2tempoTextString(dur1);
      if (dur2.isValid()) {
            tempoText += " = ";
            tempoText += TempoText::duration2tempoTextString(dur2);
            }
      else if (perMinute != "") {
            tempoText += " = ";
            tempoText += perMinute;
            }
      if (dur1.isValid() && !dur2.isValid() && perMinute != "") {
            bool ok;
            double d = perMinute.toDouble(&ok);
            if (ok) {
                  // convert fraction to beats per minute
                  r = 4 * dur1.fraction().numerator() * d / dur1.fraction().denominator();
                  }
            }
      if (parenth == "yes")
            tempoText += ")";
      return tempoText;
      }

//---------------------------------------------------------
//   checkSpannerOverlap
//---------------------------------------------------------

// check for overlapping spanners
// if necessary, delete the (incorrectly allocated) new one
// return the right one to use

static SLine* checkSpannerOverlap(SLine* cur_sp, SLine* new_sp, QString type)
      {
      //qDebug("checkSpannerOverlap(cur_sp %p, new_sp %p, type %s)", cur_sp, new_sp, qPrintable(type));
      if (cur_sp) {
            qDebug("overlapping %s not supported", qPrintable(type));
            delete new_sp;
            return cur_sp;
            }
      else
            return new_sp;
      }

//---------------------------------------------------------
//   handleSpannerStart
//---------------------------------------------------------

// note that in case of overlapping spanners, handleSpannerStart is called for every spanner
// as spanners QMap allows only one value per key, this does not hurt at all

static void handleSpannerStart(SLine* new_sp, QString /* type */, int track, QString& placement, int tick, MusicXmlSpannerMap& spanners)
      {
      new_sp->setTrack(track);
      setSLinePlacement(new_sp, placement);
      spanners[new_sp] = QPair<int, int>(tick, -1);
      //qDebug("%s %p inserted at first tick %d", qPrintable(type), new_sp, tick);
      }

//---------------------------------------------------------
//   handleSpannerStop
//---------------------------------------------------------

static void handleSpannerStop(SLine* cur_sp, QString type, int track2, int tick, MusicXmlSpannerMap& spanners)
      {
      if (!cur_sp) {
            qDebug("%s stop without start", qPrintable(type));
            return;
            }

      cur_sp->setTrack2(track2);
      spanners[cur_sp].second = tick;
      //qDebug("pedal %p second tick %d", cur_sp, tick);
      }

//---------------------------------------------------------
//   matchRepeat
//---------------------------------------------------------

/**
 Do a wild-card match with known repeat texts.
 */

static QString matchRepeat(const QString& lowerTxt)
      {
      QString repeat;
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
      return repeat;
      }

//---------------------------------------------------------
//   findJump
//---------------------------------------------------------

/**
 Try to find a Jump in \a repeat.
 */

static Jump* findJump(const QString& repeat, Score* score)
      {
      Jump* jp = 0;
      if (repeat == "daCapo") {
            jp = new Jump(score);
            jp->setTextStyleType(TextStyleType::REPEAT_RIGHT);
            jp->setJumpType(Jump::Type::DC);
            }
      else if (repeat == "daCapoAlCoda") {
            jp = new Jump(score);
            jp->setTextStyleType(TextStyleType::REPEAT_RIGHT);
            jp->setJumpType(Jump::Type::DC_AL_CODA);
            }
      else if (repeat == "daCapoAlFine") {
            jp = new Jump(score);
            jp->setTextStyleType(TextStyleType::REPEAT_RIGHT);
            jp->setJumpType(Jump::Type::DC_AL_FINE);
            }
      else if (repeat == "dalSegno") {
            jp = new Jump(score);
            jp->setTextStyleType(TextStyleType::REPEAT_RIGHT);
            jp->setJumpType(Jump::Type::DS);
            }
      else if (repeat == "dalSegnoAlCoda") {
            jp = new Jump(score);
            jp->setTextStyleType(TextStyleType::REPEAT_RIGHT);
            jp->setJumpType(Jump::Type::DS_AL_CODA);
            }
      else if (repeat == "dalSegnoAlFine") {
            jp = new Jump(score);
            jp->setTextStyleType(TextStyleType::REPEAT_RIGHT);
            jp->setJumpType(Jump::Type::DS_AL_FINE);
            }
      return jp;
      }

//---------------------------------------------------------
//   findMarker
//---------------------------------------------------------

/**
 Try to find a Marker in \a repeat.
 */

static Marker* findMarker(const QString& repeat, Score* score)
      {
      Marker* m = 0;
      if (repeat == "segno") {
            m = new Marker(score);
            // note: Marker::read() also contains code to set text style based on type
            // avoid duplicated code
            m->setTextStyleType(TextStyleType::REPEAT_LEFT);
            // apparently this MUST be after setTextStyle
            m->setMarkerType(Marker::Type::SEGNO);
            }
      else if (repeat == "coda") {
            m = new Marker(score);
            m->setTextStyleType(TextStyleType::REPEAT_LEFT);
            m->setMarkerType(Marker::Type::CODA);
            }
      else if (repeat == "fine") {
            m = new Marker(score);
            m->setTextStyleType(TextStyleType::REPEAT_RIGHT);
            m->setMarkerType(Marker::Type::FINE);
            }
      else if (repeat == "toCoda") {
            m = new Marker(score);
            m->setTextStyleType(TextStyleType::REPEAT_RIGHT);
            m->setMarkerType(Marker::Type::TOCODA);
            }
      return m;
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
      QString niente = "no";
      QString txt;
      QString formattedText;
      // int offset = 0; // not supported yet
      //int track = 0;
      int track = staff * VOICES;
      QStringList dynamics;
      // int spread;
      // qreal rx = 0.0;
      // qreal ry = 0.0;
      qreal yoffset = 0.0; // actually this is default-y
      // qreal xoffset;
      bool hasYoffset = false;
      QString dynaVelocity = "";
      QString tempo = "";
      QString sndCapo = "";
      QString sndCoda = "";
      QString sndDacapo = "";
      QString sndDalsegno = "";
      QString sndSegno = "";
      QString sndFine = "";
      bool coda = false;
      bool segno = false;
      int ottavasize = 0;
      QString pedalLine;
      QString pedalSign;
      int number = 1;
      QString lineEnd;
      // qreal endLength;
      QString lineType;
      QDomElement metrEl;
      QString enclosure = "none";

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "direction-type") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        // IMPORT_LAYOUT
                        dirType = ee.tagName();
                        if (preferences.musicxmlImportLayout) {
                              // ry      = ee.attribute(QString("relative-y"), "0").toDouble() * -.1;
                              // rx      = ee.attribute(QString("relative-x"), "0").toDouble() * .1;
                              yoffset = ee.attribute("default-y").toDouble(&hasYoffset) * -0.1;
                              // xoffset = ee.attribute("default-x", "0.0").toDouble() * 0.1;
                              }
                        if (dirType == "words") {
                              enclosure      = ee.attribute(QString("enclosure"), "none");
                              txt            = ee.text(); // support legacy code
                              formattedText += nextPartOfFormattedString(ee);
                              }
                        else if (dirType == "rehearsal") {
                              enclosure      = ee.attribute(QString("enclosure"), "square");
                              formattedText += nextPartOfFormattedString(ee);
                              }
                        else if (dirType == "pedal") {
                              type = ee.attribute(QString("type"));
                              pedalLine = ee.attribute(QString("line"));
                              pedalSign = ee.attribute(QString("sign"));
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
                              type      = ee.attribute(QString("type"));
                              number    = ee.attribute(QString("number"), "1").toInt();
                              niente    = ee.attribute(QString("niente"),"no");
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
                              number     = ee.attribute(QString("number"), "1").toInt();
                              ottavasize = ee.attribute(QString("size"), "0").toInt();
                              }
                        else if (dirType == "coda")
                              coda = true;
                        else if (dirType == "segno")
                              segno = true;
                        else if (dirType == "other-direction")
                              ; // application-specific, non-standard direction: ignore
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
                  //offset = (e.text().toInt() * MScore::division)/divisions;
                  ; // ignore, currently not supported
            else if (e.tagName() == "staff") {
                  // DEBUG: <staff>0</staff>
                  int rstaff = e.text().toInt() - 1;
                  if (rstaff < 0)         // ???
                        rstaff = 0;
                  track = (staff + rstaff) * VOICES;
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
      if (repeat == "") repeat = matchRepeat(txt.toLower());
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
            if (Jump* jp = findJump(repeat, score)) {
                  jp->setTrack(track);
                  qDebug("jumpsMarkers adding jm %p meas %p",jp, measure);
                  jumpsMarkers.append(JumpMarkerDesc(jp, measure));
                  }
            if (Marker* m = findMarker(repeat, score)) {
                  m->setTrack(track);
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

            double tpoMetro = 0;           // tempo according to metronome
            // determine tempo text and calculate bpm according to metronome
            if (metrEl.tagName() != "") formattedText += metronome(metrEl, tpoMetro);
            double tpo = tempo.toDouble(); // tempo according to sound tempo=...

            // fix for Sibelius 7.1.3 (direct export) which creates metronomes without <sound tempo="..."/>:
            // if necessary, use the value calculated by metronome()
            // note: no floating point comparisons with 0 ...
            if (tpo < 0.1 && tpoMetro > 0.1)
                  tpo = tpoMetro;

            Text* t;
            if (tpo > 0.1) {
                  tpo /= 60;
                  t = new TempoText(score);
                  ((TempoText*) t)->setTempo(tpo);
                  ((TempoText*) t)->setFollowText(true);
                  score->setTempo(tick, tpo);
                  }
            else {
                  t = new StaffText(score);
                  }

            //qDebug("formatted words '%s'", qPrintable(formattedText));
            t->setXmlText(formattedText);

            if (enclosure == "circle") {
                  t->textStyle().setHasFrame(true);
                  t->textStyle().setCircle(true);
                  }
            else if (enclosure == "rectangle") {
                  t->textStyle().setHasFrame(true);
                  t->textStyle().setFrameRound(0);
            }

            if (hasYoffset) t->textStyle().setYoff(yoffset);
            addElem(t, track, placement, measure, tick);
            }
      else if (dirType == "rehearsal") {
            Text* t = new RehearsalMark(score);
            if (!formattedText.contains("<b>"))
                  formattedText = "<b></b>" + formattedText; // explicitly turn bold off
            t->setXmlText(formattedText);
            t->textStyle().setHasFrame(enclosure != "none");
            if (hasYoffset) t->textStyle().setYoff(yoffset);
            else t->setPlacement(placement == "above" ? Element::Placement::ABOVE : Element::Placement::BELOW);
            addElem(t, track, placement, measure, tick);
            }
      else if (dirType == "pedal") {
            if (pedalLine != "yes" && pedalSign == "") pedalSign = "yes"; // MusicXML 2.0 compatibility
            if (pedalLine == "yes" && pedalSign == "") pedalSign = "no";  // MusicXML 2.0 compatibility
            if (pedalLine == "yes") {
                  if (type == "start") {
                        pedal = static_cast<Pedal*>(checkSpannerOverlap(pedal, new Pedal(score), "pedal"));
                        if (pedalSign == "yes")
                              pedal->setBeginText("<sym>keyboardPedalPed</sym>");
                        else
                              pedal->setBeginHook(true);
                        pedal->setEndHook(true);
                        if (placement == "") placement = "below";
                        handleSpannerStart(pedal, "pedal", track, placement, tick, spanners);
                        }
                  else if (type == "stop") {
                        if (pedal) {
                              handleSpannerStop(pedal, "pedal", track, tick, spanners);
                              pedal = 0;
                              }
                        }
                  else if (type == "change") {
                        // pedal change is implemented as two separate pedals
                        // first stop the first one
                        if (pedal) {
                              pedal->setEndHookType(HookType::HOOK_45);
                              handleSpannerStop(pedal, "pedal", track, tick, spanners);
                              pedalContinue = pedal; // mark for later fixup
                              pedal = 0;
                              }
                        // then start a new one
                        pedal = static_cast<Pedal*>(checkSpannerOverlap(pedal, new Pedal(score), "pedal"));
                        pedal->setBeginHook(true);
                        pedal->setBeginHookType(HookType::HOOK_45);
                        pedal->setEndHook(true);
                        if (placement == "") placement = "below";
                        handleSpannerStart(pedal, "pedal", track, placement, tick, spanners);
                        }
                  else if (type == "continue") {
                        // ignore
                        }
                  else
                        qDebug("unknown pedal type %s", qPrintable(type));
                  }
            else {
                  Symbol* s = new Symbol(score);
                  s->setAlign(AlignmentFlags::LEFT | AlignmentFlags::BASELINE);
                  s->setOffsetType(OffsetType::SPATIUM);
                  if (type == "start")
                        s->setSym(SymId::keyboardPedalPed);
                  else if (type == "stop")
                        s->setSym(SymId::keyboardPedalUp);
                  else
                        qDebug("unknown pedal type %s", qPrintable(type));
                  if (hasYoffset) s->setYoff(yoffset);
                  addElem(s, track, placement, measure, tick);
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
                  if (hasYoffset) dyn->textStyle().setYoff(yoffset);
                  addElem(dyn, track, placement, measure, tick);
                  }
            }
      else if (dirType == "wedge") {
            // qDebug("wedge type='%s' hairpin=%p", qPrintable(type), hairpin);
            int n = number - 1;
            Hairpin*& h = hairpins[n];
            if (type == "crescendo" || type == "diminuendo") {
                  h = static_cast<Hairpin*>(checkSpannerOverlap(h, new Hairpin(score), "hairpin"));
                  h->setHairpinType(type == "crescendo"
                                    ? Hairpin::Type::CRESCENDO : Hairpin::Type::DECRESCENDO);
                  if (niente == "yes")
                        h->setHairpinCircledTip(true);
                  handleSpannerStart(h, QString("wedge %1").arg(number), track, placement, tick, spanners);
                  }
            else if (type == "stop") {
                  if (h && niente == "yes")
                        h->setHairpinCircledTip(true);
                  handleSpannerStop(h, QString("wedge %1").arg(number), track, tick, spanners);
                  h = 0;
                  }
            else
                  qDebug("unknown wedge type: %s", qPrintable(type));
            }
      else if (dirType == "bracket") {
            int n = number - 1;
            TextLine*& b = bracket[n];
            if (type == "start") {
                  b = static_cast<TextLine*>(checkSpannerOverlap(b, new TextLine(score), "bracket"));

                  if (placement == "") placement = "above";  // set default

                  b->setBeginHook(lineEnd != "none");
                  if (lineEnd == "up")
                        b->setBeginHookHeight(-1 * b->beginHookHeight());

                  // hack: assume there was a words element before the bracket
                  if (!txt.isEmpty()) {
                        b->setBeginText(txt, TextStyleType::TEXTLINE);
                        }

                  if (lineType == "solid")
                        b->setLineStyle(Qt::SolidLine);
                  else if (lineType == "dashed")
                        b->setLineStyle(Qt::DashLine);
                  else if (lineType == "dotted")
                        b->setLineStyle(Qt::DotLine);
                  else
                        qDebug("unsupported line-type: %s", qPrintable(lineType));

                  handleSpannerStart(b, QString("bracket %1").arg(number), track, placement, tick, spanners);
                  //qDebug("bracket=%p inserted at first tick %d", b, tick);
                  }
            else if (type == "stop") {
                  if (b) {
                        b->setEndHook(lineEnd != "none");
                        if (lineEnd == "up")
                              b->setEndHookHeight(-1 * b->endHookHeight());
                        }
                  handleSpannerStop(b, QString("bracket %1").arg(number), track, tick, spanners);
                  //qDebug("bracket=%p second tick %d", b, tick);
                  b = 0;
                  }
            }
      else if (dirType == "dashes") {
            int n = number - 1;
            TextLine*& b = dashes[n];
            if (type == "start") {
                  b = static_cast<TextLine*>(checkSpannerOverlap(b, new TextLine(score), "dashes"));

                  if (placement == "") placement = "above";  // set default

                  // hack: assume there was a words element before the dashes
                  if (!txt.isEmpty()) {
                        b->setBeginText(txt, TextStyleType::TEXTLINE);
                        }

                  b->setBeginHook(false);
                  b->setEndHook(false);
                  b->setLineStyle(Qt::DashLine);

                  handleSpannerStart(b, QString("dashes %1").arg(number), track, placement, tick, spanners);
                  //qDebug("dashes=%p inserted at first tick %d", b, tick);
                  }
            else if (type == "stop") {
                  // TODO: MuseScore doesn't support lines which start and end on different staves
                  handleSpannerStop(b, QString("dashes %1").arg(number), track, tick, spanners);
                  //qDebug("dashes=%p second tick %d", b, tick);
                  b = 0;
                  }
            }
      else if (dirType == "octave-shift") {
            int n = number - 1;
            Ottava*& o = ottavas[n];
            if (type == "up" || type == "down") {
                  if (!(ottavasize == 8 || ottavasize == 15)) {
                        qDebug("unknown octave-shift size %d", ottavasize);
                        }
                  else {
                        o = static_cast<Ottava*>(checkSpannerOverlap(o, new Ottava(score), "octave-shift"));

                        if (placement == "") placement = "above";  // set default

                        if (type == "down" && ottavasize ==  8) o->setOttavaType(Ottava::Type::OTTAVA_8VA);
                        if (type == "down" && ottavasize == 15) o->setOttavaType(Ottava::Type::OTTAVA_15MA);
                        if (type ==   "up" && ottavasize ==  8) o->setOttavaType(Ottava::Type::OTTAVA_8VB);
                        if (type ==   "up" && ottavasize == 15) o->setOttavaType(Ottava::Type::OTTAVA_15MB);

                        handleSpannerStart(o, QString("octave-shift %1").arg(number), track, placement, tick, spanners);
                        //qDebug("ottava=%p inserted at first tick %d", o, tick);
                        }
                  }
            else if (type == "stop") {
                  handleSpannerStop(o, QString("octave-shift %1").arg(number), track, tick, spanners);
                  //qDebug("ottava=%p second tick %d", o, tick);
                  o = 0;
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
                        t->stringList() = QVector<instrString>(val).toList();
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
                              t->stringList()[line - 1].pitch = pitch;
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
            Instrument* i = score->staff(staff)->part()->instrument();
            i->setStringData(*t);
            }
      }

//---------------------------------------------------------
//   isAppr
//---------------------------------------------------------

/**
 Check if v approximately equals ref.
 Used to prevent floating point comparison for equality from failing
 */

static bool isAppr(const double v, const double ref, const double epsilon)
      {
      return v > ref - epsilon && v < ref + epsilon;
      }

//---------------------------------------------------------
//   microtonalGuess
//---------------------------------------------------------

/**
 Convert a MusicXML alter tag into a microtonal accidental in MuseScore enum Accidental::Type.
 Works only for quarter tone, half tone, three-quarters tone and whole tone accidentals.
 */

static Accidental::Type microtonalGuess(double val)
      {
      const double eps = 0.001;
      if (isAppr(val, -2, eps))
            return Accidental::Type::FLAT2;
      else if (isAppr(val, -1.5, eps))
            return Accidental::Type::MIRRORED_FLAT2;
      else if (isAppr(val, -1, eps))
            return Accidental::Type::FLAT;
      else if (isAppr(val, -0.5, eps))
            return Accidental::Type::MIRRORED_FLAT;
      else if (isAppr(val, 0, eps))
            return Accidental::Type::NATURAL;
      else if (isAppr(val, 0.5, eps))
            return Accidental::Type::SHARP_SLASH;
      else if (isAppr(val, 1, eps))
            return Accidental::Type::SHARP;
      else if (isAppr(val, 1.5, eps))
            return Accidental::Type::SHARP_SLASH4;
      else if (isAppr(val, 2, eps))
            return Accidental::Type::SHARP2;
      else
            qDebug("Guess for microtonal accidental corresponding to value %f failed.", val);

      // default
      return Accidental::Type::NONE;
      }

//---------------------------------------------------------
//   addSymToSig
//---------------------------------------------------------

/**
 Add a symbol defined as key-step \a step , -alter \a alter and -accidental \a accid to \a sig.
 */

static void addSymToSig(KeySigEvent& sig, const QString& step, const QString& alter, const QString& accid)
      {
      //qDebug("addSymToSig(step '%s' alt '%s' acc '%s')",
      //       qPrintable(step), qPrintable(alter), qPrintable(accid));

      SymId id = mxmlString2accSymId(accid);
      if (id == SymId::noSym) {
            bool ok;
            double d;
            d = alter.toDouble(&ok);
            Accidental::Type accTpAlter = ok ? microtonalGuess(d) : Accidental::Type::NONE;
            id = mxmlString2accSymId(accidentalType2MxmlString(accTpAlter));
            }

      if (step.size() == 1 && id != SymId::noSym) {
            const QString table = "FEDCBAG";
            const int line = table.indexOf(step);
            // no auto layout for custom keysig, calculate xpos
            // TODO: use symbol width ?
            const qreal spread = 1.4; // assumed glyph width in space
            const qreal x = sig.keySymbols().size() * spread;
            if (line >= 0) {
                  KeySym ks;
                  ks.sym  = id;
                  ks.spos = QPointF(x, qreal(line) * 0.5);
                  sig.keySymbols().append(ks);
                  sig.setCustom(true);
                  }
            }
      }

//---------------------------------------------------------
//   flushAlteredTone
//---------------------------------------------------------

/**
 If a valid key-step, -alter, -accidental combination has been read,
 convert it to a key symbol and add to the key.
 Clear key-step, -alter, -accidental.
 */

static void flushAlteredTone(KeySigEvent& kse, QString& step, QString& alt, QString& acc)
      {
      //qDebug("flushAlteredTone(step '%s' alt '%s' acc '%s')",
      //       qPrintable(step), qPrintable(alt), qPrintable(acc));

      if (step == "" && alt == ""  && acc == "")
            return; // nothing to do

      // step and alt are required, but also accept step and acc
      if (step != "" && (alt != ""  || acc != "")) {
            addSymToSig(kse, step, alt, acc);
            }
      else {
            qDebug("flushAlteredTone invalid combination of step '%s' alt '%s' acc '%s')",
                   qPrintable(step), qPrintable(alt), qPrintable(acc));
            }

      // clean up
      step = "";
      alt  = "";
      acc  = "";
      }

//---------------------------------------------------------
//   xmlAttributes
//---------------------------------------------------------

/**
 Read the MusicXML attributes element.
 */

// Order of attributes is divisions, key, time, staves and clef(s).
// For the first measure this means number of staves must be read first,
// as it determines how many key and time signatures must be inserted.

void MusicXml::xmlAttributes(Measure* measure, int staff, QDomElement e, KeySig* currKeySig)
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

                  // for custom keys, a single altered tone is described by
                  // key-step (required),  key-alter (required) and key-accidental (optional)
                  // none, one or more altered tone may be present
                  // a simple state machine is required to detect them
                  KeySigEvent key;
                  QString keyStep;
                  QString keyAlter;
                  QString keyAccidental;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "fifths")
                              key.setKey(Key(ee.text().toInt()));
                        else if (ee.tagName() == "mode") {
                              if (ee.text() == "none")
                                    key.setCustom(true);
                              else
                                    domNotImplemented(ee);
                              }
                        else if (ee.tagName() == "cancel")
                              domNotImplemented(ee); // TODO
                        else if (ee.tagName() == "key-step") {
                              flushAlteredTone(key, keyStep, keyAlter, keyAccidental);
                              keyStep = ee.text();
                              }
                        else if (ee.tagName() == "key-alter")
                              keyAlter = ee.text();
                        else if (ee.tagName() == "key-accidental")
                              keyAccidental = ee.text();
                        else
                              domError(ee);
                        }
                  flushAlteredTone(key, keyStep, keyAlter, keyAccidental);

                  if (number == -1) {
                        //
                        // apply key to all staves in the part
                        //
                        int staves = score->staff(staff)->part()->nstaves();
                        // apply to all staves in part
                        for (int i = 0; i < staves; ++i) {
                              Key oldkey = score->staff(staffIdx+i)->key(tick);
                              // TODO only if different custom key ?
                              if (oldkey != key.key() || key.custom()) {
                                    // new key differs from key in effect at this tick
                                    KeySig* keysig = new KeySig(score);
                                    keysig->setTrack((staffIdx + i) * VOICES);
                                    keysig->setKeySigEvent(key);
                                    keysig->setVisible(printObject == "yes");
                                    Segment* s = measure->getSegment(keysig, tick);
                                    s->add(keysig);
                                    currKeySig->setKeySigEvent(key);
                                    }
                              }
                        }
                  else {
                        //
                        // apply key to staff(staffIdx) only
                        //
                        Key oldkey = score->staff(staffIdx)->key(tick);
                        // TODO only if different custom key ?
                        if (oldkey != key.key() || key.custom()) {
                              // new key differs from key in effect at this tick
                              KeySig* keysig = new KeySig(score);
                              keysig->setTrack(staffIdx * VOICES);
                              keysig->setKeySigEvent(key);
                              keysig->setVisible(printObject == "yes");
                              Segment* s = measure->getSegment(keysig, tick);
                              s->add(keysig);
                              currKeySig->setKeySigEvent(key);
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
                  StaffTypes st = xmlClef(e, staff, measure);
                  int number = e.attribute(QString("number"), "-1").toInt();
                  int staffIdx = staff;
                  if (number != -1)
                        staffIdx += number - 1;
                  // qDebug("xmlAttributes clef score->staff(0) %p staffIdx %d score->staff(%d) %p",
                  //       score->staff(0), staffIdx, staffIdx, score->staff(staffIdx));
                  if (st == StaffTypes::TAB_DEFAULT || (hasDrumset && st == StaffTypes::PERC_DEFAULT))
                        score->staff(staffIdx)->setStaffType(StaffType::preset(st));
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
                        else if (ee.tagName() == "octave-change") {
                              interval.diatonic += i * 7;
                              interval.chromatic += i * 12;
                              }
                        else
                              domError(ee);
                        }
                  score->staff(staff)->part()->instrument()->setTranspose(interval);
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
            TimeSigType st  = TimeSigType::NORMAL;
            int bts = 0; // total beats as integer (beats may contain multiple numbers, separated by "+")
            int btp = 0; // beat-type as integer
            if (determineTimeSig(beats, beatType, timeSymbol, st, bts, btp)) {
                  fractionTSig = Fraction(bts, btp);
                  score->sigmap()->add(tick, fractionTSig);
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
//   addLyric -- add a single lyric to the score
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
                  return;
                  }
            else if (lyricNo > MAX_LYRICS) {
                  qDebug("too much lyrics (>%d)", MAX_LYRICS);
                  delete l;
                  return;
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

      QString formattedText;
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "syllabic") {
                  if (e.text() == "single")
                        l->setSyllabic(Lyrics::Syllabic::SINGLE);
                  else if (e.text() == "begin")
                        l->setSyllabic(Lyrics::Syllabic::BEGIN);
                  else if (e.text() == "end")
                        l->setSyllabic(Lyrics::Syllabic::END);
                  else if (e.text() == "middle")
                        l->setSyllabic(Lyrics::Syllabic::MIDDLE);
                  else
                        qDebug("unknown syllabic %s", qPrintable(e.text()));
                  }
            else if (e.tagName() == "text")
                  formattedText += nextPartOfFormattedString(e);
            else if (e.tagName() == "elision")
                  if (e.text().isEmpty()) {
                        formattedText += " ";
                        }
                  else {
                        formattedText += nextPartOfFormattedString(e);
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
            //qDebug("formatted lyric '%s'", qPrintable(formattedText));
            l->setXmlText(formattedText);
      }

#if 0
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
#endif

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
      if (!(int(TDuration::DurationType::V_BREVE)      == int(TDuration::DurationType::V_LONG)    + 1
            && int(TDuration::DurationType::V_WHOLE)   == int(TDuration::DurationType::V_BREVE)   + 1
            && int(TDuration::DurationType::V_HALF)    == int(TDuration::DurationType::V_WHOLE)   + 1
            && int(TDuration::DurationType::V_QUARTER) == int(TDuration::DurationType::V_HALF)    + 1
            && int(TDuration::DurationType::V_EIGHTH)  == int(TDuration::DurationType::V_QUARTER) + 1
            && int(TDuration::DurationType::V_16TH)    == int(TDuration::DurationType::V_EIGHTH)  + 1
            && int(TDuration::DurationType::V_32ND)    == int(TDuration::DurationType::V_16TH)    + 1
            && int(TDuration::DurationType::V_64TH)    == int(TDuration::DurationType::V_32ND)    + 1
            && int(TDuration::DurationType::V_128TH)   == int(TDuration::DurationType::V_64TH)    + 1
            && int(TDuration::DurationType::V_256TH)   == int(TDuration::DurationType::V_128TH)   + 1
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
      type = int(cr->durationType().type());
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
            if (de->type() == Element::Type::CHORD || de->type() == Element::Type::REST) {
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
            int matchedNormalType  = int(normalType.type());
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
                        tuplet->setBracketType(Tuplet::BracketType::SHOW_BRACKET);
                  else if (tupletBracket == "no")
                        tuplet->setBracketType(Tuplet::BracketType::SHOW_NO_BRACKET);
                  // set number, default is "actual" (=NumberType::SHOW_NUMBER)
                  if (tupletShowNumber == "both")
                        tuplet->setNumberType(Tuplet::NumberType::SHOW_RELATION);
                  else if (tupletShowNumber == "none")
                        tuplet->setNumberType(Tuplet::NumberType::NO_TEXT);
                  else
                        tuplet->setNumberType(Tuplet::NumberType::SHOW_NUMBER);
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
                  // qDebug("stop tuplet %p basetype %d", tuplet, tupletType);
                  tuplet->setBaseLen(td);
                  // TODO determine usefulness of following check
                  int totalDuration = 0;
                  foreach (DurationElement* de, tuplet->elements()) {
                        if (de->type() == Element::Type::CHORD || de->type() == Element::Type::REST) {
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
            na->setAnchor(ArticulationAnchor::TOP_STAFF);
            }
      else if (dir == "down") {
            na->setUp(false);
            na->setAnchor(ArticulationAnchor::BOTTOM_STAFF);
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
      ArticulationType articSym = ArticulationType::ARTICULATIONS; // legal but impossible ArticulationType value here indicating "not found"
      if (name == "inverted-mordent") {
            if ((attrLong == "" || attrLong == "no") && attrAppr == "" && attrDep == "") articSym = ArticulationType::Prall;
            else if (attrLong == "yes" && attrAppr == "" && attrDep == "") articSym = ArticulationType::PrallPrall;
            else if (attrLong == "yes" && attrAppr == "below" && attrDep == "") articSym = ArticulationType::UpPrall;
            else if (attrLong == "yes" && attrAppr == "above" && attrDep == "") articSym = ArticulationType::DownPrall;
            else if (attrLong == "yes" && attrAppr == "" && attrDep == "below") articSym = ArticulationType::PrallDown;
            else if (attrLong == "yes" && attrAppr == "" && attrDep == "above") articSym = ArticulationType::PrallUp;
            }
      else if (name == "mordent") {
            if ((attrLong == "" || attrLong == "no") && attrAppr == "" && attrDep == "") articSym = ArticulationType::Mordent;
            else if (attrLong == "yes" && attrAppr == "" && attrDep == "") articSym = ArticulationType::PrallMordent;
            else if (attrLong == "yes" && attrAppr == "below" && attrDep == "") articSym = ArticulationType::UpMordent;
            else if (attrLong == "yes" && attrAppr == "above" && attrDep == "") articSym = ArticulationType::DownMordent;
            }
      if (articSym != ArticulationType::ARTICULATIONS) {
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
      map["accent"]           = ArticulationType::Sforzatoaccent;
      map["staccatissimo"]    = ArticulationType::Staccatissimo;
      map["staccato"]         = ArticulationType::Staccato;
      map["tenuto"]           = ArticulationType::Tenuto;
      map["turn"]             = ArticulationType::Turn;
      map["inverted-turn"]    = ArticulationType::Reverseturn;
      map["stopped"]          = ArticulationType::Plusstop;
      map["up-bow"]           = ArticulationType::Upbow;
      map["down-bow"]         = ArticulationType::Downbow;
      map["detached-legato"]  = ArticulationType::Portato;
      map["spiccato"]         = ArticulationType::Staccatissimo;
      map["snap-pizzicato"]   = ArticulationType::Snappizzicato;
      map["schleifer"]        = ArticulationType::Schleifer;
      map["open-string"]      = ArticulationType::Ouvert;
      map["thumb-position"]   = ArticulationType::ThumbPosition;

      if (map.contains(mxmlName)) {
            addArticulationToChord(cr, map.value(mxmlName), "");
            return true;
            }
      else
            return false;
      }

//---------------------------------------------------------
//   convertNotehead
//---------------------------------------------------------

/**
 Convert a MusicXML notehead name to a MuseScore headgroup.
 */

static NoteHead::Group convertNotehead(QString mxmlName)
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
            return NoteHead::Group(map.value(mxmlName));
      else
            qDebug("unknown notehead %s", qPrintable(mxmlName));
      // default: return 0
      return NoteHead::Group::HEAD_NORMAL;
      }

//---------------------------------------------------------
//   addTextToNote
//---------------------------------------------------------

/**
 Add Text to Note.
 */

static void addTextToNote(QString txt, TextStyleType style, Score* score, Note* note)
      {
      if (!txt.isEmpty()) {
            Text* t = new Fingering(score);
            t->setTextStyleType(style);
            t->setXmlText(txt);
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
            addFermata(cr, fermataType, ArticulationType::Fermata);
      else if (fermata == "angled")
            addFermata(cr, fermataType, ArticulationType::Shortfermata);
      else if (fermata == "square")
            addFermata(cr, fermataType, ArticulationType::Longfermata);
      else
            qDebug("unknown fermata '%s'", qPrintable(fermata));
      }

//---------------------------------------------------------
//   xmlNotations
//---------------------------------------------------------

/**
 Read MusicXML notations.
 */

void MusicXml::xmlNotations(Note* note, ChordRest* cr, int trk, int tick, int ticks, QDomElement e)
      {
      Measure* measure = cr->measure();
      int track = cr->track();

      QString wavyLineType;
      int wavyLineNo = 0;
      QString arpeggioType;
//      QString glissandoType;
      int breath = -1;
      int tremolo = 0;
      QString tremoloType;
      QString placement;
      QStringList dynamics;
      // qreal rx = 0.0;
      // qreal ry = 0.0;
      qreal yoffset = 0.0; // actually this is default-y
      // qreal xoffset = 0.0; // not used
      bool hasYoffset = false;
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
                  // Similar issues happen with Sibelius 7.1.3 (direct export)

                  if (slurType == "start") {
                        if (slur[slurNo].isStart())
                              // slur start when slur already started: report error
                              qDebug("ignoring duplicate slur start at line %d", e.lineNumber());
                        else if (slur[slurNo].isStop()) {
                              // slur start when slur already stopped: wrap up
                              Slur* newSlur = slur[slurNo].slur();
                              newSlur->setTick(tick);
                              newSlur->setStartElement(cr);
                              slur[slurNo] = SlurDesc();
                              }
                        else {
                              // slur start for new slur: init
                              Slur* newSlur = new Slur(score);
                              if(cr->isGrace())
                                    newSlur->setAnchor(Spanner::Anchor::CHORD);
                              if (lineType == "dotted")
                                    newSlur->setLineType(1);
                              else if (lineType == "dashed")
                                    newSlur->setLineType(2);
                              newSlur->setTick(tick);
                              newSlur->setStartElement(cr);
                              QString pl = ee.attribute(QString("placement"));
                              if (pl == "above")
                                    newSlur->setSlurDirection(MScore::Direction::UP);
                              else if (pl == "below")
                                    newSlur->setSlurDirection(MScore::Direction::DOWN);
                              newSlur->setTrack(track);
                              newSlur->setTrack2(track);
                              slur[slurNo].start(newSlur);
                              score->addElement(newSlur);
                              }
                        }
                  else if (slurType == "stop") {
                        if (slur[slurNo].isStart()) {
                              // slur stop when slur already started: wrap up
                              Slur* newSlur = slur[slurNo].slur();
                              if(cr->isGrace()){
                                    newSlur->setAnchor(Spanner::Anchor::CHORD);
                                    newSlur->setEndElement(newSlur->startElement());
                                    newSlur->setStartElement(cr);
                                    }
                              else {
                                    newSlur->setTick2(tick);
                                    newSlur->setTrack2(track);
                                    newSlur->setEndElement(cr);
                                    }
                              slur[slurNo] = SlurDesc();
                              }
                        else if (slur[slurNo].isStop())
                              // slur stop when slur already stopped: report error
                              qDebug("ignoring duplicate slur stop at line %d", e.lineNumber());
                        else {
                              // slur stop for new slur: init
                              Slur* newSlur = new Slur(score);
                              newSlur->setTick2(tick);
                              newSlur->setTrack2(track);
                              newSlur->setEndElement(cr);
                              slur[slurNo].stop(newSlur);
                              }
                        }
                  else if (slurType == "continue")
                        ; // ignore
                  else
                        qDebug("unknown slur type %s", qPrintable(slurType));
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
                              QString tiedOrientation = ee.attribute("orientation", "auto");
                              if (tiedOrientation == "over")
                                    tie->setSlurDirection(MScore::Direction::UP);
                              else if (tiedOrientation == "under")
                                    tie->setSlurDirection(MScore::Direction::DOWN);
                              else if (tiedOrientation == "auto")
                                    ;  // ignore
                              else
                                    qDebug("unknown tied orientation: %s", tiedOrientation.toLatin1().data());

                              QString lineType  = ee.attribute(QString("line-type"), "solid");
                              if (lineType == "dotted")
                                    tie->setLineType(1);
                              else if (lineType == "dashed")
                                    tie->setLineType(2);
                              tie = 0;
                              }
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
                        // ry        = ee.attribute(QString("relative-y"), "0").toDouble() * -.1;
                        // rx        = ee.attribute(QString("relative-x"), "0").toDouble() * .1;
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
                                    addArticulationToChord(cr, ArticulationType::Marcato, "up");
                              else if (strongAccentType == "down")
                                    addArticulationToChord(cr, ArticulationType::Marcato, "down");
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
                        else if (eee.tagName() == "wavy-line") {
                              wavyLineType = eee.attribute(QString("type"));
                              wavyLineNo   = eee.attribute(QString("number"), "1").toInt() - 1;
                              }
                        else if (eee.tagName() == "tremolo") {
                              tremolo = eee.text().toInt();
                              tremoloType = eee.attribute(QString("type"));
                              }
                        else if (eee.tagName() == "accidental-mark")
                              domNotImplemented(eee);
                        else if (eee.tagName() == "delayed-turn")
                              // TODO: actually this should be offset a bit to the right
                              addArticulationToChord(cr, ArticulationType::Turn, "");
                        else if (eee.tagName() == "inverted-mordent"
                                 || eee.tagName() == "mordent")
                              addMordentToChord(cr, eee.tagName(), eee.attribute("long"), eee.attribute("approach"), eee.attribute("departure"));
                        else
                              domError(eee);
                        }
                  // note that mscore wavy line already implicitly includes a trillsym
                  // so don't add an additional one
                  if (trillMark && wavyLineType != "start")
                        addArticulationToChord(cr, ArticulationType::Trill, "");
                  }
            else if (ee.tagName() == "technical") {
                  for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                        if (readArticulations(cr, eee.tagName()))
                              continue;
                        else if (eee.tagName() == "fingering")
                              // TODO: distinguish between keyboards (style TextStyleType::FINGERING)
                              // and (plucked) strings (style TextStyleType::LH_GUITAR_FINGERING)
                              addTextToNote(eee.text(), TextStyleType::FINGERING, score, note);
                        else if (eee.tagName() == "fret") {
                              if (note->staff()->isTabStaff())
                                    note->setFret(eee.text().toInt());
                              }
                        else if (eee.tagName() == "pluck")
                              addTextToNote(eee.text(), TextStyleType::RH_GUITAR_FINGERING, score, note);
                        else if (eee.tagName() == "string") {
                              if (note->staff()->isTabStaff())
                                    note->setString(eee.text().toInt() - 1);
                              else
                                    addTextToNote(eee.text(), TextStyleType::STRING_NUMBER, score, note);
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

            if (ee.tagName() == "glissando" || ee.tagName() == "slide") {
                  int n                   = ee.attribute(QString("number"), "1").toInt() - 1;
                  QString spannerType     = ee.attribute(QString("type"));
                  int tag                 = ee.tagName() == "slide" ? 0 : 1;
//                  QString lineType  = ee.attribute(QString("line-type"), "solid");
                  Glissando*& gliss = glissandi[n][tag];
                  if (spannerType == "start") {
                        if (gliss) {
                              qDebug("overlapping glissando/slide %d not supported", n+1);
                              delete gliss;
                              gliss = 0;
                              }
                        else {
                              gliss = new Glissando(score);
                              gliss->setAnchor(Spanner::Anchor::NOTE);
                              gliss->setStartElement(note);
                              gliss->setTick(tick);
                              gliss->setTrack(track);
                              gliss->setParent(note);
                              gliss->setText(ee.text());
                              QColor color(ee.attribute("color"));
                              if (color.isValid())
                                    gliss->setColor(color);
                              gliss->setGlissandoType(tag == 0 ? Glissando::Type::STRAIGHT : Glissando::Type::WAVY);
                              spanners[gliss] = QPair<int, int>(tick, -1);
                              // qDebug("glissando/slide=%p inserted at first tick %d", gliss, tick);
                              }
                        }
                  else if (spannerType == "stop") {
                        if (!gliss) {
                              qDebug("glissando/slide %d stop without start", n+1);
                              }
                        else {
                              spanners[gliss].second = tick + ticks;
                              gliss->setEndElement(note);
                              gliss->setTick2(tick);
                              gliss->setTrack2(track);
                              // qDebug("glissando/slide=%p second tick %d", gliss, tick);
                              gliss = 0;
                              }
                        }
                  else
                        qDebug("unknown glissando/slide type %s", qPrintable(spannerType));
                  }
/*
            // glissando and slide are added to the "stop" chord only
            // but text and color are read at start
            else if (ee.tagName() == "glissando") {
                  if (ee.attribute("type") == "start") {
                        glissandoText = ee.text();
                        glissandoColor = ee.attribute("color");
                        }
                  else if (ee.attribute("type") == "stop") glissandoType = "glissando";
                  }
            else if (ee.tagName() == "slide") {
                  if (ee.attribute("type") == "start") {
                        glissandoText = ee.text();
                        glissandoColor = ee.attribute("color");
                        }
                  else if (ee.attribute("type") == "stop") glissandoType = "slide";
                  }
            else
                  domError(ee); */
            }
      // no support for arpeggio on rest
      if (!arpeggioType.isEmpty() && cr->type() == Element::Type::CHORD) {
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
/*
      if (!glissandoType.isEmpty()) {
            Glissando* g = new Glissando(score);
            if (glissandoType == "slide")
                  g->setGlissandoType(Glissando::Type::STRAIGHT);
            else if (glissandoType == "glissando")
                  g->setGlissandoType(Glissando::Type::WAVY);
            else {
                  qDebug("unknown glissando type %s", glissandoType.toLatin1().data());
                  delete g;
                  g = 0;
                  }
            if (g) {
                  if (glissandoText == "")
                        g->setShowText(false);
                  else {
                        g->setShowText(true);
                        g->setText(glissandoText);
                        glissandoText = "";
                        }
                  if (glissandoColor != "") {
                        QColor color(glissandoColor);
                        if (color.isValid())
                              g->setColor(color);
                        glissandoColor = "";
                        }
                  }
            if ((static_cast<Chord*>(cr))->glissando()) {
                  // there can be only one
                  delete g;
                  g = 0;
                  }
            else
                  cr->add(g);
            } */

      if (!wavyLineType.isEmpty()) {
            int n = wavyLineNo - 1;
            Trill*& t = trills[n];
            if (wavyLineType == "start") {
                  if (t) {
                        qDebug("overlapping wavy-line %d not supported", wavyLineNo);
                        delete t;
                        t = 0;
                        }
                  else {
                        t = new Trill(score);
                        t->setTrack(trk);
                        spanners[t] = QPair<int, int>(tick, -1);
                        // qDebug("wedge trill=%p inserted at first tick %d", trill, tick);
                        }
                  }
            else if (wavyLineType == "stop") {
                  if (!t) {
                        qDebug("wavy-line %d stop without start", wavyLineNo);
                        }
                  else {
                        spanners[t].second = tick + ticks;
                        // qDebug("wedge trill=%p second tick %d", trill, tick);
                        t = 0;
                        }
                  }
            else
                  qDebug("unknown wavy-line type %s", qPrintable(wavyLineType));
            }

      if (breath >= 0) {
            Breath* b = new Breath(score);
            // b->setTrack(trk + voice); TODO check next line
            b->setTrack(track);
            b->setBreathType(breath);
            Segment* seg = measure->getSegment(Segment::Type::Breath, tick + ticks);
            seg->add(b);
            }

      if (tremolo) {
            if (tremolo == 1 || tremolo == 2 || tremolo == 3 || tremolo == 4) {
                  if (tremoloType == "" || tremoloType == "single") {
                        Tremolo* t = new Tremolo(score);
                        switch (tremolo) {
                              case 1: t->setTremoloType(TremoloType::R8); break;
                              case 2: t->setTremoloType(TremoloType::R16); break;
                              case 3: t->setTremoloType(TremoloType::R32); break;
                              case 4: t->setTremoloType(TremoloType::R64); break;
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
                                    case 1: t->setTremoloType(TremoloType::C8); break;
                                    case 2: t->setTremoloType(TremoloType::C16); break;
                                    case 3: t->setTremoloType(TremoloType::C32); break;
                                    case 4: t->setTremoloType(TremoloType::C64); break;
                                    }
                              t->setChords(tremStart, static_cast<Chord*>(cr));
                              // fixup chord duration and type
                              t->chord1()->setDurationType(ticks);
                              t->chord1()->setDuration(Fraction::fromTicks(ticks));
                              t->chord2()->setDurationType(ticks);
                              t->chord2()->setDuration(Fraction::fromTicks(ticks));
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
                  cl->setChordLineType(ChordLineType::FALL);
            if (chordLineType == "doit")
                  cl->setChordLineType(ChordLineType::DOIT);
            if (chordLineType == "plop")
                  cl->setChordLineType(ChordLineType::PLOP);
            if (chordLineType == "scoop")
                  cl->setChordLineType(ChordLineType::SCOOP);
            note->chord()->add(cl);
            }

      // more than one dynamic ???
      // LVIFIX: check import/export of <other-dynamics>unknown_text</...>
      // TODO remove duplicate code (see MusicXml::direction)
      for (QStringList::Iterator it = dynamics.begin(); it != dynamics.end(); ++it ) {
            Dynamic* dyn = new Dynamic(score);
            dyn->setDynamicType(*it);
            if (hasYoffset) dyn->textStyle().setYoff(yoffset);
            addElem(dyn, track, placement, measure, tick);
            }

      }


#if 0 /* currently unused */
//---------------------------------------------------------
//   findLastFiguredBass
//---------------------------------------------------------

/**
 * Find last figured bass on \a track before \a seg
 */

static FiguredBass* findLastFiguredBass(int track, Segment* seg)
      {
      // qDebug("findLastFiguredBass(track %d seg %p)", track, seg);
      while ((seg = seg->prev1(Segment::Type::ChordRest))) {
            // qDebug("findLastFiguredBass seg %p", seg);
            foreach(Element* e, seg->annotations()) {
                  if (e->track() == track && e->type() == Element::Type::FIGURED_BASS) {
                        FiguredBass* fb = static_cast<FiguredBass*>(e);
                        // qDebug("findLastFiguredBass found fb %p at seg %p", fb, seg);
                        return fb;
                        }
                  }
            }
      return 0;
      }
#endif

//---------------------------------------------------------
//   graceNoteType
//---------------------------------------------------------

/**
 * convert duration and slash to grace note type
 */

NoteType graceNoteType(TDuration duration, QString graceSlash)
      {
            NoteType nt = NoteType::APPOGGIATURA;
            if (graceSlash == "yes")
                  nt = NoteType::ACCIACCATURA;
            if (duration.type() == TDuration::DurationType::V_QUARTER) {
                  nt = NoteType::GRACE4;
            }
            else if (duration.type() == TDuration::DurationType::V_16TH) {
                  nt = NoteType::GRACE16;
            }
            else if (duration.type() == TDuration::DurationType::V_32ND) {
                  nt = NoteType::GRACE32;
            }
            return nt;
      }

//---------------------------------------------------------
//   handleDisplayStep
//---------------------------------------------------------

/**
 * convert display-step and display-octave to staff line
 */

static void handleDisplayStep(ChordRest* cr, QString step, int octave, int tick, qreal spatium)
      {
            if (step != "" && 0 <= octave && octave <= 9) {
                  // qDebug("rest step=%s oct=%d", qPrintable(step), octave);
                  ClefType clef = cr->staff()->clef(tick);
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
                        cr->setUserYoffset((po - dp + 3) * spatium / 2);
                  }
            }
      }

//---------------------------------------------------------
//   setDuration
//---------------------------------------------------------

/**
 * Set \a cr duration
 */

static void setDuration(ChordRest* cr, bool rest, bool wholeMeasure, TDuration duration, int ticks)
      {
      if (rest) {
            // By convention, whole measure rests do not have a "type" element
            // As of MusicXML 3.0, this can be indicated by an attribute "measure",
            // but for backwards compatibility the "old" convention still has to be supported.
            if (duration.type() == TDuration::DurationType::V_INVALID) {
                  if (wholeMeasure)
                        duration.setType(TDuration::DurationType::V_MEASURE);
                  else
                        duration.setVal(ticks);
                  cr->setDurationType(duration);
                  cr->setDuration(Fraction::fromTicks(ticks));
            }
            else {
                  cr->setDurationType(duration);
                  cr->setDuration(cr->durationType().fraction());
            }
      }
      else {
            if (duration.type() == TDuration::DurationType::V_INVALID)
                  duration.setType(TDuration::DurationType::V_QUARTER);
            cr->setDurationType(duration);
            cr->setDuration(cr->durationType().fraction());
            }
      }

//---------------------------------------------------------
//   xmlNote
//---------------------------------------------------------

/**
 Read a MusicXML note.

 \a Staff is the number of first staff of the part this note belongs to.
 */
Note* MusicXml::xmlNote(Measure* measure, int staff, const QString& partId, Beam*& beam,
                        QString& currentVoice, QDomElement e, QList<Chord*>& graceNotes, int& alt)
      {
#ifdef DEBUG_TICK
      qDebug("xmlNote start tick=%d (%d div) divisions=%d", tick, tick * divisions / MScore::division, divisions);
#endif
      if (divisions <= 0) {
            qDebug("xmlNote: invalid divisions %d", divisions);
            return 0;
            }

      int ticks = 0;
      QDomNode pn = e; // TODO remove pn
      QDomElement org_e = e; // save e for later
      QString strVoice = "1";
      int voice = 0;
      int move  = 0;

      bool rest    = false;
      int relStaff = 0;
      Beam::Mode bm  = Beam::Mode::NONE;
      MScore::Direction sd = MScore::Direction::AUTO;
      bool grace   = false;
      QString graceSlash;
      QString step;
      int alter  = 0;
      int octave = 4;
      Accidental::Type accidental = Accidental::Type::NONE;
      bool parentheses = false;
      bool editorial = false;
      bool cautionary = false;
      TDuration duration(TDuration::DurationType::V_INVALID);
      NoteHead::Group headGroup = NoteHead::Group::HEAD_NORMAL;
      bool noStem = false;
      QColor noteheadColor = QColor::Invalid;
      bool noteheadParentheses = false;
      bool chord = false;
      int velocity = -1;
      bool unpitched = false;
      bool small = false;
      bool cue = false;
      QString instrId;
      QList<QDomElement> notations;

      // first read all elements required for voice mapping
      QDomElement e2 = e.firstChildElement();
      for (; !e2.isNull(); e2 = e2.nextSiblingElement()) {
            QString tag(e2.tagName());
            QString s(e2.text());
            if (tag == "voice")
                  strVoice = s;
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

      // Bug fix for Cubase 6.5.5 which generates <staff>2</staff> in a single staff part
      // Same fix is required in MxmlReaderFirstPass::initVoiceMapperAndMapVoices
      int nStavesInPart = score->staff(staff)->part()->nstaves();
      if (relStaff < 0 || relStaff >= nStavesInPart) {
            qDebug("ImportMusicXml: invalid staff %d (staves is %d) at line %d col %d",
                   relStaff + 1, nStavesInPart, e.lineNumber(), e.columnNumber());
            relStaff = 0;
            }

      // Bug fix for Sibelius 7.1.3 which does not write <voice> for notes with <chord>
      if (!chord)
            // remember voice
            currentVoice = strVoice;
      else
            // use voice from last note w/o <chord>
            strVoice = currentVoice;

      // Musicxml voices are counted for all staffs of an
      // instrument. They are not limited. In mscore voices are associated
      // with a staff. Every staff can have at most VOICES voices.

      // The following lines map musicXml voices to mscore voices.
      // If a voice crosses two staffs, this is expressed with the
      // "move" parameter in mscore.

      // Musicxml voices are unique within a part, but not across parts.

      // qDebug("voice mapper before: relStaff=%d voice=%d staff=%d", relStaff, voice, staff);
      int s; // staff mapped by voice mapper
      int v; // voice mapped by voice mapper
      if (voicelist.value(strVoice).overlaps()) {
            // for overlapping voices, the staff does not change
            // and the voice is mapped and staff-dependent
            s = relStaff;
            v = voicelist.value(strVoice).voice(s);
            }
      else {
            // for non-overlapping voices, both staff and voice are
            // set by the voice mapper
            s = voicelist.value(strVoice).staff();
            v = voicelist.value(strVoice).voice();
            }

      // qDebug("voice mapper before: relStaff=%d voice=%d s=%d v=%d", relStaff, voice, s, v);
      if (s < 0 || v < 0) {
            qDebug("ImportMusicXml: too many voices (staff %d, relStaff %d, voice %s at line %d col %d)",
                   staff + 1, relStaff, qPrintable(strVoice), e.lineNumber(), e.columnNumber());
            return 0;
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
                              QString altertext = ee.text();
                              alter = MxmlSupport::stringToInt(altertext, &ok); // fractions not supported by mscore
                              if (!ok || alter < -2 || alter > 2) {
                                    qDebug("ImportXml: bad 'alter' value: %s at line %d col %d",
                                           qPrintable(altertext), ee.lineNumber(), ee.columnNumber());
                                    bool ok2;
                                    double altervalue = altertext.toDouble(&ok2);
                                    if (ok2 && (qAbs(altervalue) < 2.0) && (accidental == Accidental::Type::NONE)) {
                                          // try to see if a microtonal accidental is needed
                                          accidental = microtonalGuess(altervalue);
                                          }
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
            else if (tag == "type") {
                  duration = TDuration(s);
                  small = e.attribute(QString("size")) == "cue";
                  }

            else if (tag == "chord" || tag == "duration" || tag == "staff" || tag == "voice")
                  // already handled by voice mapper, ignore here but prevent
                  // spurious "Unknown Node <staff>" or "... <voice>" messages
                  ;
            else if (tag == "stem") {
                  if (s == "up")
                        sd = MScore::Direction::UP;
                  else if (s == "down")
                        sd = MScore::Direction::DOWN;
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
                              bm = Beam::Mode::BEGIN;
                        else if (s == "end")
                              bm = Beam::Mode::END;
                        else if (s == "continue")
                              bm = Beam::Mode::MID;
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
                  duration.setDots(duration.dots() + 1);
            else if (tag == "accidental") {
                  if (s != "")
                        accidental = mxmlString2accidentalType(s);
                  else
                        qDebug("empty accidental");
                  if (e.attribute(QString("cautionary")) == "yes")
                        cautionary = true;
                  if (e.attribute(QString("editorial")) == "yes")
                        editorial = true;
                  if (e.attribute(QString("parentheses")) == "yes")
                        parentheses = true;
                  }
            else if (tag == "notations") {
                  // save the QDomElement representing <notations> for later
                  // note that multiple notations elements may be present
                  notations << e;
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
                  if (e.attribute(QString("parentheses")) == "yes")
                        noteheadParentheses = true;
                  }
            else if (tag == "instrument") {
                  instrId = e.attribute("id");
                  }
            else if (tag == "cue")
                  cue = true;
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

            // Verify the rest fits exactly in the measure, as some programs
            // (e.g. Cakewalk SONAR X2 Studio [Version: 19.0.0.306]) leave out
            // the type for all rests.
            bool wholeMeasure = (tick == measure->tick() && ticks == measure->ticks());
            setDuration(cr, rest, wholeMeasure, duration, ticks);

            if (beam) {
                  if (beam->track() == track) {
                        cr->setBeamMode(Beam::Mode::MID);
                        beam->add(cr);
                        }
                  else
                        removeBeam(beam);
                  }
            else
                  cr->setBeamMode(Beam::Mode::NONE);
            cr->setTrack(track);
            cr->setStaffMove(move);
            Segment* s = measure->getSegment(cr, loc_tick);
            // Sibelius might export two rests at the same place, ignore the 2nd one
            // <?DoletSibelius Two NoteRests in same voice at same position may be an error?>
            if (!s->element(cr->track()))
                  s->add(cr);
            cr->setVisible(printObject == "yes");
            cr->setSmall(small);
            handleDisplayStep(cr, step, octave, loc_tick, score->spatium());
            }
      else {
            if (grace) {
                  if (chord) {
                        // use last grace chord to add note to
                        if (!graceNotes.isEmpty())
                              cr = graceNotes.last();
                        }
                  else {
                        // create a new grace chord
                        Chord* ch = new Chord(score);
                        ch->setNoteType(graceNoteType(duration, graceSlash));
                        graceNotes.push_back(ch);
                        cr = ch;
                        //cr->setBeamMode(bm);
                        cr->setTrack(track);
                        setDuration(cr, false, false, duration, ticks);
                        ch->setNoteType(graceNoteType(duration, graceSlash));
                        // check whether grace is slured with prev. main note, then handle all grace
                        // notes until this as after
                        bool found = false;
                        if(!notations.empty()) {
                              foreach(QDomElement de, notations) {
                                    for (QDomElement ee = de.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                                          if (ee.tagName() == "slur") {
                                                int slurNo   = ee.attribute(QString("number"), "1").toInt() - 1;
                                                QString slurType = ee.attribute(QString("type"));
                                                if (slurType == "stop") {
                                                     QDomElement eLastNote = org_e.previousSiblingElement("note");
                                                      while(!eLastNote.isNull()){
                                                            if (eLastNote.elementsByTagName("grace").isEmpty()){
                                                                  QList<QDomElement> eSlurs = findSlurElements(eLastNote);
                                                                  foreach (QDomElement es, eSlurs){
                                                                       if (!es.isNull() && slurNo == es.attribute(QString("number"), "1").toInt() - 1 && es.attribute(QString("type")) == "start"){
                                                                             foreach (Chord* cg, graceNotes)
                                                                                   cg->toGraceAfter();
                                                                             found = true;
                                                                             break;
                                                                             }
                                                                       }
                                                                  if (found)
                                                                        break;
                                                                  }
                                                            eLastNote = eLastNote.previousSiblingElement("note");
                                                            }
                                                      break;
                                                      }
                                                }
                                          }
                                    if (found)
                                          break;
                                    }
                             }
                       }
                 }
            else {
                  // regular note
                  // if there is already a chord just add to it
                  // else create a new one
                  // this basically ignores <chord/> errors
                  cr = measure->findChord(loc_tick, track);
                  if (cr == 0) {
                        cr = new Chord(score);
                        cr->setBeamMode(bm);
                        cr->setTrack(track);

                        setDuration(cr, false, false, duration, ticks);
                        Segment* s = measure->getSegment(cr, loc_tick);
                        s->add(cr);
                        }
                  // append grace notes
                  // first excerpt grace notes after
                  QList<Chord*> toRemove;
                  if(graceNotes.length()){
                        for(int i =  0; i < graceNotes.length(); i++){
                              // grace notes from voice before, upcoming here must be grace after
                              if(graceNotes[i]->voice() != cr->voice()){
                                    addGraceNoteAfter(graceNotes[i], measure->last());
                                    toRemove.append(graceNotes[i]);
                                    }
                              else if(graceNotes[i]->isGraceAfter()){
                                    addGraceNoteAfter(graceNotes[i],  cr->segment()->prev());
                                    toRemove.append(graceNotes[i]);
                                    }
                              }
                        }
                  foreach(Chord* cRem, toRemove)
                        graceNotes.removeOne(cRem);
                  toRemove.clear();
                  // append grace notes before
                  int ii = -1;
                  for (ii = graceNotes.size() - 1; ii >= 0; ii--) {
                        Chord* gc = graceNotes[ii];
                        if(gc->voice() == cr->voice()){
                             cr->add(gc);
                             }
                        }
                  graceNotes.clear();
                  }

            if(cr)
                  cr->setStaffMove(move);

            char c = step[0].toLatin1();
            note = new Note(score);
            note->setSmall(small);
            note->setHeadGroup(headGroup);
            if (noteheadColor != QColor::Invalid)
                  note->setColor(noteheadColor);

            if (noteheadParentheses) {
                  Symbol* s = new Symbol(score);
                  s->setSym(SymId::noteheadParenthesisLeft);
                  s->setParent(note);
                  score->addElement(s);
                  s = new Symbol(score);
                  s->setSym(SymId::noteheadParenthesisRight);
                  s->setParent(note);
                  score->addElement(s);
                  }

            if (velocity > 0) {
                  note->setVeloType(Note::ValueType::USER_VAL);
                  note->setVeloOffset(velocity);
                  }

            // pitch must be set before adding note to chord as note
            // is inserted into pitch sorted list (ws)

            if (unpitched
                && hasDrumset
                && drumsets.contains(partId)
                && drumsets[partId].contains(instrId)) {
                  // step and oct are display-step and ...-oct
                  // get pitch from instrument definition in drumset instead
                  int pitch = drumsets[partId][instrId].pitch;
                  note->setPitch(pitch);
                  // TODO - does this need to be key-aware?
                  note->setTpc(pitch2tpc(pitch, Key::C, Prefer::NEAREST)); // TODO: necessary ?
                  }
            else
                  xmlSetPitch(note, c, alter, octave, ottavas, track);

            cr->add(note);

            static_cast<Chord*>(cr)->setNoStem(noStem);
            if (cue) cr->setSmall(cue); // only once per chord

            // qDebug("staff for new note: %p (staff=%d, relStaff=%d)",
            //        score->staff(staff + relStaff), staff, relStaff);

            if(accidental != Accidental::Type::NONE){
                  Accidental* a = new Accidental(score);
                  a->setAccidentalType(accidental);
                   if (editorial || cautionary || parentheses) {
                          a->setHasBracket(cautionary || parentheses);
                          a->setRole(Accidental::Role::USER);
                          }
                    else {
                          alt = alter;
                          }
                   note->add(a);
                   }

            // LVIFIX: quarter tone accidentals support is "drawing only"
            //WS-TODO if (accidental == 18
            // || accidental == 19
            // || accidental == 22
            // || accidental == 25)
            // note->setAccidentalType(accidental);

            // remember beam mode last non-grace note
            // bm == Beam::Mode::NONE means no <beam> was found
            if (!grace && bm != Beam::Mode::NONE)
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
                  if (sd == MScore::Direction::AUTO) {
                        if (line > 4)
                              sd = MScore::Direction::DOWN;
                        else
                              sd = MScore::Direction::UP;
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

      foreach(QDomElement de, notations) {
            xmlNotations(note, cr, trk, loc_tick, ticks, de);
            }

      // add lyrics found by xmlLyric
      addLyrics(cr, numberedLyrics, defaultyLyrics, unNumberedLyrics);

      // add figured bass element
      if (!figBassList.isEmpty()) {
            int sTick = tick;  // starting tick
            foreach (FiguredBass* fb, figBassList) {
                  fb->setTrack(trk);
                  // No duration tag defaults ticks() to 0; set to note value
                  if (fb->ticks() == 0)
                        fb->setTicks(ticks);
                  // TODO: set correct onNote value
                  Segment* s = measure->getSegment(Segment::Type::ChordRest, sTick);
                  s->add(fb);
                  sTick += fb->ticks();
                  }
            figBassList.clear();
            }

      // fixup pedal type="change" to end at the end of this note
      // note tick is still at note start
      if (pedalContinue) {
            handleSpannerStop(pedalContinue, "pedal", track, tick + ticks, spanners);
            pedalContinue = 0;
            }

      if (!chord && !grace)
            prevtick = tick;  // remember tick where last chordrest was inserted

#ifdef DEBUG_TICK
      qDebug(" after inserting note tick=%d prevtick=%d", tick, prevtick);
#endif
      return note;
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
      // in order to work correctly, this should probably be adjusted to account for spatium
      // but in any case, we don't support import relative-x/y for other elements
      // no reason to do so for chord symbols
      double rx = 0.0;  // 0.1 * e.attribute("relative-x", "0").toDouble();
      double ry = 0.0;  // -0.1 * e.attribute("relative-y", "0").toDouble();

      double styleYOff = score->textStyle(TextStyleType::HARMONY).offset().y();
      OffsetType offsetType = score->textStyle(TextStyleType::HARMONY).offsetType();
      if (offsetType == OffsetType::ABS) {
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
                        ha->setRootTpc(Tpc::TPC_INVALID);
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
                              degreeList << HDegree(degreeValue, degreeAlter, HDegreeType::ADD);
                        else if (degreeType == "alter")
                              degreeList << HDegree(degreeValue, degreeAlter, HDegreeType::ALTER);
                        else if (degreeType == "subtract")
                              degreeList << HDegree(degreeValue, degreeAlter, HDegreeType::SUBTRACT);
                        }
                  }
            else if (tag == "frame") {
                  qDebug("xmlHarmony: found harmony frame");
                  FretDiagram* fd = new FretDiagram(score);
                  fd->setTrack(staff * VOICES);
                  // read frame into FretDiagram
                  readFretDiagram(fd, e);
                  Segment* s = measure->getSegment(Segment::Type::ChordRest, tick);
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
      if (ha->rootTpc() != Tpc::TPC_INVALID)
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
      Segment* s = measure->getSegment(Segment::Type::ChordRest, tick + offset);
      s->add(ha);
      }

//---------------------------------------------------------
//   xmlClef
//---------------------------------------------------------

StaffTypes MusicXml::xmlClef(QDomElement e, int staffIdx, Measure* measure)
      {
      ClefType clef   = ClefType::G;
      StaffTypes res = StaffTypes::STANDARD;
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
            clef = ClefType::PERC;
            res = StaffTypes::PERC_DEFAULT;
            }
      else if (c == "TAB") {
            clef = ClefType::TAB;
            res = StaffTypes::TAB_DEFAULT;
            }
      else
            qDebug("ImportMusicXML: unknown clef <sign=%s line=%d oct ch=%d>", qPrintable(c), line, i);
      // note: also generate symbol for tick 0
      // was not necessary before 0.9.6
      if (clefno < score->staff(staffIdx)->part()->staves()->size()) {
            Clef* clefs = new Clef(score);
            clefs->setClefType(clef);
            clefs->setTrack((staffIdx + clefno) * VOICES);
            Segment* s = measure->getSegment(clefs, tick);
            s->add(clefs);
    //      clefs->staff()->setClef(tick, clefs->clefTypeList());
            }
      return res;
      }

//---------------------------------------------------------
//   findSlurElement
//---------------------------------------------------------
QList<QDomElement> MusicXml::findSlurElements(QDomElement e)
      {
      QList<QDomElement> slurs;
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "notations"){
                  for(e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement())
                        if (e.tagName() == "slur")
                              slurs.append(e);
                  }
            }
      return slurs;
      }

//---------------------------------------------------------
//   addGraceNoteAfter
//---------------------------------------------------------

void MusicXml::addGraceNoteAfter(Chord* graceNote, Segment* segm)
      {
      if(segm){
            graceNote->toGraceAfter();
            Element* el = segm->element(graceNote->track());
            if (el && el->type() == Element::Type::CHORD) {
                  Chord* cr = static_cast<Chord*>(el);
                  cr->add(graceNote);
                  }
            }
      }
}

