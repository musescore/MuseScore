//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "scoreview.h"
#include "musescore.h"
#include "libmscore/undo.h"

#include "libmscore/score.h"
#include "libmscore/element.h"
#include "seq.h"
#include "libmscore/mscore.h"

#include "articulationprop.h"
#include "timesigproperties.h"
#include "stafftextproperties.h"
#include "selinstrument.h"
#include "pianoroll/pianoroll.h"
#include "editstyle.h"
#include "editstaff.h"
#include "measureproperties.h"

#include "libmscore/staff.h"
#include "libmscore/segment.h"
#include "libmscore/bend.h"
#include "libmscore/box.h"
#include "libmscore/text.h"
#include "libmscore/articulation.h"
#include "libmscore/volta.h"
#include "libmscore/timesig.h"
#include "libmscore/accidental.h"
#include "libmscore/clef.h"
#include "libmscore/dynamic.h"
#include "libmscore/tempotext.h"
#include "libmscore/keysig.h"
#include "libmscore/stafftext.h"
#include "libmscore/staffstate.h"
#include "libmscore/note.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/image.h"
#include "libmscore/hairpin.h"
#include "libmscore/chord.h"
#include "libmscore/rest.h"
#include "libmscore/harmony.h"
#include "libmscore/glissando.h"
#include "libmscore/fret.h"
#include "libmscore/instrchange.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/slur.h"
#include "libmscore/jump.h"
#include "libmscore/marker.h"
#include "libmscore/measure.h"
#include "libmscore/iname.h"
#include "libmscore/system.h"

