//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2014 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

/**
 File handling: loading and saving.
 */

#include "config.h"
#include "globals.h"
#include "musescore.h"
#include "scoreview.h"
#include "exportmidi.h"
#include "libmscore/xml.h"
#include "libmscore/element.h"
#include "libmscore/note.h"
#include "libmscore/rest.h"
#include "libmscore/sig.h"
#include "libmscore/clef.h"
#include "libmscore/key.h"
#include "instrdialog.h"
#include "libmscore/score.h"
#include "libmscore/page.h"
#include "libmscore/dynamic.h"
#include "file.h"
#include "libmscore/style.h"
#include "libmscore/tempo.h"
#include "libmscore/select.h"
#include "preferences.h"
#include "playpanel.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/utils.h"
#include "libmscore/barline.h"
#include "palette.h"
#include "symboldialog.h"
#include "libmscore/slur.h"
#include "libmscore/hairpin.h"
#include "libmscore/ottava.h"
#include "libmscore/textline.h"
#include "libmscore/pedal.h"
#include "libmscore/trill.h"
#include "libmscore/volta.h"
#include "newwizard.h"
#include "libmscore/timesig.h"
#include "libmscore/box.h"
#include "libmscore/excerpt.h"
#include "libmscore/system.h"
#include "libmscore/tuplet.h"
#include "libmscore/keysig.h"
#include "magbox.h"
#include "libmscore/measure.h"
#include "libmscore/undo.h"
#include "libmscore/repeatlist.h"
#include "scoretab.h"
#include "libmscore/beam.h"
#include "libmscore/stafftype.h"
#include "seq.h"
#include "libmscore/revisions.h"
#include "libmscore/lyrics.h"
#include "libmscore/segment.h"
#include "libmscore/tempotext.h"
#include "libmscore/sym.h"
#include "libmscore/image.h"
#include "synthesizer/msynthesizer.h"
#include "svggenerator.h"
#include "scorePreview.h"
#include "extension.h"

#ifdef OMR
#include "omr/omr.h"
#include "omr/omrpage.h"
#include "omr/importpdf.h"
#endif

#include "diff/diff_match_patch.h"
#include "libmscore/chordlist.h"
#include "libmscore/mscore.h"
#include "thirdparty/qzip/qzipreader_p.h"

extern Ms::Score::FileError importOve(Ms::Score*, const QString& name);

