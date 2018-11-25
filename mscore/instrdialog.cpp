//=============================================================================
//  MuseScore
//  Linux Music Score Editor
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
#include "preferences.h"
#include "scoreview.h"
#include "seq.h"
#include "libmscore/barline.h"
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
#include "libmscore/bracketItem.h"

namespace Ms {

//---------------------------------------------------------
//   InstrumentsDialog
//---------------------------------------------------------

InstrumentsDialog::InstrumentsDialog(QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("Instruments");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      QAction* a = getAction("instruments");
      connect(a, SIGNAL(triggered()), SLOT(reject()));
      addAction(a);
      saveButton->setVisible(false);
      loadButton->setVisible(false);
      readSettings();
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
         tr("Save Instrument List"),
         ".",
         tr("MuseScore Instruments") + " (*.xml)",
         0,
         preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS) ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog
         );
      if (name.isEmpty())
            return;
      QString ext(".xml");
      QFileInfo info(name);

      if (info.suffix().isEmpty())
            info.setFile(info.filePath() + ext);
      QFile f(info.filePath());
      if (!f.open(QIODevice::WriteOnly)) {
            QString s = tr("Open Instruments File\n%1\nfailed: %2").arg(f.fileName(), strerror(errno));
            QMessageBox::critical(mscore, tr("Open Instruments File"), s);
            return;
            }

      XmlWriter xml(0, &f);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      for (InstrumentGroup* g : instrumentGroups) {
            xml.stag(QString("InstrumentGroup name=\"%1\" extended=\"%2\"").arg(g->name).arg(g->extended));
            for (InstrumentTemplate* t : g->instrumentTemplates)
                  t->write(xml);
            xml.etag();
            }
      xml.etag();
      if (f.error() != QFile::NoError) {
            QString s = tr("Write Instruments File failed: %1").arg(f.errorString());
            QMessageBox::critical(this, tr("Write Instruments File"), s);
            }
      }

//---------------------------------------------------------
//   on_loadButton_clicked
//---------------------------------------------------------

