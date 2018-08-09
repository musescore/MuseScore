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
#include "inspectorTextBase.h"
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
#include "inspectorBarline.h"
#include "inspectorFingering.h"
#include "inspectorDynamic.h"
#include "inspectorHarmony.h"
#include "inspectorLetRing.h"
#include "inspectorPedal.h"
#include "inspectorPalmMute.h"
#include "inspectorVibrato.h"
#include "inspectorNoteDot.h"
#include "musescore.h"
#include "scoreview.h"
#include "bendproperties.h"
#include "icons.h"

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
#include "libmscore/accidental.h"

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
      if (!el())
            return;

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
            }
      if (oe != element() || oSameTypes != sameTypes) {
            delete ie;
            ie  = 0;
            oe  = element();
            oSameTypes = sameTypes;
            if (!element())
                  ie = new InspectorEmpty(this);
            else if (!sameTypes)
                  ie = new InspectorGroupElement(this);
            else {
                  switch(element()->type()) {
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
                              ie = new InspectorBreak(this);
                              break;
                        case ElementType::BEND:
                              ie = new InspectorBend(this);
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
                        case ElementType::STAFFTYPE_CHANGE:
                              ie = new InspectorStaffTypeChange(this);
                              break;
                        case ElementType::BRACKET:
                              ie = new InspectorBracket(this);
                              break;
                        case ElementType::INSTRUMENT_NAME:
                              ie = new InspectorIname(this);
                              break;
                        case ElementType::FINGERING:
                              ie = new InspectorFingering(this);
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
                              if (element()->isText())
                                    ie = new InspectorText(this);
                              else
                                    ie = new InspectorElement(this);
                              break;
                        }
                  }
            sa->setWidget(ie);      // will destroy previous set widget

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

      iList = {         // currently empty
            };

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
            { Pid::SMALL,                  0, sl.small,           sl.resetSmall           },
            { Pid::MAG,                    0, sl.scale,           sl.resetScale           },
            { Pid::STAFF_LINES,            0, sl.lines,           sl.resetLines           },
            { Pid::STEP_OFFSET,            0, sl.stepOffset,      sl.resetStepOffset      },
            { Pid::LINE_DISTANCE,          0, sl.lineDistance,    sl.resetLineDistance    },
            { Pid::STAFF_SHOW_BARLINES,    0, sl.showBarlines,    sl.resetShowBarlines    },
            { Pid::STAFF_SHOW_LEDGERLINES, 0, sl.showLedgerlines, sl.resetShowLedgerlines },
            { Pid::STAFF_SLASH_STYLE,      0, sl.slashStyle,      sl.resetSlashStyle      },
            { Pid::STAFF_NOTEHEAD_SCHEME,  0, sl.noteheadScheme,  sl.resetNoteheadScheme  },
            { Pid::STAFF_GEN_CLEF,         0, sl.genClefs,        sl.resetGenClefs        },
            { Pid::STAFF_GEN_TIMESIG,      0, sl.genTimesig,      sl.resetGenTimesig      },
            { Pid::STAFF_GEN_KEYSIG,       0, sl.genKeysig,       sl.resetGenKeysig       },
            };

      sl.noteheadScheme->clear();
      for (auto i : { NoteHeadScheme::HEAD_NORMAL,
         NoteHeadScheme::HEAD_PITCHNAME,
         NoteHeadScheme::HEAD_PITCHNAME_GERMAN,
         NoteHeadScheme::HEAD_SOLFEGE,
         NoteHeadScheme::HEAD_SOLFEGE_FIXED,
         NoteHeadScheme::HEAD_SHAPE_NOTE_4,
         NoteHeadScheme::HEAD_SHAPE_NOTE_7_AIKIN,
         NoteHeadScheme::HEAD_SHAPE_NOTE_7_FUNK,
         NoteHeadScheme::HEAD_SHAPE_NOTE_7_WALKER} ) {
            sl.noteheadScheme->addItem(StaffType::scheme2userName(i), int(i));
            }
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
            { Pid::TOP_GAP,       0, vb.topGap,       vb.resetTopGap       },
            { Pid::BOTTOM_GAP,    0, vb.bottomGap,    vb.resetBottomGap    },
            { Pid::LEFT_MARGIN,   0, vb.leftMargin,   vb.resetLeftMargin   },
            { Pid::RIGHT_MARGIN,  0, vb.rightMargin,  vb.resetRightMargin  },
            { Pid::TOP_MARGIN,    0, vb.topMargin,    vb.resetTopMargin    },
            { Pid::BOTTOM_MARGIN, 0, vb.bottomMargin, vb.resetBottomMargin },
            { Pid::BOX_HEIGHT,    0, vb.height,       0                    }
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
            { Pid::TOP_GAP,       0, tb.topGap,       tb.resetTopGap       },
            { Pid::BOTTOM_GAP,    0, tb.bottomGap,    tb.resetBottomGap    },
            { Pid::LEFT_MARGIN,   0, tb.leftMargin,   tb.resetLeftMargin   },
            { Pid::RIGHT_MARGIN,  0, tb.rightMargin,  tb.resetRightMargin  },
            { Pid::TOP_MARGIN,    0, tb.topMargin,    tb.resetTopMargin    },
            { Pid::BOTTOM_MARGIN, 0, tb.bottomMargin, tb.resetBottomMargin },
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
            { Pid::TOP_GAP,               0, hb.leftGap,  hb.resetLeftGap  },
            { Pid::BOTTOM_GAP,            0, hb.rightGap, hb.resetRightGap },
            { Pid::BOX_WIDTH,             0, hb.width,    0                },
            { Pid::CREATE_SYSTEM_HEADER,  0, hb.createSystemHeader, hb.resetCreateSystemHeader }
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
            { Pid::ARTICULATION_ANCHOR, 0, ar.anchor,           ar.resetAnchor           },
            { Pid::DIRECTION,           0, ar.direction,        ar.resetDirection        },
            { Pid::TIME_STRETCH,        0, ar.timeStretch,      ar.resetTimeStretch      },
            { Pid::ORNAMENT_STYLE,      0, ar.ornamentStyle,    ar.resetOrnamentStyle    },
            { Pid::PLAY,                0, ar.playArticulation, ar.resetPlayArticulation }
            };
      const std::vector<InspectorPanel> ppList = { { ar.title, ar.panel } };
      mapSignals(iiList, ppList);
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
            { Pid::SMALL,          0, r.small,         r.resetSmall         },
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