namespace Ms {

extern Score::FileError importMidi(Score*, const QString& name);
extern Score::FileError importGTP(Score*, const QString& name);
extern Score::FileError importBww(Score*, const QString& path);
extern Score::FileError importMusicXml(Score*, const QString&);
extern Score::FileError importCompressedMusicXml(Score*, const QString&);
extern Score::FileError importMuseData(Score*, const QString& name);
extern Score::FileError importLilypond(Score*, const QString& name);
extern Score::FileError importBB(Score*, const QString& name);
extern Score::FileError importCapella(Score*, const QString& name);
extern Score::FileError importCapXml(Score*, const QString& name);

extern Score::FileError readScore(Score* score, QString name, bool ignoreVersionError);

extern void importSoundfont(QString name);

extern bool savePositions(Score*, const QString& name, bool segments);
extern MasterSynthesizer* synti;

//---------------------------------------------------------
//   paintElement(s)
//---------------------------------------------------------

static void paintElement(QPainter& p, const Element* e)
{
    QPointF pos(e->pagePos());
    p.translate(pos);
    e->draw(&p);
    p.translate(-pos);
}

static void paintElements(QPainter& p, const QList<const Element*>& el)
{
    foreach (const Element* e, el) {
        if (!e->visible())
            continue;
        paintElement(p, e);
    }
}

//---------------------------------------------------------
//   createDefaultFileName
//---------------------------------------------------------

static QString createDefaultFileName(QString fn)
      {
      //
      // special characters in filenames are a constant source
      // of trouble, this replaces some of them common in german:
      //
      fn = fn.simplified();
      fn = fn.replace(QChar(' '),  "_");
      fn = fn.replace(QChar('\n'), "_");
      fn = fn.replace(QChar(0xe4), "ae"); // &auml;
      fn = fn.replace(QChar(0xf6), "oe"); // &ouml;
      fn = fn.replace(QChar(0xfc), "ue"); // &uuml;
      fn = fn.replace(QChar(0xdf), "ss"); // &szlig;
      fn = fn.replace(QChar(0xc4), "Ae"); // &Auml;
      fn = fn.replace(QChar(0xd6), "Oe"); // &Ouml;
      fn = fn.replace(QChar(0xdc), "Ue"); // &Uuml;
      fn = fn.replace(QChar(0x266d),"b"); // musical flat sign, happen in instrument names, so can happen in part (file) names
      fn = fn.replace(QChar(0x266f),"#"); // musical sharp sign, can happen in titles, so can happen in score (file) names
      fn = fn.replace( QRegExp( "[" + QRegExp::escape( "\\/:*?\"<>|" ) + "]" ), "_" ); //FAT/NTFS special chars
      return fn;
      }

//---------------------------------------------------------
//   readScoreError
//    if "ask" is true, ask to ignore; returns true if
//    ignore is pressed by user
//    returns true if -f is used in converter mode
//---------------------------------------------------------

static bool readScoreError(const QString& name, Score::FileError error, bool ask)
      {
      QString msg = QObject::tr("Cannot read file %1:\n").arg(name);
      QString detailedMsg;
      bool canIgnore = false;
      switch(error) {
            case Score::FileError::FILE_NO_ERROR:
                  return false;
            case Score::FileError::FILE_BAD_FORMAT:
                  msg +=  QObject::tr("bad format");
                  detailedMsg = MScore::lastError;
                  break;
            case Score::FileError::FILE_UNKNOWN_TYPE:
                  msg += QObject::tr("unknown type");
                  break;
            case Score::FileError::FILE_NO_ROOTFILE:
                  break;
            case Score::FileError::FILE_TOO_OLD:
                  msg += QObject::tr("It was last saved with version 0.9.5 or older.\n"
                                     "You can convert this score by opening and then\n"
                                     "saving with MuseScore version 1.x.\n"
                                     "Visit the %1MuseScore download page%2 to obtain such a 1.x version.")
                              .arg("<a href=\"https://musescore.org/download#older-versions\">")
                              .arg("</a>");
                  canIgnore = true;
                  break;
            case Score::FileError::FILE_TOO_NEW:
                  msg += QObject::tr("This score was saved using a newer version of MuseScore.\n"
                                     "Visit the %1MuseScore website%2 to obtain the latest version.")
                              .arg("<a href=\"https://musescore.org\">")
                              .arg("</a>");
                  canIgnore = true;
                  break;
            case Score::FileError::FILE_NOT_FOUND:
                  msg = QObject::tr("File \"%1\" not found.").arg(name);
                  break;
            case Score::FileError::FILE_CORRUPTED:
                  msg = QObject::tr("File \"%1\" corrupted.").arg(name);
                  detailedMsg = MScore::lastError;
                  canIgnore = true;
                  break;
            case Score::FileError::FILE_ERROR:
            case Score::FileError::FILE_OPEN_ERROR:
            default:
                  msg += MScore::lastError;
                  break;
            }
      if (converterMode && canIgnore && ignoreWarnings) {
            fprintf(stderr, "%s\n\nWarning ignored, forcing score to load\n", qPrintable(msg));
            return true;
            }
       if (converterMode || pluginMode) {
            fprintf(stderr, "%s\n", qPrintable(msg));
            return false;
            }
      QMessageBox msgBox;
      msgBox.setWindowTitle(QObject::tr("Load Error"));
      msgBox.setText(msg.replace("\n", "<br/>"));
      msgBox.setDetailedText(detailedMsg);
      msgBox.setTextFormat(Qt::RichText);
      if (canIgnore && ask)  {
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setStandardButtons(
               QMessageBox::Cancel | QMessageBox::Ignore
               );
            return msgBox.exec() == QMessageBox::Ignore;
            }
      else {
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setStandardButtons(
               QMessageBox::Ok
               );
            msgBox.exec();
            }
      return false;
      }

//---------------------------------------------------------
//   checkDirty
//    if dirty, save score
//    return true on cancel
//---------------------------------------------------------

bool MuseScore::checkDirty(Score* s)
      {
      if (s->dirty() || s->created()) {
            QMessageBox::StandardButton n = QMessageBox::warning(this, tr("MuseScore"),
               tr("Save changes to the score \"%1\"\n"
               "before closing?").arg(s->name()),
               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
               QMessageBox::Save);
            if (n == QMessageBox::Save) {
                  if (s->isSavable()) {
                        if (!saveFile(s))
                              return true;
                        }
                  else {
                        if (!saveAs(s, false))
                              return true;
                        }

                  }
            else if (n == QMessageBox::Cancel)
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   loadFile
//---------------------------------------------------------

/**
 Create a modal file open dialog.
 If a file is selected, load it.
 Handles the GUI's file-open action.
 */

void MuseScore::loadFiles()
      {
      QStringList files = getOpenScoreNames(
#ifdef OMR
         tr("All Supported Files") + " (*.mscz *.mscx *.mxl *.musicxml *.xml *.mid *.midi *.kar *.md *.mgu *.sgu *.cap *.capx *.pdf *.ove *.scw *.bww *.gtp *.gp3 *.gp4 *.gp5 *.gpx);;" +
#else
         tr("All Supported Files") + " (*.mscz *.mscx *.mxl *.musicxml *.xml *.mid *.midi *.kar *.md *.mgu *.sgu *.cap *.capx *.ove *.scw *.bww *.gtp *.gp3 *.gp4 *.gp5 *.gpx);;" +
#endif
         tr("MuseScore Files") + " (*.mscz *.mscx);;" +
         tr("MusicXML Files") + " (*.mxl *.musicxml *.xml);;" +
         tr("MIDI Files") + " (*.mid *.midi *.kar);;" +
         tr("MuseData Files") + " (*.md);;" +
         tr("Capella Files") + " (*.cap *.capx);;" +
         tr("BB Files <experimental>") + " (*.mgu *.sgu);;" +
#ifdef OMR
         tr("PDF Files <experimental OMR>") + " (*.pdf);;" +
#endif
         tr("Overture / Score Writer Files <experimental>") + " (*.ove *.scw);;" +
         tr("Bagpipe Music Writer Files <experimental>") + " (*.bww);;" +
         tr("Guitar Pro") + " (*.gtp *.gp3 *.gp4 *.gp5 *.gpx)",
         tr("Load Score")
         );
      for (const QString& s : files)
            openScore(s);
      }

//---------------------------------------------------------
//   openScore
//---------------------------------------------------------

Score* MuseScore::openScore(const QString& fn)
      {
      //
      // make sure we load a file only once
      //
      QFileInfo fi(fn);
      QString path = fi.canonicalFilePath();
      for (Score* s : scoreList) {
            if (s->fileInfo()->canonicalFilePath() == path)
                  return 0;
            }

      Score* score = readScore(fn);
      if (score) {
            setCurrentScoreView(appendScore(score));
            writeSessionFile(false);
            }
      return score;
      }

//---------------------------------------------------------
//   readScore
//---------------------------------------------------------

Score* MuseScore::readScore(const QString& name)
      {
      if (name.isEmpty())
            return 0;

      Score* score = new Score(MScore::baseStyle());  // start with built-in style
      setMidiReopenInProgress(name);
      Score::FileError rv = Ms::readScore(score, name, false);
      if (rv == Score::FileError::FILE_TOO_OLD || rv == Score::FileError::FILE_TOO_NEW || rv == Score::FileError::FILE_CORRUPTED) {
            if (readScoreError(name, rv, true)) {
                  delete score;
                  score = new Score(MScore::baseStyle());
                  rv = Ms::readScore(score, name, true);
                  }
            else {
                  delete score;
                  return 0;
                  }
            }
      if (rv != Score::FileError::FILE_NO_ERROR) {
            // in case of user abort while reading, the error has already been reported
            // else report it now
            if (rv != Score::FileError::FILE_USER_ABORT && rv != Score::FileError::FILE_IGNORE_ERROR)
                  readScoreError(name, rv, false);
            delete score;
            score = 0;
            return 0;
            }
      allowShowMidiPanel(name);
      if (score)
            addRecentScore(score);
      return score;
      }

//---------------------------------------------------------
//   saveFile
///   Save the current score.
///   Handles the GUI's file-save action.
//
//    return true on success
//---------------------------------------------------------

bool MuseScore::saveFile()
      {
      return saveFile(cs->rootScore());
      }

//---------------------------------------------------------
//   saveFile
///   Save the score.
//
//    return true on success
//---------------------------------------------------------

bool MuseScore::saveFile(Score* score)
      {
      if (score == 0)
            return false;
      if (score->created()) {
            QString fn = score->fileInfo()->fileName();
            Text* t = score->getText(TextStyleType::TITLE);
            if (t)
                  fn = t->plainText(true);
            QString name = createDefaultFileName(fn);
            QString f1 = tr("MuseScore File") + " (*.mscz)";
            QString f2 = tr("Uncompressed MuseScore File") + " (*.mscx)";

            QSettings settings;
            if (mscore->lastSaveDirectory.isEmpty())
                  mscore->lastSaveDirectory = settings.value("lastSaveDirectory", preferences.myScoresPath).toString();
            QString saveDirectory = mscore->lastSaveDirectory;

            if (saveDirectory.isEmpty())
                  saveDirectory = preferences.myScoresPath;

            QString fname = QString("%1/%2").arg(saveDirectory).arg(name);
            QString filter = f1 + ";;" + f2;
            if (QFileInfo(fname).suffix().isEmpty())
                  fname += ".mscz";

            fn = mscore->getSaveScoreName(tr("Save Score"), fname, filter);
            if (fn.isEmpty())
                  return false;
            score->fileInfo()->setFile(fn);

            mscore->lastSaveDirectory = score->fileInfo()->absolutePath();

            if (!score->saveFile()) {
                  QMessageBox::critical(mscore, tr("Save File"), MScore::lastError);
                  return false;
                  }
            addRecentScore(score);
            writeSessionFile(false);
            }
      else if (!score->saveFile()) {
            QMessageBox::critical(mscore, tr("Save File"), MScore::lastError);
            return false;
            }
      score->setCreated(false);
      updateWindowTitle(score);
      int idx = scoreList.indexOf(score);
      tab1->setTabText(idx, score->name());
      if (tab2)
            tab2->setTabText(idx, score->name());
      QString tmp = score->tmpName();
      if (!tmp.isEmpty()) {
            QFile f(tmp);
            if (!f.remove())
                  qDebug("cannot remove temporary file <%s>", qPrintable(f.fileName()));
            score->setTmpName("");
            }
      writeSessionFile(false);
      return true;
      }

//---------------------------------------------------------
//   createDefaultName
//---------------------------------------------------------

QString MuseScore::createDefaultName() const
      {
      QString name(tr("Untitled"));
      int n;
      for (n = 1; ; ++n) {
            bool nameExists = false;
            QString tmpName;
            if (n == 1)
                  tmpName = name;
            else
                  tmpName = QString("%1-%2").arg(name).arg(n);
            foreach(Score* s, scoreList) {
                  if (s->name() == tmpName) {
                        nameExists = true;
                        break;
                        }
                  }
            if (!nameExists) {
                  name = tmpName;
                  break;
                  }
            }
      return name;
      }


void MuseScore::updateNewWizard()
      {
      if (newWizard == nullptr)
            newWizard = new NewWizard(this);
      newWizard->updateValues();
      }

//---------------------------------------------------------
//   newFile
//    create new score
//---------------------------------------------------------

void MuseScore::newFile()
      {
      updateNewWizard();
      newWizard->restart();
      if (newWizard->exec() != QDialog::Accepted)
            return;
      int measures            = newWizard->measures();
      Fraction timesig        = newWizard->timesig();
      TimeSigType timesigType = newWizard->timesigType();
      KeySigEvent ks          = newWizard->keysig();
      VBox* nvb               = nullptr;

      int pickupTimesigZ;
      int pickupTimesigN;
      bool pickupMeasure = newWizard->pickupMeasure(&pickupTimesigZ, &pickupTimesigN);
      if (pickupMeasure)
            measures += 1;

      Score* score;
      QString tp = newWizard->templatePath();

      QList<Excerpt*> excerpts;
      if (!newWizard->emptyScore()) {
            Score* tscore = new Score(MScore::defaultStyle());
            Score::FileError rv = Ms::readScore(tscore, tp, false);
            if (rv != Score::FileError::FILE_NO_ERROR) {
                  readScoreError(newWizard->templatePath(), rv, false);
                  delete tscore;
                  return;
                  }
            score = new Score(tscore->style());
            // create instruments from template
            for (Part* tpart : tscore->parts()) {
                  Part* part = new Part(score);
                  part->setInstrument(tpart->instrument());
                  part->setPartName(tpart->partName());

                  for (Staff* tstaff : *tpart->staves()) {
                        Staff* staff = new Staff(score);
                        staff->setPart(part);
                        staff->init(tstaff);
                        if (tstaff->linkedStaves() && !part->staves()->isEmpty()) {
                              Staff* linkedStaff = part->staves()->back();
                              staff->linkTo(linkedStaff);
                              }
                        part->insertStaff(staff, -1);
                        score->staves().append(staff);
                        }
                  score->appendPart(part);
                  }
            for (Excerpt* ex : tscore->excerpts()) {
                  Excerpt* x = new Excerpt(score);
                  x->setTitle(ex->title());
                  for (Part* p : ex->parts()) {
                        int pidx = tscore->parts().indexOf(p);
                        if (pidx == -1)
                              qDebug("newFile: part not found");
                        else
                              x->parts().append(score->parts()[pidx]);
                        }
                  excerpts.append(x);
                  }
            MeasureBase* mb = tscore->first();
            if (mb && mb->type() == Element::Type::VBOX) {
                  VBox* tvb = static_cast<VBox*>(mb);
                  nvb = new VBox(score);
                  nvb->setBoxHeight(tvb->boxHeight());
                  nvb->setBoxWidth(tvb->boxWidth());
                  nvb->setTopGap(tvb->topGap());
                  nvb->setBottomGap(tvb->bottomGap());
                  nvb->setTopMargin(tvb->topMargin());
                  nvb->setBottomMargin(tvb->bottomMargin());
                  nvb->setLeftMargin(tvb->leftMargin());
                  nvb->setRightMargin(tvb->rightMargin());
                  }
            delete tscore;
            }
      else {
            score = new Score(MScore::defaultStyle());
            newWizard->createInstruments(score);
            }
      score->setCreated(true);
      score->fileInfo()->setFile(createDefaultName());

      if (!score->style()->chordList()->loaded()) {
            if (score->style()->value(StyleIdx::chordsXmlFile).toBool())
                  score->style()->chordList()->read("chords.xml");
            score->style()->chordList()->read(score->style()->value(StyleIdx::chordDescriptionFile).toString());
            }
      if (!newWizard->title().isEmpty())
            score->fileInfo()->setFile(newWizard->title());

      score->sigmap()->add(0, timesig);

      int firstMeasureTicks = pickupMeasure ? Fraction(pickupTimesigZ, pickupTimesigN).ticks() : timesig.ticks();

      for (int i = 0; i < measures; ++i) {
            int tick = firstMeasureTicks + timesig.ticks() * (i - 1);
            if (i == 0)
                  tick = 0;
            QList<Rest*> puRests;
            for (Score* _score : score->scoreList()) {
                  Rest* rest = 0;
                  Measure* measure = new Measure(_score);
                  measure->setTimesig(timesig);
                  measure->setLen(timesig);
                  measure->setTick(tick);

                  if (pickupMeasure && tick == 0) {
                        measure->setIrregular(true);        // dont count pickup measure
                        measure->setLen(Fraction(pickupTimesigZ, pickupTimesigN));
                        }
                  _score->measures()->add(measure);

                  for (Staff* staff : _score->staves()) {
                        int staffIdx = staff->idx();
                        if (tick == 0) {
                              TimeSig* ts = new TimeSig(_score);
                              ts->setTrack(staffIdx * VOICES);
                              ts->setSig(timesig, timesigType);
                              Measure* m = _score->firstMeasure();
                              Segment* s = m->getSegment(ts, 0);
                              s->add(ts);
                              Part* part = staff->part();
                              if (!part->instrument()->useDrumset()) {
                                    //
                                    // transpose key
                                    //
                                    KeySigEvent nKey = ks;
                                    if (!nKey.custom() && !nKey.isAtonal() && part->instrument()->transpose().chromatic && !score->styleB(StyleIdx::concertPitch)) {
                                          int diff = -part->instrument()->transpose().chromatic;
                                          nKey.setKey(transposeKey(nKey.key(), diff));
                                          }
                                    // do not create empty keysig unless custom or atonal
                                    if (nKey.custom() || nKey.isAtonal() || nKey.key() != Key::C) {
                                          staff->setKey(0, nKey);
                                          KeySig* keysig = new KeySig(score);
                                          keysig->setTrack(staffIdx * VOICES);
                                          keysig->setKeySigEvent(nKey);
                                          Segment* s = measure->getSegment(keysig, 0);
                                          s->add(keysig);
                                          }
                                    }
                              }

                        // determined if this staff is linked to previous so we can reuse rests
                        bool linkedToPrevious = staffIdx && staff->isLinked(_score->staff(staffIdx - 1));
                        if (measure->timesig() != measure->len()) {
                              if (!linkedToPrevious)
                                    puRests.clear();
                              QList<TDuration> dList = toDurationList(measure->len(), false);
                              if (!dList.isEmpty()) {
                                    int ltick = tick;
                                    int k = 0;
                                    foreach (TDuration d, dList) {
                                          if (k < puRests.count())
                                                rest = static_cast<Rest*>(puRests[k]->linkedClone());
                                          else {
                                                rest = new Rest(score, d);
                                                puRests.append(rest);
                                                }
                                          rest->setScore(_score);
                                          rest->setDuration(d.fraction());
                                          rest->setTrack(staffIdx * VOICES);
                                          Segment* seg = measure->getSegment(rest, ltick);
                                          seg->add(rest);
                                          ltick += rest->actualTicks();
                                          k++;
                                          }
                                    }
                              }
                        else {
                              if (linkedToPrevious && rest)
                                    rest = static_cast<Rest*>(rest->linkedClone());
                              else
                                    rest = new Rest(score, TDuration(TDuration::DurationType::V_MEASURE));
                              rest->setScore(_score);
                              rest->setDuration(measure->len());
                              rest->setTrack(staffIdx * VOICES);
                              Segment* seg = measure->getSegment(rest, tick);
                              seg->add(rest);
                              }
                        }
                  }
            }
      score->lastMeasure()->setEndBarLineType(BarLineType::END, false);

      //
      // select first rest
      //
      Measure* m = score->firstMeasure();
      for (Segment* s = m->first(); s; s = s->next()) {
            if (s->segmentType() == Segment::Type::ChordRest) {
                  if (s->element(0)) {
                        score->select(s->element(0), SelectType::SINGLE, 0);
                        break;
                        }
                  }
            }

      QString title     = newWizard->title();
      QString subtitle  = newWizard->subtitle();
      QString composer  = newWizard->composer();
      QString poet      = newWizard->poet();
      QString copyright = newWizard->copyright();

      if (!title.isEmpty() || !subtitle.isEmpty() || !composer.isEmpty() || !poet.isEmpty()) {
            MeasureBase* measure = score->measures()->first();
            if (measure->type() != Element::Type::VBOX) {
                  MeasureBase* nm = nvb ? nvb : new VBox(score);
                  nm->setTick(0);
                  nm->setNext(measure);
                  score->measures()->add(nm);
                  measure = nm;
                  }
            else if (nvb) {
                  delete nvb;
                  }
            if (!title.isEmpty()) {
                  Text* s = new Text(score);
                  s->setTextStyleType(TextStyleType::TITLE);
                  s->setPlainText(title);
                  measure->add(s);
                  score->setMetaTag("workTitle", title);
                  }
            if (!subtitle.isEmpty()) {
                  Text* s = new Text(score);
                  s->setTextStyleType(TextStyleType::SUBTITLE);
                  s->setPlainText(subtitle);
                  measure->add(s);
                  }
            if (!composer.isEmpty()) {
                  Text* s = new Text(score);
                  s->setTextStyleType(TextStyleType::COMPOSER);
                  s->setPlainText(composer);
                  measure->add(s);
                  score->setMetaTag("composer", composer);
                  }
            if (!poet.isEmpty()) {
                  Text* s = new Text(score);
                  s->setTextStyleType(TextStyleType::POET);
                  s->setPlainText(poet);
                  measure->add(s);
                  // the poet() functions returns data called lyricist in the dialog
                  score->setMetaTag("lyricist", poet);
                  }
            }
      else if (nvb) {
            delete nvb;
            }

      if (newWizard->createTempo()) {
            double tempo = newWizard->tempo();
            TempoText* tt = new TempoText(score);
            tt->setXmlText(QString("<sym>metNoteQuarterUp</sym> = %1").arg(tempo));
            tempo /= 60;      // bpm -> bps

            tt->setTempo(tempo);
            tt->setFollowText(true);
            tt->setTrack(0);
            Segment* seg = score->firstMeasure()->first(Segment::Type::ChordRest);
            seg->add(tt);
            score->setTempo(0, tempo);
            }
      if (!copyright.isEmpty())
            score->setMetaTag("copyright", copyright);

      score->rebuildMidiMapping();
      score->doLayout();
      setCurrentScoreView(appendScore(score));

      for (Excerpt* x : excerpts) {
            Score* xs = new Score(score);
            xs->setName(x->title());
            xs->style()->set(StyleIdx::createMultiMeasureRests, true);
            x->setPartScore(xs);
            score->excerpts().append(x);
            createExcerpt(x);
            score->setExcerptsChanged(true);
            }
      }

//---------------------------------------------------------
//   addScorePreview
//    add a score preview to the file dialog
//---------------------------------------------------------

static void addScorePreview(QFileDialog* dialog)
      {
      QSplitter* splitter = dialog->findChild<QSplitter*>("splitter");
      if (splitter) {
            ScorePreview* preview = new ScorePreview;
            splitter->addWidget(preview);
            dialog->connect(dialog, SIGNAL(currentChanged(const QString&)), preview, SLOT(setScore(const QString&)));
            }
      }

//---------------------------------------------------------
//   sidebarUrls
//    return a list of standard file dialog sidebar urls
//---------------------------------------------------------

static QList<QUrl> sidebarUrls()
      {
      QList<QUrl> urls;
      urls.append(QUrl::fromLocalFile(QDir::homePath()));
      QFileInfo myScores(preferences.myScoresPath);
      urls.append(QUrl::fromLocalFile(myScores.absoluteFilePath()));
      urls.append(QUrl::fromLocalFile(QDir::currentPath()));
      return urls;
      }

//---------------------------------------------------------
//   getOpenScoreNames
//---------------------------------------------------------

QStringList MuseScore::getOpenScoreNames(const QString& filter, const QString& title)
      {
      QSettings settings;
      QString dir = settings.value("lastOpenPath", preferences.myScoresPath).toString();
      if (preferences.nativeDialogs) {
            QStringList fileList = QFileDialog::getOpenFileNames(this,
               title, dir, filter);
            if (fileList.count() > 0) {
                  QFileInfo fi(fileList[0]);
                  settings.setValue("lastOpenPath", fi.absolutePath());
                  }
            return fileList;
            }
      QFileInfo myScores(preferences.myScoresPath);
      if (myScores.isRelative())
            myScores.setFile(QDir::home(), preferences.myScoresPath);

      if (loadScoreDialog == 0) {
            loadScoreDialog = new QFileDialog(this);
            loadScoreDialog->setFileMode(QFileDialog::ExistingFiles);
            loadScoreDialog->setOption(QFileDialog::DontUseNativeDialog, true);
            loadScoreDialog->setWindowTitle(title);
            addScorePreview(loadScoreDialog);

            // setup side bar urls
            QList<QUrl> urls = sidebarUrls();
            urls.append(QUrl::fromLocalFile(mscoreGlobalShare+"/demos"));
            loadScoreDialog->setSidebarUrls(urls);

            loadScoreDialog->setNameFilter(filter);
            restoreDialogState("loadScoreDialog", loadScoreDialog);
            loadScoreDialog->setAcceptMode(QFileDialog::AcceptOpen);
            loadScoreDialog->setDirectory(dir);
            }
      else {
            // dialog already exists, but set title and filter
            loadScoreDialog->setWindowTitle(title);
            loadScoreDialog->setNameFilter(filter);
            }

      QStringList result;
      if (loadScoreDialog->exec())
            result = loadScoreDialog->selectedFiles();
      settings.setValue("lastOpenPath", loadScoreDialog->directory().absolutePath());
      return result;
      }

//---------------------------------------------------------
//   getSaveScoreName
//---------------------------------------------------------

QString MuseScore::getSaveScoreName(const QString& title,
   QString& name, const QString& filter, bool selectFolder)
      {
      QFileInfo myName(name);
      if (myName.isRelative())
            myName.setFile(QDir::home(), name);
      name = myName.absoluteFilePath();

      if (preferences.nativeDialogs) {
            QString s;
            QFileDialog::Options options = selectFolder ? QFileDialog::ShowDirsOnly : QFileDialog::Options(0);
            return QFileDialog::getSaveFileName(this, title, name, filter, &s, options);
            }

      QFileInfo myScores(preferences.myScoresPath);
      if (myScores.isRelative())
            myScores.setFile(QDir::home(), preferences.myScoresPath);
      if (saveScoreDialog == 0) {
            saveScoreDialog = new QFileDialog(this);
            saveScoreDialog->setFileMode(QFileDialog::AnyFile);
            saveScoreDialog->setOption(QFileDialog::DontConfirmOverwrite, false);
            saveScoreDialog->setOption(QFileDialog::DontUseNativeDialog, true);
            saveScoreDialog->setAcceptMode(QFileDialog::AcceptSave);
            addScorePreview(saveScoreDialog);

            // setup side bar urls
            saveScoreDialog->setSidebarUrls(sidebarUrls());

            restoreDialogState("saveScoreDialog", saveScoreDialog);
            }
      if (selectFolder)
            saveScoreDialog->setFileMode(QFileDialog::Directory);

      saveScoreDialog->setWindowTitle(title);
      saveScoreDialog->setNameFilter(filter);
      saveScoreDialog->selectFile(name);

      if (!selectFolder) {
            connect(saveScoreDialog, SIGNAL(filterSelected(const QString&)),
               SLOT(saveScoreDialogFilterSelected(const QString&)));
            }
      QString s;
      if (saveScoreDialog->exec())
            s = saveScoreDialog->selectedFiles().front();
      return s;
      }

//---------------------------------------------------------
//   saveScoreDialogFilterSelected
//    update selected file name extensions, when filter
//    has changed
//---------------------------------------------------------

void MuseScore::saveScoreDialogFilterSelected(const QString& s)
      {
      QRegExp rx(QString(".+\\(\\*\\.(.+)\\)"));
      if (rx.exactMatch(s)) {
            QFileInfo fi(saveScoreDialog->selectedFiles().front());
            saveScoreDialog->selectFile(fi.completeBaseName() + "." + rx.cap(1));
            }
      }

//---------------------------------------------------------
//   getStyleFilename
//---------------------------------------------------------

QString MuseScore::getStyleFilename(bool open, const QString& title)
      {
      QFileInfo myStyles(preferences.myStylesPath);
      if (myStyles.isRelative())
            myStyles.setFile(QDir::home(), preferences.myStylesPath);
      QString defaultPath = myStyles.absoluteFilePath();

      if (preferences.nativeDialogs) {
            QString fn;
            if (open) {
                  fn = QFileDialog::getOpenFileName(
                     this, tr("Load Style"),
                     defaultPath,
                     tr("MuseScore Styles") + " (*.mss)"
                     );
                  }
            else {
                  fn = QFileDialog::getSaveFileName(
                     this, tr("Save Style"),
                     defaultPath,
                     tr("MuseScore Style File") + " (*.mss)"
                     );
                  }
            return fn;
            }

      QFileDialog* dialog;
      QList<QUrl> urls;
      QString home = QDir::homePath();
      urls.append(QUrl::fromLocalFile(home));
      urls.append(QUrl::fromLocalFile(defaultPath));
      urls.append(QUrl::fromLocalFile(QDir::currentPath()));

      if (open) {
            if (loadStyleDialog == 0) {
                  loadStyleDialog = new QFileDialog(this);
                  loadStyleDialog->setFileMode(QFileDialog::ExistingFile);
                  loadStyleDialog->setOption(QFileDialog::DontUseNativeDialog, true);
                  loadStyleDialog->setWindowTitle(title.isEmpty() ? tr("Load Style") : title);
                  loadStyleDialog->setNameFilter(tr("MuseScore Style File") + " (*.mss)");
                  loadStyleDialog->setDirectory(defaultPath);

                  restoreDialogState("loadStyleDialog", loadStyleDialog);
                  loadStyleDialog->setAcceptMode(QFileDialog::AcceptOpen);
                  }
            urls.append(QUrl::fromLocalFile(mscoreGlobalShare+"/styles"));
            dialog = loadStyleDialog;
            }
      else {
            if (saveStyleDialog == 0) {
                  saveStyleDialog = new QFileDialog(this);
                  saveStyleDialog->setAcceptMode(QFileDialog::AcceptSave);
                  saveStyleDialog->setFileMode(QFileDialog::AnyFile);
                  saveStyleDialog->setOption(QFileDialog::DontConfirmOverwrite, false);
                  saveStyleDialog->setOption(QFileDialog::DontUseNativeDialog, true);
                  saveStyleDialog->setWindowTitle(title.isEmpty() ? tr("Save Style") : title);
                  saveStyleDialog->setNameFilter(tr("MuseScore Style File") + " (*.mss)");
                  saveStyleDialog->setDirectory(defaultPath);

                  restoreDialogState("saveStyleDialog", saveStyleDialog);
                  saveStyleDialog->setAcceptMode(QFileDialog::AcceptSave);
                  }
            dialog = saveStyleDialog;
            }
      // setup side bar urls
      dialog->setSidebarUrls(urls);

      if (dialog->exec()) {
            QStringList result = dialog->selectedFiles();
            return result.front();
            }
      return QString();
      }

//---------------------------------------------------------
//   getChordStyleFilename
//---------------------------------------------------------

QString MuseScore::getChordStyleFilename(bool open)
      {
      QString filter = tr("Chord Symbols Style File") + " (*.xml)";

      QFileInfo myStyles(preferences.myStylesPath);
      if (myStyles.isRelative())
            myStyles.setFile(QDir::home(), preferences.myStylesPath);
      QString defaultPath = myStyles.absoluteFilePath();

      if (preferences.nativeDialogs) {
            QString fn;
            if (open) {
                  fn = QFileDialog::getOpenFileName(
                     this, tr("Load Chord Symbols Style"),
                     defaultPath,
                     filter
                     );
                  }
            else {
                  fn = QFileDialog::getSaveFileName(
                     this, tr("Save Chord Symbols Style"),
                     defaultPath,
                     filter
                     );
                  }
            return fn;
            }

      QFileDialog* dialog;
      QList<QUrl> urls;
      QString home = QDir::homePath();
      urls.append(QUrl::fromLocalFile(home));
      urls.append(QUrl::fromLocalFile(defaultPath));
      urls.append(QUrl::fromLocalFile(QDir::currentPath()));

      QSettings settings;
      if (open) {
            if (loadChordStyleDialog == 0) {
                  loadChordStyleDialog = new QFileDialog(this);
                  loadChordStyleDialog->setFileMode(QFileDialog::ExistingFile);
                  loadChordStyleDialog->setOption(QFileDialog::DontUseNativeDialog, true);
                  loadChordStyleDialog->setWindowTitle(tr("Load Chord Symbols Style"));
                  loadChordStyleDialog->setNameFilter(filter);
                  loadChordStyleDialog->setDirectory(defaultPath);

                  restoreDialogState("loadChordStyleDialog", loadChordStyleDialog);
                  loadChordStyleDialog->restoreState(settings.value("loadChordStyleDialog").toByteArray());
                  loadChordStyleDialog->setAcceptMode(QFileDialog::AcceptOpen);
                  }
            // setup side bar urls
            urls.append(QUrl::fromLocalFile(mscoreGlobalShare+"/styles"));
            dialog = loadChordStyleDialog;
            }
      else {
            if (saveChordStyleDialog == 0) {
                  saveChordStyleDialog = new QFileDialog(this);
                  saveChordStyleDialog->setAcceptMode(QFileDialog::AcceptSave);
                  saveChordStyleDialog->setFileMode(QFileDialog::AnyFile);
                  saveChordStyleDialog->setOption(QFileDialog::DontConfirmOverwrite, false);
                  saveChordStyleDialog->setOption(QFileDialog::DontUseNativeDialog, true);
                  saveChordStyleDialog->setWindowTitle(tr("Save Style"));
                  saveChordStyleDialog->setNameFilter(filter);
                  saveChordStyleDialog->setDirectory(defaultPath);

                  restoreDialogState("saveChordStyleDialog", saveChordStyleDialog);
                  saveChordStyleDialog->setAcceptMode(QFileDialog::AcceptSave);
                  }
            dialog = saveChordStyleDialog;
            }
      // setup side bar urls
      dialog->setSidebarUrls(urls);
      if (dialog->exec()) {
            QStringList result = dialog->selectedFiles();
            return result.front();
            }
      return QString();
      }

//---------------------------------------------------------
//   getScanFile
//---------------------------------------------------------

QString MuseScore::getScanFile(const QString& d)
      {
      QString filter = tr("PDF Scan File") + " (*.pdf);;All (*)";
      QString defaultPath = d.isEmpty() ? QDir::homePath() : d;
      if (preferences.nativeDialogs) {
            QString s = QFileDialog::getOpenFileName(
               mscore,
               MuseScore::tr("Choose PDF Scan"),
               defaultPath,
               filter
               );
            return s;
            }

      if (loadScanDialog == 0) {
            loadScanDialog = new QFileDialog(this);
            loadScanDialog->setFileMode(QFileDialog::ExistingFile);
            loadScanDialog->setOption(QFileDialog::DontUseNativeDialog, true);
            loadScanDialog->setWindowTitle(tr("Choose PDF Scan"));
            loadScanDialog->setNameFilter(filter);
            loadScanDialog->setDirectory(defaultPath);

            restoreDialogState("loadScanDialog", loadScanDialog);
            loadScanDialog->setAcceptMode(QFileDialog::AcceptOpen);
            }

      //
      // setup side bar urls
      //
      QList<QUrl> urls;
      QString home = QDir::homePath();
      urls.append(QUrl::fromLocalFile(home));
      urls.append(QUrl::fromLocalFile(QDir::currentPath()));
      loadScanDialog->setSidebarUrls(urls);

      if (loadScanDialog->exec()) {
            QStringList result = loadScanDialog->selectedFiles();
            return result.front();
            }
      return QString();
      }

//---------------------------------------------------------
//   getAudioFile
//---------------------------------------------------------

QString MuseScore::getAudioFile(const QString& d)
      {
      QString filter = tr("Ogg Audio File") + " (*.ogg);;All (*)";
      QString defaultPath = d.isEmpty() ? QDir::homePath() : d;
      if (preferences.nativeDialogs) {
            QString s = QFileDialog::getOpenFileName(
               mscore,
               MuseScore::tr("Choose Audio File"),
               defaultPath,
               filter
               );
            return s;
            }

      if (loadAudioDialog == 0) {
            loadAudioDialog = new QFileDialog(this);
            loadAudioDialog->setFileMode(QFileDialog::ExistingFile);
            loadAudioDialog->setOption(QFileDialog::DontUseNativeDialog, true);
            loadAudioDialog->setWindowTitle(tr("Choose Ogg Audio File"));
            loadAudioDialog->setNameFilter(filter);
            loadAudioDialog->setDirectory(defaultPath);

            restoreDialogState("loadAudioDialog", loadAudioDialog);
            loadAudioDialog->setAcceptMode(QFileDialog::AcceptOpen);
            }

      //
      // setup side bar urls
      //
      QList<QUrl> urls;
      QString home = QDir::homePath();
      urls.append(QUrl::fromLocalFile(home));
      urls.append(QUrl::fromLocalFile(QDir::currentPath()));
      loadAudioDialog->setSidebarUrls(urls);

      if (loadAudioDialog->exec()) {
            QStringList result = loadAudioDialog->selectedFiles();
            return result.front();
            }
      return QString();
      }

//---------------------------------------------------------
//   getFotoFilename
//---------------------------------------------------------

QString MuseScore::getFotoFilename(QString& filter, QString* selectedFilter)
      {
      QString title = tr("Save Image");

      QFileInfo myImages(preferences.myImagesPath);
      if (myImages.isRelative())
            myImages.setFile(QDir::home(), preferences.myImagesPath);
      QString defaultPath = myImages.absoluteFilePath();

      // compute the image capture path
      QString myCapturePath;
      // if no saves were made for current score, then use the score's name as default
      if (!cs->rootScore()->savedCapture()) {
            // set the current score's name as the default name for saved captures
            QString scoreName = cs->rootScore()->fileInfo()->completeBaseName();
            QString name = createDefaultFileName(scoreName);
            QString fname = QString("%1/%2").arg(defaultPath).arg(name);
            QFileInfo myCapture(fname);
            if (myCapture.isRelative())
                myCapture.setFile(QDir::home(), fname);
            myCapturePath = myCapture.absoluteFilePath();
            }
      else
            myCapturePath = lastSaveCaptureName;

      if (preferences.nativeDialogs) {
            QString fn;
            fn = QFileDialog::getSaveFileName(
               this,
               title,
               myCapturePath,
               filter,
               selectedFilter
               );
            // if a save was made for this current score
            if (!fn.isEmpty()) {
                cs->rootScore()->setSavedCapture(true);
                // store the last name used for saving an image capture
                lastSaveCaptureName = fn;
                }
            return fn;
            }

      QList<QUrl> urls;
      urls.append(QUrl::fromLocalFile(QDir::homePath()));
      urls.append(QUrl::fromLocalFile(defaultPath));
      urls.append(QUrl::fromLocalFile(QDir::currentPath()));

      if (saveImageDialog == 0) {
            saveImageDialog = new QFileDialog(this);
            saveImageDialog->setFileMode(QFileDialog::AnyFile);
            saveImageDialog->setAcceptMode(QFileDialog::AcceptSave);
            saveImageDialog->setOption(QFileDialog::DontConfirmOverwrite, false);
            saveImageDialog->setOption(QFileDialog::DontUseNativeDialog, true);
            saveImageDialog->setWindowTitle(title);
            saveImageDialog->setNameFilter(filter);
            saveImageDialog->setDirectory(defaultPath);

            restoreDialogState("saveImageDialog", saveImageDialog);
            saveImageDialog->setAcceptMode(QFileDialog::AcceptSave);
            }

      // setup side bar urls
      saveImageDialog->setSidebarUrls(urls);

      // set file's name using the computed path
      saveImageDialog->selectFile(myCapturePath);

      if (saveImageDialog->exec()) {
            QStringList result = saveImageDialog->selectedFiles();
            *selectedFilter = saveImageDialog->selectedNameFilter();
            // a save was made for this current score
            cs->rootScore()->setSavedCapture(true);
            // store the last name used for saving an image capture
            lastSaveCaptureName = result.front();
            return result.front();
            }
      return QString();
      }

//---------------------------------------------------------
//   getPaletteFilename
//---------------------------------------------------------

QString MuseScore::getPaletteFilename(bool open, const QString& name)
      {
      QString title;
      QString filter;
      QString wd      = QString("%1/%2").arg(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)).arg(QCoreApplication::applicationName());
      if (open) {
            title  = tr("Load Palette");
            filter = tr("MuseScore Palette") + " (*.mpal)";
            }
      else {
            title  = tr("Save Palette");
            filter = tr("MuseScore Palette") + " (*.mpal)";
            }

      QFileInfo myPalettes(wd);
      QString defaultPath = myPalettes.absoluteFilePath();
      if (!name.isEmpty()) {
            QString fname = createDefaultFileName(name);
            QFileInfo myName(fname);
            if (myName.isRelative())
                  myName.setFile(defaultPath, fname);
            defaultPath = myName.absoluteFilePath();
            }

      if (preferences.nativeDialogs) {
            QString fn;
            if (open)
                  fn = QFileDialog::getOpenFileName(this, title, defaultPath, filter);
            else
                  fn = QFileDialog::getSaveFileName(this, title, defaultPath, filter);
            return fn;
            }

      QFileDialog* dialog;
      QList<QUrl> urls;
      urls.append(QUrl::fromLocalFile(QDir::homePath()));
      urls.append(QUrl::fromLocalFile(QDir::currentPath()));
      urls.append(QUrl::fromLocalFile(defaultPath));

      if (open) {
            if (loadPaletteDialog == 0) {
                  loadPaletteDialog = new QFileDialog(this);
                  loadPaletteDialog->setFileMode(QFileDialog::ExistingFile);
                  loadPaletteDialog->setOption(QFileDialog::DontUseNativeDialog, true);
                  loadPaletteDialog->setDirectory(defaultPath);

                  restoreDialogState("loadPaletteDialog", loadPaletteDialog);
                  loadPaletteDialog->setAcceptMode(QFileDialog::AcceptOpen);
                  }
            urls.append(QUrl::fromLocalFile(mscoreGlobalShare+"/styles"));
            dialog = loadPaletteDialog;
            }
      else {
            if (savePaletteDialog == 0) {
                  savePaletteDialog = new QFileDialog(this);
                  savePaletteDialog->setAcceptMode(QFileDialog::AcceptSave);
                  savePaletteDialog->setFileMode(QFileDialog::AnyFile);
                  savePaletteDialog->setOption(QFileDialog::DontConfirmOverwrite, false);
                  savePaletteDialog->setOption(QFileDialog::DontUseNativeDialog, true);
                  savePaletteDialog->setDirectory(defaultPath);

                  restoreDialogState("savePaletteDialog", savePaletteDialog);
                  savePaletteDialog->setAcceptMode(QFileDialog::AcceptSave);
                  }
            dialog = savePaletteDialog;
            }
      dialog->setWindowTitle(title);
      dialog->setNameFilter(filter);

      // setup side bar urls
      dialog->setSidebarUrls(urls);

      if (dialog->exec()) {
            QStringList result = dialog->selectedFiles();
            return result.front();
            }
      return QString();
      }

//---------------------------------------------------------
//   getPluginFilename
//---------------------------------------------------------

QString MuseScore::getPluginFilename(bool open)
      {
      QString title;
      QString filter;
      if (open) {
            title  = tr("Load Plugin");
            filter = tr("MuseScore Plugin") + " (*.qml)";
            }
      else {
            title  = tr("Save Plugin");
            filter = tr("MuseScore Plugin File") + " (*.qml)";
            }

      QFileInfo myPlugins(preferences.myPluginsPath);
      if (myPlugins.isRelative())
            myPlugins.setFile(QDir::home(), preferences.myPluginsPath);
      QString defaultPath = myPlugins.absoluteFilePath();

      QString name  = createDefaultFileName("Plugin");
      QString fname = QString("%1/%2.qml").arg(defaultPath).arg(name);
      if (preferences.nativeDialogs) {
            QString fn;
            if (open)
                  fn = QFileDialog::getOpenFileName(this, title, defaultPath, filter);
            else
                  fn = QFileDialog::getSaveFileName(this, title, defaultPath, filter);
            return fn;
            }

      QFileDialog* dialog;
      QList<QUrl> urls;
      QString home = QDir::homePath();
      urls.append(QUrl::fromLocalFile(home));
      urls.append(QUrl::fromLocalFile(defaultPath));
      urls.append(QUrl::fromLocalFile(QDir::currentPath()));

      if (open) {
            if (loadPluginDialog == 0) {
                  loadPluginDialog = new QFileDialog(this);
                  loadPluginDialog->setFileMode(QFileDialog::ExistingFile);
                  loadPluginDialog->setOption(QFileDialog::DontUseNativeDialog, true);
                  loadPluginDialog->setDirectory(defaultPath);

                  QSettings settings;
                  loadPluginDialog->restoreState(settings.value("loadPluginDialog").toByteArray());
                  loadPluginDialog->setAcceptMode(QFileDialog::AcceptOpen);
                  }
            urls.append(QUrl::fromLocalFile(mscoreGlobalShare+"/styles"));
            dialog = loadPluginDialog;
            }
      else {
            if (savePluginDialog == 0) {
                  savePluginDialog = new QFileDialog(this);
                  QSettings settings;
                  savePluginDialog->restoreState(settings.value("savePluginDialog").toByteArray());
                  savePluginDialog->setAcceptMode(QFileDialog::AcceptSave);
                  savePluginDialog->setFileMode(QFileDialog::AnyFile);
                  savePluginDialog->setOption(QFileDialog::DontConfirmOverwrite, false);
                  savePluginDialog->setOption(QFileDialog::DontUseNativeDialog, true);
                  savePluginDialog->setWindowTitle(tr("Save Plugin"));
                  savePluginDialog->setNameFilter(filter);
                  savePluginDialog->setDirectory(defaultPath);
                  savePluginDialog->selectFile(fname);
                  }
            dialog = savePluginDialog;
            }
      dialog->setWindowTitle(title);
      dialog->setNameFilter(filter);

      // setup side bar urls
      dialog->setSidebarUrls(urls);

      if (dialog->exec()) {
            QStringList result = dialog->selectedFiles();
            return result.front();
            }
      return QString();
      }

//---------------------------------------------------------
//   getDrumsetFilename
//---------------------------------------------------------

QString MuseScore::getDrumsetFilename(bool open)
      {
      QString title;
      QString filter;
      if (open) {
            title  = tr("Load Drumset");
            filter = tr("MuseScore Drumset") + " (*.drm)";
            }
      else {
            title  = tr("Save Drumset");
            filter = tr("MuseScore Drumset File") + " (*.drm)";
            }

      QFileInfo myStyles(preferences.myStylesPath);
      if (myStyles.isRelative())
            myStyles.setFile(QDir::home(), preferences.myStylesPath);
      QString defaultPath  = myStyles.absoluteFilePath();

      if (preferences.nativeDialogs) {
            QString fn;
            if (open)
                  fn = QFileDialog::getOpenFileName(this, title, defaultPath, filter);
            else
                  fn = QFileDialog::getSaveFileName(this, title, defaultPath, filter);
            return fn;
            }


      QFileDialog* dialog;
      QList<QUrl> urls;
      QString home = QDir::homePath();
      urls.append(QUrl::fromLocalFile(home));
      urls.append(QUrl::fromLocalFile(defaultPath));
      urls.append(QUrl::fromLocalFile(QDir::currentPath()));

      if (open) {
            if (loadDrumsetDialog == 0) {
                  loadDrumsetDialog = new QFileDialog(this);
                  loadDrumsetDialog->setFileMode(QFileDialog::ExistingFile);
                  loadDrumsetDialog->setOption(QFileDialog::DontUseNativeDialog, true);
                  loadDrumsetDialog->setDirectory(defaultPath);

                  restoreDialogState("loadDrumsetDialog", loadDrumsetDialog);
                  loadDrumsetDialog->setAcceptMode(QFileDialog::AcceptOpen);
                  }
            urls.append(QUrl::fromLocalFile(mscoreGlobalShare+"/styles"));
            dialog = loadDrumsetDialog;
            }
      else {
            if (saveDrumsetDialog == 0) {
                  saveDrumsetDialog = new QFileDialog(this);
                  saveDrumsetDialog->setAcceptMode(QFileDialog::AcceptSave);
                  saveDrumsetDialog->setFileMode(QFileDialog::AnyFile);
                  saveDrumsetDialog->setOption(QFileDialog::DontConfirmOverwrite, false);
                  saveDrumsetDialog->setOption(QFileDialog::DontUseNativeDialog, true);
                  saveDrumsetDialog->setDirectory(defaultPath);

                  restoreDialogState("saveDrumsetDialog", saveDrumsetDialog);
                  saveDrumsetDialog->setAcceptMode(QFileDialog::AcceptSave);
                  }
            dialog = saveDrumsetDialog;
            }
      dialog->setWindowTitle(title);
      dialog->setNameFilter(filter);

      // setup side bar urls
      dialog->setSidebarUrls(urls);

      if (dialog->exec()) {
            QStringList result = dialog->selectedFiles();
            return result.front();
            }
      return QString();
      }

//---------------------------------------------------------
//   printFile
//---------------------------------------------------------

void MuseScore::printFile()
      {
      LayoutMode layoutMode = cs->layoutMode();
      cs->switchToPageMode();

      QPrinter printerDev(QPrinter::HighResolution);
      const PageFormat* pf = cs->pageFormat();
      QPageSize ps(QPageSize::id(pf->size(), QPageSize::Inch));
      printerDev.setPageSize(ps);
      printerDev.setPageOrientation(
            pf->size().width() > pf->size().height() ? QPageLayout::Landscape : QPageLayout::Portrait
         );

      printerDev.setCreator("MuseScore Version: " VERSION);
      printerDev.setFullPage(true);
      if (!printerDev.setPageMargins(QMarginsF()))
            qDebug("unable to clear printer margins");
      printerDev.setColorMode(QPrinter::Color);
      printerDev.setDocName(cs->name());
      printerDev.setOutputFormat(QPrinter::NativeFormat);
      int pages    = cs->pages().size();
      printerDev.setFromTo(1, pages);

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
      printerDev.setOutputFileName("");
#else
      // when setting this on windows platform, pd.exec() does not
      // show dialog
      printerDev.setOutputFileName(cs->fileInfo()->path() + "/" + cs->name() + ".pdf");
#endif

      QPrintDialog pd(&printerDev, 0);

      if (pd.exec()) {
            QPainter p(&printerDev);
            p.setRenderHint(QPainter::Antialiasing, true);
            p.setRenderHint(QPainter::TextAntialiasing, true);
            double mag = printerDev.logicalDpiX() / DPI;

            p.scale(mag, mag);

            int fromPage = printerDev.fromPage() - 1;
            int toPage   = printerDev.toPage() - 1;
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
            }

      if (layoutMode != cs->layoutMode())
            cs->endCmd(true);       // rollback
      }

//---------------------------------------------------------
//   exportFile
//    return true on success
//---------------------------------------------------------

void MuseScore::exportFile()
      {
      QStringList fl;
      fl.append(tr("PDF File") + " (*.pdf)");
      fl.append(tr("PNG Bitmap Graphic") + " (*.png)");
      fl.append(tr("Scalable Vector Graphics") + " (*.svg)");
#ifdef HAS_AUDIOFILE
      fl.append(tr("Wave Audio") + " (*.wav)");
      fl.append(tr("FLAC Audio") + " (*.flac)");
      fl.append(tr("Ogg Vorbis Audio") + " (*.ogg)");
#endif
#ifdef USE_LAME
      fl.append(tr("MP3 Audio") + " (*.mp3)");
#endif
      fl.append(tr("Standard MIDI File") + " (*.mid)");
      fl.append(tr("Compressed MusicXML File") + " (*.mxl)");
      fl.append(tr("Uncompressed MusicXML File") + " (*.musicxml)");
      fl.append(tr("Uncompressed MuseScore File") + " (*.mscx)");

      QString saveDialogTitle = tr("Export");

      QSettings settings;
      if (lastSaveCopyDirectory.isEmpty())
            lastSaveCopyDirectory = settings.value("lastSaveCopyDirectory", preferences.myScoresPath).toString();
      if (lastSaveDirectory.isEmpty())
            lastSaveDirectory = settings.value("lastSaveDirectory", preferences.myScoresPath).toString();
      QString saveDirectory = lastSaveCopyDirectory;

      if (saveDirectory.isEmpty())
            saveDirectory = preferences.myScoresPath;

      if (lastSaveCopyFormat.isEmpty())
            lastSaveCopyFormat = settings.value("lastSaveCopyFormat", "pdf").toString();
      QString saveFormat = lastSaveCopyFormat;

      if (saveFormat.isEmpty())
            saveFormat = "pdf";

      QString name;
#ifdef Q_OS_WIN
      if (QSysInfo::WindowsVersion == QSysInfo::WV_XP) {
            if (cs->parentScore())
                  name = QString("%1/%2-%3").arg(saveDirectory).arg(cs->parentScore()->name()).arg(createDefaultFileName(cs->name()));
            else
                  name = QString("%1/%2").arg(saveDirectory).arg(cs->name());
            }
      else
#endif
      if (cs->parentScore())
            name = QString("%1/%2-%3.%4").arg(saveDirectory).arg(cs->parentScore()->name()).arg(createDefaultFileName(cs->name())).arg(saveFormat);
      else
            name = QString("%1/%2.%3").arg(saveDirectory).arg(cs->name()).arg(saveFormat);

      int idx = fl.indexOf(QRegExp(".+\\(\\*\\." + saveFormat + "\\)"), Qt::CaseInsensitive);
      if (idx != -1)
            fl.move(idx, 0);
      QString filter = fl.join(";;");
      QString fn = getSaveScoreName(saveDialogTitle, name, filter);
      if (fn.isEmpty())
            return;

      QFileInfo fi(fn);
      lastSaveCopyDirectory = fi.absolutePath();
      lastSaveCopyFormat = fi.suffix();

      if (fi.suffix().isEmpty())
            QMessageBox::critical(this, tr("Export"), tr("Cannot determine file type"));
      else
            saveAs(cs, true, fn, fi.suffix());
      }

//---------------------------------------------------------
//   exportParts
//    return true on success
//---------------------------------------------------------

bool MuseScore::exportParts()
      {
      QStringList fl;
      fl.append(tr("PDF File") + " (*.pdf)");
      fl.append(tr("PNG Bitmap Graphic") + " (*.png)");
      fl.append(tr("Scalable Vector Graphics") + " (*.svg)");
#ifdef HAS_AUDIOFILE
      fl.append(tr("Wave Audio") + " (*.wav)");
      fl.append(tr("FLAC Audio") + " (*.flac)");
      fl.append(tr("Ogg Vorbis Audio") + " (*.ogg)");
#endif
#ifdef USE_LAME
      fl.append(tr("MP3 Audio") + " (*.mp3)");
#endif
      fl.append(tr("Standard MIDI File") + " (*.mid)");
      fl.append(tr("Compressed MusicXML File") + " (*.mxl)");
      fl.append(tr("Uncompressed MusicXML File") + " (*.musicxml)");
      fl.append(tr("MuseScore File") + " (*.mscz)");
      fl.append(tr("Uncompressed MuseScore File") + " (*.mscx)");

      QString saveDialogTitle = tr("Export Parts");

      QSettings settings;
      if (lastSaveCopyDirectory.isEmpty())
          lastSaveCopyDirectory = settings.value("lastSaveCopyDirectory", preferences.myScoresPath).toString();
      if (lastSaveDirectory.isEmpty())
          lastSaveDirectory = settings.value("lastSaveDirectory", preferences.myScoresPath).toString();
      QString saveDirectory = lastSaveCopyDirectory;

      if (saveDirectory.isEmpty()) {
          saveDirectory = preferences.myScoresPath;
          }

      if (saveDirectory.isEmpty())
            saveDirectory = preferences.myScoresPath;

      if (lastSaveCopyFormat.isEmpty())
            lastSaveCopyFormat = settings.value("lastSaveCopyFormat", "pdf").toString();
      QString saveFormat = lastSaveCopyFormat;

      if (saveFormat.isEmpty())
            saveFormat = "pdf";

      QString scoreName = cs->parentScore() ? cs->parentScore()->name() : cs->name();
      QString name;
#ifdef Q_OS_WIN
      if (QSysInfo::WindowsVersion == QSysInfo::WV_XP)
            name = QString("%1/%2").arg(saveDirectory).arg(scoreName);
      else
#endif
      name = QString("%1/%2.%3").arg(saveDirectory).arg(scoreName).arg(saveFormat);

      int idx = fl.indexOf(QRegExp(".+\\(\\*\\." + saveFormat + "\\)"), Qt::CaseInsensitive);
      if (idx != -1)
            fl.move(idx, 0);
      QString filter = fl.join(";;");
      QString fn = getSaveScoreName(saveDialogTitle, name, filter);
      if (fn.isEmpty())
          return false;

      QFileInfo fi(fn);
      lastSaveCopyDirectory = fi.absolutePath();
      lastSaveCopyFormat = fi.suffix();

      QString ext = fi.suffix();
      if (ext.isEmpty()) {
            QMessageBox::critical(this, tr("Export Parts"), tr("Cannot determine file type"));
            return false;
            }

      Score* thisScore = cs->rootScore();
      bool overwrite = false;
      bool noToAll = false;
      QString confirmReplaceTitle = tr("Confirm Replace");
      QString confirmReplaceMessage = tr("\"%1\" already exists.\nDo you want to replace it?\n");
      QString replaceMessage = tr("Replace");
      QString skipMessage = tr("Skip");
      foreach (Excerpt* e, thisScore->excerpts())  {
            Score* pScore = e->partScore();
            QString partfn = fi.absolutePath() + QDir::separator() + fi.completeBaseName() + "-" + createDefaultFileName(pScore->name()) + "." + ext;
            QFileInfo fip(partfn);
            if(fip.exists() && !overwrite) {
                  if(noToAll)
                        continue;
                  QMessageBox msgBox( QMessageBox::Question, confirmReplaceTitle,
                        confirmReplaceMessage.arg(QDir::toNativeSeparators(partfn)),
                        QMessageBox::Yes |  QMessageBox::YesToAll | QMessageBox::No |  QMessageBox::NoToAll);
                  msgBox.setButtonText(QMessageBox::Yes, replaceMessage);
                  msgBox.setButtonText(QMessageBox::No, skipMessage);
                  msgBox.setButtonText(QMessageBox::YesToAll, tr("Replace All"));
                  msgBox.setButtonText(QMessageBox::NoToAll, tr("Skip All"));
                  int sb = msgBox.exec();
                  if(sb == QMessageBox::YesToAll) {
                        overwrite = true;
                        }
                  else if (sb == QMessageBox::NoToAll) {
                        noToAll = true;
                        continue;
                        }
                  else if (sb == QMessageBox::No)
                        continue;
                  }

            if (!saveAs(pScore, true, partfn, ext))
                  return false;
            }
      // For PDF, also export score and parts together
      if (ext.toLower() == "pdf") {
            QList<Score*> scores;
            scores.append(thisScore);
            foreach(Excerpt* e, thisScore->excerpts())  {
                  scores.append(e->partScore());
                  }
            QString partfn(fi.absolutePath() + QDir::separator() + fi.completeBaseName() + "-" + createDefaultFileName(tr("Score_and_Parts")) + ".pdf");
            QFileInfo fip(partfn);
            if(fip.exists() && !overwrite) {
                  if (!noToAll) {
                        QMessageBox msgBox( QMessageBox::Question, confirmReplaceTitle,
                              confirmReplaceMessage.arg(QDir::toNativeSeparators(partfn)),
                              QMessageBox::Yes | QMessageBox::No);
                        msgBox.setButtonText(QMessageBox::Yes, replaceMessage);
                        msgBox.setButtonText(QMessageBox::No, skipMessage);
                        int sb = msgBox.exec();
                        if(sb == QMessageBox::Yes) {
                              if (!savePdf(scores, partfn))
                                    return false;
                              }
                        }
                  }
            else if (!savePdf(scores, partfn))
                  return false;
      }
      if(!noToAll)
            QMessageBox::information(this, tr("Export Parts"), tr("Parts were successfully exported"));
      return true;
      }

//---------------------------------------------------------
//   saveAs
//---------------------------------------------------------

bool MuseScore::saveAs(Score* cs, bool saveCopy, const QString& path, const QString& ext)
      {
      bool rv = false;
      QString suffix = "." + ext;
      QString fn(path);
      if (!fn.endsWith(suffix))
            fn += suffix;

      LayoutMode layoutMode = cs->layoutMode();
      if (ext == "mscx" || ext == "mscz") {
            // save as mscore *.msc[xz] file
            QFileInfo fi(fn);
            rv = true;
            // store new file and path into score fileInfo
            // to have it accessible to resources
            QString originalScoreFName(cs->fileInfo()->canonicalFilePath());
            cs->fileInfo()->setFile(fn);
            if (cs->parentScore()) { // clone metaTags from masterScore
                  QMapIterator<QString, QString> j(cs->parentScore()->metaTags());
                  while (j.hasNext()) {
                        j.next();
                        if (j.key() != "partName") // don't copy "partName" should that exist in masterScore
                              cs->metaTags().insert(j.key(), j.value());
#if defined(Q_OS_WIN)   // Update "platform", may not be worth the effort
                        cs->metaTags().insert("platform", "Microsoft Windows");
#elif defined(Q_OS_MAC)
                        cs->metaTags().insert("platform", "Apple Macintosh");
#elif defined(Q_OS_LINUX)
                        cs->metaTags().insert("platform", "Linux");
#else
                        cs->metaTags().insert("platform", "Unknown");
#endif
                        cs->metaTags().insert("source", ""); // Empty "source" to avoid clashes with masterrScore when doing "Save online"
                        cs->metaTags().insert("creationDate", QDate::currentDate().toString(Qt::ISODate)); // update "creationDate"
                        }
                  }
            try {
                  if (ext == "mscz")
                        cs->saveCompressedFile(fi, false);
                  else
                        cs->saveFile(fi);
                  }
            catch (QString s) {
                  rv = false;
                  QMessageBox::critical(this, tr("Save As"), s);
                  }
            if (cs->parentScore()) { // remove metaTags added above
                  QMapIterator<QString, QString> j(cs->parentScore()->metaTags());
                  while (j.hasNext()) {
                        j.next();
                        // remove all but "partName", should that exist in masterScore
                        if (j.key() != "partName")
                              cs->metaTags().remove(j.key());
                        }
                  }
            cs->fileInfo()->setFile(originalScoreFName);          // restore original file name

            if (rv && !saveCopy) {
                  cs->fileInfo()->setFile(fn);
                  updateWindowTitle(cs);
                  cs->undo()->setClean();
                  dirtyChanged(cs);
                  cs->setCreated(false);
                  addRecentScore(cs);
                  writeSessionFile(false);
                  }
            }
      else if (ext == "musicxml") {
            // save as MusicXML *.musicxml file
            rv = saveXml(cs, fn);
            }
      else if (ext == "mxl") {
            // save as compressed MusicXML *.mxl file
            rv = saveMxl(cs, fn);
            }
      else if (ext == "mid") {
            // save as midi file *.mid
            rv = saveMidi(cs, fn);
            }
      else if (ext == "pdf") {
            // save as pdf file *.pdf
            cs->switchToPageMode();
            rv = savePdf(cs, fn);
            }
      else if (ext == "png") {
            // save as png file *.png
            cs->switchToPageMode();
            rv = savePng(cs, fn);
            }
      else if (ext == "svg") {
            // save as svg file *.svg
            cs->switchToPageMode();
            rv = saveSvg(cs, fn);
            }
#ifdef HAS_AUDIOFILE
      else if (ext == "wav" || ext == "flac" || ext == "ogg")
            rv = saveAudio(cs, fn);
#endif
#ifdef USE_LAME
      else if (ext == "mp3")
            rv = saveMp3(cs, fn);
#endif
      else if (ext == "spos") {
            cs->switchToPageMode();
            // save positions of segments
            rv = savePositions(cs, fn, true);
            }
      else if (ext == "mpos") {
            cs->switchToPageMode();
            // save positions of measures
            rv = savePositions(cs, fn, false);
            }
      else if (ext == "mlog") {
            rv = cs->sanityCheck(fn);
            }
      else if (ext == "metajson") {
            rv = saveMetadataJSON(cs, fn);
            }
      else {
            qDebug("Internal error: unsupported extension <%s>",
               qPrintable(ext));
            return false;
            }
      if (!rv && !MScore::noGui)
            QMessageBox::critical(this, tr("MuseScore:"), tr("Cannot write into %1").arg(fn));
      if (layoutMode != cs->layoutMode())
            cs->endCmd(true);       // rollback
      return rv;
      }

//---------------------------------------------------------
//   saveMidi
//---------------------------------------------------------

bool MuseScore::saveMidi(Score* score, const QString& name)
      {
      ExportMidi em(score);
      return em.write(name, preferences.midiExpandRepeats);
      }

bool MuseScore::saveMidi(Score* score, QIODevice* device)
      {
      ExportMidi em(score);
      return em.write(device, preferences.midiExpandRepeats);
      }

//---------------------------------------------------------
//   savePdf
//---------------------------------------------------------

bool MuseScore::savePdf(const QString& saveName)
      {
      return savePdf(cs, saveName);
      }

bool MuseScore::savePdf(Score* score, const QString& saveName)
      {
      QPdfWriter printerDev(saveName);
      return savePdf(score, printerDev);
      }

bool MuseScore::savePdf(Score* score, QPdfWriter& printerDev)
      {
      score->setPrinting(true);
      MScore::pdfPrinting = true;
      printerDev.setResolution(preferences.exportPdfDpi);
      const PageFormat* pf = score->pageFormat();
      printerDev.setPageSize(QPageSize(pf->size(), QPageSize::Inch));

      printerDev.setCreator("MuseScore Version: " VERSION);
      if (!printerDev.setPageMargins(QMarginsF()))
            qDebug("unable to clear printer margins");

      QString title = score->metaTag("workTitle");
      if (title.isEmpty()) // workTitle unset?
            title = score->rootScore()->title(); // fall back to (root)score's tab title
      if (score != score->rootScore()) { // excerpt?
            QString partname = score->metaTag("partName");
            if (partname.isEmpty()) // partName unset?
                  partname = score->title(); // fall back to excerpt's tab title
            title += " - " + partname;
            }
      printerDev.setTitle(title); // set PDF's meta data for Title

      QPainter p;
      if (!p.begin(&printerDev))
            return false;
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::TextAntialiasing, true);
      double mag = printerDev.logicalDpiX() / DPI;
      p.scale(mag, mag);

      const QList<Page*> pl = score->pages();
      int pages    = pl.size();
      bool firstPage = true;
      for (int n = 0; n < pages; ++n) {
            if (!firstPage)
                  printerDev.newPage();
            firstPage = false;
            score->print(&p, n);
            }
      p.end();
      score->setPrinting(false);
      MScore::pdfPrinting = false;
      return true;
      }

bool MuseScore::savePdf(QList<Score*> cs, const QString& saveName)
      {
      if (cs.empty())
            return false;
      Score* firstScore = cs[0];

      QPrinter printerDev(QPrinter::HighResolution);
      const PageFormat* pf = firstScore->pageFormat();
      printerDev.setPaperSize(pf->size(), QPrinter::Inch);

      printerDev.setCreator("MuseScore Version: " VERSION);
      printerDev.setFullPage(true);
      if (!printerDev.setPageMargins(QMarginsF()))
            qDebug("unable to clear printer margins");
      printerDev.setColorMode(QPrinter::Color);

      QString title = firstScore->metaTag("workTitle");
      if (title.isEmpty()) // workTitle unset?
            title = firstScore->title(); // fall back to (root)score's tab title
      title += " - " + tr("Score and Parts");
      printerDev.setDocName(title);
      printerDev.setOutputFormat(QPrinter::PdfFormat);

      printerDev.setOutputFileName(saveName);

      QPainter p;
      if (!p.begin(&printerDev))
            return false;

      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::TextAntialiasing, true);
      double mag = printerDev.logicalDpiX() / DPI;
      p.scale(mag, mag);

