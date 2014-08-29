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
#include "inspectorTextLine.h"
#include "inspectorMarker.h"
#include "inspectorJump.h"
#include "inspectorGlissando.h"
#include "inspectorNote.h"
#include "inspectorAmbitus.h"
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

namespace Ms {

//---------------------------------------------------------
//   showInspector
//---------------------------------------------------------

void MuseScore::showInspector(bool visible)
      {
      QAction* a = getAction("inspector");
      if (!_inspector) {
            _inspector = new Inspector();
            connect(_inspector, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));
            addDockWidget(Qt::RightDockWidgetArea, _inspector);
            }
      if (visible) {
            updateInspector();
            }
      if (_inspector)
            _inspector->setVisible(visible);
      }

//---------------------------------------------------------
//   Inspector
//---------------------------------------------------------

Inspector::Inspector(QWidget* parent)
   : QDockWidget(tr("Inspector"), parent)
      {
      setObjectName("inspector");
      setAllowedAreas(Qt::DockWidgetAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea));
      sa = new QScrollArea;
      sa->setAccessibleName(tr("Inspector Subwindow"));
      sa->setFrameShape(QFrame::NoFrame);
      sa->setWidgetResizable(true);
      setWidget(sa);
      sa->setFocusPolicy(Qt::NoFocus);

      _inspectorEdit = false;
      ie             = 0;
      _element       = 0;
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
      if (_inspectorEdit)     // if within an inspector-originated edit
            return;

      Element* e = l.isEmpty() ? 0 : l[0];
      if (e == 0 || _element == 0 || (_el != l)) {
            _el = l;
            ie = 0;
            _element = e;

            if (_element == 0)
                  ie = new InspectorEmpty(this);


            bool sameTypes = true;
            foreach(Element* ee, _el) {
                  if (_element->type() != ee->type())
                        sameTypes = false;
                  }
            if (!sameTypes)
                  ie = new InspectorGroupElement(this);
            else if (_element) {
                  switch(_element->type()) {
                        case Element::Type::FBOX:
                        case Element::Type::TBOX:
                        case Element::Type::VBOX:
                              ie = new InspectorVBox(this);
                              break;
                        case Element::Type::HBOX:
                              ie = new InspectorHBox(this);
                              break;
                        case Element::Type::ARTICULATION:
                              ie = new InspectorArticulation(this);
                              break;
                        case Element::Type::SPACER:
                              ie = new InspectorSpacer(this);
                              break;
                        case Element::Type::NOTE:
                              ie = new InspectorNote(this);
                              break;
                        case Element::Type::ACCIDENTAL:
                              ie = new InspectorAccidental(this);
                              break;
                        case Element::Type::REST:
                              ie = new InspectorRest(this);
                              break;
                        case Element::Type::CLEF:
                              ie = new InspectorClef(this);
                              break;
                        case Element::Type::TIMESIG:
                              ie = new InspectorTimeSig(this);
                              break;
                        case Element::Type::KEYSIG:
                              ie = new InspectorKeySig(this);
                              break;
                        case Element::Type::TUPLET:
                              ie = new InspectorTuplet(this);
                              break;
                        case Element::Type::BEAM:
                              ie = new InspectorBeam(this);
                              break;
                        case Element::Type::IMAGE:
                              ie = new InspectorImage(this);
                              break;
                        case Element::Type::LASSO:
                              ie = new InspectorLasso(this);
                              break;
                        case Element::Type::VOLTA_SEGMENT:
                              ie = new InspectorVolta(this);
                              break;
                        case Element::Type::OTTAVA_SEGMENT:
                              ie = new InspectorOttava(this);
                              break;
                        case Element::Type::TRILL_SEGMENT:
                              ie = new InspectorTrill(this);
                              break;
                        case Element::Type::HAIRPIN_SEGMENT:
                              ie = new InspectorHairpin(this);
                              break;
                        case Element::Type::TEXTLINE_SEGMENT:
                        case Element::Type::PEDAL_SEGMENT:
                              ie = new InspectorTextLine(this);
                              break;
                        case Element::Type::SLUR_SEGMENT:
                              ie = new InspectorSlur(this);
                              break;
                        case Element::Type::BAR_LINE:
                              ie = new InspectorBarLine(this);
                              break;
                        case Element::Type::JUMP:
                              ie = new InspectorJump(this);
                              break;
                        case Element::Type::MARKER:
                              ie = new InspectorMarker(this);
                              break;
                        case Element::Type::GLISSANDO:
                              ie = new InspectorGlissando(this);
                              break;
                        case Element::Type::TEMPO_TEXT:
                              ie = new InspectorTempoText(this);
                              break;
                        case Element::Type::DYNAMIC:
                              ie = new InspectorDynamic(this);
                              break;
                        case Element::Type::AMBITUS:
                              ie = new InspectorAmbitus(this);
                              break;
                        default:
                              if (_element->isText())
                                    ie = new InspectorText(this);
                              else
                                    ie = new InspectorElement(this);
                              break;
                        }
                  }
            QWidget* ww = sa->takeWidget();
            if (ww)
                  ww->deleteLater();
            sa->setWidget(ie);

            //focus policies were set by hand in each inspector_*.ui. this code just helps keeping them like they are
            //also fixes mac problem. on Mac Qt::TabFocus doesn't work, but Qt::StrongFocus works
            QList<QWidget*> widgets = ie->findChildren<QWidget*>();
            for (int i = 0; i < widgets.size(); i++) {
                  QWidget* currentWidget = widgets.at(i);
                  switch (currentWidget->focusPolicy()) {
                        case Qt::TabFocus:
#if defined(Q_OS_MAC)
                              currentWidget->setFocusPolicy(Qt::StrongFocus);
#endif
                        case Qt::WheelFocus:
                        case Qt::StrongFocus:
#if defined(Q_OS_MAC)
//leave them like they are
#else
                              if (currentWidget->parent()->inherits("QAbstractSpinBox") ||
                                  currentWidget->inherits("QAbstractSpinBox")           ||
                                  currentWidget->inherits("QLineEdit")) ; //leave it like it is
                              else
                                   currentWidget->setFocusPolicy(Qt::TabFocus);
#endif
                              break;
                        case Qt::NoFocus:
                        case Qt::ClickFocus:
                                    currentWidget->setFocusPolicy(Qt::NoFocus);
                              break;
                        }
                  }
            }
      _element = e;
      ie->setElement();
      }

