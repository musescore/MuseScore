//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: inspector.cpp
//
//  Copyright (C) 2011-2016 Werner Schweer
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
#include "inspectorArpeggio.h"
#include "inspectorNote.h"
#include "inspectorAmbitus.h"
#include "inspectorFret.h"
#include "inspectorText.h"
#include "musescore.h"
#include "scoreview.h"
#include "bendproperties.h"

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
#include "libmscore/tuplet.h"
#include "libmscore/bend.h"
#include "libmscore/tremolobar.h"
#include "libmscore/slur.h"
#include "libmscore/breath.h"
#include "libmscore/lyrics.h"

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
      if (_inspector)
            _inspector->setVisible(visible);
      if (visible)
            updateInspector();
      }

//---------------------------------------------------------
//   Inspector
//---------------------------------------------------------

Inspector::Inspector(QWidget* parent)
   : QDockWidget(parent)
      {
      setObjectName("inspector");
      setAllowedAreas(Qt::DockWidgetAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea));
      sa = new QScrollArea;
      sa->setFrameShape(QFrame::NoFrame);
      sa->setWidgetResizable(true);
      setWidget(sa);
      sa->setFocusPolicy(Qt::NoFocus);

      _inspectorEdit = false;
      ie             = 0;
      _element       = 0;
      retranslate();
      }

//---------------------------------------------------------
//   retranslate
//---------------------------------------------------------

void Inspector::retranslate()
      {
      setWindowTitle(tr("Inspector"));
      sa->setAccessibleName(tr("Inspector Subwindow"));
      QList<Element*> el = _el;
      setElements(QList<Element*>());
      setElements(el);
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
            for (Element* ee : _el) {
                  if (_element->type() != ee->type())
                        sameTypes = false;
                  else {
                        // HACK:
                        if (ee->isNote() && toNote(ee)->chord()->isGrace() != toNote(_element)->chord()->isGrace())
                              sameTypes = false;
                        }
                  }
            if (!sameTypes)
                  ie = new InspectorGroupElement(this);
            else if (_element) {
                  switch(_element->type()) {
                        case Element::Type::FBOX:
                        case Element::Type::VBOX:
                              ie = new InspectorVBox(this);
                              break;
                        case Element::Type::TBOX:
                              ie = new InspectorTBox(this);
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
//                        case Element::Type::PEDAL_SEGMENT:
                              ie = new InspectorTextLine(this);
                              break;
                        case Element::Type::SLUR_SEGMENT:
                              ie = new InspectorSlur(this);
                              break;
                        case Element::Type::BAR_LINE:
//                              if (_element->isEditable())
                                    ie = new InspectorBarLine(this);
//                              else
//                                    ie = new InspectorEmpty(this);
                              break;
                        case Element::Type::JUMP:
                              ie = new InspectorJump(this);
                              break;
                        case Element::Type::MARKER:
                              ie = new InspectorMarker(this);
                              break;
                        case Element::Type::GLISSANDO:
                        case Element::Type::GLISSANDO_SEGMENT:
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
                        case Element::Type::FRET_DIAGRAM:
                              ie = new InspectorFretDiagram(this);
                              break;
                        case Element::Type::LAYOUT_BREAK:
                              ie = new InspectorBreak(this);
                              break;
                        case Element::Type::BEND:
                              ie = new InspectorBend(this);
                              break;
                        case Element::Type::TREMOLOBAR:
                              ie = new InspectorTremoloBar(this);
                              break;
                        case Element::Type::ARPEGGIO:
                              ie = new InspectorArpeggio(this);
                              break;
                        case Element::Type::BREATH:
                              ie = new InspectorCaesura(this);
                              break;
                        case Element::Type::LYRICS:
                              ie = new InspectorLyric(this);
                              break;
                        case Element::Type::STAFF_TEXT:
                              ie = new InspectorStafftext(this);
                              break;
                        default:
                              if (_element->isText()) {
                                    if (_element->type() == Element::Type::INSTRUMENT_NAME) // these are generated
                                          ie = new InspectorEmpty(this);
                                    else
                                          ie = new InspectorText(this);
                                    }
                              else {
                                    if (_element->type() == Element::Type::BRACKET) // these are generated
                                          ie = new InspectorEmpty(this);
                                    else
                                          ie = new InspectorElement(this);
                                    }
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
                        case Qt::WheelFocus:
                        case Qt::StrongFocus:
                              if (currentWidget->inherits("QComboBox")                  ||
                                  currentWidget->parent()->inherits("QAbstractSpinBox") ||
                                  currentWidget->inherits("QAbstractSpinBox")           ||
                                  currentWidget->inherits("QLineEdit")) ; //leave it like it is
                              else
                                   currentWidget->setFocusPolicy(Qt::TabFocus);
                              break;
                        case Qt::NoFocus:
                        case Qt::ClickFocus:
                                    currentWidget->setFocusPolicy(Qt::NoFocus);
                              break;
                        case Qt::TabFocus:
                              break;
                        }
                  }
            }
      _element = e;
      ie->setElement();
      }