      bool firstPage = true;
      for (Score* s : cs) {
            LayoutMode layoutMode = s->layoutMode();
            s->switchToPageMode();
            s->setPrinting(true);
            MScore::pdfPrinting = true;

            const PageFormat* pf = s->pageFormat();
            printerDev.setPaperSize(pf->size(), QPrinter::Inch);

            const QList<Page*> pl = s->pages();
            int pages    = pl.size();
            for (int n = 0; n < pages; ++n) {
                  if (!firstPage)
                        printerDev.newPage();
                  firstPage = false;
                  s->print(&p, n);
                  }
            //reset score
            s->setPrinting(false);
            MScore::pdfPrinting = false;

            if (layoutMode != s->layoutMode())
                  s->endCmd(true);       // rollback
            }
      p.end();
      return true;
      }

//---------------------------------------------------------
//   importSoundfont
//---------------------------------------------------------

void importSoundfont(QString name)
      {
      QFileInfo info(name);
      int ret = QMessageBox::question(0, QWidget::tr("Install SoundFont"),
            QWidget::tr("Do you want to install the SoundFont %1?").arg(info.fileName()),
             QMessageBox::Yes|QMessageBox::No, QMessageBox::NoButton);
      if (ret == QMessageBox::Yes) {
            QStringList pl = preferences.mySoundfontsPath.split(";");
            QString destPath;
            for (QString s : pl) {
                  QFileInfo dest(s);
                  if (dest.isWritable())
                        destPath = s;
                  }
            if (!destPath.isEmpty()) {
                  QString destFilePath = destPath+ "/" +info.fileName();
                  QFileInfo destFileInfo(destFilePath);
                  QFile destFile(destFilePath);
                  if (destFileInfo.exists()) {
                        int ret1 = QMessageBox::question(0, QWidget::tr("Overwrite?"),
                          QWidget::tr("%1 already exists.\nDo you want to overwrite it?").arg(destFileInfo.absoluteFilePath()),
                          QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
                        if (ret1 == QMessageBox::No)
                              return;
                        destFile.remove();
                        }
                  QFile orig(name);
                  if (orig.copy(destFilePath)) {
                        QMessageBox::information(0, QWidget::tr("SoundFont installed"), QWidget::tr("SoundFont installed. Please go to View > Synthesizer to add it and View > Mixer to choose an instrument sound."));
                        }
                  }
            }
      }

//---------------------------------------------------------
//   importExtension
//---------------------------------------------------------

void importExtension(QString name)
      {
      mscore->importExtension(name);
      }

//---------------------------------------------------------
//   readScore
///   Import file \a name
//---------------------------------------------------------

Score::FileError readScore(Score* score, QString name, bool ignoreVersionError)
      {
      QFileInfo info(name);
      QString suffix  = info.suffix().toLower();
      score->setName(info.completeBaseName());
      score->setImportedFilePath(name);

      if (suffix == "mscz" || suffix == "mscx") {
            Score::FileError rv = score->loadMsc(name, ignoreVersionError);
            if (score && score->fileInfo()->path().startsWith(":/"))
                  score->setCreated(true);
            if (rv != Score::FileError::FILE_NO_ERROR)
                  return rv;
            }
      else if (suffix == "sf2" || suffix == "sf3") {
            importSoundfont(name);
            return Score::FileError::FILE_IGNORE_ERROR;
            }
      else if (suffix == "muxt") {
           importExtension(name);
           return Score::FileError::FILE_IGNORE_ERROR;
           }
      else {
            // typedef Score::FileError (*ImportFunction)(Score*, const QString&);
            struct ImportDef {
                  const char* extension;
                  // ImportFunction importF;
                  Score::FileError (*importF)(Score*, const QString&);
                  };
            static const ImportDef imports[] = {
                  { "xml",  &importMusicXml           },
                  { "musicxml", &importMusicXml       },
                  { "mxl",  &importCompressedMusicXml },
                  { "mid",  &importMidi               },
                  { "midi", &importMidi               },
                  { "kar",  &importMidi               },
                  { "md",   &importMuseData           },
                  { "mgu",  &importBB                 },
                  { "sgu",  &importBB                 },
                  { "cap",  &importCapella            },
                  { "capx", &importCapXml             },
                  { "ove",  &importOve                },
                  { "scw",  &importOve                },
#ifdef OMR
                  { "pdf",  &importPdf                },
#endif
                  { "bww",  &importBww                },
                  { "gtp",  &importGTP                },
                  { "gp3",  &importGTP                },
                  { "gp4",  &importGTP                },
                  { "gp5",  &importGTP                },
                  { "gpx",  &importGTP                },
                  };

            // import
            if (!preferences.importStyleFile.isEmpty()) {
                  QFile f(preferences.importStyleFile);
                  // silently ignore style file on error
                  if (f.open(QIODevice::ReadOnly))
                        score->style()->load(&f);
                  }
            else {
                  if (score->style()->value(StyleIdx::chordsXmlFile).toBool())
                        score->style()->chordList()->read("chords.xml");
                  score->style()->chordList()->read(score->styleSt(StyleIdx::chordDescriptionFile));
                  }
            bool found = false;
            for (auto i : imports) {
                  if (i.extension == suffix) {
                        Score::FileError rv = (*i.importF)(score, name);
                        if (rv != Score::FileError::FILE_NO_ERROR)
                              return rv;
                        found = true;
                        break;
                        }
                  }
            if (!found) {
                  qDebug("unknown file suffix <%s>, name <%s>", qPrintable(suffix), qPrintable(name));
                  return Score::FileError::FILE_UNKNOWN_TYPE;
                  }
            score->setMetaTag("originalFormat", suffix);
            score->connectTies();
            score->setCreated(true); // force save as for imported files
            }

      score->setLayoutAll(true);
      for (Score* s : score->scoreList()) {
            s->setPlaylistDirty();
            s->rebuildMidiMapping();
            s->updateChannel();
            s->setSoloMute();
            s->addLayoutFlags(LayoutFlag::FIX_TICKS | LayoutFlag::FIX_PITCH_VELO);
            }
      score->setSaved(false);
      score->update();
      if (!ignoreVersionError && !MScore::noGui)
            if (!score->sanityCheck())
                  return Score::FileError::FILE_CORRUPTED;
      return Score::FileError::FILE_NO_ERROR;
      }

//---------------------------------------------------------
//   saveAs
//    return true on success
//---------------------------------------------------------

/**
 Save the current score using a different name or type.
 Handles the GUI's file-save-as and file-save-a-copy actions.
 The saveCopy flag, if true, does not change the name of the active score nor marks it clean.
 Return true if OK and false on error.
 */

bool MuseScore::saveAs(Score* cs, bool saveCopy)
      {
      QStringList fl;
      fl.append(tr("MuseScore File") + " (*.mscz)");
      fl.append(tr("Uncompressed MuseScore File") + " (*.mscx)");     // for debugging purposes
      QString saveDialogTitle = saveCopy ? tr("Save a Copy") :
                                           tr("Save As");

      QSettings settings;
      if (mscore->lastSaveCopyDirectory.isEmpty())
            mscore->lastSaveCopyDirectory = settings.value("lastSaveCopyDirectory", preferences.myScoresPath).toString();
      if (mscore->lastSaveDirectory.isEmpty())
            mscore->lastSaveDirectory = settings.value("lastSaveDirectory", preferences.myScoresPath).toString();
      QString saveDirectory = saveCopy ? mscore->lastSaveCopyDirectory : mscore->lastSaveDirectory;

      if (saveDirectory.isEmpty())
            saveDirectory = preferences.myScoresPath;

      QString name;
#ifdef Q_OS_WIN
      if (QSysInfo::WindowsVersion == QSysInfo::WV_XP) {
            if (cs->parentScore())
                  name = QString("%1/%2-%3").arg(saveDirectory).arg(cs->parentScore()->name()).arg(createDefaultFileName(cs->name()));
            else
                  name = QString("%1/%2").arg(saveDirectory).arg(cs->name());
            }
      else
#endif
      if (cs->parentScore())
            name = QString("%1/%2-%3.mscz").arg(saveDirectory).arg(cs->parentScore()->name()).arg(createDefaultFileName(cs->name()));
      else
            name = QString("%1/%2.mscz").arg(saveDirectory).arg(cs->name());

      QString filter = fl.join(";;");
      QString fn     = mscore->getSaveScoreName(saveDialogTitle, name, filter);
      if (fn.isEmpty())
            return false;

      QFileInfo fi(fn);
      if (saveCopy)
            mscore->lastSaveCopyDirectory = fi.absolutePath();
      else
            mscore->lastSaveDirectory = fi.absolutePath();

      if (fi.suffix().isEmpty()) {
            if (!MScore::noGui)
                  QMessageBox::critical(mscore, tr("Save As"), tr("Cannot determine file type"));
            return false;
            }
      return saveAs(cs, saveCopy, fn, fi.suffix());
      }

//---------------------------------------------------------
//   saveSelection
//    return true on success
//---------------------------------------------------------

bool MuseScore::saveSelection(Score* cs)
      {
      if (!cs->selection().isRange()) {
            if(!MScore::noGui) QMessageBox::warning(mscore, tr("Save Selection"), tr("Please select one or more measures"));
            return false;
            }
      QStringList fl;
      fl.append(tr("MuseScore File") + " (*.mscz)");
      QString saveDialogTitle = tr("Save Selection");

      QSettings settings;
      if (mscore->lastSaveDirectory.isEmpty())
            mscore->lastSaveDirectory = settings.value("lastSaveDirectory", preferences.myScoresPath).toString();
      QString saveDirectory = mscore->lastSaveDirectory;

      if (saveDirectory.isEmpty())
            saveDirectory = preferences.myScoresPath;

      QString name   = QString("%1/%2.mscz").arg(saveDirectory).arg(cs->name());
      QString filter = fl.join(";;");
      QString fn     = mscore->getSaveScoreName(saveDialogTitle, name, filter);
      if (fn.isEmpty())
            return false;

      QFileInfo fi(fn);
      mscore->lastSaveDirectory = fi.absolutePath();

      QString ext = fi.suffix();
      if (ext.isEmpty()) {
            QMessageBox::critical(mscore, tr("Save Selection"), tr("Cannot determine file type"));
            return false;
            }
      bool rv = true;
      try {
            cs->saveCompressedFile(fi, true);
            }
      catch (QString s) {
            rv = false;
            QMessageBox::critical(this, tr("Save Selected"), s);
            }
      return rv;
      }

//---------------------------------------------------------
//   addImage
//---------------------------------------------------------

void MuseScore::addImage(Score* score, Element* e)
      {
      QString fn = QFileDialog::getOpenFileName(
         0,
         tr("Insert Image"),
         "",            // lastOpenPath,
         tr("All Supported Files") + " (*.svg *.jpg *.jpeg *.png);;" +
         tr("Scalable Vector Graphics") + " (*.svg);;" +
         tr("JPEG") + " (*.jpg *.jpeg);;" +
         tr("PNG") + " (*.png)",
         0,
         preferences.nativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog
         );
      if (fn.isEmpty())
            return;

      QFileInfo fi(fn);
      Image* s = new Image(score);
      QString suffix(fi.suffix().toLower());

      if (suffix == "svg")
            s->setImageType(ImageType::SVG);
      else if (suffix == "jpg" || suffix == "jpeg" || suffix == "png")
            s->setImageType(ImageType::RASTER);
      else
            return;
      s->load(fn);
      s->setParent(e);
      score->undoAddElement(s);
      }

#if 0
//---------------------------------------------------------
//   trim
//    returns copy of source with whitespace trimmed and margin added
//---------------------------------------------------------

static QRect trim(QImage source, int margin)
      {
      int w = source.width();
      int h = source.height();
      int x1 = w;
      int x2 = 0;
      int y1 = h;
      int y2 = 0;
      for (int x = 0; x < w; ++x) {
            for (int y = 0; y < h; ++y) {
                  QRgb c = source.pixel(x, y);
                  if (c != 0 && c != 0xffffffff) {
                        if (x < x1)
                              x1 = x;
                        if (x > x2)
                              x2 = x;
                        if (y < y1)
                              y1 = y;
                        if (y > y2)
                              y2 = y;
                        }
                  }
            }
      int x = qMax(x1 - margin, 0);
      int y = qMax(y1 - margin, 0);
      w = qMin(w, x2 + 1 + margin) - x;
      h = qMin(h, y2 + 1 + margin) - y;
      return QRect(x, y, w, h);
      }
#endif

//---------------------------------------------------------
//   savePng
//    return true on success. Works with editor, shows additional windows.
//---------------------------------------------------------
bool MuseScore::savePng(Score* score, const QString& name)
      {
      //TODO! Make refactoring using savePng(Score*, QIODevice*, int)
      const bool screenshot = false;
      const bool transparent = preferences.pngTransparent;
      const double convDpi = converterDpi;
      const int localTrimMargin = trimMargin;
      const QImage::Format format = QImage::Format_ARGB32_Premultiplied;

      bool rv = true;
      score->setPrinting(!screenshot);    // dont print page break symbols etc.

      QImage::Format f;
      if (format != QImage::Format_Indexed8)
          f = format;
      else
          f = QImage::Format_ARGB32_Premultiplied;

      const QList<Page*>& pl = score->pages();
      int pages = pl.size();

      int padding = QString("%1").arg(pages).size();
      bool overwrite = false;
      bool noToAll = false;
      for (int pageNumber = 0; pageNumber < pages; ++pageNumber) {
            Page* page = pl.at(pageNumber);

            QRectF r;
            if (localTrimMargin >= 0) {
                  QMarginsF margins(localTrimMargin, localTrimMargin, localTrimMargin, localTrimMargin);
                  r = page->tbbox() + margins;
                  }
            else
                  r = page->abbox();
            int w = lrint(r.width()  * convDpi / DPI);
            int h = lrint(r.height() * convDpi / DPI);

            QImage printer(w, h, f);
            printer.setDotsPerMeterX(lrint((convDpi * 1000) / INCH));
            printer.setDotsPerMeterY(lrint((convDpi * 1000) / INCH));

            printer.fill(transparent ? 0 : 0xffffffff);

            double mag = convDpi / DPI;
            QPainter p(&printer);
            p.setRenderHint(QPainter::Antialiasing, true);
            p.setRenderHint(QPainter::TextAntialiasing, true);
            p.scale(mag, mag);
            if (localTrimMargin >= 0)
                  p.translate(-r.topLeft());

            QList<const Element*> pel = page->elements();
            qStableSort(pel.begin(), pel.end(), elementLessThan);
            paintElements(p, pel);

            if (format == QImage::Format_Indexed8) {
                  //convert to grayscale & respect alpha
                  QVector<QRgb> colorTable;
                  colorTable.push_back(QColor(0, 0, 0, 0).rgba());
                  if (!transparent) {
                        for (int i = 1; i < 256; i++)
                              colorTable.push_back(QColor(i, i, i).rgb());
                        }
                  else {
                        for (int i = 1; i < 256; i++)
                              colorTable.push_back(QColor(0, 0, 0, i).rgba());
                        }
                  printer = printer.convertToFormat(QImage::Format_Indexed8, colorTable);
                  }

            QString fileName(name);
            if (fileName.endsWith(".png"))
                  fileName = fileName.left(fileName.size() - 4);
            fileName += QString("-%1.png").arg(pageNumber+1, padding, 10, QLatin1Char('0'));
            if (!converterMode) {
                  QFileInfo fip(fileName);
                  if(fip.exists() && !overwrite) {
                        if(noToAll)
                              continue;
                        QMessageBox msgBox( QMessageBox::Question, tr("Confirm Replace"),
                              tr("\"%1\" already exists.\nDo you want to replace it?\n").arg(QDir::toNativeSeparators(fileName)),
                              QMessageBox::Yes |  QMessageBox::YesToAll | QMessageBox::No |  QMessageBox::NoToAll);
                        msgBox.setButtonText(QMessageBox::Yes, tr("Replace"));
                        msgBox.setButtonText(QMessageBox::No, tr("Skip"));
                        msgBox.setButtonText(QMessageBox::YesToAll, tr("Replace All"));
                        msgBox.setButtonText(QMessageBox::NoToAll, tr("Skip All"));
                        int sb = msgBox.exec();
                        if(sb == QMessageBox::YesToAll) {
                              overwrite = true;
                              }
                        else if (sb == QMessageBox::NoToAll) {
                              noToAll = true;
                              continue;
                              }
                        else if (sb == QMessageBox::No)
                              continue;
                        }
                  }
            rv = printer.save(fileName, "png");
            if (!rv)
                  break;
            }
      score->setPrinting(false);
      return rv;
      }

//---------------------------------------------------------
//   savePng with options
//    return true on success
//---------------------------------------------------------

bool MuseScore::savePng(Score* score, QIODevice* deviceForPngs, int pageNumber)
      {
      const bool screenshot = false;
      const bool transparent = preferences.pngTransparent;
      const double convDpi = converterDpi;
      const int localTrimMargin = trimMargin;
      const QImage::Format format = QImage::Format_ARGB32_Premultiplied;

      bool rv = true;
      score->setPrinting(!screenshot);    // dont print page break symbols etc.

      QImage::Format f;
      if (format != QImage::Format_Indexed8)
          f = format;
      else
          f = QImage::Format_ARGB32_Premultiplied;

      const QList<Page*>& pl = score->pages();
      Page* page = pl.at(pageNumber);

      QRectF r;
      if (localTrimMargin >= 0) {
            QMarginsF margins(localTrimMargin, localTrimMargin, localTrimMargin, localTrimMargin);
            r = page->tbbox() + margins;
            }
      else
            r = page->abbox();
      int w = lrint(r.width()  * convDpi / DPI);
      int h = lrint(r.height() * convDpi / DPI);

      QImage printer(w, h, f);
      printer.setDotsPerMeterX(lrint((convDpi * 1000) / INCH));
      printer.setDotsPerMeterY(lrint((convDpi * 1000) / INCH));

      printer.fill(transparent ? 0 : 0xffffffff);

      double mag = convDpi / DPI;
      QPainter p(&printer);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::TextAntialiasing, true);
      p.scale(mag, mag);
      if (localTrimMargin >= 0)
            p.translate(-r.topLeft());

      QList<const Element*> pel = page->elements();
      qStableSort(pel.begin(), pel.end(), elementLessThan);
      paintElements(p, pel);

      if (format == QImage::Format_Indexed8) {
            //convert to grayscale & respect alpha
            QVector<QRgb> colorTable;
            colorTable.push_back(QColor(0, 0, 0, 0).rgba());
            if (!transparent) {
                  for (int i = 1; i < 256; i++)
                        colorTable.push_back(QColor(i, i, i).rgb());
                  }
            else {
                  for (int i = 1; i < 256; i++)
                        colorTable.push_back(QColor(0, 0, 0, i).rgba());
                  }
            printer = printer.convertToFormat(QImage::Format_Indexed8, colorTable);
            }

      printer.save(deviceForPngs, "png");
      score->setPrinting(false);
      return rv;
      }

