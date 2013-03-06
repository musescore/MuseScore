//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspector.h"
#include "inspectorBeam.h"
#include "inspectorImage.h"
#include "inspectorLasso.h"
#include "inspectorGroupElement.h"
#include "inspectorVolta.h"
#include "inspectorOttava.h"
#include "inspectorTrill.h"
#include "inspectorHairpin.h"
#include "inspectorMarker.h"
#include "inspectorJump.h"
#include "inspectorGlissando.h"
#include "musescore.h"
#include "scoreview.h"

#include "libmscore/element.h"
#include "libmscore/score.h"
#include "libmscore/box.h"
#include "libmscore/undo.h"
#include "libmscore/spacer.h"
#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "libmscore/segment.h"
#include "libmscore/rest.h"
#include "libmscore/beam.h"
#include "libmscore/clef.h"
#include "libmscore/notedot.h"
#include "libmscore/hook.h"
#include "libmscore/stem.h"
#include "libmscore/keysig.h"
#include "libmscore/barline.h"
#include "libmscore/staff.h"
#include "libmscore/measure.h"

//---------------------------------------------------------
//   showInspector
//---------------------------------------------------------

void MuseScore::showInspector(bool visible)
      {
      QAction* a = getAction("inspector");
      if (visible) {
            if (!inspector) {
                  inspector = new Inspector();
                  connect(inspector, SIGNAL(inspectorVisible(bool)), a, SLOT(setChecked(bool)));
                  addDockWidget(Qt::RightDockWidgetArea, inspector);
                  }
            updateInspector();
            }
      if (inspector)
            inspector->setVisible(visible);
      a->setChecked(visible);
      }

//---------------------------------------------------------
//   Inspector
//---------------------------------------------------------

