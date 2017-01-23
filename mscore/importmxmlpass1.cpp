//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2015 Werner Schweer and others
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

#include "libmscore/box.h"
#include "libmscore/measure.h"
#include "libmscore/page.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/stringdata.h"
#include "libmscore/sym.h"
#include "libmscore/symbol.h"
#include "libmscore/timesig.h"
#include "libmscore/style.h"

#include "importmxmlpass1.h"
#include "importmxmlpass2.h"
#include "preferences.h"

namespace Ms {

//---------------------------------------------------------
//   noteTypeToFraction
//---------------------------------------------------------

/**
 Convert MusicXML note type to fraction.
 */

static Fraction noteTypeToFraction(const QString& type)
      {
      if (type == "1024th")
            return Fraction(1, 1024);
      else if (type == "512th")
            return Fraction(1, 512);
      else if (type == "256th")
            return Fraction(1, 256);
      else if (type == "128th")
            return Fraction(1, 128);
      else if (type == "64th")
            return Fraction(1, 64);
      else if (type == "32nd")
            return Fraction(1, 32);
      else if (type == "16th")
            return Fraction(1, 16);
      else if (type == "eighth")
            return Fraction(1, 8);
      else if (type == "quarter")
            return Fraction(1, 4);
      else if (type == "half")
            return Fraction(1, 2);
      else if (type == "whole")
            return Fraction(1, 1);
      else if (type == "breve")
            return Fraction(2, 1);
      else if (type == "long")
            return Fraction(4, 1);
      else if (type == "maxima")
            return Fraction(8, 1);
      else
            return Fraction(0, 0);
      }

//---------------------------------------------------------
//   calculateFraction
//---------------------------------------------------------

/**
 Convert note type, number of dots and actual and normal notes into a duration
 */

static Fraction calculateFraction(const QString& type, const int dots, const Fraction timeMod)
      {
      // type
      Fraction f = noteTypeToFraction(type);
      if (f.isValid()) {
            // dot(s)
            Fraction f_no_dots = f;
            for (int i = 0; i < dots; ++i)
                  f += (f_no_dots / (2 << i));
            // tuplet
            if (timeMod.isValid())
                  f *= timeMod;
            // clean up (just in case)
            f.reduce();
            }
      return f;
      }

//---------------------------------------------------------
//   allocateStaves
//---------------------------------------------------------

/**
 Allocate MuseScore staff to MusicXML voices.
 For each staff, allocate at most VOICES voices to the staff.
 */

// for regular (non-overlapping) voices:
// 1) assign voice to a staff (allocateStaves)
// 2) assign voice numbers (allocateVoices)
// due to cross-staving, it is not a priori clear to which staff
// a voice has to be assigned
// allocate ordered by number of chordrests in the MusicXML voice
//
// for overlapping voices:
// 1) assign voice to staves it is found in (allocateStaves)
// 2) assign voice numbers (allocateVoices)

static void allocateStaves(VoiceList& vcLst)
      {
      // initialize
      int voicesAllocated[MAX_STAVES]; // number of voices allocated on each staff
      for (int i = 0; i < MAX_STAVES; ++i)
            voicesAllocated[i] = 0;

      // handle regular (non-overlapping) voices
      // note: outer loop executed vcLst.size() times, as each inner loop handles exactly one item
      for (int i = 0; i < vcLst.size(); ++i) {
            // find the regular voice containing the highest number of chords and rests that has not been handled yet
            int max = 0;
            QString key;
            for (VoiceList::const_iterator j = vcLst.constBegin(); j != vcLst.constEnd(); ++j) {
                  if (!j.value().overlaps() && j.value().numberChordRests() > max && j.value().staff() == -1) {
                        max = j.value().numberChordRests();
                        key = j.key();
                        }
                  }
            if (key != "") {
                  int prefSt = vcLst.value(key).preferredStaff();
                  if (voicesAllocated[prefSt] < VOICES) {
                        vcLst[key].setStaff(prefSt);
                        voicesAllocated[prefSt]++;
                        }
                  else
                        // out of voices: mark as used but not allocated
                        vcLst[key].setStaff(-2);
                  }
            }

      // handle overlapping voices
      // for every staff allocate remaining voices (if space allows)
      // the ones with the highest number of chords and rests get allocated first
      for (int h = 0; h < MAX_STAVES; ++h) {
            // note: middle loop executed vcLst.size() times, as each inner loop handles exactly one item
            for (int i = 0; i < vcLst.size(); ++i) {
                  // find the overlapping voice containing the highest number of chords and rests that has not been handled yet
                  int max = 0;
                  QString key;
                  for (VoiceList::const_iterator j = vcLst.constBegin(); j != vcLst.constEnd(); ++j) {
                        if (j.value().overlaps() && j.value().numberChordRests(h) > max && j.value().staffAlloc(h) == -1) {
                              max = j.value().numberChordRests(h);
                              key = j.key();
                              }
                        }
                  if (key != "") {
                        int prefSt = h;
                        if (voicesAllocated[prefSt] < VOICES) {
                              vcLst[key].setStaffAlloc(prefSt, 1);
                              voicesAllocated[prefSt]++;
                              }
                        else
                              // out of voices: mark as used but not allocated
                              vcLst[key].setStaffAlloc(prefSt, -2);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   allocateVoices
//---------------------------------------------------------

/**
 Allocate MuseScore voice to MusicXML voices.
 For each staff, the voices are number 1, 2, 3, 4
 in the same order they are numbered in the MusicXML file.
 */

static void allocateVoices(VoiceList& vcLst)
      {
      int nextVoice[MAX_STAVES]; // number of voices allocated on each staff
      for (int i = 0; i < MAX_STAVES; ++i)
            nextVoice[i] = 0;
      // handle regular (non-overlapping) voices
      // a voice is allocated on one specific staff
      for (VoiceList::const_iterator i = vcLst.constBegin(); i != vcLst.constEnd(); ++i) {
            int staff = i.value().staff();
            QString key   = i.key();
            if (staff >= 0) {
                  vcLst[key].setVoice(nextVoice[staff]);
                  nextVoice[staff]++;
                  }
            }
      // handle overlapping voices
      // each voice may be in every staff
      for (VoiceList::const_iterator i = vcLst.constBegin(); i != vcLst.constEnd(); ++i) {
            for (int j = 0; j < MAX_STAVES; ++j) {
                  int staffAlloc = i.value().staffAlloc(j);
                  QString key   = i.key();
                  if (staffAlloc >= 0) {
                        vcLst[key].setVoice(j, nextVoice[j]);
                        nextVoice[j]++;
                        }
                  }
            }
      }


//---------------------------------------------------------
//   copyOverlapData
//---------------------------------------------------------

/**
 Copy the overlap data from the overlap detector to the voice list.
 */

static void copyOverlapData(VoiceOverlapDetector& vod, VoiceList& vcLst)
      {
      for (VoiceList::const_iterator i = vcLst.constBegin(); i != vcLst.constEnd(); ++i) {
            QString key = i.key();
            if (vod.stavesOverlap(key))
                  vcLst[key].setOverlap(true);
            }
      }

//---------------------------------------------------------
//   MusicXMLParserPass1
//---------------------------------------------------------

MusicXMLParserPass1::MusicXMLParserPass1(Score* score)
      : _divs(0), _score(score)
      {
      // nothing
      }

//---------------------------------------------------------
//   initPartState
//---------------------------------------------------------

/**
 Initialize members as required for reading the MusicXML part element.
 TODO: factor out part reading into a separate class
 TODO: preferably use automatically initialized variables
 Note that Qt automatically initializes new elements in QVector (tuplets).
 */

void MusicXMLParserPass1::initPartState(const QString& /* partId */)
      {
      _timeSigDura = Fraction(0, 0);       // invalid
      _octaveShifts.clear();
      _firstInstrSTime = Fraction(0, 1);
      _firstInstrId = "";
      }

//---------------------------------------------------------
//   determineMeasureLength
//---------------------------------------------------------

/**
 Determine the length in ticks of each measure in all parts.
 Return false on error.
 */

bool MusicXMLParserPass1::determineMeasureLength(QVector<Fraction>& ml) const
      {
      ml.clear();

      // determine number of measures: max number of measures in any part
      int nMeasures = 0;
      foreach (const MusicXmlPart &part, _parts) {
            if (part.nMeasures() > nMeasures)
                  nMeasures = part.nMeasures();
            }

      // determine max length of a specific measure in all parts
      for (int i = 0; i < nMeasures; ++i) {
            Fraction maxMeasDur;
            foreach (const MusicXmlPart &part, _parts) {
                  if (i < part.nMeasures()) {
                        Fraction measDurPartJ = part.measureDuration(i);
                        if (measDurPartJ > maxMeasDur)
                              maxMeasDur = measDurPartJ;
                        }
                  }
            //qDebug("determineMeasureLength() measure %d %s (%d)", i, qPrintable(maxMeasDur.print()), maxMeasDur.ticks());
            ml.append(maxMeasDur);
            }
      return true;
      }

//---------------------------------------------------------
//   getVoiceList
//---------------------------------------------------------

/**
 Get the VoiceList for part \a id.
 Return an empty VoiceList on error.
 */

VoiceList MusicXMLParserPass1::getVoiceList(const QString id) const
      {
      if (_parts.contains(id))
            return _parts.value(id).voicelist;
      return VoiceList();
      }

//---------------------------------------------------------
//   getInstrList
//---------------------------------------------------------

/**
 Get the MusicXmlInstrList for part \a id.
 Return an empty MusicXmlInstrList on error.
 */

MusicXmlInstrList MusicXMLParserPass1::getInstrList(const QString id) const
      {
      if (_parts.contains(id))
            return _parts.value(id)._instrList;
      return MusicXmlInstrList();
      }

//---------------------------------------------------------
//   determineMeasureLength
//---------------------------------------------------------

/**
 Set default notehead, line and stem direction
 for instrument \a instrId in part \a id.
 */

void MusicXMLParserPass1::setDrumsetDefault(const QString& id,
                                            const QString& instrId,
                                            const NoteHead::Group hg,
                                            const int line,
                                            const Direction sd)
      {
      if (_drumsets.contains(id)
          && _drumsets[id].contains(instrId)) {
            _drumsets[id][instrId].notehead = hg;
            _drumsets[id][instrId].line = line;
            _drumsets[id][instrId].stemDirection = sd;
            }
      }


//---------------------------------------------------------
//   determineStaffMoveVoice
//---------------------------------------------------------

/**
 For part \a id, determine MuseScore (ms) staffmove, track and voice from MusicXML (mx) staff and voice
 MusicXML staff is 0 for the first staff, 1 for the second.
 Note: track is the first track of the ms staff in the score, add ms voice for elements in a voice
 Return true if OK, false on error
 TODO: finalize
 */

bool MusicXMLParserPass1::determineStaffMoveVoice(const QString& id, const int mxStaff, const QString& mxVoice,
                                                  int& msMove, int& msTrack, int& msVoice) const
      {
      VoiceList voicelist = getVoiceList(id);
      msMove = 0; // TODO
      msTrack = 0; // TODO
      msVoice = 0; // TODO


      // Musicxml voices are counted for all staffs of an
      // instrument. They are not limited. In mscore voices are associated
      // with a staff. Every staff can have at most VOICES voices.

      // The following lines map musicXml voices to mscore voices.
      // If a voice crosses two staffs, this is expressed with the
      // "move" parameter in mscore.

      // Musicxml voices are unique within a part, but not across parts.

      //qDebug("voice mapper before: voice='%s' staff=%d", qPrintable(mxVoice), mxStaff);
      int s; // staff mapped by voice mapper
      int v; // voice mapped by voice mapper
      if (voicelist.value(mxVoice).overlaps()) {
            // for overlapping voices, the staff does not change
            // and the voice is mapped and staff-dependent
            s = mxStaff;
            v = voicelist.value(mxVoice).voice(s);
            }
      else {
            // for non-overlapping voices, both staff and voice are
            // set by the voice mapper
            s = voicelist.value(mxVoice).staff();
            v = voicelist.value(mxVoice).voice();
            }

      //qDebug("voice mapper mapped: s=%d v=%d", s, v);
      if (s < 0 || v < 0) {
            qDebug("ImportMusicXml: too many voices (staff=%d voice='%s' -> s=%d v=%d)",
                   mxStaff + 1, qPrintable(mxVoice), s, v);
            return false;
            }

      msMove  = mxStaff - s;
      msVoice = v;

      // make score-relative instead on part-relative
      Part* part = _partMap.value(id);
      Q_ASSERT(part);
      int scoreRelStaff = _score->staffIdx(part); // zero-based number of parts first staff in the score
      msTrack = (scoreRelStaff + s) * VOICES;

      //qDebug("voice mapper after: scoreRelStaff=%d partRelStaff=%d msMove=%d msTrack=%d msVoice=%d",
      //       scoreRelStaff, s, msMove, msTrack, msVoice);
      // note: relStaff is the staff number relative to the parts first staff
      //       voice is the voice number in the staff

      return true;
      }

//---------------------------------------------------------
//   hasPart
//---------------------------------------------------------

/**
 Check if part \a id is found.
 */

bool MusicXMLParserPass1::hasPart(const QString& id) const
      {
      return _parts.contains(id);
      }

//---------------------------------------------------------
//   trackForPart
//---------------------------------------------------------

/**
 Return the (score relative) track number for the first staff of part \a id.
 */

int MusicXMLParserPass1::trackForPart(const QString& id) const
      {
      Part* part = _partMap.value(id);
      Q_ASSERT(part);
      int scoreRelStaff = _score->staffIdx(part); // zero-based number of parts first staff in the score
      return scoreRelStaff * VOICES;
      }

//---------------------------------------------------------
//   getMeasureStart
//---------------------------------------------------------

/**
 Return the measure start time for measure \a i.
 */

Fraction MusicXMLParserPass1::getMeasureStart(const int i) const
      {
      if (0 <= i && i < _measureStart.size())
            return _measureStart.at(i);
      else
            return Fraction(0, 0);       // invalid
      }

//---------------------------------------------------------
//   octaveShift
//---------------------------------------------------------

/**
 Return the octave shift for part \a id in \a staff at \a f.
 */

int MusicXMLParserPass1::octaveShift(const QString& id, const int staff, const Fraction f) const
      {
      if (_parts.contains(id))
            return _parts.value(id).octaveShift(staff, f);

      return 0;
      }

//---------------------------------------------------------
//   logDebugTrace
//---------------------------------------------------------

/**
 Log debug (function) trace.
 */

void MusicXMLParserPass1::logDebugTrace(const QString& /*info*/)
      {
      //qDebug("Trace %s", qPrintable(info));
      }

//---------------------------------------------------------
//   logDebugInfo
//---------------------------------------------------------

/**
 Log debug \a info (non-fatal events relevant for debugging).
 */

void MusicXMLParserPass1::logDebugInfo(const QString& /*info*/)
      {
      //qDebug("Info at line %lld col %lld: %s",
      //       _e.lineNumber(), _e.columnNumber(), qPrintable(info));
      }

//---------------------------------------------------------
//   logError
//---------------------------------------------------------

/**
 Log \a error (possibly non-fatal but to be reported to the user anyway).
 */

void MusicXMLParserPass1::logError(const QString& error)
      {
      QString err;
      err = QString("Error at line %1 col %2: %3").arg(_e.lineNumber()).arg(_e.columnNumber()).arg(error);
      qDebug("%s", qPrintable(err));
      _parseStatus += err;
      }


//---------------------------------------------------------
//   skipLogCurrElem
//---------------------------------------------------------

/**
 Skip the current element, log debug as info.
 */

void MusicXMLParserPass1::skipLogCurrElem()
      {
      logDebugInfo(QString("skipping '%1'").arg(_e.name().toString()));
      _e.skipCurrentElement();
      }

//---------------------------------------------------------
//   createMeasures
//---------------------------------------------------------

/**
 Create required measures with correct number, start tick and length for Score \a score.
 */

static void createMeasures(Score* score, const QVector<Fraction>& ml, const QVector<Fraction>& ms)
      {
      for (int i = 0; i < ml.size(); ++i) {
            Measure* measure  = new Measure(score);
            measure->setTick(ms.at(i).ticks());
            measure->setLen(ml.at(i));
            measure->setNo(i);
            score->measures()->add(measure);
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

static void determineMeasureStart(const QVector<Fraction>& ml, QVector<Fraction>& ms)
      {
      ms.resize(ml.size());
      if (!(ms.size() > 0))
            return;  // no parts read

      // first measure starts at t = 0
      ms[0] = Fraction(0, 1);
      // all others start at start time previous measure plus length previous measure
      for (int i = 1; i < ml.size(); i++)
            ms[i] = ms.at(i - 1) + ml.at(i - 1);
      //for (int i = 0; i < ms.size(); i++)
      //      qDebug("measurestart ms[%d] %s", i + 1, qPrintable(ms.at(i).print()));
      }

//---------------------------------------------------------
//   addText
//---------------------------------------------------------

/**
 Add text \a strTxt to VBox \a vbx using SubStyle \a stl.
 */

static void addText(VBox* vbx, Score* s, QString strTxt, SubStyle stl)
      {
      if (!strTxt.isEmpty()) {
            Text* text = new Text(stl, s);
            text->setXmlText(strTxt);
            vbx->add(text);
            }
      }

//---------------------------------------------------------
//   addText
//---------------------------------------------------------

/**
 Add text \a strTxt to VBox \a vbx using SubStyle \a stl.
 Also sets Align and Yoff.
 */

static void addText2(VBox* vbx, Score* s, QString strTxt, SubStyle stl, Align v, double yoffs)
      {
      if (!strTxt.isEmpty()) {
            Text* text = new Text(stl, s);
            text->setXmlText(strTxt);
            text->setAlign(v);
            text->setOffset(QPointF(0.0, yoffs));
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

static void doCredits(Score* score, const CreditWordsList& credits, const int pageWidth, const int pageHeight)
      {
      /*
      qDebug("MusicXml::doCredits()");
      qDebug("page format set (inch) w=%g h=%g tm=%g spatium=%g DPMM=%g DPI=%g",
             pf->width(), pf->height(), pf->oddTopMargin(), score->spatium(), DPMM, DPI);
      */
      // page width, height and odd top margin in tenths
      const double ph  = score->styleD(StyleIdx::pageHeight) * 10 * DPI / score->spatium();
      const int pw1 = pageWidth / 3;
      const int pw2 = pageWidth * 2 / 3;
      const int ph2 = pageHeight / 2;
      /*
      const double pw  = pf->width() * 10 * DPI / score->spatium();
      const double tm  = pf->oddTopMargin() * 10 * DPI / score->spatium();
      const double tov = ph - tm;
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
                                 SubStyle::COMPOSER, Align::RIGHT | Align::BOTTOM,
                                 (miny - w->defaultY) * score->spatium() / (10 * DPI));
                        }
                  // poet is in the left column
                  else if (defx < pw1) {
                        // found poet
                        addText2(vbox, score, w->words,
                                 SubStyle::POET, Align::LEFT | Align::BOTTOM,
                                 (miny - w->defaultY) * score->spatium() / (10 * DPI));
                        }
                  // save others (in the middle column) to be handled later
                  else {
                        creditMap.insert(defy, w);
                        }
                  }
            // keep remaining footer text for possible use as copyright
            else if (useHeader && defy < ph2) {
                  // found credit words in both header and footer
                  // header was used to create a vbox at the top of the first page
                  // footer is ignored as it conflicts with the default MuseScore footer style
                  //qDebug("add to copyright: '%s'", qPrintable(w->words));
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
                     SubStyle::TITLE, Align::HCENTER | Align::TOP,
                     (maxy - w->defaultY) * score->spatium() / (10 * DPI));
            }

      // add remaining credit-words as subtitles
      for (int i = 0; i < (keys.size() - 1); i++) {
            CreditWords* w = creditMap.value(keys.at(i));
            //qDebug("subtitle='%s'", qPrintable(w->words));
            addText2(vbox, score, w->words,
                     SubStyle::SUBTITLE, Align::HCENTER | Align::TOP,
                     (maxy - w->defaultY) * score->spatium() / (10 * DPI));
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

            addText(vbox, score, strTitle.toHtmlEscaped(),      SubStyle::TITLE);
            addText(vbox, score, strSubTitle.toHtmlEscaped(),   SubStyle::SUBTITLE);
            addText(vbox, score, strComposer.toHtmlEscaped(),   SubStyle::COMPOSER);
            addText(vbox, score, strPoet.toHtmlEscaped(),       SubStyle::POET);
            addText(vbox, score, strTranslator.toHtmlEscaped(), SubStyle::TRANSLATOR);
            }

      if (vbox) {
            vbox->setTick(0);
            score->measures()->add(vbox);
            }
      }

//---------------------------------------------------------
//   parse
//---------------------------------------------------------

/**
 Parse MusicXML in \a device and extract pass 1 data.
 */

Score::FileError MusicXMLParserPass1::parse(QIODevice* device)
      {
      logDebugTrace("MusicXMLParserPass1::parse device");
      _parts.clear();
      _e.setDevice(device);
      Score::FileError res = parse();
      if (res != Score::FileError::FILE_NO_ERROR)
            return res;

      // Determine the start tick of each measure in the part
      determineMeasureLength(_measureLength);
      determineMeasureStart(_measureLength, _measureStart);
      createMeasures(_score, _measureLength, _measureStart);
      return res;
      }

//---------------------------------------------------------
//   parse
//---------------------------------------------------------

/**
 Start the parsing process, after verifying the top-level node is score-partwise
 */

Score::FileError MusicXMLParserPass1::parse()
      {
      logDebugTrace("MusicXMLParserPass1::parse");

      bool found = false;
      while (_e.readNextStartElement()) {
            if (_e.name() == "score-partwise") {
                  found = true;
                  scorePartwise();
                  }
            else {
                  logError(QString("this is not a MusicXML score-partwise file (top-level node '%1')")
                           .arg(_e.name().toString()));
                  _e.skipCurrentElement();
                  return Score::FileError::FILE_BAD_FORMAT;
                  }
            }

      if (!found) {
            logError("this is not a MusicXML score-partwise file, node <score-partwise> not found");
            return Score::FileError::FILE_BAD_FORMAT;
            }

      return Score::FileError::FILE_NO_ERROR;
      }

//---------------------------------------------------------
//   allStaffGroupsIdentical
//---------------------------------------------------------

/**
 Return true if all staves in Part \a p have the same staff group
 */

static bool allStaffGroupsIdentical(Part const* const p)
      {
      for (int i = 1; i < p->nstaves(); ++i) {
            if (p->staff(0)->staffType(0)->group() != p->staff(i)->staffType(0)->group())
                  return false;
            }
      return true;
      }

//---------------------------------------------------------
//   scorePartwise
//---------------------------------------------------------

/**
 Parse the MusicXML top-level (XPath /score-partwise) node.
 */

void MusicXMLParserPass1::scorePartwise()
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "score-partwise");
      logDebugTrace("MusicXMLParserPass1::scorePartwise");

      MusicXmlPartGroupList partGroupList;
      CreditWordsList credits;
      int pageWidth;                             ///< Page width read from defaults
      int pageHeight;                            ///< Page height read from defaults

      while (_e.readNextStartElement()) {
            if (_e.name() == "part")
                  part();
            else if (_e.name() == "part-list") {
                  // if any credits are present, they have been read now
                  // add the credits to the score before adding any measure
                  // note that a part-list element must always be present
                  doCredits(_score, credits, pageWidth, pageHeight);
                  // and read the part list
                  partList(partGroupList);
                  }
            else if (_e.name() == "work") {
                  while (_e.readNextStartElement()) {
                        if (_e.name() == "work-number")
                              _score->setMetaTag("workNumber", _e.readElementText());
                        else if (_e.name() == "work-title")
                              _score->setMetaTag("workTitle", _e.readElementText());
                        else
                              skipLogCurrElem();
                        }
                  }
            else if (_e.name() == "identification")
                  identification();
            else if (_e.name() == "defaults")
                  defaults(pageWidth, pageHeight);
            else if (_e.name() == "movement-number")
                  _score->setMetaTag("movementNumber", _e.readElementText());
            else if (_e.name() == "movement-title")
                  _score->setMetaTag("movementTitle", _e.readElementText());
            else if (_e.name() == "credit")
                  credit(credits);
            else
                  skipLogCurrElem();
            }

      // add brackets where required

      /*
       qDebug("partGroupList");
       for (int i = 0; i < (int) partGroupList.size(); i++) {
       MusicXmlPartGroup* pg = partGroupList[i];
       qDebug("part-group span %d start %d type %hhd barlinespan %d",
       pg->span, pg->start, pg->type, pg->barlineSpan);
       }
       */

      // set of (typically multi-staff) parts containing one or more explicit brackets
      // spanning only that part: these won't get an implicit brace later
      // e.g. a two-staff piano part with an explicit brace
      QSet<Part const* const> partSet;

      // handle the explicit brackets
      const QList<Part*>& il = _score->parts();
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
                  if (allStaffGroupsIdentical(p)) {
                        // span only if the same types
                        p->staff(0)->setBarLineSpan(p->nstaves());
                        }
                  }
            }
      }

//---------------------------------------------------------
//   identification
//---------------------------------------------------------

/**
 Parse the /score-partwise/identification node:
 read the metadata.
 */

void MusicXMLParserPass1::identification()
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "identification");
      logDebugTrace("MusicXMLParserPass1::identification");

      while (_e.readNextStartElement()) {
            if (_e.name() == "creator") {
                  // type is an arbitrary label
                  QString strType = _e.attributes().value("type").toString();
                  _score->setMetaTag(strType, _e.readElementText());
                  }
            else if (_e.name() == "rights")
                  _score->setMetaTag("copyright", _e.readElementText());
            else if (_e.name() == "encoding") {
                  // TODO
                  _e.skipCurrentElement(); // skip but don't log
                  // _score->setMetaTag("encoding", _e.readElementText()); works with DOM but not with pull parser
                  // temporarily fake the encoding tag (compliant with DOM parser) to help the autotester
                  if (MScore::debugMode)
                        _score->setMetaTag("encoding", "MuseScore 0.7.02007-09-10");
                  }
            else if (_e.name() == "source")
                  _score->setMetaTag("source", _e.readElementText());
            else if (_e.name() == "miscellaneous")
                  // TODO
                  _e.skipCurrentElement();  // skip but don't log
            else
                  skipLogCurrElem();
            }
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
//---------------------------------------------------------

/**
 Decode &#...; in string \a src into UNICODE (utf8) character.
 */

static QString decodeEntities( const QString& src )
      {
      QString ret(src);
      QRegExp re("&#([0-9]+);");
      re.setMinimal(true);

      int pos = 0;
      while ( (pos = re.indexIn(src, pos)) != -1 ) {
            ret = ret.replace(re.cap(0), QChar(re.cap(1).toInt(0,10)));
            pos += re.matchedLength();
            }
      return ret;
      }

//---------------------------------------------------------
//   nextPartOfFormattedString
//---------------------------------------------------------

// TODO: probably should be shared between pass 1 and 2

/**
 Read the next part of a MusicXML formatted string and convert to MuseScore internal encoding.
 */

static QString nextPartOfFormattedString(QXmlStreamReader& e)
      {
      //QString lang       = e.attribute(QString("xml:lang"), "it");
      QString fontWeight = e.attributes().value("font-weight").toString();
      QString fontSize   = e.attributes().value("font-size").toString();
      QString fontStyle  = e.attributes().value("font-style").toString();
      QString underline  = e.attributes().value("underline").toString();
      QString fontFamily = e.attributes().value("font-family").toString();
      // TODO: color, enclosure, yoffset in only part of the text, ...

      QString txt        = e.readElementText();
      // replace HTML entities
      txt = decodeEntities(txt);
      QString syms       = text2syms(txt);

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
//   credit
//---------------------------------------------------------

/**
 Parse the /score-partwise/credit node:
 read the credits for later handling by doCredits().
 */

void MusicXMLParserPass1::credit(CreditWordsList& credits)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "credit");
      logDebugTrace("MusicXMLParserPass1::credit");

      QString page = _e.attributes().value("page").toString();
      // handle only page 1 credits (to extract title etc.)
      // assume no page attribute means page 1
      if (page == "" || page == "1") {
            // multiple credit-words elements may be present,
            // which are appended
            // use the position info from the first one
            // font information is ignored, credits will be styled
            bool creditWordsRead = false;
            double defaultx = 0;
            double defaulty = 0;
            QString justify;
            QString halign;
            QString valign;
            QString crwords;
            while (_e.readNextStartElement()) {
                  if (_e.name() == "credit-words") {
                        // IMPORT_LAYOUT
                        if (!creditWordsRead) {
                              defaultx = _e.attributes().value("default-x").toString().toDouble();
                              defaulty = _e.attributes().value("default-y").toString().toDouble();
                              justify  = _e.attributes().value("justify").toString();
                              halign   = _e.attributes().value("halign").toString();
                              valign   = _e.attributes().value("valign").toString();
                              creditWordsRead = true;
                              }
                        crwords += nextPartOfFormattedString(_e);
                        }
                  else if (_e.name() == "credit-type")
                        skipLogCurrElem();
                  else
                        skipLogCurrElem();
                  }
            if (crwords != "") {
                  CreditWords* cw = new CreditWords(defaultx, defaulty, justify, halign, valign, crwords);
                  credits.append(cw);
                  }
            }
      else
            skipLogCurrElem();

      Q_ASSERT(_e.isEndElement() && _e.name() == "credit");
      }

//---------------------------------------------------------
//   mustSetSize
//---------------------------------------------------------

/**
 Determine if i is a style type for which the default size must be set
 */

// The MusicXML specification does not specify to which kinds of text
// the word-font setting applies. Setting all sizes to the size specified
// gives bad results, e.g. for measure numbers, so a selection is made.
// Some tweaking may still be required.
#if 0
static bool mustSetSize(const int i)
      {
      return
            i == int(SubStyle::TITLE)
            || i == int(SubStyle::SUBTITLE)
            || i == int(SubStyle::COMPOSER)
            || i == int(SubStyle::POET)
            || i == int(SubStyle::INSTRUMENT_LONG)
            || i == int(SubStyle::INSTRUMENT_SHORT)
            || i == int(SubStyle::INSTRUMENT_EXCERPT)
            || i == int(SubStyle::TEMPO)
            || i == int(SubStyle::METRONOME)
            || i == int(SubStyle::TRANSLATOR)
            || i == int(SubStyle::SYSTEM)
            || i == int(SubStyle::STAFF)
            || i == int(SubStyle::REPEAT_LEFT)
            || i == int(SubStyle::REPEAT_RIGHT)
            || i == int(SubStyle::TEXTLINE)
            || i == int(SubStyle::GLISSANDO)
            || i == int(SubStyle::INSTRUMENT_CHANGE);
      }
#endif

//---------------------------------------------------------
//   updateStyles
//---------------------------------------------------------

/**
 Update the style definitions to match the MusicXML word-font and lyric-font.
 */

static void updateStyles(Score* score,
                         const QString& wordFamily, const QString& wordSize,
                         const QString& lyricFamily, const QString& lyricSize)
      {
//TODO:ws       const float fWordSize = wordSize.toFloat();   // note conversion error results in value 0.0
      const float fLyricSize = lyricSize.toFloat(); // but avoid comparing float with exact value later

      // loop over all text styles (except the empty, always hidden, first one)
      // set all text styles to the MusicXML defaults
#if 0 // TODO:ws
      for (int i = int(SubStyle::DEFAULT) + 1; i < int(SubStyle::TEXT_STYLES); ++i) {
            TextStyle ts = score->style().textStyle(TextStyleType(i));
            if (i == int(SubStyle::LYRIC1) || i == int(SubStyle::LYRIC2)) {
                  if (lyricFamily != "")
                        ts.setFamily(lyricFamily);
                  if (fLyricSize > 0.001)
                        ts.setSize(fLyricSize);
                  }
            else {
                  if (wordFamily != "")
                        ts.setFamily(wordFamily);
                  if (fWordSize > 0.001 && mustSetSize(i))
                        ts.setSize(fWordSize);
                  }
            score->style().setTextStyle(ts);
            }
#endif
      if (lyricFamily != "") {
            score->style().set(StyleIdx::lyricsOddFontFace, lyricFamily);
            score->style().set(StyleIdx::lyricsEvenFontFace, lyricFamily);
            }
      if (fLyricSize > 0.001) {
            score->style().set(StyleIdx::lyricsOddFontSize, fLyricSize);
            score->style().set(StyleIdx::lyricsEvenFontSize, fLyricSize);
            }
      }

//---------------------------------------------------------
//   defaults
//---------------------------------------------------------

/**
 Parse the /score-partwise/defaults node:
 read the general score layout settings.
 */

void MusicXMLParserPass1::defaults(int& pageWidth, int& pageHeight)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "defaults");
      logDebugTrace("MusicXMLParserPass1::defaults");

      double millimeter = _score->spatium()/10.0;
      double tenths = 1.0;
      QString lyricFontFamily;
      QString lyricFontSize;
      QString wordFontFamily;
      QString wordFontSize;

      while (_e.readNextStartElement()) {
            if (_e.name() == "scaling") {
                  while (_e.readNextStartElement()) {
                        if (_e.name() == "millimeters")
                              millimeter = _e.readElementText().toDouble();
                        else if (_e.name() == "tenths")
                              tenths = _e.readElementText().toDouble();
                        else
                              skipLogCurrElem();
                        }
                  double _spatium = DPMM * (millimeter * 10.0 / tenths);
                  if (preferences.musicxmlImportLayout)
                        _score->setSpatium(_spatium);
                  }
            else if (_e.name() == "page-layout") {
                  PageFormat pf;
                  pageLayout(pf, millimeter / (tenths * INCH), pageWidth, pageHeight);
//TODO:ws                  if (preferences.musicxmlImportLayout)
//                        _score->setPageFormat(pf);
                  }
            else if (_e.name() == "system-layout") {
                  while (_e.readNextStartElement()) {
                        if (_e.name() == "system-margins")
                              skipLogCurrElem();
                        else if (_e.name() == "system-distance") {
                              Spatium val(_e.readElementText().toDouble() / 10.0);
                              if (preferences.musicxmlImportLayout) {
                                    _score->style().set(StyleIdx::minSystemDistance, val);
                                    qDebug("system distance %f", val.val());
                                    }
                              }
                        else if (_e.name() == "top-system-distance")
                              skipLogCurrElem();
                        else
                              skipLogCurrElem();
                        }
                  }
            else if (_e.name() == "staff-layout") {
                  while (_e.readNextStartElement()) {
                        if (_e.name() == "staff-distance") {
                              Spatium val(_e.readElementText().toDouble() / 10.0);
                              if (preferences.musicxmlImportLayout)
                                    _score->style().set(StyleIdx::staffDistance, val);
                              }
                        else
                              skipLogCurrElem();
                        }
                  }
            else if (_e.name() == "music-font")
                  skipLogCurrElem();
            else if (_e.name() == "word-font") {
                  wordFontFamily = _e.attributes().value("font-family").toString();
                  wordFontSize = _e.attributes().value("font-size").toString();
                  _e.skipCurrentElement();
                  }
            else if (_e.name() == "lyric-font") {
                  lyricFontFamily = _e.attributes().value("font-family").toString();
                  lyricFontSize = _e.attributes().value("font-size").toString();
                  _e.skipCurrentElement();
                  }
            else if (_e.name() == "appearance")
                  skipLogCurrElem();
            else if (_e.name() == "lyric-language")
                  skipLogCurrElem();
            else
                  skipLogCurrElem();
            }

      /*
       qDebug("word font family '%s' size '%s' lyric font family '%s' size '%s'",
       qPrintable(wordFontFamily), qPrintable(wordFontSize),
       qPrintable(lyricFontFamily), qPrintable(lyricFontSize));
       */
      updateStyles(_score, wordFontFamily, wordFontSize, lyricFontFamily, lyricFontSize);

      _score->setDefaultsRead(true); // TODO only if actually succeeded ?
      }

//---------------------------------------------------------
//   pageLayout
//---------------------------------------------------------

/**
 Parse the /score-partwise/defaults/page-layout node:
 read the page layout.
 */

void MusicXMLParserPass1::pageLayout(PageFormat& pf, const qreal conversion,
                                     int& pageWidth, int& pageHeight)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "page-layout");
      logDebugTrace("MusicXMLParserPass1::pageLayout");