//---------------------------------------------------------
//   setupUi
//---------------------------------------------------------

void UiInspectorElement::setupUi(QWidget *InspectorElement)
      {
      Ui::InspectorElement::setupUi(InspectorElement);

      QAction* a = getAction("hraster");
      a->setCheckable(true);
      hRaster->setDefaultAction(a);
      hRaster->setContextMenuPolicy(Qt::ActionsContextMenu);
      hRaster->addAction(getAction("config-raster"));

      a = getAction("vraster");
      a->setCheckable(true);
      vRaster->setDefaultAction(a);
      vRaster->setContextMenuPolicy(Qt::ActionsContextMenu);
      vRaster->addAction(getAction("config-raster"));
      }

//---------------------------------------------------------
//   InspectorElement
//---------------------------------------------------------

InspectorElement::InspectorElement(QWidget* parent)
   : InspectorBase(parent)
      {
      b.setupUi(addWidget());

      iList = {
            { P_ID::COLOR,    0, 0, b.color,      b.resetColor   },
            { P_ID::VISIBLE,  0, 0, b.visible,    b.resetVisible },
            { P_ID::USER_OFF, 0, 0, b.offsetX,    b.resetX       },
            { P_ID::USER_OFF, 1, 0, b.offsetY,    b.resetY       }
            };

      mapSignals();
      }

//---------------------------------------------------------
//   InspectorVBox
//---------------------------------------------------------

InspectorVBox::InspectorVBox(QWidget* parent)
   : InspectorBase(parent)
      {
      vb.setupUi(addWidget());

      iList = {
            { P_ID::TOP_GAP,       0, 0, vb.topGap,       vb.resetTopGap       },
            { P_ID::BOTTOM_GAP,    0, 0, vb.bottomGap,    vb.resetBottomGap    },
            { P_ID::LEFT_MARGIN,   0, 0, vb.leftMargin,   vb.resetLeftMargin   },
            { P_ID::RIGHT_MARGIN,  0, 0, vb.rightMargin,  vb.resetRightMargin  },
            { P_ID::TOP_MARGIN,    0, 0, vb.topMargin,    vb.resetTopMargin    },
            { P_ID::BOTTOM_MARGIN, 0, 0, vb.bottomMargin, vb.resetBottomMargin },
            { P_ID::BOX_HEIGHT,    0, 0, vb.height,       0                    }
            };
      mapSignals();
      }

//---------------------------------------------------------
//   InspectorHBox
//---------------------------------------------------------

