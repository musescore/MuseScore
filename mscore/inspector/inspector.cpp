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
      if (!inspector) {
            inspector = new Inspector();
            connect(inspector, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));
            addDockWidget(Qt::RightDockWidgetArea, inspector);
            }
      if (visible) {
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
      setAllowedAreas(Qt::DockWidgetAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea));
      sa = new QScrollArea;
      sa->setFrameShape(QFrame::NoFrame);
      sa->setWidgetResizable(true);
      setWidget(sa);

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
            if (ie)
                  ie->deleteLater();
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
            sa->setWidget(ie);
            // setMinimumWidth(ie->width() + sa->frameWidth() * 2 + (width() - sa->width()) + 3);
            setMinimumWidth(ie->sizeHint().width() + sa->frameWidth() * 2 + (width() - sa->width()) + 3);
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
            otherSegm = otherMeas->findSegment(SegmentType::Clef, segmTick);
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
                  t.style->addItem(ts.at(i).name(), i);
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
                  t.style->addItem(ts.at(i).name(), i);
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
                  t.style->addItem(ts.at(i).name(), i);
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
      static const char* buildinSpanNames[BARLINE_BUILTIN_SPANS] = {
            QT_TRANSLATE_NOOP("inspector", "Staff default"),
            QT_TRANSLATE_NOOP("inspector", "Tick"),
            QT_TRANSLATE_NOOP("inspector", "Tick alt."),
            QT_TRANSLATE_NOOP("inspector", "Short"),
            QT_TRANSLATE_NOOP("inspector", "Short alt.")
            };

      BarLineType types[8] = {
            BarLineType::NORMAL,
            BarLineType::DOUBLE,
            BarLineType::START_REPEAT,
            BarLineType::END_REPEAT,
            BarLineType::BROKEN,
            BarLineType::END,
            BarLineType::END_START_REPEAT,
            BarLineType::DOTTED
            };

      e.setupUi(addWidget());
      b.setupUi(addWidget());

      for (const char* name : buildinSpanNames)
            b.spanType->addItem(tr(name));
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
      connect(b.spanType, SIGNAL(activated(int)), SLOT(spanTypeActivated(int)));
      connect(b.resetSpanType, SIGNAL(clicked()), SLOT(resetSpanType()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorBarLine::setElement()
      {
      InspectorBase::setElement();
//      BarLine* bl = static_cast<BarLine*>(inspector->element());
//      Measure* m  = static_cast<Segment*>(bl->parent())->measure();
      }

//---------------------------------------------------------
//   spanTypeActivated
//---------------------------------------------------------

void InspectorBarLine::spanTypeActivated(int /*idx*/)
      {
      }

//---------------------------------------------------------
//   resetSpanType
//---------------------------------------------------------

void InspectorBarLine::resetSpanType()
      {
      }

#if 0

#define BARLINE_TYPE_DEFAULT  -1


int InspectorBarLine::builtinSpans[BARLINE_BUILTIN_SPANS][3] =
{//   span From To
      { 0,  0,  0},           // = staff defalt
      { 1, -2,  2},           // tick 1
      { 1, -1,  1},           // tick 2
      { 1,  2,  0},           // short 1 (To depends on staff num. of lines)
      { 1,  1,  0}            // short 2 (To depends on staff num. of lines)
};
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
      if (measureBarLineType == BarLineType::START_REPEAT || measureBarLineType == BarLineType::END_REPEAT || measureBarLineType == BarLineType::END_START_REPEAT) {
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
      if (currType == BARLINE_TYPE_DEFAULT)
            currType = measureBarLineType;
      if (currType != bl->barLineType())
            score->undoChangeProperty(bl, P_ID::SUBTYPE, currType);
      // if value reverted to measure default, update combo box
      if (!bl->customSubtype())
            type->setCurrentIndex(0);

      // span: determine span, spanFrom and spanTo values for current combo box item
      int currSpan = span->currentIndex();
      int spanStaves, spanFrom, spanTo;
      if (currSpan == 0) {                 // staff default selected
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
#endif
}