      qreal _oddRightMargin  = 0.0;
      qreal _evenRightMargin = 0.0;
      QSizeF size;

      while (_e.readNextStartElement()) {
            if (_e.name() == "page-margins") {
                  QString type = _e.attributes().value("type").toString();
                  if (type == "")
                        type = "both";
                  qreal lm = 0.0, rm = 0.0, tm = 0.0, bm = 0.0;
                  while (_e.readNextStartElement()) {
                        if (_e.name() == "left-margin")
                              lm = _e.readElementText().toDouble() * conversion;
                        else if (_e.name() == "right-margin")
                              rm = _e.readElementText().toDouble() * conversion;
                        else if (_e.name() == "top-margin")
                              tm = _e.readElementText().toDouble() * conversion;
                        else if (_e.name() == "bottom-margin")
                              bm = _e.readElementText().toDouble() * conversion;
                        else
                              skipLogCurrElem();
                        }
                  pf.twosided = type == "odd" || type == "even";
                  if (type == "odd" || type == "both") {
                        pf.oddLeftMargin = lm;
                        _oddRightMargin = rm;
                        pf.oddTopMargin = tm;
                        pf.oddBottomMargin = bm;
                        }
                  if (type == "even" || type == "both") {
                        pf.evenLeftMargin = lm;
                        _evenRightMargin = rm;
                        pf.evenTopMargin = tm;
                        pf.evenBottomMargin = bm;
                        }
                  }
            else if (_e.name() == "page-height") {
                  double val = _e.readElementText().toDouble();
                  size.rheight() = val * conversion;
                  // set pageHeight and pageWidth for use by doCredits()
                  pageHeight = static_cast<int>(val + 0.5);
                  }
            else if (_e.name() == "page-width") {
                  double val = _e.readElementText().toDouble();
                  size.rwidth() = val * conversion;
                  // set pageHeight and pageWidth for use by doCredits()
                  pageWidth = static_cast<int>(val + 0.5);
                  }
            else
                  skipLogCurrElem();
            }
      pf.size = size;
      qreal w1 = size.width() - pf.oddLeftMargin - _oddRightMargin;
      qreal w2 = size.width() - pf.evenLeftMargin - _evenRightMargin;
      pf.printableWidth = qMax(w1, w2);   // silently adjust right margins
      }

