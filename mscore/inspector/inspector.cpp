//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011-2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "musescore.h"
#include "scoreview.h"

#include "inspector.h"
#include "inspectorAmbitus.h"
#include "inspectorArpeggio.h"
#include "inspectorBarline.h"
#include "inspectorBeam.h"
#include "inspectorBend.h"
#include "inspectorDynamic.h"
#include "inspectorFingering.h"
#include "inspectorFret.h"
#include "inspectorGlissando.h"
#include "inspectorGroupElement.h"
#include "inspectorHairpin.h"
#include "inspectorHarmony.h"
#include "inspectorImage.h"
#include "inspectorInstrchange.h"
#include "inspectorJump.h"
#include "inspectorLasso.h"
#include "inspectorLetRing.h"
#include "inspectorMarker.h"
#include "inspectorMeasureNumber.h"
#include "inspectorNote.h"
#include "inspectorNoteDot.h"
#include "inspectorPalmMute.h"
#include "inspectorPedal.h"
#include "inspectorText.h"
#include "inspectorTextBase.h"
#include "inspectorTextLine.h"
#include "inspectorTremoloBar.h"
#include "inspectorTrill.h"
#include "inspectorOttava.h"
#include "inspectorVibrato.h"
#include "inspectorVolta.h"

#include "libmscore/accidental.h"
#include "libmscore/articulation.h"
#include "libmscore/beam.h"
#include "libmscore/breath.h"
#include "libmscore/chord.h"
#include "libmscore/element.h"
#include "libmscore/fermata.h"
#include "libmscore/hook.h"
#include "libmscore/keysig.h"
#include "libmscore/measure.h"
#include "libmscore/mscore.h"
#include "libmscore/note.h"
#include "libmscore/notedot.h"
#include "libmscore/rest.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/slurtie.h"
#include "libmscore/staff.h"
#include "libmscore/stafftextbase.h"
#include "libmscore/stafftypechange.h"
#include "libmscore/stem.h"
#include "libmscore/timesig.h"
#include "libmscore/tuplet.h"
#include "libmscore/undo.h"

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
            reDisplayDockWidget(_inspector, visible);
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
      sa->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
      sa->setWidgetResizable(true);

//      setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
//      sa->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

      setWidget(sa);
      sa->setFocusPolicy(Qt::NoFocus);

      _inspectorEdit = false;
      ie             = 0;
      oe             = 0;
      oSameTypes     = true;
      oSameSubtypes  = true;
      _score         = 0;
//      retranslate();
      setWindowTitle(tr("Inspector"));
      }

//---------------------------------------------------------
//   retranslate
//---------------------------------------------------------

void Inspector::retranslate()
      {
      setWindowTitle(tr("Inspector"));
      sa->setAccessibleName(tr("Inspector Subwindow"));
      Score* s = _score;
      update(0);
      update(s);
      }

//---------------------------------------------------------
//   element
//---------------------------------------------------------

