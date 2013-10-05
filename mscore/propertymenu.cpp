//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
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
#include "bendproperties.h"
#include "boxproperties.h"
#include "voltaproperties.h"
#include "lineproperties.h"
#include "tremolobarprop.h"
#include "timesigproperties.h"
#include "textproperties.h"
#include "sectionbreakprop.h"
#include "stafftextproperties.h"
#include "glissandoproperties.h"
#include "fretproperties.h"
#include "selinstrument.h"
#include "chordedit.h"
#include "pianoroll.h"
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
#include "libmscore/tremolobar.h"
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
#include "libmscore/slur.h"
#include "libmscore/jump.h"
#include "libmscore/marker.h"

namespace Ms {

//---------------------------------------------------------
//   genPropertyMenu1
//---------------------------------------------------------

void ScoreView::genPropertyMenu1(Element* e, QMenu* popup)
      {
      if (!e->generated() || e->type() == Element::BAR_LINE) {
#if 0
            if (e->type() != Element::LAYOUT_BREAK) {
                  if (e->visible())
                        popup->addAction(tr("Set Invisible"))->setData("invisible");
                  else
                        popup->addAction(tr("Set Visible"))->setData("invisible");
                  popup->addAction(tr("Color..."))->setData("color");
                  }
#endif
            if (e->flag(ELEMENT_HAS_TAG)) {
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
#if 0
      if (e->visible())
            popup->addAction(tr("Set Invisible"))->setData("invisible");
      else
            popup->addAction(tr("Set Visible"))->setData("invisible");
#endif
      if (e->flag(ELEMENT_HAS_TAG)) {
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
      popup->addAction(tr("Text Properties..."))->setData("text-props");
      }

//---------------------------------------------------------
//   createElementPropertyMenu
//---------------------------------------------------------

void ScoreView::createElementPropertyMenu(Element* e, QMenu* popup)
      {
      if (e->type() == Element::BAR_LINE) {
            genPropertyMenu1(e, popup);
            }
      else if (e->type() == Element::ARTICULATION) {
            genPropertyMenu1(e, popup);
            popup->addAction(tr("Articulation Properties..."))->setData("a-props");
            }
      else if (e->type() == Element::BEAM) {
            popup->addAction(getAction("flip"));
            }
      else if (e->type() == Element::STEM) {
            popup->addAction(getAction("flip"));
            }
      else if (e->type() == Element::HOOK) {
            popup->addAction(getAction("flip"));
            }
      else if (e->type() == Element::BEND) {
            genPropertyMenu1(e, popup);
            popup->addAction(tr("Bend Properties..."))->setData("b-props");
            }
      else if (e->type() == Element::TREMOLOBAR) {
            genPropertyMenu1(e, popup);
            popup->addAction(tr("TremoloBar Properties..."))->setData("tr-props");
            }
      else if (e->type() == Element::HBOX) {
            QMenu* textMenu = popup->addMenu(tr("Add"));
            textMenu->addAction(getAction("frame-text"));
            textMenu->addAction(getAction("picture"));
            popup->addAction(tr("Frame Properties..."))->setData("f-props");
            }
      else if (e->type() == Element::VBOX) {
            QMenu* textMenu = popup->addMenu(tr("Add"));
            textMenu->addAction(getAction("frame-text"));
            textMenu->addAction(getAction("title-text"));
            textMenu->addAction(getAction("subtitle-text"));
            textMenu->addAction(getAction("composer-text"));
            textMenu->addAction(getAction("poet-text"));
            textMenu->addAction(getAction("insert-hbox"));
            textMenu->addAction(getAction("picture"));
            popup->addAction(tr("Frame Properties..."))->setData("f-props");
            }
      else if (e->type() == Element::TBOX) {
            popup->addAction(tr("Frame Properties..."))->setData("f-props");
            }
      else if (e->type() == Element::VOLTA_SEGMENT) {
            genPropertyMenu1(e, popup);
            popup->addAction(tr("Volta Properties..."))->setData("v-props");
            }
      else if (e->type() == Element::TIMESIG) {
            genPropertyMenu1(e, popup);
            TimeSig* ts = static_cast<TimeSig*>(e);
            int _track = ts->track();
            // if the time sig. is not generated (= not courtesy) and is in track 0
            // add the specific menu item
            QAction* a;
            if (!ts->generated() && !_track && ts->measure() != score()->firstMeasure()) {
                  a = popup->addAction(ts->showCourtesySig()
                     ? QT_TRANSLATE_NOOP("TimeSig", "Hide Courtesy Time Signature")
                     : QT_TRANSLATE_NOOP("TimeSig", "Show Courtesy Time Signature") );
                  a->setData("ts-courtesy");
                  }
            popup->addSeparator();
            popup->addAction(tr("Time Signature Properties..."))->setData("ts-props");
            }
      else if (e->type() == Element::CLEF) {
            genPropertyMenu1(e, popup);
            Clef* clef = static_cast<Clef*>(e);
            // if the clef is not generated (= not courtesy) add the specific menu item
            if (!e->generated() && clef->measure() != score()->firstMeasure()) {
                  QAction* a = popup->addAction(static_cast<Clef*>(e)->showCourtesy()
                     ? QT_TRANSLATE_NOOP("Clef", "Hide courtesy clef")
                     : QT_TRANSLATE_NOOP("Clef", "Show courtesy clef") );
                        a->setData("clef-courtesy");
                  }
            }
      else if (e->type() == Element::DYNAMIC) {
            popup->addSeparator();
            if (e->visible())
                  popup->addAction(tr("Set Invisible"))->setData("invisible");
            else
                  popup->addAction(tr("Set Visible"))->setData("invisible");
            popup->addAction(tr("Text Properties..."))->setData("d-props");
            }
      else if (e->type() == Element::TEXTLINE_SEGMENT
                  || e->type() == Element::OTTAVA_SEGMENT
                  || e->type() == Element::VOLTA_SEGMENT
                  || e->type() == Element::PEDAL_SEGMENT) {
            if (e->visible())
                  popup->addAction(tr("Set Invisible"))->setData("invisible");
            else
                  popup->addAction(tr("Set Visible"))->setData("invisible");
            popup->addAction(tr("Line Properties..."))->setData("l-props");
            }
      else if (e->type() == Element::STAFF_TEXT) {
            genPropertyMenuText(e, popup);
            popup->addAction(tr("Staff Text Properties..."))->setData("st-props");
            }
      else if (e->type() == Element::TEXT
               || e->type() == Element::FINGERING
               || e->type() == Element::LYRICS
               || e->type() == Element::FIGURED_BASS) {
            genPropertyMenuText(e, popup);
            }
      else if (e->type() == Element::TEMPO_TEXT) {
            genPropertyMenu1(e, popup);
            popup->addAction(tr("Text Properties..."))->setData("text-props");
            }
      else if (e->type() == Element::KEYSIG) {
            genPropertyMenu1(e, popup);
            KeySig* ks = static_cast<KeySig*>(e);
            if (!e->generated() && ks->measure() != score()->firstMeasure()) {
                  QAction* a = popup->addAction(ks->showCourtesy()
                     ? QT_TRANSLATE_NOOP("KeySig", "Hide Courtesy Key Signature")
                     : QT_TRANSLATE_NOOP("KeySig", "Show Courtesy Key Signature") );
                  a->setData("key-courtesy");
                  a = popup->addAction(ks->showNaturals()
                     ? QT_TRANSLATE_NOOP("KeySig", "Hide Naturals")
                     : QT_TRANSLATE_NOOP("KeySig", "Show Naturals") );
                  a->setData("key-naturals");
                  }
            }
      else if (e->type() == Element::STAFF_STATE && static_cast<StaffState*>(e)->staffStateType() == STAFF_STATE_INSTRUMENT) {
            popup->addAction(tr("Change Instrument Properties..."))->setData("ss-props");
            }
      else if (e->type() == Element::SLUR_SEGMENT) {
            genPropertyMenu1(e, popup);
            popup->addAction(tr("Edit Mode"))->setData("edit");
            }
      else if (e->type() == Element::REST) {
            genPropertyMenu1(e, popup);
            }
      else if (e->type() == Element::NOTE) {
            QAction* b = popup->actions()[0];
            QAction* a = popup->insertSeparator(b);
            a->setText(tr("Staff"));
            a = new QAction(tr("Staff Properties..."), 0);
            a->setData("staff-props");
            popup->insertAction(b, a);

            a = popup->insertSeparator(b);
            a->setText(tr("Measure"));
            a = new QAction(tr("Measure Properties..."), 0);
            a->setData("measure-props");
            popup->insertAction(b, a);

            genPropertyMenu1(e, popup);
            popup->addSeparator();

            popup->addAction(tr("Style..."))->setData("style");
            popup->addAction(tr("Chord Articulation..."))->setData("articulation");
            }
      else if (e->type() == Element::LAYOUT_BREAK && static_cast<LayoutBreak*>(e)->layoutBreakType() == LayoutBreak::SECTION) {
            popup->addAction(tr("Section Break Properties..."))->setData("break-props");
            }
      else if (e->type() == Element::INSTRUMENT_CHANGE) {
            genPropertyMenu1(e, popup);
            popup->addAction(tr("Change Instrument..."))->setData("ch-instr");
            }
      else if (e->type() == Element::FRET_DIAGRAM) {
            if (e->visible())
                  popup->addAction(tr("Set Invisible"))->setData("invisible");
            else
                  popup->addAction(tr("Set Visible"))->setData("invisible");
            popup->addAction(tr("Color..."))->setData("color");
            popup->addAction(tr("Fret Diagram Properties..."))->setData("fret-props");
            }
      else if (e->type() == Element::GLISSANDO) {
            genPropertyMenu1(e, popup);
            popup->addAction(tr("Glissando Properties..."))->setData("gliss-props");
            }
      else if (e->type() == Element::HARMONY) {
            genPropertyMenu1(e, popup);
            popup->addSeparator();
            popup->addAction(tr("Harmony Properties..."))->setData("ha-props");
            }
      else if (e->type() == Element::INSTRUMENT_NAME) {
            popup->addAction(tr("Staff Properties..."))->setData("staff-props");
            }
      else
            genPropertyMenu1(e, popup);
      }

//---------------------------------------------------------
//   elementPropertyAction
//---------------------------------------------------------

void ScoreView::elementPropertyAction(const QString& cmd, Element* e)
      {
      if (cmd == "a-props") {
            ArticulationProperties rp(static_cast<Articulation*>(e));
            rp.exec();
            }
      else if (cmd == "b-props") {
            Bend* bend = static_cast<Bend*>(e);
            BendProperties bp(bend, 0);
            if (bp.exec())
                  score()->undo(new ChangeBend(bend, bp.points()));
            }
      else if (cmd == "f-props") {
            BoxProperties vp(static_cast<Box*>(e), 0);
            vp.exec();
            }
      else if (cmd == "measure-props") {
            MeasureProperties vp(static_cast<Note*>(e)->chord()->segment()->measure());
            vp.exec();
            }
      else if (cmd == "frame-text") {
            Text* s = new Text(score());
//            s->setSubtype(TEXT_FRAME);
            s->setTextStyleType(TEXT_STYLE_FRAME);
            s->setParent(e);
            score()->undoAddElement(s);
            score()->select(s, SELECT_SINGLE, 0);
            startEdit(s);
            score()->setLayoutAll(true);
            }
      else if (cmd == "picture") {
            mscore->addImage(score(), static_cast<HBox*>(e));
            }
      else if (cmd == "frame-text") {
            Text* t = new Text(score());
            t->setTextStyleType(TEXT_STYLE_FRAME);
            t->setParent(e);
            score()->undoAddElement(t);
            score()->select(t, SELECT_SINGLE, 0);
            startEdit(t);
            }
      else if (cmd == "title-text") {
            Text* t = new Text(score());
            t->setTextStyleType(TEXT_STYLE_TITLE);
            t->setParent(e);
            score()->undoAddElement(t);
            score()->select(t, SELECT_SINGLE, 0);
            startEdit(t);
            }
      else if (cmd == "subtitle-text") {
            Text* t = new Text(score());
            t->setTextStyleType(TEXT_STYLE_SUBTITLE);
            t->setParent(e);
            score()->undoAddElement(t);
            score()->select(t, SELECT_SINGLE, 0);
            startEdit(t);
            }
      else if (cmd == "composer-text") {
            Text* t = new Text(score());
            t->setTextStyleType(TEXT_STYLE_COMPOSER);
            t->setParent(e);
            score()->undoAddElement(t);
            score()->select(t, SELECT_SINGLE, 0);
            startEdit(t);
            }
      else if (cmd == "poet-text") {
            Text* t = new Text(score());
            t->setTextStyleType(TEXT_STYLE_POET);
            t->setParent(e);
            score()->undoAddElement(t);
            score()->select(t, SELECT_SINGLE, 0);
            startEdit(t);
            }
      else if (cmd == "insert-hbox") {
            HBox* s = new HBox(score());
            double w = width() - s->leftMargin() * MScore::DPMM - s->rightMargin() * MScore::DPMM;
            s->setBoxWidth(Spatium(w / s->spatium()));
            s->setParent(e);
            score()->undoAddElement(s);
            score()->select(s, SELECT_SINGLE, 0);
            startEdit(s);
            }
      else if (cmd == "picture")
            mscore->addImage(score(), e);
      else if (cmd == "v-props") {
            VoltaSegment* vs = static_cast<VoltaSegment*>(e);
            VoltaProperties vp;
            vp.setText(vs->volta()->text());
            vp.setEndings(vs->volta()->endings());
            int rv = vp.exec();
            if (rv) {
                  QString txt  = vp.getText();
                  QList<int> l = vp.getEndings();
                  if (txt != vs->volta()->text())
                        score()->undoChangeVoltaText(vs->volta(), txt);
                  if (l != vs->volta()->endings())
                        score()->undoChangeVoltaEnding(vs->volta(), l);
                  }
            }
      else if (cmd == "l-props") {
            TextLineSegment* vs = static_cast<TextLineSegment*>(e);
            TextLine* nTl  = vs->textLine()->clone();
            LineProperties lp(nTl);
            if (lp.exec()) {
                  score()->undoChangeElement(vs->textLine(), nTl);
                  // force new text
                  foreach(SpannerSegment* l, nTl->spannerSegments())
                        static_cast<TextLineSegment*>(l)->clearText();
                  }
            else
                  delete nTl;
            }
      else if (cmd == "tr-props") {
            TremoloBar* tb = static_cast<TremoloBar*>(e);
            TremoloBarProperties bp(tb, 0);
            if (bp.exec())
                  score()->undo(new ChangeTremoloBar(tb, bp.points()));
            }
      if (cmd == "ts-courtesy") {
            TimeSig* ts = static_cast<TimeSig*>(e);
            score()->undo(new ChangeTimesig(static_cast<TimeSig*>(e), !ts->showCourtesySig(), ts->sig(),
                  ts->stretch(), ts->numeratorString(), ts->denominatorString(), ts->timeSigType()));
            }
      else if (cmd == "ts-props") {
            TimeSig* ts = static_cast<TimeSig*>(e);
            TimeSig r(*ts);
            TimeSigProperties vp(&r);
            int rv = vp.exec();
            if (rv) {
                  bool stretchChanged = r.stretch() != ts->stretch();
                  if (r.numeratorString() != ts->numeratorString()
                     || r.denominatorString() != ts->denominatorString()
                     || r.sig() != ts->sig()
                     || stretchChanged
                     || !(r.groups() == ts->groups())
                     || r.timeSigType() != ts->timeSigType()) {
                        score()->undo(new ChangeTimesig(ts, r.showCourtesySig(), r.sig(), r.stretch(),
                           r.numeratorString(), r.denominatorString(), r.timeSigType()));
                        if (stretchChanged)
                              score()->timesigStretchChanged(ts, ts->measure(), ts->staffIdx());
                        if (!(r.groups() == ts->groups()))
                              ts->undoSetGroups(r.groups());
                        }
                  }
            }
      else if (cmd == "smallNote")
            score()->undoChangeProperty(e, P_SMALL, !static_cast<Note*>(e)->small());
      else if (cmd == "clef-courtesy") {
            bool show = !static_cast<Clef*>(e)->showCourtesy();
            score()->undoChangeProperty(e, P_SHOW_COURTESY, show);
            }
      else if (cmd == "d-props") {
            Dynamic* dynamic = static_cast<Dynamic*>(e);
            Dynamic* nText = new Dynamic(*dynamic);
            TextProperties tp(nText, 0);
            int rv = tp.exec();
            if (rv)
                  score()->undoChangeElement(dynamic, nText);
            else
                  delete nText;
            }
      else if (cmd == "st-props") {
            StaffTextProperties rp(static_cast<StaffText*>(e));
            rp.exec();
            }
      else if (cmd == "text-props") {
            Text* ot    = static_cast<Text*>(e);
            Text* nText = static_cast<Text*>(ot->clone());
            TextProperties tp(nText);
            int rv = tp.exec();
            if (rv) {
                  QList<Element*> sl = score()->selection().elements();
                  QList<Element*> selectedElements;
                  foreach(Element* e, sl) {
                        if (e->type() != ot->type())
                              continue;

                        Text* t  = static_cast<Text*>(e);
                        Text* tt = t->clone();

                        if (nText->styled() != ot->styled() || nText->styled()) {
                              if (nText->styled())
                                    tt->setTextStyleType(nText->textStyleType());
                              else
                                    tt->setUnstyled();
                              tt->setModified(true);
                              }

                        if (!nText->styled() && (nText->textStyle() != ot->textStyle())) {
                              tt->setTextStyle(nText->textStyle());
                              tt->textStyleChanged();
                              tt->setModified(true);
                              }

                        if (t->selected())
                              selectedElements.append(tt);
                        score()->undoChangeElement(t, tt);
                        }
                  score()->select(0, SELECT_SINGLE, 0);
                  foreach(Element* e, selectedElements)
                        score()->select(e, SELECT_ADD, 0);
                  }
            delete nText;
            }
      else if (cmd == "key-courtesy") {
            KeySig* ks = static_cast<KeySig*>(e);
            score()->undo(new ChangeKeySig(ks, ks->keySigEvent(), !ks->showCourtesy(), ks->showNaturals()));
            }
      else if (cmd == "key-naturals") {
            KeySig* ks = static_cast<KeySig*>(e);
            score()->undo(new ChangeKeySig(ks, ks->keySigEvent(), ks->showCourtesy(), !ks->showNaturals()));
            }
      else if (cmd == "ss-props") {
            StaffState* ss = static_cast<StaffState*>(e);
            SelectInstrument si(ss->instrument(), 0);
            if (si.exec()) {
                  const InstrumentTemplate* it = si.instrTemplate();
                  if (it) {
                        // TODO: undo/redo
                        ss->setInstrument(Instrument::fromTemplate(it));
                        ss->staff()->part()->setInstrument(ss->instrument(), ss->segment()->tick());
                        score()->rebuildMidiMapping();
                        seq->initInstruments();
                        score()->setLayoutAll(true);
                        }
                  else
                        qDebug("no template selected?\n");
                  }
            }
      else if (cmd == "articulation") {
            Note* note = static_cast<Note*>(e);
            mscore->editInPianoroll(note->staff());
            }
      else if (cmd == "style") {
            EditStyle es(e->score(), 0);
            es.setPage(EditStyle::PAGE_NOTE);
            es.exec();
            }
      else if (cmd == "break-props") {
            LayoutBreak* lb = static_cast<LayoutBreak*>(e);
            SectionBreakProperties sbp(lb, 0);
            if (sbp.exec()) {
                  if (lb->pause() != sbp.pause()
                     || lb->startWithLongNames() != sbp.startWithLongNames()
                     || lb->startWithMeasureOne() != sbp.startWithMeasureOne()) {
                        LayoutBreak* nlb = new LayoutBreak(*lb);
                        nlb->setParent(lb->parent());
                        nlb->setPause(sbp.pause());
                        nlb->setStartWithLongNames(sbp.startWithLongNames());
                        nlb->setStartWithMeasureOne(sbp.startWithMeasureOne());
                        score()->undoChangeElement(lb, nlb);
                        }
                  }
            }
      else if (cmd == "ch-instr") {
            InstrumentChange* ic = static_cast<InstrumentChange*>(e);
            SelectInstrument si(ic->instrument(), 0);
            if (si.exec()) {
                  const InstrumentTemplate* it = si.instrTemplate();
                  if (it) {
                        ic->setInstrument(Instrument::fromTemplate(it));
                        score()->undo(new ChangeInstrument(ic, ic->instrument()));
                        }
                  else
                        qDebug("no template selected?\n");
                  }
           }
      else if (cmd == "fret-props") {
            FretDiagram* fd = static_cast<FretDiagram*>(e);
            FretDiagram* nFret = const_cast<FretDiagram*>(fd->clone());
            FretDiagramProperties fp(nFret, 0);
            int rv = fp.exec();
            if (rv) {
                  nFret->layout();
                  score()->undoChangeElement(fd, nFret);
                  return;
                  }
            delete nFret;
            }
      else if (cmd == "gliss-props") {
            GlissandoProperties vp(static_cast<Glissando*>(e));
            vp.exec();
            }
       else if (cmd == "ha-props") {
            Harmony* ha = static_cast<Harmony*>(e);
            ChordEdit ce(ha->score());
            ce.setHarmony(ha);
            int rv = ce.exec();
            if (rv) {
                  Harmony* h = ce.harmony()->clone();
                  h->render();
                  score()->undoChangeElement(ha, h);
                  }
            }
      else if (cmd == "staff-props") {
            EditStaff editStaff(e->staff(), 0);
            connect(&editStaff, SIGNAL(instrumentChanged()), mscore, SLOT(instrumentChanged()));
            editStaff.exec();
            }
      else if (cmd == "invisible")
            score()->undoChangeInvisible(e, !e->visible());
      else if (cmd == "color")
            score()->colorItem(e);
      else if (cmd.startsWith("layer-")) {
            int n = cmd.mid(6).toInt();
            uint mask = 1 << n;
            e->setTag(mask);
            }
      }
}