//---------------------------------------------------------
//   partList
//---------------------------------------------------------

/**
 Parse the /score-partwise/part-list:
 create the parts and for each part set id and name.
 Also handle the part-groups.
 */

void MusicXMLParserPass1::partList(MusicXmlPartGroupList& partGroupList)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "part-list");
      logDebugTrace("MusicXMLParserPass1::partList");

      int scoreParts = 0; // number of score-parts read sofar
      MusicXmlPartGroupMap partGroups;

      while (_e.readNextStartElement()) {
            if (_e.name() == "part-group")
                  partGroup(scoreParts, partGroupList, partGroups);
            else if (_e.name() == "score-part") {
                  scorePart();
                  scoreParts++;
                  }
            else
                  skipLogCurrElem();
            }
      }

//---------------------------------------------------------
//   createPart
//---------------------------------------------------------

/**
 Create the part, set its \a id and insert it in PartMap \a pm.
 Part name (if any) will be set later.
 */

static void createPart(Score* score, const QString& id, PartMap& pm)
      {
      Part* part = new Part(score);
      pm.insert(id, part);
      part->setId(id);
      score->appendPart(part);
      Staff* staff = new Staff(score);
      staff->setPart(part);
      part->staves()->push_back(staff);
      score->staves().push_back(staff);
      // TODO TBD tuplets.resize(VOICES); // part now contains one staff, thus VOICES voices
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
      qDebug("partGroupStart number=%d part=%d symbol=%s", n, p, s.toLatin1().data());

      if (pgs.count(n) > 0) {
            qDebug("part-group number=%d already active", n);
            return;
            }

      BracketType bracketType = BracketType::NO_BRACKET;
      if (s == "")
            ;        // ignore (handle as NO_BRACKET)
      else if (s == "none")
            ;        // already set to NO_BRACKET
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
      //qDebug("partgroupstop number=%d start=%d span=%d type=%hhd",
      //       n, pgs[n]->start, pgs[n]->span, pgs[n]->type);
      pgl.push_back(pgs[n]);
      pgs.erase(n);
      }

