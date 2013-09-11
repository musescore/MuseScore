//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "importxmlfirstpass.h"
#include "musicxmlsupport.h"

#include "libmscore/fraction.h"
#include "libmscore/mscore.h"
#include "libmscore/timesig.h"

namespace Ms {

/****************************************************************************
**
** begin code copied and adapted from MuseScores xml.cpp
**
****************************************************************************/

//---------------------------------------------------------
//   printDomElementPath
//---------------------------------------------------------

static QString domElementPath(const QDomElement e)
      {
      QString s;
      QDomNode dn(e);
      while (!dn.parentNode().isNull()) {
            dn = dn.parentNode();
            const QDomElement& ee = dn.toElement();
            const QString k(ee.tagName());
            if (!s.isEmpty())
                  s += ":";
            s += k;
            }
      return s;
      }

//---------------------------------------------------------
//   domError
//---------------------------------------------------------

static void aaadomError(const QDomElement e)
      {
      QString m;
      QString s = domElementPath(e);
      /*
      if (!docName.isEmpty())
            m = QString("<%1>:").arg(docName);
      */
      int ln = e.lineNumber();
      if (ln != -1)
            m += QString("line:%1 ").arg(ln);
      int col = e.columnNumber();
      if (col != -1)
            m += QString("col:%1 ").arg(col);
      m += QString("%1: Unknown Node <%2>, type %3").arg(s).arg(e.tagName()).arg(e.nodeType());
      if (e.isText())
            m += QString("  text node <%1>").arg(e.toText().data());
      qDebug("%s", qPrintable(m));
      }

/****************************************************************************
**
** end code copied and adapted from MuseScores xml.cpp
**
****************************************************************************/

/****************************************************************************
**
** begin code copied and adapted from MuseScores importxml.cpp
**
****************************************************************************/

//---------------------------------------------------------
//   local defines for debug output
//---------------------------------------------------------

// #define DEBUG_VOICE_MAPPER true
// #define DEBUG_TICK true

//---------------------------------------------------------
//   noteDurationAsFraction
//---------------------------------------------------------

/**
 Determine note duration as fraction based on type, dots and tuplet.
 Also return duration as fraction (calculated from the <duration> element).
 Input e is the note element.
 If chord or grace, duration is 0.
 */

static Fraction noteDurationAsFraction(const int divisions, QDomElement e, Fraction& duration, QString& noteDurDesc)
      {
      int actualNotes = 0;
      bool chord = false;
      int dots  = 0;
      bool grace = false;
      int normalNotes = 0;
      bool rest = false;
      QString type;
      noteDurDesc = "";
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "chord")
                  chord = true;
            else if (e.tagName() == "dot")
                  dots++;
            else if (e.tagName() == "duration") {
                  duration = MxmlSupport::durationAsFraction(divisions, e);
                  }
            else if (e.tagName() == "grace")
                  grace = true;
            else if (e.tagName() == "rest")
                  rest = true;
            else if (e.tagName() == "time-modification") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "actual-notes") {
                              bool ok;
                              actualNotes = MxmlSupport::stringToInt(ee.text(), &ok);
                              if (!ok || divisions <= 0)
                                    qDebug("MusicXml-Import: bad actual-notes value: <%s>",
                                           qPrintable(ee.text()));
                              }
                        if (ee.tagName() == "normal-notes") {
                              bool ok;
                              normalNotes = MxmlSupport::stringToInt(ee.text(), &ok);
                              if (!ok || divisions <= 0)
                                    qDebug("MusicXml-Import: bad normal-notes value: <%s>",
                                           qPrintable(ee.text()));
                              }
                        }
                  }
            else if (e.tagName() == "type")
                  type = e.text();
            }

      // if chord or grace, duration is 0
      if (chord || grace) {
            // overrule duration
            duration = Fraction(0, 1);
            // return 0/1
            return Fraction(0, 1);
            }

      // printable representations for specified and calculated duration
      // specified duration
      noteDurDesc += "dur ";
      noteDurDesc += duration.print();
      // calculated duration
      noteDurDesc += " calcdur '";
      noteDurDesc += MxmlSupport::noteTypeToFraction(type).print();
      for (int i = 0; i < dots; ++i) noteDurDesc += ".";
      if (actualNotes || normalNotes) noteDurDesc += QString(" norm/act %1/%2").arg(normalNotes).arg(actualNotes);

      // calculate note duration as fraction based on type, dots, normal and actual notes
      Fraction f = MxmlSupport::calculateFraction(type, dots, normalNotes, actualNotes);
      noteDurDesc += " -> ";
      if (f.isValid())
            noteDurDesc += f.print();
      else
            noteDurDesc += "invalid";
      noteDurDesc += "'";


