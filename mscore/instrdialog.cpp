//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: instrdialog.cpp 5580 2012-04-27 15:36:57Z wschweer $
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#include "instrdialog.h"
#include "musescore.h"
#include "scoreview.h"
#include "seq.h"
#include "libmscore/clef.h"
#include "libmscore/excerpt.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/measure.h"
#include "libmscore/part.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/slur.h"
#include "libmscore/staff.h"
#include "libmscore/stafftype.h"
#include "libmscore/undo.h"

namespace Ms {

extern bool useFactorySettings;

//---------------------------------------------------------
//   InstrumentsDialog
//---------------------------------------------------------

InstrumentsDialog::InstrumentsDialog(QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      QAction* a = getAction("instruments");
      connect(a, SIGNAL(triggered()), SLOT(reject()));
      addAction(a);

      if (!useFactorySettings) {
            QSettings settings;
            settings.beginGroup("Instruments");
            resize(settings.value("size", QSize(800, 500)).toSize());
            move(settings.value("pos", QPoint(10, 10)).toPoint());
            settings.endGroup();
            }

      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void InstrumentsDialog::accept()
      {
      done(1);
      }

//---------------------------------------------------------
//   on_saveButton_clicked
//---------------------------------------------------------

void InstrumentsDialog::on_saveButton_clicked()
      {
      QString name = QFileDialog::getSaveFileName(
         this,
         tr("MuseScore: Save Instrument List"),
         ".",
         tr("MuseScore Instruments (*.xml);;")
         );
      if (name.isEmpty())
            return;
      QString ext(".xml");
      QFileInfo info(name);

      if (info.suffix().isEmpty())
            info.setFile(info.filePath() + ext);
      QFile f(info.filePath());
      if (!f.open(QIODevice::WriteOnly)) {
            QString s = tr("Open Instruments File\n%1\nfailed: ")
               + QString(strerror(errno));
            QMessageBox::critical(mscore, tr("MuseScore: Open Instruments File"), s.arg(f.fileName()));
            return;
            }

      Xml xml(&f);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      foreach(InstrumentGroup* g, instrumentGroups) {
            xml.stag(QString("InstrumentGroup name=\"%1\" extended=\"%2\"").arg(g->name).arg(g->extended));
            foreach(InstrumentTemplate* t, g->instrumentTemplates)
                  t->write(xml);
            xml.etag();
            }
      xml.etag();
      if (f.error() != QFile::NoError) {
            QString s = tr("Write Style failed: ") + f.errorString();
            QMessageBox::critical(this, tr("MuseScore: Write Style"), s);
            }
      }

//---------------------------------------------------------
//   on_loadButton_clicked
//---------------------------------------------------------

void InstrumentsDialog::on_loadButton_clicked()
      {
      QString fn = QFileDialog::getOpenFileName(
         this, tr("MuseScore: Load Instrument List"),
          mscoreGlobalShare + "/templates",
         tr("MuseScore Instruments (*.xml);;"
            "All files (*)"
            )
         );
      if (fn.isEmpty())
            return;
      QFile f(fn);
      if (!loadInstrumentTemplates(fn)) {
            QMessageBox::warning(0,
               QWidget::tr("MuseScore: Load Style Failed"),
               QString(strerror(errno)),
               QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
            return;
            }
      instrumentsWidget->buildTemplateList();
      }

//---------------------------------------------------------
//   writeSettings
//---------------------------------------------------------

void InstrumentsDialog::writeSettings()
      {
      QSettings settings;
      settings.beginGroup("Instruments");
      settings.setValue("size", size());
      settings.setValue("pos", pos());
      settings.endGroup();
      }

//---------------------------------------------------------
//   genPartList
//---------------------------------------------------------

void InstrumentsDialog::genPartList(Score* s)
      {
      instrumentsWidget->genPartList(s);
      }

//---------------------------------------------------------
//   partiturList
//---------------------------------------------------------

QTreeWidget* InstrumentsDialog::partiturList()
      {
      return instrumentsWidget->getPartiturList();
      }

//---------------------------------------------------------
//   editInstrList
//---------------------------------------------------------

void MuseScore::editInstrList()
      {
      if (cs == 0)
            return;
      if (!instrList)
            instrList = new InstrumentsDialog(this);
      else if (instrList->isVisible()) {
            instrList->done(0);
            return;
            }
      Score* rootScore = cs->rootScore();
      instrList->genPartList(rootScore);
      rootScore->startCmd();
      rootScore->deselectAll();
      int rv = instrList->exec();

      if (rv == 0) {
            rootScore->endCmd();
            return;
            }
      ScoreView* cv = currentScoreView();
      if (cv && cv->noteEntryMode()) {
		cv->cmd(getAction("escape"));
            qApp->processEvents();
            updateInputState(cv->score());
            }
      rootScore->inputState().setTrack(-1);

      // keep the keylist of the first staff to apply it to new ones
      KeyList tmpKeymap;
      Staff* firstStaff = nullptr;
      for (Staff* s : rootScore->staves()) {
            KeyList* km = s->keyList();
            if (!s->isDrumStaff()) {
                  tmpKeymap.insert(km->begin(), km->end());
                  firstStaff = s;
                  break;
                  }
            }
      //normalize the keyevent to concert pitch if necessary
      if (firstStaff && !rootScore->styleB(StyleIdx::concertPitch) && firstStaff->part()->instr()->transpose().chromatic ) {
            int interval = firstStaff->part()->instr()->transpose().chromatic;
            for (auto i = tmpKeymap.begin(); i != tmpKeymap.end(); ++i) {
                  int tick = i->first;
                  Key oKey = i->second;
                  tmpKeymap[tick] = transposeKey(oKey, interval);
                  }
            }

      //
      // process modified partitur list
      //
      QTreeWidget* pl = instrList->partiturList();
      Part* part   = 0;
      int staffIdx = 0;

      QTreeWidgetItem* item = 0;
      for (int idx = 0; (item = pl->topLevelItem(idx)); ++idx) {
            PartListItem* pli = static_cast<PartListItem*>(item);
            // check if the part contains any remaining staves
            // mark to remove part if not
            QTreeWidgetItem* ci = 0;
            int staves = 0;
            for (int cidx = 0; (ci = pli->child(cidx)); ++cidx) {
                  StaffListItem* sli = static_cast<StaffListItem*>(ci);
                  if (sli->op() != ListItemOp::I_DELETE)
                        ++staves;
                  }
            if (staves == 0)
                  pli->op = ListItemOp::I_DELETE;
            }

      item = 0;
      for (int idx = 0; (item = pl->topLevelItem(idx)); ++idx) {
            int rstaff = 0;
            PartListItem* pli = static_cast<PartListItem*>(item);
            if (pli->op == ListItemOp::I_DELETE)
                  rootScore->cmdRemovePart(pli->part);
            else if (pli->op == ListItemOp::ADD) {
                  const InstrumentTemplate* t = ((PartListItem*)item)->it;
                  part = new Part(rootScore);
                  part->initFromInstrTemplate(t);
                  rootScore->undo(new InsertPart(part, staffIdx));

                  pli->part = part;
                  QTreeWidgetItem* ci = 0;
                  QList<Staff*> linked;
                  for (int cidx = 0; (ci = pli->child(cidx)); ++cidx) {
                        StaffListItem* sli = static_cast<StaffListItem*>(ci);
                        Staff* staff       = new Staff(rootScore);
                        staff->setPart(part);
                        sli->setStaff(staff);

                        staff->init(t, sli->staffType(), cidx);
                        staff->setDefaultClefType(sli->defaultClefType());

                        rootScore->undoInsertStaff(staff, cidx);
                        ++staffIdx;

                        Staff* linkedStaff = part->staves()->front();
                        if (sli->linked() && linkedStaff != staff) {
                              cloneStaff(linkedStaff, staff);
                              linked.append(staff);
                              }
                        }
                  if (linked.size() == 0)
                        part->staves()->front()->setBarLineSpan(part->nstaves());

                  //insert keysigs
                  int sidx = rootScore->staffIdx(part);
                  int eidx = sidx + part->nstaves();
                  if (firstStaff)
                        rootScore->adjustKeySigs(sidx, eidx, tmpKeymap);
                  }
            else {
                  part = pli->part;
                  if (part->show() != pli->visible()) {
                        part->score()->undo()->push(new ChangePartProperty(part, 0, pli->visible()));
                        }
                  QTreeWidgetItem* ci = 0;
                  for (int cidx = 0; (ci = pli->child(cidx)); ++cidx) {
                        StaffListItem* sli = (StaffListItem*)ci;
                        if (sli->op() == ListItemOp::I_DELETE) {
                              rootScore->systems()->clear();
                              Staff* staff = sli->staff();
                              int sidx = staff->idx();
                              rootScore->cmdRemoveStaff(sidx);
                              }
                        else if (sli->op() == ListItemOp::ADD) {
                              Staff* staff = new Staff(rootScore);
                              staff->setPart(part);
                              staff->initFromStaffType(sli->staffType());
                              sli->setStaff(staff);
                              staff->setDefaultClefType(sli->defaultClefType());

                              rootScore->undoInsertStaff(staff, rstaff);

                              Key nKey = part->staff(0)->key(0);
                              staff->setKey(0, nKey);

                              Staff* linkedStaff = 0;
                              if (sli->linked()) {
                                    if (rstaff > 0)
                                          linkedStaff = part->staves()->front();
                                    else {
                                          for (Staff* st : *part->staves()) {
                                                if (st != staff) {
                                                      linkedStaff = st;
                                                      break;
                                                      }
                                                }
                                          }
                                    }
                              if (linkedStaff)
                                    cloneStaff(linkedStaff, staff);
                              else {
                                    if (firstStaff)
                                          rootScore->adjustKeySigs(staffIdx, staffIdx+1, tmpKeymap);
                                    }
                              ++staffIdx;
                              ++rstaff;
                              }
                        else if (sli->op() == ListItemOp::UPDATE) {
                              // check changes in staff type
                              Staff* staff = sli->staff();
                              const StaffType* stfType = sli->staffType();
                              // before changing staff type, check if notes need to be updated
                              // (true if changing into or away from TAB)
                              StaffGroup ng = stfType->group();         // new staff group
                              StaffGroup og = staff->staffGroup();      // old staff group
                              bool updateNeeded = (ng == StaffGroup::TAB) != (og == StaffGroup::TAB);

                              // use selected staff type
                              if (stfType->name() != staff->staffType()->name())
                                    rootScore->undo(new ChangeStaffType(staff, *stfType));

                              if (updateNeeded)
                                    rootScore->cmdUpdateNotes();
                              }
                        else {
                              ++staffIdx;
                              ++rstaff;
                              }
                        }
                  }
            }

      //
      //    sort staves
      //
      QList<Staff*> dst;
      for (int idx = 0; idx < pl->topLevelItemCount(); ++idx) {
            PartListItem* pli = (PartListItem*)pl->topLevelItem(idx);
            if (pli->op == ListItemOp::I_DELETE)
                  continue;
            QTreeWidgetItem* ci = 0;
            for (int cidx = 0; (ci = pli->child(cidx)); ++cidx) {
                  StaffListItem* sli = (StaffListItem*) ci;
                  if (sli->op() == ListItemOp::I_DELETE)
                        continue;
                  dst.push_back(sli->staff());
                  }
            }

      QList<int> dl;
      int idx2 = 0;
      bool sort = false;
      for(Staff* staff : dst) {
            int idx = rootScore->staves().indexOf(staff);
            if (idx == -1)
                  qDebug("staff in dialog(%p) not found in score", staff);
            else
                  dl.push_back(idx);
            if (idx != idx2)
                  sort = true;
            ++idx2;
            }

      if (sort)
            rootScore->undo(new SortStaves(rootScore, dl));

      //
      // check for valid barLineSpan and bracketSpan
      // in all staves
      //

      int n = rootScore->nstaves();
      for (int i = 0; i < n; ++i) {
            Staff* staff = rootScore->staff(i);
            if (staff->barLineSpan() > (n - i))
                  rootScore->undoChangeBarLineSpan(staff, n - i, 0, (rootScore->staff(n-1)->lines()-1) * 2);
            QList<BracketItem> brackets = staff->brackets();
            int nn = brackets.size();
            for (int ii = 0; ii < nn; ++ii) {
                  if ((brackets[ii]._bracket != BracketType::NO_BRACKET) && (brackets[ii]._bracketSpan > (n - i)))
                        rootScore->undoChangeBracketSpan(staff, ii, n - i);
                  }
            }
      //
      // there should be at least one measure
      //
      if (rootScore->measures()->size() == 0)
            rootScore->insertMeasure(Element::Type::MEASURE, 0, false);

      QList<Score*> toDelete;
      for (Excerpt* excpt : rootScore->excerpts()) {
            if (excpt->score()->staves().size() == 0)
                  toDelete.append(excpt->score());
            }
      for(Score* s: toDelete)
            rootScore->undo(new RemoveExcerpt(s));

      rootScore->setLayoutAll(true);
      rootScore->cmdUpdateNotes();  // do it before global layout or layout of chords will not
      rootScore->endCmd();          // find notes in right positions for stems, ledger lines, etc
      rootScore->rebuildMidiMapping();
      seq->initInstruments();
      }

}