//---------------------------------------------------------
//   partGroup
//---------------------------------------------------------

/**
 Parse the /score-partwise/part-list/part-group node.
 */

void MusicXMLParserPass1::partGroup(const int scoreParts,
                                    MusicXmlPartGroupList& partGroupList,
                                    MusicXmlPartGroupMap& partGroups)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "part-group");
      logDebugTrace("MusicXMLParserPass1::partGroup");
      bool barlineSpan = true;
      int number = _e.attributes().value("number").toInt();
      if (number > 0) number--;
      QString symbol = "";
      QString type = _e.attributes().value("type").toString();

      while (_e.readNextStartElement()) {
            if (_e.name() == "group-symbol")
                  symbol = _e.readElementText();
            else if (_e.name() == "group-barline") {
                  if (_e.readElementText() == "no")
                        barlineSpan = false;
                  }
            else
                  skipLogCurrElem();
            }

      if (type == "start")
            partGroupStart(partGroups, number, scoreParts, symbol, barlineSpan);
      else if (type == "stop")
            partGroupStop(partGroups, number, scoreParts, partGroupList);
      else
            qDebug("MusicXMLParserPass1::partGroup: part-group type '%s' not supported",
                   qPrintable(type));  // TODO
      }

//---------------------------------------------------------
//   scorePart
//---------------------------------------------------------