Inspector::Inspector(QWidget* parent)
   : QDockWidget(tr("Inspector"), parent)
      {
      setObjectName("inspector");
      setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
      QScrollArea* sa = new QScrollArea;
      sa->setWidgetResizable(true);
      QWidget* mainWidget = new QWidget;
      mainWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); //??
      setWidget(sa);
      sa->setWidget(mainWidget);

      layout = new QVBoxLayout;
      layout->setSpacing(0);
      mainWidget->setLayout(layout);
      _inspectorEdit = false;
      ie             = 0;
      _element       = 0;
      layout->addStretch(10);
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void Inspector::closeEvent(QCloseEvent* ev)
      {
      emit inspectorVisible(false);
      QWidget::closeEvent(ev);
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Inspector::reset()
      {
      if (ie)
            ie->setElement();
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void Inspector::setElement(Element* e)
      {
      QList<Element*> el;
      if (e)
            el.append(e);
      setElements(el);
      }

//---------------------------------------------------------
//   setElements
//---------------------------------------------------------

void Inspector::setElements(const QList<Element*>& l)
      {
      if (_inspectorEdit) {          // if within an inspector-originated edit
            _inspectorEdit = false;  // reset flag
            if (_el == l)            // if element is not changing...
                  return;            // ...do nothing
            }
      Element* e = l.isEmpty() ? 0 : l[0];
      if (e == 0 || _element == 0 || (_el != l)) {
            _el = l;
            if (ie)
                  ie->deleteLater();
            ie = 0;
            _element = e;

            if (_element == 0)
                  return;

            bool sameTypes = true;
            foreach(Element* ee, _el) {
                  if (_element->type() != ee->type())
                        sameTypes = false;
                  }
            if (!sameTypes)
                  ie = new InspectorGroupElement(this);
            else {
                  switch(_element->type()) {
                        case Element::FBOX:
                        case Element::TBOX:
                        case Element::VBOX:
                              ie = new InspectorVBox(this);
                              break;
                        case Element::HBOX:
                              ie = new InspectorHBox(this);
                              break;
                        case Element::ARTICULATION:
                              ie = new InspectorArticulation(this);
                              break;
                        case Element::SPACER:
                              ie = new InspectorSpacer(this);
                              break;
                        case Element::NOTE:
                              ie = new InspectorNote(this);
                              break;
                        case Element::REST:
                              ie = new InspectorRest(this);
                              break;
                        case Element::CLEF:
                              ie = new InspectorClef(this);
                              break;
                        case Element::TIMESIG:
                              ie = new InspectorTimeSig(this);
                              break;
                        case Element::KEYSIG:
                              ie = new InspectorKeySig(this);
                              break;
                        case Element::BEAM:
                              ie = new InspectorBeam(this);
                              break;
                        case Element::IMAGE:
                              ie = new InspectorImage(this);
                              break;
                        case Element::LASSO:
                              ie = new InspectorLasso(this);
                              break;
                        case Element::VOLTA_SEGMENT:
                              ie = new InspectorVolta(this);
                              break;
                        case Element::OTTAVA_SEGMENT:
                              ie = new InspectorOttava(this);
                              break;
                        case Element::TRILL_SEGMENT:
                              ie = new InspectorTrill(this);
                              break;
                        case Element::HAIRPIN_SEGMENT:
                              ie = new InspectorHairpin(this);
                              break;
                        case Element::BAR_LINE:
                              ie = new InspectorBarLine(this);
                              break;
                        case Element::JUMP:
                              ie = new InspectorJump(this);
                              break;
                        case Element::MARKER:
                              ie = new InspectorMarker(this);
                              break;
                        case Element::GLISSANDO:
                              ie = new InspectorGlissando(this);
                              break;
                        default:
                              ie = new InspectorElement(this);
                              break;
                        }
                  }
            layout->insertWidget(0, ie);
            }
      _element = e;
      ie->setElement();
      }

//---------------------------------------------------------
//   InspectorElementElement
//---------------------------------------------------------

InspectorElementElement::InspectorElementElement(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);
      connect(color,        SIGNAL(colorChanged(QColor)), SLOT(colorChanged(QColor)));
      connect(offsetX,      SIGNAL(valueChanged(double)), SLOT(offsetXChanged(double)));
      connect(offsetY,      SIGNAL(valueChanged(double)), SLOT(offsetYChanged(double)));
      connect(visible,      SIGNAL(stateChanged(int)),    SLOT(apply()));
      connect(resetColor,   SIGNAL(clicked()), SLOT(resetColorClicked()));
      connect(resetX,       SIGNAL(clicked()), SLOT(resetXClicked()));
      connect(resetY,       SIGNAL(clicked()), SLOT(resetYClicked()));
      connect(resetVisible, SIGNAL(clicked()), SLOT(resetVisibleClicked()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorElementElement::setElement(Element* element)
      {
      e = element;
      elementName->setText(e->name());
      qreal _spatium = e->score()->spatium();

      color->blockSignals(true);
      offsetX->blockSignals(true);
      offsetY->blockSignals(true);
      visible->blockSignals(true);

      color->setColor(e->color());
      offsetX->setValue(e->pos().x() / _spatium);
      offsetY->setValue(e->pos().y() / _spatium);
      resetColor->setEnabled(e->color() != MScore::defaultColor);
      resetX->setEnabled(e->userOff().x() != 0.0);
      resetY->setEnabled(e->userOff().y() != 0.0);
      visible->setChecked(e->visible());

      visible->blockSignals(false);
      color->blockSignals(false);
      offsetX->blockSignals(false);
      offsetY->blockSignals(false);

      resetVisible->setEnabled(!e->visible());
      }

//---------------------------------------------------------
//   colorChanged
//---------------------------------------------------------

void InspectorElementElement::colorChanged(QColor)
      {
      resetColor->setEnabled(color->color() != MScore::defaultColor);
      apply();
      }

//---------------------------------------------------------
//   offsetXChanged
//---------------------------------------------------------

void InspectorElementElement::offsetXChanged(double)
      {
      resetX->setEnabled(offsetX->value() != e->ipos().x());
      apply();
      }

//---------------------------------------------------------
//   offsetYChanged
//---------------------------------------------------------

void InspectorElementElement::offsetYChanged(double)
      {
      resetY->setEnabled(offsetY->value() != e->ipos().y());
      apply();
      }

//---------------------------------------------------------
//   resetColorClicked
//---------------------------------------------------------

void InspectorElementElement::resetColorClicked()
      {
      color->setColor(MScore::defaultColor);
      resetColor->setEnabled(false);
      apply();
      }

//---------------------------------------------------------
//   resetXClicked
//---------------------------------------------------------

void InspectorElementElement::resetXClicked()
      {
      qreal _spatium = e->score()->spatium();
      offsetX->setValue(e->ipos().x() / _spatium);
      resetX->setEnabled(false);
      apply();
      }

//---------------------------------------------------------
//   resetVisibleClicked
//---------------------------------------------------------

void InspectorElementElement::resetVisibleClicked()
      {
      visible->setChecked(true);
      resetVisible->setEnabled(false);
      apply();
      }

//---------------------------------------------------------
//   resetTrailingSpace
//---------------------------------------------------------

void InspectorElementElement::resetYClicked()
      {
      qreal _spatium = e->score()->spatium();
      offsetY->setValue(e->ipos().y() / _spatium);
      resetY->setEnabled(false);
      apply();
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorElementElement::apply()
      {
      resetVisible->setEnabled(!visible->isChecked());

      qreal _spatium = e->score()->spatium();
      if (offsetX->value()        == (e->pos().x() / _spatium)
         &&  offsetY->value()     == (e->pos().y() / _spatium)
         &&  color->color()       == e->color()
         &&  visible->isChecked() == e->visible())
            return;

      mscore->getInspector()->setInspectorEdit(true); // this edit is coming from within the inspector itself:
                                                      // do not set element values again
      if (mscore->state() == STATE_EDIT) {
            if (e->color() != color->color()) {
                  ChangeProperty cp(e, P_COLOR, color->color());
                  cp.redo();
                  e->score()->update();
                  }
            }
      else {
            Score* score = e->score();
            score->startCmd();
            QPointF o(offsetX->value() * _spatium, offsetY->value() * _spatium);
            if (o != e->pos())
                  score->undoChangeProperty(e, P_USER_OFF, o - e->ipos());
            if (e->color() != color->color())
                  score->undoChangeProperty(e, P_COLOR, color->color());
            if (e->visible() != visible->isChecked())
                  score->undoChangeProperty(e, P_VISIBLE, visible->isChecked());
            score->endCmd();
            mscore->endCmd();
            }
      }

//---------------------------------------------------------
//   InspectorElement
//---------------------------------------------------------

InspectorElement::InspectorElement(QWidget* parent)
   : InspectorBase(parent)
      {
      QWidget* w = new QWidget;
      b.setupUi(w);
      layout->addWidget(w);

      iList = {
            { P_COLOR,    0, false, b.color,      b.resetColor   },
            { P_VISIBLE,  0, false, b.visible,    b.resetVisible },
            { P_USER_OFF, 0, false, b.offsetX,    b.resetX       },
            { P_USER_OFF, 1, false, b.offsetY,    b.resetY       }
            };

      mapSignals();
      }

//---------------------------------------------------------
//   InspectorVBox
//---------------------------------------------------------

InspectorVBox::InspectorVBox(QWidget* parent)
   : InspectorBase(parent)
      {
      QWidget* w = new QWidget;
      vb.setupUi(w);
      layout->addWidget(w);

      iList = {
            { P_TOP_GAP,       0, false, vb.topGap,       vb.resetTopGap       },
            { P_BOTTOM_GAP,    0, false, vb.bottomGap,    vb.resetBottomGap    },
            { P_LEFT_MARGIN,   0, false, vb.leftMargin,   vb.resetLeftMargin   },
            { P_RIGHT_MARGIN,  0, false, vb.rightMargin,  vb.resetRightMargin  },
            { P_TOP_MARGIN,    0, false, vb.topMargin,    vb.resetTopMargin    },
            { P_BOTTOM_MARGIN, 0, false, vb.bottomMargin, vb.resetBottomMargin },
            { P_BOX_HEIGHT,    0, false, vb.height,       0                    }
            };
      mapSignals();
      }

//---------------------------------------------------------
//   InspectorHBox
//---------------------------------------------------------

InspectorHBox::InspectorHBox(QWidget* parent)
   : InspectorBase(parent)
      {
      QWidget* w = new QWidget;
      hb.setupUi(w);
      layout->addWidget(w);

      iList = {
            { P_TOP_GAP,    0, false, hb.leftGap,  hb.resetLeftGap  },
            { P_BOTTOM_GAP, 0, false, hb.rightGap, hb.resetRightGap },
            { P_BOX_WIDTH,  0, false, hb.width,    0                }
            };

      mapSignals();
      }

//---------------------------------------------------------
//   InspectorArticulation
//---------------------------------------------------------

InspectorArticulation::InspectorArticulation(QWidget* parent)
   : InspectorBase(parent)
      {
      QWidget* w1 = new QWidget;
      e.setupUi(w1);
      layout->addWidget(w1);
      QWidget* w = new QWidget;
      ar.setupUi(w);
      layout->addWidget(w);

      iList = {
            { P_COLOR,               0, false, e.color,        e.resetColor      },
            { P_VISIBLE,             0, false, e.visible,      e.resetVisible    },
            { P_USER_OFF,            0, false, e.offsetX,      e.resetX          },
            { P_USER_OFF,            1, false, e.offsetY,      e.resetY          },
            { P_ARTICULATION_ANCHOR, 0, false, ar.anchor,      ar.resetAnchor    },
            { P_DIRECTION,           0, false, ar.direction,   ar.resetDirection }
            };
      mapSignals();
      }

//---------------------------------------------------------
//   InspectorSpacer
//---------------------------------------------------------

InspectorSpacer::InspectorSpacer(QWidget* parent)
   : InspectorBase(parent)
      {
      QWidget* w = new QWidget;
      sp.setupUi(w);
      layout->addWidget(w);

      iList = {
            { P_SPACE, 0, false, sp.height, sp.resetHeight  }
            };
      mapSignals();
      }

//---------------------------------------------------------
//   InspectorSegment
//---------------------------------------------------------

InspectorSegment::InspectorSegment(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);
      connect(leadingSpace,       SIGNAL(valueChanged(double)), SLOT(leadingSpaceChanged(double)));
      connect(trailingSpace,      SIGNAL(valueChanged(double)), SLOT(trailingSpaceChanged(double)));
      connect(resetLeadingSpace,  SIGNAL(clicked()), SLOT(resetLeadingSpaceClicked()));
      connect(resetTrailingSpace, SIGNAL(clicked()), SLOT(resetTrailingSpaceClicked()));
      }

//---------------------------------------------------------
//   dirty
//---------------------------------------------------------

bool InspectorSegment::dirty() const
      {
      return segment->extraLeadingSpace().val() != leadingSpace->value()
         || segment->extraTrailingSpace().val() != trailingSpace->value();
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorSegment::setElement(Segment* s)
      {
      segment = s;
      leadingSpace->setValue(segment->extraLeadingSpace().val());
      trailingSpace->setValue(segment->extraTrailingSpace().val());
      resetLeadingSpace->setEnabled(leadingSpace->value() != 0.0);
      resetTrailingSpace->setEnabled(leadingSpace->value() != 0.0);
      }

//---------------------------------------------------------
//   leadingSpaceChanged
//---------------------------------------------------------

void InspectorSegment::leadingSpaceChanged(double)
      {
      resetLeadingSpace->setEnabled(leadingSpace->value() != 0.0);
      apply();
      }

//---------------------------------------------------------
//   trailingSpaceChanged
//---------------------------------------------------------

void InspectorSegment::trailingSpaceChanged(double)
      {
      resetTrailingSpace->setEnabled(trailingSpace->value() != 0.0);
      apply();
      }

//---------------------------------------------------------
//   resetLeadingSpace
//---------------------------------------------------------

void InspectorSegment::resetLeadingSpaceClicked()
      {
      leadingSpace->setValue(0.0);
      apply();
      }

//---------------------------------------------------------
//   resetTrailingSpace
//---------------------------------------------------------

void InspectorSegment::resetTrailingSpaceClicked()
      {
      trailingSpace->setValue(0.0);
      apply();
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorSegment::apply()
      {
      if (!dirty())
            return;
      mscore->getInspector()->setInspectorEdit(true); // this edit is coming from within the inspector itself:
                                                      // do not set element values again
      Score* score = segment->score();
      score->startCmd();
      qreal val = leadingSpace->value();
      if (segment->extraLeadingSpace().val() != val)
            segment->score()->undoChangeProperty(segment, P_LEADING_SPACE, val);
      val = trailingSpace->value();
      if (segment->extraTrailingSpace().val() != val)
            segment->score()->undoChangeProperty(segment, P_TRAILING_SPACE, val);
      score->endCmd();
      mscore->endCmd();
      }

static const int heads[] = {
      Note::HEAD_NORMAL, Note::HEAD_CROSS, Note::HEAD_DIAMOND, Note::HEAD_TRIANGLE,
      Note::HEAD_SLASH, Note::HEAD_XCIRCLE, Note::HEAD_DO, Note::HEAD_RE, Note::HEAD_MI, Note::HEAD_FA,
      Note::HEAD_SOL, Note::HEAD_LA, Note::HEAD_TI,
      Note::HEAD_BREVIS_ALT
      };

//---------------------------------------------------------
//   InspectorNoteBase
//---------------------------------------------------------

InspectorNoteBase::InspectorNoteBase(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);
      //
      // fix order of note heads
      //
      for (int i = 0; i < Note::HEAD_GROUPS; ++i) {
            noteHeadGroup->setItemData(i, QVariant(heads[i]));
            }
      connect(small,              SIGNAL(stateChanged(int)),        SLOT(smallChanged(int)));
      connect(mirrorHead,         SIGNAL(currentIndexChanged(int)), SLOT(mirrorHeadChanged(int)));
      connect(dotPosition,        SIGNAL(currentIndexChanged(int)), SLOT(dotPositionChanged(int)));
      connect(ontimeOffset,       SIGNAL(valueChanged(int)),        SLOT(ontimeOffsetChanged(int)));
      connect(offtimeOffset,      SIGNAL(valueChanged(int)),        SLOT(offtimeOffsetChanged(int)));
      connect(resetSmall,         SIGNAL(clicked()), SLOT(resetSmallClicked()));
      connect(resetMirrorHead,    SIGNAL(clicked()), SLOT(resetMirrorClicked()));
      connect(resetDotPosition,   SIGNAL(clicked()), SLOT(resetDotPositionClicked()));
      connect(resetOntimeOffset,  SIGNAL(clicked()), SLOT(resetOntimeOffsetClicked()));
      connect(resetOfftimeOffset, SIGNAL(clicked()), SLOT(resetOfftimeOffsetClicked()));

      connect(noteHeadGroup,      SIGNAL(currentIndexChanged(int)), SLOT(noteHeadGroupChanged(int)));
      connect(noteHeadType,       SIGNAL(currentIndexChanged(int)), SLOT(noteHeadTypeChanged(int)));
      connect(tuning,             SIGNAL(valueChanged(double)),     SLOT(tuningChanged(double)));
      connect(velocityType,       SIGNAL(currentIndexChanged(int)), SLOT(velocityTypeChanged(int)));
      connect(velocity,           SIGNAL(valueChanged(int)),        SLOT(velocityChanged(int)));

      connect(resetNoteHeadGroup, SIGNAL(clicked()), SLOT(resetNoteHeadGroupClicked()));
      connect(resetNoteHeadType,  SIGNAL(clicked()), SLOT(resetNoteHeadTypeClicked()));
      connect(resetTuning,        SIGNAL(clicked()), SLOT(resetTuningClicked()));
      connect(resetVelocityType,  SIGNAL(clicked()), SLOT(resetVelocityTypeClicked()));
      }

//---------------------------------------------------------
//   block
//---------------------------------------------------------

void InspectorNoteBase::block(bool val)
      {
      small->blockSignals(val);
      mirrorHead->blockSignals(val);
      dotPosition->blockSignals(val);
      ontimeOffset->blockSignals(val);
      offtimeOffset->blockSignals(val);
      noteHeadGroup->blockSignals(val);
      noteHeadType->blockSignals(val);
      tuning->blockSignals(val);
      velocity->blockSignals(val);
      velocityType->blockSignals(val);
      resetSmall->blockSignals(val);
      resetMirrorHead->blockSignals(val);
      resetDotPosition->blockSignals(val);
      resetOntimeOffset->blockSignals(val);
      resetOfftimeOffset->blockSignals(val);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorNoteBase::setElement(Note* n)
      {
      _userVelocity = 0;
      _veloOffset   = 0;
      note          = n;

      block(true);
      small->setChecked(note->small());
      mirrorHead->setCurrentIndex(note->userMirror());
      dotPosition->setCurrentIndex(note->dotPosition());
//      ontimeOffset->setValue(note->onTimeUserOffset());
//      offtimeOffset->setValue(note->offTimeUserOffset());

      int headGroup = note->headGroup();
      int headGroupIndex = 0;
      for (int i = 0; i < Note::HEAD_GROUPS; ++i) {
            noteHeadGroup->setItemData(i, QVariant(heads[i]));
            if (headGroup == heads[i])
                  headGroupIndex = i;
            }
      noteHeadGroup->setCurrentIndex(headGroupIndex);
      noteHeadType->setCurrentIndex(int(note->headType())+1);   // NoteHeadType goes from -1 while combo box goes from 0
      tuning->setValue(note->tuning());
      int val = note->veloOffset();
      velocity->setValue(val);
      velocityType->setCurrentIndex(int(note->veloType()));
      if (note->veloType() == MScore::USER_VAL)
            _userVelocity = val;
      else
            _veloOffset = val;

      resetSmall->setEnabled(note->small());
      resetMirrorHead->setEnabled(note->userMirror() != MScore::DH_AUTO);
      resetDotPosition->setEnabled(note->dotPosition() != MScore::AUTO);
//      resetOntimeOffset->setEnabled(note->onTimeUserOffset());
//      resetOfftimeOffset->setEnabled(note->offTimeUserOffset());
      block(false);
      }

//---------------------------------------------------------
//   dirty
//---------------------------------------------------------

bool InspectorNoteBase::dirty() const
      {
      return note->small()            != small->isChecked()
         || note->userMirror()        != mirrorHead->currentIndex()
         || note->dotPosition()       != dotPosition->currentIndex()
//         || note->onTimeUserOffset()  != ontimeOffset->value()
//         || note->offTimeUserOffset() != offtimeOffset->value()
         || note->headGroup()         != noteHeadGroup->itemData(noteHeadGroup->currentIndex())
         || note->headType()          != (noteHeadType->currentIndex()-1)     // NoteHeadType goes from -1 while combo box goes from 0
         || note->tuning()            != tuning->value()
         || note->veloOffset()        != velocity->value()
         || note->veloType()          != velocityType->currentIndex()
         ;
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorNoteBase::apply()
      {
      mscore->getInspector()->setInspectorEdit(true); // this edit is coming from within the inspector itself:
                                                      // do not set element values again
      Score* score = note->score();
      score->startCmd();
      bool b = small->isChecked();
      if (note->small() != b)
            score->undoChangeProperty(note, P_SMALL, b);
      int val = mirrorHead->currentIndex();
      if (note->userMirror() != val)
            score->undoChangeProperty(note, P_MIRROR_HEAD, val);
      val = dotPosition->currentIndex();
      if (note->dotPosition() != val)
            score->undoChangeProperty(note, P_DOT_POSITION, val);
//      val = ontimeOffset->value();
//      if (note->onTimeOffset() != val)
//            score->undoChangeProperty(note, P_ONTIME_OFFSET, val);
//      val = offtimeOffset->value();
//      if (note->offTimeOffset() != val)
//            score->undoChangeProperty(note, P_OFFTIME_OFFSET, val);
      val = noteHeadGroup->itemData(noteHeadGroup->currentIndex()).toInt();
      if (note->headGroup() != val)
            score->undoChangeProperty(note, P_HEAD_GROUP, val);
      val = noteHeadType->currentIndex()-1;                       // NoteHeadType goes from -1 while combo box goes from 0
      if (note->headType() != val)
            score->undoChangeProperty(note, P_HEAD_TYPE, val);
      if (note->tuning() != tuning->value())
            score->undoChangeProperty(note, P_TUNING, tuning->value());
      if (note->veloOffset() != velocity->value())
            score->undoChangeProperty(note, P_VELO_OFFSET, velocity->value());
      if (note->veloType() != velocityType->currentIndex())
            score->undoChangeProperty(note, P_VELO_TYPE, velocityType->currentIndex());
      score->endCmd();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   smallChanged
//---------------------------------------------------------

void InspectorNoteBase::smallChanged(int)
      {
      resetSmall->setEnabled(small->isChecked());
      apply();
      }

//---------------------------------------------------------
//   mirrorHeadChanged
//---------------------------------------------------------

void InspectorNoteBase::mirrorHeadChanged(int)
      {
      resetMirrorHead->setEnabled(note->userMirror() != MScore::DH_AUTO);
      apply();
      }

//---------------------------------------------------------
//   dotPositionChanged
//---------------------------------------------------------

void InspectorNoteBase::dotPositionChanged(int)
      {
      resetDotPosition->setEnabled(note->dotPosition() != MScore::AUTO);
      apply();
      }

//---------------------------------------------------------
//   ontimeOffsetChanged
//---------------------------------------------------------

void InspectorNoteBase::ontimeOffsetChanged(int)
      {
//      resetOntimeOffset->setEnabled(note->onTimeUserOffset());
      apply();
      }

//---------------------------------------------------------
//   offtimeOffsetChanged
//---------------------------------------------------------

void InspectorNoteBase::offtimeOffsetChanged(int)
      {
//      resetOfftimeOffset->setEnabled(note->offTimeUserOffset());
      apply();
      }

//---------------------------------------------------------
//   noteHeadGroupChanged
//---------------------------------------------------------

void InspectorNoteBase::noteHeadGroupChanged(int val)
      {
      resetNoteHeadGroup->setEnabled(val != 0);
      apply();
      }

//---------------------------------------------------------
//   noteHeadTypeChanged
//---------------------------------------------------------

void InspectorNoteBase::noteHeadTypeChanged(int val)
      {
      resetNoteHeadType->setEnabled(val != 0);
      apply();
      }

//---------------------------------------------------------
//   tuningChanged
//---------------------------------------------------------

void InspectorNoteBase::tuningChanged(double val)
      {
      resetTuning->setEnabled(val != 0.0);
      apply();
      }

//---------------------------------------------------------
//   velocityTypeChanged
//---------------------------------------------------------

void InspectorNoteBase::velocityTypeChanged(int val)
      {
      switch(val) {
            case MScore::USER_VAL:
                  velocity->setEnabled(true);
                  velocity->setSuffix("");
                  velocity->setRange(0, 127);
                  velocity->setValue(_userVelocity);
                  break;
            case MScore::OFFSET_VAL:
                  velocity->setEnabled(true);
                  velocity->setSuffix("%");
                  velocity->setRange(-200, 200);
                  velocity->setValue(_veloOffset);
                  break;
            }
      resetVelocityType->setEnabled(val != 0);
      apply();
      }

//---------------------------------------------------------
//   velocityChanged
//---------------------------------------------------------

void InspectorNoteBase::velocityChanged(int val)
      {
      if (velocityType->currentIndex() == MScore::USER_VAL)
            _userVelocity = val;
      else
            _veloOffset = val;
      apply();
      }

//---------------------------------------------------------
//   resetSmall
//---------------------------------------------------------

void InspectorNoteBase::resetSmallClicked()
      {
      small->setChecked(false);
      }

//---------------------------------------------------------
//   resetMirrorClicked
//---------------------------------------------------------

void InspectorNoteBase::resetMirrorClicked()
      {
      mirrorHead->setCurrentIndex(0);
      }

//---------------------------------------------------------
//   resetDotPositionClicked
//---------------------------------------------------------

void InspectorNoteBase::resetDotPositionClicked()
      {
      dotPosition->setCurrentIndex(0);
      }

//---------------------------------------------------------
//   resetOntimeOffsetClicked
//---------------------------------------------------------

void InspectorNoteBase::resetOntimeOffsetClicked()
      {
      ontimeOffset->setValue(0);
      }

//---------------------------------------------------------
//   resetOfftimeOffsetClicked
//---------------------------------------------------------

void InspectorNoteBase::resetOfftimeOffsetClicked()
      {
      offtimeOffset->setValue(0);
      }

//---------------------------------------------------------
//   resetNoteHeadGroupClicked
//---------------------------------------------------------

void InspectorNoteBase::resetNoteHeadGroupClicked()
      {
      noteHeadGroup->setCurrentIndex(0);
      }

//---------------------------------------------------------
//   resetNoteHeadTypeClicked
//---------------------------------------------------------

void InspectorNoteBase::resetNoteHeadTypeClicked()
      {
      noteHeadType->setCurrentIndex(0);
      }

//---------------------------------------------------------
//   resetTuningClicked
//---------------------------------------------------------

void InspectorNoteBase::resetTuningClicked()
      {
      tuning->setValue(0.0);
      }

//---------------------------------------------------------
//   resetVelocityTypeClicked
//---------------------------------------------------------

void InspectorNoteBase::resetVelocityTypeClicked()
      {
      velocityType->setCurrentIndex(0);
      }

//---------------------------------------------------------
//   InspectorNote
//---------------------------------------------------------

InspectorNote::InspectorNote(QWidget* parent)
   : InspectorBase(parent)
      {
      iElement = new InspectorElementElement(this);
      layout->addWidget(iElement);

      iNote    = new InspectorNoteBase(this);
      layout->addWidget(iNote);

      iChord = new InspectorChord(this);
      layout->addWidget(iChord);

      iSegment = new InspectorSegment(this);
      layout->addWidget(iSegment);

      layout->addSpacing(20);

      //
      // Select
      //
      QLabel* l = new QLabel;
      l->setText(tr("Select"));
      QFont font(l->font());
      font.setBold(true);
      l->setFont(font);
      l->setAlignment(Qt::AlignHCenter);
      layout->addWidget(l);
      QFrame* f = new QFrame;
      f->setFrameStyle(QFrame::HLine | QFrame::Raised);
      f->setLineWidth(2);
      layout->addWidget(f);

      QHBoxLayout* hbox = new QHBoxLayout;
      dot1 = new QToolButton(this);
      dot1->setText(tr("Dot1"));
      dot1->setEnabled(false);
      hbox->addWidget(dot1);
      dot2 = new QToolButton(this);
      dot2->setText(tr("Dot2"));
      dot2->setEnabled(false);
      hbox->addWidget(dot2);
      dot3 = new QToolButton(this);
      dot3->setText(tr("Dot3"));
      dot3->setEnabled(false);
      hbox->addWidget(dot3);
      hook = new QToolButton(this);
      hook->setText(tr("Hook"));
      hook->setEnabled(false);
      hbox->addWidget(hook);
      layout->addLayout(hbox);

      hbox = new QHBoxLayout;
      stem = new QToolButton(this);
      stem->setText(tr("Stem"));
      stem->setEnabled(false);
      hbox->addWidget(stem);
      beam = new QToolButton(this);
      beam->setText(tr("Beam"));
      beam->setEnabled(false);
      hbox->addWidget(beam);
      layout->addLayout(hbox);

      connect(dot1,     SIGNAL(clicked()),     SLOT(dot1Clicked()));
      connect(dot2,     SIGNAL(clicked()),     SLOT(dot2Clicked()));
      connect(dot3,     SIGNAL(clicked()),     SLOT(dot3Clicked()));
      connect(hook,     SIGNAL(clicked()),     SLOT(hookClicked()));
      connect(stem,     SIGNAL(clicked()),     SLOT(stemClicked()));
      connect(beam,     SIGNAL(clicked()),     SLOT(beamClicked()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorNote::setElement()
      {
      Note* note = static_cast<Note*>(inspector->element());
      Segment* segment = note->chord()->segment();

      iElement->setElement(note);
      iNote->setElement(note);
      iChord->setElement(note->chord());
      iSegment->setElement(segment);
      dot1->setEnabled(note->dot(0));
      dot2->setEnabled(note->dot(1));
      dot3->setEnabled(note->dot(2));
      stem->setEnabled(note->chord()->stem());
      hook->setEnabled(note->chord()->hook());
      beam->setEnabled(note->chord()->beam());
      }

//---------------------------------------------------------
//   dot1Clicked
//---------------------------------------------------------

void InspectorNote::dot1Clicked()
      {
      Note* note = static_cast<Note*>(inspector->element());
      if (note == 0)
            return;
      NoteDot* dot = note->dot(0);
      if (dot) {
            dot->score()->select(dot);
            inspector->setElement(dot);
            dot->score()->end();
            }
      }

//---------------------------------------------------------
//   dot2Clicked
//---------------------------------------------------------

void InspectorNote::dot2Clicked()
      {
      Note* note = static_cast<Note*>(inspector->element());
      if (note == 0)
            return;
      NoteDot* dot = note->dot(1);
      if (dot) {
            dot->score()->select(dot);
            inspector->setElement(dot);
            dot->score()->end();
            }
      }

//---------------------------------------------------------
//   dot3Clicked
//---------------------------------------------------------

void InspectorNote::dot3Clicked()
      {
      Note* note = static_cast<Note*>(inspector->element());
      if (note == 0)
            return;
      NoteDot* dot = note->dot(2);
      if (dot) {
            dot->score()->select(dot);
            inspector->setElement(dot);
            dot->score()->end();
            }
      }

//---------------------------------------------------------
//   hookClicked
//---------------------------------------------------------

void InspectorNote::hookClicked()
      {
      Note* note = static_cast<Note*>(inspector->element());
      if (note == 0)
            return;
      Hook* hook = note->chord()->hook();
      if (hook) {
            note->score()->select(hook);
            inspector->setElement(hook);
            note->score()->end();
            }
      }

//---------------------------------------------------------
//   stemClicked
//---------------------------------------------------------

void InspectorNote::stemClicked()
      {
      Note* note = static_cast<Note*>(inspector->element());
      if (note == 0)
            return;
      Stem* stem = note->chord()->stem();
      if (stem) {
            note->score()->select(stem);
            inspector->setElement(stem);
            note->score()->end();
            }
      }

//---------------------------------------------------------
//   beamClicked
//---------------------------------------------------------

void InspectorNote::beamClicked()
      {
      Note* note = static_cast<Note*>(inspector->element());
      if (note == 0)
            return;
      Beam* beam = note->chord()->beam();
      if (beam) {
            note->score()->select(beam);
            inspector->setElement(beam);
            note->score()->end();
            }
      }

//---------------------------------------------------------
//   InspectorRest
//---------------------------------------------------------

InspectorRest::InspectorRest(QWidget* parent)
   : InspectorBase(parent)
      {
      QWidget* w1 = new QWidget;
      e.setupUi(w1);
      layout->addWidget(w1);
      QWidget* w2 = new QWidget;
      s.setupUi(w2);
      layout->addWidget(w2);
      QWidget* w3 = new QWidget;
      r.setupUi(w3);
      layout->addWidget(w3);

      iList = {
            { P_COLOR,          0, false, e.color,         e.resetColor         },
            { P_VISIBLE,        0, false, e.visible,       e.resetVisible       },
            { P_USER_OFF,       0, false, e.offsetX,       e.resetX             },
            { P_USER_OFF,       1, false, e.offsetY,       e.resetY             },
            { P_SMALL,          0, false, r.small,         r.resetSmall         },
            { P_LEADING_SPACE,  0, true,  s.leadingSpace,  s.resetLeadingSpace  },
            { P_TRAILING_SPACE, 0, true,  s.trailingSpace, s.resetTrailingSpace }
            };
      mapSignals();
      }

//---------------------------------------------------------
//   InspectorTimeSig
//---------------------------------------------------------

InspectorTimeSig::InspectorTimeSig(QWidget* parent)
   : InspectorBase(parent)
      {
      iElement = new InspectorElementElement(this);
      iSegment = new InspectorSegment(this);

      layout->addWidget(iElement);

      QHBoxLayout* l = new QHBoxLayout;
      showCourtesy = new QCheckBox;
      showCourtesy->setText(tr("Show Courtesy Time Signature"));
      connect(showCourtesy, SIGNAL(toggled(bool)), SLOT(apply()));
      l->addWidget(showCourtesy);
      layout->addLayout(l);
      layout->addWidget(iSegment);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorTimeSig::setElement()
      {
      TimeSig* sig = static_cast<TimeSig*>(inspector->element());
      Segment* segment = sig->segment();

      iElement->setElement(sig);
      iSegment->setElement(segment);
      showCourtesy->blockSignals(true);
      showCourtesy->setChecked(sig->showCourtesySig());
      showCourtesy->blockSignals(false);
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorTimeSig::apply()
      {
      TimeSig* sig = static_cast<TimeSig*>(inspector->element());

      bool val = showCourtesy->isChecked();
      if (val != sig->showCourtesySig()) {
            mscore->getInspector()->setInspectorEdit(true); // this edit is coming from within the inspector itself:
                                                            // do not set element values again
            Score* score = sig->score();
            score->startCmd();
            score->undoChangeProperty(sig, P_SHOW_COURTESY, val);
            score->endCmd();
            mscore->endCmd();
            }
      }

//---------------------------------------------------------
//   InspectorKeySig
//---------------------------------------------------------

InspectorKeySig::InspectorKeySig(QWidget* parent)
   : InspectorBase(parent)
      {
      iElement = new InspectorElementElement(this);
      iSegment = new InspectorSegment(this);
      layout->addWidget(iElement);

      QHBoxLayout* l = new QHBoxLayout;
      showCourtesy = new QCheckBox;
      showCourtesy->setText(tr("Show Courtesy Time Signature"));
      connect(showCourtesy, SIGNAL(toggled(bool)), SLOT(apply()));
      l->addWidget(showCourtesy);
      layout->addLayout(l);

      l = new QHBoxLayout;
      showNaturals = new QCheckBox;
      showNaturals->setText(tr("Show Naturals"));
      connect(showNaturals, SIGNAL(toggled(bool)), SLOT(apply()));
      l->addWidget(showNaturals);
      layout->addLayout(l);

      layout->addWidget(iSegment);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorKeySig::setElement()
      {
      KeySig* sig = static_cast<KeySig*>(inspector->element());
      Segment* segment = sig->segment();

      iElement->setElement(sig);
      iSegment->setElement(segment);
      showCourtesy->blockSignals(true);
      showNaturals->blockSignals(true);
      showCourtesy->setChecked(sig->showCourtesy());
      showNaturals->setChecked(sig->showNaturals());
      showCourtesy->blockSignals(false);
      showNaturals->blockSignals(false);
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorKeySig::apply()
      {
      KeySig* sig = static_cast<KeySig*>(inspector->element());

      bool sc = showCourtesy->isChecked();
      bool sn = showNaturals->isChecked();
      if (sc != sig->showCourtesy() || sn != sig->showNaturals()) {
            mscore->getInspector()->setInspectorEdit(true); // this edit is coming from within the inspector itself:
                                                            // do not set element values again
            Score* score = sig->score();
            score->startCmd();
            if (sc != sig->showCourtesy())
                  score->undoChangeProperty(sig, P_SHOW_COURTESY, sc);
            if (sn != sig->showNaturals())
                  score->undoChangeProperty(sig, P_SHOW_NATURALS, sn);
            score->endCmd();
            mscore->endCmd();
            }
      }

//---------------------------------------------------------
//   InspectorClef
//---------------------------------------------------------

InspectorClef::InspectorClef(QWidget* parent)
   : InspectorBase(parent)
      {
      QWidget* w1 = new QWidget;
      e.setupUi(w1);
      layout->addWidget(w1);
      QWidget* w2 = new QWidget;
      s.setupUi(w2);
      layout->addWidget(w2);
      QWidget* w3 = new QWidget;
      c.setupUi(w3);
      layout->addWidget(w3);

      iList = {
            { P_COLOR,          0, false, e.color,         e.resetColor         },
            { P_VISIBLE,        0, false, e.visible,       e.resetVisible       },
            { P_USER_OFF,       0, false, e.offsetX,       e.resetX             },
            { P_USER_OFF,       1, false, e.offsetY,       e.resetY             },
            { P_LEADING_SPACE,  0, true,  s.leadingSpace,  s.resetLeadingSpace  },
            { P_TRAILING_SPACE, 0, true,  s.trailingSpace, s.resetTrailingSpace },
            { P_SHOW_COURTESY,  0, false, c.showCourtesy,  c.resetShowCourtesy  }
            };
      mapSignals();
      }

//---------------------------------------------------------
//   InspectorChord
//---------------------------------------------------------

InspectorChord::InspectorChord(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);
      connect(small,         SIGNAL(toggled(bool)),            SLOT(smallChanged(bool)));
      connect(stemless,      SIGNAL(toggled(bool)),            SLOT(stemlessChanged(bool)));
      connect(stemDirection, SIGNAL(currentIndexChanged(int)), SLOT(stemDirectionChanged(int)));
      connect(offsetX,       SIGNAL(valueChanged(double)),     SLOT(offsetXChanged(double)));
      connect(offsetY,       SIGNAL(valueChanged(double)),     SLOT(offsetYChanged(double)));

      connect(resetSmall,    SIGNAL(clicked()),      SLOT(resetSmallClicked()));
      connect(resetStemless, SIGNAL(clicked()),      SLOT(resetStemlessClicked()));
      connect(resetStemDirection, SIGNAL(clicked()), SLOT(resetStemDirectionClicked()));
      connect(resetX,        SIGNAL(clicked()),      SLOT(resetXClicked()));
      connect(resetY,        SIGNAL(clicked()),      SLOT(resetYClicked()));
      }

//---------------------------------------------------------
//   dirty
//---------------------------------------------------------

bool InspectorChord::dirty() const
      {
      return chord->small() != small->isChecked()
         || chord->noStem() != stemless->isChecked()
         || chord->stemDirection() != (MScore::Direction)(stemDirection->currentIndex())
         || chord->userOff().x() != offsetX->value()
         || chord->userOff().y() != offsetY->value()
         ;
      }

//---------------------------------------------------------
//   block
//---------------------------------------------------------

void InspectorChord::block(bool val)
      {
      small->blockSignals(val);
      stemless->blockSignals(val);
      stemDirection->blockSignals(val);
      offsetX->blockSignals(val);
      offsetY->blockSignals(val);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorChord::setElement(Chord* c)
      {
      chord = c;

      block(true);

      offsetX->setValue(c->userOff().x());
      offsetY->setValue(c->userOff().y());

      small->setChecked(chord->small());
      stemless->setChecked(chord->noStem());
      stemDirection->setCurrentIndex(chord->stemDirection());

      resetSmall->setEnabled(chord->small());
      resetStemless->setEnabled(chord->noStem());
      resetStemDirection->setEnabled(stemDirection->currentIndex() != 0);

      block(false);
      }

//---------------------------------------------------------
//   smallChanged
//---------------------------------------------------------

void InspectorChord::smallChanged(bool val)
      {
      resetSmall->setEnabled(val);
      apply();
      }

//---------------------------------------------------------
//   stemlessChanged
//---------------------------------------------------------

void InspectorChord::stemlessChanged(bool val)
      {
      resetStemless->setEnabled(val);
      apply();
      }

//---------------------------------------------------------
//   stemDirectionChanged
//---------------------------------------------------------

void InspectorChord::stemDirectionChanged(int idx)
      {
      resetStemDirection->setEnabled(idx != 0);
      apply();
      }

//---------------------------------------------------------
//   offsetXChanged
//---------------------------------------------------------

void InspectorChord::offsetXChanged(double val)
      {
      resetX->setEnabled(val != 0);
      apply();
      }

//---------------------------------------------------------
//   offsetYChanged
//---------------------------------------------------------

void InspectorChord::offsetYChanged(double val)
      {
      resetY->setEnabled(val != 0);
      apply();
      }

//---------------------------------------------------------
//   resetSmall
//---------------------------------------------------------

void InspectorChord::resetSmallClicked()
      {
      small->setChecked(false);
      apply();
      }

//---------------------------------------------------------
//   resetStemless
//---------------------------------------------------------

void InspectorChord::resetStemlessClicked()
      {
      stemless->setChecked(false);
      apply();
      }

//---------------------------------------------------------
//   resetStemDirection
//---------------------------------------------------------

void InspectorChord::resetStemDirectionClicked()
      {
      stemDirection->setCurrentIndex(0);
      apply();
      }

//---------------------------------------------------------
//   resetX
//---------------------------------------------------------

void InspectorChord::resetXClicked()
      {
      offsetX->setValue(0.0);
      apply();
      }

//---------------------------------------------------------
//   resetY
//---------------------------------------------------------

void InspectorChord::resetYClicked()
      {
      offsetY->setValue(0.0);
      apply();
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorChord::apply()
      {
      if (!dirty())
            return;
      mscore->getInspector()->setInspectorEdit(true); // this edit is coming from within the inspector itself:
                                                      // do not set element values again
      Score* score = chord->score();
      score->startCmd();
      if (small->isChecked() != chord->small())
            score->undoChangeProperty(chord, P_SMALL, small->isChecked());
      if (stemless->isChecked() != chord->noStem())
            score->undoChangeProperty(chord, P_NO_STEM, stemless->isChecked());
      MScore::Direction d = MScore::Direction(stemDirection->currentIndex());
      if (d != chord->stemDirection())
            score->undoChangeProperty(chord, P_STEM_DIRECTION, d);
      score->endCmd();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   InspectorBarLine
//---------------------------------------------------------

#define BARLINE_TYPE_DEFAULT  -1

QString InspectorBarLine::builtinSpanNames[BARLINE_BUILTIN_SPANS] =
{
      tr("Staff default"), tr("Tick"), tr("Tick alt."), tr("Short"), tr("Short alt.")
};

int InspectorBarLine::builtinSpans[BARLINE_BUILTIN_SPANS][3] =
{//   span From To
      { 0,  0,  0},           // = staff defalt
      { 1, -2,  2},           // tick 1
      { 1, -1,  1},           // tick 2
      { 1,  2,  0},           // short 1 (To depends on staff num. of lines)
      { 1,  1,  0}            // short 2 (To depends on staff num. of lines)
};

InspectorBarLine::InspectorBarLine(QWidget* parent)
   : InspectorBase(parent)
      {
      iElement = new InspectorElementElement(this);
      layout->addWidget(iElement);
      layout->addSpacing(20);

      // "Type" Combo box
      QHBoxLayout* l = new QHBoxLayout;
      QLabel* label = new QLabel(tr("Type:"));
      type = new QComboBox;
      type->addItem(tr("Measure default"), BARLINE_TYPE_DEFAULT);
      type->addItem(tr("Normal"), NORMAL_BAR);
      type->addItem(tr("Dashed"), BROKEN_BAR);
      type->addItem(tr("Dotted"), DOTTED_BAR);
      type->addItem(tr("Double"), DOUBLE_BAR);
      type->addItem(tr("End"), END_BAR);
      connect(type, SIGNAL(currentIndexChanged(int)), SLOT(apply()));
      l->addWidget(label);
      l->addWidget(type);
      layout->addLayout(l);

      // "Span" combo box
      l = new QHBoxLayout;
      label = new QLabel(tr("Span:"));
      span = new QComboBox;
      for(int i=0; i < BARLINE_BUILTIN_SPANS; i++)
            span->addItem(builtinSpanNames[i]);
      span->addItem(tr("Custom"));
      connect(span, SIGNAL(currentIndexChanged(int)), SLOT(apply()));
      l->addWidget(label);
      l->addWidget(span);
      layout->addLayout(l);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorBarLine::setElement()
      {
      BarLine* bl = static_cast<BarLine*>(inspector->element());
      Measure* m = static_cast<Segment*>(bl->parent())->measure();
      measureBarLineType = m->endBarLineType();

      iElement->setElement(bl);
      type->blockSignals(true);
      span->blockSignals(true);

      type->setEnabled(true);
      // set type: if measure bar line is a repeat, no other type is possible; disable combo
      if (measureBarLineType == START_REPEAT || measureBarLineType == END_REPEAT || measureBarLineType == END_START_REPEAT) {
            type->setEnabled(false);
            type->setCurrentIndex(0);
            }
      // if same as parent measure, set combo to Measure default
      else if (bl->barLineType() == measureBarLineType) {
            type->setCurrentIndex(0);
            }
      // if custom type, set combo to item corresponding to bar line type
      else
            for (int i = 1; i < type->count(); i++)
                  if (type->itemData(i) == bl->barLineType()) {
                        type->setCurrentIndex(i);
                        break;
                        }

      // set span: fix spanTo values depending from staff number of lines
      if(bl->staff()) {
            Staff* st = bl->staff();
            int maxSpanTo = (st->lines()-1) * 2;
            builtinSpans[3][2] = maxSpanTo-2;         // short
            builtinSpans[4][2] = maxSpanTo-1;         // short alt
      }
      else {
            builtinSpans[3][2] = DEFAULT_BARLINE_TO-2;
            builtinSpans[4][2] = DEFAULT_BARLINE_TO-1;
            }
      // if bar line span is same as staff, set to "Staff default"
      if (!bl->customSpan())
            span->setCurrentIndex(0);
      // if custom span, look for corresponding item in combo box
      else {
            int i;
            for(i=1; i < BARLINE_BUILTIN_SPANS; i++)
                  if(bl->span() == builtinSpans[i][0]
                              && bl->spanFrom() == builtinSpans[i][1]
                              && bl->spanTo() == builtinSpans[i][2])
                        break;
            // if no match found among combo items, will set to "Custom"
            span->setCurrentIndex(i);
            }

      type->blockSignals(false);
      span->blockSignals(false);
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorBarLine::apply()
      {
      BarLine*    bl = static_cast<BarLine*>(inspector->element());
      Score*      score = bl->score();

      mscore->getInspector()->setInspectorEdit(true); // this edit is coming from within the inspector itself:
                                                      // do not set element values again
      score->startCmd();

      // type
      int currType = type->itemData(type->currentIndex()).toInt();
      if(currType == BARLINE_TYPE_DEFAULT)
            currType = measureBarLineType;
      if (currType != bl->barLineType())
            score->undoChangeProperty(bl, P_SUBTYPE, currType);
      // if value reverted to measure default, update combo box
      if(!bl->customSubtype())
            type->setCurrentIndex(0);

      // span: determine span, spanFrom and spanTo values for current combo box item
      int currSpan = span->currentIndex();
      int spanStaves, spanFrom, spanTo;
      if(currSpan == 0) {                 // staff default selected
            if(bl->staff()) {                               // if there is a staff
                  Staff* st = bl->staff();                  // use its span values as selected values
                  spanStaves  = st->barLineSpan();
                  spanFrom    = st->barLineFrom();
                  spanTo      = st->barLineTo();
                  }
            else {                                          // if no staff
                  spanStaves  = 1;                          // use values for a 'standard' staff
                  spanFrom    = 0;
                  spanTo      = DEFAULT_BARLINE_TO;
                  }
            }
      else {                              // specific value selected
            if (currSpan == BARLINE_BUILTIN_SPANS+1) {      // selecting custom has no effect:
                  spanStaves  = bl->span();                 // use values from bar line itself
                  spanFrom    = bl->spanFrom();
                  spanTo      = bl->spanTo();
                  }
            else {
                  spanStaves  = builtinSpans[currSpan][0];  // use values from selected combo item
                  spanFrom    = builtinSpans[currSpan][1];
                  spanTo      = builtinSpans[currSpan][2];
                  }
            }
      // if combo values different from bar line's, set them
      if(bl->span() != spanStaves || bl->spanFrom() != spanFrom || bl->spanTo() != spanTo)
            score->undoChangeSingleBarLineSpan(bl, spanStaves, spanFrom, spanTo);
      // if value reverted to staff default, update combo box
      if(!bl->customSpan())
            span->setCurrentIndex(0);

      score->endCmd();
      mscore->endCmd();
      }