namespace Ms {

//---------------------------------------------------------
//   genPropertyMenu1
//---------------------------------------------------------

void ScoreView::genPropertyMenu1(Element* e, QMenu* popup)
      {
      if ((!e->generated() || e->type() == ElementType::BAR_LINE) && enableExperimental){
            if (e->flag(ElementFlag::HAS_TAG)) {
                  popup->addSeparator();

                  QMenu* menuLayer = new QMenu(tr("Layer"));
                  for (int i = 0; i < MAX_TAGS; ++i) {
                        QString tagName = score()->layerTags()[i];
                        if (!tagName.isEmpty()) {
                              QAction* a = menuLayer->addAction(tagName);
                              a->setData(QString("layer-%1").arg(i));
                              a->setCheckable(true);
                              a->setChecked(e->tag() & (1 << i));
                              }
                        }
                  popup->addMenu(menuLayer);
                  }
            }
      }

//---------------------------------------------------------
//   genPropertyMenuText
//---------------------------------------------------------

void ScoreView::genPropertyMenuText(Element* e, QMenu* popup)
      {
      if (e->flag(ElementFlag::HAS_TAG) && enableExperimental) {
            popup->addSeparator();

            QMenu* menuLayer = new QMenu(tr("Layer"));
            for (int i = 0; i < MAX_TAGS; ++i) {
                  QString tagName = score()->layerTags()[i];
                  if (!tagName.isEmpty()) {
                        QAction* a = menuLayer->addAction(tagName);
                        a->setData(QString("layer-%1").arg(i));
                        a->setCheckable(true);
                        a->setChecked(e->tag() & (1 << i));
                        }
                  }
            popup->addMenu(menuLayer);
            }
//      popup->addAction(tr("Text Style…"))->setData("text-style");
//      popup->addAction(tr("Text Properties…"))->setData("text-props");
      }

//---------------------------------------------------------
//   createElementPropertyMenu
//---------------------------------------------------------

void ScoreView::createElementPropertyMenu(Element* e, QMenu* popup)
      {
      if (e->isBarLine())
            genPropertyMenu1(e, popup);
      else if (e->isArticulation()) {
            genPropertyMenu1(e, popup);
            popup->addAction(tr("Articulation Properties…"))->setData("a-props");
            }
      else if (e->isBeam())
            popup->addAction(getAction("flip"));
      else if (e->isStem())
            popup->addAction(getAction("flip"));
      else if (e->isHook())
            popup->addAction(getAction("flip"));
      else if (e->isHBox()) {
            QMenu* textMenu = popup->addMenu(tr("Add"));
            // borrow translation info from global actions
            // but create new actions with local handler
            textMenu->addAction(getAction("frame-text")->text())->setData("frame-text");
            textMenu->addAction(getAction("picture")->text())->setData("picture");
            }
      else if (e->isVBox()) {
            QMenu* textMenu = popup->addMenu(tr("Add"));
            // borrow translation info from global actions
            // but create new actions with local handler
            textMenu->addAction(getAction("frame-text")->text())->setData("frame-text");
            textMenu->addAction(getAction("title-text")->text())->setData("title-text");
            textMenu->addAction(getAction("subtitle-text")->text())->setData("subtitle-text");
            textMenu->addAction(getAction("composer-text")->text())->setData("composer-text");
            textMenu->addAction(getAction("poet-text")->text())->setData("poet-text");
            textMenu->addAction(getAction("part-text")->text())->setData("part-text");
            textMenu->addAction(getAction("insert-hbox")->text())->setData("insert-hbox");
            textMenu->addAction(getAction("picture")->text())->setData("picture");
            }
      else if (e->isVoltaSegment())
            genPropertyMenu1(e, popup);
      else if (e->isTimeSig()) {
            genPropertyMenu1(e, popup);
            TimeSig* ts = toTimeSig(e);
            // if the time sig. is not generated (= not courtesy) add the specific menu item
            QAction* a;
            if (!ts->generated() && ts->measure() != score()->firstMeasure()) {
                  a = popup->addAction(ts->showCourtesySig()
                     ? tr("Hide Courtesy Time Signature")
                     : tr("Show Courtesy Time Signature") );
                  a->setData("ts-courtesy");
                  }
            if (!ts->generated()) {
                  popup->addSeparator();
                  popup->addAction(tr("Time Signature Properties…"))->setData("ts-props");
                  }
            }
      else if (e->isClef()) {
            genPropertyMenu1(e, popup);
            Clef* clef = toClef(e);
            if (clef->measure() != score()->firstMeasure()) {
                  QAction* a = popup->addAction(toClef(e)->showCourtesy()
                     ? tr("Hide Courtesy Clef")
                     : tr("Show Courtesy Clef") );
                        a->setData("clef-courtesy");
                  }
            }
      else if (e->isStaffText()) {
            genPropertyMenuText(e, popup);
            popup->addAction(tr("Staff Text Properties…"))->setData("st-props");
            }
      else if (e->isSystemText()) {
            genPropertyMenuText(e, popup);
            popup->addAction(tr("System Text Properties…"))->setData("st-props");
            }
      else if (e->isText()
               || e->isSystemText()
               || e->isRehearsalMark()
               || e->isMarker()
               || e->isJump()
               || e->isLyrics()
               || e->isFiguredBass()) {
            genPropertyMenuText(e, popup);
            }
      else if (e->isHarmony()) {
            genPropertyMenu1(e, popup);
            popup->addAction(getAction("realize-chord-symbols"));
            }
      else if (e->isTempoText())
            genPropertyMenu1(e, popup);
      else if (e->isKeySig()) {
            genPropertyMenu1(e, popup);
            KeySig* ks = toKeySig(e);
            if (!e->generated() && ks->measure() != score()->firstMeasure()) {
                  QAction* a = popup->addAction(ks->showCourtesy()
                     ? tr("Hide Courtesy Key Signature")
                     : tr("Show Courtesy Key Signature") );
                  a->setData("key-courtesy");
                  }
            }
      else if (e->isStaffState() && toStaffState(e)->staffStateType() == StaffStateType::INSTRUMENT) {
            popup->addAction(tr("Change Instrument Properties…"))->setData("ss-props");
            }
      else if (e->isSlurSegment()) {
            genPropertyMenu1(e, popup);
            }
      else if (e->isRest()) {
            QAction* b = popup->actions()[0];
            QAction* a = popup->insertSeparator(b);
            a->setText(tr("Staff"));
            a = new QAction(tr("Staff/Part Properties…"), 0);
            a->setData("staff-props");
            popup->insertAction(b, a);

            a = popup->insertSeparator(b);
            a->setText(tr("Measure"));
            a = new QAction(tr("Measure Properties…"), 0);
            a->setData("measure-props");
            // disable property changes for multi measure rests
            a->setEnabled(!toRest(e)->segment()->measure()->isMMRest());

            popup->insertAction(b, a);
            genPropertyMenu1(e, popup);
            }
      else if (e->isNote()) {
            QAction* b = popup->actions()[0];
            QAction* a = popup->insertSeparator(b);
            a->setText(tr("Staff"));
            a = new QAction(tr("Staff/Part Properties…"), 0);
            a->setData("staff-props");
            popup->insertAction(b, a);

            a = popup->insertSeparator(b);
            a->setText(tr("Measure"));
            a = new QAction(tr("Measure Properties…"), 0);
            a->setData("measure-props");
            // disable property changes for multi measure rests
            a->setEnabled(!toNote(e)->chord()->segment()->measure()->isMMRest());

            popup->insertAction(b, a);

            genPropertyMenu1(e, popup);

            if (enableExperimental) {
                  popup->addSeparator();
                  popup->addAction(tr("Chord Articulation…"))->setData("articulation");
                  }
            }
      else if (e->isInstrumentChange()) {
            genPropertyMenu1(e, popup);
            popup->addAction(tr("Select Instrument…"))->setData("ch-instr");
            }
      else if (e->isInstrumentName())
            popup->addAction(tr("Staff/Part Properties…"))->setData("staff-props");
      else
            genPropertyMenu1(e, popup);

      if (EditStyle::elementHasPage(e)) {
            popup->addSeparator();
            popup->addAction(tr("Style…"))->setData("style");
            }
      }

//---------------------------------------------------------
//   elementPropertyAction
//---------------------------------------------------------

void ScoreView::elementPropertyAction(const QString& cmd, Element* e)
      {
      if (cmd == "a-props") {
            editArticulationProperties(toArticulation(e));
            }
      else if (cmd == "measure-props") {
            Measure* m = 0;
            if (e->type() == ElementType::NOTE)
                  m = toNote(e)->chord()->segment()->measure();
            else if (e->type() == ElementType::REST)
                  m = toRest(e)->segment()->measure();
            if (m) {
                  MeasureProperties vp(m);
                  vp.exec();
                  }
            }
      else if (cmd == "picture") {
            mscore->addImage(score(), toHBox(e));
            }
      else if (cmd == "frame-text") {
            Text* t = new Text(score(), Tid::FRAME);
            t->setParent(e);
            score()->undoAddElement(t);
            score()->select(t, SelectType::SINGLE, 0);
            startEditMode(t);
            }
      else if (cmd == "title-text") {
            Text* t = new Text(score(), Tid::TITLE);
            t->setParent(e);
            score()->undoAddElement(t);
            score()->select(t, SelectType::SINGLE, 0);
            startEditMode(t);
            }
      else if (cmd == "subtitle-text") {
            Text* t = new Text(score(), Tid::SUBTITLE);
            t->setParent(e);
            score()->undoAddElement(t);
            score()->select(t, SelectType::SINGLE, 0);
            startEditMode(t);
            }
      else if (cmd == "composer-text") {
            Text* t = new Text(score(), Tid::COMPOSER);
            t->setParent(e);
            score()->undoAddElement(t);
            score()->select(t, SelectType::SINGLE, 0);
            startEditMode(t);
            }
      else if (cmd == "poet-text") {
            Text* t = new Text(score(), Tid::POET);
            t->setParent(e);
            score()->undoAddElement(t);
            score()->select(t, SelectType::SINGLE, 0);
            startEditMode(t);
            }
      else if (cmd == "part-text") {
            Text* t = new Text(score(), Tid::INSTRUMENT_EXCERPT);
            t->setParent(e);
            score()->undoAddElement(t);
            score()->select(t, SelectType::SINGLE, 0);
            startEditMode(t);
            }
      else if (cmd == "insert-hbox") {
            HBox* s = new HBox(score());
            double w = e->width() - s->leftMargin() * DPMM - s->rightMargin() * DPMM;
            s->setBoxWidth(Spatium(w / s->spatium()));
            s->setParent(e);
            score()->undoAddElement(s);
            score()->select(s, SelectType::SINGLE, 0);
            startEditMode(s);
            }
      if (cmd == "ts-courtesy") {
            for (int stave = 0; stave < score()->nstaves(); stave++) {
                  TimeSig* ts = toTimeSig(toSegment(e->parent())->element(stave*VOICES));
                  if (ts)
                        ts->undoChangeProperty(Pid::SHOW_COURTESY, !ts->showCourtesySig());
                  }
            }
      else if (cmd == "ts-props") {
            editTimeSigProperties(toTimeSig(e));
            }
      else if (cmd == "smallNote")
            e->undoChangeProperty(Pid::SMALL, !toNote(e)->small());
      else if (cmd == "clef-courtesy") {
            Clef* clef = toClef(e);
            bool show = !clef->showCourtesy();
            clef->undoChangeProperty(Pid::SHOW_COURTESY, show);
            Clef* otherClef = clef->otherClef();
            if (otherClef)
                  otherClef->undoChangeProperty(Pid::SHOW_COURTESY, show);
            }
      else if (cmd == "st-props") {
            editStaffTextProperties(toStaffTextBase(e));
            }
#if 0
      else if (cmd == "text-style") {
            Text* t = toText(e);
            QString name = t->textStyle().name();
            TextStyleDialog ts(0, score());
            ts.setPage(name);
            ts.exec();
            }
      else if (cmd == "text-props") {
            Text* ot    = toText(e);
            Text* nText = toText(ot->clone());
            TextProperties tp(nText);
            int rv = tp.exec();
            if (rv) {
                  qDebug("text-props %d %d", int(ot->textStyleType()), int(nText->textStyleType()));
//                  if (ot->textStyleType() != nText->textStyleType()) {
//                        nText->restyle(ot->textStyleType());
//                        ot->undoChangeProperty(Pid::TEXT_STYLE_TYPE, int(nText->textStyleType()));
//                        }
//                  if (ot->textStyle() != nText->textStyle())
//                        ot->undoChangeProperty(Pid::TEXT_STYLE, QVariant::fromValue<TextStyle>(nText->textStyle()));
                  if (ot->xmlText() != nText->xmlText())
                        ot->undoChangeProperty(Pid::TEXT, nText->xmlText());
                  }
            delete nText;
            }
#endif
      else if (cmd == "key-courtesy") {
            for (int stave = 0; stave < score()->nstaves(); stave++) {
                  KeySig* ks = toKeySig(toSegment(e->parent())->element(stave*VOICES));
                  score()->undo(new ChangeKeySig(ks, ks->keySigEvent(), !ks->showCourtesy() /*, ks->showNaturals()*/));
                  }
            }
      else if (cmd == "ss-props") {
            StaffState* ss = toStaffState(e);
            SelectInstrument si(ss->instrument(), 0);
            if (si.exec()) {
                  const InstrumentTemplate* it = si.instrTemplate();
                  if (it) {
                        // TODO: undo/redo
                        ss->setInstrument(Instrument::fromTemplate(it));
                        ss->staff()->part()->setInstrument(ss->instrument(), ss->segment()->tick());
                        score()->masterScore()->rebuildMidiMapping();
                        seq->initInstruments();
                        score()->setLayoutAll();
                        }
                  else
                        qDebug("no template selected?");
                  }
            }
      else if (cmd == "articulation") {
            Note* note = toNote(e);
            mscore->editInPianoroll(note->staff());
            }
      else if (cmd == "style") {
            if (!mscore->styleDlg())
                  mscore->setStyleDlg(new EditStyle { _score, mscore });
            else
                  mscore->styleDlg()->setScore(mscore->currentScore());
            mscore->styleDlg()->gotoElement(e);
            mscore->styleDlg()->exec();
            }
      else if (cmd == "style-header-footer") { // used to go to the header/footer dialog by double-clicking on a header/footer
            if (!mscore->styleDlg())
                  mscore->setStyleDlg(new EditStyle { _score, mscore });
            else
                  mscore->styleDlg()->setScore(mscore->currentScore());
            mscore->styleDlg()->gotoHeaderFooterPage();
            mscore->styleDlg()->exec();
            }
      else if (cmd == "ch-instr")
            selectInstrument(toInstrumentChange(e));
      else if (cmd == "staff-props") {
            Fraction tick = {-1,1};
            if (e->isChordRest()) {
                  tick = toChordRest(e)->tick();
                  }
            else if (e->isNote()) {
                  tick = toNote(e)->chord()->tick();
                  }
            else if (e->isMeasure()) {
                  tick = toMeasure(e)->tick();
                  }
            else if (e->isInstrumentName()) {
                  System* system = toSystem(toInstrumentName(e)->parent());
                  Measure* m = system ? system->firstMeasure() : nullptr;
                  if (m)
                        tick = m->tick();
                  }
            EditStaff editStaff(e->staff(), tick, 0);
            connect(&editStaff, SIGNAL(instrumentChanged()), mscore, SLOT(instrumentChanged()));
            editStaff.exec();
            }
      else if (cmd.startsWith("layer-")) {
            int n = cmd.mid(6).toInt();
            uint mask = 1 << n;
            e->setTag(mask);
            }
      }

//---------------------------------------------------------
//   editArticulationProperties
//---------------------------------------------------------

void ScoreView::editArticulationProperties(Articulation* ar)
      {
      ArticulationProperties rp(ar);
      rp.exec();
      }

//---------------------------------------------------------
//   editTimeSigProperties
//---------------------------------------------------------

void ScoreView::editTimeSigProperties(TimeSig* ts)
      {
      TimeSig* r = new TimeSig(*ts);
      TimeSigProperties tsp(r);

      if (tsp.exec()) {
            ts->undoChangeProperty(Pid::SHOW_COURTESY, r->showCourtesySig());
            ts->undoChangeProperty(Pid::NUMERATOR_STRING, r->numeratorString());
            ts->undoChangeProperty(Pid::DENOMINATOR_STRING, r->denominatorString());
            ts->undoChangeProperty(Pid::TIMESIG_TYPE, int(r->timeSigType()));
            ts->undoChangeProperty(Pid::GROUPS, QVariant::fromValue<Groups>(r->groups()));

            if (r->sig() != ts->sig()) {
                  score()->cmdAddTimeSig(ts->measure(), ts->staffIdx(), r, true);
                  r = 0;
                  }
            }
      delete r;
      }

//---------------------------------------------------------
//   selectInstrument
//---------------------------------------------------------

void Ms::ScoreView::selectInstrument(InstrumentChange* ic)
      {
      SelectInstrument si(ic->instrument(), 0);
      if (si.exec()) {
            const InstrumentTemplate* it = si.instrTemplate();
            if (it) {
                  Instrument instr = Instrument::fromTemplate(it);
                  ic->setInit(true);
                  ic->setupInstrument(&instr);
                  }
            else
                  qDebug("no template selected?");
            }
      }

//---------------------------------------------------------
//   editStaffTextProperties
//---------------------------------------------------------

void ScoreView::editStaffTextProperties(StaffTextBase* st)
      {
      StaffTextProperties rp(st);
      if (rp.exec()) {
            Score* score = st->score();
            StaffTextBase* nt = toStaffTextBase(rp.staffTextBase()->clone());
            nt->setScore(score);
            score->undoChangeElement(st, nt);
            score->masterScore()->updateChannel();
            score->updateCapo();
            score->updateSwing();
            score->setPlaylistDirty();
            }
      }

}