/**
 Parse the /score-partwise/part-list/score-part node:
 create the part and sets id and name.
 Note that a part is created even if no part-name is present
 which is invalid MusicXML but is (sometimes ?) generated by NWC2MusicXML.
 */

void MusicXMLParserPass1::scorePart()
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "score-part");
      logDebugTrace("MusicXMLParserPass1::scorePart");
      QString id = _e.attributes().value("id").toString();

      if (_parts.contains(id)) {
            logError(QString("duplicate part id '%1'").arg(id));
            skipLogCurrElem();
            return;
            }
      else {
            _parts.insert(id, MusicXmlPart(id));
            _drumsets.insert(id, MusicXMLDrumset());
            createPart(_score, id, _partMap);
            }

      while (_e.readNextStartElement()) {
            if (_e.name() == "part-name") {
                  // Element part-name contains the displayed (full) part name
                  // It is displayed by default, but can be suppressed (print-object=”no”)
                  // As of MusicXML 3.0, formatting is deprecated, with part-name in plain text
                  // and the formatted version in the part-name-display element
                  bool doLong = !(_e.attributes().value("print-object") == "no");
                  QString name = _e.readElementText();
                  _partMap[id]->setPartName(name);
                  if (doLong)
                        _partMap[id]->setLongName(name);
                  }
            else if (_e.name() == "part-name-display") {
                  // TODO
                  _e.skipCurrentElement(); // skip but don't log
                  }
            else if (_e.name() == "part-abbreviation") {
                  // Element part-name contains the displayed (abbreviated) part name
                  // It is displayed by default, but can be suppressed (print-object=”no”)
                  // As of MusicXML 3.0, formatting is deprecated, with part-name in plain text
                  // and the formatted version in the part-abbreviation-display element
                  QString name = _e.readElementText();
                  if (!(_e.attributes().value("print-object") == "no"))
                        _partMap[id]->setPlainShortName(name);
                  }
            else if (_e.name() == "part-abbreviation-display")
                  _e.skipCurrentElement();  // skip but don't log
            else if (_e.name() == "score-instrument")
                  scoreInstrument(id);
            else if (_e.name() == "midi-device") {
                  if (!_e.attributes().hasAttribute("port")) {
                        _e.readElementText(); // empty string
                        continue;
                        }
                  QString instrId = _e.attributes().value("id").toString();
                  QString port = _e.attributes().value("port").toString();
                  // If instrId is missing, the device assignment affects all
                  // score-instrument elements in the score-part
                  if (instrId.isEmpty()) {
                        for (auto it = _drumsets[id].cbegin(); it != _drumsets[id].cend(); ++it)
                              _drumsets[id][it.key()].midiPort = port.toInt() - 1;
                        }
                  else if (_drumsets[id].contains(instrId))
                        _drumsets[id][instrId].midiPort = port.toInt() - 1;

                  _e.readElementText(); // empty string
                  }
            else if (_e.name() == "midi-instrument")
                  midiInstrument(id);
            else
                  skipLogCurrElem();
            }
      Q_ASSERT(_e.isEndElement() && _e.name() == "score-part");
      }

//---------------------------------------------------------
//   scoreInstrument
//---------------------------------------------------------

/**
 Parse the /score-partwise/part-list/score-part/score-instrument node.
 */

void MusicXMLParserPass1::scoreInstrument(const QString& partId)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "score-instrument");
      logDebugTrace("MusicXMLParserPass1::scoreInstrument");
      QString instrId = _e.attributes().value("id").toString();

      while (_e.readNextStartElement()) {
            if (_e.name() == "ensemble")
                  skipLogCurrElem();
            else if (_e.name() == "instrument-name") {
                  QString instrName = _e.readElementText();
                  /*
                  qDebug("partId '%s' instrId '%s' instrName '%s'",
                         qPrintable(partId),
                         qPrintable(instrId),
                         qPrintable(instrName)
                         );
                   */
                  _drumsets[partId].insert(instrId, MusicXMLDrumInstrument(instrName));
                  // Element instrument-name is typically not displayed in the score,
                  // but used only internally
                  if (_drumsets[partId].contains(instrId))
                        _drumsets[partId][instrId].name = instrName;
                  // try to prevent an empty track name
                  if (_partMap[partId]->partName() == "")
                        _partMap[partId]->setPartName(instrName);
                  }
            else
                  skipLogCurrElem();
            }
      Q_ASSERT(_e.isEndElement() && _e.name() == "score-instrument");
      }

//---------------------------------------------------------
//   midiInstrument
//---------------------------------------------------------

/**
 Parse the /score-partwise/part-list/score-part/midi-instrument node.
 */

void MusicXMLParserPass1::midiInstrument(const QString& partId)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "midi-instrument");
      logDebugTrace("MusicXMLParserPass1::midiInstrument");
      QString instrId = _e.attributes().value("id").toString();

      while (_e.readNextStartElement()) {
            if (_e.name() == "midi-bank")
                  skipLogCurrElem();
            else if (_e.name() == "midi-channel") {
                  int channel = _e.readElementText().toInt();
                  if (channel < 1) {
                        logError(QString("MusicXml::xmlScorePart: incorrect midi-channel: %1").arg(channel));
                        channel = 1;
                        }
                  else if (channel > 16) {
                        logError(QString("MusicXml::xmlScorePart: incorrect midi-channel: %1").arg(channel));
                        channel = 16;
                        }
                  if (_drumsets[partId].contains(instrId))
                        _drumsets[partId][instrId].midiChannel = channel - 1;
                  }
            else if (_e.name() == "midi-program") {
                  int program = _e.readElementText().toInt();
                  // Bug fix for Cubase 6.5.5 which generates <midi-program>0</midi-program>
                  // Check program number range
                  if (program < 1) {
                        logError(QString("MusicXml::xmlScorePart: incorrect midi-program: %1").arg(program));
                        program = 1;
                        }
                  else if (program > 128) {
                        logError(QString("MusicXml::xmlScorePart: incorrect midi-program: %1").arg(program));
                        program = 128;
                        }
                  if (_drumsets[partId].contains(instrId))
                        _drumsets[partId][instrId].midiProgram = program - 1;
                  }
            else if (_e.name() == "midi-unpitched") {
                  if (_drumsets[partId].contains(instrId))
                        _drumsets[partId][instrId].pitch = _e.readElementText().toInt() - 1;
                  }
            else if (_e.name() == "volume") {
                  double vol = _e.readElementText().toDouble();
                  if (vol >= 0 && vol <= 100) {
                        if (_drumsets[partId].contains(instrId))
                              _drumsets[partId][instrId].midiVolume = static_cast<int>((vol / 100) * 127);
                        }
                  else
                        logError(QString("MusicXml::xmlScorePart: incorrect midi-volume: %1").arg(vol));
                  }
            else if (_e.name() == "pan") {
                  double pan = _e.readElementText().toDouble();
                  if (pan >= -90 && pan <= 90) {
                        if (_drumsets[partId].contains(instrId))
                              _drumsets[partId][instrId].midiPan = static_cast<int>(((pan + 90) / 180) * 127);
                        }
                  else
                        logError(QString("MusicXml::xmlScorePart: incorrect midi-volume: %g1").arg(pan));
                  }
            else
                  skipLogCurrElem();
            }
      Q_ASSERT(_e.isEndElement() && _e.name() == "midi-instrument");
      }

//---------------------------------------------------------
//   part
//---------------------------------------------------------

