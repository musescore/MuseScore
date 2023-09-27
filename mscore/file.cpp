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

#include <QFileInfo>
#include <QMessageBox>

#include "cloud/loginmanager.h"

#include "config.h"
#include "globals.h"
#include "musescore.h"
#include "scoreview.h"

#include "audio/exports/exportmidi.h"

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
#include "zoombox.h"
#include "libmscore/measure.h"
#include "libmscore/undo.h"
#include "libmscore/repeatlist.h"
#include "scoretab.h"
#include "libmscore/beam.h"
#include "libmscore/stafftype.h"
#include "libmscore/stafftypechange.h"
#include "seq.h"
#include "libmscore/revisions.h"
#include "libmscore/lyrics.h"
#include "libmscore/segment.h"
#include "libmscore/tempotext.h"
#include "libmscore/sym.h"
#include "libmscore/image.h"
#include "libmscore/stafflines.h"
#include "audio/midi/msynthesizer.h"
#include "svggenerator.h"
#include "scorePreview.h"
#include "scorecmp/scorecmp.h"
#include "extension.h"
#include "tourhandler.h"
#include "preferences.h"
#include "libmscore/instrtemplate.h"

#ifdef OMR
#include "omr/omr.h"
#include "omr/omrpage.h"
#include "omr/importpdf.h"
#endif

#include "libmscore/chordlist.h"
#include "libmscore/mscore.h"
#include "thirdparty/qzip/qzipreader_p.h"
#include "migration/scoremigrator_3_6.h"
#include "migration/handlers/styledefaultshandler.h"
#include "migration/handlers/lelandstylehandler.h"
#include "migration/handlers/edwinstylehandler.h"
#include "migration/handlers/resetallelementspositionshandler.h"