#ifdef DEBUG_TICK
      qDebug("time-in-fraction: %s", qPrintable(noteDurDesc));
#endif

      if (rest && (type == "" || type == "whole"))
            // accept typeless and "whole" measure rests
            return duration;

      return f;
      }

//---------------------------------------------------------
//   moveTick
//---------------------------------------------------------

/**
 Move tick by amount specified in the element e, which must be
 a forward, backup or note.
 */

// TODO: error handling for invalid <duration>
// either here or in durationAsFraction

static void aaamoveTick(Fraction& tick, Fraction& maxtick,
                        const int divisions, const QDomElement e,
                        Fraction& duration, QString& noteDurDesc, QString& errorStr)
      {
      if (e.tagName() == "forward") {
            for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                  if (ee.tagName() == "duration") {
                        Fraction f = MxmlSupport::durationAsFraction(divisions, ee);
#ifdef DEBUG_TICK
                        qDebug("forward %s", qPrintable(f.print()));
#endif
                        tick += f;
                        if (tick > maxtick)
                              maxtick = tick;
                        }
                  else if (ee.tagName() == "voice")
                        ;
                  else if (ee.tagName() == "staff")
                        ;
                  else
                        aaadomError(ee);
                  }
            }
      else if (e.tagName() == "backup") {
            for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                  if (ee.tagName() == "duration") {
                        Fraction f = MxmlSupport::durationAsFraction(divisions, ee);
#ifdef DEBUG_TICK
                        qDebug("backup %s", qPrintable(f.print()));
#endif
                        if (f > tick) {
                              tick = 0;
                              errorStr = "backup beyond start of measure";
                              noteDurDesc = f.print();
                              }
                        else
                              tick -= f;
                        }
                  else
                        aaadomError(ee);
                  }
            }
      else if (e.tagName() == "note") {
            Fraction fraction = noteDurationAsFraction(divisions, e, duration, noteDurDesc);
#ifdef DEBUG_TICK
            qDebug("note %s %s", qPrintable(fraction.print()), qPrintable(duration.print()));
#endif
            // determine new tick, prefer specified duration over calculated duration
            if (duration.isValid() && fraction.isValid()) {
                  if (duration != fraction)
                        errorStr = "calculated duration not equal to specified duration";
                  tick += duration;
                  }
            else if (duration.isValid()) {
                  errorStr = "calculated duration invalid, using specified duration";
                  tick += duration;
                  }
            else if (fraction.isValid()) {
                  errorStr = "specified duration invalid, using calculated duration";
                  tick += fraction;
                  }
            else
                  errorStr = "calculated and specified duration invalid";

            if (tick > maxtick)
                  maxtick = tick;
            }
      }


MusicXmlPart::MusicXmlPart(QString id, QString name)
      : id(id), name(name)
      {
      // nothing yet
      }


void MusicXmlPart::addMeasureNumberAndDuration(QString measureNumber, Fraction measureDuration)
      {
      measureNumbers.append(measureNumber);
      measureDurations.append(measureDuration);
      }

Fraction MusicXmlPart::measureDuration(int i) const
      {
      if (i >= 0 && i < measureDurations.size())
            return measureDurations.at(i);
      return Fraction(0, 0); // return invalid fraction
      }

QString MusicXmlPart::toString() const
      {
      QString res;
      res = QString("part id '%1' name '%2'\n").arg(id).arg(name);

      for (QMap<int, VoiceDesc>::const_iterator i = voicelist.constBegin(); i != voicelist.constEnd(); ++i) {
            res += QString("voice %1 map staff data %2\n")
                  .arg(i.key() + 1)
                  .arg(i.value().toString());
            }

      for (int i = 0; i < measureNumbers.size(); ++i) {
            if (i > 0)
                  res += "\n";
            res += QString("measure %1 duration %2 (%3)")
                  .arg(measureNumbers.at(i))
                  .arg(measureDurations.at(i).print())
                  .arg(measureDurations.at(i).ticks());
            }

      return res;
      }


MxmlReaderFirstPass::MxmlReaderFirstPass()
      {
      qDebug("MxmlReaderFirstPass::MxmlReaderFirstPass()");
      }


// allocate MuseScore staff to MusicXML voices
// for each staff, allocate at most VOICES voices to the staff
//
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