//TODO      e.offset->setSingleStep(1.0);        // step in spatium units

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
            rest->score()->update();
            inspector->update();
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
            { Pid::LEADING_SPACE,  1, s.leadingSpace,   s.resetLeadingSpace  },
            { Pid::SHOW_COURTESY,  0, t.showCourtesy,   t.resetShowCourtesy  },
            { Pid::SCALE,          0, t.scale,          t.resetScale         },
//          { Pid::TIMESIG,        0, t.timesigZ,       t.resetTimesig       },
//          { Pid::TIMESIG,        0, t.timesigN,       t.resetTimesig       },
//          { Pid::TIMESIG_GLOBAL, 0, t.globalTimesigZ, t.resetGlobalTimesig },
//          { Pid::TIMESIG_GLOBAL, 0, t.globalTimesigN, t.resetGlobalTimesig }
            };
      const std::vector<InspectorPanel> ppList = {
            { s.title, s.panel },
            { t.title, t.panel }
            };
      mapSignals(iiList, ppList);
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
            { Pid::LEADING_SPACE,  1, s.leadingSpace,  s.resetLeadingSpace  },
            { Pid::SHOW_COURTESY,  0, k.showCourtesy,  k.resetShowCourtesy  },
//          { Pid::SHOW_NATURALS,  0, k.showNaturals,  k.resetShowNaturals  }
            };
      const std::vector<InspectorPanel> ppList = {
            { s.title, s.panel },
            { k.title, k.panel }
            };
      mapSignals(iiList, ppList);
      }

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
            { Pid::FONT_FACE,      0, t.tupletFontFace,  t.resetTupletFontFace    },
            { Pid::FONT_SIZE,      0, t.tupletFontSize,  t.resetTupletFontSize    },
            { Pid::FONT_BOLD,      0, t.tupletBold,      t.resetTupletBold        },
            { Pid::FONT_ITALIC,    0, t.tupletItalic,    t.resetTupletItalic      },
            { Pid::FONT_UNDERLINE, 0, t.tupletUnderline, t.resetTupletUnderline   },
            { Pid::DIRECTION,      0, t.direction,       t.resetDirection         },
            { Pid::NUMBER_TYPE,    0, t.numberType,      t.resetNumberType        },
            { Pid::BRACKET_TYPE,   0, t.bracketType,     t.resetBracketType       },
            { Pid::LINE_WIDTH,     0, t.lineWidth,       t.resetLineWidth         }
            };
      const std::vector<InspectorPanel> ppList = { {t.title, t.panel} };
      t.tupletBold->setIcon(*icons[int(Icons::textBold_ICON)]);
      t.tupletUnderline->setIcon(*icons[int(Icons::textUnderline_ICON)]);
      t.tupletItalic->setIcon(*icons[int(Icons::textItalic_ICON)]);
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
            { Pid::SMALL,               0, a.small,    a.resetSmall    },
            { Pid::ACCIDENTAL_BRACKET,  0, a.bracket,  a.resetBracket  }
            };
      a.bracket->clear();
      a.bracket->addItem(tr("None"), int(AccidentalBracket::NONE));
      a.bracket->addItem(tr("Parenthesis"), int(AccidentalBracket::PARENTHESIS));
      a.bracket->addItem(tr("Bracket"), int(AccidentalBracket::BRACKET));

      const std::vector<InspectorPanel> ppList = { { a.title, a.panel } };
      mapSignals(iiList, ppList);
      }