InspectorHBox::InspectorHBox(QWidget* parent)
   : InspectorBase(parent)
      {
      hb.setupUi(addWidget());

      iList = {
            { P_ID::TOP_GAP,    0, 0, hb.leftGap,  hb.resetLeftGap  },
            { P_ID::BOTTOM_GAP, 0, 0, hb.rightGap, hb.resetRightGap },
            { P_ID::BOX_WIDTH,  0, 0, hb.width,    0                }
            };

      mapSignals();
      }

//---------------------------------------------------------
//   InspectorArticulation
//---------------------------------------------------------

InspectorArticulation::InspectorArticulation(QWidget* parent)
   : InspectorBase(parent)
      {
      e.setupUi(addWidget());
      ar.setupUi(addWidget());

      iList = {
            { P_ID::COLOR,               0, 0, e.color,        e.resetColor      },
            { P_ID::VISIBLE,             0, 0, e.visible,      e.resetVisible    },
            { P_ID::USER_OFF,            0, 0, e.offsetX,      e.resetX          },
            { P_ID::USER_OFF,            1, 0, e.offsetY,      e.resetY          },
            { P_ID::ARTICULATION_ANCHOR, 0, 0, ar.anchor,      ar.resetAnchor    },
            { P_ID::DIRECTION,           0, 0, ar.direction,   ar.resetDirection },
            { P_ID::TIME_STRETCH,        0, 0, ar.timeStretch, ar.resetTimeStretch }
            };
      mapSignals();
      }

//---------------------------------------------------------
//   InspectorSpacer
//---------------------------------------------------------

InspectorSpacer::InspectorSpacer(QWidget* parent)
   : InspectorBase(parent)
      {
      sp.setupUi(addWidget());

      iList = {
            { P_ID::SPACE, 0, false, sp.height, sp.resetHeight  }
            };
      mapSignals();
      }

//---------------------------------------------------------
//   InspectorRest
//---------------------------------------------------------

InspectorRest::InspectorRest(QWidget* parent)
   : InspectorBase(parent)
      {
      e.setupUi(addWidget());
      s.setupUi(addWidget());
      r.setupUi(addWidget());

      iList = {
            { P_ID::COLOR,          0, 0, e.color,         e.resetColor         },
            { P_ID::VISIBLE,        0, 0, e.visible,       e.resetVisible       },
            { P_ID::USER_OFF,       0, 0, e.offsetX,       e.resetX             },
            { P_ID::USER_OFF,       1, 0, e.offsetY,       e.resetY             },
            { P_ID::SMALL,          0, 0, r.small,         r.resetSmall         },
            { P_ID::LEADING_SPACE,  0, 1, s.leadingSpace,  s.resetLeadingSpace  },
            { P_ID::TRAILING_SPACE, 0, 1, s.trailingSpace, s.resetTrailingSpace }
            };
      mapSignals();
      }

//---------------------------------------------------------
//   InspectorTimeSig
//---------------------------------------------------------

InspectorTimeSig::InspectorTimeSig(QWidget* parent)
   : InspectorBase(parent)
      {
      e.setupUi(addWidget());
      s.setupUi(addWidget());
      t.setupUi(addWidget());

      iList = {
            { P_ID::COLOR,          0, 0, e.color,          e.resetColor         },
            { P_ID::VISIBLE,        0, 0, e.visible,        e.resetVisible       },
            { P_ID::USER_OFF,       0, 0, e.offsetX,        e.resetX             },
            { P_ID::USER_OFF,       1, 0, e.offsetY,        e.resetY             },
            { P_ID::LEADING_SPACE,  0, 1, s.leadingSpace,   s.resetLeadingSpace  },
            { P_ID::TRAILING_SPACE, 0, 1, s.trailingSpace,  s.resetTrailingSpace },
            { P_ID::SHOW_COURTESY,  0, 0, t.showCourtesy,   t.resetShowCourtesy  },
//            { P_ID::TIMESIG,        0, 0, t.timesigZ,       t.resetTimesig       },
//            { P_ID::TIMESIG,        1, 0, t.timesigN,       t.resetTimesig       },
//            { P_ID::TIMESIG_GLOBAL, 0, 0, t.globalTimesigZ, t.resetGlobalTimesig },
//            { P_ID::TIMESIG_GLOBAL, 1, 0, t.globalTimesigN, t.resetGlobalTimesig }
            };
      mapSignals();
      }

//---------------------------------------------------------
//   InspectorKeySig
//---------------------------------------------------------

