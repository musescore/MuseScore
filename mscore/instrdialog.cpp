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
         tr("MuseScore: Save Instrument List"),
         ".",
         tr("MuseScore Instruments") + " (*.xml)"
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
         tr("MuseScore Instruments") + " (*.xml)"
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
      ScoreView* cv = currentScoreView();
      if (cv && cv->noteEntryMode()) {
		cv->cmd(getAction("escape"));
            qApp->processEvents();
            updateInputState(cv->score());
            }
      masterScore->inputState().setTrack(-1);

      // keep the keylist of the first pitched staff to apply it to new ones
      KeyList tmpKeymap;
      Staff* firstStaff = nullptr;
      for (Staff* s : masterScore->staves()) {
            KeyList* km = s->keyList();
            if (!s->isDrumStaff()) {
                  tmpKeymap.insert(km->begin(), km->end());
                  firstStaff = s;
                  break;
                  }
            }
      Key normalizedC = Key::C;
      // normalize the keyevents to concert pitch if necessary
      if (firstStaff && !masterScore->styleB(StyleIdx::concertPitch) && firstStaff->part()->instrument()->transpose().chromatic ) {
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
                              cloneStaff(linkedStaff, staff);
                              linked.append(staff);
                              }
                        }
                  if (linked.size() == 0)
                        part->staves()->front()->setBarLineSpan(part->nstaves());

                  //insert keysigs
                  int sidx = masterScore->staffIdx(part);
                  int eidx = sidx + part->nstaves();
                  if (firstStaff)
                        masterScore->adjustKeySigs(sidx, eidx, tmpKeymap);
                  }
            else {
                  part = pli->part;
                  if (part->show() != pli->visible())
                        part->undoChangeProperty(P_ID::VISIBLE, pli->visible());
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
                                          StaffListItem* i = static_cast<StaffListItem*>(pli->child(k));
                                          if (i->op() == ListItemOp::I_DELETE && i->staff() == linkedStaff) {
                                                linkedStaff = 0;
                                                break;
                                                }
                                          }
                                    }
                              masterScore->undoInsertStaff(staff, rstaff, linkedStaff == 0);
                              if (linkedStaff)
                                    cloneStaff(linkedStaff, staff);
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
                              if (stfType->name() != staff->staffType()->name())
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
            for (int i = 0; i < n; ++i) {
                  Staff* staff = s->staff(i);
                  int span = staff->barLineSpan();
                  int setSpan = -1;

                  // determine if we need to update barline span
                  if (curSpan == 0) {
                        // no current span; this staff must start a new one
                        if (span == 0) {
                              // no span; this staff must have been within a span
                              // update it to a span of 1
                              setSpan = 1;
                              }
                        else if (span > (n - i)) {
                              // span too big; staves must have been removed
                              // reduce span to last staff
                              setSpan = n - i;
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
                  QList<BracketItem> brackets = staff->brackets();
                  int nn = brackets.size();
                  for (int ii = 0; ii < nn; ++ii) {
                        if ((brackets[ii]._bracket != BracketType::NO_BRACKET) && (brackets[ii]._bracketSpan > (n - i)))
                              s->undoChangeBracketSpan(staff, ii, n - i);
                        }
                  }
            }

      //
      // there should be at least one measure
      //
      if (masterScore->measures()->size() == 0)
            masterScore->insertMeasure(Element::Type::MEASURE, 0, false);

      for (Excerpt* excpt : masterScore->excerpts()) {
            QList<Staff*> sl = excpt->partScore()->staves();
            QMultiMap<int, int> tr = excpt->tracks();
            if (sl.size() == 0)
                  masterScore->undo(new RemoveExcerpt(excpt->partScore(), excpt->tracks()));
            else {
                  for (Staff* s : sl) {
                        LinkedStaves* sll = s->linkedStaves();
                        for (Staff* ss : sll->staves())
                              if (ss->primaryStaff()) {
                                    for (int i = s->idx() * VOICES; i < (s->idx() + 1) * VOICES; i++) {
                                          int strack = tr.key(i, -1);
                                          if (strack != -1 && ((strack & ~3) == ss->idx()))
                                                break;
                                          else if (strack != -1)
                                                tr.insert(ss->idx() + strack % VOICES, tr.value(strack, -1));
                                          }
                                    }
                        }
                  }
            }

      masterScore->setLayoutAll();
      masterScore->endCmd();
      masterScore->rebuildMidiMapping();
      seq->initInstruments();
      }

}