//---------------------------------------------------------
//   InspectorBend
//---------------------------------------------------------

InspectorBend::InspectorBend(QWidget* parent)
   : InspectorElementBase(parent)
      {
      g.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { Pid::LINE_WIDTH,     0, g.lineWidth,   g.resetLineWidth   },
            { Pid::PLAY,           0, g.playBend,    g.resetPlayBend    },
            { Pid::FONT_FACE,      0, g.fontFace,    g.resetFontFace    },
            { Pid::FONT_SIZE,      0, g.fontSize,    g.resetFontSize    },
            { Pid::FONT_BOLD,      0, g.bold,        g.resetBold        },
            { Pid::FONT_ITALIC,    0, g.italic,      g.resetItalic      },
            { Pid::FONT_UNDERLINE, 0, g.underline,   g.resetUnderline   },
            };
      const std::vector<InspectorPanel> ppList = { {g.title, g.panel} };
      g.bold->setIcon(*icons[int(Icons::textBold_ICON)]);
      g.underline->setIcon(*icons[int(Icons::textUnderline_ICON)]);
      g.italic->setIcon(*icons[int(Icons::textItalic_ICON)]);
      mapSignals(iiList, ppList);
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
            { Pid::PLAY,       0, g.play,        g.resetPlay        },
            { Pid::LINE_WIDTH, 0, g.lineWidth,   g.resetLineWidth   },
            { Pid::MAG,        0, g.mag,         g.resetMag         }
            };
      const std::vector<InspectorPanel> ppList = { { g.title, g.panel } };

      mapSignals(iiList, ppList);
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
//   setElement
//---------------------------------------------------------

void InspectorClef::setElement()
      {
      otherClef = toClef(inspector->element())->otherClef();
      InspectorElementBase::setElement();
      }

void InspectorClef::valueChanged(int idx)
      {
      // copy into 'other clef' the ShowCouretsy ser of this clef
      if (idx == 6 && otherClef)
            otherClef->setShowCourtesy(c.showCourtesy->isChecked());
      InspectorBase::valueChanged(idx);
      }

//---------------------------------------------------------
//   InspectorStem
//---------------------------------------------------------