//---------------------------------------------------------
//   WallpaperPreview
//---------------------------------------------------------

WallpaperPreview::WallpaperPreview(QWidget* parent)
   : QFrame(parent)
      {
      _pixmap = 0;
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void WallpaperPreview::paintEvent(QPaintEvent* ev)
      {
      QPainter p(this);
      int fw = frameWidth();
      QRect r(frameRect().adjusted(fw, fw, -2*fw, -2*fw));
      if (_pixmap)
            p.drawTiledPixmap(r, *_pixmap);
      QFrame::paintEvent(ev);
      }

//---------------------------------------------------------
//   setImage
//---------------------------------------------------------

void WallpaperPreview::setImage(const QString& path)
      {
      qDebug("setImage <%s>", qPrintable(path));
      delete _pixmap;
      _pixmap = new QPixmap(path);
      update();
      }

//---------------------------------------------------------
//   getWallpaper
//---------------------------------------------------------

QString MuseScore::getWallpaper(const QString& caption)
      {
      QString filter = tr("Images") + " (*.jpg *.jpeg *.png);;" + tr("All") + " (*)";
      QString d = mscoreGlobalShare + "/wallpaper";

      if (preferences.nativeDialogs) {
            QString s = QFileDialog::getOpenFileName(
               this,                            // parent
               caption,
               d,
               filter
               );
            return s;
            }

      if (loadBackgroundDialog == 0) {
            loadBackgroundDialog = new QFileDialog(this);
            loadBackgroundDialog->setFileMode(QFileDialog::ExistingFile);
            loadBackgroundDialog->setOption(QFileDialog::DontUseNativeDialog, true);
            loadBackgroundDialog->setWindowTitle(caption);
            loadBackgroundDialog->setNameFilter(filter);
            loadBackgroundDialog->setDirectory(d);

            QSettings settings;
            loadBackgroundDialog->restoreState(settings.value("loadBackgroundDialog").toByteArray());
            loadBackgroundDialog->setAcceptMode(QFileDialog::AcceptOpen);

            QSplitter* splitter = loadBackgroundDialog->findChild<QSplitter*>("splitter");
            if (splitter) {
                  WallpaperPreview* preview = new WallpaperPreview;
                  splitter->addWidget(preview);
                  connect(loadBackgroundDialog, SIGNAL(currentChanged(const QString&)),
                     preview, SLOT(setImage(const QString&)));
                  }
            }

      //
      // setup side bar urls
      //
      QList<QUrl> urls;
      QString home = QDir::homePath();
      urls.append(QUrl::fromLocalFile(d));
      urls.append(QUrl::fromLocalFile(home));
      urls.append(QUrl::fromLocalFile(QDir::currentPath()));
      loadBackgroundDialog->setSidebarUrls(urls);

      if (loadBackgroundDialog->exec()) {
            QStringList result = loadBackgroundDialog->selectedFiles();
            return result.front();
            }
      return QString();
      }

//---------------------------------------------------------
//   MuseScore::saveSvg
//---------------------------------------------------------
// [TODO:
// [In most of the functions above the Score* parameter is named "cs".
// [But there is also a member variable in class MuseScoreCore called "cs"
// [and it too is a Score*. If the member variable exists, the functions
// [should not bother to pass it around, especially with multiple names.
// [I have continued to use the "score" argument in this function, but
// [I was just following the existing convention, inconsistent as it is.
// [All the class MuseScore member functions should use the member variable.
// [This file is currently undergoing a bunch of changes, and that's the kind
// [of edit that must be coordinated with the MuseScore master code base.
//
bool MuseScore::saveSvg(Score* score, const QString& saveName)
      {
      QString title(score->title());
      score->setPrinting(true);
      MScore::pdfPrinting = true;
      MScore::svgPrinting = true;
      const QList<Page*>& pl = score->pages();
      int pages = pl.size();
      int padding = QString("%1").arg(pages).size();
      bool overwrite = false;
      bool noToAll = false;
      for (int pageNumber = 0; pageNumber < pages; ++pageNumber) {
            Page* page = pl.at(pageNumber);
            SvgGenerator printer;
            printer.setTitle(pages > 1 ? QString("%1 (%2)").arg(title).arg(pageNumber + 1) : title);

            QString fileName(saveName);
            if (fileName.endsWith(".svg"))
                  fileName = fileName.left(fileName.size() - 4);
            fileName += QString("-%1.svg").arg(pageNumber + 1, padding, 10, QLatin1Char('0'));
            if (!converterMode) {
                  QFileInfo fip(fileName);
                  if(fip.exists() && !overwrite) {
                        if(noToAll)
                              continue;
                        QMessageBox msgBox( QMessageBox::Question, tr("Confirm Replace"),
                              tr("\"%1\" already exists.\nDo you want to replace it?\n").arg(QDir::toNativeSeparators(fileName)),
                              QMessageBox::Yes |  QMessageBox::YesToAll | QMessageBox::No |  QMessageBox::NoToAll);
                        msgBox.setButtonText(QMessageBox::Yes, tr("Replace"));
                        msgBox.setButtonText(QMessageBox::No, tr("Skip"));
                        msgBox.setButtonText(QMessageBox::YesToAll, tr("Replace All"));
                        msgBox.setButtonText(QMessageBox::NoToAll, tr("Skip All"));
                        int sb = msgBox.exec();
                        if(sb == QMessageBox::YesToAll) {
                              overwrite = true;
                              }
                        else if (sb == QMessageBox::NoToAll) {
                              noToAll = true;
                              continue;
                              }
                        else if (sb == QMessageBox::No)
                              continue;
                        }
                  }
            printer.setFileName(fileName);

            QRectF r;
            if (trimMargin >= 0) {
                  QMarginsF margins(trimMargin, trimMargin, trimMargin, trimMargin);
                  r = page->tbbox() + margins;
                  }
            else
                  r = page->abbox();
            qreal w = r.width();
            qreal h = r.height();

            printer.setSize(QSize(w, h));
            printer.setViewBox(QRectF(0, 0, w, h));
            QPainter p(&printer);
            p.setRenderHint(QPainter::Antialiasing, true);
            p.setRenderHint(QPainter::TextAntialiasing, true);
            if (trimMargin >= 0)
                  p.translate(-r.topLeft());
            // 1st pass: StaffLines
            for  (System* s : *page->systems()) {
                  for (int i = 0, n = s->staves()->size(); i < n; i++) {
                        if (score->staff(i)->invisible() || !score->staff(i)->show())
                              continue;  // ignore invisible staves
                        if (s->staves()->isEmpty() || !s->staff(i)->show())
                              continue;
                        if (!s->firstMeasure())
                              continue;

                        // The goal here is to draw SVG staff lines more efficiently.
                        // MuseScore draws staff lines by measure, but for SVG they can
                        // generally be drawn once for each system. This makes a big
                        // difference for scores that scroll horizontally on a single
                        // page. But there are exceptions to this rule:
                        //
                        //   ~ One (or more) invisible measure(s) in a system/staff ~
                        //   ~ One (or more) elements of type HBOX or VBOX          ~
                        //
                        // In these cases the SVG staff lines for the system/staff
                        // are drawn by measure.
                        //
                        bool byMeasure = false;
                        MeasureBase* mb = nullptr;
                        for (mb = s->firstMeasure(); mb != 0; mb = s->nextMeasure(mb)) {
                              if (mb->type() == Element::Type::HBOX
                               || mb->type() == Element::Type::VBOX
                               || (!static_cast<Measure*>(mb)->visible(i) && mb->system() == s)) {
                                    byMeasure = true;
                                    break;
                                    }
                              }
                        if (mb && mb->type() == Element::Type::VBOX) // no need for staff lines
                              byMeasure = false;
                        if (!byMeasure && (!s->lastMeasure() || !s->lastMeasure()->system()))
                              byMeasure = true;
                        if (byMeasure) { // Draw visible staff lines by measure
                              for (MeasureBase* mb = s->firstMeasure(); mb != 0; mb = s->nextMeasure(mb)) {
                                    if (mb->type() != Element::Type::HBOX
                                     && mb->type() != Element::Type::VBOX
                                     && static_cast<Measure*>(mb)->visible(i)) {
                                          Measure* m = static_cast<Measure*>(mb);
                                          if (score->styleB(StyleIdx::createMultiMeasureRests) && m->hasMMRest())
                                                m = m->mmRest();
                                          StaffLines* sl = m->staffLines(i);
                                          if (sl->measure()->system()) {
                                                printer.setElement(sl);
                                                paintElement(p, sl);
                                                }
                                          }
                                    }
                              }
                        else { // Draw staff lines once per system
                              StaffLines* firstSL = s->firstMeasure()->staffLines(i)->clone();
                              StaffLines*  lastSL =  s->lastMeasure()->staffLines(i);
                              firstSL->bbox().setRight(lastSL->bbox().right()
                                                    + lastSL->pagePos().x()
                                                    - firstSL->pagePos().x());
                              printer.setElement(firstSL);
                              paintElement(p, firstSL);
                        }
                  }
            }
            // 2nd pass: the rest of the elements
            QList<const Element*> pel = page->elements();
            qStableSort(pel.begin(), pel.end(), elementLessThan);
            Element::Type eType;
            for (const Element* e : pel) {
                  // Always exclude invisible elements
                  if (!e->visible())
                        continue;

                  eType = e->type();
                  switch (eType) { // In future sub-type code, this switch() grows, and eType gets used
                  case Element::Type::STAFF_LINES : // Handled in the 1st pass above
                        continue; // Exclude from 2nd pass
                        break;
                  default:
                        break;
                  } // switch(eType)

                  // Set the Element pointer inside SvgGenerator/SvgPaintEngine
                  printer.setElement(e);

                  // Paint it
                  paintElement(p, e);
            }
            p.end(); // Writes MuseScore SVG file to disk, finally
      }

      // Clean up and return
      score->setPrinting(false);
      MScore::pdfPrinting = false;
      MScore::svgPrinting = false;
      return true;
      }

//---------------------------------------------------------
//   MuseScore::saveSvg
//---------------------------------------------------------
bool MuseScore::saveSvg(Score* score, QIODevice* device, int pageNumber)
      {
      QString title(score->title());
      score->setPrinting(true);
      MScore::pdfPrinting = true;
      MScore::svgPrinting = true;
      const auto& pages = score->pages();
      Page* page = pages.at(pageNumber);
      SvgGenerator printer;
      printer.setTitle(pages.size() > 1 ? QString("%1 (%2)").arg(title).arg(pageNumber + 1) : title);
      printer.setOutputDevice(device);
      QRectF r;
      if (trimMargin >= 0) {
            QMarginsF margins(trimMargin, trimMargin, trimMargin, trimMargin);
            r = page->tbbox() + margins;
            }
      else
            r = page->abbox();
      qreal w = r.width();
      qreal h = r.height();

      printer.setSize(QSize(w, h));
      printer.setViewBox(QRectF(0, 0, w, h));
      QPainter p(&printer);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::TextAntialiasing, true);
      if (trimMargin >= 0)
            p.translate(-r.topLeft());
      // 1st pass: StaffLines
      for  (System* s : *page->systems()) {
            for (int i = 0, n = s->staves()->size(); i < n; i++) {
                  if (score->staff(i)->invisible() || !score->staff(i)->show())
                        continue;  // ignore invisible staves
                  if (s->staves()->isEmpty() || !s->staff(i)->show())
                        continue;
                  if (!s->firstMeasure())
                        continue;

                  // The goal here is to draw SVG staff lines more efficiently.
                  // MuseScore draws staff lines by measure, but for SVG they can
                  // generally be drawn once for each system. This makes a big
                  // difference for scores that scroll horizontally on a single
                  // page. But there are exceptions to this rule:
                  //
                  //   ~ One (or more) invisible measure(s) in a system/staff ~
                  //   ~ One (or more) elements of type HBOX or VBOX          ~
                  //
                  // In these cases the SVG staff lines for the system/staff
                  // are drawn by measure.
                  //
                  bool byMeasure = false;
                  MeasureBase* mb = nullptr;
                  for (mb = s->firstMeasure(); mb != 0; mb = s->nextMeasure(mb)) {
                        if (mb->type() == Element::Type::HBOX
                         || mb->type() == Element::Type::VBOX
                         || (!static_cast<Measure*>(mb)->visible(i) && mb->system() == s)) {
                              byMeasure = true;
                              break;
                              }
                        }
                  if (mb && mb->type() == Element::Type::VBOX) // no need for staff lines
                        byMeasure = false;
                  if (!byMeasure && (!s->lastMeasure() || !s->lastMeasure()->system()))
                        byMeasure = true;
                  if (byMeasure) { // Draw visible staff lines by measure
                        for (MeasureBase* mb = s->firstMeasure(); mb != 0; mb = s->nextMeasure(mb)) {
                              if (mb->type() != Element::Type::HBOX
                               && mb->type() != Element::Type::VBOX
                               && static_cast<Measure*>(mb)->visible(i)) {
                                    Measure* m = static_cast<Measure*>(mb);
                                    if (score->styleB(StyleIdx::createMultiMeasureRests) && m->hasMMRest())
                                          m = m->mmRest();
                                    StaffLines* sl = m->staffLines(i);
                                    if (sl->measure()->system()) {
                                          printer.setElement(sl);
                                          paintElement(p, sl);
                                          }
                                    }
                              }
                        }
                  else { // Draw staff lines once per system
                        StaffLines* firstSL = s->firstMeasure()->staffLines(i)->clone();
                        StaffLines*  lastSL =  s->lastMeasure()->staffLines(i);
                        firstSL->bbox().setRight(lastSL->bbox().right()
                                              + lastSL->pagePos().x()
                                              - firstSL->pagePos().x());
                        printer.setElement(firstSL);
                        paintElement(p, firstSL);
                  }
            }
      }
      // 2nd pass: the rest of the elements
      QList<const Element*> pel = page->elements();
      qStableSort(pel.begin(), pel.end(), elementLessThan);
      Element::Type eType;
      for (const Element* e : pel) {
            // Always exclude invisible elements
            if (!e->visible())
                  continue;

            eType = e->type();
            switch (eType) { // In future sub-type code, this switch() grows, and eType gets used
            case Element::Type::STAFF_LINES : // Handled in the 1st pass above
                  continue; // Exclude from 2nd pass
                  break;
            default:
                  break;
            } // switch(eType)

            // Set the Element pointer inside SvgGenerator/SvgPaintEngine
            printer.setElement(e);

            // Paint it
            paintElement(p, e);
      }
      p.end(); // Writes MuseScore SVG file to disk, finally


      // Clean up and return
      score->setPrinting(false);
      MScore::pdfPrinting = false;
      MScore::svgPrinting = false;
      return true;
      }

