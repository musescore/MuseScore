//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: file.cpp 5645 2012-05-18 13:34:03Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer et al.
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

/**
 File handling: loading and saving.
 */

#include "config.h"
#include "globals.h"
#include "musescore.h"
#include "scoreview.h"
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
#include "zarchive/zarchive.h"
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
#include "msynth/synti.h"
#include "svggenerator.h"

#ifdef OMR
#include "omr/omr.h"
#include "omr/omrpage.h"
#include "omr/importpdf.h"
#endif

#include "diff/diff_match_patch.h"
#include "libmscore/chordlist.h"
#include "libmscore/mscore.h"

//---------------------------------------------------------
//   paintElements
//---------------------------------------------------------

static void paintElements(QPainter& p, const QList<const Element*>& el)
      {
      foreach(const Element* e, el) {
            if (!e->visible())
                  continue;
            QPointF pos(e->pagePos());
            p.translate(pos);
            e->draw(&p);
            p.translate(-pos);
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
      fn = fn.replace(QChar(' '),  "_");
      fn = fn.replace(QChar('\n'), "_");
      fn = fn.replace(QChar(0xe4), "ae");
      fn = fn.replace(QChar(0xf6), "oe");
      fn = fn.replace(QChar(0xfc), "ue");
      fn = fn.replace(QChar(0xdf), "ss");
      fn = fn.replace(QChar(0xc4), "Ae");
      fn = fn.replace(QChar(0xd6), "Oe");
      fn = fn.replace(QChar(0xdc), "Ue");
      return fn;
      }

//---------------------------------------------------------
//   readScoreError
//---------------------------------------------------------

void MuseScore::readScoreError(const QString& name) const
      {
      QMessageBox::critical(0,
         tr("MuseScore: Load error"),
         QString(tr("Cannot read file: %1 error: %2").arg(name).arg(MScore::lastError))
         );
      }

//---------------------------------------------------------
//   load
///   Load file \a name.
///   Display message box with error if loading fails.
///   Return true if OK and false on error.
//---------------------------------------------------------

bool LoadFile::load(const QString& name)
      {
      if (name.isEmpty())
            return false;

      QFile fp(name);
      if (!fp.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(0,
               QWidget::tr("MuseScore: file not found:"),
               name,
               QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
            return false;
            }
      if (!loader(&fp)) {
            QMessageBox::warning(0,
               QWidget::tr("MuseScore: load failed:"),
               error,
               QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
            fp.close();
            return false;
            }
      fp.close();
      return true;
      }

//---------------------------------------------------------
//   checkDirty
//    if dirty, save score
//    return true on cancel
//---------------------------------------------------------

bool MuseScore::checkDirty(Score* s)
      {
      if (s->dirty()) {
            QMessageBox::StandardButton n = QMessageBox::warning(this, tr("MuseScore"),
               tr("Save changes to the score \"%1\"\n"
               "before closing?").arg(s->name()),
               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
               QMessageBox::Save);
            if (n == QMessageBox::Save) {
                  if (s->isSavable()) {
                        if (!s->saveFile())
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
         lastOpenPath,
#ifdef OMR
         tr("All Supported Files (*.mscz *.mscx *.xml *.mxl *.mid *.midi *.kar *.md *.mgu *.MGU *.sgu *.SGU *.cap *.pdf *.ove *.scw *.bww *.GTP *.GP3 *.GP4 *.GP5);;")+
#else
         tr("All Supported Files (*.mscz *.mscx *.xml *.mxl *.mid *.midi *.kar *.md *.mgu *.MGU *.sgu *.SGU *.cap *.ove *.scw *.bww *.GTP *.GP3 *.GP4 *.GP5);;")+
#endif
         tr("MuseScore Files (*.mscz *.mscx);;")+
         tr("MusicXML Files (*.xml *.mxl);;")+
         tr("MIDI Files (*.mid *.midi *.kar);;")+
         tr("Muse Data Files (*.md);;")+
         tr("Capella Files (*.cap);;")+
         tr("BB Files <experimental> (*.mgu *.MGU *.sgu *.SGU);;")+
#ifdef OMR
         tr("PDF Files <experimental omr> (*.pdf);;")+
#endif
         tr("Overture / Score Writer Files <experimental> (*.ove *.scw);;")+
         tr("Bagpipe Music Writer Files <experimental> (*.bww);;")+
         tr("Guitar Pro (*.GTP *.GP3 *.GP4 *.GP5);;")+
         tr("All Files (*)")
         );
      QStringList list = files;
      QStringList::Iterator it = list.begin();
      while(it != list.end()) {
            openScore(*it);
            ++it;
            }
      }

void MuseScore::openScore(const QString& fn)
      {
      if (fn.isEmpty())
            return;
            Score* score = new Score(MScore::defaultStyle());
      if (readScore(score, fn)) {
            setCurrentScoreView(appendScore(score));
            lastOpenPath = score->fileInfo()->path();
            updateRecentScores(score);
            writeSessionFile(false);
            }
      else {
            delete score;
            readScoreError(fn);
            }
      }

//---------------------------------------------------------
//   saveFile
//    return true on success
//---------------------------------------------------------

/**
 Save the current score.
 Handles the GUI's file-save action.
 */

void MuseScore::saveFile()
      {
      if (cs == 0)
            return;
      cs->setSyntiState(synti->state());
      if (cs->created()) {
            QString selectedFilter;
            QString fn = cs->fileInfo()->fileName();
            Text* t = cs->getText(TEXT_TITLE);
            if (t)
                  fn = t->getText();
            QString name = createDefaultFileName(fn);
            QString f1 = tr("Compressed MuseScore File (*.mscz)");
            QString f2 = tr("MuseScore File (*.mscx)");

            QString fname   = name + ".mscz";
            QString filter = f1 + ";;" + f2;
            fn = mscore->getSaveScoreName(
               tr("MuseScore: Save Score"),
               fname,
               filter,
               &selectedFilter
               );
            if (fn.isEmpty())
                  return;
            cs->fileInfo()->setFile(fn);
            updateRecentScores(cs);
            cs->setCreated(false);
            writeSessionFile(false);
            }
      if (!cs->saveFile()) {
            QMessageBox::critical(mscore, tr("MuseScore: Save File"), MScore::lastError);
            return;
            }
      setWindowTitle("MuseScore: " + cs->name());
      int idx = scoreList.indexOf(cs);
      tab1->setTabText(idx, cs->name());
      if (tab2)
            tab2->setTabText(idx, cs->name());
      QString tmp = cs->tmpName();
      if (!tmp.isEmpty()) {
            QFile f(tmp);
            if (!f.remove())
                  qDebug("cannot remove temporary file <%s>\n", qPrintable(f.fileName()));
            cs->setTmpName("");
            }
      writeSessionFile(false);
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

//---------------------------------------------------------
//   newFile
//    create new score
//---------------------------------------------------------

void MuseScore::newFile()
      {
      if (newWizard == 0)
            newWizard = new NewWizard(this);
      newWizard->restart();
      if (newWizard->exec() != QDialog::Accepted)
            return;
      int pickupTimesigZ, pickupTimesigN;
      int measures       = newWizard->measures();
      Fraction timesig   = newWizard->timesig();
      TimeSigType timesigType = newWizard->timesigType();

      bool pickupMeasure = newWizard->pickupMeasure(&pickupTimesigZ, &pickupTimesigN);
      if (pickupMeasure)
            measures += 1;
      KeySigEvent ks     = newWizard->keysig();

      Score* score = new Score(MScore::defaultStyle());

      //
      //  create score from template
      //
      if (newWizard->useTemplate()) {
            if (!readScore(score, newWizard->templatePath())) {
                  readScoreError(newWizard->templatePath());
                  delete score;
                  return;
                  }
            score->setCreated(true);
            score->fileInfo()->setFile(createDefaultName());

            int m = 0;
            for (Measure* mb = score->firstMeasure(); mb; mb = mb->nextMeasure()) {
                  if (mb->type() == MEASURE)
                        ++m;
                  }
            //
            // remove all notes & rests
            //
            score->deselectAll();
            for (Segment* s = score->firstMeasure()->first(); s;) {
                  Segment* ns = s->next1();
                  if (s->subtype() == SegChordRest && s->tick() == 0) {
                        int tracks = s->elist().size();
                        for (int track = 0; track < tracks; ++track) {
                              delete s->element(track);
                              s->setElement(track, 0);
                              }
                        }
                  else if (
                     (s->subtype() == SegChordRest)
//                     || (s->subtype() == SegClef)
                     || (s->subtype() == SegKeySig)
                     || (s->subtype() == SegGrace)
                     || (s->subtype() == SegBreath)
                     ) {
                        s->measure()->remove(s);
                        delete s;
                        }
                  s = ns;
                  }
            }
      //
      //  create new score from scratch
      //
      else {
            score->setCreated(true);
            score->fileInfo()->setFile(createDefaultName());
            newWizard->createInstruments(score);
            }
      if (!newWizard->title().isEmpty())
            score->fileInfo()->setFile(newWizard->title());
      Measure* pm = score->firstMeasure();
      for (int i = 0; i < measures; ++i) {
            Measure* m;
            if (pm) {
                  m  = pm;
                  pm = pm->nextMeasure();
                  }
            else {
                  m = new Measure(score);
                  score->measures()->add(m);
                  }
            m->setTimesig(timesig);
            m->setLen(timesig);
            if (pickupMeasure) {
                  if (i == 0) {
                        m->setIrregular(true);        // dont count pickup measure
                        m->setLen(Fraction(pickupTimesigZ, pickupTimesigN));
                        }
                  else if (i == (measures - 1)) {
                        // last measure is shorter
                        m->setLen(timesig - Fraction(pickupTimesigZ, pickupTimesigN));
                        }
                  }
            m->setEndBarLineType(i == (measures - 1) ? END_BAR : NORMAL_BAR, false);
            }

      int tick = 0;
      for (MeasureBase* mb = score->measures()->first(); mb; mb = mb->next()) {
            mb->setTick(tick);
            if (mb->type() != MEASURE)
                  continue;
            Measure* measure = static_cast<Measure*>(mb);
            int ticks = measure->ticks();
	      for (int staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
                  Staff* staff = score->staff(staffIdx);
                  if (tick == 0) {
                        if (!staff->useTablature()) {
                              TimeSig* ts = new TimeSig(score);
                              ts->setSig(timesig);
                              ts->setSubtype(timesigType);
                              ts->setTrack(staffIdx * VOICES);
                              Segment* s = measure->getSegment(ts, 0);
                              s->add(ts);
                              }
                        Part* part = staff->part();
                        if (!part->instr()->useDrumset()) {
                              //
                              // transpose key
                              //
                              KeySigEvent nKey = ks;
                              if (part->instr()->transpose().chromatic && !newWizard->useTemplate()) {
                                    int diff = -part->instr()->transpose().chromatic;
                                    nKey.setAccidentalType(transposeKey(nKey.accidentalType(), diff));
                                    }
                              (*(staff->keymap()))[0] = nKey;
                              KeySig* keysig = new KeySig(score);
                              keysig->setTrack(staffIdx * VOICES);
                              keysig->setKeySigEvent(nKey);
                              Segment* s = measure->getSegment(keysig, 0);
                              s->add(keysig);
                              }
                        }
                  if (staff->primaryStaff()) {
                        if (measure->timesig() != measure->len()) {
                              int tick = measure->tick();
                              QList<TDuration> dList = toDurationList(measure->len(), false);
                              if (!dList.isEmpty()) {
                                    foreach(TDuration d, dList) {
		                              Rest* rest = new Rest(score, d);
                                          rest->setDuration(d.fraction());
            	                        rest->setTrack(staffIdx * VOICES);
	            	                  Segment* s = measure->getSegment(rest, tick);
		                              s->add(rest);
                                          tick += rest->actualTicks();
                                          }
                                    }
                              }
                        else {
		                  Rest* rest = new Rest(score, TDuration(TDuration::V_MEASURE));
                              rest->setDuration(measure->len());
            	            rest->setTrack(staffIdx * VOICES);
	            	      Segment* s = measure->getSegment(rest, tick);
		                  s->add(rest);
                              }
                        }
                  }
            tick += ticks;
            }
      score->fixTicks();
      //
      // ceate linked staves
      //
      foreach(Staff* staff, score->staves()) {
            if (!staff->linkedStaves())
                  continue;
            foreach(Staff* lstaff, staff->linkedStaves()->staves()) {
                  if (staff != lstaff) {
                        cloneStaff(staff, lstaff);
                        }
                  }
            }
      //
      // select first rest
      //
      Measure* m = score->firstMeasure();
      for (Segment* s = m->first(); s; s = s->next()) {
            if (s->subtype() == SegChordRest) {
                  if (s->element(0)) {
                        score->select(s->element(0), SELECT_SINGLE, 0);
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
            if (measure->type() != VBOX) {
                  MeasureBase* nm = new VBox(score);
                  nm->setTick(0);
                  nm->setNext(measure);
                  score->measures()->add(nm);
                  measure = nm;
                  }
            if (!title.isEmpty()) {
                  Text* s = new Text(score);
                  s->setTextStyleType(TEXT_STYLE_TITLE);
                  s->setText(title);
                  measure->add(s);
                  score->setMetaTag("workTitle", title);
                  }
            if (!subtitle.isEmpty()) {
                  Text* s = new Text(score);
                  s->setTextStyleType(TEXT_STYLE_SUBTITLE);
                  s->setText(subtitle);
                  measure->add(s);
                  }
            if (!composer.isEmpty()) {
                  Text* s = new Text(score);
                  s->setTextStyleType(TEXT_STYLE_COMPOSER);
                  s->setText(composer);
                  measure->add(s);
                  }
            if (!poet.isEmpty()) {
                  Text* s = new Text(score);
                  s->setTextStyleType(TEXT_STYLE_POET);
                  s->setText(poet);
                  measure->add(s);
                  }
            }
      if (newWizard->createTempo()) {
            double tempo = newWizard->tempo();
            TempoText* tt = new TempoText(score);

            int uc = 0x1d15f;
            QChar h(QChar::highSurrogate(uc));
            QChar l(QChar::lowSurrogate(uc));
            tt->setText(QString("%1%2 = %3").arg(h).arg(l).arg(tempo));
            tempo /= 60;      // bpm -> bps

            tt->setTempo(tempo);
            tt->setTrack(0);
            Segment* seg = score->firstMeasure()->first(SegChordRest);
            seg->add(tt);
            score->setTempo(0, tempo);
            }
      if (!copyright.isEmpty())
            score->setMetaTag("copyright", copyright);

      score->syntiState().prepend(SyntiParameter("soundfont", MScore::soundFont));

      score->rebuildMidiMapping();
      score->doLayout();
      setCurrentScoreView(appendScore(score));
      }

//---------------------------------------------------------
//   getOpenScoreNames
//---------------------------------------------------------

QStringList MuseScore::getOpenScoreNames(QString& dir, const QString& filter)
      {
      if (preferences.nativeDialogs) {
            return QFileDialog::getOpenFileNames(this,
               tr("MuseScore: Load Score"), dir, filter);
            }
      QFileInfo myScores(preferences.myScoresPath);
      if (myScores.isRelative())
            myScores.setFile(QDir::home(), preferences.myScoresPath);
      if (loadScoreDialog == 0) {
            loadScoreDialog = new QFileDialog(this);
            loadScoreDialog->setFileMode(QFileDialog::ExistingFiles);
            loadScoreDialog->setOption(QFileDialog::DontUseNativeDialog, true);
            loadScoreDialog->setWindowTitle(tr("MuseScore: Load Score"));

            QSettings settings;
            loadScoreDialog->restoreState(settings.value("loadScoreDialog").toByteArray());
            loadScoreDialog->setAcceptMode(QFileDialog::AcceptOpen);
            }
      // setup side bar urls
      QList<QUrl> urls;
      QString home = QDir::homePath();
      urls.append(QUrl::fromLocalFile(home));
      urls.append(QUrl::fromLocalFile(myScores.absoluteFilePath()));
      urls.append(QUrl::fromLocalFile(QDir::currentPath()));
      urls.append(QUrl::fromLocalFile(mscoreGlobalShare+"/demos"));
      loadScoreDialog->setSidebarUrls(urls);

      loadScoreDialog->setNameFilter(filter);
      loadScoreDialog->setDirectory(dir);

      QStringList result;
      if (loadScoreDialog->exec()) {
            result = loadScoreDialog->selectedFiles();
            return result;
            }
      return QStringList();
      }

//---------------------------------------------------------
//   getSaveScoreName
//---------------------------------------------------------
QString MuseScore::getSaveScoreName(const QString& title,
   QString& name, const QString& filter, QString* selectedFilter)
      {
      return getSaveScoreName(title, name, filter, selectedFilter, false);
}

QString MuseScore::getSaveScoreName(const QString& title,
   QString& name, const QString& filter, QString* selectedFilter, bool selectFolder)
      {
      if (preferences.nativeDialogs) {
	    if (!selectFolder) {
		    QString fn = QFileDialog::getSaveFileName(this,
				    title,
				    name,
				    filter,
				    selectedFilter
				    );
		    return fn;
                    } else {
		    QString fn = QFileDialog::getSaveFileName(this,
				    title,
				    name,
				    filter,
				    selectedFilter,
				    QFileDialog::ShowDirsOnly
				    );
		    return fn;
	    	   }
            }

      QFileInfo myScores(preferences.myScoresPath);
      if (myScores.isRelative())
            myScores.setFile(QDir::home(), preferences.myScoresPath);
      if (saveScoreDialog == 0) {
            saveScoreDialog = new QFileDialog(this);
            QSettings settings;
            saveScoreDialog->restoreState(settings.value("saveScoreDialog").toByteArray());
            saveScoreDialog->setFileMode(QFileDialog::AnyFile);
            saveScoreDialog->setOption(QFileDialog::DontConfirmOverwrite, false);
            saveScoreDialog->setOption(QFileDialog::DontUseNativeDialog, true);
            saveScoreDialog->setAcceptMode(QFileDialog::AcceptSave);
            }
      if (selectFolder) {
            saveScoreDialog->setFileMode(QFileDialog::Directory);
      }
      // setup side bar urls
      QList<QUrl> urls;
      QString home = QDir::homePath();
      urls.append(QUrl::fromLocalFile(home));
      urls.append(QUrl::fromLocalFile(myScores.absoluteFilePath()));
      urls.append(QUrl::fromLocalFile(QDir::currentPath()));
      saveScoreDialog->setSidebarUrls(urls);

      saveScoreDialog->setWindowTitle(title);
      saveScoreDialog->setNameFilter(filter);
      // saveScoreDialog->setDirectory(name);
      saveScoreDialog->selectFile(name);
      QStringList result;

      if (!selectFolder) {
            connect(saveScoreDialog, SIGNAL(filterSelected(const QString&)),
               SLOT(saveScoreDialogFilterSelected(const QString&)));
      }
      if (saveScoreDialog->exec()) {
            result = saveScoreDialog->selectedFiles();
            *selectedFilter = saveScoreDialog->selectedNameFilter();
            return result.front();
            }
      return QString();
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
      QString currentPath = QDir::currentPath();
      if (preferences.nativeDialogs) {
            QString fn;
            if (open) {
                  fn = QFileDialog::getOpenFileName(
                     this, tr("MuseScore: Load Style"),
                     currentPath,
                     tr("MuseScore Styles (*.mss);;" "All Files (*)")
                     );
                  }
            else {
                  fn = QFileDialog::getSaveFileName(
                     this, tr("MuseScore: Save Style"),
                     currentPath,
                     tr("MuseScore Style File (*.mss)")
                     );
                  }
            return fn;
            }

      QFileInfo myStyles(preferences.myStylesPath);
      if (myStyles.isRelative())
            myStyles.setFile(QDir::home(), preferences.myStylesPath);
      QFileDialog* dialog;
      QList<QUrl> urls;
      QString home = QDir::homePath();
      urls.append(QUrl::fromLocalFile(home));
      urls.append(QUrl::fromLocalFile(myStyles.absoluteFilePath()));
      urls.append(QUrl::fromLocalFile(QDir::currentPath()));

      if (open) {
            if (loadStyleDialog == 0) {
                  loadStyleDialog = new QFileDialog(this);
                  loadStyleDialog->setFileMode(QFileDialog::ExistingFile);
                  loadStyleDialog->setOption(QFileDialog::DontUseNativeDialog, true);
                  loadStyleDialog->setWindowTitle(title.isEmpty() ? tr("MuseScore: Load Style") : title);
                  loadStyleDialog->setNameFilter(tr("MuseScore Style File (*.mss)"));
                  loadStyleDialog->setDirectory(currentPath);

                  QSettings settings;
                  loadStyleDialog->restoreState(settings.value("loadStyleDialog").toByteArray());
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
                  saveStyleDialog->setWindowTitle(title.isEmpty() ? tr("MuseScore: Save Style") : title);
                  saveStyleDialog->setNameFilter(tr("MuseScore Style File (*.mss)"));
                  saveStyleDialog->setDirectory(currentPath);

                  QSettings settings;
                  saveStyleDialog->restoreState(settings.value("saveStyleDialog").toByteArray());
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
//   getSoundFont
//---------------------------------------------------------

QStringList MuseScore::getSoundFont(const QString& d)
      {
      QString filter = tr("SoundFont Files (*.sf2 *.SF2);;All (*)");

      if (preferences.nativeDialogs) {
             QStringList s = QFileDialog::getOpenFileNames(
               mscore,
               MuseScore::tr("Choose Synthesizer SoundFont"),
               d,
               filter
               );
            return s;
            }

      if (loadSoundFontDialog == 0) {
            loadSoundFontDialog = new QFileDialog(this);
            loadSoundFontDialog->setFileMode(QFileDialog::ExistingFiles);
            loadSoundFontDialog->setOption(QFileDialog::DontUseNativeDialog, true);
            loadSoundFontDialog->setWindowTitle(tr("MuseScore: Choose Synthesizer SoundFont"));
            loadSoundFontDialog->setNameFilter(filter);
            loadSoundFontDialog->setDirectory(d);

            QSettings settings;
            loadSoundFontDialog->restoreState(settings.value("loadSoundFontDialog").toByteArray());
            loadSoundFontDialog->setAcceptMode(QFileDialog::AcceptOpen);
            }

      //
      // setup side bar urls
      //
      QFileInfo mySoundFonts(preferences.mySoundFontsPath);
      if (mySoundFonts.isRelative())
            mySoundFonts.setFile(QDir::home(), preferences.mySoundFontsPath);

      QList<QUrl> urls;
      QString home = QDir::homePath();
      urls.append(QUrl::fromLocalFile(home));
      urls.append(QUrl::fromLocalFile(mySoundFonts.absoluteFilePath()));
      urls.append(QUrl::fromLocalFile(QDir::currentPath()));
      urls.append(QUrl::fromLocalFile(mscoreGlobalShare+"/sound"));
      loadSoundFontDialog->setSidebarUrls(urls);

      if (loadSoundFontDialog->exec()) {
            QStringList result = loadSoundFontDialog->selectedFiles();
            return result;
            }
      return QStringList();
      }

//---------------------------------------------------------
//   getChordStyleFilename
//---------------------------------------------------------

QString MuseScore::getChordStyleFilename(bool open)
      {
      QString filter = tr("MuseScore Chord Style File (*.xml)");
      if (open)
            filter.append(tr(";;All Files (*)"));

      QString currentPath = QDir::currentPath();
      if (preferences.nativeDialogs) {
            QString fn;
            if (open) {
                  fn = QFileDialog::getOpenFileName(
                     this, tr("MuseScore: Load Chord Style"),
                     QString(currentPath),
                     filter
                     );
                  }
            else {
                  fn = QFileDialog::getSaveFileName(
                     this, tr("MuseScore: Save Chord Style"),
                     QString(currentPath),
                     filter
                     );
                  }
            return fn;
            }

      QFileInfo myStyles(preferences.myStylesPath);
      if (myStyles.isRelative())
            myStyles.setFile(QDir::home(), preferences.myStylesPath);
      QFileDialog* dialog;
      QList<QUrl> urls;
      QString home = QDir::homePath();
      urls.append(QUrl::fromLocalFile(home));
      urls.append(QUrl::fromLocalFile(myStyles.absoluteFilePath()));
      urls.append(QUrl::fromLocalFile(QDir::currentPath()));

      QSettings settings;
      if (open) {
            if (loadChordStyleDialog == 0) {
                  loadChordStyleDialog = new QFileDialog(this);
                  loadChordStyleDialog->setFileMode(QFileDialog::ExistingFile);
                  loadChordStyleDialog->setOption(QFileDialog::DontUseNativeDialog, true);
                  loadChordStyleDialog->setWindowTitle(tr("MuseScore: Load Chord Style"));
                  loadChordStyleDialog->setNameFilter(filter);
                  loadChordStyleDialog->setDirectory(currentPath);

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
                  saveChordStyleDialog->setWindowTitle(tr("MuseScore: Save Style"));
                  saveChordStyleDialog->setNameFilter(filter);
                  saveChordStyleDialog->setDirectory(currentPath);

                  saveChordStyleDialog->restoreState(settings.value("saveChordStyleDialog").toByteArray());
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
      QString filter = tr("PDF Scan File (*.pdf);;All (*)");

      if (preferences.nativeDialogs) {
            QString s = QFileDialog::getOpenFileName(
               mscore,
               MuseScore::tr("Choose PDF Scan"),
               d,
               filter
               );
            return s;
            }

      if (loadScanDialog == 0) {
            loadScanDialog = new QFileDialog(this);
            loadScanDialog->setFileMode(QFileDialog::ExistingFile);
            loadScanDialog->setOption(QFileDialog::DontUseNativeDialog, true);
            loadScanDialog->setWindowTitle(tr("MuseScore: Choose PDF Scan"));
            loadScanDialog->setNameFilter(filter);
            loadScanDialog->setDirectory(d);

            QSettings settings;
            loadScanDialog->restoreState(settings.value("loadScanDialog").toByteArray());
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
      QString filter = tr("OGG Audio File (*.ogg);;All (*)");

      if (preferences.nativeDialogs) {
            QString s = QFileDialog::getOpenFileName(
               mscore,
               MuseScore::tr("Choose Audio File"),
               d,
               filter
               );
            return s;
            }

      if (loadAudioDialog == 0) {
            loadAudioDialog = new QFileDialog(this);
            loadAudioDialog->setFileMode(QFileDialog::ExistingFile);
            loadAudioDialog->setOption(QFileDialog::DontUseNativeDialog, true);
            loadAudioDialog->setWindowTitle(tr("MuseScore: Choose OGG Audio File"));
            loadAudioDialog->setNameFilter(filter);
            loadAudioDialog->setDirectory(d);

            QSettings settings;
            loadAudioDialog->restoreState(settings.value("loadAudioDialog").toByteArray());
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

QString MuseScore::getFotoFilename()
      {
      QString filter =
         tr("PNG Bitmap Graphic (*.png);;")+
         tr("PDF File (*.pdf);;")+
         tr("Encapsulated PostScript File (*.eps);;")+
         tr("Scalable Vector Graphic (*.svg);;");

      QString title       = tr("MuseScore: Save Image");
      QString currentPath = QDir::currentPath();
      if (preferences.nativeDialogs) {
            QString fn;
            fn = QFileDialog::getSaveFileName(
               this,
               title,
               currentPath,
               filter
               );
            return fn;
            }

      QFileInfo myImages(preferences.myImagesPath);
      if (myImages.isRelative())
            myImages.setFile(QDir::home(), preferences.myImagesPath);
      QList<QUrl> urls;
      QString home = QDir::homePath();
      urls.append(QUrl::fromLocalFile(home));
      urls.append(QUrl::fromLocalFile(myImages.absoluteFilePath()));
      urls.append(QUrl::fromLocalFile(QDir::currentPath()));

      if (saveImageDialog == 0) {
            saveImageDialog = new QFileDialog(this);
            saveImageDialog->setFileMode(QFileDialog::AnyFile);
            saveImageDialog->setAcceptMode(QFileDialog::AcceptSave);
            saveImageDialog->setOption(QFileDialog::DontConfirmOverwrite, false);
            saveImageDialog->setOption(QFileDialog::DontUseNativeDialog, true);
            saveImageDialog->setWindowTitle(title);
            saveImageDialog->setNameFilter(filter);
            saveImageDialog->setDirectory(currentPath);

            QSettings settings;
            saveImageDialog->restoreState(settings.value("saveImageDialog").toByteArray());
            saveImageDialog->setAcceptMode(QFileDialog::AcceptSave);
            }

      // setup side bar urls
      saveImageDialog->setSidebarUrls(urls);

      if (saveImageDialog->exec()) {
            QStringList result = saveImageDialog->selectedFiles();
            return result.front();
            }
      return QString();
      }

//---------------------------------------------------------
//   getPaletteFilename
//---------------------------------------------------------

QString MuseScore::getPaletteFilename(bool open)
      {
      QString title;
      QString filter;
      if (open) {
            title  = tr("MuseScore: Load Palette");
            filter = tr("MuseScore Palette (*.mpal);;All Files (*)");
            }
      else {
            title  = tr("MuseScore: Save Palette");
            filter = tr("MuseScore Palette (*.mpal)");

            // create dataPath/profiles if it does not exist
            QDir dir;
            dir.mkpath(dataPath);
            QString path = dataPath + "/profiles";
            dir.mkpath(path);
            }

      QString currentPath(QDir::currentPath());
      if (preferences.nativeDialogs) {
            QString fn;
            if (open)
                  fn = QFileDialog::getOpenFileName(this, title, currentPath, filter);
            else
                  fn = QFileDialog::getSaveFileName(this, title, currentPath, filter);
            return fn;
            }

      QFileInfo myPalettes(dataPath + "/profiles");
      QFileDialog* dialog;
      QList<QUrl> urls;
      urls.append(QUrl::fromLocalFile(QDir::homePath()));
      urls.append(QUrl::fromLocalFile(QDir::currentPath()));
      urls.append(QUrl::fromLocalFile(myPalettes.absoluteFilePath()));

      if (open) {
            if (loadPaletteDialog == 0) {
                  loadPaletteDialog = new QFileDialog(this);
                  loadPaletteDialog->setFileMode(QFileDialog::ExistingFile);
                  loadPaletteDialog->setOption(QFileDialog::DontUseNativeDialog, true);
                  loadPaletteDialog->setDirectory(myPalettes.absoluteFilePath());

                  QSettings settings;
                  loadPaletteDialog->restoreState(settings.value("loadPaletteDialog").toByteArray());
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
                  savePaletteDialog->setDirectory(myPalettes.absoluteFilePath());

                  QSettings settings;
                  savePaletteDialog->restoreState(settings.value("savePaletteDialog").toByteArray());
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
//   getDrumsetFilename
//---------------------------------------------------------

QString MuseScore::getDrumsetFilename(bool open)
      {
      QString title;
      QString filter;
      if (open) {
            title  = tr("MuseScore: Load Drumset");
            filter = tr("MuseScore Drumset (*.drm);;All Files (*)");
            }
      else {
            title  = tr("MuseScore: Save Drumset");
            filter = tr("MuseScore Drumset File (*.drm)");
            }

      QString currentPath(QDir::currentPath());
      if (preferences.nativeDialogs) {
            QString fn;
            if (open)
                  fn = QFileDialog::getOpenFileName(this, title, currentPath, filter);
            else
                  fn = QFileDialog::getSaveFileName(this, title, currentPath, filter);
            return fn;
            }

      QFileInfo myStyles(preferences.myStylesPath);
      if (myStyles.isRelative())
            myStyles.setFile(QDir::home(), preferences.myStylesPath);
      QFileDialog* dialog;
      QList<QUrl> urls;
      QString home = QDir::homePath();
      urls.append(QUrl::fromLocalFile(home));
      urls.append(QUrl::fromLocalFile(myStyles.absoluteFilePath()));
      urls.append(QUrl::fromLocalFile(QDir::currentPath()));

      if (open) {
            if (loadDrumsetDialog == 0) {
                  loadDrumsetDialog = new QFileDialog(this);
                  loadDrumsetDialog->setFileMode(QFileDialog::ExistingFile);
                  loadDrumsetDialog->setOption(QFileDialog::DontUseNativeDialog, true);
                  loadDrumsetDialog->setDirectory(currentPath);

                  QSettings settings;
                  loadDrumsetDialog->restoreState(settings.value("loadDrumsetDialog").toByteArray());
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
                  saveDrumsetDialog->setDirectory(currentPath);

                  QSettings settings;
                  saveDrumsetDialog->restoreState(settings.value("saveDrumsetDialog").toByteArray());
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
      QPrinter printerDev(QPrinter::HighResolution);
      const PageFormat* pf = cs->pageFormat();
      printerDev.setPaperSize(pf->size(), QPrinter::Inch);

      printerDev.setCreator("MuseScore Version: " VERSION);
      printerDev.setFullPage(true);
      printerDev.setColorMode(QPrinter::Color);
      printerDev.setDocName(cs->name());
      printerDev.setDoubleSidedPrinting(pf->twosided());
      printerDev.setOutputFormat(QPrinter::NativeFormat);

#if defined(Q_WS_MAC) || defined(__MINGW32__)
      printerDev.setOutputFileName("");
#else
      // when setting this on windows platform, pd.exec() does not
      // show dialog
      printerDev.setOutputFileName(cs->fileInfo()->path() + "/" + cs->name() + ".pdf");
#endif

      QPrintDialog pd(&printerDev, 0);
      if (!pd.exec())
            return;
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
      }

//---------------------------------------------------------
//   exportFile
//    return true on success
//---------------------------------------------------------

bool MuseScore::exportFile()
      {
      QStringList fl;
      fl.append(tr("Uncompressed MuseScore Format (*.mscx)"));
      fl.append(tr("MusicXML Format (*.xml)"));
      fl.append(tr("Compressed MusicXML Format (*.mxl)"));
      fl.append(tr("Standard MIDI File (*.mid)"));
      fl.append(tr("PDF File (*.pdf)"));
      fl.append(tr("PostScript File (*.ps)"));
      fl.append(tr("PNG Bitmap Graphic (*.png)"));
      fl.append(tr("Scalable Vector Graphic (*.svg)"));
      fl.append(tr("LilyPond Format (*.ly)"));
#ifdef HAS_AUDIOFILE
      fl.append(tr("Wave Audio (*.wav)"));
      fl.append(tr("Flac Audio (*.flac)"));
      fl.append(tr("Ogg Vorbis Audio (*.ogg)"));
#endif
      fl.append(tr("MP3 Audio (*.mp3)"));
      QString saveDialogTitle = tr("MuseScore: Export");

      QSettings settings;
      if (lastSaveCopyDirectory.isEmpty())
            lastSaveCopyDirectory = settings.value("lastSaveCopyDirectory", preferences.myScoresPath).toString();
      if (lastSaveDirectory.isEmpty())
            lastSaveDirectory = settings.value("lastSaveDirectory", preferences.myScoresPath).toString();
      QString saveDirectory = lastSaveCopyDirectory;

      if (saveDirectory.isEmpty()) {
            saveDirectory = preferences.myScoresPath;
            }

      QString selectedFilter;
      QString name   = QString("%1.mscx").arg(cs->name());
      QString filter = fl.join(";;");
      QString fn = getSaveScoreName(saveDialogTitle, name, filter, &selectedFilter);
      if (fn.isEmpty())
            return false;

      QFileInfo fi(fn);
      lastSaveCopyDirectory = fi.absolutePath();

      QString ext;
      if (selectedFilter.isEmpty())
            ext = fi.suffix();
      else {
            int idx = fl.indexOf(selectedFilter);
            if (idx != -1) {
                  static const char* extensions[] = {
                        "mscx", "xml", "mxl", "mid", "pdf", "ps", "png", "svg", "ly",
#ifdef HAS_AUDIOFILE
                        "wav", "flac", "ogg",
#endif
                        "mp3"
                        };
                  ext = extensions[idx];
                  }
            }
      if (ext.isEmpty()) {
            QMessageBox::critical(this, tr("MuseScore: Save As"), tr("cannot determine file type"));
            return false;
            }

      if (fi.suffix() != ext)
            fn += "." + ext;
      return saveAs(cs, true, fn, ext);
      }

//---------------------------------------------------------
//   exportParts
//    return true on success
//---------------------------------------------------------

bool MuseScore::exportParts()
      {
      QStringList fl;
      fl.append(tr("Uncompressed MuseScore Format (*.mscx)"));
      fl.append(tr("MusicXML Format (*.xml)"));
      fl.append(tr("Compressed MusicXML Format (*.mxl)"));
      fl.append(tr("Standard MIDI File (*.mid)"));
      fl.append(tr("PDF File (*.pdf)"));
      fl.append(tr("PostScript File (*.ps)"));
      fl.append(tr("PNG Bitmap Graphic (*.png)"));
      fl.append(tr("Scalable Vector Graphic (*.svg)"));
      fl.append(tr("LilyPond Format (*.ly)"));
#ifdef HAS_AUDIOFILE
      fl.append(tr("Wave Audio (*.wav)"));
      fl.append(tr("Flac Audio (*.flac)"));
      fl.append(tr("Ogg Vorbis Audio (*.ogg)"));
#endif
      fl.append(tr("MP3 Audio (*.mp3)"));
      QString saveDialogTitle = tr("MuseScore: Export Parts");

      QSettings settings;
      if (lastSaveCopyDirectory.isEmpty())
	    lastSaveCopyDirectory = settings.value("lastSaveCopyDirectory", preferences.myScoresPath).toString();
      if (lastSaveDirectory.isEmpty())
	    lastSaveDirectory = settings.value("lastSaveDirectory", preferences.myScoresPath).toString();
      QString saveDirectory = lastSaveCopyDirectory;

      if (saveDirectory.isEmpty()) {
	    saveDirectory = preferences.myScoresPath;
	    }

      QString selectedFilter;
      QString name = QString("");
      QString filter = fl.join(";;");
      QString fn = getSaveScoreName(saveDialogTitle, name, filter, &selectedFilter, true);
      if (fn.isEmpty())
	    return false;

      lastSaveCopyDirectory = fn;

      Score* thisScore = cs;
      if (thisScore->parentScore())
            thisScore = thisScore->parentScore();

      foreach(Excerpt* e, thisScore->excerpts())  {
	      Score* pScore = e->score();
            QString partfn = fn + QDir::separator() + thisScore->name() + "-" + pScore->name();
	      QFileInfo fi(partfn);
	      QString ext;
            if (selectedFilter.isEmpty())
		      ext = fi.suffix();
            else {
		      int idx = fl.indexOf(selectedFilter);
		      if (idx != -1) {
			      static const char* extensions[] = {
				      "mscx", "xml", "mxl", "mid", "pdf", "ps", "png", "svg", "ly",
#ifdef HAS_AUDIOFILE
				      "wav", "flac", "ogg",
#endif
				      "mp3"
			            };
                        ext = extensions[idx];
		            }
	            }
            if (ext.isEmpty()) {
                  QMessageBox::critical(this, tr("MuseScore: Export Parts"), tr("cannot determine file type"));
                  return false;
                  }

            if (fi.suffix() != ext)
                  partfn += "." + ext;

            if (!saveAs(pScore, true, partfn, ext))
                  return false;
            }
      QMessageBox::information(this, tr("MuseScore: Export Parts"), tr("Parts were successfully exported"));
      return true;
      }

//---------------------------------------------------------
//   saveAs
//---------------------------------------------------------

bool MuseScore::saveAs(Score* cs, bool saveCopy, const QString& path, const QString& ext)
      {
      cs->setSyntiState(synti->state());

      bool rv = false;
      QString suffix = "." + ext;
      QString fn(path);
      if (!fn.endsWith(suffix))
            fn += suffix;
      if (ext == "mscx" || ext == "mscz") {
            // save as mscore *.msc[xz] file
            QFileInfo fi(fn);
            rv = true;
            try {
                  if (ext == "mscz")
                        cs->saveCompressedFile(fi, false);
                  else
                        cs->saveFile(fi);
                  }
            catch (QString s) {
                  rv = false;
                  QMessageBox::critical(this, tr("MuseScore: Save As"), s);
                  }
            if (rv && !saveCopy) {
                  cs->fileInfo()->setFile(fn);
                  setWindowTitle("MuseScore: " + cs->name());
                  cs->undo()->setClean();
                  dirtyChanged(cs);
                  cs->setCreated(false);
                  updateRecentScores(cs);
                  writeSessionFile(false);
                  }
            }
      else if (ext == "xml") {
            // save as MusicXML *.xml file
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
            rv = savePsPdf(cs, fn, QPrinter::PdfFormat);
            }
      else if (ext == "ps") {
            // save as postscript file *.ps
            rv = savePsPdf(cs, fn, QPrinter::PostScriptFormat);
            }
      else if (ext == "png") {
            // save as png file *.png
            rv = savePng(cs, fn);
            }
      else if (ext == "svg") {
            // save as svg file *.svg
            rv = saveSvg(cs, fn);
            }
      else if (ext == "ly") {
            // save as lilypond file *.ly
            rv = saveLilypond(cs, fn);
            }
#ifdef HAS_AUDIOFILE
      else if (ext == "wav" || ext == "flac" || ext == "ogg")
            rv = saveAudio(cs, fn, ext);
#endif
      else if (ext == "mp3")
            rv = saveMp3(cs, fn);
      else {
            qDebug("internal error: unsupported extension <%s>\n",
               qPrintable(ext));
            return false;
            }
      return rv;
      }

//---------------------------------------------------------
//   savePsPdf
//---------------------------------------------------------

bool MuseScore::savePsPdf(const QString& saveName, QPrinter::OutputFormat format)
      {
      return savePsPdf(cs, saveName, format);
      }

bool MuseScore::savePsPdf(Score* cs, const QString& saveName, QPrinter::OutputFormat format)
      {
      QPrinter printerDev(QPrinter::HighResolution);
      const PageFormat* pf = cs->pageFormat();
      printerDev.setPaperSize(pf->size(), QPrinter::Inch);

      printerDev.setCreator("MuseScore Version: " VERSION);
      printerDev.setFullPage(true);
      printerDev.setColorMode(QPrinter::Color);
      printerDev.setDocName(cs->name());
      printerDev.setDoubleSidedPrinting(pf->twosided());
      printerDev.setOutputFormat(format);

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
//   readScore
///   Import file \a name
//    return 0 - OK, 1 _errno, 2 - bad file type
//---------------------------------------------------------

bool MuseScore::readScore(Score* score, QString name)
      {
      score->setName(name);

      QString cs  = score->fileInfo()->suffix();
      QString csl = cs.toLower();

      if (csl == "mscz") {
            if (!score->loadCompressedMsc(name))
                  return false;
            }
      else if (csl == "msc" || csl == "mscx") {
            if (!score->loadMsc(name))
                  return false;
            }
      else {
            typedef bool (*ImportFunction)(Score*, const QString&);
            struct ImportDef {
                  const char* extension;
                  ImportFunction importF;
                  };
            ImportDef imports[] = {
                  { "xml",  &MuseScore::importMusicXml           },
                  { "mxl",  &MuseScore::importCompressedMusicXml },
                  { "mid",  &importMidi                          },
                  { "midi", &importMidi                          },
                  { "kar",  &importMidi                          },
                  { "md",   &MuseScore::importMuseData           },
                  { "mgu",  &MuseScore::importBB                 },
                  { "sgu",  &MuseScore::importBB                 },
                  { "cap",  &MuseScore::importCapella            },
                  { "ove",  &MuseScore::importOve                },
                  { "scw",  &MuseScore::importOve                },
#ifdef OMR
                  { "pdf",  &importPdf                           },
#endif
                  { "bww",  &MuseScore::importBww                },
                  { "gtp",  &MuseScore::importGTP                },
                  { "gp3",  &MuseScore::importGTP                },
                  { "gp4",  &MuseScore::importGTP                },
                  { "gp5",  &MuseScore::importGTP                },
                  };

            // import
            if (!preferences.importStyleFile.isEmpty()) {
                  QFile f(preferences.importStyleFile);
                  // silently ignore style file on error
                  if (f.open(QIODevice::ReadOnly))
                        score->style()->load(&f);
                  }
            uint n = sizeof(imports)/sizeof(*imports);
            uint i;
            for (i = 0; i < n; ++i) {
                  if (imports[i].extension == csl) {
                        // if (!(this->*imports[i].importF)(score, name))
                        if (!(*imports[i].importF)(score, name))
                              return false;
                        break;
                        }
                  }
            if (i == n) {
                  qDebug("unknown file suffix <%s>, name <%s>\n", qPrintable(cs), qPrintable(name));
                  return false;
                  }
            score->syntiState().append(SyntiParameter("soundfont", MScore::soundFont));
            score->connectTies();
            }
      score->rebuildMidiMapping();
      score->setCreated(false);
      score->setSaved(false);

      int staffIdx = 0;
      foreach(Staff* st, score->staves()) {
            if (st->updateKeymap())
                  st->keymap()->clear();
            int track = staffIdx * VOICES;
            KeySig* key1 = 0;
            for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
                  for (Segment* s = m->first(); s; s = s->next()) {
                        if (!s->element(track))
                              continue;
                        Element* e = s->element(track);
                        if (e->generated())
                              continue;
                        //if ((s->subtype() == SegClef) && st->updateClefList()) {
                        //      Clef* clef = static_cast<Clef*>(e);
                        //      st->setClef(s->tick(), clef->clefTypeList());
                        //      }
                        if ((s->subtype() == SegKeySig) && st->updateKeymap()) {
                              KeySig* ks = static_cast<KeySig*>(e);
                              int naturals = key1 ? key1->keySigEvent().accidentalType() : 0;
                              ks->setOldSig(naturals);
                              st->setKey(s->tick(), ks->keySigEvent());
                              key1 = ks;
                              }
                        }
                  if (m->sectionBreak())
                        key1 = 0;
                  }
            st->setUpdateKeymap(false);
            ++staffIdx;
            }
      score->updateNotes();
//      score->doLayout();            // DEBUG
#if 0
      score->doLayout();
      foreach (const Excerpt* ex, score->excerpts()) {
            ex->score()->doLayout();
            }
#endif
      return true;
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
      fl.append(tr("MuseScore Format (*.mscz)"));
      fl.append(tr("MuseScore Format (*.mscx)"));     // for debugging purposes
      fl.append(tr("All Files (*)"));
      QString saveDialogTitle = saveCopy ? tr("MuseScore: Save a Copy") :
                                           tr("MuseScore: Save As");

      QSettings settings;
      if (mscore->lastSaveCopyDirectory.isEmpty())
            mscore->lastSaveCopyDirectory = settings.value("lastSaveCopyDirectory", preferences.myScoresPath).toString();
      if (mscore->lastSaveDirectory.isEmpty())
            mscore->lastSaveDirectory = settings.value("lastSaveDirectory", preferences.myScoresPath).toString();
      QString saveDirectory = saveCopy ? mscore->lastSaveCopyDirectory : mscore->lastSaveDirectory;

      if (saveDirectory.isEmpty())
            saveDirectory = preferences.myScoresPath;

      QString selectedFilter;
      QString name   = QString("%1.mscz").arg(cs->name());
      QString filter = fl.join(";;");
      QString fn     = mscore->getSaveScoreName(saveDialogTitle, name, filter, &selectedFilter);
      if (fn.isEmpty())
            return false;

      QFileInfo fi(fn);
      if (saveCopy)
            mscore->lastSaveCopyDirectory = fi.absolutePath();
      else
            mscore->lastSaveDirectory = fi.absolutePath();

      QString ext;
      if (selectedFilter.isEmpty())
            ext = fi.suffix();
      else {
            int idx = fl.indexOf(selectedFilter);
            if (idx != -1) {
                  static const char* extensions[] = {
                        "mscz", "mscx"
                        };
                  ext = extensions[idx];
                  }
            }
      if (ext.isEmpty()) {
            QMessageBox::critical(mscore, tr("MuseScore: Save As"), tr("cannot determine file type"));
            return false;
            }

      if (fi.suffix() != ext)
            fn += "." + ext;
      return saveAs(cs, saveCopy, fn, ext);
      }

//---------------------------------------------------------
//   saveSelection
//    return true on success
//---------------------------------------------------------

bool MuseScore::saveSelection(Score* cs)
      {
      QStringList fl;
      fl.append(tr("MuseScore Format (*.mscz)"));
      fl.append(tr("All Files (*)"));
      QString saveDialogTitle = tr("MuseScore: Save Selection");

      QSettings settings;
      if (mscore->lastSaveCopyDirectory.isEmpty())
            mscore->lastSaveCopyDirectory = settings.value("lastSaveCopyDirectory", preferences.myScoresPath).toString();
      if (mscore->lastSaveDirectory.isEmpty())
            mscore->lastSaveDirectory = settings.value("lastSaveDirectory", preferences.myScoresPath).toString();
      QString saveDirectory = mscore->lastSaveDirectory;

      if (saveDirectory.isEmpty())
            saveDirectory = preferences.myScoresPath;

      QString selectedFilter;
      QString name   = QString("%1.mscz").arg(cs->name());
      QString filter = fl.join(";;");
      QString fn     = mscore->getSaveScoreName(saveDialogTitle, name, filter, &selectedFilter);
      if (fn.isEmpty())
            return false;

      QFileInfo fi(fn);
      mscore->lastSaveDirectory = fi.absolutePath();

      QString ext;
      if (selectedFilter.isEmpty())
            ext = fi.suffix();
      else {
            int idx = fl.indexOf(selectedFilter);
            if (idx != -1) {
                  static const char* extensions[] = {
                        "mscz"
                        };
                  ext = extensions[idx];
                  }
            }
      if (ext.isEmpty()) {
            QMessageBox::critical(mscore, tr("MuseScore: Save Selection"), tr("cannot determine file type"));
            return false;
            }

      if (fi.suffix() != ext)
            fn += "." + ext;
      bool rv = true;
      try {
            cs->saveCompressedFile(fi, true);
            }
      catch (QString s) {
            rv = false;
            QMessageBox::critical(this, tr("MuseScore: Save Selected"), s);
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
         tr("MuseScore: InsertImage"),
         "",            // lastOpenPath,
         tr("All Supported Files (*.svg *.jpg *.png *.xpm);;"
            "Scalable vector graphics (*.svg);;"
            "JPEG (*.jpg);;"
            "PNG (*.png);;"
            "XPM (*.xpm);;"
            "All Files (*)"
            )
         );
      if (fn.isEmpty())
            return;

      QFileInfo fi(fn);
      Image* s = 0;
      QString suffix(fi.suffix().toLower());

      if (suffix == "svg")
            s = new SvgImage(score);
      else if (suffix == "jpg" || suffix == "png" || suffix == "xpm")
            s = new RasterImage(score);
      else
            return;
      s->load(fn);
      s->setParent(e);
      score->undoAddElement(s);
      }

//---------------------------------------------------------
//   savePng
//    return true on success
//---------------------------------------------------------

bool MuseScore::savePng(Score* score, const QString& name)
      {
      return savePng(score, name, false, true, converterDpi, QImage::Format_ARGB32_Premultiplied );
      }

//---------------------------------------------------------
//   savePng with options
//    return true on success
//---------------------------------------------------------

bool MuseScore::savePng(Score* score, const QString& name, bool screenshot, bool transparent, double convDpi, QImage::Format format)
      {
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
      for (int pageNumber = 0; pageNumber < pages; ++pageNumber) {
            Page* page = pl.at(pageNumber);

            QRectF r = page->abbox();
            int w = lrint(r.width()  * convDpi / MScore::DPI);
            int h = lrint(r.height() * convDpi / MScore::DPI);

            QImage printer(w, h, f);
            printer.setDotsPerMeterX(lrint((convDpi * 1000) / INCH));
            printer.setDotsPerMeterY(lrint((convDpi * 1000) / INCH));

            printer.fill(transparent ? 0 : 0xffffffff);

            double mag = convDpi / MScore::DPI;
            QPainter p(&printer);

            p.setRenderHint(QPainter::Antialiasing, true);
            p.setRenderHint(QPainter::TextAntialiasing, true);
            p.scale(mag, mag);

            paintElements(p, page->elements());

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

            rv = printer.save(fileName, "png");
            if (!rv)
                  break;
            }
      cs->setPrinting(false);
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
      qDebug("setImage <%s>\n", qPrintable(path));
      delete _pixmap;
      _pixmap = new QPixmap(path);
      update();
      }

//---------------------------------------------------------
//   getWallpaper
//---------------------------------------------------------

QString MuseScore::getWallpaper(const QString& caption)
      {
      QString filter = tr("Images (*.jpg *.gif *.png);;All (*)");
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
                  qDebug("splitter found\n");
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
//   saveSvg
//---------------------------------------------------------

bool MuseScore::saveSvg(Score* score, const QString& saveName)
      {
      SvgGenerator printer;
      printer.setResolution(converterDpi);
      QString title(score->metaTag("workTitle"));
      if(title.isEmpty())
            title = "MuseScore";
      printer.setTitle(title);
      printer.setDescription(QString("Generated by MuseScore %1").arg(VERSION));
      printer.setFileName(saveName);
      const PageFormat* pf = cs->pageFormat();
      double mag = converterDpi / MScore::DPI;

      qreal w = pf->width() * MScore::DPI * score->pages().size();
      qreal h = pf->height() * MScore::DPI;
      printer.setSize(QSize(w * mag, h * mag));
      printer.setViewBox(QRectF(0.0, 0.0, w * mag, h * mag));

      score->setPrinting(true);

      QPainter p(&printer);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::TextAntialiasing, true);
      p.scale(mag, mag);

      foreach (Page* page, score->pages()) {
            paintElements(p, page->elements());
            p.translate(QPointF(pf->width() * MScore::DPI, 0.0));
            }

      score->setPrinting(false);
      p.end();
      return true;
      }