InspectorKeySig::InspectorKeySig(QWidget* parent)
   : InspectorBase(parent)
      {
      e.setupUi(addWidget());
      s.setupUi(addWidget());
      k.setupUi(addWidget());

      iList = {
            { P_ID::COLOR,          0, 0, e.color,         e.resetColor         },
            { P_ID::VISIBLE,        0, 0, e.visible,       e.resetVisible       },
            { P_ID::USER_OFF,       0, 0, e.offsetX,       e.resetX             },
            { P_ID::USER_OFF,       1, 0, e.offsetY,       e.resetY             },
            { P_ID::LEADING_SPACE,  0, 1, s.leadingSpace,  s.resetLeadingSpace  },
            { P_ID::TRAILING_SPACE, 0, 1, s.trailingSpace, s.resetTrailingSpace },
            { P_ID::SHOW_COURTESY,  0, 0, k.showCourtesy,  k.resetShowCourtesy  },
//            { P_ID::SHOW_NATURALS,  0, 0, k.showNaturals,  k.resetShowNaturals  }
            };
      mapSignals();
      }

//---------------------------------------------------------
//   InspectorTuplet
//---------------------------------------------------------

InspectorTuplet::InspectorTuplet(QWidget* parent)
   : InspectorBase(parent)
      {
      e.setupUi(addWidget());
      t.setupUi(addWidget());

      iList = {
            { P_ID::COLOR,        0, 0, e.color,       e.resetColor       },
            { P_ID::VISIBLE,      0, 0, e.visible,     e.resetVisible     },
            { P_ID::USER_OFF,     0, 0, e.offsetX,     e.resetX           },
            { P_ID::USER_OFF,     1, 0, e.offsetY,     e.resetY           },
            { P_ID::DIRECTION,    0, 0, t.direction,   t.resetDirection   },
            { P_ID::NUMBER_TYPE,  0, 0, t.numberType,  t.resetNumberType  },
            { P_ID::BRACKET_TYPE, 0, 0, t.bracketType, t.resetBracketType }
            };
      mapSignals();
      }

//---------------------------------------------------------
//   InspectorAccidental
//---------------------------------------------------------

InspectorAccidental::InspectorAccidental(QWidget* parent)
   : InspectorBase(parent)
      {
      e.setupUi(addWidget());
      a.setupUi(addWidget());

      iList = {
            { P_ID::COLOR,        0, 0, e.color,       e.resetColor       },
            { P_ID::VISIBLE,      0, 0, e.visible,     e.resetVisible     },
            { P_ID::USER_OFF,     0, 0, e.offsetX,     e.resetX           },
            { P_ID::USER_OFF,     1, 0, e.offsetY,     e.resetY           },
            { P_ID::SMALL,        0, 0, a.small,       a.resetSmall       }
            };
      mapSignals();
      }

//---------------------------------------------------------
//   InspectorClef
//---------------------------------------------------------

InspectorClef::InspectorClef(QWidget* parent)
   : InspectorBase(parent)
      {
      e.setupUi(addWidget());
      s.setupUi(addWidget());
      c.setupUi(addWidget());

      iList = {
            { P_ID::COLOR,          0, 0, e.color,         e.resetColor         },
            { P_ID::VISIBLE,        0, 0, e.visible,       e.resetVisible       },
            { P_ID::USER_OFF,       0, 0, e.offsetX,       e.resetX             },
            { P_ID::USER_OFF,       1, 0, e.offsetY,       e.resetY             },
            { P_ID::LEADING_SPACE,  0, 1, s.leadingSpace,  s.resetLeadingSpace  },
            { P_ID::TRAILING_SPACE, 0, 1, s.trailingSpace, s.resetTrailingSpace },
            { P_ID::SHOW_COURTESY,  0, 0, c.showCourtesy,  c.resetShowCourtesy  }
            };
      mapSignals();
      }

//   InspectorClef::setElement

void InspectorClef::setElement()
      {
      otherClef = nullptr;                      // no 'other clef' yet
      InspectorBase::setElement();

      // try to locate the 'other clef' of a courtesy / main pair
      Clef * clef = static_cast<Clef*>(inspector->element());
      // if not in a clef-segment-measure hierachy, do nothing
      if (!clef->parent() || clef->parent()->type() != Element::Type::SEGMENT)
            return;
      Segment*    segm = static_cast<Segment*>(clef->parent());
      int         segmTick = segm->tick();
      if (!segm->parent() || segm->parent()->type() != Element::Type::MEASURE)
            return;

      Measure* meas = static_cast<Measure*>(segm->parent());
      Measure* otherMeas = nullptr;
      Segment* otherSegm = nullptr;
      if (segmTick == meas->tick())                         // if clef segm is measure-initial
            otherMeas = meas->prevMeasure();                // look for a previous measure
      else if (segmTick == meas->tick()+meas->ticks())      // if clef segm is measure-final
            otherMeas = meas->nextMeasure();                // look for a next measure
      // look for a clef segment in the 'other' measure at the same tick of this clef segment
      if (otherMeas)
            otherSegm = otherMeas->findSegment(Segment::Type::Clef, segmTick);
      // if any 'other' segment found, look for a clef in the same track as this
      if (otherSegm)
            otherClef = static_cast<Clef*>(otherSegm->element(clef->track()));
      }