InspectorStem::InspectorStem(QWidget* parent)
   : InspectorElementBase(parent)
      {
      s.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { Pid::LINE_WIDTH, 0, s.lineWidth,  s.resetLineWidth  },
            { Pid::USER_LEN,   0, s.userLength, s.resetUserLength },
            };
      mapSignals(iiList);
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
            { Pid::PLACEMENT,         0, tt.placement,   tt.resetPlacement   }
            };
      const std::vector<InspectorPanel> ppList = {
            { tt.title, tt.panel }
            };
      populatePlacement(tt.placement);
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
            { Pid::PLACEMENT,          0, l.placement, l.resetPlacement },
            { Pid::VERSE,              0, l.verse,     l.resetVerse     }
            };
      const std::vector<InspectorPanel> ppList = {
            { l.title, l.panel }
            };
      populatePlacement(l.placement);
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

      Element* e = inspector->element();
      bool sameTypes = true;

      for (const auto& ee : *inspector->el()) {
            if (e->isSystemText() != ee->isSystemText()) {
                  sameTypes = false;
                  break;
                  }
            }
      if (sameTypes)
            s.title->setText(e->isSystemText() ? tr("System Text") : tr("Staff Text"));

      const std::vector<InspectorItem> il = {
            { Pid::PLACEMENT,  0, s.placement, s.resetPlacement },
            { Pid::SUB_STYLE,  0, s.style,     s.resetStyle     }
            };
      const std::vector<InspectorPanel> ppList = {
            { s.title, s.panel }
            };
      populatePlacement(s.placement);

      s.style->clear();
      for (auto ss : {
         Tid::SYSTEM,
         Tid::STAFF,
         Tid::TEMPO,
         Tid::METRONOME,
         Tid::REHEARSAL_MARK,
         Tid::EXPRESSION,
         Tid::REPEAT_LEFT,
         Tid::REPEAT_RIGHT,
         Tid::FRAME,
         Tid::TITLE,
         Tid::SUBTITLE,
         Tid::COMPOSER,
         Tid::POET,
         Tid::INSTRUMENT_EXCERPT,
         Tid::TRANSLATOR,
         Tid::HEADER,
         Tid::FOOTER,
         Tid::USER1,
         Tid::USER2,
         Tid::USER3,
         Tid::USER4,
         Tid::USER5,
         Tid::USER6
         } )
            {
            s.style->addItem(textStyleUserName(ss), int(ss));
            }

      mapSignals(il, ppList);
      }

//---------------------------------------------------------
//   InspectorSlurTie
//---------------------------------------------------------

InspectorSlurTie::InspectorSlurTie(QWidget* parent)
   : InspectorElementBase(parent)
      {
      s.setupUi(addWidget());

      Element* e = inspector->element();
      bool sameTypes = true;

      for (const auto& ee : *inspector->el()) {
            if (ee->accessibleInfo() != e->accessibleInfo()) {
                  sameTypes = false;
                  break;
                  }
            }
      if (sameTypes)
            s.title->setText(e->accessibleInfo());

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
//   InspectorCaesura
//---------------------------------------------------------

InspectorCaesura::InspectorCaesura(QWidget* parent) : InspectorElementBase(parent)
      {
      c.setupUi(addWidget());

      Breath* b = toBreath(inspector->element());
      bool sameTypes = true;
      for (const auto& ee : *inspector->el()) {
            if (ee->accessibleInfo() != b->accessibleInfo()) {
                  sameTypes = false;
                  break;
                  }
            }
      if (sameTypes)
            c.title->setText(b->accessibleInfo());

      const std::vector<InspectorItem> il = {
            { Pid::PAUSE,  0, c.pause,         c.resetPause         }
            };
      const std::vector<InspectorPanel> ppList = { {c.title, c.panel} };
      mapSignals(il, ppList);
      }

//---------------------------------------------------------
//   InspectorBracket
//---------------------------------------------------------

InspectorBracket::InspectorBracket(QWidget* parent) : InspectorBase(parent)
      {
      b.setupUi(addWidget());

      const std::vector<InspectorItem> il = {
            { Pid::BRACKET_COLUMN, 0, b.column, b.resetColumn }
            };
      mapSignals(il);
      }

//---------------------------------------------------------
//   InspectorIname
//---------------------------------------------------------

InspectorIname::InspectorIname(QWidget* parent) : InspectorTextBase(parent)
      {
      i.setupUi(addWidget());

      const std::vector<InspectorItem> il = {
            { Pid::INAME_LAYOUT_POSITION, 0, i.layoutPosition, i.resetLayoutPosition }
            };
      mapSignals(il);
      }

}