//---------------------------------------------------------
//   changeEvent
//---------------------------------------------------------

void Inspector::changeEvent(QEvent *event)
      {
      QDockWidget::changeEvent(event);
      if (event->type() == QEvent::LanguageChange)
            retranslate();
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
//   InspectorElementBase
//---------------------------------------------------------

InspectorElementBase::InspectorElementBase(QWidget* parent)
   : InspectorBase(parent)
      {
      e.setupUi(addWidget());

      iList = {
            { P_ID::VISIBLE,   0, 0, e.visible,    e.resetVisible   },
            { P_ID::Z,         0, 0, e.z,          e.resetZ         },
            { P_ID::COLOR,     0, 0, e.color,      e.resetColor     },
            { P_ID::USER_OFF,  0, 0, e.offsetX,    e.resetX         },
            { P_ID::USER_OFF,  1, 0, e.offsetY,    e.resetY         },
            { P_ID::AUTOPLACE, 0, 0, e.autoplace,  e.resetAutoplace },
            };
      connect(e.resetAutoplace, SIGNAL(clicked()), SLOT(resetAutoplace()));
      connect(e.autoplace, SIGNAL(toggled(bool)),  SLOT(autoplaceChanged(bool)));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorElementBase::setElement()
      {
      InspectorBase::setElement();
      autoplaceChanged(inspector->element()->autoplace());
      }

//---------------------------------------------------------
//   autoplaceChanged
//---------------------------------------------------------

void InspectorElementBase::autoplaceChanged(bool val)
      {
      for (auto i : std::vector<QWidget*> { e.offsetX, e.offsetY, e.resetX, e.resetY, e.hRaster, e.vRaster })
            i->setEnabled(!val);
      }

//---------------------------------------------------------
//   resetAutoplace
//---------------------------------------------------------

void InspectorElementBase::resetAutoplace()
      {
      autoplaceChanged(true);
      }

//---------------------------------------------------------
//   InspectorElement
//---------------------------------------------------------

InspectorElement::InspectorElement(QWidget* parent)
   : InspectorElementBase(parent)
      {
      mapSignals();
      }

//---------------------------------------------------------
//   InspectorBreak
//---------------------------------------------------------

InspectorBreak::InspectorBreak(QWidget* parent)
   : InspectorBase(parent)
      {
      b.setupUi(addWidget());

      iList = {         // currently empty
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
//   InspectorTBox
//---------------------------------------------------------

InspectorTBox::InspectorTBox(QWidget* parent)
   : InspectorBase(parent)
      {
      tb.setupUi(addWidget());

      iList = {
            { P_ID::TOP_GAP,       0, 0, tb.topGap,       tb.resetTopGap       },
            { P_ID::BOTTOM_GAP,    0, 0, tb.bottomGap,    tb.resetBottomGap    },
            { P_ID::LEFT_MARGIN,   0, 0, tb.leftMargin,   tb.resetLeftMargin   },
            { P_ID::RIGHT_MARGIN,  0, 0, tb.rightMargin,  tb.resetRightMargin  },
            { P_ID::TOP_MARGIN,    0, 0, tb.topMargin,    tb.resetTopMargin    },
            { P_ID::BOTTOM_MARGIN, 0, 0, tb.bottomMargin, tb.resetBottomMargin },
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
   : InspectorElementBase(parent)
      {
      ar.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { P_ID::ARTICULATION_ANCHOR, 0, 0, ar.anchor,      ar.resetAnchor    },
            { P_ID::DIRECTION,           0, 0, ar.direction,   ar.resetDirection },
            { P_ID::TIME_STRETCH,        0, 0, ar.timeStretch, ar.resetTimeStretch },
            { P_ID::ORNAMENT_STYLE,      0, 0, ar.ornamentStyle, ar.resetOrnamentStyle },
            { P_ID::PLAY,                0, 0, ar.playArticulation, ar.resetPlayArticulation}
            };
      mapSignals(iiList);
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
   : InspectorElementBase(parent)
      {
      s.setupUi(addWidget());
      r.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { P_ID::LEADING_SPACE,  0, 1, s.leadingSpace,  s.resetLeadingSpace  },
            { P_ID::SMALL,          0, 0, r.small,         r.resetSmall         },
            };
      mapSignals(iiList);

      //
      // Select
      //
      QLabel* l = new QLabel;
      l->setText(tr("Select"));
      QFont font(l->font());
      font.setBold(true);
      l->setFont(font);
      l->setAlignment(Qt::AlignHCenter);
      _layout->addWidget(l);
      QFrame* f = new QFrame;
      f->setFrameStyle(QFrame::HLine | QFrame::Raised);
      f->setLineWidth(2);
      _layout->addWidget(f);

      QHBoxLayout* hbox = new QHBoxLayout;
      tuplet = new QToolButton(this);
      tuplet->setText(tr("Tuplet"));
      tuplet->setEnabled(false);
      hbox->addWidget(tuplet);
      _layout->addLayout(hbox);

      e.offsetY->setSingleStep(1.0);        // step in spatium units

      connect(tuplet,   SIGNAL(clicked()),     SLOT(tupletClicked()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorRest::setElement()
      {
      Rest* rest = toRest(inspector->element());
      tuplet->setEnabled(rest->tuplet());
      InspectorElementBase::setElement();
      }

//---------------------------------------------------------
//   tupletClicked
//---------------------------------------------------------

void InspectorRest::tupletClicked()
      {
      Rest* rest = toRest(inspector->element());
      if (rest == 0)
            return;
      Tuplet* tuplet = rest->tuplet();
      if (tuplet) {
            rest->score()->select(tuplet);
            inspector->setElement(tuplet);
            rest->score()->update();
            }
      }

//---------------------------------------------------------
//   InspectorTimeSig
//---------------------------------------------------------

InspectorTimeSig::InspectorTimeSig(QWidget* parent)
   : InspectorElementBase(parent)
      {
      s.setupUi(addWidget());
      t.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { P_ID::LEADING_SPACE,  0, 1, s.leadingSpace,   s.resetLeadingSpace  },
            { P_ID::SHOW_COURTESY,  0, 0, t.showCourtesy,   t.resetShowCourtesy  },
//            { P_ID::TIMESIG,        0, 0, t.timesigZ,       t.resetTimesig       },
//            { P_ID::TIMESIG,        1, 0, t.timesigN,       t.resetTimesig       },
//            { P_ID::TIMESIG_GLOBAL, 0, 0, t.globalTimesigZ, t.resetGlobalTimesig },
//            { P_ID::TIMESIG_GLOBAL, 1, 0, t.globalTimesigN, t.resetGlobalTimesig }
            };
      mapSignals(iiList);
      }

//   InspectorTimeSig::setElement

void InspectorTimeSig::setElement()
      {
      InspectorElementBase::setElement();
      TimeSig* ts = toTimeSig(inspector->element());
      if (ts->generated())
            t.showCourtesy->setEnabled(false);
      }

//---------------------------------------------------------
//   InspectorKeySig
//---------------------------------------------------------

InspectorKeySig::InspectorKeySig(QWidget* parent)
   : InspectorElementBase(parent)
      {
      s.setupUi(addWidget());
      k.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { P_ID::LEADING_SPACE,  0, 1, s.leadingSpace,  s.resetLeadingSpace  },
            { P_ID::SHOW_COURTESY,  0, 0, k.showCourtesy,  k.resetShowCourtesy  },
//            { P_ID::SHOW_NATURALS,  0, 0, k.showNaturals,  k.resetShowNaturals  }
            };
      mapSignals(iiList);
      }

//   InspectorKeySig::setElement

void InspectorKeySig::setElement()
      {
      InspectorElementBase::setElement();
      KeySig* ks = toKeySig(inspector->element());
      if (ks->generated())
            k.showCourtesy->setEnabled(false);
      }

//---------------------------------------------------------
//   InspectorTuplet
//---------------------------------------------------------

InspectorTuplet::InspectorTuplet(QWidget* parent)
   : InspectorElementBase(parent)
      {
      t.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { P_ID::DIRECTION,    0, 0, t.direction,   t.resetDirection   },
            { P_ID::NUMBER_TYPE,  0, 0, t.numberType,  t.resetNumberType  },
            { P_ID::BRACKET_TYPE, 0, 0, t.bracketType, t.resetBracketType }
            };

      mapSignals(iiList);
      }

//---------------------------------------------------------
//   InspectorAccidental
//---------------------------------------------------------

InspectorAccidental::InspectorAccidental(QWidget* parent)
   : InspectorElementBase(parent)
      {
      a.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { P_ID::SMALL,               0, 0, a.small,       a.resetSmall       },
            { P_ID::ACCIDENTAL_BRACKET,  0, 0, a.hasBracket,  a.resetHasBracket  }
            };
      mapSignals(iiList);
      }

//---------------------------------------------------------
//   InspectorBend
//---------------------------------------------------------

InspectorBend::InspectorBend(QWidget* parent)
   : InspectorElementBase(parent)
      {
      g.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { P_ID::PLAY,         0, 0, g.playBend,    g.resetPlayBend    }
            };

      mapSignals(iiList);
      connect(g.properties, SIGNAL(clicked()), SLOT(propertiesClicked()));
      }

//---------------------------------------------------------
//   propertiesClicked
//---------------------------------------------------------

void InspectorBend::propertiesClicked()
      {
      Bend* b = toBend(inspector->element());
      Score* score = b->score();
      score->startCmd();
      mscore->currentScoreView()->editBendProperties(b);
      score->setLayoutAll();
      score->endCmd();
      }

//---------------------------------------------------------
//   InspectorTremoloBar
//---------------------------------------------------------

InspectorTremoloBar::InspectorTremoloBar(QWidget* parent)
   : InspectorElementBase(parent)
      {
      g.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { P_ID::PLAY,         0, 0, g.play,        g.resetPlay       },
            { P_ID::LINE_WIDTH,   0, 0, g.lineWidth,   g.resetLineWidth  },
            { P_ID::MAG,          0, 0, g.mag,         g.resetMag        }
            };

      mapSignals(iiList);
      connect(g.properties, SIGNAL(clicked()), SLOT(propertiesClicked()));
      }

//---------------------------------------------------------
//   propertiesClicked
//---------------------------------------------------------

void InspectorTremoloBar::propertiesClicked()
      {
      TremoloBar* b = toTremoloBar(inspector->element());
      Score* score = b->score();
      score->startCmd();
      mscore->currentScoreView()->editTremoloBarProperties(b);
      score->setLayoutAll();
      score->endCmd();
      }

//---------------------------------------------------------
//   InspectorClef
//---------------------------------------------------------

InspectorClef::InspectorClef(QWidget* parent)
   : InspectorElementBase(parent)
      {
      s.setupUi(addWidget());
      c.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { P_ID::LEADING_SPACE,  0, 1, s.leadingSpace,  s.resetLeadingSpace  },
            { P_ID::SHOW_COURTESY,  0, 0, c.showCourtesy,  c.resetShowCourtesy  }
            };
      mapSignals(iiList);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorClef::setElement()
      {
      otherClef = nullptr;                      // no 'other clef' yet
      InspectorElementBase::setElement();

      // try to locate the 'other clef' of a courtesy / main pair
      Clef* clef = toClef(inspector->element());
      // if not in a clef-segment-measure hierachy, do nothing
      if (!clef->parent() || clef->parent()->type() != Element::Type::SEGMENT)
            return;
      Segment*    segm = toSegment(clef->parent());
      int         segmTick = segm->tick();
      if (!segm->parent() || segm->parent()->type() != Element::Type::MEASURE)
            return;

      Measure* meas = toMeasure(segm->parent());
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
            otherClef = toClef(otherSegm->element(clef->track()));
      }

void InspectorClef::valueChanged(int idx)
      {
      // copy into 'other clef' the ShowCouretsy ser of this clef
      if (idx == 6 && otherClef)
            otherClef->setShowCourtesy(c.showCourtesy->isChecked());
      InspectorBase::valueChanged(idx);
      }

//---------------------------------------------------------
//   InspectorTempoText
//---------------------------------------------------------

InspectorTempoText::InspectorTempoText(QWidget* parent)
   : InspectorElementBase(parent)
      {
      t.setupUi(addWidget());
      tt.setupUi(addWidget());

      std::vector<InspectorItem> il = {
            { P_ID::TEXT_STYLE_TYPE,   0, 0, t.style,       t.resetStyle       },
            { P_ID::TEMPO,             0, 0, tt.tempo,      tt.resetTempo      },
            { P_ID::TEMPO_FOLLOW_TEXT, 0, 0, tt.followText, tt.resetFollowText }
            };
      mapSignals(il);
      connect(t.resetToStyle, SIGNAL(clicked()), SLOT(resetToStyle()));
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
                  t.style->addItem(qApp->translate("TextStyle",ts.at(i).name().toUtf8().data()), i);
            }
      t.style->blockSignals(false);
      InspectorBase::setElement();
      }

//---------------------------------------------------------
//   postInit
//---------------------------------------------------------

void InspectorTempoText::postInit()
      {
      bool followText = tt.followText->isChecked();
      //tt.resetFollowText->setDisabled(followText);
      tt.tempo->setDisabled(followText);
      tt.resetTempo->setDisabled(followText || tt.tempo->value() == 120.0);  // a default of 120 BPM is assumed all over the place
      }

//---------------------------------------------------------
//   InspectorDynamic
//---------------------------------------------------------

InspectorDynamic::InspectorDynamic(QWidget* parent)
   : InspectorElementBase(parent)
      {
      t.setupUi(addWidget());
      d.setupUi(addWidget());

      std::vector<InspectorItem> il = {
            { P_ID::TEXT_STYLE_TYPE,    0, 0, t.style,     t.resetStyle     },
            { P_ID::DYNAMIC_RANGE,      0, 0, d.dynRange,  d.resetDynRange  },
            { P_ID::VELOCITY,           0, 0, d.velocity,  d.resetVelocity  },
            { P_ID::PLACEMENT,          0, 0, d.placement, d.resetPlacement }
            };
      d.placement->clear();
      d.placement->addItem(tr("Above"), 0);
      d.placement->addItem(tr("Below"), 1);
      mapSignals(il);
      connect(t.resetToStyle, SIGNAL(clicked()), SLOT(resetToStyle()));
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
                  t.style->addItem(qApp->translate("TextStyle",ts.at(i).name().toUtf8().data()), i);
            }
      t.style->blockSignals(false);
      InspectorElementBase::setElement();
      }