//   InspectorClef::valueChanged

void InspectorClef::valueChanged(int idx)
      {
      // copy into 'other clef' the ShowCouretsy ser of this clef
      if (idx == 6 && otherClef)
            otherClef->setShowCourtesy(c.showCourtesy->isChecked());
      InspectorBase::valueChanged(idx);
      }

//---------------------------------------------------------
//   InspectorText
//---------------------------------------------------------

InspectorText::InspectorText(QWidget* parent)
   : InspectorBase(parent)
      {
      e.setupUi(addWidget());
      t.setupUi(addWidget());

      iList = {
            { P_ID::COLOR,              0, 0, e.color,    e.resetColor    },
            { P_ID::VISIBLE,            0, 0, e.visible,  e.resetVisible  },
            { P_ID::USER_OFF,           0, 0, e.offsetX,  e.resetX        },
            { P_ID::USER_OFF,           1, 0, e.offsetY,  e.resetY        },
            { P_ID::TEXT_STYLE_TYPE,    0, 0, t.style,    t.resetStyle    }
            };
      mapSignals();
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorText::setElement()
      {
      Element* e = inspector->element();
      Score* score = e->score();

      t.style->blockSignals(true);
      t.style->clear();
      const QList<TextStyle>& ts = score->style()->textStyles();
      int n = ts.size();
      for (int i = 0; i < n; ++i) {
            if (!(ts.at(i).hidden() & TextStyleHidden::IN_LISTS) )
                  t.style->addItem(qApp->translate("TextStyle",ts.at(i).name().toLatin1().data()), i);
            }
      t.style->blockSignals(false);
      InspectorBase::setElement();
      }

//---------------------------------------------------------
//   InspectorTempoText
//---------------------------------------------------------

InspectorTempoText::InspectorTempoText(QWidget* parent)
   : InspectorBase(parent)
      {
      e.setupUi(addWidget());
      t.setupUi(addWidget());
      tt.setupUi(addWidget());

      iList = {
            { P_ID::COLOR,             0, 0, e.color,       e.resetColor       },
            { P_ID::VISIBLE,           0, 0, e.visible,     e.resetVisible     },
            { P_ID::USER_OFF,          0, 0, e.offsetX,     e.resetX           },
            { P_ID::USER_OFF,          1, 0, e.offsetY,     e.resetY           },
            { P_ID::TEXT_STYLE_TYPE,   0, 0, t.style,       t.resetStyle       },
            { P_ID::TEMPO,             0, 0, tt.tempo,      tt.resetTempo      },
            { P_ID::TEMPO_FOLLOW_TEXT, 0, 0, tt.followText, tt.resetFollowText }
            };
      mapSignals();
      connect(tt.followText, SIGNAL(toggled(bool)), tt.tempo, SLOT(setDisabled(bool)));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorTempoText::setElement()
      {
      Element* e = inspector->element();
      Score* score = e->score();

      t.style->blockSignals(true);
      t.style->clear();
      const QList<TextStyle>& ts = score->style()->textStyles();
      int n = ts.size();
      for (int i = 0; i < n; ++i) {
            if (!(ts.at(i).hidden() & TextStyleHidden::IN_LISTS) )
                  t.style->addItem(qApp->translate("TextStyle",ts.at(i).name().toLatin1().data()), i);
            }
      t.style->blockSignals(false);
      InspectorBase::setElement();
      }

//---------------------------------------------------------
//   postInit
//---------------------------------------------------------

void InspectorTempoText::postInit()
      {
      tt.tempo->setDisabled(tt.followText->isChecked());
      }

//---------------------------------------------------------
//   InspectorDynamic
//---------------------------------------------------------

InspectorDynamic::InspectorDynamic(QWidget* parent)
   : InspectorBase(parent)
      {
      e.setupUi(addWidget());
      t.setupUi(addWidget());
      d.setupUi(addWidget());

      iList = {
            { P_ID::COLOR,              0, 0, e.color,    e.resetColor    },
            { P_ID::VISIBLE,            0, 0, e.visible,  e.resetVisible  },
            { P_ID::USER_OFF,           0, 0, e.offsetX,  e.resetX        },
            { P_ID::USER_OFF,           1, 0, e.offsetY,  e.resetY        },
            { P_ID::TEXT_STYLE_TYPE,    0, 0, t.style,    t.resetStyle    },
            { P_ID::DYNAMIC_RANGE,      0, 0, d.dynRange, d.resetDynRange },
            { P_ID::VELOCITY,           0, 0, d.velocity, d.resetVelocity }
            };
      mapSignals();
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorDynamic::setElement()
      {
      Element* e = inspector->element();
      Score* score = e->score();

      t.style->blockSignals(true);
      t.style->clear();
      const QList<TextStyle>& ts = score->style()->textStyles();
      int n = ts.size();
      for (int i = 0; i < n; ++i) {
            if (!(ts.at(i).hidden() & TextStyleHidden::IN_LISTS) )
                  t.style->addItem(qApp->translate("TextStyle",ts.at(i).name().toLatin1().data()), i);
            }
      t.style->blockSignals(false);
      InspectorBase::setElement();
      }

//---------------------------------------------------------
//   InspectorSlur
//---------------------------------------------------------

InspectorSlur::InspectorSlur(QWidget* parent)
   : InspectorBase(parent)
      {
      e.setupUi(addWidget());
      s.setupUi(addWidget());

      iList = {
            { P_ID::COLOR,      0, 0, e.color,    e.resetColor    },
            { P_ID::VISIBLE,    0, 0, e.visible,  e.resetVisible  },
            { P_ID::USER_OFF,   0, 0, e.offsetX,  e.resetX        },
            { P_ID::USER_OFF,   1, 0, e.offsetY,  e.resetY        },
            { P_ID::LINE_TYPE,  0, 0, s.lineType, s.resetLineType }
            };
      mapSignals();
      }

//---------------------------------------------------------
//   InspectorEmpty
//---------------------------------------------------------

InspectorEmpty::InspectorEmpty(QWidget* parent)
      :InspectorBase(parent)
      {
      setToolTip(tr("Select an element to display its properties"));
      }

//---------------------------------------------------------
//   InspectorBarLine
//---------------------------------------------------------

InspectorBarLine::InspectorBarLine(QWidget* parent)
   : InspectorBase(parent)
      {
      static const char* builtinSpanNames[BARLINE_BUILTIN_SPANS+1] = {
            QT_TRANSLATE_NOOP("inspector", "Staff default"),
            QT_TRANSLATE_NOOP("inspector", "Tick 1"),
            QT_TRANSLATE_NOOP("inspector", "Tick 2"),
            QT_TRANSLATE_NOOP("inspector", "Short 1"),
            QT_TRANSLATE_NOOP("inspector", "Short 2"),
            QT_TRANSLATE_NOOP("inspector", "[Custom]")
            };

      BarLineType types[8] = {
            BarLineType::NORMAL,
            BarLineType::BROKEN,
            BarLineType::DOTTED,
            BarLineType::DOUBLE,
            BarLineType::END,
            BarLineType::START_REPEAT,          // repeat types cannot be set for a single bar line
            BarLineType::END_REPEAT,            // of a multi-staff scores
            BarLineType::END_START_REPEAT,
            };

      e.setupUi(addWidget());
      b.setupUi(addWidget());

      for (const char* name : builtinSpanNames)
            b.spanType->addItem(qApp->translate("inspector", name));
      for (BarLineType t : types)
            b.type->addItem(BarLine::userTypeName(t), int(t));

      iList = {
            { P_ID::COLOR,             0, 0, e.color,    e.resetColor    },
            { P_ID::VISIBLE,           0, 0, e.visible,  e.resetVisible  },
            { P_ID::USER_OFF,          0, 0, e.offsetX,  e.resetX        },
            { P_ID::USER_OFF,          1, 0, e.offsetY,  e.resetY        },
            { P_ID::SUBTYPE,           0, 0, b.type,     b.resetType     },
            { P_ID::BARLINE_SPAN,      0, 0, b.span,     b.resetSpan     },
            { P_ID::BARLINE_SPAN_FROM, 0, 0, b.spanFrom, b.resetSpanFrom },
            { P_ID::BARLINE_SPAN_TO,   0, 0, b.spanTo,   b.resetSpanTo   },
            };
      mapSignals();
      // when any of the span parameters is changed, span data need to be managed
      connect(b.span,         SIGNAL(valueChanged(int)),    SLOT(manageSpanData()));
      connect(b.spanFrom,     SIGNAL(valueChanged(int)),    SLOT(manageSpanData()));
      connect(b.spanTo,       SIGNAL(valueChanged(int)),    SLOT(manageSpanData()));
      connect(b.spanType,     SIGNAL(activated(int)),       SLOT(spanTypeChanged(int)));
      connect(b.resetSpanType,SIGNAL(clicked()),            SLOT(resetSpanType()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

#include <QStandardItem>

void InspectorBarLine::setElement()
      {
      blockSpanDataSignals(true);
      InspectorBase::setElement();
      BarLine* bl = static_cast<BarLine*>(inspector->element());

      // enable / disable individual type combo items according to score and selected bar line status
      bool bMultiStaff  = bl->score()->nstaves() > 1;
      BarLineType blt   = bl->barLineType();
      bool bIsRepeat    = (blt == BarLineType::START_REPEAT
            || blt == BarLineType::END_REPEAT
            || blt == BarLineType::END_START_REPEAT);

      // scan type combo items
      const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>(b.type->model());
      for (int i = 0; i < b.type->count(); i++) {
            BarLineType type = (BarLineType)(b.type->itemData(i).toInt());
            QStandardItem* item = model->item(i);
            // if combo item is repeat type, should be disabled for multi-staff scores
            if (type == BarLineType::START_REPEAT
                        || type == BarLineType::END_REPEAT
                        || type == BarLineType::END_START_REPEAT) {
                  // disable / enable
                  item->setFlags(bMultiStaff ?
                        item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled) :
                        item->flags() | (Qt::ItemFlags)(Qt::ItemIsSelectable|Qt::ItemIsEnabled) );
/*                  visually disable by greying out (currently unnecessary, but kept for future reference)
                  item->setData(bMultiStaff ?
                              b.type->palette().color(QPalette::Disabled, QPalette::Text)
                              : QVariant(),       // clear item data in order to use default color
                        Qt::TextColorRole); */
                  }
            // if combo item is NOT repeat type, should be disabled if selected bar line is a repeat
            else {
                  item->setFlags(bIsRepeat ?
                        item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled) :
                        item->flags() | (Qt::ItemFlags)(Qt::ItemIsSelectable|Qt::ItemIsEnabled) );
                  }
            }
      // make custom span type unselectable (it is informative only)
      model = qobject_cast<const QStandardItemModel*>(b.spanType->model());
      QStandardItem* item = model->item(5);
      item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));

      manageSpanData();
      blockSpanDataSignals(false);
      }

//---------------------------------------------------------
//   spanTypeChanged
//---------------------------------------------------------

void InspectorBarLine::spanTypeChanged(int idx)
      {
      BarLine*    bl    = static_cast<BarLine*>(inspector->element());
      Score*      score = bl->score();
      score->startCmd();

      int spanStaves, spanFrom, spanTo;
      // the amount to adjust To value of short types, if staff num. of lines != 5
      int shortDelta = bl->staff() ? (bl->staff()->lines() - 5)*2 : 0;
      spanStaves = 1;               // in most cases, num. of spanned staves is 1
      switch (idx) {
            case 0:                 // staff default selected
                  if(bl->staff()) {                   // if there is a staff
                        Staff* st = bl->staff();      // use its span values as selected values
                        spanStaves  = st->barLineSpan();
                        spanFrom    = st->barLineFrom();
                        spanTo      = st->barLineTo();
                        }
                  else {                              // if no staff, use default values
                        spanFrom    = 0;
                        spanTo      = DEFAULT_BARLINE_TO;
                        }
                  break;
            case 1:
                  spanFrom    = BARLINE_SPAN_TICK1_FROM;
                  spanTo      = BARLINE_SPAN_TICK1_TO;
                  break;
            case 2:
                  spanFrom    = BARLINE_SPAN_TICK2_FROM;
                  spanTo      = BARLINE_SPAN_TICK2_TO;
                  break;
            case 3:
                  spanFrom    = BARLINE_SPAN_SHORT1_FROM;
                  spanTo      = BARLINE_SPAN_SHORT1_TO + shortDelta;
                  break;
            case 4:
                  spanFrom    = BARLINE_SPAN_SHORT2_FROM;
                  spanTo      = BARLINE_SPAN_SHORT2_TO + shortDelta;
                  break;
            case 5:                 // custom type has no effect
                  spanStaves  = bl->span();           // use values from bar line itself
                  spanFrom    = bl->spanFrom();
                  spanTo      = bl->spanTo();
                  break;
            }

      // if combo values different from bar line's, set them
      if(bl->span() != spanStaves || bl->spanFrom() != spanFrom || bl->spanTo() != spanTo) {
            blockSpanDataSignals(true);
            score->undoChangeSingleBarLineSpan(bl, spanStaves, spanFrom, spanTo);
            // if value reverted to staff default, update combo box
            if(!bl->customSpan())
                  b.spanType->setCurrentIndex(0);
            blockSpanDataSignals(false);
            }

      score->endCmd();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   manageSpanData
//
//    Makes sure span data are legal and consistent
//---------------------------------------------------------

void InspectorBarLine::manageSpanData()
      {
      BarLine* bl = static_cast<BarLine*>(inspector->element());

      // determine MIN and MAX for SPANFROM and SPANTO
      Staff* staffFrom  = bl->staff();
      Staff* staffTo    = bl->score()->staff(bl->staffIdx() + bl->span() - 1);
      int staffFromLines= (staffFrom ? staffFrom->lines() : 5);
      int staffToLines  = (staffTo   ? staffTo->lines()   : 5);

      // From:    min = minimum possible according to number of staff lines
      //          max = if same as To, at least 1sp (2 units) above To; if not, max possible according to num.of lines
      int min     = staffFromLines == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : MIN_BARLINE_SPAN_FROMTO;
      int max     = bl->span() < 2 ? bl->spanTo() - MIN_BARLINE_FROMTO_DIST
                        : (staffFromLines == 1 ? BARLINE_SPAN_1LINESTAFF_TO : (staffFromLines-1) * 2 + 2);
      b.spanFrom->setMinimum(min);
      b.spanFrom->setMaximum(max);
      b.spanFrom->setWrapping(false);

      // To:      min = if same as From, at least 1sp (2 units) below From; if not, min possible according to num.of lines
      //          max = max possible according to number of staff lines
      min   = bl->span() < 2 ? bl->spanFrom() + MIN_BARLINE_FROMTO_DIST
                  : (staffToLines == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : MIN_BARLINE_SPAN_FROMTO);
      max   = staffFromLines == 1 ? BARLINE_SPAN_1LINESTAFF_TO : (staffToLines-1) * 2 + 2;
      b.spanTo->setMinimum(min);
      b.spanTo->setMaximum(max);
      b.spanTo->setWrapping(false);

      // determin MAX for SPAN
      max = bl->score()->nstaves() - bl->staffIdx();
      b.span->setMaximum(max);
      b.span->setWrapping(false);

      // determine SPAN TYPE
      int short1To      = BARLINE_SPAN_SHORT1_TO + (staffFromLines - 5) * 2;
      int short2To      = BARLINE_SPAN_SHORT2_TO + (staffFromLines - 5) * 2;
      if (!bl->customSpan())
            b.spanType->setCurrentIndex(0);           // staff default
      else if (bl->span() == 1 && bl->spanFrom() == BARLINE_SPAN_TICK1_FROM  && bl->spanTo() == BARLINE_SPAN_TICK1_TO)
            b.spanType->setCurrentIndex(1);
      else if (bl->span() == 1 && bl->spanFrom() == BARLINE_SPAN_TICK2_FROM  && bl->spanTo() == BARLINE_SPAN_TICK2_TO)
            b.spanType->setCurrentIndex(2);
      else if (bl->span() == 1 && bl->spanFrom() == BARLINE_SPAN_SHORT1_FROM && bl->spanTo() == short1To)
            b.spanType->setCurrentIndex(3);
      else if (bl->span() == 1 && bl->spanFrom() == BARLINE_SPAN_SHORT2_FROM && bl->spanTo() == short2To)
            b.spanType->setCurrentIndex(4);
      else
            b.spanType->setCurrentIndex(5);           // custom
      }

//---------------------------------------------------------
//   resetSpanType
//---------------------------------------------------------

void InspectorBarLine::resetSpanType()
      {
      }

//---------------------------------------------------------
//   blockSpanDataSignals
//
//    block/unblok signals for span value controls
//---------------------------------------------------------

void InspectorBarLine::blockSpanDataSignals(bool val)
      {
      b.span->blockSignals(val);
      b.spanFrom->blockSignals(val);
      b.spanTo->blockSignals(val);
      b.spanType->blockSignals(val);
      }

}