static void allocateStaves(QMap<int, VoiceDesc>& vcLst)
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
            int key = -1;
            for (QMap<int, VoiceDesc>::const_iterator j = vcLst.constBegin(); j != vcLst.constEnd(); ++j) {
                  if (!j.value().overlaps() && j.value().numberChordRests() > max && j.value().staff() == -1) {
                        max = j.value().numberChordRests();
                        key = j.key();
                        }
                  }
            if (key >= 0) {
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
                  int key = -1;
                  for (QMap<int, VoiceDesc>::const_iterator j = vcLst.constBegin(); j != vcLst.constEnd(); ++j) {
                        if (j.value().overlaps() && j.value().numberChordRests(h) > max && j.value().staffAlloc(h) == -1) {
                              max = j.value().numberChordRests(h);
                              key = j.key();
                              }
                        }
                  if (key >= 0) {
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


// allocate MuseScore voice to MusicXML voices
// for each staff, the voices are number 1, 2, 3, 4
// in the same order they are numbered in the MusicXML file

static void allocateVoices(QMap<int, VoiceDesc>& vcLst)
      {
      int nextVoice[MAX_STAVES]; // number of voices allocated on each staff
      for (int i = 0; i < MAX_STAVES; ++i)
            nextVoice[i] = 0;
      // handle regular (non-overlapping) voices
      // a voice is allocated on one specific staff
      for (QMap<int, VoiceDesc>::const_iterator i = vcLst.constBegin(); i != vcLst.constEnd(); ++i) {
            int staff = i.value().staff();
            int key   = i.key();
            if (staff >= 0) {
                  vcLst[key].setVoice(nextVoice[staff]);
                  nextVoice[staff]++;
                  }
            }
      // handle overlapping voices
      // each voice may be in every staff
      for (QMap<int, VoiceDesc>::const_iterator i = vcLst.constBegin(); i != vcLst.constEnd(); ++i) {
            for (int j = 0; j < MAX_STAVES; ++j) {
                  int staffAlloc = i.value().staffAlloc(j);
                  int key   = i.key();
                  if (staffAlloc >= 0) {
                        vcLst[key].setVoice(j, nextVoice[j]);
                        nextVoice[j]++;
                        }
                  }
            }
      }

//  copy the overlap data from the overlap detector to the voice list

static void copyOverlapData(VoiceOverlapDetector& vod, QMap<int, VoiceDesc>& vcLst)
      {
      for (QMap<int, VoiceDesc>::const_iterator i = vcLst.constBegin(); i != vcLst.constEnd(); ++i) {
            int key = i.key();
            if (vod.stavesOverlap(key))
                  vcLst[key].setOverlap(true);
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

/**
 Setup the voice mapper for a MusicXML part.
 In: e is the "part" node
 */

void MxmlReaderFirstPass::initVoiceMapperAndMapVoices(QDomElement e, int partNr)
      {
      VoiceOverlapDetector vod;
      int loc_divisions = -1;
      Fraction loc_tick;
      Fraction loc_maxtick;
      QString beats = "";
      QString beatType = "";
      int timeSigLen = -1; // measure length in ticks according to the last timesig read

      // count number of chordrests on each MusicXML staff
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "measure") {
                  Fraction measureStartTick = loc_tick;
                  QString measureNumber = e.attribute("number");
                  vod.newMeasure();
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "attributes") {
                              for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                                    if (eee.tagName() == "divisions") {
                                          bool ok;
                                          loc_divisions = MxmlSupport::stringToInt(eee.text(), &ok);
                                          if (!ok || loc_divisions <= 0)
                                                qDebug("MusicXml-Import: bad divisions value: <%s>",
                                                       qPrintable(eee.text()));
#ifdef DEBUG_TICK
                                          qDebug("measurelength loc_divisions %d", loc_divisions);
#endif
                                          }
                                    else if (eee.tagName() == "time") {
                                          for (QDomElement eeee = eee.firstChildElement(); !eeee.isNull(); eeee = eeee.nextSiblingElement()) {
                                                if (eeee.tagName() == "beats")
                                                      beats = eeee.text();
                                                else if (eeee.tagName() == "beat-type") {
                                                      beatType = eeee.text();
                                                      }
                                                else if (eeee.tagName() == "senza-misura")
                                                      ;
                                                else
                                                      domError(eeee);
                                                }
                                          if (beats != "" && beatType != "") {
                                                TimeSigType st = TSIG_NORMAL;
                                                int bts        = 0; // the beats (max 4 separated by "+") as integer
                                                int btp        = 0; // beat-type as integer
#ifdef DEBUG_TICK
                                                qDebug("measurelength beats %s beattype %s",
                                                       qPrintable(beats), qPrintable(beatType));
#endif
                                                if (determineTimeSig(beats, beatType, "", st, bts, btp)) {
                                                      Fraction f(bts, btp);
                                                      timeSigLen = f.ticks();
#ifdef DEBUG_TICK
                                                      qDebug("measurelength fraction %s len %d",
                                                             qPrintable(f.print()), timeSigLen);
#endif
                                                      }
                                                }
                                          }
                                    }
                              }
                        // most of following tags can only be handled if duration is valid
                        if (loc_divisions > 0) {
                              if (ee.tagName() == "note") {
                                    bool chord = false;
                                    bool grace = false;
                                    int voice = -1;
                                    QString pitch = "    ";
                                    int staff = -1;
                                    bool rest = false;
                                    for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                                          QString tag(eee.tagName());
                                          QString s(eee.text());
                                          if (tag == "chord")
                                                chord = true;
                                          else if (tag == "grace")
                                                grace = true;
                                          else if (tag == "voice")
                                                voice = s.toInt() - 1;
                                          else if (tag == "staff")
                                                staff = s.toInt() - 1;
                                          else if (tag == "pitch")
                                                ;  // TODO pitch = parsePitch(eee);
                                          else if (tag == "rest")
                                                rest = true;
                                          }
                                    if (rest)
                                          pitch = "rest";
                                    // set correct defaults for missing elements
                                    if (voice == -1) voice = 0;
                                    if (staff == -1) staff = 0;
                                    if (!chord) {
                                          // count the chords (only the first note in a chord is counted)
                                          if (0 <= staff && staff < MAX_STAVES) {
                                                if (!parts[partNr].voicelist.contains(voice)) {
                                                      VoiceDesc vs;
                                                      parts[partNr].voicelist.insert(voice, vs);
                                                      }
                                                parts[partNr].voicelist[voice].incrChordRests(staff);
                                                }
                                          // determine note length for voice overlap detection
                                          if (!grace) {
                                                Fraction startTick = loc_tick; // start tick for the last note
                                                Fraction duration;
                                                QString noteDurDesc;
                                                QString errorStr;
                                                aaamoveTick(loc_tick, loc_maxtick, loc_divisions, ee,
                                                            duration, noteDurDesc, errorStr);
                                                // TODO: migrate voice overlap detector to Fraction
                                                vod.addNote(startTick.ticks(), loc_tick.ticks(), voice, staff);
                                                }
                                          }
                                    }
                              else if (ee.tagName() == "backup") {
                                    Fraction dummyFr;
                                    QString noteDurDesc;
                                    QString errorStr;
                                    aaamoveTick(loc_tick, loc_maxtick, loc_divisions, ee, dummyFr, noteDurDesc, errorStr);
                                    }
                              else if (ee.tagName() == "forward") {
                                    QString dummyStr;
                                    Fraction dummyFr;
                                    QString errorStr;
                                    aaamoveTick(loc_tick, loc_maxtick, loc_divisions, ee, dummyFr, dummyStr, errorStr);
                                    }
                              }
                        }
                  // debug vod
                  // vod.dump();
                  // copy overlap data from vod to voicelist
                  copyOverlapData(vod, parts[partNr].voicelist);

                  // set measure number and duration
                  Fraction fMeasureDuration;
                  if (measureStartTick.isValid() && loc_maxtick.isValid()) {
                        fMeasureDuration = loc_maxtick - measureStartTick;
                        fMeasureDuration.reduce();
                        }

                  // fix for PDFtoMusic Pro v1.3.0d Build BF4E (which sometimes generates empty measures)
                  // if no valid length found and length according to time signature is known,
                  // use length according to time signature
                  // TODO: use fraction instead of timeSigLen
                  if (fMeasureDuration.isZero() && timeSigLen > 0)
                        fMeasureDuration = Fraction::fromTicks(timeSigLen);

                  // if necessary, round up to an integral number of 1/64s,
                  // to comply with MuseScores actual measure length constraints
                  // TODO: calculate in fraction
                  int length = fMeasureDuration.ticks();
                  if ((length % (MScore::division/16)) != 0) {
                        int correctedLength = ((length / (MScore::division/16)) + 1) * (MScore::division/16);
                        fMeasureDuration = Fraction::fromTicks(correctedLength);
                        }

                  parts[partNr].addMeasureNumberAndDuration(measureNumber, fMeasureDuration);

                  } // e.tagName() == "measure"
            }

      // allocate MuseScore staff to MusicXML voices
      allocateStaves(parts[partNr].voicelist);
      // allocate MuseScore voice to MusicXML voices
      allocateVoices(parts[partNr].voicelist);

      // debug: print results
      /*
      qDebug("voiceMapperStats: new staff");
      for (QMap<int, VoiceDesc>::const_iterator i = parts[partNr].voicelist.constBegin();
           i != parts[partNr].voicelist.constEnd(); ++i) {
            qDebug("voiceMapperStats: voice %d staff data %s",
                   i.key() + 1, qPrintable(i.value().toString()));
            }
      */
      }


