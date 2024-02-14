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
#include "libmscore/chordrest.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/measure.h"
#include "libmscore/page.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/stringdata.h"
#include "libmscore/sym.h"
#include "libmscore/symbol.h"
#include "libmscore/timesig.h"
#include "libmscore/style.h"
#include "libmscore/spanner.h"
#include "libmscore/bracketItem.h"
#include "libmscore/utils.h"

#include "importmxmllogger.h"
#include "importmxmlnoteduration.h"
#include "importmxmlpass1.h"
#include "importmxmlpass2.h"

#include "mscore/preferences.h"

namespace Ms {

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
      int voicesAllocated[MAX_VOICE_DESC_STAVES]; // number of voices allocated on each staff
      for (int i = 0; i < MAX_VOICE_DESC_STAVES; ++i)
            voicesAllocated[i] = 0;

      // handle regular (non-overlapping) voices
      // note: outer loop executed vcLst.size() times, as each inner loop handles exactly one item
      for (int i = 0; i < vcLst.size(); ++i) {
            // find the regular voice containing the highest number of chords and rests that has not been handled yet
            int max = 0;
            int key = -1;
            for (VoiceList::const_iterator j = vcLst.constBegin(); j != vcLst.constEnd(); ++j) {
                  if (!j.value().overlaps() && j.value().numberChordRests() > max && j.value().staff() == -1) {
                        max = j.value().numberChordRests();
                        key = j.key();
                        }
                  }
            if (key > 0) {
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
      for (int h = 0; h < MAX_VOICE_DESC_STAVES; ++h) {
            // note: middle loop executed vcLst.size() times, as each inner loop handles exactly one item
            for (int i = 0; i < vcLst.size(); ++i) {
                  // find the overlapping voice containing the highest number of chords and rests that has not been handled yet
                  int max = 0;
                  int key = -1;
                  for (VoiceList::const_iterator j = vcLst.constBegin(); j != vcLst.constEnd(); ++j) {
                        if (j.value().overlaps() && j.value().numberChordRests(h) > max && j.value().staffAlloc(h) == -1) {
                              max = j.value().numberChordRests(h);
                              key = j.key();
                              }
                        }
                  if (key > 0) {
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
      int nextVoice[MAX_VOICE_DESC_STAVES]; // number of voices allocated on each staff
      for (int i = 0; i < MAX_VOICE_DESC_STAVES; ++i)
            nextVoice[i] = 0;
      // handle regular (non-overlapping) voices
      // a voice is allocated on one specific staff
      for (VoiceList::const_iterator i = vcLst.constBegin(); i != vcLst.constEnd(); ++i) {
            int staff = i.value().staff();
            int key   = i.key();
            if (staff >= 0) {
                  vcLst[key].setVoice(nextVoice[staff]);
                  nextVoice[staff]++;
                  }
            }
      // handle overlapping voices
      // each voice may be in every staff
      for (VoiceList::const_iterator i = vcLst.constBegin(); i != vcLst.constEnd(); ++i) {
            for (int j = 0; j < MAX_VOICE_DESC_STAVES; ++j) {
                  int staffAlloc = i.value().staffAlloc(j);
                  int key        = i.key();
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
            int key = i.key();
            if (vod.stavesOverlap(key))
                  vcLst[key].setOverlap(true);
            }
      }

//---------------------------------------------------------
//   MusicXMLParserPass1
//---------------------------------------------------------

MusicXMLParserPass1::MusicXMLParserPass1(Score* score, MxmlLogger* logger)
      : _divs(0), _score(score), _logger(logger), _hasBeamingInfo(false), _hasInferredHeaderText(false)
      {
      // nothing
      }

//---------------------------------------------------------
//   addError
//---------------------------------------------------------

void MusicXMLParserPass1::addError(const QString& error)
      {
      if (!error.isEmpty()) {
            _logger->logError(error, &_e);
            QString errorWithLocation = xmlReaderLocation(_e) + ' ' + error + '\n';
            _errors += errorWithLocation;
            }
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
//   getIntervals
//---------------------------------------------------------

/**
 Get the MusicXmlIntervalList for part \a id.
 Return an empty MusicXmlIntervalList on error.
 */

MusicXmlIntervalList MusicXMLParserPass1::getIntervals(const QString id) const
      {
      if (_parts.contains(id))
            return _parts.value(id)._intervals;
      return MusicXmlIntervalList();
      }

//---------------------------------------------------------
//   determineMeasureLength
//---------------------------------------------------------

/**
 Set default notehead, line and stem direction
 for instrument \a instrId in part \a id.
 Called from pass 2, notehead, line and stemDirection are not read in pass 1.
 */

void MusicXMLParserPass1::setDrumsetDefault(const QString& id,
                                            const QString& instrId,
                                            const NoteHead::Group hg,
                                            const int line,
                                            const Direction sd)
      {
      if (_instruments.contains(id)
          && _instruments[id].contains(instrId)) {
            _instruments[id][instrId].notehead = hg;
            _instruments[id][instrId].line = line;
            _instruments[id][instrId].stemDirection = sd;
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

bool MusicXMLParserPass1::determineStaffMoveVoice(const QString& id, const int mxStaff, const int& mxVoice,
                                                  int& msMove, int& msTrack, int& msVoice) const
      {
      VoiceList voicelist = getVoiceList(id);
      msMove = 0; // TODO
      msTrack = 0; // TODO
      msVoice = 0; // TODO


      // Musicxml voices are counted for all staves of an
      // instrument. They are not limited. In mscore voices are associated
      // with a staff. Every staff can have at most VOICES voices.

      // The following lines map musicXml voices to mscore voices.
      // If a voice crosses two staves, this is expressed with the
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
            qDebug("too many voices (staff=%d voice='%d' -> s=%d v=%d)",
                   mxStaff + 1, mxVoice, s, v);
            return false;
            }

      msMove  = mxStaff - s;
      msVoice = v;

      // make score-relative instead on part-relative
      Part* part = _partMap.value(id);
      if (!part)
            return false;
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
      if (!part)
            return -1;
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
//   skipLogCurrElem
//---------------------------------------------------------

/**
 Skip the current element, log debug as info.
 */

void MusicXMLParserPass1::skipLogCurrElem()
      {
      _logger->logDebugInfo(QString("skipping '%1'").arg(_e.name().toString()), &_e);
      _e.skipCurrentElement();
      }

//---------------------------------------------------------
//   addBreak
//---------------------------------------------------------

static void addBreak(Score* const score, MeasureBase* const mb, const LayoutBreak::Type type)
      {
      LayoutBreak* lb = new LayoutBreak(score);
      lb->setLayoutBreakType(type);
      mb->add(lb);
      }

//---------------------------------------------------------
//   addBreakToPreviousMeasureBase
//---------------------------------------------------------

static void addBreakToPreviousMeasureBase(Score* const score, MeasureBase* const mb, const LayoutBreak::Type type)
      {
      const auto pm = mb->prev();
      if (pm && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTBREAKS))
            addBreak(score, pm, type);
      }

//---------------------------------------------------------
//   addText
//---------------------------------------------------------

/**
 Add text \a strTxt to VBox \a vbx using Tid \a stl.
 */

static void addText(VBox* vbx, Score* s, const QString strTxt, const Tid stl)
      {
      if (!strTxt.isEmpty()) {
            Text* text = new Text(s, stl);
            text->setXmlText(strTxt.trimmed());
            vbx->add(text);
            }
      }

//---------------------------------------------------------
//   addText2
//---------------------------------------------------------

/**
 Add text \a strTxt to VBox \a vbx using Tid \a stl.
 Also sets Align and Yoff.
 */

static void addText2(VBox* vbx, Score* s, const QString strTxt, const Tid stl, const Align align, const double yoffs)
      {
      if (!strTxt.isEmpty()) {
            Text* text = new Text(s, stl);
            text->setXmlText(strTxt.trimmed());
            text->setAlign(align);
            text->setPropertyFlags(Pid::ALIGN, PropertyFlags::UNSTYLED);
            text->setOffset(QPointF(0.0, yoffs));
            text->setPropertyFlags(Pid::OFFSET, PropertyFlags::UNSTYLED);
            vbx->add(text);
            }
      }

//---------------------------------------------------------
//   findYMinYMaxInWords
//---------------------------------------------------------

static void findYMinYMaxInWords(const std::vector<const CreditWords*>& words, int& miny, int& maxy)
      {
      miny = 0;
      maxy = 0;

      if (words.empty())
            return;

      miny = words.at(0)->defaultY;
      maxy = words.at(0)->defaultY;
      for (const auto w : words) {
            if (w->defaultY < miny) miny = w->defaultY;
            if (w->defaultY > maxy) maxy = w->defaultY;
            }
      }

//---------------------------------------------------------
//   alignForCreditWords
//---------------------------------------------------------

static Align alignForCreditWords(const CreditWords* const w, const int pageWidth)
      {
      Align align = Align::LEFT;
      if (w->defaultX > (pageWidth / 3)) {
            if (w->defaultX < (2 * pageWidth / 3))
                  align = Align::HCENTER;
            else
                  align = Align::RIGHT;
            }
      return align;
      }

//---------------------------------------------------------
//   creditWordTypeToTid
//---------------------------------------------------------

static Tid creditWordTypeToTid(const QString& type)
      {
      if (type == "composer")
            return Tid::COMPOSER;
      else if (type == "lyricist")
            return Tid::POET;
      /*
      else if (type == "page number")
            return Tid::;
      else if (type == "rights")
            return Tid::;
       */
      else if (type == "subtitle")
            return Tid::SUBTITLE;
      else if (type == "title")
            return Tid::TITLE;
      else
            return Tid::DEFAULT;
      }

//---------------------------------------------------------
//   creditWordTypeGuess
//---------------------------------------------------------

static Tid creditWordTypeGuess(const CreditWords* const word, std::vector<const CreditWords*>& words, const int pageWidth)
      {
      const auto pw1 = pageWidth / 3;
      const auto pw2 = pageWidth * 2 / 3;
      const auto defx = word->defaultX;
      // composer is in the right column
      if (pw2 < defx) {
            // found composer
            return Tid::COMPOSER;
            }
      // poet is in the left column
      else if (defx < pw1) {
            // found poet/lyricist
            return Tid::POET;
            }
      // title is in the middle column
      else {
            // if another word in the middle column has a larger font size, this word is not the title
            for (const auto w : words) {
                  if (w == word) {
                        continue;         // it's me
                        }
                  if (w->defaultX < pw1 || pw2 < w->defaultX) {
                        continue;         // it's not in the middle column
                        }
                  if (word->fontSize < w->fontSize) {
                        return Tid::SUBTITLE;          // word does not have the largest font size, assume subtitle
                        }
                  }
            return Tid::TITLE;            // no better title candidate found
            }
      }

//---------------------------------------------------------
//   tidForCreditWords
//---------------------------------------------------------

static Tid tidForCreditWords(const CreditWords* const word, std::vector<const CreditWords*>& words, const int pageWidth)
      {
      const Tid tid = creditWordTypeToTid(word->type);
      if (tid != Tid::DEFAULT) {
            // type recognized, done
            return tid;
            }
      else {
            // type not recognized, guess
            return creditWordTypeGuess(word, words, pageWidth);
            }
      }

//---------------------------------------------------------
//   createAndAddVBoxForCreditWords
//---------------------------------------------------------

VBox* MusicXMLParserPass1::createAndAddVBoxForCreditWords(Score* const score, const int miny, const int maxy)
      {
      auto vbox = new VBox(score);
      qreal vboxHeight = 10;                         // default height in tenths
      double diff = maxy - miny;                     // calculate height in tenths
      if (diff > vboxHeight)                         // and size is reasonable
            vboxHeight = diff;
      vboxHeight /= 10;                              // height in spatium
      vboxHeight += 2.5;                             // guesstimated correction for last line

      vbox->setBoxHeight(Spatium(vboxHeight));
      score->measures()->add(vbox);
      return vbox;
      }

//---------------------------------------------------------
//   mustAddWordToVbox
//---------------------------------------------------------

// determine if specific types of credit words must be added: do not add copyright and page number,
// as these typically conflict with MuseScore's style and/or layout

static bool mustAddWordToVbox(const QString& creditType)
      {
      return creditType != "rights" && creditType != "page number";
      }

//---------------------------------------------------------
//   isLikelySubtitleText
//---------------------------------------------------------

bool isLikelySubtitleText(const QString& text, const bool caseInsensitive = true)
      {
      QRegularExpression::PatternOption caseOption = caseInsensitive ? QRegularExpression::CaseInsensitiveOption : QRegularExpression::NoPatternOption;
      return (text.trimmed().contains(QRegularExpression("^[Ff]rom\\s+(?!$)", caseOption))
            || text.trimmed().contains(QRegularExpression("^Theme from\\s+(?!$)", caseOption))
            || text.trimmed().contains(QRegularExpression("(((Op\\.?\\s?\\d+)|(No\\.?\\s?\\d+))\\s?)+", caseOption))
            || text.trimmed().contains(QRegularExpression("\\(.*[Ff]rom\\s.*\\)", caseOption)));
      }

//---------------------------------------------------------
//   isLikelyCreditText
//---------------------------------------------------------

bool isLikelyCreditText(const QString& text, const bool caseInsensitive = true)
      {
      QRegularExpression::PatternOption caseOption = caseInsensitive ? QRegularExpression::CaseInsensitiveOption : QRegularExpression::NoPatternOption;
      return (text.trimmed().contains(QRegularExpression("^((Words|Music|Lyrics|Composed),?(\\sand|\\s&amp;|\\s&)?\\s)*[Bb]y\\s+(?!$)", caseOption))
            || text.trimmed().contains(QRegularExpression("^(Traditional|Trad\\.)", caseOption)));
      }

//---------------------------------------------------------
//   inferSubTitleFromTitle
//---------------------------------------------------------

// Extracts likely subtitle and credits from the title string

static void inferFromTitle(QString& title, QString& inferredSubtitle, QString& inferredCredits)
      {
      QStringList subtitleLines;
      QStringList creditLines;
      QStringList titleLines = title.split(QRegularExpression("\\n"));
      for (int i = titleLines.length() - 1; i > 0; --i) {
            QString line = titleLines[i];
            if (isLikelyCreditText(line, true)) {
                  for (int j = titleLines.length() - 1; j >= i; --j) {
                        creditLines.push_front(titleLines[j]);
                        titleLines.removeAt(j);
                        }
                  continue;
                  }
            if (isLikelySubtitleText(line, true)) {
                  for (int j = titleLines.length() - 1; j >= i; --j) {
                        subtitleLines.push_front(titleLines[j]);
                        titleLines.removeAt(j);
                        }
                  continue;
                  }
            }
      title = titleLines.join("\n");
      inferredSubtitle = subtitleLines.join("\n");
      inferredCredits = creditLines.join("\n");
      }
//---------------------------------------------------------
//   addCreditWords
//---------------------------------------------------------

static VBox* addCreditWords(Score* const score, const CreditWordsList& crWords,
                            const int pageNr, const QSize pageSize,
                            const bool top)
      {
      VBox* vbox = nullptr;

      std::vector<const CreditWords*> headerWords;
      std::vector<const CreditWords*> footerWords;
      for (const auto w : crWords) {
            if (w->page == pageNr) {
                  if (w->defaultY > (pageSize.height() / 2))
                        headerWords.push_back(w);
                  else
                        footerWords.push_back(w);
                  }
            }

      std::vector<const CreditWords*> words;
      if (pageNr == 1) {
            // if there are more credit words in the footer than in header,
            // swap heaer and footer, assuming this will result in a vertical
            // frame with the title on top of the page.
            // Sibelius (direct export) typically exports no header
            // and puts the title etc. in the footer
            const bool doSwap = footerWords.size() > headerWords.size();
            if (top) {
                  words = doSwap ? footerWords : headerWords;
                  }
            else {
                  words = doSwap ? headerWords : footerWords;
                  }
            }
      else {
            words = top ? headerWords : footerWords;
            }

      int miny = 0;
      int maxy = 0;
      findYMinYMaxInWords(words, miny, maxy);

      for (const auto w : words) {
            if (mustAddWordToVbox(w->type)) {
                  const auto align = alignForCreditWords(w, pageSize.width());
                  const auto tid = (pageNr == 1 && top) ? tidForCreditWords(w, words, pageSize.width()) : Tid::DEFAULT;
                  double yoffs = (maxy - w->defaultY) * score->spatium() / 10;
                  if (!vbox)
                        vbox = MusicXMLParserPass1::createAndAddVBoxForCreditWords(score, miny, maxy);
                  addText2(vbox, score, w->words, tid, align, yoffs);
                  }
            }

      return vbox;
      }

//---------------------------------------------------------
//   createDefaultHeader
//---------------------------------------------------------

void MusicXMLParserPass1::createDefaultHeader(Score* const score)
      {
      QString strTitle;
      QString strSubTitle;
      QString inferredStrSubTitle;
      QString inferredStrComposer;
      QString strComposer;
      QString strPoet;
      QString strTranslator;

      if (!(score->metaTag("movementTitle").isEmpty() && score->metaTag("workTitle").isEmpty())) {
            strTitle = score->metaTag("movementTitle");
            if (strTitle.isEmpty())
                  strTitle = score->metaTag("workTitle");
            inferFromTitle(strTitle, inferredStrSubTitle, inferredStrComposer);
            }
      if (!(score->metaTag("movementNumber").isEmpty() && score->metaTag("workNumber").isEmpty())) {
            strSubTitle = score->metaTag("movementNumber");
            if (strSubTitle.isEmpty())
                  strSubTitle = score->metaTag("workNumber");
            }
      if (!inferredStrSubTitle.isEmpty()) {
            strSubTitle = inferredStrSubTitle;
            _hasInferredHeaderText = true;
            }
      QString metaComposer = score->metaTag("composer");
      QString metaPoet = score->metaTag("poet");
      QString metaTranslator = score->metaTag("translator");
      if (!metaComposer.isEmpty()) strComposer = metaComposer;
      if (!inferredStrComposer.isEmpty()) {
            strComposer = inferredStrComposer;
            _hasInferredHeaderText = true;
            }
      if (metaPoet.isEmpty()) metaPoet = score->metaTag("lyricist");
      if (!metaPoet.isEmpty()) strPoet = metaPoet;
      if (!metaTranslator.isEmpty()) strTranslator = metaTranslator;

      const auto vbox = MusicXMLParserPass1::createAndAddVBoxForCreditWords(score);
      addText(vbox, score, strTitle.toHtmlEscaped(),      Tid::TITLE);
      addText(vbox, score, strSubTitle.toHtmlEscaped(),   Tid::SUBTITLE);
      addText(vbox, score, strComposer.toHtmlEscaped(),   Tid::COMPOSER);
      addText(vbox, score, strPoet.toHtmlEscaped(),       Tid::POET);
      addText(vbox, score, strTranslator.toHtmlEscaped(), Tid::TRANSLATOR);
      }

//---------------------------------------------------------
//   createMeasuresAndVboxes
//---------------------------------------------------------

/**
 Create required measures with correct number, start tick and length for Score \a score.
 */

void MusicXMLParserPass1::createMeasuresAndVboxes(Score* const score,
                                    const QVector<Fraction>& ml, const QVector<Fraction>& ms,
                                    const std::set<int>& systemStartMeasureNrs,
                                    const std::set<int>& pageStartMeasureNrs,
                                    const CreditWordsList& crWords,
                                    const QSize pageSize)
      {
      if (crWords.empty())
            createDefaultHeader(score);

      int pageNr = 0;
      for (int i = 0; i < ml.size(); ++i) {

            VBox* vbox = nullptr;

            // add a header vbox if the this measure is the first in the score or the first on a new page
            if (pageStartMeasureNrs.count(i) || i == 0) {
                  ++pageNr;
                  vbox = addCreditWords(score, crWords, pageNr, pageSize, true);
                  }

            // create and add the measure
            Measure* measure  = new Measure(score);
            measure->setTick(ms.at(i));
            measure->setTicks(ml.at(i));
            measure->setNo(i);
            score->measures()->add(measure);

            // add break to previous measure or vbox
            MeasureBase* mb = vbox;
            if (!mb) mb = measure;
            if (pageStartMeasureNrs.count(i))
                  addBreakToPreviousMeasureBase(score, mb, LayoutBreak::Type::PAGE);
            else if (systemStartMeasureNrs.count(i))
                  addBreakToPreviousMeasureBase(score, mb, LayoutBreak::Type::LINE);

            // add a footer vbox if the next measure is on a new page or end of score has been reached
            if (pageStartMeasureNrs.count(i+1) || i == (ml.size() - 1))
                  addCreditWords(score, crWords, pageNr, pageSize, false);
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
//   dumpPageSize
//---------------------------------------------------------

static void dumpPageSize(const QSize& pageSize)
      {
#if 0
      qDebug("page size width=%d height=%d", pageSize.width(), pageSize.height());
#else
      Q_UNUSED(pageSize);
#endif
      }

//---------------------------------------------------------
//   dumpCredits
//---------------------------------------------------------

static void dumpCredits(const CreditWordsList& credits)
      {
#if 0
      for (const auto w : credits) {
            qDebug("credit-words pg=%d tp='%s' defx=%g defy=%g just=%s hal=%s val=%s words='%s'",
                   w->page,
                   qPrintable(w->type),
                   w->defaultX,
                   w->defaultY,
                   qPrintable(w->justify),
                   qPrintable(w->hAlign),
                   qPrintable(w->vAlign),
                   qPrintable(w->words));
            }
#else
      Q_UNUSED(credits);
#endif
      }

//---------------------------------------------------------
//   fixupSigmap
//---------------------------------------------------------

/**
 To enable error handling in pass2, ensure sigmap contains a valid entry at tick = 0.
 Required by TimeSigMap::tickValues(), called (indirectly) by Segment::add().
 */

static void fixupSigmap(MxmlLogger* logger, Score* score, const QVector<Fraction>& measureLength)
      {
      auto it = score->sigmap()->find(0);

      if (it == score->sigmap()->end()) {
            // no valid timesig at tick = 0
            logger->logDebugInfo("no valid time signature at tick = 0");
            // use length of first measure instead time signature.
            // if there is no first measure, we probably don't care,
            // but set a default anyway.
            Fraction tsig = measureLength.isEmpty() ? Fraction(4, 4) : measureLength.at(0);
            score->sigmap()->add(0, tsig);
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
      _logger->logDebugTrace("MusicXMLParserPass1::parse device");
      _parts.clear();
      _e.setDevice(device);
      auto res = parse();
      if (res != Score::FileError::FILE_NO_ERROR)
            return res;

      // Determine the start tick of each measure in the part
      determineMeasureLength(_measureLength);
      determineMeasureStart(_measureLength, _measureStart);
      // Fixup timesig at tick = 0 if necessary
      fixupSigmap(_logger, _score, _measureLength);
      // Debug: dump gae size and credits read
      dumpPageSize(_pageSize);
      dumpCredits(_credits);
      // Create the measures
      createMeasuresAndVboxes(_score, _measureLength, _measureStart, _systemStartMeasureNrs, _pageStartMeasureNrs, _credits, _pageSize);

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
      _logger->logDebugTrace("MusicXMLParserPass1::parse");

      bool found = false;
      while (_e.readNextStartElement()) {
            if (_e.name() == "score-partwise") {
                  found = true;
                  scorePartwise();
                  }
            else {
                  _logger->logError(QString("this is not a MusicXML score-partwise file (top-level node '%1')")
                                    .arg(_e.name().toString()), &_e);
                  _e.skipCurrentElement();
                  return Score::FileError::FILE_BAD_FORMAT;
                  }
            }

      if (!found) {
            _logger->logError("this is not a MusicXML score-partwise file, node <score-partwise> not found", &_e);
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
            if (p->staff(0)->constStaffType(Fraction(0,1))->group() != p->staff(i)->constStaffType(Fraction(0,1))->group())
                  return false;
            }
      return true;
      }

//---------------------------------------------------------
//   isRedundantBracket
//---------------------------------------------------------

/**
 Return true if there is already an existing bracket
 with the same type and span.
 This prevents double brackets, which are sometimes exported
 by Dolet.
 */

static bool isRedundantBracket(Staff const* const staff, const BracketType bracketType, const int span)
      {
      for (auto bracket : staff->brackets())
            if (bracket->bracketType() == bracketType && bracket->bracketSpan() == span)
                  return true;
      return false;
      }

//---------------------------------------------------------
//   scorePartwise
//---------------------------------------------------------

/**
 Parse the MusicXML top-level (XPath /score-partwise) node.
 */

void MusicXMLParserPass1::scorePartwise()
      {
      _logger->logDebugTrace("MusicXMLParserPass1::scorePartwise", &_e);

      MusicXmlPartGroupList partGroupList;

      while (_e.readNextStartElement()) {
            if (_e.name() == "part")
                  part();
            else if (_e.name() == "part-list")
                  partList(partGroupList);
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
                  defaults();
            else if (_e.name() == "movement-number")
                  _score->setMetaTag("movementNumber", _e.readElementText());
            else if (_e.name() == "movement-title")
                  _score->setMetaTag("movementTitle", _e.readElementText());
            else if (_e.name() == "credit")
                  credit(_credits);
            else
                  skipLogCurrElem();
            }

      // add brackets where required

      /*
       qDebug("partGroupList");
       for (size_t i = 0; i < partGroupList.size(); i++) {
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
      for (size_t i = 0; i < partGroupList.size(); i++) {
            MusicXmlPartGroup* pg = partGroupList[i];
            // determine span in staves
            // and span all barlines except last if applicable
            int stavesSpan = 0;
            for (int j = 0; j < pg->span; j++) {
                  Part* spannedPart = il.at(pg->start + j);
                  stavesSpan += spannedPart->nstaves();

                  if (pg->barlineSpan) {
                        for (auto spannedStaff : *spannedPart->staves()) {
                              if ((j == pg->span - 1) && (spannedStaff == spannedPart->staves()->back()))
                                    // Very last staff of group,
                                    continue;
                              else
                                    spannedStaff->setBarLineSpan(true);
                              }
                        }
                  }

            // add bracket and set the span
            // TODO: use group-symbol default-x to determine horizontal order of brackets
            Staff* staff = il.at(pg->start)->staff(0);
            if (pg->type != BracketType::NO_BRACKET && !isRedundantBracket(staff, pg->type, stavesSpan)) {
                  staff->setBracketType(pg->column, pg->type);
                  staff->setBracketSpan(pg->column, stavesSpan);
                  // add part to set (skip implicit bracket later)
                  if (pg->span == 1)
                      partSet << il.at(pg->start);
                  }
            }

      // handle the implicit brackets:
      // multi-staff parts w/o explicit brackets get a brace
      for (Part const* const p : il) {
            if (p->nstaves() > 1 && !partSet.contains(p)) {
                  const int column = p->staff(0)->bracketLevels() + 1;
                  p->staff(0)->setBracketType(column, BracketType::BRACE);
                  p->staff(0)->setBracketSpan(column, p->nstaves());
                  if (allStaffGroupsIdentical(p))
                        // span only if the same types
                        for (auto spannedStaff : *p->staves())
                              if (spannedStaff != p->staves()->back()) // not last staff
                                    spannedStaff->setBarLineSpan(true);
                  }
            }
      addError(checkAtEndElement(_e, "score-partwise"));
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
      _logger->logDebugTrace("MusicXMLParserPass1::identification", &_e);

      while (_e.readNextStartElement()) {
            if (_e.name() == "creator") {
                  // type is an arbitrary label
                  QString strType = _e.attributes().value("type").toString();
                  _score->setMetaTag(strType, _e.readElementText());
                  }
            else if (_e.name() == "rights") {
                  _score->setMetaTag("copyright", _e.readElementText().trimmed());
                  bool copyrightFirstPageOnly = true; // TODO: expose as import setting
                  if (copyrightFirstPageOnly)
                        // Somewhat temporary fix: hide footer and make copyright a text box
                        _score->setStyleValue(Sid::showFooter, false);
                  }
            else if (_e.name() == "encoding") {
                  // TODO
                  while (_e.readNextStartElement()) {
                        if (_e.name() == "software")
                              _exporterString += _e.readElementText().toLower();
                        else if (_e.name() == "supports" && _e.attributes().value("element") == "beam" && _e.attributes().value("type") == "yes") {
                              _hasBeamingInfo = true;
                              _e.skipCurrentElement();
                              }
                        else if (_e.name() == "supports" && _e.attributes().value("element") == "transpose") {
                              _supportsTranspose = _e.attributes().value("type").toString();
                              _e.skipCurrentElement();
                              }
                        else
                              _e.skipCurrentElement();
                        }
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

      while (!in.isEmpty()) {
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
                  res += in.leftRef(1);
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
      QString strike     = e.attributes().value("line-through").toString();
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

      bool needUseDefaultFont = preferences.getBool(PREF_MIGRATION_APPLY_EDWIN_FOR_XML_FILES);

      if (!fontFamily.isEmpty() && txt == syms && !needUseDefaultFont) {
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
            if (ok && (lines > 0))  // 1, 2, or 3 underlines are imported as single underline
                  importedtext += "<u>";
            else
                  underline.clear();
            }
      if (!strike.isEmpty()) {
            bool ok = true;
            int lines = strike.toInt(&ok);
            if (ok && (lines > 0))  // 1, 2, or 3 strikes are imported as single strike
                  importedtext += "<s>";
            else
                  strike.clear();
            }
      if (txt == syms) {
            txt.replace(QString("\r"), QString("")); // convert Windows line break \r\n -> \n
            importedtext += txt.toHtmlEscaped();
            }
      else {
            // <sym> replacement made, should be no need for line break or other conversions
            importedtext += syms;
            }
      if (!strike.isEmpty())
            importedtext += "</s>";
      if (!underline.isEmpty())
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
      _logger->logDebugTrace("MusicXMLParserPass1::credit", &_e);

      const auto page = _e.attributes().value("page").toString().toInt();       // ignoring errors implies incorrect conversion defaults to the first page
      // multiple credit-words elements may be present,
      // which are appended
      // use the position info from the first one
      // font information is ignored, credits will be styled
      bool creditWordsRead = false;
      double defaultx = 0;
      double defaulty = 0;
      double fontSize = 0;
      QString justify;
      QString halign;
      QString valign;
      QStringList crtypes;
      QString crwords;
      while (_e.readNextStartElement()) {
            if (_e.name() == "credit-words") {
                  // IMPORT_LAYOUT
                  if (!creditWordsRead) {
                        defaultx = _e.attributes().value("default-x").toString().toDouble();
                        defaulty = _e.attributes().value("default-y").toString().toDouble();
                        fontSize = _e.attributes().value("font-size").toString().toDouble();
                        justify  = _e.attributes().value("justify").toString();
                        halign   = _e.attributes().value("halign").toString();
                        valign   = _e.attributes().value("valign").toString();
                        creditWordsRead = true;
                        }
                  crwords += nextPartOfFormattedString(_e);
                  }
            else if (_e.name() == "credit-type") {
                  // multiple credit-type elements may be present, supported by
                  // e.g. Finale v26.3 for Mac.
                  crtypes += _e.readElementText();
                  }
            else
                  skipLogCurrElem();
            }
      if (!crwords.isEmpty()) {
            // as the meaning of multiple credit-types is undocumented,
            // use credit-type only if exactly one was found
            QString crtype = (crtypes.size() == 1) ? crtypes.at(0) : QString();
            CreditWords* cw = new CreditWords(page, crtype, defaultx, defaulty, fontSize, justify, halign, valign, crwords);
            credits.append(cw);
            }

      }

//---------------------------------------------------------
//   isTitleFrameStyle
//---------------------------------------------------------

/**
 Determine if tid is a style type used in a title frame
 */

static bool isTitleFrameStyle(const Tid tid)
      {
      return tid == Tid::TITLE
             || tid == Tid::SUBTITLE
             || tid == Tid::COMPOSER
             || tid == Tid::POET;
      }

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
      const auto dblWordSize = wordSize.toDouble();   // note conversion error results in value 0.0
      const auto dblLyricSize = lyricSize.toDouble(); // but avoid comparing (double) floating point number with exact value later
      const auto epsilon = 0.001;                     // use epsilon instead

      bool needUseDefaultFont = preferences.getBool(PREF_MIGRATION_APPLY_EDWIN_FOR_XML_FILES);

      // loop over all text styles (except the empty, always hidden, first one)
      // set all text styles to the MusicXML defaults
      for (const auto tid : allTextStyles()) {

            // The MusicXML specification does not specify to which kinds of text
            // the word-font setting applies. Setting all sizes to the size specified
            // gives bad results, so a selection is made:
            // exclude lyrics odd and even lines (handled separately),
            // Roman numeral analysis (special case, leave untouched)
            // and text types used in the title frame
            // Some further tweaking may still be required.

            if (tid == Tid::LYRICS_ODD || tid == Tid::LYRICS_EVEN
                || tid == Tid::HARMONY_ROMAN
                || isTitleFrameStyle(tid))
                  continue;
            const TextStyle* ts = textStyle(tid);
            for (const StyledProperty& a :* ts) {
                  if (a.pid == Pid::FONT_FACE && !wordFamily.isEmpty() && !needUseDefaultFont)
                        score->style().set(a.sid, wordFamily);
                  else if (a.pid == Pid::FONT_SIZE && dblWordSize > epsilon)
                        score->style().set(a.sid, dblWordSize);
                  }
            }

      // handle lyrics odd and even lines separately
      if (!lyricFamily.isEmpty() && !needUseDefaultFont) {
            score->style().set(Sid::lyricsOddFontFace, lyricFamily);
            score->style().set(Sid::lyricsEvenFontFace, lyricFamily);
            }
      if (dblLyricSize > epsilon) {
            score->style().set(Sid::lyricsOddFontSize, QVariant(dblLyricSize));
            score->style().set(Sid::lyricsEvenFontSize, QVariant(dblLyricSize));
            }
      }

//---------------------------------------------------------
//   setPageFormat
//---------------------------------------------------------

static void setPageFormat(Score* score, const MxmlPageFormat& pf)
      {
      score->style().set(Sid::pageWidth, pf.size.width());
      score->style().set(Sid::pageHeight, pf.size.height());
      score->style().set(Sid::pagePrintableWidth, pf.printableWidth);
      score->style().set(Sid::pageEvenLeftMargin, pf.evenLeftMargin);
      score->style().set(Sid::pageOddLeftMargin, pf.oddLeftMargin);
      score->style().set(Sid::pageEvenTopMargin, pf.evenTopMargin);
      score->style().set(Sid::pageEvenBottomMargin, pf.evenBottomMargin);
      score->style().set(Sid::pageOddTopMargin, pf.oddTopMargin);
      score->style().set(Sid::pageOddBottomMargin, pf.oddBottomMargin);
      score->style().set(Sid::pageTwosided, pf.twosided);
      }

//---------------------------------------------------------
//   defaults
//---------------------------------------------------------

/**
 Parse the /score-partwise/defaults node:
 read the general score layout settings.
 */

void MusicXMLParserPass1::defaults()
      {
      //_logger->logDebugTrace("MusicXMLParserPass1::defaults", &_e);

      double millimeter = _score->spatium()/10.0;
      double tenths = 1.0;
      QString lyricFontFamily;
      QString lyricFontSize;
      QString wordFontFamily;
      QString wordFontSize;

      bool isImportLayout = preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT);

      while (_e.readNextStartElement()) {
            if (/*isImportLayout && */_e.name() == "scaling") {
                  while (_e.readNextStartElement()) {
                        if (_e.name() == "millimeters")
                              millimeter = _e.readElementText().toDouble();
                        else if (_e.name() == "tenths")
                              tenths = _e.readElementText().toDouble();
                        else
                              skipLogCurrElem();
                        }
                  double _spatium = DPMM * (millimeter * 10.0 / tenths);
                  if (isImportLayout)
                        _score->setSpatium(_spatium);
                  }
            else if (_e.name() == "concert-score") {
                  _score->style().set(Sid::concertPitch, true);
                  _e.skipCurrentElement();            // skip but don't log
                  }
            else if (/*isImportLayout && */_e.name() == "page-layout") {
                  MxmlPageFormat pf;
                  pageLayout(pf, millimeter / (tenths * INCH));
                  if (isImportLayout)
                        setPageFormat(_score, pf);
                  }
            else if (/*isImportLayout && */_e.name() == "system-layout") {
                  while (_e.readNextStartElement()) {
                        if (_e.name() == "system-margins")
                              _e.skipCurrentElement();  // skip but don't log
                        else if (/*isImportLayout && */_e.name() == "system-distance") {
                              const Spatium val(_e.readElementText().toDouble() / 10.0);
                              if (isImportLayout) {
                                    _score->style().set(Sid::minSystemDistance, val);
                                    //qDebug("system distance %f", val.val());
                                    }
                              }
                        else if (_e.name() == "top-system-distance")
                              _e.skipCurrentElement();  // skip but don't log
                        else if (/*isImportLayout && */_e.name() == "system-dividers") {
                              while (_e.readNextStartElement()) {
                              if (_e.name() == "left-divider") {
                                  _score->style().set(Sid::dividerLeft, (_e.attributes().value("print-object") != "no"));
                                  if (isImportLayout) {
                                      _score->style().set(Sid::dividerLeftX, _e.attributes().value("relative-x").toDouble() / 10.0);
                                      _score->style().set(Sid::dividerLeftY, _e.attributes().value("relative-y").toDouble() / 10.0);
                                  }
                                  _e.skipCurrentElement();
                              }
                              else if (_e.name() == "right-divider") {
                                  _score->style().set(Sid::dividerRight, (_e.attributes().value("print-object") != "no"));
                                  if (isImportLayout) {
                                      _score->style().set(Sid::dividerRightX, _e.attributes().value("relative-x").toDouble() / 10.0);
                                      _score->style().set(Sid::dividerRightY, _e.attributes().value("relative-y").toDouble() / 10.0);
                                  }
                                  _e.skipCurrentElement();
                              }
                              else
                                  skipLogCurrElem();
                              }
                        }
                        else
                              skipLogCurrElem();
                        }
                  }
            else if (/*isImportLayout && */_e.name() == "staff-layout") {
                  while (_e.readNextStartElement()) {
                        if (_e.name() == "staff-distance") {
                              Spatium val(_e.readElementText().toDouble() / 10.0);
                              if (isImportLayout)
                                    _score->style().set(Sid::staffDistance, val);
                              }
                        else
                              skipLogCurrElem();
                        }
                  }
            else if (_e.name() == "appearance") {
                  while (_e.readNextStartElement()) {
                        const QString type = _e.attributes().value("type").toString();
                        if (_e.name() == "line-width") {
                              const double val = _e.readElementText().toDouble();
                              if (isImportLayout)
                                    setStyle(type, val);
                              }
                        else if (_e.name() == "note-size") {
                              const double val = _e.readElementText().toDouble();
                              if (isImportLayout)
                                    setStyle(type, val);
                              }
                        else if (_e.name() == "distance")
                              _e.skipCurrentElement();        // skip but don't log
                        else if (_e.name() == "glyph")
                              _e.skipCurrentElement();        // skip but don't log
                        else if (_e.name() == "other-appearance")
                              _e.skipCurrentElement();        // skip but don't log
                        else
                              skipLogCurrElem();
                        }
                  }
            else if (_e.name() == "music-font")
                  _e.skipCurrentElement();  // skip but don't log
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
            else if (_e.name() == "lyric-language")
                  _e.skipCurrentElement();  // skip but don't log
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
//   setStyle
//---------------------------------------------------------

void MusicXMLParserPass1::setStyle(const QString& type, const double val)
      {
      if (type == "light barline")
            _score->style().set(Sid::barWidth, Spatium(val / 10));
      else if (type == "heavy barline")
            _score->style().set(Sid::endBarWidth, Spatium(val / 10));
      else if (type == "beam")
            _score->style().set(Sid::beamWidth, Spatium(val / 10));
      else if (type == "bracket")
            _score->style().set(Sid::bracketWidth, Spatium(val / 10));
      else if (type == "dashes")
            _score->style().set(Sid::lyricsDashLineThickness, Spatium(val / 10));
      else if (type == "enclosure")
            _score->style().set(Sid::staffTextFrameWidth, Spatium(val / 10));
      else if (type == "ending")
            _score->style().set(Sid::voltaLineWidth, Spatium(val / 10));
      else if (type == "extend")
            _score->style().set(Sid::lyricsLineThickness, Spatium(val / 10));
      else if (type == "leger")
            _score->style().set(Sid::ledgerLineWidth, Spatium(val / 10));
      else if (type == "pedal")
            _score->style().set(Sid::pedalLineWidth, Spatium(val / 10));
      else if (type == "octave shift")
            _score->style().set(Sid::ottavaLineWidth, Spatium(val / 10));
      else if (type == "staff")
            _score->style().set(Sid::staffLineWidth, Spatium(val / 10));
      else if (type == "stem")
            _score->style().set(Sid::stemWidth, Spatium(val / 10));
      else if (type == "tuplet bracket")
            _score->style().set(Sid::tupletBracketWidth, Spatium(val / 10));
      else if (type == "wedge")
            _score->style().set(Sid::hairpinLineWidth, Spatium(val / 10));
      else if (type == "slur middle")
            _score->style().set(Sid::SlurMidWidth, Spatium(val / 10));
      else if (type == "tie middle")
            _score->style().set(Sid::TieMidWidth, Spatium(val / 10));
      else if (type == "slur tip")
            _score->style().set(Sid::SlurEndWidth, Spatium(val / 10));
      else if (type == "tie tip")
            _score->style().set(Sid::TieEndWidth, Spatium(val / 10));
      else if (type == "cue")
            _score->style().set(Sid::smallNoteMag, val / 100);
      else if (type == "grace")
            _score->style().set(Sid::graceNoteMag, val / 100);
      else if (type == "grace-cue") {
            } // not supported
      //else if (type == "large")
      //else if (type == "beam")
      //else if (type == "hyphen")
      }

//---------------------------------------------------------
//   pageLayout
//---------------------------------------------------------

/**
 Parse the /score-partwise/defaults/page-layout node: read the page layout.
 Note that MuseScore does not support a separate value for left and right margins
 for odd and even pages. Only odd and even left margins are used, together  with
 the printable width, which is calculated from the left and right margins in the
 MusicXML file.
 */

void MusicXMLParserPass1::pageLayout(MxmlPageFormat& pf, const qreal conversion)
      {
      _logger->logDebugTrace("MusicXMLParserPass1::pageLayout", &_e);

      qreal _oddRightMargin  = 0.0;
      qreal _evenRightMargin = 0.0;
      QSizeF size;

      while (_e.readNextStartElement()) {
            if (_e.name() == "page-margins") {
                  QString type = _e.attributes().value("type").toString();
                  if (type.isEmpty())
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
                  const double val = _e.readElementText().toDouble();
                  size.rheight() = val * conversion;
                  // set pageHeight and pageWidth for use by doCredits()
                  _pageSize.setHeight(static_cast<int>(val + 0.5));
                  }
            else if (_e.name() == "page-width") {
                  const double val = _e.readElementText().toDouble();
                  size.rwidth() = val * conversion;
                  // set pageHeight and pageWidth for use by doCredits()
                  _pageSize.setWidth(static_cast<int>(val + 0.5));
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
      _logger->logDebugTrace("MusicXMLParserPass1::partList", &_e);

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
      staff->setHideWhenEmpty(Staff::HideMode::INSTRUMENT);
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
      //qDebug("partGroupStart number=%d part=%d symbol=%s", n, p, qPrintable(s));

      if (pgs.count(n) > 0) {
            qDebug("part-group number=%d already active", n);
            return;
            }

      BracketType bracketType = BracketType::NO_BRACKET;
      if (s.isEmpty())
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
            qDebug("part-group symbol=%s not supported", qPrintable(s));
            return;
            }

      MusicXmlPartGroup* pg = new MusicXmlPartGroup;
      pg->span = 0;
      pg->start = p;
      pg->barlineSpan = barlineSpan,
      pg->type = bracketType;
      pg->column = n;
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
      _logger->logDebugTrace("MusicXMLParserPass1::partGroup", &_e);
      bool barlineSpan = true;
      int number = _e.attributes().value("number").toInt();
      if (number > 0)
            number--;
      QString symbol;
      QString type = _e.attributes().value("type").toString();

      while (_e.readNextStartElement()) {
            if (_e.name() == "group-name")
                  _e.skipCurrentElement();  // skip but don't log
            else if (_e.name() == "group-abbreviation")
                  symbol = _e.readElementText();
            else if (_e.name() == "group-symbol")
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
            _logger->logError(QString("part-group type '%1' not supported").arg(type), &_e);
      }

#if 0 // not used
//---------------------------------------------------------
//   findInstrument
//---------------------------------------------------------

/**
 Find the first InstrumentTemplate with musicXMLid instrSound
 and a non-empty set of channels.
 */

static const InstrumentTemplate* findInstrument(const QString& instrSound)
      {
      const InstrumentTemplate* instr = nullptr;

      for (const InstrumentGroup* group : instrumentGroups) {
            for (const InstrumentTemplate* templ : group->instrumentTemplates) {
                  if (templ->musicXMLid == instrSound && !templ->channel.isEmpty()) {
                        return templ;
                        }
                  }
            }
      return instr;
      }
#endif

//---------------------------------------------------------
//   addInferredTranspose
//---------------------------------------------------------
/**
 In the case that transposition information is missing,
 instrument-level transpositions are inferred here.
 This changes the *written* pitch, but retains the sounding pitch.
 */
void MusicXMLParserPass1::addInferredTranspose(const QString& partId)
      {
      if (_parts[partId].getName().contains("guitar", Qt::CaseInsensitive)
            && !_parts[partId].hasTab()) {
            _parts[partId]._inferredTranspose = Interval(12);
            _parts[partId]._intervals[Fraction(0, 1)] = Interval(-12);
            }
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
      _logger->logDebugTrace("MusicXMLParserPass1::scorePart", &_e);
      QString id = _e.attributes().value("id").toString().trimmed();

      if (_parts.contains(id)) {
            _logger->logError(QString("duplicate part id '%1'").arg(id), &_e);
            skipLogCurrElem();
            return;
            }
      else {
            _parts.insert(id, MusicXmlPart(id));
            _instruments.insert(id, MusicXMLInstruments());
            createPart(_score, id, _partMap);
            }

      while (_e.readNextStartElement()) {
            if (_e.name() == "part-name") {
                  // Element part-name contains the displayed (full) part name
                  // It is displayed by default, but can be suppressed (print-object=”no”)
                  // As of MusicXML 3.0, formatting is deprecated, with part-name in plain text
                  // and the formatted version in the part-name-display element
                  _parts[id].setPrintName(_e.attributes().value("print-object") != "no");
                  QString name = _e.readElementText();
                  _parts[id].setName(name);
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
                  _parts[id].setPrintAbbr(_e.attributes().value("print-object") != "no");
                  QString name = _e.readElementText();
                  _parts[id].setAbbr(name);
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
                        for (auto it = _instruments[id].cbegin(); it != _instruments[id].cend(); ++it)
                              _instruments[id][it.key()].midiPort = port.toInt() - 1;
                        }
                  else if (_instruments[id].contains(instrId))
                        _instruments[id][instrId].midiPort = port.toInt() - 1;

                  _e.readElementText(); // empty string
                  }
            else if (_e.name() == "midi-instrument")
                  midiInstrument(id);
            else
                  skipLogCurrElem();
            }
      }

//---------------------------------------------------------
//   scoreInstrument
//---------------------------------------------------------

/**
 Parse the /score-partwise/part-list/score-part/score-instrument node.
 */

void MusicXMLParserPass1::scoreInstrument(const QString& partId)
      {
      _logger->logDebugTrace("MusicXMLParserPass1::scoreInstrument", &_e);
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
                  _instruments[partId].insert(instrId, MusicXMLInstrument(instrName));
                  // Element instrument-name is typically not displayed in the score,
                  // but used only internally
                  if (_instruments[partId].contains(instrId))
                        _instruments[partId][instrId].name = instrName;
                  }
            else if (_e.name() == "instrument-abbreviation") {
                  QString abbreviation = _e.readElementText();
                  if (_instruments[partId].contains(instrId))
                        _instruments[partId][instrId].abbreviation = abbreviation;
                  }
            else if (_e.name() == "instrument-sound") {
                  QString instrSound = _e.readElementText();
                  if (_instruments[partId].contains(instrId))
                        _instruments[partId][instrId].sound = instrSound;
                  }
            else if (_e.name() == "virtual-instrument") {
                  while (_e.readNextStartElement()) {
                        if (_e.name() == "virtual-library") {
                              QString virtualLibrary = _e.readElementText();
                              if (_instruments[partId].contains(instrId))
                                    _instruments[partId][instrId].virtLib = virtualLibrary;
                              }
                        else if (_e.name() == "virtual-name") {
                              QString virtualName = _e.readElementText();
                              if (_instruments[partId].contains(instrId))
                                    _instruments[partId][instrId].virtName = virtualName;
                              }
                        else
                              skipLogCurrElem();
                        }
                  }
            else
                  skipLogCurrElem();
            }
      }

//---------------------------------------------------------
//   midiInstrument
//---------------------------------------------------------

/**
 Parse the /score-partwise/part-list/score-part/midi-instrument node.
 */

void MusicXMLParserPass1::midiInstrument(const QString& partId)
      {
      _logger->logDebugTrace("MusicXMLParserPass1::midiInstrument", &_e);
      QString instrId = _e.attributes().value("id").toString();

      while (_e.readNextStartElement()) {
            if (_e.name() == "midi-bank")
                  skipLogCurrElem();
            else if (_e.name() == "midi-channel") {
                  int channel = _e.readElementText().toInt();
                  if (channel < 1) {
                        _logger->logError(QString("incorrect midi-channel: %1").arg(channel), &_e);
                        channel = 1;
                        }
                  else if (channel > 16) {
                        _logger->logError(QString("incorrect midi-channel: %1").arg(channel), &_e);
                        channel = 16;
                        }
                  if (_instruments[partId].contains(instrId))
                        _instruments[partId][instrId].midiChannel = channel - 1;
                  }
            else if (_e.name() == "midi-program") {
                  int program = _e.readElementText().toInt();
                  // Bug fix for Cubase 6.5.5 which generates <midi-program>0</midi-program>
                  // Check program number range
                  if (program < 1) {
                        _logger->logError(QString("incorrect midi-program: %1").arg(program), &_e);
                        program = 1;
                        }
                  else if (program > 128) {
                        _logger->logError(QString("incorrect midi-program: %1").arg(program), &_e);
                        program = 128;
                        }
                  if (_instruments[partId].contains(instrId))
                        _instruments[partId][instrId].midiProgram = program - 1;
                  }
            else if (_e.name() == "midi-unpitched") {
                  if (_instruments[partId].contains(instrId))
                        _instruments[partId][instrId].unpitched = _e.readElementText().toInt() - 1;
                  }
            else if (_e.name() == "volume") {
                  double vol = _e.readElementText().toDouble();
                  if (vol >= 0 && vol <= 100) {
                        if (_instruments[partId].contains(instrId))
                              _instruments[partId][instrId].midiVolume = static_cast<int>((vol / 100) * 127);
                        }
                  else
                        _logger->logError(QString("incorrect midi-volume: %1").arg(vol), &_e);
                  }
            else if (_e.name() == "pan") {
                  double pan = _e.readElementText().toDouble();
                  if (pan >= -90 && pan <= 90) {
                        if (_instruments[partId].contains(instrId))
                              _instruments[partId][instrId].midiPan = static_cast<int>(((pan + 90) / 180) * 127);
                        }
                  else
                        _logger->logError(QString("incorrect midi-volume: %g1").arg(pan), &_e);
                  }
            else
                  skipLogCurrElem();
            }
      }

//---------------------------------------------------------
//   setNumberOfStavesForPart
//---------------------------------------------------------

/**
 Set number of staves for part \a partId to the max value
 of the current value \a staves.
 Also handle HideMode.
 */

static void setNumberOfStavesForPart(Part* const part, const int staves)
      {
      if (!part)
            return;
      int prevnstaves = part->nstaves();
      if (staves > part->nstaves()) {
            part->setStaves(staves);
            // New staves default to INSTRUMENT hide mode
            for (int i = prevnstaves; i < staves; ++i)
                  part->staff(i)->setHideWhenEmpty(Staff::HideMode::INSTRUMENT);
            }
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
      _logger->logDebugTrace("MusicXMLParserPass1::part", &_e);
      const QString id = _e.attributes().value("id").toString().trimmed();

      if (!_parts.contains(id)) {
            _logger->logError(QString("cannot find part '%1'").arg(id), &_e);
            skipLogCurrElem();
            return;
            }

      initPartState(id);

      VoiceOverlapDetector vod;
      Fraction time;  // current time within part
      Fraction mdur;  // measure duration

      int measureNr = 0;
      while (_e.readNextStartElement()) {
            if (_e.name() == "measure") {
                  measure(id, time, mdur, vod, measureNr);
                  time += mdur;
                  ++measureNr;
                  }
            else
                  skipLogCurrElem();
            }

      // Bug fix for Cubase 6.5.5..9.5.10 which generate <staff>2</staff> in a single staff part
      setNumberOfStavesForPart(_partMap.value(id), _parts[id].maxStaff() + 1);
      // allocate MuseScore staff to MusicXML voices
      allocateStaves(_parts[id].voicelist);
      // allocate MuseScore voice to MusicXML voices
      allocateVoices(_parts[id].voicelist);
      // calculate the octave shifts
      _parts[id].calcOctaveShifts();
      // determine the lyric numbers for this part
      _parts[id].lyricNumberHandler().determineLyricNos();

#if 0
      // debug: print results
      //qDebug("%s", qPrintable(_parts[id].toString()));

      //qDebug("lyric numbers: %s", qPrintable(_parts[id].lyricNumberHandler().toString()));

      qDebug("instrument map:");
      for (auto& instr : _parts[id]._instrList) {
            qDebug("- %s '%s'", qPrintable(instr.first.print()), qPrintable(instr.second));
            }
      qDebug("transpose map:");
      for (auto& it : _parts[id]._intervals) {
            qDebug("- %s %d %d", qPrintable(it.first.print()), it.second.diatonic, it.second.chromatic);
            }
      qDebug("instrument transpositions:");
      if (_parts[id]._instrList.empty()) {
            const Fraction tick { 0, 1 };
            const QString name { "none" };
            const auto interval = _parts[id]._intervals.interval(tick);
            qDebug("- %s '%s' -> %d %d",
                   qPrintable(tick.print()), qPrintable(name), interval.diatonic, interval.chromatic);
            }
      else {
            for (auto& instr : _parts[id]._instrList) {
                  const auto& tick = instr.first;
                  const auto& name = instr.second;
                  const auto interval = _parts[id].interval(tick);
                  qDebug("- %s '%s' -> %d %d",
                         qPrintable(tick.print()), qPrintable(name), interval.diatonic, interval.chromatic);
                  }
            }

      /*
      qDebug("voiceMapperStats: new staff");
      VoiceList& vl = _parts[id].voicelist;
      for (auto i = vl.constBegin(); i != vl.constEnd(); ++i) {
            qDebug("voiceMapperStats: voice %s staff data %s",
                   qPrintable(i.key()), qPrintable(i.value().toString()));
            }
      */
#endif

      addError(checkAtEndElement(_e, "part"));
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
                                  const Fraction cTime,
                                  Fraction& mdur,
                                  VoiceOverlapDetector& vod,
                                  const int measureNr)
      {
      _logger->logDebugTrace("MusicXMLParserPass1::measure", &_e);
      QString number = _e.attributes().value("number").toString();

      Fraction mTime; // current time stamp within measure
      Fraction mDura; // current total measure duration
      vod.newMeasure();
      MxmlTupletStates tupletStates;

      while (_e.readNextStartElement()) {
            if (_e.name() == "attributes")
                  attributes(partId, cTime + mTime);
            else if (_e.name() == "barline")
                  _e.skipCurrentElement();  // skip but don't log
            else if (_e.name() == "note") {
                  Fraction missingPrev;
                  Fraction dura;
                  Fraction missingCurr;
                  // note: chord and grace note handling done in note()
                  note(partId, cTime + mTime, missingPrev, dura, missingCurr, vod, tupletStates);
                  if (missingPrev.isValid()) {
                        mTime += missingPrev;
                        }
                  if (dura.isValid()) {
                        mTime += dura;
                        }
                  if (missingCurr.isValid()) {
                        mTime += missingCurr;
                        }
                  if (mTime > mDura)
                        mDura = mTime;
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
                              _logger->logError("backup beyond measure start", &_e);
                              mTime.set(0, 1);
                              }
                        }
                  }
            else if (_e.name() == "direction")
                  direction(partId, cTime + mTime);
            else if (_e.name() == "harmony")
                  _e.skipCurrentElement();  // skip but don't log
            else if (_e.name() == "print")
                  print(measureNr);
            else if (_e.name() == "sound")
                  _e.skipCurrentElement();  // skip but don't log
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

      // fix for PDFtoMusic Pro v1.3.0d Build BF4E and PlayScore / ReadScoreLib Version 3.11
      // which sometimes generate empty measures
      // if no valid length found and length according to time signature is known,
      // use length according to time signature
      if (mDura.isZero() && _timeSigDura.isValid() && _timeSigDura > Fraction(0, 1))
            mDura = _timeSigDura;
      // if no valid length found and time signature is unknown, use default
      if (mDura.isZero() && !_timeSigDura.isValid())
            mDura = Fraction(4, 4);

      // if necessary, round up to an integral number of 1/128th,
      // to comply with MuseScores actual measure length constraints
      Fraction length = mDura * Fraction(128,1);
      Fraction correctedLength = mDura;
      length.reduce();
      if (length.denominator() != 1) {
            Fraction roundDown = Fraction(length.numerator() / length.denominator(), 128);
            Fraction roundUp = Fraction(length.numerator() / length.denominator() + 1, 128);
            // mDura is not an integer multiple of 1/128;
            // first check if the duration is larger than an integer multiple of 1/128
            // by an amount smaller than the minimum division resolution
            // in that case, round down (rounding errors have possibly occurred),
            // otherwise, round up
            if ((_divs > 0) && ((mDura - roundDown) < Fraction(1, 4 * _divs))) {
                  _logger->logError(QString("rounding down measure duration %1 to %2")
                                    .arg(qPrintable(mDura.print()), qPrintable(roundDown.print())),
                                    &_e);
                  correctedLength = roundDown;
                  }
            else {
                  _logger->logError(QString("rounding up measure duration %1 to %2")
                                    .arg(qPrintable(mDura.print()), qPrintable(roundUp.print())),
                                    &_e);
                  correctedLength = roundUp;
                  }
            mDura = correctedLength;
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

      addError(checkAtEndElement(_e, "measure"));
      }

//---------------------------------------------------------
//   print
//---------------------------------------------------------

void MusicXMLParserPass1::print(const int measureNr)
      {
      _logger->logDebugTrace("MusicXMLParserPass1::print", &_e);

      const QString newPage = _e.attributes().value("new-page").toString();
      const QString newSystem = _e.attributes().value("new-system").toString();
      if (newPage == "yes")
            _pageStartMeasureNrs.insert(measureNr);
      if (newSystem == "yes")
            _systemStartMeasureNrs.insert(measureNr);

      _e.skipCurrentElement();        // skip but don't log
      }

//---------------------------------------------------------
//   attributes
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes node.
 */

void MusicXMLParserPass1::attributes(const QString& partId, const Fraction cTime)
      {
      _logger->logDebugTrace("MusicXMLParserPass1::attributes", &_e);

      int staves = 0;
      std::set<int> hiddenStaves = {};

      while (_e.readNextStartElement()) {
            if (_e.name() == "clef")
                  clef(partId);
            else if (_e.name() == "divisions")
                  divisions();
            else if (_e.name() == "key")
                  _e.skipCurrentElement();  // skip but don't log
            else if (_e.name() == "instruments")
                  _e.skipCurrentElement();  // skip but don't log
            else if (_e.name() == "staff-details") {
                  if (_e.attributes().value("print-object") == "no")
                        hiddenStaves.emplace(_e.attributes().value("number").toInt());
                  _e.skipCurrentElement();
                  }
            else if (_e.name() == "staves")
                  staves = _e.readElementText().toInt();
            else if (_e.name() == "time")
                  time(cTime);
            else if (_e.name() == "transpose")
                  transpose(partId, cTime);
            else
                  skipLogCurrElem();
            }

      if (staves - static_cast<int>(hiddenStaves.size()) > MAX_VOICE_DESC_STAVES) {
            _logger->logError("staves exceed MAX_VOICE_DESC_STAVES, even when discarding hidden staves", &_e);
            return;
            }
      else if (staves > MAX_VOICE_DESC_STAVES
               && static_cast<int>(hiddenStaves.size()) > 0
               && _parts[partId].staffNumberToIndex().size() == 0) {
            _logger->logError("staves exceed MAX_VOICE_DESC_STAVES, but hidden staves can be discarded", &_e);
            // Some scores have parts with many staves (~10), but most are hidden
            // When this occurs, we can discard hidden staves
            // and store a QMap between staffNumber and staffIndex.
            int staffNumber = 1;
            int staffIndex = 0;
            for (; staffNumber <= staves; ++staffNumber) {
                  if (hiddenStaves.find(staffNumber) != hiddenStaves.end()) {
                        _logger->logError(QString("removing hidden staff %1").arg(staffNumber), &_e);
                        continue;
                        }
                  _parts[partId].insertStaffNumberToIndex(staffNumber, staffIndex);
                  ++staffIndex;
                  }
            Q_ASSERT(staffIndex == _parts[partId].staffNumberToIndex().size());

            setNumberOfStavesForPart(_partMap.value(partId), staves - static_cast<int>(hiddenStaves.size()));
            }
      else {
            // Otherwise, don't discard any staves
            // And set hidden staves to HideMode::AUTO
            // (MuseScore doesn't currently have a mechanism
            // for hiding non-empty staves, so this is an approximation
            // of the correct implementation)
            setNumberOfStavesForPart(_partMap.value(partId), staves);
            for (int hiddenStaff : hiddenStaves) {
                  int hiddenStaffIndex = _parts.value(partId).staffNumberToIndex(hiddenStaff);
                  if (hiddenStaffIndex >= 0)
                        _partMap.value(partId)->staff(hiddenStaffIndex)->setHideWhenEmpty(Staff::HideMode::AUTO);
                  }
            }
      }

//---------------------------------------------------------
//   clef
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes/clef node.
 TODO: Store the clef type, to simplify staff type setting in pass 2.
 */

void MusicXMLParserPass1::clef(const QString& partId)
      {
      _logger->logDebugTrace("MusicXMLParserPass1::clef", &_e);

      while (_e.readNextStartElement()) {
            if (_e.name() == "line")
                  _e.skipCurrentElement();  // skip but don't log
            else if (_e.name() == "sign") {
                  QString sign = _e.readElementText();
                  if (sign == "TAB")
                        _parts[partId].hasTab(true);
                  }
            else
                  skipLogCurrElem();
            }
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

static bool determineTimeSig(MxmlLogger* logger, const QXmlStreamReader* const xmlreader,
                             const QString beats, const QString beatType, const QString timeSymbol,
                             TimeSigType& st, int& bts, int& btp)
      {
      // initialize
      st  = TimeSigType::NORMAL;
      bts = 0;             // the beats (max 4 separated by "+") as integer
      btp = 0;             // beat-type as integer
      // determine if timesig is valid
      if (timeSymbol == "cut")
            st = TimeSigType::ALLA_BREVE;
      else if (timeSymbol == "common")
            st = TimeSigType::FOUR_FOUR;
      else if (!timeSymbol.isEmpty() && timeSymbol != "normal") {
            logger->logError(QString("time symbol '%1' not recognized")
                             .arg(timeSymbol), xmlreader);
            return false;
            }

      btp = beatType.toInt();
      QStringList list = beats.split("+");
      for (int i = 0; i < list.size(); i++)
            bts += list.at(i).toInt();

      // determine if bts and btp are valid
      if (bts <= 0 || btp <=0) {
            logger->logError(QString("beats=%1 and/or beat-type=%2 not recognized")
                             .arg(beats, beatType), xmlreader);
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

void MusicXMLParserPass1::time(const Fraction cTime)
      {

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

      if (!beats.isEmpty() && !beatType.isEmpty()) {
            // determine if timesig is valid
            TimeSigType st  = TimeSigType::NORMAL;
            int bts = 0;       // total beats as integer (beats may contain multiple numbers, separated by "+")
            int btp = 0;       // beat-type as integer
            if (determineTimeSig(_logger, &_e, beats, beatType, timeSymbol, st, bts, btp)) {
                  _timeSigDura = Fraction(bts, btp);
                  _score->sigmap()->add(cTime.ticks(), _timeSigDura);
                  }
            }
      }

//---------------------------------------------------------
//   transpose
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes/transpose node.
 */

void MusicXMLParserPass1::transpose(const QString& partId, const Fraction& tick)
      {
      Interval interval;
      while (_e.readNextStartElement()) {
            int i = _e.readElementText().toInt();
            if (_e.name() == "diatonic") {
                  interval.diatonic = i;
                  }
            else if (_e.name() == "chromatic") {
                  interval.chromatic = i;
                  }
            else if (_e.name() == "octave-change") {
                  interval.diatonic += i * 7;
                  interval.chromatic += i * 12;
                  }
            else
                  skipLogCurrElem();
            }

      if (_parts[partId]._intervals.count(tick) == 0) {
            if (!interval.diatonic)
                  interval.diatonic = chromatic2diatonic(interval.chromatic);
            _parts[partId]._intervals[tick] = interval;
            }
      else
            qDebug("duplicate transpose at tick %s", qPrintable(tick.print()));
      }

//---------------------------------------------------------
//   divisions
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes/divisions node.
 */

void MusicXMLParserPass1::divisions()
      {
      _divs = _e.readElementText().toInt();
      if (!(_divs > 0))
            _logger->logError("illegal divisions", &_e);
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
                  staff = _parts[partId].staffNumberToIndex(strStaff.toInt());
                  if (0 <= staff && staff < nstaves)
                        ;  //qDebug("direction staff %d", staff + 1);
                  else {
                        _logger->logError(QString("invalid staff %1").arg(strStaff), &_e);
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
                        _parts[partId].addOctaveShift(staff, prevDesc.size, prevDesc.time);
                        _parts[partId].addOctaveShift(staff, -prevDesc.size, desc.time);
                        }
                  else
                        _logger->logError("double octave-shift stop", &_e);
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
                        _parts[partId].addOctaveShift(staff, desc.size, desc.time);
                        _parts[partId].addOctaveShift(staff, -desc.size, prevDesc.time);
                        }
                  else
                        _logger->logError("double octave-shift start", &_e);
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
      while (_e.readNextStartElement()) {
            if (_e.name() == "octave-shift") {
                  QString number = _e.attributes().value("number").toString();
                  int n = 0;
                  if (!number .isEmpty()) {
                        n = number.toInt();
                        if (n <= 0)
                              _logger->logError(QString("invalid number %1").arg(number), &_e);
                        else
                              n--;  // make zero-based
                        }

                  if (0 <= n && n < MAX_NUMBER_LEVEL) {
                        short size = _e.attributes().value("size").toShort();
                        QString type = _e.attributes().value("type").toString();
                        //qDebug("octave-shift type '%s' size %d number %d", qPrintable(type), size, n);
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
                        _logger->logError(QString("invalid octave-shift number %1").arg(number), &_e);
                        }
                  _e.skipCurrentElement();
                  }
            else
                  _e.skipCurrentElement();
            }
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
                  _logger->logError(QString("invalid octave-shift size %1").arg(size), &_e);
                  return;
            }

      if (!cTime.isValid() || cTime < Fraction(0, 1))
            _logger->logError("invalid current time", &_e);

      if (type == "up")
            tp = MxmlOctaveShiftDesc::Type::UP;
      else if (type == "down") {
            tp = MxmlOctaveShiftDesc::Type::DOWN;
            sz *= -1;
            }
      else if (type == "stop")
            tp = MxmlOctaveShiftDesc::Type::STOP;
      else {
            _logger->logError(QString("invalid octave-shift type '%1'").arg(type), &_e);
            return;
            }

      desc = MxmlOctaveShiftDesc(tp, sz, cTime);
      }

//---------------------------------------------------------
//   notations
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations node.
 */

void MusicXMLParserPass1::notations(MxmlStartStop& tupletStartStop)
      {
      //_logger->logDebugTrace("MusicXMLParserPass1::note", &_e);

      while (_e.readNextStartElement()) {
            if (_e.name() == "tuplet") {
                  QString tupletType       = _e.attributes().value("type").toString();

                  // ignore possible children (currently not supported)
                  _e.skipCurrentElement();

                  if (tupletType == "start")
                        tupletStartStop = MxmlStartStop::START;
                  else if (tupletType == "stop")
                        tupletStartStop = MxmlStartStop::STOP;
                  else if (!tupletType.isEmpty() && tupletType != "start" && tupletType != "stop") {
                        _logger->logError(QString("unknown tuplet type '%1'").arg(tupletType), &_e);
                        }
                  }
            else {
                  _e.skipCurrentElement();        // skip but don't log
                  }
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

static void smallestTypeAndCount(const TDuration durType, int& type, int& count)
      {
      type = int(durType.type());
      count = 1;
      switch (durType.dots()) {
            case 0:
                  // nothing to do
                  break;
            case 1:
                  type += 1;       // next-smaller type
                  count = 3;
                  break;
            case 2:
                  type += 2;       // next-next-smaller type
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
//   addDurationToTuplet
//---------------------------------------------------------

/**
 Add duration to tuplet duration
 Determine type and number of smallest notes in the tuplet
 */

void MxmlTupletState::addDurationToTuplet(const Fraction duration, const Fraction timeMod)
      {
      /*
      qDebug("1 duration %s timeMod %s -> state.tupletType %d state.tupletCount %d state.actualNotes %d state.normalNotes %d",
             qPrintable(duration.print()),
             qPrintable(timeMod.print()),
             m_tupletType,
             m_tupletCount,
             m_actualNotes,
             m_normalNotes
             );
      */
      if (m_duration <= Fraction(0, 1)) {
            // first note: init variables
            m_actualNotes = timeMod.denominator();
            m_normalNotes = timeMod.numerator();
            smallestTypeAndCount(duration / timeMod, m_tupletType, m_tupletCount);
            }
      else {
            int noteType = 0;
            int noteCount = 0;
            smallestTypeAndCount(duration / timeMod, noteType, noteCount);
            // match the types
            matchTypeAndCount(m_tupletType, m_tupletCount, noteType, noteCount);
            m_tupletCount += noteCount;
            }
      m_duration += duration;
      /*
      qDebug("2 duration %s -> state.tupletType %d state.tupletCount %d state.actualNotes %d state.normalNotes %d",
             qPrintable(duration.print()),
             m_tupletType,
             m_tupletCount,
             m_actualNotes,
             m_normalNotes
             );
      */
      }

//---------------------------------------------------------
//   determineTupletFractionAndFullDuration
//---------------------------------------------------------

/**
 Split duration into two factors where fullDuration is note sized
 (i.e. the denominator is a power of 2), 1/2 < fraction <= 1/1
 and fraction * fullDuration equals duration.
 */

void determineTupletFractionAndFullDuration(const Fraction duration, Fraction& fraction, Fraction& fullDuration)
      {
      fraction = duration;
      fullDuration = Fraction(1, 1);
      // move denominator's powers of 2 from fraction to fullDuration
      while (fraction.denominator() % 2 == 0) {
            fraction *= 2;
            fraction.reduce();
            fullDuration *= Fraction(1, 2);
            }
      // move numerator's powers of 2 from fraction to fullDuration
      while ( fraction.numerator() % 2 == 0) {
            fraction *= Fraction(1, 2);
            fraction.reduce();
            fullDuration *= 2;
            fullDuration.reduce();
            }
      // make sure 1/2 < fraction <= 1/1
      while (fraction <= Fraction(1, 2)) {
            fullDuration *= Fraction(1, 2);
            fraction *= 2;
            }
      fullDuration.reduce();
      fraction.reduce();

      /*
      Examples (note result when denominator is not a power of two):
      3:2 tuplet of 1/4 results in fraction 1/1 and fullDuration 1/2
      2:3 tuplet of 1/4 results in fraction 3/1 and fullDuration 1/4
      4:3 tuplet of 1/4 results in fraction 3/1 and fullDuration 1/4
      3:4 tuplet of 1/4 results in fraction 1/1 and fullDuration 1/1

       Bring back fraction in 1/2 .. 1/1 range.
       */

      if (fraction > Fraction(1, 1) && fraction.denominator() == 1) {
            fullDuration *= fraction;
            fullDuration.reduce();
            fraction = Fraction(1, 1);
            }

      /*
      qDebug("duration %s fraction %s fullDuration %s",
             qPrintable(duration.toString()),
             qPrintable(fraction.toString()),
             qPrintable(fullDuration.toString())
             );
      */
      }

//---------------------------------------------------------
//   isTupletFilled
//---------------------------------------------------------

/**
 Determine if the tuplet is completely filled,
 because either (1) it is at least the same duration
 as the specified number of the specified normal type notes
 or (2) the duration adds up to a normal note duration.

 Example (1): a 3:2 tuplet with a 1/4 and a 1/8 note
 is filled if normal type is 1/8,
 it is not filled if normal type is 1/4.

 Example (2): a 3:2 tuplet with a 1/4 and a 1/8 note is filled.
 */

static bool isTupletFilled(const MxmlTupletState& state, const TDuration normalType, const Fraction timeMod)
      {
      Q_UNUSED(timeMod);
      bool res { false };
      const auto actualNotes = state.m_actualNotes;
      /*
      const auto normalNotes = state.m_normalNotes;
      qDebug("duration %s normalType %s timeMod %s normalNotes %d actualNotes %d",
             qPrintable(state.m_duration.toString()),
             qPrintable(normalType.fraction().toString()),
             qPrintable(timeMod.toString()),
             normalNotes,
             actualNotes
             );
      */

      auto tupletType = state.m_tupletType;
      auto tupletCount = state.m_tupletCount;

      if (normalType.isValid()) {
            int matchedNormalType  = int(normalType.type());
            int matchedNormalCount = actualNotes;
            // match the types
            matchTypeAndCount(tupletType, tupletCount, matchedNormalType, matchedNormalCount);
            // ... result scenario (1)
            res = tupletCount >= matchedNormalCount;
            /*
            qDebug("normalType valid tupletType %d tupletCount %d matchedNormalType %d matchedNormalCount %d res %d",
                   tupletType,
                   tupletCount,
                   matchedNormalType,
                   matchedNormalCount,
                   res
                   );
             */
            }
      else {
            // ... result scenario (2)
            res = tupletCount >= actualNotes;
            /*
            qDebug("normalType not valid tupletCount %d actualNotes %d res %d",
                   tupletCount,
                   actualNotes,
                   res
                   );
             */
            }
      return res;
      }

//---------------------------------------------------------
//   missingTupletDuration
//---------------------------------------------------------

Fraction missingTupletDuration(const Fraction duration)
      {
      Fraction tupletFraction;
      Fraction tupletFullDuration;

      determineTupletFractionAndFullDuration(duration, tupletFraction, tupletFullDuration);
      auto missing = (Fraction(1, 1) - tupletFraction) * tupletFullDuration;

      return missing;
      }

//---------------------------------------------------------
//   voiceToInt
//---------------------------------------------------------

int MusicXMLParserPass1::voiceToInt(const QString& voice)
      {
      bool ok;
      int voiceInt = voice.toInt(&ok);
      if (voice.isEmpty())
            voiceInt = 1;
      else if (!ok)
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
            voiceInt = qHash(voice);  // Handle the rare but technically in-spec case of a non-int voice
#else //seeding with a real magic value ;-)
            voiceInt = qHash(voice, 42);  // Handle the rare but technically in-spec case of a non-int voice
#endif
      return voiceInt;
      }

//---------------------------------------------------------
//   determineTupletAction
//---------------------------------------------------------

/**
 Update tuplet state using parse result tupletDesc.
 Tuplets with <actual-notes> and <normal-notes> but without <tuplet>
 are handled correctly.
 TODO Nested tuplets are not (yet) supported.
 */

MxmlTupletFlags MxmlTupletState::determineTupletAction(const Fraction noteDuration,
                                                       const Fraction timeMod,
                                                       const MxmlStartStop tupletStartStop,
                                                       const TDuration normalType,
                                                       Fraction& missingPreviousDuration,
                                                       Fraction& missingCurrentDuration
                                                       )
      {
      const auto actualNotes = timeMod.denominator();
      const auto normalNotes = timeMod.numerator();
      MxmlTupletFlags res = MxmlTupletFlag::NONE;

      // check for unexpected termination of previous tuplet
      if (m_inTuplet && timeMod == Fraction(1, 1)) {
            // recover by simply stopping the current tuplet first
            if (!isTupletFilled(*this, normalType, timeMod)) {
                  missingPreviousDuration = missingTupletDuration(m_duration);
                  //qDebug("tuplet incomplete, missing %s", qPrintable(missingPreviousDuration.print()));
                  }
            *this = {};
            res |= MxmlTupletFlag::STOP_PREVIOUS;
            }

      // check for obvious errors
      if (m_inTuplet && tupletStartStop == MxmlStartStop::START) {
            qDebug("tuplet already started");
            // recover by simply stopping the current tuplet first
            if (!isTupletFilled(*this, normalType, timeMod)) {
                  missingPreviousDuration = missingTupletDuration(m_duration);
                  //qDebug("tuplet incomplete, missing %s", qPrintable(missingPreviousDuration.print()));
                  }
            *this = {};
            res |= MxmlTupletFlag::STOP_PREVIOUS;
            }
      if (tupletStartStop == MxmlStartStop::STOP && !m_inTuplet) {
            qDebug("tuplet stop but no tuplet started");       // TODO
            // recovery handled later (automatically, no special case needed)
            }

      // Tuplet are either started by the tuplet start
      // or when the time modification is first found.
      if (!m_inTuplet) {
            if (tupletStartStop == MxmlStartStop::START
                || (!m_inTuplet && (actualNotes != 1 || normalNotes != 1))) {
                  if (tupletStartStop != MxmlStartStop::START) {
                        m_implicit = true;
                        }
                  else {
                        m_implicit = false;
                        }
                  // create a new tuplet
                  m_inTuplet = true;
                  res |= MxmlTupletFlag::START_NEW;
                  }
            }

      // Add chord to the current tuplet.
      // Must also check for actual/normal notes to prevent
      // adding one chord too much if tuplet stop is missing.
      if (m_inTuplet && !(actualNotes == 1 && normalNotes == 1)) {
            addDurationToTuplet(noteDuration, timeMod);
            res |= MxmlTupletFlag::ADD_CHORD;
            }

      // Tuplets are stopped by the tuplet stop
      // or when the tuplet is filled completely
      // (either with knowledge of the normal type
      // or as a last resort calculated based on
      // actual and normal notes plus total duration)
      // or when the time-modification is not found.

      if (m_inTuplet) {
            if (tupletStartStop == MxmlStartStop::STOP
                || (m_implicit && isTupletFilled(*this, normalType, timeMod))
                || (actualNotes == 1 && normalNotes == 1)) {       // incorrect ??? check scenario incomplete tuplet w/o start
                  if (actualNotes > normalNotes && !isTupletFilled(*this, normalType, timeMod)) {
                        missingCurrentDuration = missingTupletDuration(m_duration);
                        qDebug("current tuplet incomplete, missing %s", qPrintable(missingCurrentDuration.print()));
                        }

                  *this = {};
                  res |= MxmlTupletFlag::STOP_CURRENT;
                  }
            }

      return res;
      }

//---------------------------------------------------------
//   note
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note node.
 */

void MusicXMLParserPass1::note(const QString& partId,
                               const Fraction sTime,
                               Fraction& missingPrev,
                               Fraction& dura,
                               Fraction& missingCurr,
                               VoiceOverlapDetector& vod,
                               MxmlTupletStates& tupletStates)
      {
      //_logger->logDebugTrace("MusicXMLParserPass1::note", &_e);

      if (_e.attributes().value("print-spacing") == "no") {
            notePrintSpacingNo(dura);
            return;
            }

      //float alter = 0;
      bool chord = false;
      bool grace = false;
      //int octave = -1;
      bool bRest = false;
      int staff = 0;
      //int step = 0;
      QString type;
      QString voice = "1";
      QString instrId;
      MxmlStartStop tupletStartStop { MxmlStartStop::NONE };

      mxmlNoteDuration mnd(_divs, _logger, this);

      while (_e.readNextStartElement()) {
            if (mnd.readProperties(_e)) {
                  // element handled
                  }
            else if (_e.name() == "accidental")
                  _e.skipCurrentElement();  // skip but don't log
            else if (_e.name() == "beam") {
                  _hasBeamingInfo = true;
                  _e.skipCurrentElement();  // skip but don't log
                  }
            else if (_e.name() == "chord") {
                  chord = true;
                  _e.skipCurrentElement();  // skip but don't log
                  }
            else if (_e.name() == "cue")
                  _e.skipCurrentElement();  // skip but don't log
            else if (_e.name() == "grace") {
                  grace = true;
                  _e.skipCurrentElement();  // skip but don't log
                  }
            else if (_e.name() == "instrument") {
                  instrId = _e.attributes().value("id").toString();
                  _e.skipCurrentElement();  // skip but don't log
                  }
            else if (_e.name() == "lyric") {
                  const auto number = _e.attributes().value("number").toString();
                  _parts[partId].lyricNumberHandler().addNumber(number);
                  _parts[partId].hasLyrics(true);
                  _e.skipCurrentElement();
                  }
            else if (_e.name() == "notations")
                  notations(tupletStartStop);
            else if (_e.name() == "notehead")
                  _e.skipCurrentElement();  // skip but don't log
            else if (_e.name() == "pitch")
                  _e.skipCurrentElement();  // skip but don't log
            else if (_e.name() == "rest") {
                  bRest = true;
                  rest();
                  }
            else if (_e.name() == "staff") {
                  auto ok = false;
                  auto strStaff = _e.readElementText();
                  staff = _parts[partId].staffNumberToIndex(strStaff.toInt(&ok));
                  _parts[partId].setMaxStaff(staff);
                  Part* part = _partMap.value(partId);
                  if (!part)
                        continue;
                  if (!ok || staff < 0 || staff >= part->nstaves())
                        _logger->logError(QString("illegal or hidden staff '%1'").arg(strStaff), &_e);
                  }
            else if (_e.name() == "stem")
                  _e.skipCurrentElement();  // skip but don't log
            else if (_e.name() == "tie")
                  _e.skipCurrentElement();  // skip but don't log
            else if (_e.name() == "type")
                  type = _e.readElementText();
            else if (_e.name() == "unpitched")
                  _e.skipCurrentElement();  // skip but don't log
            else if (_e.name() == "voice")
                  voice = _e.readElementText();
            else
                  skipLogCurrElem();
            }

      // multi-instrument handling
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

      // check for timing error(s) and set dura
      // keep in this order as checkTiming() might change dura
      auto errorStr = mnd.checkTiming(type, bRest, grace);
      dura = mnd.duration();
      if (!errorStr.isEmpty())
            _logger->logError(errorStr, &_e);

      // don't count chord or grace note duration
      // note that this does not check the MusicXML requirement that notes in a chord
      // cannot have a duration longer than the first note in the chord
      missingPrev.set(0, 1);
      if (chord || grace)
            dura.set(0, 1);

      if (!chord && !grace) {
            // do tuplet
            auto timeMod = mnd.timeMod();
            auto& tupletState = tupletStates[voice];
            tupletState.determineTupletAction(mnd.duration(), timeMod, tupletStartStop, mnd.normalType(), missingPrev, missingCurr);
            }

      // store result
      if (dura.isValid() && dura > Fraction(0, 1)) {
            // count the chords
            int voiceInt = voiceToInt(voice);
            if (!_parts.value(partId).voicelist.contains(voiceInt)) {
                  VoiceDesc vs;
                  _parts[partId].voicelist.insert(voiceInt, vs);
                  }
            _parts[partId].voicelist[voiceInt].incrChordRests(staff);
            // determine note length for voiceInt overlap detection
            vod.addNote((sTime + missingPrev).ticks(), (sTime + missingPrev + dura).ticks(), voiceInt, staff);
            }

      addError(checkAtEndElement(_e, "note"));
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
      //_logger->logDebugTrace("MusicXMLParserPass1::notePrintSpacingNo", &_e);

      bool chord = false;
      bool grace = false;

      while (_e.readNextStartElement()) {
            if (_e.name() == "chord") {
                  chord = true;
                  _e.skipCurrentElement();  // skip but don't log
                  }
            else if (_e.name() == "duration")
                  duration(dura);
            else if (_e.name() == "grace") {
                  grace = true;
                  _e.skipCurrentElement();  // skip but don't log
                  }
            else
                  _e.skipCurrentElement();        // skip but don't log
            }

      // don't count chord or grace note duration
      // note that this does not check the MusicXML requirement that notes in a chord
      // cannot have a duration longer than the first note in the chord
      if (chord || grace)
            dura.set(0, 1);
      }

//---------------------------------------------------------
//   calcTicks
//---------------------------------------------------------

Fraction MusicXMLParserPass1::calcTicks(const int& intTicks, const int& _divisions, const QXmlStreamReader* const xmlReader)
      {
      Fraction dura(0, 1);              // invalid unless set correctly

      if (_divisions > 0) {
            dura.set(intTicks, 4 * _divisions);
            dura.reduce(); // prevent overflow in later Fraction operations

            // Correct for previously adjusted durations
            // This is necessary when certain tuplets are
            // followed by a <backup> element.
            // There are two strategies:
            // 1. Use a lookup table of previous adjustments
            // 2. Check if within maxDiff of a seenDenominator
            if (_adjustedDurations.contains(dura)) {
                  dura = _adjustedDurations.value(dura);
                  }
            else if (dura.reduced().denominator() > 64) {
                  for (auto seenDenominator : _seenDenominators) {
                        int seenDenominatorTicks = Fraction(1, seenDenominator).ticks();
                        if (qAbs(dura.ticks() % seenDenominatorTicks) <= _maxDiff) {
                              Fraction roundedDura = Fraction(std::round(dura.ticks() / double(seenDenominatorTicks)), seenDenominator);
                              roundedDura.reduce();
                              _logger->logError(QString("calculated duration (%1) assumed to be a rounding error by proximity to (%2)")
                                                .arg(dura.toString(), roundedDura.toString()));
                              insertAdjustedDuration(dura, roundedDura);
                              dura = roundedDura;
                              break;
                              }
                        }
                  }
            }
      else
            _logger->logError("illegal or uninitialized divisions", xmlReader);
      //qDebug("duration %s valid %d", qPrintable(dura.print()), dura.isValid());

      return dura;
      }

//---------------------------------------------------------
//   duration
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/duration node.
 */

void MusicXMLParserPass1::duration(Fraction& dura, QXmlStreamReader& e)
      {
      Q_ASSERT(e.isStartElement() && e.name() == "duration");
      _logger->logDebugTrace("MusicXMLParserPass1::duration", &e);

      dura.set(0, 0);  // invalid unless set correctly
      int intDura = e.readElementText().toInt();
      dura = calcTicks(intDura);
      }

//---------------------------------------------------------
//   forward
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/forward node.
 */

void MusicXMLParserPass1::forward(Fraction& dura)
      {
      //_logger->logDebugTrace("MusicXMLParserPass1::forward", &_e);

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
      //_logger->logDebugTrace("MusicXMLParserPass1::backup", &_e);

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
      //_logger->logDebugTrace("MusicXMLParserPass1::timeModification", &_e);

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
            _logger->logError(QString("illegal time-modification: actual-notes %1 normal-notes %2")
                              .arg(strActual, strNormal), &_e);
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
      //_logger->logDebugTrace("MusicXMLParserPass1::rest", &_e);

      while (_e.readNextStartElement()) {
            if (_e.name() == "display-octave")
                  _e.skipCurrentElement();  // skip but don't log
            else if (_e.name() == "display-step")
                  _e.skipCurrentElement();  // skip but don't log
            else
                  skipLogCurrElem();
            }
      }

} // namespace Ms