namespace Ms {

extern void importSoundfont(QString name);

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

static void paintElements(QPainter& p, const QList<Element*>& el)
      {
      for (Element* e : el) {
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

QString MuseScore::saveFilename(QString fn)
      {
      return createDefaultFileName(fn);
      }

//---------------------------------------------------------
//   readScoreError
//    if "ask" is true, ask to ignore; returns true if
//    ignore is pressed by user
//    returns true if -f is used in converter mode
//---------------------------------------------------------

static bool readScoreError(const QString& name, Score::FileError error, bool ask)
      {
//printf("<%s> %d\n", qPrintable(name), int(error));
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
                  msg += QObject::tr("It was last saved with a version older than 2.0.0.\n"
                                     "You can convert this score by opening and then\n"
                                     "saving with MuseScore version 2.x.\n"
                                     "Visit the %1MuseScore download page%2 to obtain such a 2.x version.")
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
            case Score::FileError::FILE_CRITICALLY_CORRUPTED:
                  msg = QObject::tr("File \"%1\" is critically corrupted and cannot be processed.").arg(name);
                  detailedMsg = MScore::lastError;
                  break;
            case Score::FileError::FILE_OLD_300_FORMAT:
                  msg += QObject::tr("It was last saved with a developer version of 3.0.\n");
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

bool MuseScore::checkDirty(MasterScore* s)
      {
      if (s->dirty() || (s->created() && !s->startedEmpty())) {
            QMessageBox::StandardButton n = QMessageBox::warning(this, tr("MuseScore"),
               tr("Save changes to the score \"%1\"\n"
               "before closing?").arg(s->fileInfo()->completeBaseName()),
               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
               QMessageBox::Save);
            if (n == QMessageBox::Save) {
                  if (s->masterScore()->isSavable()) {
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
//   openFiles
//---------------------------------------------------------

void MuseScore::openFiles(bool switchTab, bool singleFile)
      {
      QString allExt = "*.mscz *.mscx *.mxl *.musicxml *.xml *.mid *.midi *.kar *.md *.mgu *.sgu *.cap *.capx *.ove *.scw *.bmw *.bww *.gtp *.gp3 *.gp4 *.gp5 *.gpx *.gp *.ptb *.mscz, *.mscx,";
#ifdef AVSOMR
      allExt += " *.msmr"; // omr project with omr data and musicxml or score
#endif

      QStringList filter;
      filter << tr("All Supported Files") + " (" + allExt + ")"
             << tr("MuseScore Files") + " (*.mscz *.mscx)"
             << tr("MusicXML Files") + " (*.mxl *.musicxml *.xml)"
             << tr("MIDI Files") + " (*.mid *.midi *.kar)"
             << tr("MuseData Files") + " (*.md)"
             << tr("Capella Files") + " (*.cap *.capx)"
             << tr("BB Files (experimental)") + " (*.mgu *.sgu)"
             << tr("Overture / Score Writer Files (experimental)") + " (*.ove *.scw)"
             << tr("Bagpipe Music Writer Files (experimental)") + " (*.bmw *.bww)"
             << tr("Guitar Pro Files") + " (*.gtp *.gp3 *.gp4 *.gp5 *.gpx *.gp)"
             << tr("Power Tab Editor Files (experimental)") + " (*.ptb)"
             << tr("MuseScore Backup Files") + " (*.mscz, *.mscx,)";

      doLoadFiles(filter, switchTab, singleFile);
      }

//---------------------------------------------------------
//   importFiles
//---------------------------------------------------------

void MuseScore::importScore(bool switchTab, bool singleFile)
      {
#ifndef AVSOMR
      Q_UNUSED(switchTab);
      Q_UNUSED(singleFile);
      openExternalLink("https://musescore.com/import");
#else
      QStringList filter;
      filter << tr("Optical Music Recognition") + " (*.pdf *.png *.jpg)";
      doLoadFiles(filter, switchTab, singleFile);
#endif
      }

//---------------------------------------------------------
//   doLoadFiles
//---------------------------------------------------------

/**
 Create a modal file open dialog.
 If a file is selected, load it.
 Handles the GUI's file-open action.
 */

void MuseScore::doLoadFiles(const QStringList& filter, bool switchTab, bool singleFile)
      {
      QString filterStr = filter.join(";;");
      QStringList files = getOpenScoreNames(filterStr, tr("Load Score"), singleFile);
      for (const QString& s : files)
            openScore(s, switchTab);
      mscore->tourHandler()->showDelayedWelcomeTour();
      }

void MuseScore::askAboutApplyingEdwinIfNeed(const QString& fileSuffix)
{
    if (MScore::noGui) {
        return;
    }

    static const QSet<QString> suitedSuffixes {
        "xml",
        "musicxml",
        "mxl"
    };

    if (!suitedSuffixes.contains(fileSuffix)) {
        return;
    }

    if (preferences.getBool(PREF_MIGRATION_DO_NOT_ASK_ME_AGAIN_XML)) {
        return;
    }

    QMessageBox dialog;
    dialog.setWindowTitle(QObject::tr("MuseScore"));
    dialog.setText(QObject::tr("Would you like to apply our default typeface (%1) to this score?").arg("Edwin"));
    QPushButton* noButton = dialog.addButton(QObject::tr("No"), QMessageBox::NoRole); // No
    QPushButton* yesButton = dialog.addButton(QObject::tr("Yes"), QMessageBox::YesRole);
    dialog.setDefaultButton(noButton);

    QCheckBox askAgainCheckbox(QObject::tr("Remember my choice and don't ask again"));
    dialog.setCheckBox(&askAgainCheckbox);

    QObject::connect(&askAgainCheckbox, &QCheckBox::stateChanged, [](int state) {
        if (static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked) {
            preferences.setPreference(PREF_MIGRATION_DO_NOT_ASK_ME_AGAIN_XML, true);
        }
    });

    dialog.exec();

    bool needApplyEdwin = dialog.clickedButton() == yesButton;
    preferences.setPreference(PREF_MIGRATION_APPLY_EDWIN_FOR_XML_FILES, needApplyEdwin);
}

//---------------------------------------------------------
//   openScore
//---------------------------------------------------------

Score* MuseScore::openScore(const QString& fn, bool switchTab, const bool considerInCurrentSession, const QString& name)
      {
      //
      // make sure we load a file only once
      //
      QFileInfo fi(fn);
      QString path = fi.canonicalFilePath();
      for (Score* s : scoreList) {
            if (s->masterScore() && s->masterScore()->fileInfo()->canonicalFilePath() == path) {
                  if (switchTab && !isModalDialogOpen())
                        setCurrentScoreView(scoreList.indexOf(s->masterScore()));
                  return 0;
                  }
            }

      askAboutApplyingEdwinIfNeed(fi.suffix().toLower());

      MasterScore* score = readScore(fn);
      if (score) {
            if (!name.isEmpty())
                  score->masterScore()->fileInfo()->setFile(name);
                  // TODO: what if that path is no longer valid?
            
            ScoreMigrator_3_6 migrator;

            migrator.registerHandler(new StyleDefaultsHandler());

            if (preferences.getBool(PREF_MIGRATION_DO_NOT_ASK_ME_AGAIN) && score->mscVersion() < MSCVERSION) {
                  if (preferences.getBool(PREF_MIGRATION_APPLY_LELAND_STYLE))
                        migrator.registerHandler(new LelandStyleHandler());

                  if (preferences.getBool(PREF_MIGRATION_APPLY_EDWIN_STYLE))
                        migrator.registerHandler(new EdwinStyleHandler());

                  if (preferences.getBool(PREF_MIGRATION_RESET_ELEMENT_POSITIONS))
                        migrator.registerHandler(new ResetAllElementsPositionsHandler());
                  }

            migrator.migrateScore(score);

            score->updateCapo();
            score->update();
            score->styleChanged();
            score->doLayout();

            if (considerInCurrentSession) {
                  const int tabIdx = appendScore(score);
                  if (switchTab && !isModalDialogOpen())
                        setCurrentScoreView(tabIdx);
                  writeSessionFile(false);
                  }
            }
      return score;
      }

//---------------------------------------------------------
//   readScore
//---------------------------------------------------------

MasterScore* MuseScore::readScore(const QString& name)
      {
      if (name.isEmpty())
            return 0;

      if (instrumentGroups.isEmpty()) {
            QString tmplPath = preferences.getString(PREF_APP_PATHS_INSTRUMENTLIST1);

            if (tmplPath.isEmpty())
                  tmplPath = preferences.getString(PREF_APP_PATHS_INSTRUMENTLIST2);

            loadInstrumentTemplates(tmplPath);
            }

      MasterScore* score = new MasterScore(MScore::baseStyle());
      setMidiReopenInProgress(name);
      Score::FileError rv = Ms::readScore(score, name, false);
      if (rv == Score::FileError::FILE_TOO_OLD || rv == Score::FileError::FILE_TOO_NEW || rv == Score::FileError::FILE_CORRUPTED) {
            if (readScoreError(name, rv, true)) {
                  if (rv != Score::FileError::FILE_CORRUPTED) {
                        // don’t read file again if corrupted
                        // the check routine may try to fix it
                        delete score;
                        score = new MasterScore();
                        score->setMovements(new Movements());
                        score->setStyle(MScore::baseStyle());
                        rv = Ms::readScore(score, name, true);
                        }
                  else
                        rv = Score::FileError::FILE_NO_ERROR;
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
      return saveFile(cs->masterScore());
      }

//---------------------------------------------------------
//   saveFile
///   Save the score.
//
//    return true on success
//---------------------------------------------------------

bool MuseScore::saveFile(MasterScore* score)
      {
      if (score == 0)
            return false;
      if (score->created()) {
            QString fileBaseName = score->masterScore()->fileInfo()->completeBaseName();
            QString fileName = score->masterScore()->fileInfo()->fileName();
            // is backup file
            if (fileBaseName.startsWith(".")
               && (fileName.endsWith(".mscz,") || fileName.endsWith(".mscx,")))
                  fileBaseName.remove(0, 1); // remove the "." at the beginning of file name
            Text* t = score->getText(Tid::TITLE);
            if (t)
                  fileBaseName = t->plainText();
            QString name = createDefaultFileName(fileBaseName);
            QString msczType = tr("MuseScore 3 File") + " (*.mscz)";
            QString mscxType = tr("Uncompressed MuseScore 3 File") + " (*.mscx)";     // for debugging purposes

            QSettings set;
            if (mscore->lastSaveDirectory.isEmpty())
                  mscore->lastSaveDirectory = set.value("lastSaveDirectory", preferences.getString(PREF_APP_PATHS_MYSCORES)).toString();
            QString saveDirectory = mscore->lastSaveDirectory;

            if (saveDirectory.isEmpty())
                  saveDirectory = preferences.getString(PREF_APP_PATHS_MYSCORES);

            QString fname = QString("%1/%2").arg(saveDirectory).arg(name);
            QString filter;
#ifdef AVSOMR
            if (score->avsOmr()) {
                  QString msmrType = tr("Music Recognition MuseScore 3 File") + " (*.msmr)";
                  fname = QFileInfo(fname).baseName() + ".msmr";
                  filter = msmrType + ";;" + mscxType + ";;" + msczType;
                  }
            else {
#endif
            filter = msczType + ";;" + mscxType;
            if (QFileInfo(fname).suffix().isEmpty())
                  fname += ".mscz";
#ifdef AVSOMR
            }
#endif

            fileBaseName = mscore->getSaveScoreName(tr("Save Score"), fname, filter);
            if (fileBaseName.isEmpty())
                  return false;
            score->masterScore()->fileInfo()->setFile(fileBaseName);

            mscore->lastSaveDirectory = score->masterScore()->fileInfo()->absolutePath();

            if (!score->masterScore()->saveFile(preferences.getBool(PREF_APP_BACKUP_GENERATE_BACKUP))) {
                  QMessageBox::critical(mscore, tr("Save File"), MScore::lastError);
                  return false;
                  }
            addRecentScore(score);
            writeSessionFile(false);
            }
      else if (!score->masterScore()->saveFile(preferences.getBool(PREF_APP_BACKUP_GENERATE_BACKUP))) {
            QMessageBox::critical(mscore, tr("Save File"), MScore::lastError);
            return false;
            }
      score->setCreated(false);
      updateWindowTitle(score);
      scoreCmpTool->updateScoreVersions(score);
      int idx = scoreList.indexOf(score->masterScore());
      tab1->setTabText(idx, score->fileInfo()->completeBaseName());
      if (tab2)
            tab2->setTabText(idx, score->fileInfo()->completeBaseName());
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
            for (MasterScore* s : scoreList) {
                  if (s->fileInfo()->completeBaseName() == tmpName) {
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

//---------------------------------------------------------
//   getNewFile
//    create new score
//---------------------------------------------------------

MasterScore* MuseScore::getNewFile()
      {
      if (!newWizard)
            newWizard = new NewWizard(this);
      else {
            newWizard->updateValues();
            newWizard->restart();
            }
      if (newWizard->exec() != QDialog::Accepted)
            return 0;
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

      MasterScore* score = new MasterScore(MScore::defaultStyle());
      QString tp         = newWizard->templatePath();

      QList<Excerpt*> excerpts;
      if (!newWizard->emptyScore()) {
            MasterScore* tscore = new MasterScore(MScore::defaultStyle());
            tscore->setCreated(true);
            Score::FileError rv = Ms::readScore(tscore, tp, false);
            if (rv != Score::FileError::FILE_NO_ERROR) {
                  readScoreError(newWizard->templatePath(), rv, false);
                  delete tscore;
                  delete score;
                  return 0;
                  }
            if (MScore::harmonyPlayDisableNew) {
                  tscore->style().set(Sid::harmonyPlay, false);
                  }
            else if (MScore::harmonyPlayDisableCompatibility) {
                  // if template was older, then harmonyPlay may have been forced off by the compatibility preference
                  // that's not appropriate when creating new scores from old templates
                  // if template was pre-3.5, return harmonyPlay to default
                  QString programVersion = tscore->mscoreVersion();
                  if (!programVersion.isEmpty() && programVersion < "3.5")
                        tscore->style().set(Sid::harmonyPlay, MScore::defaultStyle().value(Sid::harmonyPlay));
                  }
            score->setStyle(tscore->style());
            score->setScoreOrder(tscore->scoreOrder());

            // create instruments from template
            for (Part* tpart : tscore->parts()) {
                  Part* part = new Part(score);
                  part->setInstrument(tpart->instrument());
                  part->setPartName(tpart->partName());

                  for (Staff* tstaff : *tpart->staves()) {
                        Staff* staff = new Staff(score);
                        staff->setPart(part);
                        staff->init(tstaff);
                        if (tstaff->links() && !part->staves()->isEmpty()) {
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
            if (mb && mb->isVBox()) {
                  VBox* tvb = toVBox(mb);
                  nvb = new VBox(score);
                  nvb->setBoxHeight(tvb->boxHeight());
                  nvb->setBoxWidth(tvb->boxWidth());
                  nvb->setTopGap(tvb->topGap());
                  nvb->setBottomGap(tvb->bottomGap());
                  nvb->setTopMargin(tvb->topMargin());
                  nvb->setBottomMargin(tvb->bottomMargin());
                  nvb->setLeftMargin(tvb->leftMargin());
                  nvb->setRightMargin(tvb->rightMargin());
                  nvb->setAutoSizeEnabled(tvb->isAutoSizeEnabled());
                  }
            delete tscore;
            }
      else {
            score = new MasterScore(MScore::defaultStyle());
            if (MScore::harmonyPlayDisableNew) {
                  score->style().set(Sid::harmonyPlay, false);
                  }
            newWizard->createInstruments(score);
            }
      score->setCreated(true);
      score->fileInfo()->setFile(createDefaultName());

      score->style().checkChordList();
      if (!newWizard->title().isEmpty())
            score->fileInfo()->setFile(newWizard->title());

      score->sigmap()->add(0, timesig);

      Fraction firstMeasureTicks = pickupMeasure ? Fraction(pickupTimesigZ, pickupTimesigN) : timesig;

      for (int i = 0; i < measures; ++i) {
            Fraction tick = firstMeasureTicks + timesig * (i - 1);
            if (i == 0)
                  tick = Fraction(0,1);
            QList<Rest*> puRests;
            for (Score* _score : score->scoreList()) {
                  Rest* rest = 0;
                  Measure* measure = new Measure(_score);
                  measure->setTimesig(timesig);
                  measure->setTicks(timesig);
                  measure->setTick(tick);

                  if (pickupMeasure && tick.isZero()) {
                        measure->setIrregular(true);        // don’t count pickup measure
                        measure->setTicks(Fraction(pickupTimesigZ, pickupTimesigN));
                        }
                  _score->measures()->add(measure);

                  for (Staff* staff : _score->staves()) {
                        int staffIdx = staff->idx();
                        if (tick.isZero()) {
                              TimeSig* ts = new TimeSig(_score);
                              ts->setTrack(staffIdx * VOICES);
                              ts->setSig(timesig, timesigType);
                              Measure* m = _score->firstMeasure();
                              Segment* s = m->getSegment(SegmentType::TimeSig, Fraction(0,1));
                              s->add(ts);
                              Part* part = staff->part();
                              if (!part->instrument()->useDrumset()) {  //tick?
                                    //
                                    // transpose key
                                    //
                                    KeySigEvent nKey = ks;
                                    if (!nKey.custom() && !nKey.isAtonal() && part->instrument()->transpose().chromatic && !score->styleB(Sid::concertPitch)) {
                                          int diff = -part->instrument()->transpose().chromatic;
                                          nKey.setKey(transposeKey(nKey.key(), diff, part->preferSharpFlat()));
                                          }
                                    // do not create empty keysig unless custom or atonal
                                    if (nKey.custom() || nKey.isAtonal() || nKey.key() != Key::C) {
                                          staff->setKey(Fraction(0,1), nKey);
                                          KeySig* keysig = new KeySig(score);
                                          keysig->setTrack(staffIdx * VOICES);
                                          keysig->setKeySigEvent(nKey);
                                          Segment* ss = measure->getSegment(SegmentType::KeySig, Fraction(0,1));
                                          ss->add(keysig);
                                          }
                                    }
                              }

                        // determined if this staff is linked to previous so we can reuse rests
                        bool linkedToPrevious = staffIdx && staff->isLinked(_score->staff(staffIdx - 1));
                        if (measure->timesig() != measure->ticks()) {
                              if (!linkedToPrevious)
                                    puRests.clear();
                              std::vector<TDuration> dList = toDurationList(measure->ticks(), false);
                              if (!dList.empty()) {
                                    Fraction ltick = tick;
                                    int k = 0;
                                    foreach (TDuration d, dList) {
                                          if (k < puRests.count())
                                                rest = static_cast<Rest*>(puRests[k]->linkedClone());
                                          else {
                                                rest = new Rest(score, d);
                                                puRests.append(rest);
                                                }
                                          rest->setScore(_score);
                                          rest->setTicks(d.fraction());
                                          rest->setTrack(staffIdx * VOICES);
                                          Segment* seg = measure->getSegment(SegmentType::ChordRest, ltick);
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
                              rest->setTicks(measure->ticks());
                              rest->setTrack(staffIdx * VOICES);
                              Segment* seg = measure->getSegment(SegmentType::ChordRest, tick);
                              seg->add(rest);
                              }
                        }
                  }
            }
//TODO      score->lastMeasure()->setEndBarLineType(BarLineType::END, false);

      //
      // select first rest
      //
      Measure* m = score->firstMeasure();
      for (Segment* s = m->first(); s; s = s->next()) {
            if (s->segmentType() == SegmentType::ChordRest) {
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
            if (measure->type() != ElementType::VBOX) {
                  MeasureBase* nm = nvb ? nvb : new VBox(score);
                  nm->setTick(Fraction(0,1));
                  nm->setNext(measure);
                  score->measures()->add(nm);
                  measure = nm;
                  }
            else if (nvb) {
                  delete nvb;
                  }
            if (!title.isEmpty()) {
                  Text* s = new Text(score, Tid::TITLE);
                  s->setPlainText(title);
                  measure->add(s);
                  score->setMetaTag("workTitle", title);
                  }
            if (!subtitle.isEmpty()) {
                  Text* s = new Text(score, Tid::SUBTITLE);
                  s->setPlainText(subtitle);
                  measure->add(s);
                  }
            if (!composer.isEmpty()) {
                  Text* s = new Text(score, Tid::COMPOSER);
                  s->setPlainText(composer);
                  measure->add(s);
                  score->setMetaTag("composer", composer);
                  }
            if (!poet.isEmpty()) {
                  Text* s = new Text(score, Tid::POET);
                  s->setPlainText(poet);
                  measure->add(s);
                  // the poet() functions returns data called lyricist in the dialog
                  score->setMetaTag("lyricist", poet);
                  }
            }
      else if (nvb) {
            delete nvb;
            }

      double tempo = Score::defaultTempo() * 60; // quarter notes per minute
      if (newWizard->tempo(&tempo)) {

            Fraction ts = newWizard->timesig();

            QString text("<sym>metNoteQuarterUp</sym> = %1");
            double bpm = tempo;
            switch (ts.denominator()) {
                  case 1:
                        text = "<sym>metNoteWhole</sym> = %1";
                        bpm /= 4;
                        break;
                  case 2:
                        text = "<sym>metNoteHalfUp</sym> = %1";
                        bpm /= 2;
                        break;
                  case 4:
                        text = "<sym>metNoteQuarterUp</sym> = %1";
                        break;
                  case 8:
                        if (ts.numerator() % 3 == 0) {
                              text = "<sym>metNoteQuarterUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = %1";
                              bpm /= 1.5;
                              }
                        else {
                              text = "<sym>metNote8thUp</sym> = %1";
                              bpm *= 2;
                              }
                        break;
                  case 16:
                        if (ts.numerator() % 3 == 0) {
                              text = "<sym>metNote8thUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = %1";
                              bpm *= 1.5;
                              }
                        else {
                              text = "<sym>metNote16thUp</sym> = %1";
                              bpm *= 4;
                              }
                        break;
                  case 32:
                        if (ts.numerator() % 3 == 0) {
                              text = "<sym>metNote16thUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = %1";
                              bpm *= 3;
                              }
                        else {
                              text = "<sym>metNote32ndUp</sym> = %1";
                              bpm *= 8;
                              }
                        break;
                  case 64:
                        if (ts.numerator() % 3 == 0) {
                              text = "<sym>metNote32ndUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = %1";
                              bpm *= 6;
                              }
                        else {
                              text = "<sym>metNote64thUp</sym> = %1";
                              bpm *= 16;
                              }
                        break;
                  case 128:
                        if (ts.numerator() % 3 == 0) {
                              text = "<sym>metNote64ndUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = %1";
                              bpm *= 9;
                              }
                        else {
                              text = "<sym>metNote128thUp</sym> = %1";
                              bpm *= 32;
                              }
                        break;
                  default:
                        break;
                  }

            TempoText* tt = new TempoText(score);
            tt->setXmlText(text.arg(bpm));
            tempo /= 60;      // bpm -> bps

            tt->setTempo(tempo);
            tt->setFollowText(true);
            tt->setTrack(0);
            Segment* seg = score->firstMeasure()->first(SegmentType::ChordRest);
            seg->add(tt);
            score->setTempo(seg, tempo);
            }
      if (!copyright.isEmpty())
            score->setMetaTag("copyright", copyright);

      if (synti)
            score->setSynthesizerState(synti->state());

      // Call this even if synti doesn't exist - we need to rebuild either way
      score->rebuildAndUpdateExpressive(MuseScore::synthesizer("Fluid"));

      {
            ScoreLoad sl;
            score->doLayout();
            }

      for (Excerpt* x : excerpts) {
            Score* xs = new Score(static_cast<MasterScore*>(score));
            xs->style().set(Sid::createMultiMeasureRests, true);
            x->setPartScore(xs);
            xs->setExcerpt(x);
            score->excerpts().append(x);
            Excerpt::createExcerpt(x);
            }
      score->setExcerptsChanged(true);
      return score;
      }

//---------------------------------------------------------
//   newFile
//    create new score
//---------------------------------------------------------

void MuseScore::newFile()
      {
      MasterScore* score = getNewFile();
      if (score)
            setCurrentScoreView(appendScore(score));
      mscore->tourHandler()->showDelayedWelcomeTour();
      }

//---------------------------------------------------------
//   copy
//    Copy content of src file do dest file overwriting it.
//    Implemented manually as QFile::copy refuses to
//    overwrite existing files.
//---------------------------------------------------------

static bool copy(QFile& src, QFile& dest)
      {
      src.open(QIODevice::ReadOnly);
      dest.open(QIODevice::WriteOnly);
      constexpr qint64 size = 1024 * 1024;
      char* buf = new char[size];
      bool err = false;
      while (qint64 r = src.read(buf, size)) {
            if (r < 0) {
                  err = true;
                  break;
                  }
            qint64 w = dest.write(buf, r);
            if (w < r) {
                  err = true;
                  break;
                  }
            }
      dest.close();
      src.close();
      delete[] buf;
      return !err;
      }

//---------------------------------------------------------
//   getTemporaryScoreFileCopy
//    baseNameTemplate is the template to be passed to
//    QTemporaryFile constructor but without suffix and
//    directory --- they are defined automatically.
//---------------------------------------------------------

QTemporaryFile* MuseScore::getTemporaryScoreFileCopy(const QFileInfo& info, const QString& baseNameTemplate)
      {
      QString suffix(info.suffix());
      if (suffix.endsWith(",")) // some backup files created by MuseScore
            suffix.chop(1);
      QTemporaryFile* f = new QTemporaryFile(
         QDir::temp().absoluteFilePath(baseNameTemplate + '.' + suffix),
         this
         );
      QFile src(info.absoluteFilePath());
      if (!copy(src, *f)) {
            delete f;
            return nullptr;
            }
      return f;
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
      QFileInfo myScores(preferences.getString(PREF_APP_PATHS_MYSCORES));
      urls.append(QUrl::fromLocalFile(myScores.absoluteFilePath()));
      urls.append(QUrl::fromLocalFile(QDir::currentPath()));
      return urls;
      }

//---------------------------------------------------------
//   getOpenScoreNames
//---------------------------------------------------------

QStringList MuseScore::getOpenScoreNames(const QString& filter, const QString& title, bool singleFile)
      {
      QSettings set;
      QString dir = set.value("lastOpenPath", preferences.getString(PREF_APP_PATHS_MYSCORES)).toString();
      if (preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS)) {
            QStringList fileList = QFileDialog::getOpenFileNames(this,
               title, dir, filter);
            if (fileList.count() > 0) {
                  QFileInfo fi(fileList[0]);
                  set.setValue("lastOpenPath", fi.absolutePath());
                  }
            return fileList;
            }
      QFileInfo myScores(preferences.getString(PREF_APP_PATHS_MYSCORES));
      if (myScores.isRelative())
            myScores.setFile(QDir::home(), preferences.getString(PREF_APP_PATHS_MYSCORES));

      if (loadScoreDialog == 0) {
            loadScoreDialog = new QFileDialog(this);
            loadScoreDialog->setFileMode(singleFile ? QFileDialog::ExistingFile : QFileDialog::ExistingFiles);
            loadScoreDialog->setOption(QFileDialog::DontUseNativeDialog, true);
            loadScoreDialog->setWindowTitle(title);
            addScorePreview(loadScoreDialog);

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
      // setup side bar urls
      QList<QUrl> urls = sidebarUrls();
      urls.append(QUrl::fromLocalFile(mscoreGlobalShare+"/demos"));
      loadScoreDialog->setSidebarUrls(urls);

      QStringList result;
      if (loadScoreDialog->exec())
            result = loadScoreDialog->selectedFiles();
      set.setValue("lastOpenPath", loadScoreDialog->directory().absolutePath());
      return result;
      }

//---------------------------------------------------------
//   getSaveScoreName
//---------------------------------------------------------

QString MuseScore::getSaveScoreName(const QString& title, QString& name, const QString& filter, bool selectFolder, bool askOverwrite)
      {
      QFileInfo myName(name);
      if (myName.isRelative())
            myName.setFile(QDir::home(), name);
      name = myName.absoluteFilePath();

      if (preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS)) {
            QString s;
            QFileDialog::Options options;
            if (!askOverwrite) {
                  options |= QFileDialog::DontConfirmOverwrite;
            }
            if (selectFolder)
                  options |= QFileDialog::ShowDirsOnly;
            return QFileDialog::getSaveFileName(this, title, name, filter, &s, options);
            }

      QFileInfo myScores(preferences.getString(PREF_APP_PATHS_MYSCORES));
      if (myScores.isRelative())
            myScores.setFile(QDir::home(), preferences.getString(PREF_APP_PATHS_MYSCORES));
      if (saveScoreDialog == 0) {
            saveScoreDialog = new QFileDialog(this);
            saveScoreDialog->setFileMode(QFileDialog::AnyFile);
            saveScoreDialog->setOption(QFileDialog::DontUseNativeDialog, true);
            saveScoreDialog->setAcceptMode(QFileDialog::AcceptSave);
            addScorePreview(saveScoreDialog);

            restoreDialogState("saveScoreDialog", saveScoreDialog);
            }
      // setup side bar urls
      saveScoreDialog->setSidebarUrls(sidebarUrls());

      if (selectFolder)
            saveScoreDialog->setFileMode(QFileDialog::Directory);
      saveScoreDialog->setOption(QFileDialog::DontConfirmOverwrite, !askOverwrite);

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
      QFileInfo myStyles(preferences.getString(PREF_APP_PATHS_MYSTYLES));
      if (myStyles.isRelative())
            myStyles.setFile(QDir::home(), preferences.getString(PREF_APP_PATHS_MYSTYLES));
      QString defaultPath = myStyles.absoluteFilePath();

      if (preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS)) {
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

      QFileInfo myStyles(preferences.getString(PREF_APP_PATHS_MYSTYLES));
      if (myStyles.isRelative())
            myStyles.setFile(QDir::home(), preferences.getString(PREF_APP_PATHS_MYSTYLES));
      QString defaultPath = myStyles.absoluteFilePath();

      if (preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS)) {
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

      QSettings set;
      if (open) {
            if (loadChordStyleDialog == 0) {
                  loadChordStyleDialog = new QFileDialog(this);
                  loadChordStyleDialog->setFileMode(QFileDialog::ExistingFile);
                  loadChordStyleDialog->setOption(QFileDialog::DontUseNativeDialog, true);
                  loadChordStyleDialog->setWindowTitle(tr("Load Chord Symbols Style"));
                  loadChordStyleDialog->setNameFilter(filter);
                  loadChordStyleDialog->setDirectory(defaultPath);

                  restoreDialogState("loadChordStyleDialog", loadChordStyleDialog);
                  loadChordStyleDialog->restoreState(set.value("loadChordStyleDialog").toByteArray());
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
      if (preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS)) {
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
      if (preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS)) {
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

      QFileInfo myImages(preferences.getString(PREF_APP_PATHS_MYIMAGES));
      if (myImages.isRelative())
            myImages.setFile(QDir::home(), preferences.getString(PREF_APP_PATHS_MYIMAGES));
      QString defaultPath = myImages.absoluteFilePath();

      // compute the image capture path
      QString myCapturePath;
      // if no saves were made for current score, then use the score's name as default
      if (!cs->masterScore()->savedCapture()) {
            // set the current score's name as the default name for saved captures
            QString scoreName = cs->masterScore()->fileInfo()->completeBaseName();
            QString name = createDefaultFileName(scoreName);
            QString fname = QString("%1/%2").arg(defaultPath).arg(name);
            QFileInfo myCapture(fname);
            if (myCapture.isRelative())
                myCapture.setFile(QDir::home(), fname);
            myCapturePath = myCapture.absoluteFilePath();
            }
      else
            myCapturePath = lastSaveCaptureName;

      if (preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS)) {
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
                cs->masterScore()->setSavedCapture(true);
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
            cs->masterScore()->setSavedCapture(true);
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
#if defined(WIN_PORTABLE)
      QString wd      = QDir::cleanPath(QString("%1/../../../Data/settings").arg(QCoreApplication::applicationDirPath()).arg(QCoreApplication::applicationName()));
#else
      QString wd      = QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).arg(QCoreApplication::applicationName());
#endif
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

      if (preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS)) {
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

      QFileInfo myPlugins(preferences.getString(PREF_APP_PATHS_MYPLUGINS));
      if (myPlugins.isRelative())
            myPlugins.setFile(QDir::home(), preferences.getString(PREF_APP_PATHS_MYPLUGINS));
      QString defaultPath = myPlugins.absoluteFilePath();

      QString name  = createDefaultFileName("Plugin");
      QString fname = QString("%1/%2.qml").arg(defaultPath).arg(name);
      if (preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS)) {
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

                  QSettings set;
                  loadPluginDialog->restoreState(set.value("loadPluginDialog").toByteArray());
                  loadPluginDialog->setAcceptMode(QFileDialog::AcceptOpen);
                  }
            urls.append(QUrl::fromLocalFile(mscoreGlobalShare+"/plugins"));
            dialog = loadPluginDialog;
            }
      else {
            if (savePluginDialog == 0) {
                  savePluginDialog = new QFileDialog(this);
                  QSettings set;
                  savePluginDialog->restoreState(set.value("savePluginDialog").toByteArray());
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

      QFileInfo myStyles(preferences.getString(PREF_APP_PATHS_MYSTYLES));
      if (myStyles.isRelative())
            myStyles.setFile(QDir::home(), preferences.getString(PREF_APP_PATHS_MYSTYLES));
      QString defaultPath  = myStyles.absoluteFilePath();

      if (preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS)) {
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
#ifndef QT_NO_PRINTER
      LayoutMode layoutMode = cs->layoutMode();
      if (layoutMode != LayoutMode::PAGE) {
            cs->setLayoutMode(LayoutMode::PAGE);
            cs->doLayout();
            }

      QPrinter printerDev(QPrinter::HighResolution);
      QSizeF size(cs->styleD(Sid::pageWidth), cs->styleD(Sid::pageHeight));
      QPageSize ps(QPageSize::id(size, QPageSize::Inch));
      printerDev.setPageSize(ps);
      printerDev.setPageOrientation(size.width() > size.height() ? QPageLayout::Landscape : QPageLayout::Portrait);

      printerDev.setCreator("MuseScore Version: " VERSION);
      printerDev.setFullPage(true);
      if (!printerDev.setPageMargins(QMarginsF()))
            qDebug("unable to clear printer margins");
      printerDev.setColorMode(QPrinter::Color);
      if (cs->isMaster())
            printerDev.setDocName(cs->masterScore()->fileInfo()->completeBaseName());
      else
            printerDev.setDocName(cs->excerpt()->title());
      printerDev.setOutputFormat(QPrinter::NativeFormat);
      int pages    = cs->pages().size();
      printerDev.setFromTo(1, pages);

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
      printerDev.setOutputFileName("");
#else
      // when setting this on windows platform, pd.exec() does not
      // show dialog
      if (cs->isMaster())
            printerDev.setOutputFileName(cs->masterScore()->fileInfo()->path() + "/" + cs->masterScore()->fileInfo()->completeBaseName() + ".pdf");
      else
            printerDev.setOutputFileName(cs->masterScore()->fileInfo()->path() + "/" + cs->excerpt()->title() + ".pdf");
#endif

      QPrintDialog pd(&printerDev, 0);

      if (pd.exec()) {
            QPainter p(&printerDev);
            p.setRenderHint(QPainter::Antialiasing, true);
            p.setRenderHint(QPainter::TextAntialiasing, true);
            double mag_ = printerDev.logicalDpiX() / DPI;

            double pr = MScore::pixelRatio;
            MScore::pixelRatio = 1.0 / mag_;
            p.scale(mag_, mag_);

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
            MScore::pixelRatio = pr;
            }

      if (layoutMode != cs->layoutMode()) {
            cs->setLayoutMode(layoutMode);
            cs->doLayout();
            }
#endif
      }

//---------------------------------------------------------
//   saveAs
//---------------------------------------------------------

bool MuseScore::saveAs(Score* cs_, bool saveCopy, const QString& path, const QString& ext, SaveReplacePolicy* replacePolicy)
      {
      bool rv = false;
      QString suffix = "." + ext;
      QString fn(path);
      if (!fn.endsWith(suffix))
            fn += suffix;

      LayoutMode layoutMode = cs_->layoutMode();
      if (ext == "mscx" || ext == "mscz") {
            // save as mscore *.msc[xz] file
            QFileInfo fi(fn);
            rv = true;
            // store new file and path into score fileInfo
            // to have it accessible to resources
            QFileInfo originalScoreFileInfo(*cs_->masterScore()->fileInfo());
            cs_->masterScore()->fileInfo()->setFile(fn);
            if (!cs_->isMaster()) { // clone metaTags from masterScore
                  QMapIterator<QString, QString> j(cs_->masterScore()->metaTags());
                  while (j.hasNext()) {
                        j.next();
                        if (j.key() != "partName") // don't copy "partName" should that exist in masterScore
                              cs_->metaTags().insert(j.key(), j.value());
#if defined(Q_OS_WIN)   // Update "platform", may not be worth the effort
                        cs_->metaTags().insert("platform", "Microsoft Windows");
#elif defined(Q_OS_MAC)
                        cs_->metaTags().insert("platform", "Apple Macintosh");
#elif defined(Q_OS_LINUX)
                        cs_->metaTags().insert("platform", "Linux");
#else
                        cs_->metaTags().insert("platform", "Unknown");
#endif
                        cs_->metaTags().insert("source", ""); // Empty "source" to avoid clashes with masterrScore when doing "Save online"
                        cs_->metaTags().insert("creationDate", QDate::currentDate().toString(Qt::ISODate)); // update "creationDate"
                        }
                  }
            try {
                  if (ext == "mscz")
                        cs_->saveCompressedFile(fi, false);
                  else
                        cs_->saveFile(fi);
                  }
            catch (QString s) {
                  rv = false;
                  QMessageBox::critical(this, tr("Save As"), s);
                  }
            if (!cs_->isMaster()) { // remove metaTags added above
                  QMapIterator<QString, QString> j(cs_->masterScore()->metaTags());
                  while (j.hasNext()) {
                        j.next();
                        // remove all but "partName", should that exist in masterScore
                        if (j.key() != "partName")
                              cs_->metaTags().remove(j.key());
                        }
                  }
            *cs_->masterScore()->fileInfo() = originalScoreFileInfo;          // restore original file info

            if (rv && !saveCopy) {
                  cs_->masterScore()->fileInfo()->setFile(fn);
                  updateWindowTitle(cs_);
                  cs_->undoStack()->setClean();
                  dirtyChanged(cs_);
                  cs_->setCreated(false);
                  scoreCmpTool->updateScoreVersions(cs_);
                  addRecentScore(cs_);
                  writeSessionFile(false);
                  }
            }
      else if ((ext == "musicxml") || (ext == "xml")) {
            // save as MusicXML *.musicxml file
            rv = saveXml(cs_, fn);
            }
      else if (ext == "mxl") {
            // save as compressed MusicXML *.mxl file
            rv = saveMxl(cs_, fn);
            }
      else if ((ext == "mid") || (ext == "midi")) {
            // save as midi file *.mid resp. *.midi
            rv = saveMidi(cs_, fn);
            }
      else if (ext == "pdf") {
            // save as pdf file *.pdf
            cs_->switchToPageMode();
            rv = savePdf(cs_, fn);
            }
      else if (ext == "png") {
            // save as png file *.png
            cs_->switchToPageMode();
            rv = savePng(cs_, fn, replacePolicy);
            }
      else if (ext == "svg") {
            // save as svg file *.svg
            cs_->switchToPageMode();
            rv = saveSvg(cs_, fn, NotesColors(), replacePolicy);
            }
#ifdef HAS_AUDIOFILE
      else if (ext == "wav" || ext == "flac" || ext == "ogg")
            rv = saveAudio(cs_, fn);
#endif
#ifdef USE_LAME
      else if (ext == "mp3")
            rv = saveMp3(cs_, fn);
#endif
      else if (ext == "spos") {
            cs_->switchToPageMode();
            // save positions of segments
            rv = savePositions(cs_, fn, true);
            }
      else if (ext == "mpos") {
            cs_->switchToPageMode();
            // save positions of measures
            rv = savePositions(cs_, fn, false);
            }
      else if (ext == "mlog") {
            rv = cs_->sanityCheck(fn);
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

      if (layoutMode != cs_->layoutMode()) {
            cs_->setLayoutMode(layoutMode);
            cs_->doLayout();
            }
      return rv;
      }

//---------------------------------------------------------
//   saveMidi
//---------------------------------------------------------

bool MuseScore::saveMidi(Score* score, const QString& name)
      {
      ExportMidi em(score);
      return em.write(name, preferences.getBool(PREF_IO_MIDI_EXPANDREPEATS), preferences.getBool(PREF_IO_MIDI_EXPORTRPNS), synthesizerState());
      }

bool MuseScore::saveMidi(Score* score, QIODevice* device)
      {
      ExportMidi em(score);
      return em.write(device, preferences.getBool(PREF_IO_MIDI_EXPANDREPEATS), preferences.getBool(PREF_IO_MIDI_EXPORTRPNS), synthesizerState());
      }

//---------------------------------------------------------
//   savePdf
//---------------------------------------------------------

bool MuseScore::savePdf(const QString& saveName)
      {
      return savePdf(cs, saveName);
      }

bool MuseScore::savePdf(Score* cs_, const QString& saveName)
      {
      QPrinter printer;
      printer.setOutputFileName(saveName);
      return savePdf(cs_, printer);
      }

bool MuseScore::savePdf(Score* cs_, QPrinter& printer)
      {
      cs_->setPrinting(true);
      MScore::pdfPrinting = true;

      printer.setResolution(preferences.getInt(PREF_EXPORT_PDF_DPI));
      QSizeF size(cs_->styleD(Sid::pageWidth), cs_->styleD(Sid::pageHeight));
      printer.setPaperSize(size, QPrinter::Inch);
      printer.setFullPage(true);
      printer.setColorMode(QPrinter::Color);
#if defined(Q_OS_MAC)
      printer.setOutputFormat(QPrinter::NativeFormat);
#else
      printer.setOutputFormat(QPrinter::PdfFormat);
#endif

      printer.setCreator("MuseScore Version: " VERSION);
      if (!printer.setPageMargins(QMarginsF()))
            qDebug("unable to clear printer margins");

      QString title = cs_->metaTag("workTitle");
      if (title.isEmpty()) // workTitle unset?
            title = cs_->masterScore()->title(); // fall back to (master)score's tab title
      if (!cs_->isMaster()) { // excerpt?
            QString partname = cs_->metaTag("partName");
            if (partname.isEmpty()) // partName unset?
                  partname = cs_->title(); // fall back to excerpt's tab title
            title += " - " + partname;
            }
      printer.setDocName(title); // set PDF's meta data for Title

      QPainter p;
      if (!p.begin(&printer))
            return false;
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::TextAntialiasing, true);

      p.setViewport(QRect(0.0, 0.0, size.width() * printer.logicalDpiX(),
         size.height() * printer.logicalDpiY()));
      p.setWindow(QRect(0.0, 0.0, size.width() * DPI, size.height() * DPI));

      double pr = MScore::pixelRatio;
      MScore::pixelRatio = DPI / printer.logicalDpiX();

      const QList<Page*> pl = cs_->pages();
      int pages = pl.size();
      bool firstPage = true;

      const QRect fillRect(0.0, 0.0, size.width() * DPI, size.height() * DPI);
      int exportBgStyle = preferences.getInt(PREF_EXPORT_BG_STYLE);
      bool useFgColor = preferences.getBool(PREF_UI_CANVAS_FG_USECOLOR);
      const QColor fgColor = preferences.getColor(PREF_UI_CANVAS_FG_COLOR);
      const QColor customColor = preferences.getColor(PREF_EXPORT_BG_CUSTOM_COLOR);
      const QPixmap fgPixMap(preferences.getString(PREF_UI_CANVAS_FG_WALLPAPER));

      for (int n = 0; n < pages; ++n) {
            if (!firstPage)
                  printer.newPage();
            firstPage = false;
            switch (exportBgStyle) {
                  case 1:
                        if (useFgColor)
                              p.fillRect(fillRect, fgColor);
                        else
                              p.drawTiledPixmap(fillRect, fgPixMap, fillRect.topLeft());
                        break;
                  case 2:
                        p.fillRect(fillRect, customColor);
                        break;
                  default:
                        break;
                  }
            cs_->print(&p, n);
            }
      p.end();
      cs_->setPrinting(false);

      MScore::pixelRatio = pr;
      MScore::pdfPrinting = false;
      return true;
      }

bool MuseScore::savePdf(QList<Score*> cs_, const QString& saveName)
      {
      if (cs_.empty())
            return false;
      Score* firstScore = cs_[0];

      QPrinter printer;
      printer.setOutputFileName(saveName);
      printer.setResolution(preferences.getInt(PREF_EXPORT_PDF_DPI));
      QSizeF size(firstScore->styleD(Sid::pageWidth), firstScore->styleD(Sid::pageHeight));
      QPageSize ps(QPageSize::id(size, QPageSize::Inch, QPageSize::FuzzyOrientationMatch));
      printer.setPageSize(ps);
      printer.setPageOrientation(size.width() > size.height() ? QPageLayout::Landscape : QPageLayout::Portrait);
      printer.setFullPage(true);
      printer.setColorMode(QPrinter::Color);
#if defined(Q_OS_MAC)
      printer.setOutputFormat(QPrinter::NativeFormat);
#else
      printer.setOutputFormat(QPrinter::PdfFormat);
#endif
      
      printer.setCreator("MuseScore Version: " VERSION);
      if (!printer.setPageMargins(QMarginsF()))
            qDebug("unable to clear printer margins");

      QString title = firstScore->metaTag("workTitle");
      if (title.isEmpty()) // workTitle unset?
            title = firstScore->title(); // fall back to (master)score's tab title
      printer.setDocName(title); // set PDF's meta data for Title

      QPainter p;
      if (!p.begin(&printer))
            return false;

      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::TextAntialiasing, true);

      double pr = MScore::pixelRatio;

      bool firstPage = true;
      const QRect fillRect(0.0, 0.0, size.width() * DPI, size.height() * DPI);
      int exportBgStyle = preferences.getInt(PREF_EXPORT_BG_STYLE);
      bool useFgColor = preferences.getBool(PREF_UI_CANVAS_FG_USECOLOR);
      const QColor fgColor = preferences.getColor(PREF_UI_CANVAS_FG_COLOR);
      const QColor customColor = preferences.getColor(PREF_EXPORT_BG_CUSTOM_COLOR);
      const QPixmap fgPixMap(preferences.getString(PREF_UI_CANVAS_FG_WALLPAPER));

      for (Score* s : cs_) {
            LayoutMode layoutMode = s->layoutMode();
            if (layoutMode != LayoutMode::PAGE) {
                  s->setLayoutMode(LayoutMode::PAGE);
                  }
            s->doLayout();

            // done in Score::print() also, but do it here as well to be safe
            s->setPrinting(true);
            MScore::pdfPrinting = true;

            QSizeF size1(s->styleD(Sid::pageWidth), s->styleD(Sid::pageHeight));
            QPageSize ps1(QPageSize::id(size1, QPageSize::Inch, QPageSize::FuzzyOrientationMatch));
            printer.setPageSize(ps1);
            printer.setPageOrientation(size1.width() > size1.height() ? QPageLayout::Landscape : QPageLayout::Portrait);
            p.setViewport(QRect(0.0, 0.0, size1.width() * printer.logicalDpiX(),
               size1.height() * printer.logicalDpiY()));
            p.setWindow(QRect(0.0, 0.0, size1.width() * DPI, size1.height() * DPI));

            MScore::pixelRatio = DPI / printer.logicalDpiX();
            const QList<Page*> pl = s->pages();
            int pages    = pl.size();
            for (int n = 0; n < pages; ++n) {
                  if (!firstPage)
                        printer.newPage();
                  firstPage = false;
                  s->print(&p, n);
                  }
            MScore::pixelRatio = pr;

            //reset score
            s->setPrinting(false);

            switch (exportBgStyle) {
                  case 1:
                      if (useFgColor)
                          p.fillRect(fillRect, fgColor);
                      else
                          p.drawTiledPixmap(fillRect, fgPixMap, fillRect.topLeft());
                      break;
                  case 2:
                      p.fillRect(fillRect, customColor);
                      break;
                  default:
                      break;
                  }

            MScore::pdfPrinting = false;

            if (layoutMode != s->layoutMode()) {
                  s->setLayoutMode(layoutMode);
                  s->doLayout();
                  }
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
            QStringList pl = preferences.getString(PREF_APP_PATHS_MYSOUNDFONTS).split(";");
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

Score::FileError readScore(MasterScore* score, QString name, bool ignoreVersionError)
      {
      ScoreLoad sl;

      QFileInfo info(name);
      QString suffix  = info.suffix().toLower();
      score->setName(info.completeBaseName());
      score->setImportedFilePath(name);

      // Set the default synthesizer state before we read
      if (synti)
            score->setSynthesizerState(synti->state());

      auto read = [](MasterScore* score, const QString& name, bool ignoreVersionError, bool imported = false)->Score::FileError {
            Score::FileError rv = score->loadMsc(name, ignoreVersionError);
            if (imported || (score && score->masterScore()->fileInfo()->path().startsWith(":/")))
                  score->setCreated(true);
            score->setAutosaveDirty(!imported);
            return rv;
            };

      if (suffix == "mscz" || suffix == "mscx") {
            Score::FileError rv = read(score, name, ignoreVersionError);
            if (rv != Score::FileError::FILE_NO_ERROR)
                  return rv;
            }
      else if (suffix == "mscz," || suffix == "mscx,") {
            Score::FileError rv = read(score, name, ignoreVersionError, true);
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
            // typedef Score::FileError (*ImportFunction)(MasterScore*, const QString&);
            struct ImportDef {
                  const char* extension;
                  // ImportFunction importF;
                  Score::FileError (*importF)(MasterScore*, const QString&);
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
                  { "bmw",  &importBww                },
                  { "bww",  &importBww                },
                  { "gtp",  &importGTP                },
                  { "gp3",  &importGTP                },
                  { "gp4",  &importGTP                },
                  { "gp5",  &importGTP                },
                  { "gpx",  &importGTP                },
                  { "gp",   &importGTP                },
                  { "ptb",  &importGTP                },
#ifdef AVSOMR
                  { "msmr", &importMSMR               },
                  { "pdf",  &loadAndImportMSMR        },
                  { "png",  &loadAndImportMSMR        },
                  { "jpg",  &loadAndImportMSMR        },
#endif
                  };

            // import
            if (!preferences.getString(PREF_IMPORT_STYLE_STYLEFILE).isEmpty()) {
                  QFile f(preferences.getString(PREF_IMPORT_STYLE_STYLEFILE));
                  // silently ignore style file on error
                  if (f.open(QIODevice::ReadOnly))
                        score->style().load(&f);
                  }
            else {
                  score->style().checkChordList();
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
            if (!score->avsOmr()) //! NOTE For avsomr сreated is set upon import
                  score->setCreated(true); // force save as for imported files
            }

      for (Part* p : score->parts()) {
            p->updateHarmonyChannels(false);
            }
      score->rebuildMidiMapping();
      score->setSoloMute();
      for (Score* s : score->scoreList()) {
            s->setPlaylistDirty();
            s->addLayoutFlags(LayoutFlag::FIX_PITCH_VELO);
            s->setLayoutAll();
            }
      score->updateChannel();
      score->updateExpressive(MuseScore::synthesizer("Fluid"));
      score->setSaved(false);
      score->update();
      score->styleChanged();

      if (!ignoreVersionError && !MScore::noGui)
            if (!score->sanityCheck(QString()))
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

bool MuseScore::saveAs(Score* cs_, bool saveCopy)
      {
      QStringList fl;
      fl.append(tr("MuseScore 3 File") + " (*.mscz)");
      fl.append(tr("Uncompressed MuseScore 3 File") + " (*.mscx)");     // for debugging purposes
      QString saveDialogTitle = saveCopy ? tr("Save a Copy") :
                                           tr("Save As");

      QString saveDirectory;
      if (cs_->masterScore()->fileInfo()->exists())
            saveDirectory = cs_->masterScore()->fileInfo()->dir().path();
      else {
            QSettings set;
            if (saveCopy) {
                  if (mscore->lastSaveCopyDirectory.isEmpty())
                        mscore->lastSaveCopyDirectory = set.value("lastSaveCopyDirectory", preferences.getString(PREF_APP_PATHS_MYSCORES)).toString();
                  saveDirectory = mscore->lastSaveCopyDirectory;
                  }
            else {
                  if (mscore->lastSaveDirectory.isEmpty())
                        mscore->lastSaveDirectory = set.value("lastSaveDirectory", preferences.getString(PREF_APP_PATHS_MYSCORES)).toString();
                  saveDirectory = mscore->lastSaveDirectory;
                  }
            }

      if (saveDirectory.isEmpty())
            saveDirectory = preferences.getString(PREF_APP_PATHS_MYSCORES);

      QString fileBaseName = cs_->masterScore()->fileInfo()->completeBaseName();
      QString fileName = cs_->masterScore()->fileInfo()->fileName();
      // is backup file
      if (fileBaseName.startsWith(".")
         && (fileName.endsWith(".mscz,") || fileName.endsWith(".mscx,")))
            fileBaseName.remove(0, 1); // remove the "." at the beginning of file name

      QString name;
#ifdef Q_OS_WIN
      if (QOperatingSystemVersion::current() <= QOperatingSystemVersion(QOperatingSystemVersion::Windows, 5, 1)) {   //XP
            if (!cs_->isMaster())
                  name = QString("%1/%2-%3").arg(saveDirectory).arg(fileBaseName).arg(createDefaultFileName(cs->title()));
            else
                  name = QString("%1/%2").arg(saveDirectory).arg(fileBaseName);
            }
      else
#endif
      if (!cs_->isMaster())
            name = QString("%1/%2-%3.mscz").arg(saveDirectory).arg(fileBaseName).arg(createDefaultFileName(cs->title()));
      else
            name = QString("%1/%2.mscz").arg(saveDirectory).arg(fileBaseName);

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
      return saveAs(cs_, saveCopy, fn, fi.suffix());
      }

//---------------------------------------------------------
//   saveSelection
//    return true on success
//---------------------------------------------------------

bool MuseScore::saveSelection(Score* cs_)
      {
      if (!cs_->selection().isRange()) {
            if(!MScore::noGui) QMessageBox::warning(mscore, tr("Save Selection"), tr("Please select one or more measures"));
            return false;
            }
      QStringList fl;
      fl.append(tr("MuseScore 3 File") + " (*.mscz)");
      fl.append(tr("Uncompressed MuseScore 3 File") + " (*.mscx)");     // for debugging purposes
      QString saveDialogTitle = tr("Save Selection");

      QString saveDirectory;
      if (cs_->masterScore()->fileInfo()->exists())
            saveDirectory = cs_->masterScore()->fileInfo()->dir().path();
      else {
            QSettings set;
            if (mscore->lastSaveDirectory.isEmpty())
                  mscore->lastSaveDirectory = set.value("lastSaveDirectory", preferences.getString(PREF_APP_PATHS_MYSCORES)).toString();
            saveDirectory = mscore->lastSaveDirectory;
            }

      if (saveDirectory.isEmpty())
            saveDirectory = preferences.getString(PREF_APP_PATHS_MYSCORES);

      QString name   = QString("%1/%2.mscz").arg(saveDirectory).arg(cs_->title());
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
            cs_->saveCompressedFile(fi, true);
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
         tr("PNG Bitmap Graphic") + " (*.png)",
         0,
         preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS) ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog
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
//    return true on success.  Works with editor, shows additional windows.
//---------------------------------------------------------

bool MuseScore::savePng(Score* score, const QString& name, SaveReplacePolicy* replacePolicy)
      {
      int pages    = score->pages().size();
      int padding  = QString("%1").arg(pages).size();
      bool success = true;
      SaveReplacePolicy _replacePolicy = (replacePolicy != nullptr ? *replacePolicy : SaveReplacePolicy::NO_CHOICE);
      
      for (int pageNumber = 0; pageNumber < pages; ++pageNumber) {
            QString fileName(name);
            if (fileName.endsWith(".png"))
                  fileName = fileName.left(fileName.size() - 4);
            fileName += QString("-%1.png").arg(pageNumber+1, padding, 10, QLatin1Char('0'));
            if (!converterMode && QFileInfo(fileName).exists()) {
                  switch (_replacePolicy) {
                        case SaveReplacePolicy::NO_CHOICE:
                              {
                              int responseCode = mscore->askOverwriteAll(fileName);
                              if (responseCode == QMessageBox::YesToAll) {
                                    _replacePolicy = SaveReplacePolicy::REPLACE_ALL;
                                    break; // Break out of the switch; go on and replace the existing file
                                    }
                              else if (responseCode == QMessageBox::NoToAll) {
                                    _replacePolicy = SaveReplacePolicy::SKIP_ALL;
                                    continue; // Continue in the `for` loop
                                    }
                              else if (responseCode == QMessageBox::No)
                                    continue;
                              break;
                              }
                        case SaveReplacePolicy::SKIP_ALL:
                              continue;
                        case SaveReplacePolicy::REPLACE_ALL:
                              break;
                        }
                  }
            QFile f(fileName);
            if (!f.open(QIODevice::WriteOnly) || !savePng(score, &f, pageNumber)) {
                  success = false;
                  break;
                  }
            }
      if (replacePolicy != nullptr)
            *replacePolicy = _replacePolicy;
      return success;
      }

//---------------------------------------------------------
//   savePng with options
//    return true on success
//---------------------------------------------------------

bool MuseScore::savePng(Score* score, QIODevice* device, int pageNumber, bool drawPageBackground)
      {
      Q_UNUSED(drawPageBackground);
      const bool screenshot = false;
      const double convDpi = preferences.getDouble(PREF_EXPORT_PNG_RESOLUTION);
      const int localTrimMargin = trimMargin;
      const QImage::Format format = QImage::Format_ARGB32_Premultiplied;

      bool rv = true;
      score->setPrinting(!screenshot);    // don’t print page break symbols etc.
      double pr = MScore::pixelRatio;

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

      int exportBgStyle = preferences.getInt(PREF_EXPORT_BG_STYLE);
      bool useFgColor = preferences.getBool(PREF_UI_CANVAS_FG_USECOLOR);
      const QColor fgColor = preferences.getColor(PREF_UI_CANVAS_FG_COLOR);
      const QColor customColor = preferences.getColor(PREF_EXPORT_BG_CUSTOM_COLOR);
      const QPixmap fgPixMap(preferences.getString(PREF_UI_CANVAS_FG_WALLPAPER));

      switch (exportBgStyle) {
            case 0:
                  printer.fill(0);
                  break;
            case 1:
                  if (useFgColor)
                        printer.fill(fgColor);
                  else {
                        QPainter painter(&printer);
                        painter.drawTiledPixmap(r, fgPixMap, r.topLeft());
                  }
                  break;
            case 2:
                  printer.fill(customColor);
                  break;
            default:
                  break;
            }

      double mag_ = convDpi / DPI;
      MScore::pixelRatio = 1.0 / mag_;

      QPainter p(&printer);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::TextAntialiasing, true);
      p.scale(mag_, mag_);
      if (localTrimMargin >= 0)
            p.translate(-r.topLeft());

      QList< Element*> pel = page->elements();
      std::stable_sort(pel.begin(), pel.end(), elementLessThan);
      paintElements(p, pel);
       if (format == QImage::Format_Indexed8) {
            //convert to grayscale & respect alpha
            QVector<QRgb> colorTable;
            colorTable.push_back(QColor(0, 0, 0, 0).rgba());
            if (exportBgStyle != 0) {
                  for (int i = 1; i < 256; i++)
                        colorTable.push_back(QColor(i, i, i).rgb());
                  }
            else {
                  for (int i = 1; i < 256; i++)
                        colorTable.push_back(QColor(0, 0, 0, i).rgba());
                  }
            printer = printer.convertToFormat(QImage::Format_Indexed8, colorTable);
            }
      printer.save(device, "png");
      score->setPrinting(false);
      MScore::pixelRatio = pr;
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

      if (preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS)) {
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

            QSettings set;
            loadBackgroundDialog->restoreState(set.value("loadBackgroundDialog").toByteArray());
            loadBackgroundDialog->setAcceptMode(QFileDialog::AcceptOpen);

            QSplitter* sp = loadBackgroundDialog->findChild<QSplitter*>("splitter");
            if (sp) {
                  WallpaperPreview* preview = new WallpaperPreview;
                  sp->addWidget(preview);
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
//   askOverwriteAll
//---------------------------------------------------------

int MuseScore::askOverwriteAll(QString& filename)
{
      QMessageBox msgBox(QMessageBox::Question,
                         tr("Confirm Replace"),
                         tr("\"%1\" already exists.\nDo you want to replace it?\n").arg(QDir::toNativeSeparators(filename)),
                         QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll);
      msgBox.setButtonText(QMessageBox::Yes, tr("Replace"));
      msgBox.setButtonText(QMessageBox::No,  tr("Skip"));
      msgBox.setButtonText(QMessageBox::YesToAll, tr("Replace All"));
      msgBox.setButtonText(QMessageBox::NoToAll,  tr("Skip All"));
      return msgBox.exec();
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
bool MuseScore::saveSvg(Score* score, const QString& name, const NotesColors& notesColors, SaveReplacePolicy* replacePolicy)
      {
      int pages    = score->pages().size();
      int padding  = QString("%1").arg(pages).size();
      bool success = true;
      SaveReplacePolicy _replacePolicy = (replacePolicy != nullptr ? *replacePolicy : SaveReplacePolicy::NO_CHOICE);
      
      for (int pageNumber = 0; pageNumber < pages; ++pageNumber) {
            QString fileName(name);
            if (fileName.endsWith(".svg"))
                  fileName = fileName.left(fileName.size() - 4);
            fileName += QString("-%1.svg").arg(pageNumber+1, padding, 10, QLatin1Char('0'));
            if (!converterMode && QFileInfo(fileName).exists()) {
                  switch (_replacePolicy) {
                        case SaveReplacePolicy::NO_CHOICE:
                              {
                              int responseCode = mscore->askOverwriteAll(fileName);
                              if (responseCode == QMessageBox::YesToAll) {
                                    _replacePolicy = SaveReplacePolicy::REPLACE_ALL;
                                    break; // Break out of the switch; go on and replace the existing file
                                    }
                              else if (responseCode == QMessageBox::NoToAll) {
                                    _replacePolicy = SaveReplacePolicy::SKIP_ALL;
                                    continue; // Continue in the `for` loop
                                    }
                              else if (responseCode == QMessageBox::No)
                                    continue;
                              break;
                              }
                        case SaveReplacePolicy::SKIP_ALL:
                              continue;
                        case SaveReplacePolicy::REPLACE_ALL:
                              break;
                        }
                  }
            QFile f(fileName);
            if (!f.open(QIODevice::WriteOnly) || !saveSvg(score, &f, pageNumber, /*drawPageBackground*/ false, notesColors)) {
                  success = false;
                  break;
                  }
            }
      if (replacePolicy != nullptr)
            *replacePolicy = _replacePolicy;
      return success;
      }

//---------------------------------------------------------
//   MuseScore::readNotesColors
///  Read notes colors from json file
//---------------------------------------------------------

NotesColors MuseScore::readNotesColors(const QString& filePath) const
{
    if (filePath.isEmpty()) {
        return NotesColors();
    }

    QFile file;
    file.setFileName(filePath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString content = file.readAll();
    file.close();

    QJsonDocument document = QJsonDocument::fromJson(content.toUtf8());
    QJsonObject obj = document.object();
    QJsonArray colors = obj.value("highlight").toArray();

    NotesColors result;

    for (const QJsonValue colorObj: colors) {
        QJsonObject cobj = colorObj.toObject();
        QJsonArray notesIndexes = cobj.value("notes").toArray();
        QColor notesColor = QColor(cobj.value("color").toString());

        for (const QJsonValue index: notesIndexes) {
            result.insert(index.toInt(), notesColor);
        }
    }

    return result;
}

//---------------------------------------------------------
//   MuseScore::saveSvg
///  Save a single page
//---------------------------------------------------------

bool MuseScore::saveSvg(Score* score, QIODevice* device, int pageNumber, bool drawPageBackground, const NotesColors& notesColors)
      {
      Q_UNUSED(drawPageBackground);
      QString title(score->title());
      score->setPrinting(true);
      MScore::pdfPrinting = true;
      MScore::svgPrinting = true;
      const QList<Page*>& pl = score->pages();
      int pages = pl.size();
      double pr = MScore::pixelRatio;

      Page* page = pl.at(pageNumber);
      SvgGenerator printer;
      printer.setTitle(pages > 1 ? QString("%1 (%2)").arg(title).arg(pageNumber + 1) : title);
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
      if (trimMargin >= 0 && score->npages() == 1)
            p.translate(-r.topLeft());
      MScore::pixelRatio = DPI / printer.logicalDpiX();

      int exportBgStyle = preferences.getInt(PREF_EXPORT_BG_STYLE);
      bool useFgColor = preferences.getBool(PREF_UI_CANVAS_FG_USECOLOR);
      const QColor fgColor = preferences.getColor(PREF_UI_CANVAS_FG_COLOR);
      const QColor customColor = preferences.getColor(PREF_EXPORT_BG_CUSTOM_COLOR);
      const QPixmap fgPixMap(preferences.getString(PREF_UI_CANVAS_FG_WALLPAPER));

      switch (exportBgStyle) {
            case 1:
                  if (useFgColor)
                        p.fillRect(r, fgColor);
                  else
                        p.drawTiledPixmap(r, fgPixMap, r.topLeft());
                  break;
            case 2:
                  p.fillRect(r, customColor);
                  break;
            default:
                  break;
      }

      QList<Element*> pel = page->elements();
      std::stable_sort(pel.begin(), pel.end(), elementLessThan);

      int lastNoteIndex = -1;
      for (int i = 0; i < pageNumber; ++i) {
            for (const Element* element: score->pages()[i]->elements()) {
                  if (element->type() == ElementType::NOTE) {
                        lastNoteIndex++;
                        }
                  }
            }

      System* currentSystem               { nullptr };
      Measure* firstMeasureOfSystem       { nullptr };
      const Measure* currentMeasure       { nullptr };
      std::vector<System*> printedSystems;
      for (const Element* e : pel) {
            // Always exclude invisible elements
            if (!e->visible())
                  continue;
            if (e->type() == ElementType::STAFF_LINES) {
                  currentMeasure = e->findMeasure();
                  currentSystem = currentMeasure->system();

                  if (std::find(printedSystems.begin(), printedSystems.end(), currentSystem) != printedSystems.end())
                        continue; // Skip lines if current system has been drawn already

                  firstMeasureOfSystem = currentSystem->firstMeasure();
                  if (!firstMeasureOfSystem) // only boxes, hence no staff lines
                        continue;
                  for (int i = 0, n = currentSystem->staves()->size(); i < n; i++) {
                        if (score->staff(i)->invisible(Fraction(0,1)) || !score->staff(i)->show())
                              continue;  // ignore invisible staves
                        if (currentSystem->staves()->isEmpty() || !currentSystem->staff(i)->show())
                              continue;
                        // Draw SVG lines per entire system for efficiency
                        // Exceptions (draw SVG staff lines by measure instead):
                        //
                        //   One (or more) invisible measure(s) in a system/staff
                        //   One (or more) elements of type HBOX or VBOX
                        //   One (or more) Staff Type Change(s) within a system
                        //
                        bool byMeasure = false;
                        for (MeasureBase* mb = firstMeasureOfSystem;
                             mb && !byMeasure;
                             mb = currentSystem->nextMeasure(mb)) {
                              if (!mb->isMeasure() || !toMeasure(mb)->visible(i)) {
                                    byMeasure = true;
                                    break;
                                    }
                              for (Element* element : toMeasure(mb)->el()) {
                                    if (element->isStaffTypeChange()) {
                                          byMeasure = true;
                                          break;
                                          }
                                    }
                              }
                        if (byMeasure) {
                              // Draw visible staff lines by measure (all of current system)
                              for (MeasureBase* mb = firstMeasureOfSystem; mb; mb = currentSystem->nextMeasure(mb)) {
                                    if (mb->isMeasure() && toMeasure(mb)->visible(i)) {
                                          StaffLines* sl = toMeasure(mb)->staffLines(i);
                                          printer.setElement(sl);
                                          paintElement(p, sl);
                                          }
                                    }
                              }
                        else { // Draw staff lines once per system
                              StaffLines* firstSL = firstMeasureOfSystem->staffLines(i)->clone();
                              StaffLines*  lastSL = currentSystem->lastMeasure()->staffLines(i);

                              qreal lastX = lastSL->bbox().right()
                                            + lastSL->pagePos().x()
                                            - firstSL->pagePos().x();
                              QVector<QLineF>& lines = firstSL->getLines();
                              for (int l = 0, c = lines.size(); l < c; l++)
                                    lines[l].setP2(QPointF(lastX, lines[l].p2().y()));

                              printer.setElement(firstSL);
                              paintElement(p, firstSL);
                              }
                        }
                  printedSystems.push_back(currentSystem);
                  continue; // Drawing of staff-line element complete
                  }
            // Set the Element pointer inside SvgGenerator/SvgPaintEngine
            printer.setElement(e);

            // Paint it
            if (e->type() == ElementType::NOTE && !notesColors.isEmpty()) {
                  QColor color = e->color();
                  int currentNoteIndex = (++lastNoteIndex);

                  if (notesColors.contains(currentNoteIndex)) {
                        color = notesColors[currentNoteIndex];
                        }

                  Element *note = dynamic_cast<const Note*>(e)->clone();
                  note->setColor(color);
                  paintElement(p, note);
                  delete note;
                  }
            else paintElement(p, e);
            } // End of element loop

      p.end(); // Writes MuseScore SVG file to disk, finally

      // Clean up and return
      MScore::pixelRatio = pr;
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
      if (!(name.endsWith(".mscx") || name.endsWith(".mscz")))
            return QPixmap();
      MasterScore* score = new MasterScore(MScore::defaultStyle());
      Score::FileError error = readScore(score, name, true);
      if (error != Score::FileError::FILE_NO_ERROR || !score->firstMeasure()) {
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

//---------------------------------------------------------
//   saveMetadataJSON
//---------------------------------------------------------

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

//---------------------------------------------------------
//   findTextByType
//    @data must contain std::pair<Tid, QStringList*>*
//          Tid specifies text style
//          QStringList* specifies the container to keep found text
//
//    For usage with Score::scanElements().
//    Finds all text elements with specified style.
//---------------------------------------------------------
static void findTextByType(void* data, Element* element)
      {
      if (!element->isTextBase())
            return;
      TextBase* text = toTextBase(element);
      auto* typeStringsData = static_cast<std::pair<Tid, QStringList*>*>(data);
      if (text->tid() == typeStringsData->first) {
            // or if score->getTextStyleUserName().contains("Title") ???
            // That is bad since it may be localized
            QStringList* titleStrings = typeStringsData->second;
            Q_ASSERT(titleStrings);
            titleStrings->append(text->plainText());
            }
      }

QJsonObject MuseScore::saveMetadataJSON(Score* score)
      {
      auto boolToString = [](bool b) { return b ? "true" : "false"; };
      QJsonObject json;

      // title
      QString title;
      Text* t = score->getText(Tid::TITLE);
      if (t)
            title = t->plainText();
      if (title.isEmpty())
            title = score->metaTag("workTitle");
      if (title.isEmpty())
            title = score->title();
      json.insert("title", title);

      // subtitle
      QString subtitle;
      t = score->getText(Tid::SUBTITLE);
      if (t)
            subtitle = t->plainText();
      json.insert("subtitle", subtitle);

      // composer
      QString composer;
      t = score->getText(Tid::COMPOSER);
      if (t)
            composer = t->plainText();
      if (composer.isEmpty())
            composer = score->metaTag("composer");
      json.insert("composer", composer);

      // poet
      QString poet;
      t = score->getText(Tid::POET);
      if (t)
            poet = t->plainText();
      if (poet.isEmpty())
            poet = score->metaTag("lyricist");
      json.insert("poet", poet);

      json.insert("mscoreVersion", score->mscoreVersion());
      json.insert("fileVersion", score->mscVersion());

      json.insert("pages", score->npages());
      json.insert("measures", score->nmeasures());
      json.insert("hasLyrics", boolToString(score->hasLyrics()));
      json.insert("hasHarmonies", boolToString(score->hasHarmonies()));
      json.insert("keysig", score->keysig());
      json.insert("previousSource", score->metaTag("source"));

      // timeSig
      QString timeSig;
      int staves = score->nstaves();
      int tracks = staves * VOICES;
      Segment* tss = score->firstSegmentMM(SegmentType::TimeSig);
      if (tss) {
            Element* e = nullptr;
            for (int track = 0; track < tracks; ++track) {
                  e = tss->element(track);
                  if (e) break;
                  }
            if (e && e->isTimeSig()) {
                  TimeSig* ts = toTimeSig(e);
                  timeSig = QString("%1/%2").arg(ts->numerator()).arg(ts->denominator());
                  }
            }
      json.insert("timesig", timeSig);

      json.insert("duration", score->duration());
      json.insert("lyrics", score->extractLyrics());

      // tempo
       int tempo = 0;
       QString tempoText;
       for (Segment* seg = score->firstSegmentMM(SegmentType::All); seg; seg = seg->next1MM()) {
             auto annotations = seg->annotations();
             for (Element* a : annotations) {
                   if (a && a->isTempoText()) {
                         TempoText* tt = toTempoText(a);
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
      jsonPageformat.insert("height",round(score->styleD(Sid::pageHeight) * INCH));
      jsonPageformat.insert("width", round(score->styleD(Sid::pageWidth) * INCH));
      jsonPageformat.insert("twosided", boolToString(score->styleB(Sid::pageTwosided)));
      json.insert("pageFormat", jsonPageformat);

      //text frames metadata
      QJsonObject jsonTypeData;
      static std::vector<std::pair<QString, Tid>> namesTypesList {
            {"titles", Tid::TITLE},
            {"subtitles", Tid::SUBTITLE},
            {"composers", Tid::COMPOSER},
            {"poets", Tid::POET}
            };
      for (auto nameType : namesTypesList) {
            QJsonArray typeData;
            QStringList typeTextStrings;
            std::pair<Tid, QStringList*> extendedTitleData = std::make_pair(nameType.second, &typeTextStrings);
            score->scanElements(&extendedTitleData, findTextByType);
            for (auto typeStr : typeTextStrings)
                  typeData.append(typeStr);
            jsonTypeData.insert(nameType.first, typeData);
            }
      json.insert("textFramesData", jsonTypeData);

      return json;
      }

class CustomJsonWriter
{
public:
      CustomJsonWriter(const QString& filePath)
      {
      jsonFormatFile.setFileName(filePath);
      jsonFormatFile.open(QIODevice::WriteOnly);
      jsonFormatFile.write("{\n");
      }

      ~CustomJsonWriter()
      {
      jsonFormatFile.write("\n}\n");
      jsonFormatFile.close();
      }

      void addKey(const char* arrayName)
      {
      jsonFormatFile.write("\"");
      jsonFormatFile.write(arrayName);
      jsonFormatFile.write("\": ");
      }

      void addValue(const QByteArray& data, bool lastJsonElement = false, bool isJson = false)
      {
      if (!isJson)
            jsonFormatFile.write("\"");
      jsonFormatFile.write(data);
      if (!isJson)
            jsonFormatFile.write("\"");
      if (!lastJsonElement)
            jsonFormatFile.write(",\n");
      }

      void openArray()
      {
      jsonFormatFile.write(" [");
      }

      void closeArray(bool lastJsonElement = false)
      {
      jsonFormatFile.write("]");
      if (!lastJsonElement)
            jsonFormatFile.write(",");
      jsonFormatFile.write("\n");
      }

private:
      QFile jsonFormatFile;
};

//---------------------------------------------------------
//   exportMp3AsJSON
//---------------------------------------------------------

bool MuseScore::exportMp3AsJSON(const QString& inFilePath, const QString& outFilePath)
      {
      std::unique_ptr<MasterScore> score(mscore->readScore(inFilePath));
      if (!score)
            return false;

      CustomJsonWriter jsonWriter(outFilePath);
      jsonWriter.addKey("mp3");
      //export score audio
      QByteArray mp3Data;
      QBuffer mp3Device(&mp3Data);
      mp3Device.open(QIODevice::ReadWrite);
      bool dummy = false;
      mscore->saveMp3(score.get(), &mp3Device, dummy);
      jsonWriter.addValue(mp3Data.toBase64(), true);
      return true;
      }

QByteArray MuseScore::exportPdfAsJSON(Score* score)
      {
      QTemporaryFile tempPdfFile;
      bool ok = tempPdfFile.open();

      if (!ok) {
          return QByteArray();
      }

      QPrinter printer;
      printer.setOutputFileName(tempPdfFile.fileName());
      mscore->savePdf(score, printer);

      QByteArray pdfData = tempPdfFile.readAll();
      tempPdfFile.close();

      return pdfData.toBase64();
      }

//---------------------------------------------------------
//   parseSourceUrl
//---------------------------------------------------------

static void parseSourceUrl(const QString& sourceUrl, int& uid, int& nid)
      {
      if (!sourceUrl.isEmpty()) {
            QStringList sl = sourceUrl.split("/");
            if (sl.length() >= 1) {
                  nid = sl.last().toInt();
                  if (sl.length() >= 3) {
                        uid = sl.at(sl.length() - 3).toInt();
                        }
                  }
            }
      }

//---------------------------------------------------------
//   saveOnline
//---------------------------------------------------------

bool MuseScore::saveOnline(const QStringList& inFilePaths)
      {
      if (MuseScore::unstable()) {
            qCritical() << qUtf8Printable(tr("Error: Saving scores online is disabled in this unstable prerelease version of MuseScore."));
            return false;
      }
      if (!_loginManager->syncGetUser()) {
            return false;
            }

      QTemporaryDir tempDir;
      if (!tempDir.isValid()) {
            qCritical() << qUtf8Printable(tr("Error: %1").arg(tempDir.errorString()));
            return false;
            }
      QString tempPath = tempDir.path() + "/score.mscz";

      bool all_successful = true;

      for (auto path : inFilePaths) {
            Score* score = mscore->readScore(path);
            if (!score) {
                  all_successful = false;
                  continue;
                  }

            int uid = 0;
            int nid = 0;
            parseSourceUrl(score->metaTag("source"), uid, nid);

            if (nid <= 0) {
                  qCritical() << qUtf8Printable(tr("Error: '%1' tag missing or malformed in %2").arg("source").arg(path));
                  all_successful = false;
                  continue;
                  }

            if (uid && uid != _loginManager->uid()) {
                  qCritical() << qUtf8Printable(tr("Error: You are not the owner of the online score for %1").arg(path));
                  all_successful = false;
                  continue;
                  }

            if (!_loginManager->syncGetScoreInfo(nid)) {
                  all_successful = false;
                  continue;
                  }
            QString title = _loginManager->scoreTitle();

            if (!mscore->saveAs(score, true, tempPath, "mscz")) {
                  all_successful = false;
                  continue;
                  }

            if (!_loginManager->syncUpload(tempPath, nid, title)) { // keep same title
                  all_successful = false;
                  continue;
                  }

            qInfo() << qUtf8Printable(tr("Uploaded score")) << path;
            }

      return all_successful;
      }

//---------------------------------------------------------
//   exportAllMediaFiles
//---------------------------------------------------------

bool MuseScore::exportAllMediaFiles(const QString& inFilePath, const QString& highlightConfigPath, const QString& outFilePath)
      {
      std::unique_ptr<MasterScore> score(mscore->readScore(inFilePath));
      if (!score)
            return false;

      score->switchToPageMode();

      score->updateCapo();
      score->update();
      score->styleChanged();
      score->doLayout();

      //// JSON specification ///////////////////////////
      //jsonForMedia["pngs"] = pngsJsonArray;
      //jsonForMedia["mposXML"] = mposJson;
      //jsonForMedia["sposXML"] = sposJson;
      //jsonForMedia["pdf"] = pdfJson;
      //jsonForMedia["svgs"] = svgsJsonArray;
      //jsonForMedia["midi"] = midiJson;
      //jsonForMedia["mxml"] = mxmlJson;
      //jsonForMedia["metadata"] = mdJson;
      ///////////////////////////////////////////////////

      bool res = true;
      CustomJsonWriter jsonWriter(outFilePath);
      //export score pngs and svgs
      jsonWriter.addKey("pngs");
      jsonWriter.openArray();
      for (int i = 0; i < score->pages().size(); ++i) {
            QByteArray pngData;
            QBuffer pngDevice(&pngData);
            pngDevice.open(QIODevice::ReadWrite);
            res &= mscore->savePng(score.get(), &pngDevice, i, /* drawPageBackground */ true);
            bool lastArrayValue = ((score->pages().size() - 1) == i);
            jsonWriter.addValue(pngData.toBase64(), lastArrayValue);
            }
      jsonWriter.closeArray();

      jsonWriter.addKey("svgs");
      jsonWriter.openArray();
      for (int i = 0; i < score->pages().size(); ++i) {
            QByteArray svgData;
            QBuffer svgDevice(&svgData);
            svgDevice.open(QIODevice::ReadWrite);

            NotesColors notesColors = readNotesColors(highlightConfigPath);
            res &= mscore->saveSvg(score.get(), &svgDevice, i, /* drawPageBackground */ true, notesColors);

            bool lastArrayValue = ((score->pages().size() - 1) == i);
            jsonWriter.addValue(svgData.toBase64(), lastArrayValue);
            }
      jsonWriter.closeArray();

      {
      //export score .spos
      QByteArray partDataPos;
      QBuffer partPosDevice(&partDataPos);
      partPosDevice.open(QIODevice::ReadWrite);
      savePositions(score.get(), &partPosDevice, true);
      jsonWriter.addKey("sposXML");
      jsonWriter.addValue(partDataPos.toBase64());
      partPosDevice.close();
      partDataPos.clear();

      //export score .mpos
      partPosDevice.open(QIODevice::ReadWrite);
      savePositions(score.get(), &partPosDevice, false);
      jsonWriter.addKey("mposXML");
      jsonWriter.addValue(partDataPos.toBase64());
      }

      //export score pdf
      jsonWriter.addKey("pdf");
      jsonWriter.addValue(exportPdfAsJSON(score.get()));

      {
      //export score midi
      QByteArray midiData;
      QBuffer midiDevice(&midiData);
      midiDevice.open(QIODevice::ReadWrite);
      res &= mscore->saveMidi(score.get(), &midiDevice);
      jsonWriter.addKey("midi");
      jsonWriter.addValue(midiData.toBase64());
      }

      {
      //export musicxml
      QByteArray mxmlData;
      QBuffer mxmlDevice(&mxmlData);
      mxmlDevice.open(QIODevice::ReadWrite);
      res &= saveMxl(score.get(), &mxmlDevice);
      jsonWriter.addKey("mxml");
      jsonWriter.addValue(mxmlData.toBase64());
      }

      //export metadata
      QJsonDocument doc(mscore->saveMetadataJSON(score.get()));
      jsonWriter.addKey("metadata");
      jsonWriter.addValue(doc.toJson(QJsonDocument::Compact), true, true);

      return res;
      }

//---------------------------------------------------------
//   exportScoreMetadata
//---------------------------------------------------------

bool MuseScore::exportScoreMetadata(const QString& inFilePath, const QString& outFilePath)
      {
      std::unique_ptr<MasterScore> score(mscore->readScore(inFilePath));
      if (!score)
            return false;

      score->switchToPageMode();

      //// JSON specification ///////////////////////////
      //jsonForMedia["metadata"] = mdJson;
      ///////////////////////////////////////////////////

      CustomJsonWriter jsonWriter(outFilePath);

      //export metadata
      QJsonDocument doc(mscore->saveMetadataJSON(score.get()));
      jsonWriter.addKey("metadata");
      jsonWriter.addValue(doc.toJson(QJsonDocument::Compact), true, true);

      return true;
      }

//---------------------------------------------------------
//   exportTransposedScoreToJSON
//---------------------------------------------------------

bool MuseScore::exportTransposedScoreToJSON(const QString& inFilePath, const QString& transposeOptions, const QString& outFilePath)
      {
      QJsonDocument doc = QJsonDocument::fromJson(transposeOptions.toUtf8());
      if (!doc.isObject()) {
            qCritical("Transpose options JSON is not an object: %s", qUtf8Printable(transposeOptions));
            return false;
            }

      QJsonObject options = doc.object();

      TransposeMode mode;
      const QString modeName = options["mode"].toString();
      if (modeName == "by_key" || modeName == "to_key") // "by_key" for backwards compatibility
            mode = TransposeMode::TO_KEY;
      else if (modeName == "by_interval")
            mode = TransposeMode::BY_INTERVAL;
      else if (modeName == "diatonically")
            mode = TransposeMode::DIATONICALLY;
      else {
            qCritical("Transpose: invalid \"mode\" option: %s", qUtf8Printable(modeName));
            return false;
            }

      TransposeDirection direction;
      const QString directionName = options["direction"].toString();
      if (directionName == "up")
            direction = TransposeDirection::UP;
      else if (directionName == "down")
            direction = TransposeDirection::DOWN;
      else if (directionName == "closest")
            direction = TransposeDirection::CLOSEST;
      else {
            qCritical("Transpose: invalid \"direction\" option: %s", qUtf8Printable(directionName));
            return false;
            }

      constexpr int defaultKey = int(Key::INVALID);
      const Key targetKey = Key(options["targetKey"].toInt(defaultKey));
      if (mode == TransposeMode::TO_KEY) {
            const bool targetKeyValid = int(Key::MIN) <= int(targetKey) && int(targetKey) <= int(Key::MAX);
            if (!targetKeyValid) {
                  qCritical("Transpose: invalid targetKey: %d", int(targetKey));
                  return false;
                  }
            }

      const int transposeInterval = options["transposeInterval"].toInt(-1);
      if (mode != TransposeMode::TO_KEY) {
            const bool transposeIntervalValid = -1 < transposeInterval && transposeInterval < intervalListSize;
            if (!transposeIntervalValid) {
                  qCritical("Transpose: invalid transposeInterval: %d", transposeInterval);
                  return false;
                  }
            }

      const bool transposeKeySignatures = options["transposeKeySignatures"].toBool();
      const bool transposeChordNames = options["transposeChordNames"].toBool();
      const bool useDoubleSharpsFlats = options["useDoubleSharpsFlats"].toBool();

      std::unique_ptr<MasterScore> score(mscore->readScore(inFilePath));
      if (!score)
            return false;

      score->switchToPageMode();
      score->cmdSelectAll();

      score->startCmd();
      const bool transposed = score->transpose(mode, direction, targetKey, transposeInterval, transposeKeySignatures, transposeChordNames, useDoubleSharpsFlats);
      if (!transposed) {
            qCritical("Transposition failed");
            return false;
            }
      score->endCmd();

      bool res = true;
      CustomJsonWriter jsonWriter(outFilePath);

      // export mscz
      {
      jsonWriter.addKey("mscz");
      bool saved = false;
      QTemporaryFile tmpFile(QString("%1_transposed.XXXXXX.mscz").arg(score->title()));
      if (tmpFile.open()) {
            QString fileName = QFileInfo(tmpFile.fileName()).completeBaseName() + ".mscx";
            saved = score->Score::saveCompressedFile(&tmpFile, fileName, /* onlySelection */ false);
            tmpFile.close();
            tmpFile.open();
            jsonWriter.addValue(tmpFile.readAll().toBase64());
            tmpFile.close();
            }

      if (!saved) {
            qCritical("Transpose: adding mscz failed");
            jsonWriter.addValue("");
            res = false;
            }
      }

      // export score pdf
      jsonWriter.addKey("pdf");
      jsonWriter.addValue(exportPdfAsJSON(score.get()), /* lastJsonElement */ true);

      return res;
      }

bool MuseScore::updateSource(const QString& scorePath, const QString& newSource)
{
    MasterScore* score = mscore->readScore(scorePath);
    if (!score) {
        return false;
    }

    score->setMetaTag("source", newSource);

    return score->saveFile(false);
}
}