/**
 Parse the /score-partwise/part node:
 read the parts data to determine measure timing and octave shifts.
 Assign voices and staves.
 */

void MusicXMLParserPass1::part()
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "part");
      logDebugTrace("MusicXMLParserPass1::part");
      const QString id = _e.attributes().value("id").toString();

      if (!_parts.contains(id)) {
            logError(QString("MusicXMLParserPass1::part cannot find part '%1'").arg(id));
            skipLogCurrElem();
            }

      initPartState(id);

      VoiceOverlapDetector vod;
      Fraction time;  // current time within part
      Fraction mdur;  // measure duration

      while (_e.readNextStartElement()) {
            if (_e.name() == "measure") {
                  measure(id, time, mdur, vod);
                  time += mdur;
                  }
            else
                  skipLogCurrElem();
            }

      // allocate MuseScore staff to MusicXML voices
      allocateStaves(_parts[id].voicelist);
      // allocate MuseScore voice to MusicXML voices
      allocateVoices(_parts[id].voicelist);
      // calculate the octave shifts
      _parts[id].calcOctaveShifts();
      // set first instrument for multi-instrument part starting with rest
      if (_firstInstrId != "" && _firstInstrSTime > Fraction(0, 1))
            _parts[id]._instrList.setInstrument(_firstInstrId, Fraction(0, 1));

      // debug: print results
      /*
      qDebug("voiceMapperStats: new staff");
      VoiceList& vl = _parts[id].voicelist;
      for (auto i = vl.constBegin(); i != vl.constEnd(); ++i) {
            qDebug("voiceMapperStats: voice %s staff data %s",
                   qPrintable(i.key()), qPrintable(i.value().toString()));
            }
      */
      }

//---------------------------------------------------------
//   measureDurationAsFraction
//---------------------------------------------------------

/**
 Determine a suitable measure duration value given the time signature
 by setting the duration denominator to be greater than or equal
 to the time signature denominator
 */

static Fraction measureDurationAsFraction(const Fraction length, const int tsigtype)
      {
      if (tsigtype <= 0)
            // invalid tsigtype
            return length;

      Fraction res = length;
      while (res.denominator() < tsigtype) {
            res.setNumerator(res.numerator() * 2);
            res.setDenominator(res.denominator() * 2);
            }
      return res;
      }

//---------------------------------------------------------
//   measure
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure node:
 read the measures data as required to determine measure timing, octave shifts
 and assign voices and staves.
 */

void MusicXMLParserPass1::measure(const QString& partId,
                                  const Fraction time,
                                  Fraction& mdur,
                                  VoiceOverlapDetector& vod)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "measure");
      logDebugTrace("MusicXMLParserPass1::measure");
      QString number = _e.attributes().value("number").toString();

      Fraction mTime; // current time stamp within measure
      Fraction mDura; // current total measure duration
      vod.newMeasure();

      while (_e.readNextStartElement()) {
            if (_e.name() == "attributes")
                  attributes(partId);
            else if (_e.name() == "note") {
                  Fraction dura;
                  // note: chord and grace note handling done in note()
                  note(partId, time + mTime, dura, vod);
                  if (dura.isValid()) {
                        mTime += dura;
                        if (mTime > mDura)
                              mDura = mTime;
                        }
                  }
            else if (_e.name() == "forward") {
                  Fraction dura;
                  forward(dura);
                  if (dura.isValid()) {
                        mTime += dura;
                        if (mTime > mDura)
                              mDura = mTime;
                        }
                  }
            else if (_e.name() == "backup") {
                  Fraction dura;
                  backup(dura);
                  if (dura.isValid()) {
                        if (dura <= mTime)
                              mTime -= dura;
                        else {
                              logError("backup beyond measure start");
                              mTime.set(0, 1);
                              }
                        }
                  }
            else if (_e.name() == "direction")
                  direction(partId, time + mTime);
            else
                  skipLogCurrElem();

            /*
             qDebug("mTime %s (%s) mDura %s (%s)",
             qPrintable(mTime.print()),
             qPrintable(mTime.reduced().print()),
             qPrintable(mDura.print()),
             qPrintable(mDura.reduced().print()));
             */
            }

      // debug vod
      // vod.dump();
      // copy overlap data from vod to voicelist
      copyOverlapData(vod, _parts[partId].voicelist);

      // measure duration fixups
      mDura.reduce();

      // fix for PDFtoMusic Pro v1.3.0d Build BF4E (which sometimes generates empty measures)
      // if no valid length found and length according to time signature is known,
      // use length according to time signature
      if (mDura.isZero() && _timeSigDura.isValid() && _timeSigDura > Fraction(0, 1))
            mDura = _timeSigDura;

      // if necessary, round up to an integral number of 1/64s,
      // to comply with MuseScores actual measure length constraints
      // TODO: calculate in fraction
      int length = mDura.ticks();
      int correctedLength = length;
      if ((length % (MScore::division/16)) != 0) {
            correctedLength = ((length / (MScore::division/16)) + 1) * (MScore::division/16);
            mDura = Fraction::fromTicks(correctedLength);
            }

      // set measure duration to a suitable value given the time signature
      if (_timeSigDura.isValid() && _timeSigDura > Fraction(0, 1)) {
            int btp = _timeSigDura.denominator();
            if (btp > 0)
                  mDura = measureDurationAsFraction(mDura, btp);
            }

      // set return value(s)
      mdur = mDura;

      // set measure number and duration
      /*
      qDebug("part %s measure %s dura %s (%d)",
             qPrintable(partId), qPrintable(number), qPrintable(mdur.print()), mdur.ticks());
       */
      _parts[partId].addMeasureNumberAndDuration(number, mdur);
      }

//---------------------------------------------------------
//   attributes
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes node.
 */

void MusicXMLParserPass1::attributes(const QString& partId)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "attributes");
      logDebugTrace("MusicXMLParserPass1::attributes");

      while (_e.readNextStartElement()) {
            if (_e.name() == "clef")
                  clef(partId);
            else if (_e.name() == "divisions")
                  divisions();
            else if (_e.name() == "staff-details")
                  staffDetails(partId);
            else if (_e.name() == "staves")
                  staves(partId);
            else if (_e.name() == "time")
                  time();
            else
                  skipLogCurrElem();
            }
      }

//---------------------------------------------------------
//   clef
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes/clef node.
 Set the staff type based on clef type
 TODO: check if staff type setting could be simplified
 */

void MusicXMLParserPass1::clef(const QString& partId)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "clef");
      logDebugTrace("MusicXMLParserPass1::clef");

      QString number = _e.attributes().value("number").toString();
      int n = 0;
      if (number != "") {
            n = number.toInt();
            if (n <= 0) {
                  logError(QString("invalid number %1").arg(number));
                  n = 0;
                  }
            else
                  n--;              // make zero-based
            }

      StaffTypes staffType = StaffTypes::STANDARD;

      while (_e.readNextStartElement()) {
            if (_e.name() == "sign") {
                  QString sign = _e.readElementText();
                  if (sign == "TAB")
                        staffType = StaffTypes::TAB_DEFAULT;
                  else if (sign == "percussion")
                        staffType = StaffTypes::PERC_DEFAULT;
                  }
            else
                  skipLogCurrElem();
            }

      Part* part = getPart(partId);
      Q_ASSERT(part);
      int staves = part->nstaves();
      int staffIdx = _score->staffIdx(part);

      // TODO: changed for #55501, but now staff type init is shared between pass 1 and 2
      // old code: if (0 <= n && n < staves && staffType != StaffTypes::STANDARD)
      if (0 <= n && n < staves && staffType == StaffTypes::TAB_DEFAULT)
            _score->staff(staffIdx + n)->setStaffType(0, StaffType::preset(staffType));
      }

//---------------------------------------------------------
//   determineTimeSig
//---------------------------------------------------------

/**
 Determine the time signature based on \a beats, \a beatType and \a timeSymbol.
 Sets return parameters \a st, \a bts, \a btp.
 Return true if OK, false on error.
 */

// TODO: share between pass 1 and pass 2

static bool determineTimeSig(const QString beats, const QString beatType, const QString timeSymbol,
                             TimeSigType& st, int& bts, int& btp)
      {
      // initialize
      st  = TimeSigType::NORMAL;
      bts = 0;             // the beats (max 4 separated by "+") as integer
      btp = 0;             // beat-type as integer
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
                  qDebug("determineTimeSig: time symbol <%s> not recognized with beats=%s and beat-type=%s",
                         qPrintable(timeSymbol), qPrintable(beats), qPrintable(beatType));       // TODO
                  return false;
                  }

            btp = beatType.toInt();
            QStringList list = beats.split("+");
            for (int i = 0; i < list.size(); i++)
                  bts += list.at(i).toInt();
            }

      // determine if bts and btp are valid
      if (bts <= 0 || btp <=0) {
            qDebug("determineTimeSig: beats=%s and/or beat-type=%s not recognized",
                   qPrintable(beats), qPrintable(beatType));         // TODO
            return false;
            }

      return true;
      }

//---------------------------------------------------------
//   time
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes/time node.
 */

void MusicXMLParserPass1::time()
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "time");

      QString beats;
      QString beatType;
      QString timeSymbol = _e.attributes().value("symbol").toString();

      while (_e.readNextStartElement()) {
            if (_e.name() == "beats")
                  beats = _e.readElementText();
            else if (_e.name() == "beat-type")
                  beatType = _e.readElementText();
            else
                  skipLogCurrElem();
            }

      if (beats != "" && beatType != "") {
            // determine if timesig is valid
            TimeSigType st  = TimeSigType::NORMAL;
            int bts = 0;       // total beats as integer (beats may contain multiple numbers, separated by "+")
            int btp = 0;       // beat-type as integer
            if (determineTimeSig(beats, beatType, timeSymbol, st, bts, btp)) {
                  _timeSigDura = Fraction(bts, btp);
                  }
            }
      }

//---------------------------------------------------------
//   divisions
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes/divisions node.
 */