void InstrumentsDialog::on_loadButton_clicked()
      {
      QString fn = QFileDialog::getOpenFileName(
         this, tr("Load Instrument List"),
          mscoreGlobalShare + "/templates",
         tr("MuseScore Instruments") + " (*.xml)",
         0,
         preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS) ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog
         );
      if (fn.isEmpty())
            return;
      QFile f(fn);
      if (!loadInstrumentTemplates(fn)) {
            QMessageBox::warning(0,
               QWidget::tr("Load Style Failed"),
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
      MuseScore::saveGeometry(this);
      }

//---------------------------------------------------------
//   readSettings
//---------------------------------------------------------

void InstrumentsDialog::readSettings()
      {
      MuseScore::restoreGeometry(this);
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
//   buildInstrumentsList
//---------------------------------------------------------

void InstrumentsDialog::buildInstrumentsList()
      {
      instrumentsWidget->buildTemplateList();
      }

//---------------------------------------------------------
//   updateInstrumentDialog
//---------------------------------------------------------

void MuseScore::updateInstrumentDialog()
      {
      if (instrList)
            instrList->buildInstrumentsList();
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
      MasterScore* masterScore = cs->masterScore();
      instrList->genPartList(masterScore);
      masterScore->startCmd();
      masterScore->deselectAll();
      int rv = instrList->exec();

      if (rv == 0) {
            masterScore->endCmd();
            return;
            }
      ScoreView* csv = currentScoreView();
      if (csv && csv->noteEntryMode()) {
		csv->cmd(getAction("escape"));
            qApp->processEvents();
            updateInputState(csv->score());
            }
      masterScore->inputState().setTrack(-1);

      // keep the keylist of the first pitched staff to apply it to new ones
      KeyList tmpKeymap;
      Staff* firstStaff = 0;
      for (Staff* s : masterScore->staves()) {
            KeyList* km = s->keyList();
            if (!s->isDrumStaff(0)) {     // TODO
                  tmpKeymap.insert(km->begin(), km->end());
                  firstStaff = s;
                  break;
                  }
            }
      Key normalizedC = Key::C;
      // normalize the keyevents to concert pitch if necessary
      if (firstStaff && !masterScore->styleB(Sid::concertPitch) && firstStaff->part()->instrument()->transpose().chromatic ) {
            int interval = firstStaff->part()->instrument()->transpose().chromatic;
            normalizedC = transposeKey(normalizedC, interval);
            for (auto i = tmpKeymap.begin(); i != tmpKeymap.end(); ++i) {
                  int tick = i->first;
                  Key oKey = i->second.key();
                  tmpKeymap[tick].setKey(transposeKey(oKey, interval));
                  }
            }
      // create initial keyevent for transposing instrument if necessary
      auto i = tmpKeymap.begin();
      if (i == tmpKeymap.end() || i->first != 0)
            tmpKeymap[0].setKey(normalizedC);

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
                  masterScore->cmdRemovePart(pli->part);
            else if (pli->op == ListItemOp::ADD) {
                  const InstrumentTemplate* t = ((PartListItem*)item)->it;
                  part = new Part(masterScore);
                  part->initFromInstrTemplate(t);
                  masterScore->undo(new InsertPart(part, staffIdx));

                  pli->part = part;
                  QList<Staff*> linked;
                  for (int cidx = 0; pli->child(cidx); ++cidx) {
                        StaffListItem* sli = static_cast<StaffListItem*>(pli->child(cidx));
                        Staff* staff       = new Staff(masterScore);
                        staff->setPart(part);
                        sli->setStaff(staff);

                        staff->init(t, sli->staffType(), cidx);
                        staff->setDefaultClefType(sli->defaultClefType());

                        masterScore->undoInsertStaff(staff, cidx);
                        ++staffIdx;

                        Staff* linkedStaff = part->staves()->front();
                        if (sli->linked() && linkedStaff != staff) {
                              Excerpt::cloneStaff(linkedStaff, staff);
                              linked.append(staff);
                              }
                        }

                  //insert keysigs
                  int sidx = masterScore->staffIdx(part);
                  int eidx = sidx + part->nstaves();
                  if (firstStaff)
                        masterScore->adjustKeySigs(sidx, eidx, tmpKeymap);
                  }
            else {
                  part = pli->part;
                  if (part->show() != pli->visible())
                        part->undoChangeProperty(Pid::VISIBLE, pli->visible());
                  for (int cidx = 0; pli->child(cidx); ++cidx) {
                        StaffListItem* sli = static_cast<StaffListItem*>(pli->child(cidx));
                        if (sli->op() == ListItemOp::I_DELETE) {
                              masterScore->systems().clear();
                              Staff* staff = sli->staff();
                              int sidx = staff->idx();
                              masterScore->cmdRemoveStaff(sidx);
                              }
                        else if (sli->op() == ListItemOp::ADD) {
                              Staff* staff = new Staff(masterScore);
                              staff->setPart(part);
                              staff->initFromStaffType(sli->staffType());
                              sli->setStaff(staff);
                              staff->setDefaultClefType(sli->defaultClefType());

                              KeySigEvent ke;
                              if (part->staves()->empty())
                                    ke.setKey(Key::C);
                              else
                                    ke = part->staff(0)->keySigEvent(0);

                              staff->setKey(0, ke);

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
                              if (linkedStaff) {
                                    // do not create a link if linkedStaff will be removed,
                                    for (int k = 0; pli->child(k); ++k) {
                                          StaffListItem* li = static_cast<StaffListItem*>(pli->child(k));
                                          if (li->op() == ListItemOp::I_DELETE && li->staff() == linkedStaff) {
                                                linkedStaff = 0;
                                                break;
                                                }
                                          }
                                    }
                              masterScore->undoInsertStaff(staff, rstaff, linkedStaff == 0);
                              if (linkedStaff)
                                    Excerpt::cloneStaff(linkedStaff, staff);
                              else {
                                    if (firstStaff)
                                          masterScore->adjustKeySigs(staffIdx, staffIdx+1, tmpKeymap);
                                    }
                              ++staffIdx;
                              ++rstaff;
                              }
                        else if (sli->op() == ListItemOp::UPDATE) {
                              // check changes in staff type
                              Staff* staff = sli->staff();
                              const StaffType* stfType = sli->staffType();

                              // use selected staff type
                              if (stfType->name() != staff->staffType(0)->name())
                                    masterScore->undo(new ChangeStaffType(staff, *stfType));
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
      for (Staff* staff : dst) {
            int idx = masterScore->staves().indexOf(staff);
            if (idx == -1)
                  qDebug("staff in dialog(%p) not found in score", staff);
            else
                  dl.push_back(idx);
            if (idx != idx2)
                  sort = true;
            ++idx2;
            }

      if (sort)
            masterScore->undo(new SortStaves(masterScore, dl));

      //
      // check for valid barLineSpan and bracketSpan
      // in all staves
      //
      for (Score* s : masterScore->scoreList()) {
            int n = s->nstaves();
            int curSpan = 0;
            for (int j = 0; j < n; ++j) {
                  Staff* staff = s->staff(j);
                  int span = staff->barLineSpan();
                  int setSpan = -1;

                  // determine if we need to update barline span
                  if (curSpan == 0) {
                        // no current span; this staff must start a new one
                        if (span == 0) {
                              // no span; this staff must have been within a span
                              // update it to a span of 1
                              setSpan = j;
                              }
                        else if (span > (n - j)) {
                              // span too big; staves must have been removed
                              // reduce span to last staff
                              setSpan = n - j;
                              }
                        else if (span > 1 && staff->barLineTo() > 0) {
                              // TODO: check if span is still valid
                              // (true if the last staff is the same as it was before this edit)
                              // the code here fixes https://musescore.org/en/node/41786
                              // but by forcing an update,
                              // we lose custom modifications to staff barLineTo
                              // at least this happens only for span > 1, and not for Mensurstrich (barLineTo<=0)
                              setSpan = span;   // force update to pick up new barLineTo value
                              }
                        else {
                              // this staff starts a span
                              curSpan = span;
                              }
                        }
                  else if (span && staff->barLineTo() > 0) {
                        // within a current span; staff must have span of 0
                        // except for Mensurstrich (barLineTo<=0)
                        // for consistency with Barline::endEdit,
                        // don't special case 1-line staves
//TODO                        s->undoChangeBarLineSpan(staff, 0, 0, (staff->lines() - 1) * 2);
                        }

                  // update barline span if necessary
                  if (setSpan > 0) {
                        // this staff starts a span
                        curSpan = setSpan;
                        // calculate spanFrom and spanTo values
//                        int spanFrom = staff->lines() == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : 0;
//                        int linesTo = masterScore->staff(i + setSpan - 1)->lines();
//                        int spanTo = linesTo == 1 ? BARLINE_SPAN_1LINESTAFF_TO : (linesTo - 1) * 2;
//TODO                         s->undoChangeBarLineSpan(staff, setSpan, spanFrom, spanTo);
                        }

                  // count off one from barline span
                  --curSpan;

                  // update brackets
                  for (BracketItem* bi : staff->brackets()) {
                        if ((bi->bracketSpan() > (n - j)))
                              bi->undoChangeProperty(Pid::BRACKET_SPAN, n - j);
                        }
                  }
            }

      //
      // there should be at least one measure
      //
      if (masterScore->measures()->size() == 0)
            masterScore->insertMeasure(ElementType::MEASURE, 0, false);

      for (Excerpt* excerpt : masterScore->excerpts()) {
            QList<Staff*> sl       = excerpt->partScore()->staves();
            QMultiMap<int, int> tr = excerpt->tracks();
            if (sl.size() == 0)
                  masterScore->undo(new RemoveExcerpt(excerpt));
            else {
                  for (Staff* s : sl) {
                        const LinkedElements* sll = s->links();
                        for (auto le : *sll) {
                              Staff* ss = toStaff(le);
                              if (ss->primaryStaff()) {
                                    for (int j = s->idx() * VOICES; j < (s->idx() + 1) * VOICES; j++) {
                                          int strack = tr.key(j, -1);
                                          if (strack != -1 && ((strack & ~3) == ss->idx()))
                                                break;
                                          else if (strack != -1)
                                                tr.insert(ss->idx() + strack % VOICES, tr.value(strack, -1));
                                          }
                                    }
                              }
                        }
                  }
            }
      masterScore->setLayoutAll();
      masterScore->setInstrumentsChanged(true);
      masterScore->endCmd();
      seq->initInstruments();
      }

}
