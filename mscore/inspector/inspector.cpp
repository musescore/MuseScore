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
      if (oe != element()) {
            delete ie;
            ie  = 0;
            oe  = element();
            bool sameTypes = true;
            if (!element())
                  ie = new InspectorEmpty(this);
            else {
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
                  }
            if (!sameTypes)
                  ie = new InspectorGroupElement(this);
            else if (element()) {
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
                        case ElementType::PEDAL_SEGMENT:
                        case ElementType::LET_RING_SEGMENT:
                        case ElementType::PALM_MUTE_SEGMENT:
                              ie = new InspectorTextLine(this);
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
            { P_ID::STAFF_YOFFSET,          0, sl.yoffset,         sl.resetYoffset         },
            { P_ID::SMALL,                  0, sl.small,           sl.resetSmall           },
            { P_ID::MAG,                    0, sl.scale,           sl.resetScale           },
            { P_ID::STAFF_LINES,            0, sl.lines,           sl.resetLines           },
            { P_ID::STEP_OFFSET,            0, sl.stepOffset,      sl.resetStepOffset      },
            { P_ID::LINE_DISTANCE,          0, sl.lineDistance,    sl.resetLineDistance    },
            { P_ID::STAFF_SHOW_BARLINES,    0, sl.showBarlines,    sl.resetShowBarlines    },
            { P_ID::STAFF_SHOW_LEDGERLINES, 0, sl.showLedgerlines, sl.resetShowLedgerlines },
            { P_ID::STAFF_SLASH_STYLE,      0, sl.slashStyle,      sl.resetSlashStyle      },
            { P_ID::STAFF_NOTEHEAD_SCHEME,  0, sl.noteheadScheme,  sl.resetNoteheadScheme  },
            { P_ID::STAFF_GEN_CLEF,         0, sl.genClefs,        sl.resetGenClefs        },
            { P_ID::STAFF_GEN_TIMESIG,      0, sl.genTimesig,      sl.resetGenTimesig      },
            { P_ID::STAFF_GEN_KEYSIG,       0, sl.genKeysig,       sl.resetGenKeysig       },
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
            { P_ID::TOP_GAP,       0, vb.topGap,       vb.resetTopGap       },
            { P_ID::BOTTOM_GAP,    0, vb.bottomGap,    vb.resetBottomGap    },
            { P_ID::LEFT_MARGIN,   0, vb.leftMargin,   vb.resetLeftMargin   },
            { P_ID::RIGHT_MARGIN,  0, vb.rightMargin,  vb.resetRightMargin  },
            { P_ID::TOP_MARGIN,    0, vb.topMargin,    vb.resetTopMargin    },
            { P_ID::BOTTOM_MARGIN, 0, vb.bottomMargin, vb.resetBottomMargin },
            { P_ID::BOX_HEIGHT,    0, vb.height,       0                    }
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
            { P_ID::TOP_GAP,       0, tb.topGap,       tb.resetTopGap       },
            { P_ID::BOTTOM_GAP,    0, tb.bottomGap,    tb.resetBottomGap    },
            { P_ID::LEFT_MARGIN,   0, tb.leftMargin,   tb.resetLeftMargin   },
            { P_ID::RIGHT_MARGIN,  0, tb.rightMargin,  tb.resetRightMargin  },
            { P_ID::TOP_MARGIN,    0, tb.topMargin,    tb.resetTopMargin    },
            { P_ID::BOTTOM_MARGIN, 0, tb.bottomMargin, tb.resetBottomMargin },
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
            { P_ID::TOP_GAP,               0, hb.leftGap,  hb.resetLeftGap  },
            { P_ID::BOTTOM_GAP,            0, hb.rightGap, hb.resetRightGap },
            { P_ID::BOX_WIDTH,             0, hb.width,    0                },
            { P_ID::CREATE_SYSTEM_HEADER,  0, hb.createSystemHeader, hb.resetCreateSystemHeader }
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
            { P_ID::ARTICULATION_ANCHOR, 0, ar.anchor,           ar.resetAnchor           },
            { P_ID::DIRECTION,           0, ar.direction,        ar.resetDirection        },
            { P_ID::TIME_STRETCH,        0, ar.timeStretch,      ar.resetTimeStretch      },
            { P_ID::ORNAMENT_STYLE,      0, ar.ornamentStyle,    ar.resetOrnamentStyle    },
            { P_ID::PLAY,                0, ar.playArticulation, ar.resetPlayArticulation }
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
            { P_ID::PLACEMENT,           0, f.placement,        f.resetPlacement        },
            { P_ID::TIME_STRETCH,        0, f.timeStretch,      f.resetTimeStretch      },
            { P_ID::PLAY,                0, f.playArticulation, f.resetPlayArticulation }
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
            { P_ID::SPACE, 0, sp.height, 0 }
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
            { P_ID::LEADING_SPACE,  1, s.leadingSpace,  s.resetLeadingSpace  },
            { P_ID::SMALL,          0, r.small,         r.resetSmall         },
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
            { P_ID::LEADING_SPACE,  1, s.leadingSpace,   s.resetLeadingSpace  },
            { P_ID::SHOW_COURTESY,  0, t.showCourtesy,   t.resetShowCourtesy  },
            { P_ID::SCALE,          0, t.scale,          t.resetScale         },
//          { P_ID::TIMESIG,        0, t.timesigZ,       t.resetTimesig       },
//          { P_ID::TIMESIG,        0, t.timesigN,       t.resetTimesig       },
//          { P_ID::TIMESIG_GLOBAL, 0, t.globalTimesigZ, t.resetGlobalTimesig },
//          { P_ID::TIMESIG_GLOBAL, 0, t.globalTimesigN, t.resetGlobalTimesig }
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
            { P_ID::LEADING_SPACE,  1, s.leadingSpace,  s.resetLeadingSpace  },
            { P_ID::SHOW_COURTESY,  0, k.showCourtesy,  k.resetShowCourtesy  },
//          { P_ID::SHOW_NATURALS,  0, k.showNaturals,  k.resetShowNaturals  }
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
            { P_ID::FONT_FACE,      0, t.tupletFontFace,  t.resetTupletFontFace    },
            { P_ID::FONT_SIZE,      0, t.tupletFontSize,  t.resetTupletFontSize    },
            { P_ID::FONT_BOLD,      0, t.tupletBold,      t.resetTupletBold        },
            { P_ID::FONT_ITALIC,    0, t.tupletItalic,    t.resetTupletItalic      },
            { P_ID::FONT_UNDERLINE, 0, t.tupletUnderline, t.resetTupletUnderline   },
            { P_ID::DIRECTION,      0, t.direction,       t.resetDirection         },
            { P_ID::NUMBER_TYPE,    0, t.numberType,      t.resetNumberType        },
            { P_ID::BRACKET_TYPE,   0, t.bracketType,     t.resetBracketType       }
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
            { P_ID::SMALL,               0, a.small,    a.resetSmall    },
            { P_ID::ACCIDENTAL_BRACKET,  0, a.bracket,  a.resetBracket  }
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
            { P_ID::PLAY,           0, g.playBend,    g.resetPlayBend    },
            { P_ID::FONT_FACE,      0, g.fontFace,    g.resetFontFace    },
            { P_ID::FONT_SIZE,      0, g.fontSize,    g.resetFontSize    },
            { P_ID::FONT_BOLD,      0, g.bold,        g.resetBold        },
            { P_ID::FONT_ITALIC,    0, g.italic,      g.resetItalic      },
            { P_ID::FONT_UNDERLINE, 0, g.underline,   g.resetUnderline   },
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
            { P_ID::PLAY,       0, g.play,        g.resetPlay        },
            { P_ID::LINE_WIDTH, 0, g.lineWidth,   g.resetLineWidth   },
            { P_ID::MAG,        0, g.mag,         g.resetMag         }
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
            { P_ID::LEADING_SPACE, 1, s.leadingSpace,  s.resetLeadingSpace  },
            { P_ID::SHOW_COURTESY, 0, c.showCourtesy,  c.resetShowCourtesy  }
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
      otherClef = nullptr;                      // no 'other clef' yet
      InspectorElementBase::setElement();

      // try to locate the 'other clef' of a courtesy / main pair
      Clef* clef = toClef(inspector->element());
      // if not in a clef-segment-measure hierachy, do nothing
      if (!clef->parent() || clef->parent()->type() != ElementType::SEGMENT)
            return;
      Segment*    segm = toSegment(clef->parent());
      int         segmTick = segm->tick();
      if (!segm->parent() || segm->parent()->type() != ElementType::MEASURE)
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
            otherSegm = otherMeas->findSegment(SegmentType::Clef, segmTick);
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
   : InspectorTextBase(parent)
      {
      tt.setupUi(addWidget());

      const std::vector<InspectorItem> il = {
            { P_ID::TEMPO,             0, tt.tempo,       tt.resetTempo       },
            { P_ID::TEMPO_FOLLOW_TEXT, 0, tt.followText,  tt.resetFollowText  },
            { P_ID::PLACEMENT,         0, tt.placement,   tt.resetPlacement   }
            };
      const std::vector<InspectorPanel> ppList = {
            { tt.title, tt.panel }
            };
      tt.placement->clear();
      tt.placement->addItem(tr("Above"), 0);
      tt.placement->addItem(tr("Below"), 1);
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
            { P_ID::PLACEMENT,          0, l.placement, l.resetPlacement },
            { P_ID::VERSE,              0, l.verse,     l.resetVerse     }
            };
      const std::vector<InspectorPanel> ppList = {
            { l.title, l.panel }
            };
      l.placement->clear();
      l.placement->addItem(tr("Above"), 0);
      l.placement->addItem(tr("Below"), 1);
      mapSignals(il, ppList);
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
            Lyrics* nl = l->chordRest()->lyrics(val, l->placement());
            if (nl) {
                  nl->undoChangeProperty(P_ID::VERSE, l->no());
                  }
            }
      InspectorBase::valueChanged(idx);
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
            { P_ID::PLACEMENT,  0, s.placement,    s.resetPlacement    },
            { P_ID::SUB_STYLE,  0, s.subStyle,     s.resetSubStyle     }
            };
      const std::vector<InspectorPanel> ppList = {
            { s.title, s.panel }
            };
      s.placement->clear();
      s.placement->addItem(tr("Above"), 0);
      s.placement->addItem(tr("Below"), 1);

      s.subStyle->clear();
      for (auto ss : { SubStyle::SYSTEM, SubStyle::STAFF, SubStyle::TEMPO, SubStyle::METRONOME,
         SubStyle::REHEARSAL_MARK, SubStyle::EXPRESSION,
         SubStyle::REPEAT_LEFT, SubStyle::REPEAT_RIGHT, SubStyle::USER1, SubStyle::USER2 } )
            {
            s.subStyle->addItem(subStyleUserName(ss), int(ss));
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
            { P_ID::LINE_TYPE,       0, s.lineType,      s.resetLineType      },
            { P_ID::SLUR_DIRECTION,  0, s.slurDirection, s.resetSlurDirection }
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
            { P_ID::PAUSE,  0, c.pause,         c.resetPause         }
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
            { P_ID::BRACKET_COLUMN, 0, b.column, b.resetColumn }
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
            { P_ID::INAME_LAYOUT_POSITION, 0, i.layoutPosition, i.resetLayoutPosition }
            };
      mapSignals(il);
      }

}