//---------------------------------------------------------
//   determineMeasureLength
//---------------------------------------------------------

/**
 Determine the length in ticks of each measure in all parts.
 Return false on error.
 */

// TODO set measurelength in fraction

bool MxmlReaderFirstPass::determineMeasureLength(QVector<int>& ml) const
      {
      ml.clear();

      // determine number of measures: max number of measures in any part
      int nMeasures = 0;
      for (int j = 0; j < parts.size(); ++j) {
            int nMeasPartJ = parts.at(j).nMeasures();
            if (nMeasPartJ > nMeasures)
                  nMeasures = nMeasPartJ;
            }

      // determine max length of a specific measure in all parts
      for (int i = 0; i < nMeasures; ++i) {
            Fraction maxMeasDur;
            for (int j = 0; j < parts.size(); ++j) {
                  if (i < parts.at(j).nMeasures()) {
                        Fraction measDurPartJ = parts.at(j).measureDuration(i);
                        if (measDurPartJ > maxMeasDur)
                              maxMeasDur = measDurPartJ;
                        }
                  }
            // qDebug("determineMeasureLength() measure %d %s (%d)", i, qPrintable(maxMeasDur.print()), maxMeasDur.ticks());
            ml.append(maxMeasDur.ticks());
            }
      return true;
      }