void MusicXMLParserPass1::divisions()
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "divisions");

      _divs = _e.readElementText().toInt();
      if (!(_divs > 0))
            logError("illegal divisions");
      }

//---------------------------------------------------------
//   setStaffLines
//---------------------------------------------------------

/**
 Set stafflines and barline span for a single staff
 */

static void setStaffLines(Score* score, int staffIdx, int stafflines)
      {
      score->staff(staffIdx)->setLines(0, stafflines);
      score->staff(staffIdx)->setBarLineTo(0);  // default
      }

//---------------------------------------------------------
//   staffDetails
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes/staff-details node.
 */

void MusicXMLParserPass1::staffDetails(const QString& partId)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "staff-details");
      logDebugTrace("MusicXMLParserPass1::staffDetails");

      Part* part = getPart(partId);
      Q_ASSERT(part);
      int staves = part->nstaves();

      QString number = _e.attributes().value("number").toString();
      int n = 1; // default
      if (number != "") {
            n = number.toInt();
            if (n <= 0 || n > staves) {
                  logError(QString("invalid staff-details number %1").arg(number));
                  n = 1;
                  }
            }
      n--;        // make zero-based

      int staffIdx = _score->staffIdx(part) + n;

      StringData* t = 0;
      if (_score->staff(staffIdx)->isTabStaff(0)) {
            t = new StringData;
            t->setFrets(25);       // sensible default
            }

      int staffLines = 0;
      while (_e.readNextStartElement()) {
            if (_e.name() == "staff-lines") {
                  // save staff lines for later
                  staffLines = _e.readElementText().toInt();
                  // for a TAB staff also resize the string table and init with zeroes
                  if (t) {
                        if (0 < staffLines)
                              t->stringList() = QVector<instrString>(staffLines).toList();
                        else
                              logError(QString("illegal staff-lines %1").arg(staffLines));
                        }
                  }
            else if (_e.name() == "staff-tuning")
                  staffTuning(t);
            else
                  skipLogCurrElem();
            }

      if (staffLines > 0) {
            setStaffLines(_score, staffIdx, staffLines);
            }

      if (t) {
            Instrument* i = part->instrument();
            i->setStringData(*t);
            }
      }

//---------------------------------------------------------
//   MusicXMLStepAltOct2Pitch
//---------------------------------------------------------

/**
 Convert MusicXML \a step (0=C, 1=D, etc.) / \a alter / \a octave to midi pitch.
 Note: same code is in pass 1 and in pass 2.
 TODO: combine
 */

static int MusicXMLStepAltOct2Pitch(int step, int alter, int octave)
      {
      //                       c  d  e  f  g  a   b
      static int table[7]  = { 0, 2, 4, 5, 7, 9, 11 };
      if (step < 0 || step > 6) {
            qDebug("MusicXMLStepAltOct2Pitch: illegal step %d", step);
            return -1;
            }
      int pitch = table[step] + alter + (octave+1) * 12;

      if (pitch < 0)
            pitch = -1;
      if (pitch > 127)
            pitch = -1;

      return pitch;
      }

//---------------------------------------------------------
//   staffTuning
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes/staff-details/staff-tuning node.
 */

void MusicXMLParserPass1::staffTuning(StringData* t)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "staff-tuning");
      logDebugTrace("MusicXMLParserPass1::staffTuning");

      // ignore <staff-tuning> if not a TAB staff
      if (!t) {
            logError("<staff-tuning> on non-TAB staff");
            skipLogCurrElem();
            return;
            }

      int line   = _e.attributes().value("line").toInt();
      int step   = 0;
      int alter  = 0;
      int octave = 0;
      while (_e.readNextStartElement()) {
            if (_e.name() == "tuning-alter")
                  alter = _e.readElementText().toInt();
            else if (_e.name() == "tuning-octave")
                  octave = _e.readElementText().toInt();
            else if (_e.name() == "tuning-step") {
                  QString strStep = _e.readElementText();
                  int pos = QString("CDEFGAB").indexOf(strStep);
                  if (strStep.size() == 1 && pos >=0 && pos < 7)
                        step = pos;
                  else
                        logError(QString("invalid step '%1'").arg(strStep));
                  }
            else
                  skipLogCurrElem();
            }

      if (0 < line && line <= t->stringList().size()) {
            int pitch = MusicXMLStepAltOct2Pitch(step, alter, octave);
            if (pitch >= 0)
                  t->stringList()[line - 1].pitch = pitch;
            else
                  logError(QString("invalid string %1 tuning step/alter/oct %2/%3/%4")
                           .arg(line).arg(step).arg(alter).arg(octave));
            }

      }

//---------------------------------------------------------
//   staves
//---------------------------------------------------------

/**
 Set number of staves for part \a partId to the max value of the current value
 and the value in the <staves> element.
 */

void MusicXMLParserPass1::staves(const QString& partId)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "staves");
      logDebugTrace("MusicXMLParserPass1::staves");

      int staves = _e.readElementText().toInt();
      if (!(staves > 0 && staves <= MAX_STAVES)) {
            logError("illegal staves");
            return;
            }

      Part* part = _partMap.value(partId);
      Q_ASSERT(part);
      if (staves > part->nstaves())
            part->setStaves(staves);
      }

//---------------------------------------------------------
//   direction
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/direction node
 to be able to handle octave-shifts, as these must be interpreted
 in musical order instead of in MusicXML file order.
 */

void MusicXMLParserPass1::direction(const QString& partId, const Fraction cTime)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "direction");

      // note: file order is direction-type first, then staff
      // this means staff is still unknown when direction-type is handled

      QList<MxmlOctaveShiftDesc> starts;
      QList<MxmlOctaveShiftDesc> stops;
      int staff = 0;

      while (_e.readNextStartElement()) {
            if (_e.name() == "direction-type")
                  directionType(cTime, starts, stops);
            else if (_e.name() == "staff") {
                  int nstaves = getPart(partId)->nstaves();
                  QString strStaff = _e.readElementText();
                  staff = strStaff.toInt() - 1;
                  if (0 <= staff && staff < nstaves)
                        ;  //qDebug("direction staff %d", staff + 1);
                  else {
                        logError(QString("invalid staff %1").arg(strStaff));
                        staff = 0;
                        }
                  }
            else
                  _e.skipCurrentElement();
            }

      // handle the stops first
      foreach (auto desc, stops) {
            if (_octaveShifts.contains(desc.num)) {
                  MxmlOctaveShiftDesc prevDesc = _octaveShifts.value(desc.num);
                  if (prevDesc.tp == MxmlOctaveShiftDesc::Type::UP
                      || prevDesc.tp == MxmlOctaveShiftDesc::Type::DOWN) {
                        // a complete pair
                        qDebug("octave-shift start %s delta %d",
                               qPrintable(prevDesc.time.print()), prevDesc.size);
                        _parts[partId].addOctaveShift(staff, prevDesc.size, prevDesc.time);
                        qDebug("octave-shift stop %s delta %d",
                               qPrintable(desc.time.print()), -prevDesc.size);
                        _parts[partId].addOctaveShift(staff, -prevDesc.size, desc.time);
                        }
                  else
                        logError("double octave-shift stop");
                  _octaveShifts.remove(desc.num);
                  }
            else
                  _octaveShifts.insert(desc.num, desc);
            }

      // then handle the starts
      foreach (auto desc, starts) {
            if (_octaveShifts.contains(desc.num)) {
                  MxmlOctaveShiftDesc prevDesc = _octaveShifts.value(desc.num);
                  if (prevDesc.tp == MxmlOctaveShiftDesc::Type::STOP) {
                        // a complete pair
                        qDebug("octave-shift start %s delta %d",
                               qPrintable(desc.time.print()), desc.size);
                        _parts[partId].addOctaveShift(staff, desc.size, desc.time);
                        qDebug("octave-shift stop %s delta %d",
                               qPrintable(prevDesc.time.print()), -desc.size);
                        _parts[partId].addOctaveShift(staff, -desc.size, prevDesc.time);
                        }
                  else
                        logError("double octave-shift start");
                  _octaveShifts.remove(desc.num);
                  }
            else
                  _octaveShifts.insert(desc.num, desc);
            }
      }

//---------------------------------------------------------
//   directionType
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/direction/direction-type node.
 */

void MusicXMLParserPass1::directionType(const Fraction cTime,
                                        QList<MxmlOctaveShiftDesc>& starts,
                                        QList<MxmlOctaveShiftDesc>& stops)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "direction-type");

      while (_e.readNextStartElement()) {
            if (_e.name() == "octave-shift") {
                  QString number = _e.attributes().value("number").toString();
                  int n = 0;
                  if (number != "") {
                        n = number.toInt();
                        if (n <= 0)
                              logError(QString("invalid number %1").arg(number));
                        else
                              n--;  // make zero-based
                        }

                  if (0 <= n && n < MAX_NUMBER_LEVEL) {
                        short size = _e.attributes().value("size").toShort();
                        QString type = _e.attributes().value("type").toString();
                        qDebug("octave-shift type '%s' size %d number %d", qPrintable(type), size, n);
                        MxmlOctaveShiftDesc osDesc;
                        handleOctaveShift(cTime, type, size, osDesc);
                        osDesc.num = n;
                        if (osDesc.tp == MxmlOctaveShiftDesc::Type::UP
                            || osDesc.tp == MxmlOctaveShiftDesc::Type::DOWN)
                              starts.append(osDesc);
                        else if (osDesc.tp == MxmlOctaveShiftDesc::Type::STOP)
                              stops.append(osDesc);
                        }
                  else {
                        logError(QString("invalid octave-shift number %1").arg(number));
                        }
                  _e.skipCurrentElement();
                  }
            else
                  _e.skipCurrentElement();
            }

      Q_ASSERT(_e.isEndElement() && _e.name() == "direction-type");
      }

//---------------------------------------------------------
//   handleOctaveShift
//---------------------------------------------------------