Element* Inspector::element() const
      {
      return el() && !el()->empty() ? (*el())[0] : 0;
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Inspector::update()
      {
      update(_score);
      }

//---------------------------------------------------------
//   el
//---------------------------------------------------------

const QList<Element*>* Inspector::el() const
      {
      return _score ? &_score->selection().elements() : 0;
      }

//---------------------------------------------------------
//   update
//---------------------------------------------------------

void Inspector::update(Score* s)
      {
      if (_inspectorEdit)     // if within an inspector-originated edit
            return;
      _score = s;
      bool sameTypes = true;
      bool sameSubtypes = true;
      if (el()) {
            for (Element* ee : *el()) {
                  if (((element()->type() != ee->type()) && // different and
                      (!element()->isSystemText()     || !ee->isStaffText())  && // neither system text nor
                      (!element()->isStaffText()      || !ee->isSystemText()) && // staff text either side and
                      (!element()->isPedalSegment()   || !ee->isTextLineSegment()) && // neither pedal nor
                      (!element()->isTextLineSegment()|| !ee->isPedalSegment())    && // text line either side and
                      (!element()->isSlurTieSegment() || !ee->isSlurTieSegment())) || // neither Slur nor Tie either side, or
                      (ee->isNote() && toNote(ee)->chord()->isGrace() != toNote(element())->chord()->isGrace())) // HACK
                        {
                        sameTypes = false;
                        break;
                        }
                      // an articulation and an ornament
                  if ((ee->isArticulation() && toArticulation(ee)->isOrnament() != toArticulation(element())->isOrnament()) ||
                      // a slur and a tie
                      (ee->isSlurTieSegment() && toSlurTieSegment(ee)->isSlurSegment() != toSlurTieSegment(element())->isSlurSegment()) ||
                      // a breath and a caesura
                      (ee->isBreath() && toBreath(ee)->isCaesura() != toBreath(element())->isCaesura()) ||
                      // a staff text and a system text
                      ((ee->isStaffText() || ee->isSystemText())
                          && (ee->type() != element()->type() || ee->isSystemText() != element()->isSystemText())))
                        sameSubtypes = false;
                  }
            }
      if (oe != element() ||
          (oSameTypes != sameTypes) ||
          (oSameSubtypes != sameSubtypes)) {
            ie  = 0;
            oe  = element();
            oSameTypes = sameTypes;
            oSameSubtypes = sameSubtypes;
            if (!element())
                  ie = new InspectorEmpty(this);
            else if (!sameTypes)
                  ie = new InspectorGroupElement(this);
            else {
                  switch (element()->type()) {
                        case ElementType::FBOX:
                        case ElementType::VBOX:
                              ie = new InspectorVBox(this);
                              break;
                        case ElementType::TBOX:
                              ie = new InspectorTBox(this);
                              break;
                        case ElementType::HBOX:
                              ie = new InspectorHBox(this);
                              break;
                        case ElementType::ARTICULATION:
                              ie = new InspectorArticulation(this);
                              break;
                        case ElementType::FERMATA:
                              ie = new InspectorFermata(this);
                              break;
                        case ElementType::SPACER:
                              ie = new InspectorSpacer(this);
                              break;
                        case ElementType::NOTE:
                              ie = new InspectorNote(this);
                              break;
                        case ElementType::ACCIDENTAL:
                              ie = new InspectorAccidental(this);
                              break;
                        case ElementType::REST:
                              if (toRest(element())->measure()->isMMRest())
                                    ie = new InspectorMMRest(this);
                              else
                                    ie = new InspectorRest(this);
                              break;
                        case ElementType::CLEF:
                              ie = new InspectorClef(this);
                              break;
                        case ElementType::TIMESIG:
                              ie = new InspectorTimeSig(this);
                              break;
                        case ElementType::KEYSIG:
                              ie = new InspectorKeySig(this);
                              break;
                        case ElementType::TUPLET:
                              ie = new InspectorTuplet(this);
                              break;
                        case ElementType::BEAM:
                              ie = new InspectorBeam(this);
                              break;
                        case ElementType::IMAGE:
                              ie = new InspectorImage(this);
                              break;
                        case ElementType::LASSO:
                              ie = new InspectorLasso(this);
                              break;
                        case ElementType::VOLTA_SEGMENT:
                              ie = new InspectorVolta(this);
                              break;
                        case ElementType::OTTAVA_SEGMENT:
                              ie = new InspectorOttava(this);
                              break;
                        case ElementType::TRILL_SEGMENT:
                              ie = new InspectorTrill(this);
                              break;
                        case ElementType::HAIRPIN_SEGMENT:
                              ie = new InspectorHairpin(this);
                              break;
                        case ElementType::TEXTLINE_SEGMENT:
                              ie = new InspectorTextLine(this);
                              break;
                        case ElementType::PEDAL_SEGMENT:
                              ie = new InspectorPedal(this);
                              break;
                        case ElementType::LET_RING_SEGMENT:
                              ie = new InspectorLetRing(this);
                              break;
                        case ElementType::PALM_MUTE_SEGMENT:
                              ie = new InspectorPalmMute(this);
                              break;
                        case ElementType::SLUR_SEGMENT:
                        case ElementType::TIE_SEGMENT:
                              ie = new InspectorSlurTie(this);
                              break;
                        case ElementType::BAR_LINE:
                              ie = new InspectorBarLine(this);
                              break;
                        case ElementType::JUMP:
                              ie = new InspectorJump(this);
                              break;
                        case ElementType::MARKER:
                              ie = new InspectorMarker(this);
                              break;
                        case ElementType::GLISSANDO:
                        case ElementType::GLISSANDO_SEGMENT:
                              ie = new InspectorGlissando(this);
                              break;
                        case ElementType::TEMPO_TEXT:
                              ie = new InspectorTempoText(this);
                              break;
                        case ElementType::DYNAMIC:
                              ie = new InspectorDynamic(this);
                              break;
                        case ElementType::AMBITUS:
                              ie = new InspectorAmbitus(this);
                              break;
                        case ElementType::FRET_DIAGRAM:
                              ie = new InspectorFretDiagram(this);
                              break;
                        case ElementType::LAYOUT_BREAK:
                              if (toLayoutBreak(element())->layoutBreakType() == LayoutBreak::Type::SECTION)
                                    ie = new InspectorSectionBreak(this);
#if 0 // currently empty and such not needed
                              else
                                    ie = new InspectorBreak(this);
#endif
                              break;
                        case ElementType::BEND:
                              ie = new InspectorBend(this);
                              break;
                        case ElementType::TREMOLO:
                              if (toTremolo(element())->customStyleApplicable())
                                    ie = new InspectorTremolo(this);
                              else
                                    ie = new InspectorElement(this);
                              break;
                        case ElementType::TREMOLOBAR:
                              ie = new InspectorTremoloBar(this);
                              break;
                        case ElementType::ARPEGGIO:
                              ie = new InspectorArpeggio(this);
                              break;
                        case ElementType::BREATH:
                              ie = new InspectorCaesura(this);
                              break;
                        case ElementType::LYRICS:
                              ie = new InspectorLyric(this);
                              break;
                        case ElementType::STAFF_TEXT:
                        case ElementType::SYSTEM_TEXT:
                        case ElementType::REHEARSAL_MARK:
                              ie = new InspectorStaffText(this);
                              break;
                        case ElementType::INSTRUMENT_CHANGE:
                              ie = new InspectorInstrumentChange(this);
                              break;
                        case ElementType::MEASURE_NUMBER:
                        case ElementType::MMREST_RANGE:
                              ie = new InspectorMeasureNumber(this);
                              break;
                        case ElementType::STAFFTYPE_CHANGE:
                              ie = new InspectorStaffTypeChange(this);
                              break;
                        case ElementType::BRACKET:
                              ie = new InspectorBracket(this);
                              break;
                        case ElementType::INSTRUMENT_NAME:
                              //ie = new InspectorIname(this);
                              break;
                        case ElementType::FINGERING:
                              ie = new InspectorFingering(this);
                              break;
                        case ElementType::STICKING:
                              ie = new InspectorText(this); // TODO: add a separate inspector for sticking?
                              break;
                        case ElementType::STEM:
                              ie = new InspectorStem(this);
                              break;
                        case ElementType::HARMONY:
                              ie = new InspectorHarmony(this);
                              break;
                        case ElementType::VIBRATO_SEGMENT:
                              ie = new InspectorVibrato(this);
                              break;
                        case ElementType::NOTEDOT:
                              ie = new InspectorNoteDot(this);
                              break;
                        default:
                              if (element()->isText()) {
                                    // don't allow footers/headers to be edited via the inspector
                                    if (toText(element())->tid() != Tid::FOOTER && toText(element())->tid() != Tid::HEADER)
                                          ie = new InspectorText(this);
                                    else
                                          ie = new InspectorEmpty(this);
                                    }
                              else {
                                    ie = new InspectorElement(this);
                                    }
                              break;
                        }
                  }
            if (!ie)
                  return;
            connect(ie, &InspectorBase::elementChanged, this, QOverload<>::of(&Inspector::update), Qt::QueuedConnection);
            if (sa->widget()) { // If old inspector exist
                  QWidget *q = sa->takeWidget();
                  q->deleteLater();
                  }
            sa->setWidget(ie);      // will destroy previous set widget, unless takeWidget() call

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
      if (ie)
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

void UiInspectorElement::setupUi(QWidget* inspectorElement)
      {
      Ui::InspectorElement::setupUi(inspectorElement);
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

      // currently empty
      iList = {};
      pList = { { b.title, b.panel } };
      mapSignals();
      }

//---------------------------------------------------------
//   InspectorSectionBreak
//---------------------------------------------------------

InspectorSectionBreak::InspectorSectionBreak(QWidget* parent)
   : InspectorBase(parent)
      {
      scb.setupUi(addWidget());

      iList = {
            { Pid::PAUSE,                    0, scb.pause,                  scb.resetPause                  },
            { Pid::START_WITH_LONG_NAMES,    0, scb.startWithLongNames,     scb.resetStartWithLongNames     },
            { Pid::START_WITH_MEASURE_ONE,   0, scb.startWithMeasureOne,    scb.resetStartWithMeasureOne    },
            { Pid::FIRST_SYSTEM_INDENTATION, 0, scb.firstSystemIndentation, scb.resetFirstSystemIndentation },
            { Pid::SHOW_COURTESY,            0, scb.showCourtesy,           scb.resetShowCourtesy           }
            };
      pList = { { scb.title, scb.panel } };
      mapSignals();
      }

//---------------------------------------------------------
//   InspectorStaffTypeChange
//---------------------------------------------------------

InspectorStaffTypeChange::InspectorStaffTypeChange(QWidget* parent)
   : InspectorBase(parent)
      {
      sl.setupUi(addWidget());

      iList = {
            { Pid::STAFF_YOFFSET,          0, sl.yoffset,         sl.resetYoffset         },
            { Pid::SMALL,                  0, sl.isSmall,         sl.resetSmall           },
            { Pid::MAG,                    0, sl.scale,           sl.resetScale           },
            { Pid::STAFF_LINES,            0, sl.lines,           sl.resetLines           },
            { Pid::STEP_OFFSET,            0, sl.stepOffset,      sl.resetStepOffset      },
            { Pid::LINE_DISTANCE,          0, sl.lineDistance,    sl.resetLineDistance    },
            { Pid::STAFF_SHOW_BARLINES,    0, sl.showBarlines,    sl.resetShowBarlines    },
            { Pid::STAFF_SHOW_LEDGERLINES, 0, sl.showLedgerlines, sl.resetShowLedgerlines },
            { Pid::STAFF_STEMLESS,         0, sl.stemless,        sl.resetStemless        },
            { Pid::HEAD_SCHEME,            0, sl.noteheadScheme,  sl.resetNoteheadScheme  },
            { Pid::STAFF_GEN_CLEF,         0, sl.genClefs,        sl.resetGenClefs        },
            { Pid::STAFF_GEN_TIMESIG,      0, sl.genTimesig,      sl.resetGenTimesig      },
            { Pid::STAFF_GEN_KEYSIG,       0, sl.genKeysig,       sl.resetGenKeysig       },
            { Pid::STAFF_INVISIBLE,        0, sl.invisible,       sl.resetInvisible       },
            { Pid::STAFF_COLOR,            0, sl.color,           sl.resetColor           },
            };
      pList = { { sl.title, sl.panel } };

      sl.noteheadScheme->clear();
      for (auto i : { NoteHead::Scheme::HEAD_NORMAL,
         NoteHead::Scheme::HEAD_PITCHNAME,
         NoteHead::Scheme::HEAD_PITCHNAME_GERMAN,
         NoteHead::Scheme::HEAD_SOLFEGE,
         NoteHead::Scheme::HEAD_SOLFEGE_FIXED,
         NoteHead::Scheme::HEAD_SHAPE_NOTE_4,
         NoteHead::Scheme::HEAD_SHAPE_NOTE_7_AIKIN,
         NoteHead::Scheme::HEAD_SHAPE_NOTE_7_FUNK,
         NoteHead::Scheme::HEAD_SHAPE_NOTE_7_WALKER} ) {
            sl.noteheadScheme->addItem(NoteHead::scheme2userName(i), int(i));
            }
      mapSignals();
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorStaffTypeChange::setElement()
      {
      InspectorBase::setElement();
      bool hasTabStaff = false;
      bool hasNonTabStaff = false;
      for (Element* ee : *(inspector->el())) {
            StaffTypeChange* stc = toStaffTypeChange(ee);
            // tab staff shouldn't have key signature
            if (stc->staffType()->group() == StaffGroup::TAB) {
                  hasTabStaff = true;
                  sl.genKeysig->setEnabled(false);
                  sl.resetGenKeysig->setEnabled(false);
                  }
            else {
                  hasNonTabStaff = true;
                  }
            }
      if (hasTabStaff && !hasNonTabStaff)
            sl.genKeysig->setChecked(false);
      }

//---------------------------------------------------------
//   InspectorVBox
//---------------------------------------------------------

InspectorVBox::InspectorVBox(QWidget* parent)
   : InspectorBase(parent)
      {
      vb.setupUi(addWidget());

      iList = {
            { Pid::TOP_GAP,       0, vb.topGap,       vb.resetTopGap       },
            { Pid::BOTTOM_GAP,    0, vb.bottomGap,    vb.resetBottomGap    },
            { Pid::LEFT_MARGIN,   0, vb.leftMargin,   vb.resetLeftMargin   },
            { Pid::RIGHT_MARGIN,  0, vb.rightMargin,  vb.resetRightMargin  },
            { Pid::TOP_MARGIN,    0, vb.topMargin,    vb.resetTopMargin    },
            { Pid::BOTTOM_MARGIN, 0, vb.bottomMargin, vb.resetBottomMargin },
            { Pid::BOX_HEIGHT,    0, vb.height,       0                    },
            { Pid::BOX_AUTOSIZE,  0, vb.enableAutoSize, vb.resetAutoSize   }
            };
      pList = { { vb.title, vb.panel } };
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
            { Pid::TOP_GAP,       0, tb.topGap,       tb.resetTopGap       },
            { Pid::BOTTOM_GAP,    0, tb.bottomGap,    tb.resetBottomGap    },
            { Pid::LEFT_MARGIN,   0, tb.leftMargin,   tb.resetLeftMargin   },
            { Pid::RIGHT_MARGIN,  0, tb.rightMargin,  tb.resetRightMargin  },
            { Pid::TOP_MARGIN,    0, tb.topMargin,    tb.resetTopMargin    },
            { Pid::BOTTOM_MARGIN, 0, tb.bottomMargin, tb.resetBottomMargin },
            };
      pList = { { tb.title, tb.panel } };
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
            { Pid::TOP_GAP,               0, hb.leftGap,  hb.resetLeftGap  },
            { Pid::BOTTOM_GAP,            0, hb.rightGap, hb.resetRightGap },
            { Pid::BOX_WIDTH,             0, hb.width,    0                },
            { Pid::CREATE_SYSTEM_HEADER,  0, hb.createSystemHeader, hb.resetCreateSystemHeader }
            };
      pList = { { hb.title, hb.panel } };
      mapSignals();
      }

//---------------------------------------------------------
//   InspectorArticulation
//---------------------------------------------------------

InspectorArticulation::InspectorArticulation(QWidget* parent)
   : InspectorElementBase(parent)
      {
      ar.setupUi(addWidget());

      Articulation* el = toArticulation(inspector->element());
      bool sameTypes = true;

      for (const auto& ee : *inspector->el()) {
            if (toArticulation(ee)->isOrnament() != el->isOrnament()) {
                  sameTypes = false;
                  break;
                  }
            }
      if (sameTypes)
            ar.title->setText(el->isOrnament() ? tr("Ornament") : tr("Articulation"));

      const std::vector<InspectorItem> iiList = {
            { Pid::ARTICULATION_ANCHOR, 0, ar.anchor,           ar.resetAnchor           },
            { Pid::DIRECTION,           0, ar.direction,        ar.resetDirection        },
            { Pid::TIME_STRETCH,        0, ar.timeStretch,      ar.resetTimeStretch      },
            { Pid::ORNAMENT_STYLE,      0, ar.ornamentStyle,    ar.resetOrnamentStyle    },
            { Pid::PLAY,                0, ar.playArticulation, ar.resetPlayArticulation }
            };
      const std::vector<InspectorPanel> ppList = { { ar.title, ar.panel } };
      mapSignals(iiList, ppList);
      connect(ar.properties, SIGNAL(clicked()), SLOT(propertiesClicked()));
      }

//---------------------------------------------------------
//   propertiesClicked
//---------------------------------------------------------

void InspectorArticulation::propertiesClicked()
      {
      Articulation* a = toArticulation(inspector->element());
      Score* score = a->score();
      score->startCmd();
      mscore->currentScoreView()->editArticulationProperties(a);
      a->triggerLayoutAll();
      score->endCmd();
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorArticulation::setElement()
      {
      InspectorElementBase::setElement();
      if (!ar.playArticulation->isChecked())
            ar.gridWidget->setEnabled(false);
      }

//---------------------------------------------------------
//   InspectorFermata
//---------------------------------------------------------

InspectorFermata::InspectorFermata(QWidget* parent)
   : InspectorElementBase(parent)
      {
      f.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { Pid::PLACEMENT,           0, f.placement,        f.resetPlacement        },
            { Pid::TIME_STRETCH,        0, f.timeStretch,      f.resetTimeStretch      },
            { Pid::PLAY,                0, f.playArticulation, f.resetPlayArticulation }
            };
      const std::vector<InspectorPanel> ppList = { { f.title, f.panel } };
      mapSignals(iiList, ppList);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorFermata::setElement()
      {
      InspectorElementBase::setElement();
      if (!f.playArticulation->isChecked()) {
            f.labelTimeStretch->setEnabled(false);
            f.timeStretch->setEnabled(false);
            f.resetTimeStretch->setEnabled(false);
            }
      }

//---------------------------------------------------------
//   InspectorSpacer
//---------------------------------------------------------

InspectorSpacer::InspectorSpacer(QWidget* parent)
   : InspectorBase(parent)
      {
      sp.setupUi(addWidget());

      iList = {
            { Pid::SPACE, 0, sp.height, 0 }
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
            { Pid::LEADING_SPACE,  1, s.leadingSpace,  s.resetLeadingSpace  },
            { Pid::SMALL,          0, r.isSmall,       r.resetSmall         },
            };
      const std::vector<InspectorPanel> ppList = {
            { s.title, s.panel },
            { r.title, r.panel }
            };
      mapSignals(iiList, ppList);

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

      QVBoxLayout* vbox = new QVBoxLayout;
      vbox->setSpacing(3);
      vbox->setContentsMargins(3,3,3,3);
      _layout->addLayout(vbox);

      QHBoxLayout* hbox = new QHBoxLayout;
      hbox->setSpacing(3);
      dot1 = new QPushButton(this);
      dot1->setText(tr("Dot 1"));
      hbox->addWidget(dot1);
      dot2 = new QPushButton(this);
      dot2->setText(tr("Dot 2"));
      hbox->addWidget(dot2);
      dot3 = new QPushButton(this);
      dot3->setText(tr("Dot 3"));
      hbox->addWidget(dot3);
      vbox->addLayout(hbox);

      hbox = new QHBoxLayout;
      hbox->setSpacing(3);
      dot4 = new QPushButton(this);
      dot4->setText(tr("Dot 4"));
      hbox->addWidget(dot4);
      tuplet = new QPushButton(this);
      tuplet->setText(tr("Tuplet"));
      tuplet->setEnabled(false);
      hbox->addWidget(tuplet);
      vbox->addLayout(hbox);

//TODO      e.offset->setSingleStep(1.0);        // step in spatium units

      connect(dot1,     SIGNAL(clicked()),     SLOT(dot1Clicked()));
      connect(dot2,     SIGNAL(clicked()),     SLOT(dot2Clicked()));
      connect(dot3,     SIGNAL(clicked()),     SLOT(dot3Clicked()));
      connect(dot4,     SIGNAL(clicked()),     SLOT(dot4Clicked()));
      connect(tuplet,   SIGNAL(clicked()),     SLOT(tupletClicked()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorRest::setElement()
      {
      Rest* rest = toRest(inspector->element());
      int dots = rest->dots();
      dot1->setEnabled(dots > 0);
      dot2->setEnabled(dots > 1);
      dot3->setEnabled(dots > 2);
      dot4->setEnabled(dots > 3);
      tuplet->setEnabled(rest->tuplet());
      InspectorElementBase::setElement();
      }

//---------------------------------------------------------
//   dotClicked
//---------------------------------------------------------

void InspectorRest::dotClicked(int n)
      {
      Rest* rest = toRest(inspector->element());
      if (rest == 0)
            return;
      if (rest->dots() > n) {
            NoteDot* dot = rest->dot(n);
            dot->score()->select(dot);
            dot->score()->update();
            inspector->update();
            }
      }

//---------------------------------------------------------
//   dot1Clicked
//---------------------------------------------------------

void InspectorRest::dot1Clicked()
      {
      dotClicked(0);
      }

//---------------------------------------------------------
//   dot2Clicked
//---------------------------------------------------------

void InspectorRest::dot2Clicked()
      {
      dotClicked(1);
      }

//---------------------------------------------------------
//   dot3Clicked
//---------------------------------------------------------

void InspectorRest::dot3Clicked()
      {
      dotClicked(2);
      }

//---------------------------------------------------------
//   dot4Clicked
//---------------------------------------------------------

void InspectorRest::dot4Clicked()
      {
      dotClicked(3);
      }

//---------------------------------------------------------
//   tupletClicked
//---------------------------------------------------------

void InspectorRest::tupletClicked()
      {
      Rest* rest = toRest(inspector->element());
      if (rest == 0)
            return;
      Tuplet* t = rest->tuplet();
      if (t) {
            rest->score()->select(t);
            rest->score()->update();
            inspector->update();
            }
      }

//---------------------------------------------------------
//   InspectorMMRest
//---------------------------------------------------------

InspectorMMRest::InspectorMMRest(QWidget* parent)
   : InspectorElementBase(parent)
      {
      m.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { Pid::MMREST_NUMBER_POS, 0, m.yPos, m.resetYPos }
            };

      const std::vector<InspectorPanel> ppList = { { m.title, m.panel } };
      mapSignals(iiList, ppList);
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
            { Pid::LEADING_SPACE,  1, s.leadingSpace,   s.resetLeadingSpace  },
            { Pid::SHOW_COURTESY,  0, t.showCourtesy,   t.resetShowCourtesy  },
            { Pid::SCALE,          0, t.scale,          t.resetScale         },
//          { Pid::TIMESIG,        0, t.timesigZ,       t.resetTimesig       },
//          { Pid::TIMESIG,        0, t.timesigN,       t.resetTimesig       },
            };
      const std::vector<InspectorPanel> ppList = {
            { s.title, s.panel },
            { t.title, t.panel }
            };
      mapSignals(iiList, ppList);
      connect(t.properties, SIGNAL(clicked()), SLOT(propertiesClicked()));
      }

//---------------------------------------------------------
//   propertiesClicked
//---------------------------------------------------------

void InspectorTimeSig::propertiesClicked()
      {
      TimeSig* ts = toTimeSig(inspector->element());
      Score* score = ts->score();
      score->startCmd();
      mscore->currentScoreView()->editTimeSigProperties(ts);
      ts->triggerLayoutAll();
      score->endCmd();
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

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
            { Pid::LEADING_SPACE,  1, s.leadingSpace,  s.resetLeadingSpace  },
            { Pid::SHOW_COURTESY,  0, k.showCourtesy,  k.resetShowCourtesy  },
//          { Pid::SHOW_NATURALS,  0, k.showNaturals,  k.resetShowNaturals  }
            { Pid::KEYSIG_MODE,    0, k.keysigMode,    k.resetKeysigMode    }
            };
      const std::vector<InspectorPanel> ppList = {
            { s.title, s.panel },
            { k.title, k.panel }
            };
      k.keysigMode->clear();
      k.keysigMode->addItem(tr("Unknown"),    int(KeyMode::UNKNOWN));
      k.keysigMode->addItem(tr("None"),       int(KeyMode::NONE));
      k.keysigMode->addItem(tr("Major"),      int(KeyMode::MAJOR));
      k.keysigMode->addItem(tr("Minor"),      int(KeyMode::MINOR));
      k.keysigMode->addItem(tr("Dorian"),     int(KeyMode::DORIAN));
      k.keysigMode->addItem(tr("Phrygian"),   int(KeyMode::PHRYGIAN));
      k.keysigMode->addItem(tr("Lydian"),     int(KeyMode::LYDIAN));
      k.keysigMode->addItem(tr("Mixolydian"), int(KeyMode::MIXOLYDIAN));
      k.keysigMode->addItem(tr("Aeolian"),    int(KeyMode::AEOLIAN));
      k.keysigMode->addItem(tr("Ionian"),     int(KeyMode::IONIAN));
      k.keysigMode->addItem(tr("Locrian"),    int(KeyMode::LOCRIAN));
      mapSignals(iiList, ppList);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorKeySig::setElement()
      {
      InspectorElementBase::setElement();
      KeySig* ks = toKeySig(inspector->element());
      if (ks->generated()) {
            k.showCourtesy->setEnabled(false);
            k.keysigModeLabel->setEnabled(false);
            k.keysigMode->setEnabled(false);
            }
      }

//---------------------------------------------------------
//   InspectorTuplet
//---------------------------------------------------------

InspectorTuplet::InspectorTuplet(QWidget* parent)
   : InspectorElementBase(parent)
      {
      t.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { Pid::FONT_FACE,      0, t.tupletFontFace,  t.resetTupletFontFace    },
            { Pid::FONT_SIZE,      0, t.tupletFontSize,  t.resetTupletFontSize    },
            { Pid::FONT_STYLE,     0, t.tupletFontStyle, t.resetTupletFontStyle   },
            { Pid::DIRECTION,      0, t.direction,       t.resetDirection         },
            { Pid::NUMBER_TYPE,    0, t.numberType,      t.resetNumberType        },
            { Pid::BRACKET_TYPE,   0, t.bracketType,     t.resetBracketType       },
            { Pid::LINE_WIDTH,     0, t.lineWidth,       t.resetLineWidth         },
            { Pid::SIZE_SPATIUM_DEPENDENT,      0,    t.spatiumDependent,     t.resetSpatiumDependent },
            };
      const std::vector<InspectorPanel> ppList = { { t.title, t.panel } };
      mapSignals(iiList, ppList);
      }

//---------------------------------------------------------
//   InspectorAccidental
//---------------------------------------------------------

InspectorAccidental::InspectorAccidental(QWidget* parent)
   : InspectorElementBase(parent)
      {
      a.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { Pid::SMALL,               0, a.isSmall,  a.resetSmall    },
            { Pid::ACCIDENTAL_BRACKET,  0, a.bracket,  a.resetBracket  }
            };
      a.bracket->clear();
      a.bracket->addItem(tr("None", "no accidental bracket type"), int(AccidentalBracket::NONE));
      a.bracket->addItem(tr("Parenthesis"), int(AccidentalBracket::PARENTHESIS));
      a.bracket->addItem(tr("Bracket"), int(AccidentalBracket::BRACKET));
      a.bracket->addItem(tr("Brace"), int(AccidentalBracket::BRACE));

      const std::vector<InspectorPanel> ppList = { { a.title, a.panel } };
      mapSignals(iiList, ppList);
      }

//---------------------------------------------------------
//   InspectorTremolo
//---------------------------------------------------------

InspectorTremolo::InspectorTremolo(QWidget* parent)
   : InspectorElementBase(parent)
      {
      g.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { Pid::TREMOLO_STYLE, 0, g.style, g.resetStyle }
            };
      const std::vector<InspectorPanel> ppList = { { g.title, g.panel } };

      mapSignals(iiList, ppList);
      }

#if 0 // not needed currently
//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorTremolo::setElement()
      {
      InspectorElementBase::setElement();
      for (Element* ee : *(inspector->el())) {
            if (!(toTremolo(ee)->customStyleApplicable())) {
                  g.labelStyle->setVisible(false);
                  g.style->setVisible(false);
                  g.resetStyle->setVisible(false);
                  break;
                  }
            }
      }
#endif

//---------------------------------------------------------
//   InspectorClef
//---------------------------------------------------------

InspectorClef::InspectorClef(QWidget* parent)
   : InspectorElementBase(parent)
      {
      s.setupUi(addWidget());
      c.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { Pid::LEADING_SPACE, 1, s.leadingSpace,  s.resetLeadingSpace  },
            { Pid::SHOW_COURTESY, 0, c.showCourtesy,  c.resetShowCourtesy  }
            };
      const std::vector<InspectorPanel> ppList = {
            { s.title, s.panel },
            { c.title, c.panel }
            };
      mapSignals(iiList, ppList);
      }