//---------------------------------------------------------
//   InspectorLyric
//---------------------------------------------------------

InspectorLyric::InspectorLyric(QWidget* parent)
   : InspectorElementBase(parent)
      {
      t.setupUi(addWidget());
      l.setupUi(addWidget());

      std::vector<InspectorItem> il = {
            { P_ID::TEXT_STYLE_TYPE,    0, 0, t.style,     t.resetStyle     },
            { P_ID::PLACEMENT,          0, 0, l.placement, l.resetPlacement },
            { P_ID::VERSE,              0, 0, l.verse,     0 }
            };
      l.placement->clear();
      l.placement->addItem(tr("Above"), 0);
      l.placement->addItem(tr("Below"), 1);
      mapSignals(il);
      connect(t.resetToStyle, SIGNAL(clicked()), SLOT(resetToStyle()));
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void InspectorLyric::valueChanged(int idx)
      {
      if (iList[idx].t == P_ID::VERSE) {
            int val    = getValue(iList[idx]).toInt();
            Lyrics* l  = toLyrics(inspector->element());
            printf("value changed %d  old %d\n", val, l->no());
            Lyrics* nl = l->chordRest()->lyrics(val, l->placement());
            if (nl) {
                  printf("   move away %d -> %d\n", nl->no(), l->no());
                  nl->undoChangeProperty(P_ID::VERSE, l->no());
                  }
            }
      InspectorBase::valueChanged(idx);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorLyric::setElement()
      {
      Element* e = inspector->element();
      Score* score = e->score();

      t.style->blockSignals(true);
      t.style->clear();
      const QList<TextStyle>& ts = score->style()->textStyles();
      int n = ts.size();
      for (int i = 0; i < n; ++i) {
            if (!(ts.at(i).hidden() & TextStyleHidden::IN_LISTS) )
                  t.style->addItem(qApp->translate("TextStyle",ts.at(i).name().toUtf8().data()), i);
            }
      t.style->blockSignals(false);
      InspectorElementBase::setElement();
      }

//---------------------------------------------------------
//   InspectorStafftext
//---------------------------------------------------------

InspectorStafftext::InspectorStafftext(QWidget* parent)
   : InspectorElementBase(parent)
      {
      t.setupUi(addWidget());
      s.setupUi(addWidget());

      Element* e = inspector->element();
      Text* te = static_cast<Text*>(e);
      bool sameType = true;

      for (const auto& ee : inspector->el()) {
            Text* tt = static_cast<Text*>(ee);

            if (tt->systemFlag() != te->systemFlag()) {
                  sameType = false;
                  break;
                  }
            }
      if (sameType) {
            if (te->systemFlag())
                  s.elementName->setText(tr("System Text"));
            else
                  s.elementName->setText(tr("Staff Text"));
            }
      else
            s.elementName->setText(tr("System/Staff Text"));

      std::vector<InspectorItem> il = {
            { P_ID::TEXT_STYLE_TYPE,    0, 0, t.style,     t.resetStyle     },
            { P_ID::PLACEMENT,          0, 0, s.placement, s.resetPlacement },
            };
      s.placement->clear();
      s.placement->addItem(tr("Above"), 0);
      s.placement->addItem(tr("Below"), 1);
      mapSignals(il);
      connect(t.resetToStyle, SIGNAL(clicked()), SLOT(resetToStyle()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorStafftext::setElement()
      {
      Element* e = inspector->element();
      Score* score = e->score();

      t.style->blockSignals(true);
      t.style->clear();
      const QList<TextStyle>& ts = score->style()->textStyles();
      int n = ts.size();
      for (int i = 0; i < n; ++i) {
            if (!(ts.at(i).hidden() & TextStyleHidden::IN_LISTS) )
                  t.style->addItem(qApp->translate("TextStyle",ts.at(i).name().toUtf8().data()), i);
            }
      t.style->blockSignals(false);
      InspectorElementBase::setElement();
      }

//---------------------------------------------------------
//   InspectorSlur
//---------------------------------------------------------

InspectorSlur::InspectorSlur(QWidget* parent)
   : InspectorElementBase(parent)
      {
      s.setupUi(addWidget());

      Element* e = inspector->element();
      bool sameType = true;

      for (const auto& ee : inspector->el()) {
            if (ee->accessibleInfo() != e->accessibleInfo()) {
                  sameType = false;
                  break;
                  }
            }
      if (sameType)
            s.elementName->setText(e->accessibleInfo());

      const std::vector<InspectorItem> iiList = {
            { P_ID::LINE_TYPE,       0, 0, s.lineType,      s.resetLineType      },
            { P_ID::SLUR_DIRECTION,  0, 0, s.slurDirection, s.resetSlurDirection }
            };
      mapSignals(iiList);
      }

//---------------------------------------------------------
//   InspectorEmpty
//---------------------------------------------------------

InspectorEmpty::InspectorEmpty(QWidget* parent)
      :InspectorBase(parent)
      {
      e.setupUi(addWidget());
      }

//---------------------------------------------------------
//   sizeHint
//---------------------------------------------------------

QSize InspectorEmpty::sizeHint() const
      {
      return QSize(255 * guiScaling, 170 * guiScaling);
      }

//---------------------------------------------------------
//   InspectorBarLine
//---------------------------------------------------------

InspectorBarLine::InspectorBarLine(QWidget* parent)
   : InspectorElementBase(parent)
      {
      s.setupUi(addWidget());
      b.setupUi(addWidget());

      for (auto i : BarLine::barLineTable)
            b.type->addItem(qApp->translate("Palette", i.userName), int(i.type));

      std::vector<InspectorItem> il = {
            { P_ID::LEADING_SPACE,     0, 1, s.leadingSpace,  s.resetLeadingSpace  },
            { P_ID::BARLINE_TYPE,      0, 0, b.type,     b.resetType     },
            { P_ID::BARLINE_SPAN,      0, 0, b.span,     b.resetSpan     },
            { P_ID::BARLINE_SPAN_FROM, 0, 0, b.spanFrom, b.resetSpanFrom },
            { P_ID::BARLINE_SPAN_TO,   0, 0, b.spanTo,   b.resetSpanTo   },
            };
      mapSignals(il);
      // when any of the span parameters is changed, span data need to be managed
      connect(b.span,          SIGNAL(valueChanged(int)), SLOT(manageSpanData()));
      connect(b.spanFrom,      SIGNAL(valueChanged(int)), SLOT(manageSpanData()));
      connect(b.spanTo,        SIGNAL(valueChanged(int)), SLOT(manageSpanData()));
      connect(b.presetDefault, SIGNAL(clicked()),         SLOT(presetDefaultClicked()));
      connect(b.presetTick1,   SIGNAL(clicked()),         SLOT(presetTick1Clicked()));
      connect(b.presetTick2,   SIGNAL(clicked()),         SLOT(presetTick2Clicked()));
      connect(b.presetShort1,  SIGNAL(clicked()),         SLOT(presetShort1Clicked()));
      connect(b.presetShort2,  SIGNAL(clicked()),         SLOT(presetShort2Clicked()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

#include <QStandardItem>

void InspectorBarLine::setElement()
      {
      blockSpanDataSignals(true);
      InspectorElementBase::setElement();
      BarLine* bl = toBarLine(inspector->element());

      // enable / disable individual type combo items according to score and selected bar line status
      bool bMultiStaff  = bl->score()->nstaves() > 1;
      BarLineType blt   = bl->barLineType();
      bool isRepeat     = blt & (BarLineType::START_REPEAT | BarLineType::END_REPEAT | BarLineType::END_START_REPEAT);

      const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>(b.type->model());
      int i = 0;
      for (auto& k : BarLine::barLineTable) {
            QStandardItem* item = model->item(i);
            // if combo item is repeat type, should be disabled for multi-staff scores
            if (k.type & (BarLineType::START_REPEAT | BarLineType::END_REPEAT | BarLineType::END_START_REPEAT)) {
                  // disable / enable
                  item->setFlags(bMultiStaff ?
                        item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled) :
                        item->flags() | (Qt::ItemFlags)(Qt::ItemIsSelectable|Qt::ItemIsEnabled) );
                  }
            // if combo item is NOT repeat type, should be disabled if selected bar line is a repeat
            else {
                  item->setFlags(isRepeat ?
                        item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled) :
                        item->flags() | (Qt::ItemFlags)(Qt::ItemIsSelectable|Qt::ItemIsEnabled) );
                  }
            ++i;
            }
      manageSpanData();
      blockSpanDataSignals(false);
      }

//---------------------------------------------------------
//   presetDefaultClicked
//---------------------------------------------------------

void InspectorBarLine::presetDefaultClicked()
      {
      BarLine* bl = toBarLine(inspector->element());
      Score* score = bl->score();
      score->startCmd();

      bl->undoResetProperty(P_ID::BARLINE_SPAN);
      bl->undoResetProperty(P_ID::BARLINE_SPAN_FROM);
      bl->undoResetProperty(P_ID::BARLINE_SPAN_TO);

      score->endCmd();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   presetTick1Clicked
//---------------------------------------------------------

void InspectorBarLine::presetTick1Clicked()
      {
      BarLine* bl = toBarLine(inspector->element());
      Score* score = bl->score();
      score->startCmd();

      bl->undoChangeProperty(P_ID::BARLINE_SPAN, 1);
      bl->undoChangeProperty(P_ID::BARLINE_SPAN_FROM, BARLINE_SPAN_TICK1_FROM);
      bl->undoChangeProperty(P_ID::BARLINE_SPAN_TO,   BARLINE_SPAN_TICK1_TO);

      score->endCmd();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   presetTick2Clicked
//---------------------------------------------------------

void InspectorBarLine::presetTick2Clicked()
      {
      BarLine* bl = toBarLine(inspector->element());
      Score* score = bl->score();
      score->startCmd();

      bl->undoChangeProperty(P_ID::BARLINE_SPAN, 1);
      bl->undoChangeProperty(P_ID::BARLINE_SPAN_FROM, BARLINE_SPAN_TICK2_FROM);
      bl->undoChangeProperty(P_ID::BARLINE_SPAN_TO,   BARLINE_SPAN_TICK2_TO);

      score->endCmd();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   presetShort1Clicked
//---------------------------------------------------------

void InspectorBarLine::presetShort1Clicked()
      {
      BarLine* bl = toBarLine(inspector->element());
      Score* score = bl->score();
      score->startCmd();

      bl->undoChangeProperty(P_ID::BARLINE_SPAN, 1);
      bl->undoChangeProperty(P_ID::BARLINE_SPAN_FROM, BARLINE_SPAN_SHORT1_FROM);
      int shortDelta = bl->staff() ? (bl->staff()->lines() - 5) * 2 : 0;
      bl->undoChangeProperty(P_ID::BARLINE_SPAN_TO,   BARLINE_SPAN_SHORT1_TO + shortDelta);

      score->endCmd();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   presetShort2Clicked
//---------------------------------------------------------

void InspectorBarLine::presetShort2Clicked()
      {
      BarLine* bl = toBarLine(inspector->element());
      Score* score = bl->score();
      score->startCmd();

      bl->undoChangeProperty(P_ID::BARLINE_SPAN, 1);
      bl->undoChangeProperty(P_ID::BARLINE_SPAN_FROM, BARLINE_SPAN_SHORT2_FROM);
      int shortDelta = bl->staff() ? (bl->staff()->lines() - 5) * 2 : 0;
      bl->undoChangeProperty(P_ID::BARLINE_SPAN_TO,   BARLINE_SPAN_SHORT2_TO + shortDelta);

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
      BarLine* bl = toBarLine(inspector->element());

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
      max   = staffToLines == 1 ? BARLINE_SPAN_1LINESTAFF_TO : (staffToLines-1) * 2 + 2;

      b.spanTo->setMinimum(min);
      b.spanTo->setMaximum(max);
      b.spanTo->setWrapping(false);

      // determin MAX for SPAN
      max = bl->score()->nstaves() - bl->staffIdx();
      b.span->setMaximum(max);
      b.span->setWrapping(false);
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
      }

//---------------------------------------------------------
//   InspectorCaesura
//---------------------------------------------------------

InspectorCaesura::InspectorCaesura(QWidget* parent) : InspectorElementBase(parent)
      {
      c.setupUi(addWidget());

      Breath* b = toBreath(inspector->element());
      bool sameType = true;
      for (const auto& ee : inspector->el()) {
            if (ee->accessibleInfo() != b->accessibleInfo()) {
                  sameType = false;
                  break;
                  }
            }
      if (sameType)
            c.elementName->setText(b->accessibleInfo());

      std::vector<InspectorItem> il = {
            { P_ID::PAUSE,          0, 0, c.pause,         c.resetPause         }
            };
      mapSignals(il);
      }

}