void MusicXMLParserPass1::handleOctaveShift(const Fraction cTime,
                                            const QString& type, short size,
                                            MxmlOctaveShiftDesc& desc)
      {
      MxmlOctaveShiftDesc::Type tp = MxmlOctaveShiftDesc::Type::NONE;
      short sz = 0;

      switch (size) {
            case   8: sz =  1; break;
            case  15: sz =  2; break;
            default:
                  logError(QString("invalid octave-shift size %1").arg(size));
                  return;
            }

      if (!cTime.isValid() || cTime < Fraction(0, 1))
            logError("invalid current time");

      if (type == "up")
            tp = MxmlOctaveShiftDesc::Type::UP;
      else if (type == "down") {
            tp = MxmlOctaveShiftDesc::Type::DOWN;
            sz *= -1;
            }
      else if (type == "stop")
            tp = MxmlOctaveShiftDesc::Type::STOP;
      else {
            logError(QString("invalid octave-shift type '%1'").arg(type));
            return;
            }

      desc = MxmlOctaveShiftDesc(tp, sz, cTime);
      }

//---------------------------------------------------------
//   setFirstInstr
//---------------------------------------------------------

void MusicXMLParserPass1::setFirstInstr(const QString& id, const Fraction stime)
      {
      // check for valid arguments
      if (id == "" || !stime.isValid() || stime < Fraction(0, 1))
            return;

      // check for no instrument found yet or new earliest start time
      // note: compare using <= to catch instrument at t=0
      if (_firstInstrId == "" || stime <= _firstInstrSTime) {
            _firstInstrId = id;
            _firstInstrSTime = stime;
            }
      }

//---------------------------------------------------------
//   note
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note node.
 */

void MusicXMLParserPass1::note(const QString& partId,
                               const Fraction sTime,
                               Fraction& dura,
                               VoiceOverlapDetector& vod)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "note");
      //logDebugTrace("MusicXMLParserPass1::note");

      if (_e.attributes().value("print-spacing") == "no") {
            notePrintSpacingNo(dura);
            return;
            }

      //float alter = 0;
      bool chord = false;
      int dots = 0;
      bool grace = false;
      //int octave = -1;
      bool bRest = false;
      int staff = 1;
      //int step = 0;
      Fraction timeMod(1, 1);
      QString type;
      QString voice = "1";
      QString instrId;

      while (_e.readNextStartElement()) {
            if (_e.name() == "chord") {
                  chord = true;
                  _e.readNext();
                  }
            else if (_e.name() == "dot") {
                  dots++;
                  _e.readNext();
                  }
            else if (_e.name() == "duration")
                  duration(dura);
            else if (_e.name() == "grace") {
                  grace = true;
                  _e.readNext();
                  }
            else if (_e.name() == "instrument") {
                  instrId = _e.attributes().value("id").toString();
                  _e.readNext();
                  }
            else if (_e.name() == "pitch")
                  _e.skipCurrentElement();  // skip but don't log
            else if (_e.name() == "rest") {
                  bRest = true;
                  rest();
                  }
            else if (_e.name() == "staff") {
                  QString strStaff = _e.readElementText();
                  staff = strStaff.toInt();
                  // Bug fix for Cubase 6.5.5 which generates <staff>2</staff> in a single staff part
                  // Same fix is required in pass 1 and pass 2
                  Part* part = _partMap.value(partId);
                  Q_ASSERT(part);
                  if (staff <= 0 || staff > part->nstaves()) {
                        logError(QString("illegal staff '%1'").arg(strStaff));
                        staff = 1;
                        }
                  }
            else if (_e.name() == "time-modification")
                  timeModification(timeMod);
            else if (_e.name() == "type")
                  type = _e.readElementText();
            else if (_e.name() == "voice")
                  voice = _e.readElementText();
            else
                  skipLogCurrElem();
            }

      // convert staff to zero-based
      staff--;

      // multi-instrument handling
      setFirstInstr(instrId, sTime);
      QString prevInstrId = _parts[partId]._instrList.instrument(sTime);
      bool mustInsert = instrId != prevInstrId;
      /*
      qDebug("tick %s (%d) staff %d voice '%s' previnst='%s' instrument '%s' mustInsert %d",
             qPrintable(sTime.print()),
             sTime.ticks(),
             staff + 1,
             qPrintable(voice),
             qPrintable(prevInstrId),
             qPrintable(instrId),
             mustInsert
             );
      */
      if (mustInsert)
            _parts[partId]._instrList.setInstrument(instrId, sTime);

      // normalize duration
      if (dura.isValid())
            dura.reduce();

      // timing error check(s)
      QString errorStr;
      Fraction calcDura = calculateFraction(type, dots, timeMod);
      if (dura.isValid() && calcDura.isValid()) {
            if (dura != calcDura) {
                  errorStr = "calculated duration not equal to specified duration";

                  if (bRest && type == "whole" && dura.isValid()) {
                        // Sibelius whole measure rest (not an error)
                        errorStr = "";
                        }
                  else {
                        const int maxDiff = 3; // maximum difference considered a rounding error
                        if (qAbs(calcDura.ticks() - dura.ticks()) <= maxDiff) {
                              errorStr += " -> assuming rounding error";
                              dura = calcDura;
                              }
                        }
                  }
            }
      else if (dura.isValid()) {
            // do not report an error for typeless (whole measure) rests
            if (!(bRest && type == ""))
                  errorStr = "calculated duration invalid, using specified duration";
            }
      else if (calcDura.isValid()) {
            if (!grace) {
                  errorStr = "specified duration invalid, using calculated duration";
                  dura = calcDura; // overrule dura
                  }
            }
      else {
            errorStr = "calculated and specified duration invalid, using 4/4";
            dura = Fraction(4, 4);
      }

      if (errorStr != "")
            logError(errorStr);

      // don't count chord or grace note duration
      // note that this does not check the MusicXML requirement that notes in a chord
      // cannot have a duration longer than the first note in the chord
      if (chord || grace)
            dura.set(0, 1);

      // store result
      if (dura.isValid() && dura > Fraction(0, 1)) {
            // count the chords
            if (!_parts.value(partId).voicelist.contains(voice)) {
                  VoiceDesc vs;
                  _parts[partId].voicelist.insert(voice, vs);
                  }
            _parts[partId].voicelist[voice].incrChordRests(staff);
            // determine note length for voice overlap detection
            // TODO
            vod.addNote(sTime.ticks(), (sTime + dura).ticks(), voice, staff);
            }
      }

//---------------------------------------------------------
//   notePrintSpacingNo
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note node for a note with print-spacing="no".
 These are handled like a forward: only moving the time forward.
 */

void MusicXMLParserPass1::notePrintSpacingNo(Fraction& dura)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "note");
      //logDebugTrace("MusicXMLParserPass1::notePrintSpacingNo");

      bool chord = false;
      bool grace = false;

      while (_e.readNextStartElement()) {
            if (_e.name() == "chord") {
                  chord = true;
                  _e.readNext();
                  }
            else if (_e.name() == "duration")
                  duration(dura);
            else if (_e.name() == "grace") {
                  grace = true;
                  _e.readNext();
                  }
            else
                  _e.skipCurrentElement();        // skip but don't log
            }

      // don't count chord or grace note duration
      // note that this does not check the MusicXML requirement that notes in a chord
      // cannot have a duration longer than the first note in the chord
      if (chord || grace)
            dura.set(0, 1);

      Q_ASSERT(_e.isEndElement() && _e.name() == "note");
      }

//---------------------------------------------------------
//   duration
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/duration node.
 */

void MusicXMLParserPass1::duration(Fraction& dura)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "duration");
      //logDebugTrace("MusicXMLParserPass1::duration");

      dura.set(0, 0);  // invalid unless set correctly
      int intDura = _e.readElementText().toInt();
      if (intDura > 0) {
            if (_divs > 0)
                  dura.set(intDura, 4 * _divs);
            else
                  logError("illegal or uninitialized divisions");
            }
      else
            logError("illegal duration");
      //qDebug("duration %s valid %d", qPrintable(dura.print()), dura.isValid());
      }

//---------------------------------------------------------
//   forward
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/forward node.
 */

void MusicXMLParserPass1::forward(Fraction& dura)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "forward");
      //logDebugTrace("MusicXMLParserPass1::forward");

      while (_e.readNextStartElement()) {
            if (_e.name() == "duration")
                  duration(dura);
            else if (_e.name() == "staff")
                  _e.skipCurrentElement();  // skip but don't log
            else if (_e.name() == "voice")
                  _e.skipCurrentElement();  // skip but don't log
            else
                  skipLogCurrElem();
            }
      }

//---------------------------------------------------------
//   backup
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/backup node.
 */

void MusicXMLParserPass1::backup(Fraction& dura)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "backup");
      //logDebugTrace("MusicXMLParserPass1::backup");

      while (_e.readNextStartElement()) {
            if (_e.name() == "duration")
                  duration(dura);
            else
                  skipLogCurrElem();
            }
      }

//---------------------------------------------------------
//   timeModification
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/time-modification node.
 */

void MusicXMLParserPass1::timeModification(Fraction& timeMod)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "time-modification");
      //logDebugTrace("MusicXMLParserPass1::timeModification");

      int intActual = 0;
      int intNormal = 0;
      QString strActual;
      QString strNormal;

      while (_e.readNextStartElement()) {
            if (_e.name() == "actual-notes")
                  strActual = _e.readElementText();
            else if (_e.name() == "normal-notes")
                  strNormal = _e.readElementText();
            else
                  skipLogCurrElem();
            }

      intActual = strActual.toInt();
      intNormal = strNormal.toInt();
      if (intActual > 0 && intNormal > 0)
            timeMod.set(intNormal, intActual);
      else {
            timeMod.set(1, 1);
            logError(QString("illegal time-modification: actual-notes %1 normal-notes %2")
                     .arg(strActual).arg(strNormal));
            }
      }

//---------------------------------------------------------
//   rest
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/rest node.
 */

void MusicXMLParserPass1::rest()
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "rest");
      //logDebugTrace("MusicXMLParserPass1::rest");

      while (_e.readNextStartElement()) {
            skipLogCurrElem();
            }
      }

} // namespace Ms