//---------------------------------------------------------
//   InspectorStem
//---------------------------------------------------------

InspectorStem::InspectorStem(QWidget* parent)
   : InspectorElementBase(parent)
      {
      s.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { Pid::LINE_WIDTH,     0, s.lineWidth,     s.resetLineWidth     },
            { Pid::USER_LEN,       0, s.userLength,    s.resetUserLength    },
            { Pid::STEM_DIRECTION, 0, s.stemDirection, s.resetStemDirection }
            };
      const std::vector<InspectorPanel> ppList = { { s.title, s.panel } };
      mapSignals(iiList, ppList);
      }

//---------------------------------------------------------
//   populatePlacement
//---------------------------------------------------------

void populatePlacement(QComboBox* b)
      {
      b->clear();
      b->addItem(b->QObject::tr("Above"), int(Placement::ABOVE));
      b->addItem(b->QObject::tr("Below"), int(Placement::BELOW));
      }

//---------------------------------------------------------
//   InspectorTempoText
//---------------------------------------------------------

InspectorTempoText::InspectorTempoText(QWidget* parent)
   : InspectorTextBase(parent)
      {
      tt.setupUi(addWidget());

      const std::vector<InspectorItem> il = {
            { Pid::TEMPO,             0, tt.tempo,       tt.resetTempo       },
            { Pid::TEMPO_FOLLOW_TEXT, 0, tt.followText,  tt.resetFollowText  },
            { Pid::SUB_STYLE,         0, tt.style,       tt.resetStyle       },
            { Pid::PLACEMENT,         0, tt.placement,   tt.resetPlacement   }
            };
      const std::vector<InspectorPanel> ppList = {
            { tt.title, tt.panel }
            };
      populatePlacement(tt.placement);
      populateStyle(tt.style);
      mapSignals(il, ppList);
      connect(tt.followText, SIGNAL(toggled(bool)), tt.tempo, SLOT(setDisabled(bool)));
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
//   InspectorLyric
//---------------------------------------------------------

InspectorLyric::InspectorLyric(QWidget* parent)
   : InspectorTextBase(parent)
      {
      l.setupUi(addWidget());

      const std::vector<InspectorItem> il = {
            { Pid::VERSE,              0, l.verse,        l.resetVerse        },
            { Pid::SUB_STYLE,          0, l.style,        l.resetStyle        },
            { Pid::PLACEMENT,          0, l.placement,    l.resetPlacement    }
            };
      const std::vector<InspectorPanel> ppList = {
            { l.title, l.panel }
            };
      populatePlacement(l.placement);
      populateStyle(l.style);
      mapSignals(il, ppList);
      connect(t.resetToStyle, SIGNAL(clicked()), SLOT(resetToStyle()));
      }

//---------------------------------------------------------
//   InspectorStaffText
//---------------------------------------------------------

InspectorStaffText::InspectorStaffText(QWidget* parent)
   : InspectorTextBase(parent)
      {
      s.setupUi(addWidget());

      Element* el = inspector->element();
      bool sameTypes = true;

      for (const auto& ee : *inspector->el()) {
            if (el->type() != ee->type() || el->isSystemText() != ee->isSystemText()) {
                  sameTypes = false;
                  break;
                  }
            }
      if (sameTypes)
            s.title->setText(el->userName());

      const std::vector<InspectorItem> il = {
            { Pid::SUB_STYLE,  0, s.style,     s.resetStyle     },
            { Pid::PLACEMENT,  0, s.placement, s.resetPlacement }
            };
      const std::vector<InspectorPanel> ppList = {
            { s.title, s.panel }
            };
      populatePlacement(s.placement);
      populateStyle(s.style);
      mapSignals(il, ppList);
      connect(s.properties, SIGNAL(clicked()), SLOT(propertiesClicked()));
      }

//---------------------------------------------------------
//   propertiesClicked
//---------------------------------------------------------

void InspectorStaffText::propertiesClicked()
      {
      StaffTextBase* st = toStaffTextBase(inspector->element());
      Score* score = st->score();
      score->startCmd();
      mscore->currentScoreView()->editStaffTextProperties(st);
      st->triggerLayoutAll();
      score->endCmd();
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorStaffText::setElement()
      {
      InspectorTextBase::setElement();
      Element* el = inspector->element();
      s.properties->setVisible(el->isStaffText() || el->isSystemText());
      }

//---------------------------------------------------------
//   InspectorSlurTie
//---------------------------------------------------------

InspectorSlurTie::InspectorSlurTie(QWidget* parent)
   : InspectorElementBase(parent)
      {
      s.setupUi(addWidget());

      Element* el = inspector->element();
      bool sameTypes = true;

      for (const auto& ee : *inspector->el()) {
            if (ee->isSlurSegment() != el->isSlurSegment()) {
                  sameTypes = false;
                  break;
                  }
            }
      if (sameTypes)
            s.title->setText(el->isSlurSegment() ? tr("Slur") : tr("Tie"));

      const std::vector<InspectorItem> iiList = {
            { Pid::LINE_TYPE,       0, s.lineType,      s.resetLineType      },
            { Pid::SLUR_DIRECTION,  0, s.slurDirection, s.resetSlurDirection }
            };
      const std::vector<InspectorPanel> ppList = { { s.title, s.panel } };
      mapSignals(iiList, ppList);
      }

//---------------------------------------------------------
//   InspectorEmpty
//---------------------------------------------------------

InspectorEmpty::InspectorEmpty(QWidget* parent)
   : InspectorBase(parent)
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
//   InspectorCaesura
//---------------------------------------------------------

InspectorCaesura::InspectorCaesura(QWidget* parent)
   : InspectorElementBase(parent)
      {
      c.setupUi(addWidget());

      Breath* b = toBreath(inspector->element());
      bool sameTypes = true;
      for (const auto& ee : *inspector->el()) {
            if (toBreath(ee)->isCaesura() != b->isCaesura()) {
                  sameTypes = false;
                  break;
                  }
            }
      if (sameTypes)
            c.title->setText(b->isCaesura() ? tr("Caesura") : tr("Breath"));

      const std::vector<InspectorItem> iiList = {
            { Pid::PAUSE    ,  0, c.pause,         c.resetPause         },
            { Pid::PLACEMENT,  0, c.placement,     c.resetPlacement     }
            };
      const std::vector<InspectorPanel> ppList = { {c.title, c.panel} };
      populatePlacement(c.placement);
      mapSignals(iiList, ppList);
      }

//---------------------------------------------------------
//   InspectorBracket
//---------------------------------------------------------

InspectorBracket::InspectorBracket(QWidget* parent)
   : InspectorBase(parent)
      {
      b.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { Pid::BRACKET_COLUMN, 0, b.column, b.resetColumn }
            };
      const std::vector<InspectorPanel> ppList = { { b.title, b.panel } };
      mapSignals(iiList, ppList);
      }

//---------------------------------------------------------
//   InspectorIname
//---------------------------------------------------------

InspectorIname::InspectorIname(QWidget* parent)
   : InspectorTextBase(parent)
      {
      i.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { Pid::INAME_LAYOUT_POSITION, 0, i.layoutPosition, i.resetLayoutPosition }
            };
      const std::vector<InspectorPanel> ppList = { { i.title, i.panel } };
      mapSignals(iiList, ppList);
      }

}