VoiceList MxmlReaderFirstPass::getVoiceList(const int i) const
      {
      if (i >= 0 && i < nParts())
            return parts.at(i).voicelist;
      else
            return VoiceList();
      }


VoiceList MxmlReaderFirstPass::getVoiceList(const QString id) const
      {
      for (int i = 0; i < nParts(); ++i) {
            if (parts.at(i).getId() == id)
                  return parts.at(i).voicelist;
            }
      return VoiceList();
      }


Score::FileError MxmlReaderFirstPass::setContent(QIODevice* d)
      {
      int line;
      int column;
      QString err;

      if (!doc.setContent(d, false, &err, &line, &column)) {
            QString s = QT_TRANSLATE_NOOP("file", "error at line %1 column %2: %3\n");
            MScore::lastError = s.arg(line).arg(column).arg(err);
            return Score::FILE_BAD_FORMAT;
            }

      // return OK
      return Score::FILE_NO_ERROR;
      }


// parse the part
// in: e is the "part" node
// equivalent to MuseScores xmlPart

void MxmlReaderFirstPass::parsePart(const QDomElement e, QString& /* partName */, int partNr)
      {
      initVoiceMapperAndMapVoices(e, partNr);
      }


// parse the part list
// in: e is the "part-list" node
// creates the parts and for each part sets id and name

void MxmlReaderFirstPass::parsePartList(QDomElement e)
      {
      QString partId;
      QString partName;
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "score-part") {
                  partId = e.attribute("id");
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "part-name")
                              partName = ee.text();
                        else {
                              ; // ignore
                              }
                        }
                  parts.append(MusicXmlPart(partId, partName));
                  }
            else {
                  ; // ignore
                  }
            }
      }


// parse the file

void MxmlReaderFirstPass::parseFile()
      {
      qDebug("MxmlReaderFirstPass::parseFile() begin");
      QTime t;
      t.start();
      // get the root element
      QDomElement e = doc.documentElement();

      // read the score
      int partNr = 0; // part number while reading parts
      qDebug("part list");
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "part") {
                  QString partName = e.attribute("id");
                  parsePart(e, partName, partNr);
                  ++partNr;
                  qDebug("part %d id '%s'", partNr, qPrintable(partName));
                  }
            else if (e.tagName() == "part-list") {
                  parsePartList(e);
                  }
            else {
                  // ignore
                  }
            }

      // debug: print results
      /*
      for (int i = 0; i < parts.size(); ++i) {
            qDebug("part %d\n%s", i + 1, qPrintable(parts.at(i).toString()));
            }
       */

      qDebug("Parsing time elapsed: %d ms", t.elapsed());
      qDebug("MxmlReaderFirstPass::parseFile() end");
      }
}