//---------------------------------------------------------
//   createThumbnail
//---------------------------------------------------------

static QPixmap createThumbnail(const QString& name)
      {
      Score* score = new Score;
      Score::FileError error = readScore(score, name, true);
      if (error != Score::FileError::FILE_NO_ERROR) {
            delete score;
            return QPixmap();
            }
      score->doLayout();
      QImage pm = score->createThumbnail();
      delete score;
      return QPixmap::fromImage(pm);
      }

//---------------------------------------------------------
//   extractThumbnail
//---------------------------------------------------------

QPixmap MuseScore::extractThumbnail(const QString& name)
      {
      QPixmap pm; //  = icons[File_ICON].pixmap(QSize(100,140));
      if (!name.endsWith(".mscz"))
            return createThumbnail(name);
      MQZipReader uz(name);
      if (!uz.exists()) {
            qDebug("extractThumbnail: <%s> not found", qPrintable(name));
            return pm;
            }
      QByteArray ba = uz.fileData("Thumbnails/thumbnail.png");
      if (ba.isEmpty())
            return createThumbnail(name);
      pm.loadFromData(ba, "PNG");
      return pm;
      }

bool MuseScore::saveMetadataJSON(Score* score, const QString& name)
      {
      QFile f(name);
      if (!f.open(QIODevice::WriteOnly))
            return false;

      QJsonObject json = saveMetadataJSON(score);
      QJsonDocument saveDoc(json);
      f.write(saveDoc.toJson());
      f.close();
      return true;
      }

QJsonObject MuseScore::saveMetadataJSON(Score* score)
      {
      auto boolToString = [](bool b) { return b ? "true" : "false"; };
      QJsonObject json;

      // title
      QString title;
      json.insert("title", score->title());

      // subtitle
      json.insert("subtitle", score->subtitle());

      // composer
      json.insert("composer", score->composer());

      // poet
      json.insert("poet", score->poet());

      json.insert("mscoreVersion", score->mscoreVersion());
      json.insert("fileVersion", score->mscRealVersion());

      json.insert("pages", score->npages());
      json.insert("measures", score->nmeasures());
      json.insert("hasLyrics", boolToString(score->hasLyrics()));
      json.insert("hasHarmonies", boolToString(score->hasHarmonies()));
      json.insert("keysig", score->keysig());

      // timeSig
      QString timeSig;
      int staves = score->nstaves();
      int tracks = staves * VOICES;
      Segment* tss = score->firstSegmentMM(Segment::Type::TimeSig);
      if (tss) {
            Element* e = nullptr;
            for (int track = 0; track < tracks; ++track) {
                  e = tss->element(track);
                  if (e) break;
                  }
            if (e && e->type() == Element::Type::TIMESIG) {
                  TimeSig* ts = static_cast<TimeSig*>(e);
                  timeSig = QString("%1/%2").arg(ts->numerator()).arg(ts->denominator());
                  }
            }
      json.insert("timesig", timeSig);

      json.insert("duration", score->duration());
      json.insert("lyrics", score->extractLyrics());

      // tempo
       int tempo = 0;
       QString tempoText;
       for (Segment* seg = score->firstSegmentMM(); seg; seg = seg->next1MM()) {
             auto annotations = seg->annotations();
             for (Element* a : annotations) {
                   if (a && a->type() == Element::Type::TEMPO_TEXT) {
                         TempoText* tt = static_cast<TempoText*>(a);
                         tempo = round(tt->tempo() * 60);
                         tempoText = tt->xmlText();
                         }
                   }
             }
      json.insert("tempo", tempo);
      json.insert("tempoText", tempoText);

      // parts
      QJsonArray jsonPartsArray;
      for (Part* p : score->parts()) {
            QJsonObject jsonPart;
            jsonPart.insert("name", p->longName().replace("\n", ""));
            int midiProgram = p->midiProgram();
            if (p->midiChannel() == 9)
                midiProgram = 128;
            jsonPart.insert("program", midiProgram);
            jsonPart.insert("instrumentId", p->instrumentId());
            jsonPart.insert("lyricCount", p->lyricCount());
            jsonPart.insert("harmonyCount", p->harmonyCount());
            jsonPart.insert("hasPitchedStaff", boolToString(p->hasPitchedStaff()));
            jsonPart.insert("hasTabStaff", boolToString(p->hasTabStaff()));
            jsonPart.insert("hasDrumStaff", boolToString(p->hasDrumStaff()));
            jsonPart.insert("isVisible", boolToString(p->show()));
            jsonPartsArray.append(jsonPart);
            }
      json.insert("parts", jsonPartsArray);

      // pageFormat
      QJsonObject jsonPageformat;
      jsonPageformat.insert("height",round(score->pageFormat()->height() * INCH));
      jsonPageformat.insert("width", round(score->pageFormat()->width() * INCH));
      jsonPageformat.insert("twosided", boolToString(score->pageFormat()->twosided()));
      json.insert("pageFormat", jsonPageformat);

      return json;
      }

}

