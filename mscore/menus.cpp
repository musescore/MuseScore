//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2016 Werner Schweer and others
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

// For menus in the menu bar, like File, Edit, and View, see mscore/musescore.cpp

#include "menus.h"
#include "musescore.h"
#include "palette.h"
#include "scoreview.h"
#include "shortcut.h"
#include "symboldialog.h"
#include "workspace.h"

#include "libmscore/accidental.h"
#include "libmscore/ambitus.h"
#include "libmscore/arpeggio.h"
#include "libmscore/articulation.h"
#include "libmscore/bagpembell.h"
#include "libmscore/barline.h"
#include "libmscore/bend.h"
#include "libmscore/bracket.h"
#include "libmscore/breath.h"
#include "libmscore/chord.h"
#include "libmscore/chordline.h"
#include "libmscore/chordrest.h"
#include "libmscore/clef.h"
#include "libmscore/dynamic.h"
#include "libmscore/fermata.h"
#include "libmscore/fingering.h"
#include "libmscore/fret.h"
#include "libmscore/glissando.h"
#include "libmscore/hairpin.h"
#include "libmscore/icon.h"
#include "libmscore/instrchange.h"
#include "libmscore/jump.h"
#include "libmscore/keysig.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/letring.h"
#include "libmscore/marker.h"
#include "libmscore/measure.h"
#include "libmscore/measurenumber.h"
#include "libmscore/note.h"
#include "libmscore/ottava.h"
#include "libmscore/palmmute.h"
#include "libmscore/pedal.h"
#include "libmscore/rehearsalmark.h"
#include "libmscore/repeat.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/select.h"
#include "libmscore/slur.h"
#include "libmscore/spacer.h"
#include "libmscore/stafftext.h"
#include "libmscore/stafftypechange.h"
#include "libmscore/sym.h"
#include "libmscore/systemtext.h"
#include "libmscore/tempotext.h"
#include "libmscore/textline.h"
#include "libmscore/timesig.h"
#include "libmscore/tremolo.h"
#include "libmscore/tremolobar.h"
#include "libmscore/trill.h"
#include "libmscore/undo.h"
#include "libmscore/vibrato.h"
#include "libmscore/volta.h"

#include "palette/palettetree.h"
#include "palette/palettewidget.h"
#include "palette/paletteworkspace.h"

#include "qml/msqmlengine.h"

namespace Ms {

extern bool useFactorySettings;

static Palette* toPalette(PalettePanel* pp)
      {
      return new Palette(std::unique_ptr<PalettePanel>(pp));
      }

//---------------------------------------------------------
//   populateIconPalette
//---------------------------------------------------------

void populateIconPalette(Palette* p, const IconAction* a)
      {
      while (a->subtype != IconType::NONE) {
            Icon* ik = new Icon(gscore);
            ik->setIconType(a->subtype);
            const Shortcut* s = Shortcut::getShortcut(a->action);
            QAction* action = s->action();
            QIcon icon(action->icon());
            ik->setAction(a->action, icon);
            p->append(ik, s->help());
            ++a;
            }
      }

//---------------------------------------------------------
//   newBeamPalette
//---------------------------------------------------------

Palette* MuseScore::newBeamPalette()
      {
      return toPalette(newBeamPalettePanel());
      }

//---------------------------------------------------------
//   newFramePalette
//---------------------------------------------------------

Palette* MuseScore::newFramePalette()
      {
      return toPalette(newFramePalettePanel());
      }

//---------------------------------------------------------
//   newDynamicsPalette
//---------------------------------------------------------

Palette* MuseScore::newDynamicsPalette(bool defaultPalette)
      {
      return toPalette(newDynamicsPalettePanel(defaultPalette));
      }

//---------------------------------------------------------
//   newKeySigPalette
//---------------------------------------------------------

Palette* MuseScore::newKeySigPalette()
      {
      return toPalette(newKeySigPalettePanel());
      }

//---------------------------------------------------------
//   newAccidentalsPalette
//---------------------------------------------------------

Palette* MuseScore::newAccidentalsPalette(bool defaultPalette)
      {
      return toPalette(newAccidentalsPalettePanel(defaultPalette));
      }

//---------------------------------------------------------
//   newBarLinePalette
//---------------------------------------------------------

Palette* MuseScore::newBarLinePalette()
      {
      return toPalette(newBarLinePalettePanel());
      }

//---------------------------------------------------------
//   newRepeatsPalette
//---------------------------------------------------------

Palette* MuseScore::newRepeatsPalette()
      {
      return toPalette(newRepeatsPalettePanel());
      }

//---------------------------------------------------------
//   newBreaksPalette
//---------------------------------------------------------

Palette* MuseScore::newBreaksPalette()
      {
      return toPalette(newBreaksPalettePanel());
      }

//---------------------------------------------------------
//   newFingeringPalette
//---------------------------------------------------------

Palette* MuseScore::newFingeringPalette()
      {
      return toPalette(newFingeringPalettePanel());
      }

//---------------------------------------------------------
//   newTremoloPalette
//---------------------------------------------------------

Palette* MuseScore::newTremoloPalette()
      {
      return toPalette(newTremoloPalettePanel());
      }

//---------------------------------------------------------
//   newNoteHeadsPalette
//---------------------------------------------------------

Palette* MuseScore::newNoteHeadsPalette()
      {
      return toPalette(newNoteHeadsPalettePanel());
      }

//---------------------------------------------------------
//   newArticulationsPalette
//---------------------------------------------------------

Palette* MuseScore::newArticulationsPalette()
      {
      return toPalette(newArticulationsPalettePanel());
      }

//---------------------------------------------------------
//   newOrnamentsPalette
//---------------------------------------------------------

Palette* MuseScore::newOrnamentsPalette()
      {
      return toPalette(newOrnamentsPalettePanel());
      }

//---------------------------------------------------------
//   newAccordionPalette
//---------------------------------------------------------

Palette* MuseScore::newAccordionPalette()
      {
      return toPalette(newAccordionPalettePanel());
      }

//---------------------------------------------------------
//   newBracketsPalette
//---------------------------------------------------------

Palette* MuseScore::newBracketsPalette()
      {
      return toPalette(newBracketsPalettePanel());
      }

//---------------------------------------------------------
//   newBreathPalette
//---------------------------------------------------------

Palette* MuseScore::newBreathPalette()
      {
      return toPalette(newBreathPalettePanel());
      }

//---------------------------------------------------------
//   newArpeggioPalette
//---------------------------------------------------------

Palette* MuseScore::newArpeggioPalette()
      {
      return toPalette(newArpeggioPalettePanel());
      }

//---------------------------------------------------------
//   newClefsPalette
//---------------------------------------------------------

Palette* MuseScore::newClefsPalette(bool defaultPalette)
      {
      return toPalette(newClefsPalettePanel(defaultPalette));
      }

//---------------------------------------------------------
//   newGraceNotePalette
//---------------------------------------------------------

Palette* MuseScore::newGraceNotePalette()
      {
      return toPalette(newGraceNotePalettePanel());
      }

//---------------------------------------------------------
//   newBagpipeEmbellishmentPalette
//---------------------------------------------------------

Palette* MuseScore::newBagpipeEmbellishmentPalette()
      {
      return toPalette(newBagpipeEmbellishmentPalettePanel());
      }

//---------------------------------------------------------
//   newLinesPalette
//---------------------------------------------------------

Palette* MuseScore::newLinesPalette()
      {
      return toPalette(newLinesPalettePanel());
      }

//---------------------------------------------------------
//   showPalette
//---------------------------------------------------------

void MuseScore::showPalette(bool visible)
      {
      QAction* a = getAction("toggle-palette");
      if (!paletteWidget) {
            WorkspacesManager::currentWorkspace()->read();
            preferencesChanged();
            updateIcons();

            paletteWidget = new PaletteWidget(getPaletteWorkspace(), getQmlUiEngine(), this);
            a = getAction("toggle-palette");
            connect(paletteWidget, &PaletteWidget::visibilityChanged, a, &QAction::setChecked);
            addDockWidget(Qt::LeftDockWidgetArea, paletteWidget);
            }
      reDisplayDockWidget(paletteWidget, visible);
      a->setChecked(visible);
      }

//---------------------------------------------------------
//   TempoPattern
//---------------------------------------------------------

struct TempoPattern {
      QString pattern;
      const char* name;
      double f;
      bool relative;
      bool italian;
      bool followText;
      bool basic;
      bool masterOnly;

      TempoPattern(const QString& s, const char* n, double v, bool r, bool i, bool f, bool b, bool m) : pattern(s), name(n), f(v), relative(r), italian(i), followText(f), basic(b), masterOnly(m) {}
      };

//---------------------------------------------------------
//   newTempoPalette
//---------------------------------------------------------

Palette* MuseScore::newTempoPalette(bool defaultPalette)
      {
      return toPalette(newTempoPalettePanel(defaultPalette));
      }

//---------------------------------------------------------
//   newTextPalette
//---------------------------------------------------------

Palette* MuseScore::newTextPalette(bool defaultPalette)
      {
      return toPalette(newTextPalettePanel(defaultPalette));
      }

//---------------------------------------------------------
//   newTimePalette
//    create default time signature palette
//---------------------------------------------------------

Palette* MuseScore::newTimePalette()
      {
      return toPalette(newTimePalettePanel());
      }

//---------------------------------------------------------
//    newFretboardDiagramPalette
//---------------------------------------------------------

Palette* MuseScore::newFretboardDiagramPalette()
      {
      return toPalette(newFretboardDiagramPalettePanel());
      }

//---------------------------------------------------------
//   newMasterPaletteTree
//---------------------------------------------------------

PaletteTree* MuseScore::newMasterPaletteTree()
      {
      PaletteTree* tree = new PaletteTree();

      tree->append(MuseScore::newClefsPalettePanel());
      tree->append(MuseScore::newKeySigPalettePanel());
      tree->append(MuseScore::newTimePalettePanel());

      tree->append(MuseScore::newBracketsPalettePanel());
      tree->append(MuseScore::newAccidentalsPalettePanel());
      tree->append(MuseScore::newArticulationsPalettePanel());
      tree->append(MuseScore::newOrnamentsPalettePanel());
      tree->append(MuseScore::newBreathPalettePanel());
      tree->append(MuseScore::newGraceNotePalettePanel());
      tree->append(MuseScore::newNoteHeadsPalettePanel());
      tree->append(MuseScore::newLinesPalettePanel());
      tree->append(MuseScore::newBarLinePalettePanel());
      tree->append(MuseScore::newArpeggioPalettePanel());
      tree->append(MuseScore::newTremoloPalettePanel());
      tree->append(MuseScore::newTextPalettePanel());
      tree->append(MuseScore::newTempoPalettePanel());
      tree->append(MuseScore::newDynamicsPalettePanel());
      tree->append(MuseScore::newFingeringPalettePanel());
      tree->append(MuseScore::newRepeatsPalettePanel());
      tree->append(MuseScore::newFretboardDiagramPalettePanel());
      tree->append(MuseScore::newAccordionPalettePanel());
      tree->append(MuseScore::newBagpipeEmbellishmentPalettePanel());
      tree->append(MuseScore::newBreaksPalettePanel());
      tree->append(MuseScore::newFramePalettePanel());
      tree->append(MuseScore::newBeamPalettePanel());

      return tree;
      }

//---------------------------------------------------------
//   populateIconPalettePanel
//---------------------------------------------------------

static void populateIconPalettePanel(PalettePanel* p, const IconAction* a)
      {
      while (a->subtype != IconType::NONE) {
            Icon* ik = new Icon(gscore);
            ik->setIconType(a->subtype);
            const Shortcut* s = Shortcut::getShortcut(a->action);
            QAction* action = s->action();
            QIcon icon(action->icon());
            ik->setAction(a->action, icon);
            p->append(ik, s->help());
            ++a;
            }
      }

//---------------------------------------------------------
//   newBeamPalettePanel
//---------------------------------------------------------

PalettePanel* MuseScore::newBeamPalettePanel()
      {
      PalettePanel* sp = new PalettePanel(PalettePanel::Type::Beam);
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Beam Properties"));
      sp->setGrid(27, 40);
      sp->setDrawGrid(true);

      const IconAction bpa[] = {
            { IconType::SBEAM,    "beam-start" },
            { IconType::MBEAM,    "beam-mid" },
            { IconType::NBEAM,    "no-beam" },
            { IconType::BEAM32,   "beam32" },
            { IconType::BEAM64,   "beam64" },
            { IconType::AUTOBEAM, "auto-beam" },
            { IconType::FBEAM1,   "fbeam1" },
            { IconType::FBEAM2,   "fbeam2" },
            { IconType::NONE,     ""}
            };

      populateIconPalettePanel(sp, bpa);
      return sp;
      }

//---------------------------------------------------------
//   newFramePalettePanel
//---------------------------------------------------------

PalettePanel* MuseScore::newFramePalettePanel()
      {
      PalettePanel* sp = new PalettePanel(PalettePanel::Type::Frame);
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Frames & Measures"));
      sp->setGrid(27, 40);
      sp->setDrawGrid(true);

      if (enableExperimental) {
            static const IconAction bpa[] = {
                  { IconType::VFRAME,   "insert-vbox" },
                  { IconType::HFRAME,   "insert-hbox" },
                  { IconType::TFRAME,   "insert-textframe" },
                  { IconType::FFRAME,   "insert-fretframe" },    // experimental
                  { IconType::MEASURE,  "insert-measure" },
                  { IconType::NONE,     ""}
                  };
            populateIconPalettePanel(sp, bpa);
            }
      else {
            static const IconAction bpa[] = {
                  { IconType::VFRAME,   "insert-vbox" },
                  { IconType::HFRAME,   "insert-hbox" },
                  { IconType::TFRAME,   "insert-textframe" },
                  { IconType::MEASURE,  "insert-measure" },
                  { IconType::NONE,     ""}
                };
            populateIconPalettePanel(sp, bpa);
            }

      return sp;
      }

//---------------------------------------------------------
//   newDynamicsPalettePanel
//---------------------------------------------------------

PalettePanel* MuseScore::newDynamicsPalettePanel(bool defaultPalettePanel)
      {
      PalettePanel* sp = new PalettePanel(PalettePanel::Type::Dynamic);
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Dynamics"));
      sp->setMag(.8);
      sp->setDrawGrid(true);

      static const std::vector<const char*> array1 = {
            "pppppp", "ppppp", "pppp",
            "ppp", "pp", "p", "mp", "mf", "f", "ff", "fff",
            "ffff", "fffff", "ffffff",
            "fp", "pf", "sf", "sfz", "sff", "sffz", "sfff", "sfffz", "sfp", "sfpp",
            "rfz", "rf", "fz", "m", "r", "s", "z", "n"
            };
      static const std::vector<const char*> arrayDefault = {
            "ppp", "pp", "p", "mp", "mf", "f", "ff", "fff",
            "fp", "pf", "sf", "sfz", "sff", "sffz", "sfp", "sfpp",
            "rfz", "rf", "fz", "m", "r", "s", "z", "n"
            };

      const std::vector<const char*>* array = nullptr;
      if (defaultPalettePanel) {
            array = &arrayDefault;
            sp->setGrid(42, 28);
            sp->setMoreElements(true);
            }
      else {
            array = &array1;
            sp->setGrid(60, 28);
            }

      for (const char* c :  *array) {
            Dynamic* dynamic = new Dynamic(gscore);
            dynamic->setDynamicType(c);
            sp->append(dynamic, dynamic->dynamicTypeName());
            }
      return sp;
      }

//---------------------------------------------------------
//   newKeySigPalettePanel
//---------------------------------------------------------

PalettePanel* MuseScore::newKeySigPalettePanel()
      {
      PalettePanel* sp = new PalettePanel(PalettePanel::Type::KeySig);
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Key Signatures"));
      sp->setMag(1.0);
      sp->setGrid(56, 55);
      sp->setYOffset(1.0);

      for (int i = 0; i < 7; ++i) {
            KeySig* k = new KeySig(gscore);
            k->setKey(Key(i + 1));
            sp->append(k, keyNames[i*2]);
            }
      for (int i = -7; i < 0; ++i) {
            KeySig* k = new KeySig(gscore);
            k->setKey(Key(i));
            sp->append(k, keyNames[(7 + i) * 2 + 1]);
            }
      KeySig* k = new KeySig(gscore);
      k->setKey(Key::C);
      sp->append(k, keyNames[14]);

      // atonal key signature
      KeySigEvent nke;
      nke.setKey(Key::C);
      nke.setCustom(true);
      nke.setMode(KeyMode::NONE);
      KeySig* nk = new KeySig(gscore);
      nk->setKeySigEvent(nke);
      sp->append(nk, keyNames[15]);

      return sp;
      }

//---------------------------------------------------------
//   newAccidentalsPalettePanel
//---------------------------------------------------------

PalettePanel* MuseScore::newAccidentalsPalettePanel(bool defaultPalettePanel)
      {
      PalettePanel* sp = new PalettePanel(PalettePanel::Type::Accidental);
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Accidentals"));
      sp->setGrid(33, 36);
      sp->setDrawGrid(true);

      int end = 0;
      if (defaultPalettePanel)
            end = int(AccidentalType::SHARP_SHARP);
      else
            end = int(AccidentalType::END);

      Accidental* a = new Accidental(gscore);
      a->setAccidentalType(AccidentalType::NONE);
      sp->append(a, a->subtypeUserName());
      for (int i = int(AccidentalType::FLAT); i < end; ++i) {
            Accidental* ac = new Accidental(gscore);
            ac->setAccidentalType(AccidentalType(i));
            if (ac->symbol() != SymId::noSym)
                  sp->append(ac, ac->subtypeUserName());
            else
                  delete ac;
            }

      if (defaultPalettePanel) {
            sp->setMoreElements(true);
            }

      Icon* ik = new Icon(gscore);
      ik->setIconType(IconType::BRACKETS);
      const Shortcut* s = Shortcut::getShortcut("add-brackets");
      QAction* action = s->action();
      ik->setAction(QByteArray("add-brackets"), action->icon());
      sp->append(ik, s->help());

      ik = new Icon(gscore);
      ik->setIconType(IconType::PARENTHESES);
      s = Shortcut::getShortcut("add-parentheses");
      action = s->action();
      ik->setAction(QByteArray("add-parentheses"), action->icon());
      sp->append(ik, s->help());

      ik = new Icon(gscore);
      ik->setIconType(IconType::BRACES);
      s = Shortcut::getShortcut("add-braces");
      action = s->action();
      ik->setAction(QByteArray("add-braces"), action->icon());
      sp->append(ik, s->help());
      return sp;
      }

//---------------------------------------------------------
//   newBarLinePalettePanel
//---------------------------------------------------------

PalettePanel* MuseScore::newBarLinePalettePanel()
      {
      PalettePanel* sp = new PalettePanel(PalettePanel::Type::BarLine);
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Barlines"));
      sp->setMag(0.8);
      sp->setGrid(42, 38);

      // bar line styles
      for (unsigned i = 0;; ++i) {
            const BarLineTableItem* bti = BarLine::barLineTableItem(i);
            if (!bti)
                  break;
            BarLine* b = new BarLine(gscore);
            b->setBarLineType(bti->type);
            sp->append(b, BarLine::userTypeName(bti->type));
            }

      // bar line spans
      struct {
            int         from, to;
            const char* userName;
            } spans[] = {
            { BARLINE_SPAN_TICK1_FROM,  BARLINE_SPAN_TICK1_TO,  Sym::symUserNames[int(SymId::barlineDashed)]         },
            { BARLINE_SPAN_TICK2_FROM,  BARLINE_SPAN_TICK2_TO,  QT_TRANSLATE_NOOP("symUserNames", "Tick barline 2")  }, // Not in SMuFL
            { BARLINE_SPAN_SHORT1_FROM, BARLINE_SPAN_SHORT1_TO, Sym::symUserNames[int(SymId::barlineShort)]          },
            { BARLINE_SPAN_SHORT2_FROM, BARLINE_SPAN_SHORT2_TO, QT_TRANSLATE_NOOP("symUserNames", "Short barline 2") }, // Not in SMuFL
            };
      for (auto span : spans) {
            BarLine* b = new BarLine(gscore);
            b->setBarLineType(BarLineType::NORMAL);
            b->setSpanFrom(span.from);
            b->setSpanTo(span.to);
            sp->append(b, span.userName);
            }
      return sp;
      }

//---------------------------------------------------------
//   newRepeatsPalettePanel
//---------------------------------------------------------

PalettePanel* MuseScore::newRepeatsPalettePanel()
      {
      PalettePanel* sp = new PalettePanel(PalettePanel::Type::Repeat);
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Repeats & Jumps"));
      sp->setMag(0.65);
      sp->setGrid(75, 28);
      sp->setDrawGrid(true);

      RepeatMeasure* rm = new RepeatMeasure(gscore);
      sp->append(rm, qApp->translate("symUserNames", Sym::symUserNames[int(SymId::repeat1Bar)]));

      for (int i = 0; i < markerTypeTableSize(); i++) {
            if (markerTypeTable[i].type == Marker::Type::CODETTA) // not in SMuFL
                  continue;

            Marker* mk = new Marker(gscore);
            mk->setMarkerType(markerTypeTable[i].type);
            mk->styleChanged();
            sp->append(mk, markerTypeTable[i].name);
            }

      for (int i = 0; i < jumpTypeTableSize(); i++) {
            Jump* jp = new Jump(gscore);
            jp->setJumpType(jumpTypeTable[i].type);
            sp->append(jp, jumpTypeTable[i].userText);
            }

      for (unsigned i = 0;; ++i) {
            const BarLineTableItem* bti = BarLine::barLineTableItem(i);
            if (!bti)
                  break;
            switch (bti->type) {
                  case BarLineType::START_REPEAT:
                  case BarLineType::END_REPEAT:
                  case BarLineType::END_START_REPEAT:
                        break;
                  default:
                        continue;
                  }

            BarLine* b = new BarLine(gscore);
            b->setBarLineType(bti->type);
            PaletteCell* cell = sp->append(b, BarLine::userTypeName(bti->type));
            cell->drawStaff = false;
            }

      return sp;
      }

//---------------------------------------------------------
//   newBreaksPalettePanel
//---------------------------------------------------------

PalettePanel* MuseScore::newBreaksPalettePanel()
      {
      qreal _spatium = gscore->spatium();
      PalettePanel* sp = new PalettePanel(PalettePanel::Type::Break);
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Breaks & Spacers"));
      sp->setMag(1.0);
      sp->setGrid(42, 36);
      sp->setDrawGrid(true);

      struct BreakItem {
            LayoutBreak b;
            };
      LayoutBreak* lb = new LayoutBreak(gscore);
      lb->setLayoutBreakType(LayoutBreak::Type::LINE);
      PaletteCell* cell = sp->append(lb, QT_TRANSLATE_NOOP("Palette", "System break"));
      cell->mag = 1.2;

      lb = new LayoutBreak(gscore);
      lb->setLayoutBreakType(LayoutBreak::Type::PAGE);
      cell = sp->append(lb, QT_TRANSLATE_NOOP("Palette", "Page break"));
      cell->mag = 1.2;

      lb = new LayoutBreak(gscore);
      lb->setLayoutBreakType(LayoutBreak::Type::SECTION);
      cell = sp->append(lb, QT_TRANSLATE_NOOP("Palette", "Section break"));
      cell->mag = 1.2;

#if 0
      lb = new LayoutBreak(gscore);
      lb->setLayoutBreakType(LayoutBreak::Type::NOBREAK);
      cell = sp->append(lb, QT_TRANSLATE_NOOP("Palette", "Don't break"));
      cell->mag = 1.2;
#endif

      Spacer* spacer = new Spacer(gscore);
      spacer->setSpacerType(SpacerType::DOWN);
      spacer->setGap(3 * _spatium);
      cell = sp->append(spacer, QT_TRANSLATE_NOOP("Palette", "Staff spacer down"));
      cell->mag = .7;

      spacer = new Spacer(gscore);
      spacer->setSpacerType(SpacerType::UP);
      spacer->setGap(3 * _spatium);
      cell = sp->append(spacer, QT_TRANSLATE_NOOP("Palette", "Staff spacer up"));
      cell->mag = .7;

      spacer = new Spacer(gscore);
      spacer->setSpacerType(SpacerType::FIXED);
      spacer->setGap(3 * _spatium);
      cell = sp->append(spacer, QT_TRANSLATE_NOOP("Palette", "Staff spacer fixed down"));
      cell->mag = .7;

      return sp;
      }

//---------------------------------------------------------
//   newFingeringPalettePanel
//---------------------------------------------------------

PalettePanel* MuseScore::newFingeringPalettePanel()
      {
      PalettePanel* sp = new PalettePanel(PalettePanel::Type::Fingering);
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Fingering"));
      sp->setMag(1.5);
      sp->setGrid(28, 30);
      sp->setDrawGrid(true);

      const char* finger = "012345";
      for (unsigned i = 0; i < strlen(finger); ++i) {
            Fingering* f = new Fingering(gscore);
            f->setXmlText(QString(finger[i]));
            sp->append(f, QT_TRANSLATE_NOOP("Palette", "Fingering %1"));
            }
      finger = "pimac";
      for (unsigned i = 0; i < strlen(finger); ++i) {
            Fingering* f = new Fingering(gscore, Tid::RH_GUITAR_FINGERING);
            f->setXmlText(QString(finger[i]));
            sp->append(f, QT_TRANSLATE_NOOP("Palette", "RH Guitar Fingering %1"));
            }
      finger = "012345T";
      for (unsigned i = 0; i < strlen(finger); ++i) {
            Fingering* f = new Fingering(gscore, Tid::LH_GUITAR_FINGERING);
            f->setXmlText(QString(finger[i]));
            sp->append(f, QT_TRANSLATE_NOOP("Palette", "LH Guitar Fingering %1"));
            }
      finger = "0123456";
      for (unsigned i = 0; i < strlen(finger); ++i) {
            Fingering* f = new Fingering(gscore, Tid::STRING_NUMBER);
            f->setXmlText(QString(finger[i]));
            sp->append(f, QT_TRANSLATE_NOOP("Palette", "String number %1"));
            }

      static const std::vector<SymId> lute {
            SymId::stringsThumbPosition,
            SymId::luteFingeringRHThumb, SymId::luteFingeringRHFirst,
            SymId::luteFingeringRHSecond, SymId::luteFingeringRHThird
            };
      // include additional symbol-based fingerings (temporarily?) implemented as articulations
      for (auto i : lute) {
            Articulation* s = new Articulation(i, gscore);
            sp->append(s, s->userName());
            }
      return sp;
      }

//---------------------------------------------------------
//   newTremoloPalettePanel
//---------------------------------------------------------

PalettePanel* MuseScore::newTremoloPalettePanel()
      {
      PalettePanel* sp = new PalettePanel(PalettePanel::Type::Tremolo);
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Tremolos"));
      sp->setGrid(27, 40);
      sp->setDrawGrid(true);

      for (int i = int(TremoloType::R8); i <= int(TremoloType::C64); ++i) {
            Tremolo* tremolo = new Tremolo(gscore);
            tremolo->setTremoloType(TremoloType(i));
            sp->append(tremolo, tremolo->subtypeName());
            }

      static const std::vector<SymId> dots {
            SymId::tremoloDivisiDots2,
            SymId::tremoloDivisiDots3,
            SymId::tremoloDivisiDots4,
            SymId::tremoloDivisiDots6
            };
      // include additional symbol-based tremolo articulations implemented as articulations
      for (auto i : dots) {
            Articulation* s = new Articulation(i, gscore);
            sp->append(s, s->userName());
            }
      return sp;
      }

//---------------------------------------------------------
//   newNoteHeadsPalettePanel
//---------------------------------------------------------

PalettePanel* MuseScore::newNoteHeadsPalettePanel()
      {
      PalettePanel* sp = new PalettePanel(PalettePanel::Type::NoteHead);
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Noteheads"));
      sp->setMag(1.3);
      sp->setGrid(33, 36);
      sp->setDrawGrid(true);

      for (int i = 0; i < int(NoteHead::Group::HEAD_DO_WALKER); ++i) {
            SymId sym = Note::noteHead(0, NoteHead::Group(i), NoteHead::Type::HEAD_HALF);
            // HEAD_BREVIS_ALT shows up only for brevis value
            if (i == int(NoteHead::Group::HEAD_BREVIS_ALT))
                  sym = Note::noteHead(0, NoteHead::Group(i), NoteHead::Type::HEAD_BREVIS);
            NoteHead* nh = new NoteHead(gscore);
            nh->setSym(sym);
            sp->append(nh, NoteHead::group2userName(NoteHead::Group(i)));
            }
      Icon* ik = new Icon(gscore);
      ik->setIconType(IconType::PARENTHESES);
      const Shortcut* s = Shortcut::getShortcut("add-parentheses");
      QAction* action = s->action();
      QIcon icon(action->icon());
      ik->setAction("add-parentheses", icon);
      sp->append(ik, s->help());
      return sp;
      }

//---------------------------------------------------------
//   newArticulationsPalettePanel
//---------------------------------------------------------

PalettePanel* MuseScore::newArticulationsPalettePanel()
      {
      PalettePanel* sp = new PalettePanel(PalettePanel::Type::Articulation);
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Articulations"));
      sp->setGrid(42, 25);
      sp->setDrawGrid(true);

      static const std::vector<SymId> fermatas {
            SymId::fermataAbove,
            SymId::fermataShortAbove,
            SymId::fermataLongAbove,
            SymId::fermataLongHenzeAbove,
            SymId::fermataShortHenzeAbove,
            SymId::fermataVeryLongAbove,
            SymId::fermataVeryShortAbove,
            };
      for (auto i : fermatas) {
            Fermata* f = new Fermata(i, gscore);
            sp->append(f, f->userName());
            }
      // do not include additional symbol-based fingerings (temporarily?) implemented as articulations
      static const std::vector<SymId> art {
            SymId::articAccentAbove,
            SymId::articStaccatoAbove,
            SymId::articStaccatissimoAbove,
            SymId::articTenutoAbove,
            SymId::articTenutoStaccatoAbove,
            SymId::articMarcatoAbove,
            SymId::articAccentStaccatoAbove,
            SymId::articLaissezVibrerAbove,
            SymId::articMarcatoStaccatoAbove,
            SymId::articMarcatoTenutoAbove,
            SymId::articStaccatissimoStrokeAbove,
            SymId::articStaccatissimoWedgeAbove,
            SymId::articStressAbove,
            SymId::articTenutoAccentAbove,
            SymId::articUnstressAbove,

            SymId::articSoftAccentAbove,                    // supplemental articulations
            SymId::articSoftAccentStaccatoAbove,
            SymId::articSoftAccentTenutoAbove,
            SymId::articSoftAccentTenutoStaccatoAbove,

            SymId::guitarFadeIn,
            SymId::guitarFadeOut,
            SymId::guitarVolumeSwell,
            SymId::wiggleSawtooth,
            SymId::wiggleSawtoothWide,
            SymId::wiggleVibratoLargeFaster,
            SymId::wiggleVibratoLargeSlowest,
            SymId::brassMuteOpen,
            SymId::brassMuteClosed,
            SymId::stringsHarmonic,
            SymId::stringsUpBow,
            SymId::stringsDownBow,
            SymId::pluckedSnapPizzicatoAbove,
            SymId::pictHalfOpen2,
            // SymId::stringsThumbPosition,
            // SymId::luteFingeringRHThumb,
            // SymId::luteFingeringRHFirst,
            // SymId::luteFingeringRHSecond,
            // SymId::luteFingeringRHThird,
            };
      for (auto i : art) {
            Articulation* s = new Articulation(i, gscore);
            sp->append(s, s->userName());
            }
      Bend* bend = new Bend(gscore);
      bend->points().append(PitchValue(0,    0, false));
      bend->points().append(PitchValue(15, 100, false));
      bend->points().append(PitchValue(60, 100, false));
      sp->append(bend, QT_TRANSLATE_NOOP("Palette", "Bend"));

      TremoloBar* tb = new TremoloBar(gscore);
      tb->points().append(PitchValue(0,     0, false));     // "Dip"
      tb->points().append(PitchValue(30, -100, false));
      tb->points().append(PitchValue(60,    0, false));
      sp->append(tb, QT_TRANSLATE_NOOP("Palette", "Tremolo bar"));

      return sp;
      }

//---------------------------------------------------------
//   newOrnamentsPalettePanel
//---------------------------------------------------------

PalettePanel* MuseScore::newOrnamentsPalettePanel()
      {
      PalettePanel* sp = new PalettePanel(PalettePanel::Type::Ornament);
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Ornaments"));
      sp->setGrid(42, 25);
      sp->setDrawGrid(true);

      static const std::vector<SymId> art {
            SymId::ornamentTurnInverted,
            SymId::ornamentTurnSlash,
            SymId::ornamentTurn,
            SymId::ornamentTurnUp,
            SymId::ornamentHaydn,
            SymId::ornamentTurnUpS,
            SymId::ornamentTrill,
            SymId::ornamentShortTrill,
            SymId::ornamentMordent,
            SymId::ornamentTremblement,
            SymId::ornamentPrallMordent,
            SymId::ornamentUpPrall,
            SymId::ornamentPrecompMordentUpperPrefix,       // SymId::ornamentDownPrall,
            SymId::ornamentUpMordent,
            SymId::ornamentDownMordent,
            SymId::ornamentPrallDown,
            SymId::ornamentPrallUp,
            SymId::ornamentLinePrall,
            SymId::ornamentPrecompSlide,
            SymId::ornamentShake3,
            SymId::ornamentShakeMuffat1,
            SymId::ornamentTremblementCouperin,
            SymId::ornamentPinceCouperin,
            };
      for (auto i : art) {
            Articulation* s = new Articulation(i, gscore);
            sp->append(s, s->userName());
            }
      return sp;
      }

//---------------------------------------------------------
//   newAccordionPalettePanel
//---------------------------------------------------------

PalettePanel* MuseScore::newAccordionPalettePanel()
      {
      PalettePanel* sp = new PalettePanel(PalettePanel::Type::Accordion);
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Accordion"));
      sp->setGrid(42, 25);
      sp->setDrawGrid(true);

      static const std::vector<SymId> art {
            SymId::accdnCombDot,
            SymId::accdnCombLH2RanksEmpty,
            SymId::accdnCombLH3RanksEmptySquare,
            SymId::accdnCombRH3RanksEmpty,
            SymId::accdnCombRH4RanksEmpty,
            SymId::accdnDiatonicClef,
            SymId::accdnLH2Ranks16Round,
            SymId::accdnLH2Ranks8Plus16Round,
            SymId::accdnLH2Ranks8Round,
            SymId::accdnLH2RanksFullMasterRound,

            SymId::accdnLH2RanksMasterPlus16Round,
            SymId::accdnLH2RanksMasterRound,
            SymId::accdnLH3Ranks2Plus8Square,
            SymId::accdnLH3Ranks2Square,
            SymId::accdnLH3Ranks8Square,
            SymId::accdnLH3RanksDouble8Square,
            SymId::accdnLH3RanksTuttiSquare,
            SymId::accdnPull,
            SymId::accdnPush,
            SymId::accdnRH3RanksAccordion,

            SymId::accdnRH3RanksAuthenticMusette,
            SymId::accdnRH3RanksBandoneon,
            SymId::accdnRH3RanksBassoon,
            SymId::accdnRH3RanksClarinet,
            SymId::accdnRH3RanksDoubleTremoloLower8ve,
            SymId::accdnRH3RanksDoubleTremoloUpper8ve,
            SymId::accdnRH3RanksFullFactory,
            SymId::accdnRH3RanksHarmonium,
            SymId::accdnRH3RanksImitationMusette,
            SymId::accdnRH3RanksLowerTremolo8,

            SymId::accdnRH3RanksMaster,
            SymId::accdnRH3RanksOboe,
            SymId::accdnRH3RanksOrgan,
            SymId::accdnRH3RanksPiccolo,
            SymId::accdnRH3RanksTremoloLower8ve,
            SymId::accdnRH3RanksTremoloUpper8ve,
            SymId::accdnRH3RanksTwoChoirs,
            SymId::accdnRH3RanksUpperTremolo8,
            SymId::accdnRH3RanksViolin,
            SymId::accdnRH4RanksAlto,

            SymId::accdnRH4RanksBassAlto,
            SymId::accdnRH4RanksMaster,
            SymId::accdnRH4RanksSoftBass,
            SymId::accdnRH4RanksSoftTenor,
            SymId::accdnRH4RanksSoprano,
            SymId::accdnRH4RanksTenor,
            SymId::accdnRicochet2,
            SymId::accdnRicochet3,
            SymId::accdnRicochet4,
            SymId::accdnRicochet5,

            SymId::accdnRicochet6,
            SymId::accdnRicochetStem2,
            SymId::accdnRicochetStem3,
            SymId::accdnRicochetStem4,
            SymId::accdnRicochetStem5,
            SymId::accdnRicochetStem6
            };
      for (auto i : art) {
            Symbol* s = new Symbol(gscore);
            s->setSym(i);
            sp->append(s, Sym::id2userName(i));
            }
      return sp;
      }

//---------------------------------------------------------
//   newBracketsPalettePanel
//---------------------------------------------------------

PalettePanel* MuseScore::newBracketsPalettePanel()
      {
      PalettePanel* sp = new PalettePanel(PalettePanel::Type::Bracket);
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Brackets"));
      sp->setMag(0.7);
      sp->setGrid(40, 60);
      sp->setDrawGrid(true);

      for (auto t : std::array<std::pair<BracketType,const char*>, 4> {
         {{ BracketType::NORMAL, QT_TRANSLATE_NOOP("Palette", "Bracket")          },
          { BracketType::BRACE,  QT_TRANSLATE_NOOP("Palette", "Brace")            },
          { BracketType::SQUARE, QT_TRANSLATE_NOOP("Palette", "Square")           },
          { BracketType::LINE,   QT_TRANSLATE_NOOP("Palette", "Line")    }}
         } ) {
            Bracket* b1      = new Bracket(gscore);
            BracketItem* bi1 = new BracketItem(gscore);
            bi1->setBracketType(t.first);
            b1->setBracketItem(bi1);
            sp->append(b1, t.second);      // Bracket, Brace, Square, Line
            }
      return sp;
      }

//---------------------------------------------------------
//   newBreathPalettePanel
//---------------------------------------------------------

PalettePanel* MuseScore::newBreathPalettePanel()
      {
      PalettePanel* sp = new PalettePanel(PalettePanel::Type::Breath);
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Breaths & Pauses"));
      sp->setGrid(42, 40);
      sp->setDrawGrid(true);
      sp->setDrawGrid(true);

      for (BreathType bt : Breath::breathList) {
            Breath* a = new Breath(gscore);
            a->setSymId(bt.id);
            a->setPause(bt.pause);
            sp->append(a, Sym::id2userName(bt.id));
            }
      return sp;
      }

//---------------------------------------------------------
//   newArpeggioPalettePanel
//---------------------------------------------------------

PalettePanel* MuseScore::newArpeggioPalettePanel()
      {
      PalettePanel* sp = new PalettePanel(PalettePanel::Type::Arpeggio);
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Arpeggios & Glissandi"));
      sp->setGrid(27, 50);
      sp->setDrawGrid(true);

      for (int i = 0; i < 6; ++i) {
            Arpeggio* a = new Arpeggio(gscore);
            a->setArpeggioType(ArpeggioType(i));
            sp->append(a, a->arpeggioTypeName());
            }
      for (int i = 0; i < 2; ++i) {
            Glissando* a = new Glissando(gscore);
            a->setGlissandoType(GlissandoType(i));
            sp->append(a, a->glissandoTypeName());
            }

      //fall and doits

      ChordLine* cl = new ChordLine(gscore);
      cl->setChordLineType(ChordLineType::FALL);
      sp->append(cl, scorelineNames[0]);

      cl = new ChordLine(gscore);
      cl->setChordLineType(ChordLineType::DOIT);
      sp->append(cl, scorelineNames[1]);

      cl = new ChordLine(gscore);
      cl->setChordLineType(ChordLineType::PLOP);
      sp->append(cl, scorelineNames[2]);

      cl = new ChordLine(gscore);
      cl->setChordLineType(ChordLineType::SCOOP);
      sp->append(cl, scorelineNames[3]);

      cl = new ChordLine(gscore);
      cl->setChordLineType(ChordLineType::FALL);
      cl->setStraight(true);
      sp->append(cl, QT_TRANSLATE_NOOP("Ms", "Slide out down"));

      cl = new ChordLine(gscore);
      cl->setChordLineType(ChordLineType::DOIT);
      cl->setStraight(true);
      sp->append(cl, QT_TRANSLATE_NOOP("Ms", "Slide out up"));

      cl = new ChordLine(gscore);
      cl->setChordLineType(ChordLineType::PLOP);
      cl->setStraight(true);
      sp->append(cl, QT_TRANSLATE_NOOP("Ms", "Slide in above"));

      cl = new ChordLine(gscore);
      cl->setChordLineType(ChordLineType::SCOOP);
      cl->setStraight(true);
      sp->append(cl, QT_TRANSLATE_NOOP("Ms", "Slide in below"));

      return sp;
      }

//---------------------------------------------------------
//   newClefsPalettePanel
//---------------------------------------------------------

PalettePanel* MuseScore::newClefsPalettePanel(bool defaultPalettePanel)
      {
      PalettePanel* sp = new PalettePanel(PalettePanel::Type::Clef);
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Clefs"));
      sp->setMag(0.8);
      sp->setGrid(35, 50);
      sp->setYOffset(1.0);

      static std::vector<ClefType> clefsDefault  {
            ClefType::G, ClefType::G8_VA, ClefType::G15_MA, ClefType::G8_VB, ClefType::G15_MB, ClefType::G8_VB_O,
            ClefType::G8_VB_P, ClefType::G_1, ClefType::C1, ClefType::C2, ClefType::C3,
            ClefType::C4, ClefType::C5, ClefType::F, ClefType::F_8VA, ClefType::F_15MA,
            ClefType::F8_VB, ClefType::F15_MB, ClefType::F_B, ClefType::F_C, ClefType::PERC,
            ClefType::PERC2, ClefType::TAB, ClefType::TAB4
            };
      static std::vector<ClefType> clefsMaster  {
            ClefType::G, ClefType::G8_VA, ClefType::G15_MA, ClefType::G8_VB, ClefType::G15_MB, ClefType::G8_VB_O,
            ClefType::G8_VB_C, ClefType::G8_VB_P, ClefType::G_1, ClefType::C1, ClefType::C2, ClefType::C3,
            ClefType::C4, ClefType::C4_8VB, ClefType::C5, ClefType::C_19C, ClefType::C1_F18C, ClefType::C3_F18C, ClefType::C4_F18C, ClefType::C1_F20C, ClefType::C3_F20C, ClefType::C4_F20C,
            ClefType::F, ClefType::F_8VA, ClefType::F_15MA,
            ClefType::F8_VB, ClefType::F15_MB, ClefType::F_B, ClefType::F_C, ClefType::F_F18C, ClefType::F_19C, ClefType::PERC,
            ClefType::PERC2, ClefType::TAB, ClefType::TAB4, ClefType::TAB_SERIF, ClefType::TAB4_SERIF
            };

      std::vector<ClefType>* items = nullptr;
      if (defaultPalettePanel) {
            items = &clefsDefault;
            sp->setMoreElements(true);
            }
      else {
            items = &clefsMaster;
            sp->setMoreElements(false);
            }

      for (ClefType j : *items) {
            Clef* k = new Ms::Clef(gscore);
            k->setClefType(ClefTypeList(j, j));
            sp->append(k, ClefInfo::name(j));
            }
      return sp;
      }

//---------------------------------------------------------
//   newGraceNotePalettePanel
//---------------------------------------------------------

PalettePanel* MuseScore::newGraceNotePalettePanel()
      {
      PalettePanel* sp = new PalettePanel(PalettePanel::Type::GraceNote);
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Grace Notes"));
      sp->setGrid(32, 40);
      sp->setDrawGrid(true);
      static const IconAction gna[] = {
            { IconType::ACCIACCATURA,  "acciaccatura" },
            { IconType::APPOGGIATURA,  "appoggiatura" },
            { IconType::GRACE4,        "grace4" },
            { IconType::GRACE16,       "grace16" },
            { IconType::GRACE32,       "grace32" },
            { IconType::GRACE8_AFTER,  "grace8after" },
            { IconType::GRACE16_AFTER, "grace16after" },
            { IconType::GRACE32_AFTER, "grace32after" },
            { IconType::NONE,          "" }
            };
      populateIconPalettePanel(sp, gna);
      return sp;
      }

//---------------------------------------------------------
//   newBagpipeEmbellishmentPalettePanel
//---------------------------------------------------------

PalettePanel* MuseScore::newBagpipeEmbellishmentPalettePanel()
      {
      PalettePanel* sp = new PalettePanel(PalettePanel::Type::BagpipeEmbellishment);
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Bagpipe Embellishments"));
      sp->setMag(0.8);
      sp->setYOffset(2.0);
      sp->setGrid(55, 55);
      for (int i = 0; i < BagpipeEmbellishment::nEmbellishments(); ++i) {
            BagpipeEmbellishment* b  = new BagpipeEmbellishment(gscore);
            b->setEmbelType(i);
            sp->append(b, BagpipeEmbellishment::BagpipeEmbellishmentList[i].name);
            }

      return sp;
      }

//---------------------------------------------------------
//   newLinesPalettePanel
//---------------------------------------------------------

PalettePanel* MuseScore::newLinesPalettePanel()
      {
      PalettePanel* sp = new PalettePanel(PalettePanel::Type::Line);
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Lines"));
      sp->setMag(.8);
      sp->setGrid(75, 28);
      sp->setDrawGrid(true);

      qreal w = gscore->spatium() * 8;

      Slur* slur = new Slur(gscore);
      sp->append(slur, QT_TRANSLATE_NOOP("Palette", "Slur"));

      Hairpin* gabel0 = new Hairpin(gscore);
      gabel0->setHairpinType(HairpinType::CRESC_HAIRPIN);
      gabel0->setLen(w);
      sp->append(gabel0, QT_TRANSLATE_NOOP("Palette", "Crescendo hairpin"));

      Hairpin* gabel1 = new Hairpin(gscore);
      gabel1->setHairpinType(HairpinType::DECRESC_HAIRPIN);
      gabel1->setLen(w);
      sp->append(gabel1, QT_TRANSLATE_NOOP("Palette", "Diminuendo hairpin"));

      Hairpin* gabel2 = new Hairpin(gscore);
      gabel2->setHairpinType(HairpinType::CRESC_LINE);
      gabel2->setLen(w);
      sp->append(gabel2, QT_TRANSLATE_NOOP("Palette", "Crescendo line"));

      Hairpin* gabel3 = new Hairpin(gscore);
      gabel3->setHairpinType(HairpinType::DECRESC_LINE);
      gabel3->setLen(w);
      sp->append(gabel3, QT_TRANSLATE_NOOP("Palette", "Diminuendo line"));

      Hairpin* gabel4 = new Hairpin(gscore);
      gabel4->setHairpinType(HairpinType::CRESC_HAIRPIN);
      gabel4->setBeginText("<sym>dynamicMezzo</sym><sym>dynamicForte</sym>");
      gabel4->setPropertyFlags(Pid::BEGIN_TEXT, PropertyFlags::UNSTYLED);
      gabel4->setBeginTextAlign(Align::VCENTER);
      gabel4->setPropertyFlags(Pid::BEGIN_TEXT_ALIGN, PropertyFlags::UNSTYLED);
      gabel4->setLen(w);
      sp->append(gabel4, QT_TRANSLATE_NOOP("Palette", "Dynamic + hairpin"));

      Volta* volta = new Volta(gscore);
      volta->setVoltaType(Volta::Type::CLOSED);
      volta->setLen(w);
      volta->setText("1.");
      QList<int> il;
      il.append(1);
      volta->setEndings(il);
      sp->append(volta, QT_TRANSLATE_NOOP("Palette", "Prima volta"));

      volta = new Volta(gscore);
      volta->setVoltaType(Volta::Type::CLOSED);
      volta->setLen(w);
      volta->setText("2.");
      il.clear();
      il.append(2);
      volta->setEndings(il);
      sp->append(volta, QT_TRANSLATE_NOOP("Palette", "Seconda volta"));

      volta = new Volta(gscore);
      volta->setVoltaType(Volta::Type::CLOSED);
      volta->setLen(w);
      volta->setText("3.");
      il.clear();
      il.append(3);
      volta->setEndings(il);
      sp->append(volta, QT_TRANSLATE_NOOP("Palette", "Terza volta"));

      volta = new Volta(gscore);
      volta->setVoltaType(Volta::Type::OPEN);
      volta->setLen(w);
      volta->setText("2.");
      il.clear();
      il.append(2);
      volta->setEndings(il);
      sp->append(volta, QT_TRANSLATE_NOOP("Palette", "Seconda volta, open"));

      Ottava* ottava = new Ottava(gscore);
      ottava->setOttavaType(OttavaType::OTTAVA_8VA);
      ottava->setLen(w);
      ottava->styleChanged();
      sp->append(ottava, QT_TRANSLATE_NOOP("Palette", "8va alta"));

      ottava = new Ottava(gscore);
      ottava->setOttavaType(OttavaType::OTTAVA_8VB);
      ottava->setLen(w);
      ottava->setPlacement(Placement::BELOW);
      ottava->styleChanged();
      sp->append(ottava, QT_TRANSLATE_NOOP("Palette", "8va bassa"));

      ottava = new Ottava(gscore);
      ottava->setOttavaType(OttavaType::OTTAVA_15MA);
      ottava->setLen(w);
      ottava->styleChanged();
      sp->append(ottava, QT_TRANSLATE_NOOP("Palette", "15ma alta"));

      ottava = new Ottava(gscore);
      ottava->setOttavaType(OttavaType::OTTAVA_15MB);
      ottava->setLen(w);
      ottava->setPlacement(Placement::BELOW);
      ottava->styleChanged();
      sp->append(ottava, QT_TRANSLATE_NOOP("Palette", "15ma bassa"));

      ottava = new Ottava(gscore);
      ottava->setOttavaType(OttavaType::OTTAVA_22MA);
      ottava->setLen(w);
      ottava->styleChanged();
      sp->append(ottava, QT_TRANSLATE_NOOP("Palette", "22ma alta"));

      ottava = new Ottava(gscore);
      ottava->setOttavaType(OttavaType::OTTAVA_22MB);
      ottava->setLen(w);
      ottava->setPlacement(Placement::BELOW);
      ottava->styleChanged();
      sp->append(ottava, QT_TRANSLATE_NOOP("Palette", "22ma bassa"));

      Pedal* pedal;
      pedal = new Pedal(gscore);
      pedal->setLen(w);
      pedal->setBeginText("<sym>keyboardPedalPed</sym>");
      pedal->setContinueText("(<sym>keyboardPedalPed</sym>)");
      pedal->setEndHookType(HookType::HOOK_90);
      sp->append(pedal, QT_TRANSLATE_NOOP("Palette", "Pedal (with ped and line)"));

      pedal = new Pedal(gscore);
      pedal->setLen(w);
      pedal->setBeginText("<sym>keyboardPedalPed</sym>");
      pedal->setContinueText("(<sym>keyboardPedalPed</sym>)");
      pedal->setEndText("<sym>keyboardPedalUp</sym>");
      pedal->setLineVisible(false);
      sp->append(pedal, QT_TRANSLATE_NOOP("Palette", "Pedal (with ped and asterisk)"));

      pedal = new Pedal(gscore);
      pedal->setLen(w);
      pedal->setBeginHookType(HookType::HOOK_90);
      pedal->setEndHookType(HookType::HOOK_90);
      sp->append(pedal, QT_TRANSLATE_NOOP("Palette", "Pedal (straight hooks)"));

      pedal = new Pedal(gscore);
      pedal->setLen(w);
      pedal->setBeginHookType(HookType::HOOK_90);
      pedal->setEndHookType(HookType::HOOK_45);
      sp->append(pedal, QT_TRANSLATE_NOOP("Palette", "Pedal (angled end hook)"));

      pedal = new Pedal(gscore);
      pedal->setLen(w);
      pedal->setBeginHookType(HookType::HOOK_45);
      pedal->setEndHookType(HookType::HOOK_45);
      sp->append(pedal, QT_TRANSLATE_NOOP("Palette", "Pedal (both hooks angled)"));

      pedal = new Pedal(gscore);
      pedal->setLen(w);
      pedal->setBeginHookType(HookType::HOOK_45);
      pedal->setEndHookType(HookType::HOOK_90);
      sp->append(pedal, QT_TRANSLATE_NOOP("Palette", "Pedal (angled start hook)"));

      for (int i = 0; i < trillTableSize(); i++) {
            Trill* trill = new Trill(gscore);
            trill->setTrillType(trillTable[i].type);
            trill->setLen(w);
            sp->append(trill, trillTable[i].userName);
            }

      TextLine* textLine = new TextLine(gscore);
      textLine->setLen(w);
      textLine->setBeginText("Staff");
      textLine->setEndHookType(HookType::HOOK_90);
      sp->append(textLine, QT_TRANSLATE_NOOP("Palette", "Staff Text line"));

      TextLine* systemTextLine = new TextLine(gscore, true);
      systemTextLine->setLen(w);
      systemTextLine->setBeginText("System");
      systemTextLine->setEndHookType(HookType::HOOK_90);
      sp->append(systemTextLine, QT_TRANSLATE_NOOP("Palette", "System Text line"));

      TextLine* line = new TextLine(gscore);
      line->setLen(w);
      line->setDiagonal(true);
      sp->append(line, QT_TRANSLATE_NOOP("Palette", "Line"));

      Ambitus* a = new Ambitus(gscore);
      sp->append(a, QT_TRANSLATE_NOOP("Palette", "Ambitus"));

      LetRing* letRing = new LetRing(gscore);
      letRing->setLen(w);
      sp->append(letRing, QT_TRANSLATE_NOOP("Palette", "Let Ring"));

      for (int i = 0; i < vibratoTableSize(); i++) {
            Vibrato* vibrato = new Vibrato(gscore);
            vibrato->setVibratoType(vibratoTable[i].type);
            vibrato->setLen(w);
            sp->append(vibrato, vibratoTable[i].userName);
            }

      PalmMute* pm = new PalmMute(gscore);
      pm->setLen(w);
      sp->append(pm, QT_TRANSLATE_NOOP("Palette", "Palm Mute"));

      return sp;
      }

//---------------------------------------------------------
//   newTempoPalettePanel
//---------------------------------------------------------

PalettePanel* MuseScore::newTempoPalettePanel(bool defaultPalettePanel)
      {
      PalettePanel* sp = new PalettePanel(PalettePanel::Type::Tempo);
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Tempo"));
      sp->setMag(0.65);
      if (defaultPalettePanel)
            sp->setGrid(66, 28);
      else
            sp->setGrid(116, 28);
      sp->setDrawGrid(true);

      static const TempoPattern tps[] = {
            TempoPattern("<sym>metNoteHalfUp</sym> = 80",    QT_TRANSLATE_NOOP("Palette", "Half note = 80 BPM"),    80.0/ 30.0, false, false, true, true, false),                // 1/2
            TempoPattern("<sym>metNoteQuarterUp</sym> = 80", QT_TRANSLATE_NOOP("Palette", "Quarter note = 80 BPM"), 80.0/ 60.0, false, false, true, true, false),                // 1/4
            TempoPattern("<sym>metNote8thUp</sym> = 80",     QT_TRANSLATE_NOOP("Palette", "Eighth note = 80 BPM"),  80.0/120.0, false, false, true, true, false),                // 1/8
            TempoPattern("<sym>metNoteHalfUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80",    QT_TRANSLATE_NOOP("Palette", "Dotted half note = 80 BPM"),    120/ 30.0, false, false, true, false, false),   // dotted 1/2
            TempoPattern("<sym>metNoteQuarterUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80", QT_TRANSLATE_NOOP("Palette", "Dotted quarter note = 80 BPM"), 120/ 60.0, false, false, true, true,  false),   // dotted 1/4
            TempoPattern("<sym>metNote8thUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80",     QT_TRANSLATE_NOOP("Palette", "Dotted eighth note = 80 BPM"),  120/120.0, false, false, true, false, false),   // dotted 1/8

            TempoPattern("Grave",            "Grave",             35.0/60.0, false, true, false, false, false),
            TempoPattern("Largo",            "Largo",             50.0/60.0, false, true, false, false, false),
            TempoPattern("Lento",            "Lento",             52.5/60.0, false, true, false, false, false),
            TempoPattern("Larghetto",        "Larghetto",         63.0/60.0, false, true, false, false, true),
            TempoPattern("Adagio",           "Adagio",            71.0/60.0, false, true, false, false, false),
            TempoPattern("Andante",          "Andante",           92.0/60.0, false, true, false, false, false),
            TempoPattern("Andantino",        "Andantino",         94.0/60.0, false, true, false, false, true),
            TempoPattern("Moderato",         "Moderato",         114.0/60.0, false, true, false, false, false),
            TempoPattern("Allegretto",       "Allegretto",       116.0/60.0, false, true, false, false, false),
            TempoPattern("Allegro moderato", "Allegro moderato", 118.0/60.0, false, true, false, false, true),
            TempoPattern("Allegro",          "Allegro",          144.0/60.0, false, true, false, false, false),
            TempoPattern("Vivace",           "Vivace",           172.0/60.0, false, true, false, false, false),
            TempoPattern("Presto",           "Presto",           187.0/60.0, false, true, false, false, false),
            TempoPattern("Prestissimo",      "Prestissimo",      200.0/60.0, false, true, false, false, true),

            TempoPattern("<sym>metNoteQuarterUp</sym> = <sym>metNoteQuarterUp</sym><sym>space</sym><sym>metAugmentationDot</sym>", QT_TRANSLATE_NOOP("Palette", "Quarter note = dotted quarter note metric modulation"), 3.0/2.0, true, false, true, false, false),
            TempoPattern("<sym>metNoteQuarterUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = <sym>metNoteQuarterUp</sym>", QT_TRANSLATE_NOOP("Palette", "Dotted quarter note = quarter note metric modulation"), 2.0/3.0, true, false, true, false, false),
            TempoPattern("<sym>metNoteHalfUp</sym> = <sym>metNoteQuarterUp</sym>",    QT_TRANSLATE_NOOP("Palette", "Half note = quarter note metric modulation"),    1.0/2.0, true, false, true, false, false),
            TempoPattern("<sym>metNoteQuarterUp</sym> = <sym>metNoteHalfUp</sym>",    QT_TRANSLATE_NOOP("Palette", "Quarter note = half note metric modulation"),    2.0/1.0, true, false, true, false, false),
            TempoPattern("<sym>metNote8thUp</sym> = <sym>metNote8thUp</sym>",         QT_TRANSLATE_NOOP("Palette", "Eighth note = eighth note metric modulation"),   1.0/1.0, true, false, true, false, false),
            TempoPattern("<sym>metNoteQuarterUp</sym> = <sym>metNoteQuarterUp</sym>", QT_TRANSLATE_NOOP("Palette", "Quarter note = quarter note metric modulation"), 1.0/1.0, true, false, true, false, false),
            TempoPattern("<sym>metNote8thUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = <sym>metNoteQuarterUp</sym>",     QT_TRANSLATE_NOOP("Palette", "Dotted eighth note = quarter note metric modulation"),  4.0/3.0, true, false, true, false, false),
            };
      for (TempoPattern tp : tps) {
            TempoText* tt = new TempoText(gscore);
            tt->setFollowText(tp.followText);
            tt->setXmlText(tp.pattern);
            if (tp.relative) {
                  tt->setRelative(tp.f);
                  sp->append(tt, qApp->translate("Palette", tp.name), QString(), 1.5);
                  }
            else if (tp.italian) {
                  tt->setTempo(tp.f);
                  sp->append(tt, tp.name, QString(), 1.3);
                  }
            else {
                  tt->setTempo(tp.f);
                  sp->append(tt, qApp->translate("Palette", tp.name), QString(), 1.5);
                  }
            }
      sp->setMoreElements(false);

      return sp;
      }

//---------------------------------------------------------
//   newTextPalettePanel
//---------------------------------------------------------

PalettePanel* MuseScore::newTextPalettePanel(bool defaultPalettePanel)
      {
      PalettePanel* sp = new PalettePanel(PalettePanel::Type::Text);
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Text"));
      sp->setMag(0.85);
      sp->setGrid(84, 28);
      sp->setDrawGrid(true);

      StaffText* st = new StaffText(gscore);
      st->setXmlText(QT_TRANSLATE_NOOP("Palette", "Staff Text"));
      sp->append(st, QT_TRANSLATE_NOOP("Palette", "Staff text"))->setElementTranslated(true);

      st = new StaffText(gscore, Tid::EXPRESSION);
      st->setXmlText(QT_TRANSLATE_NOOP("Palette", "Expression"));
      st->setPlacement(Placement::BELOW);
      st->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
      sp->append(st, QT_TRANSLATE_NOOP("Palette", "Expression text"))->setElementTranslated(true);

      InstrumentChange* is = new InstrumentChange(gscore);
      is->setXmlText(QT_TRANSLATE_NOOP("Palette", "Change Instr."));
      sp->append(is, QT_TRANSLATE_NOOP("Palette", "Instrument change"))->setElementTranslated(true);

      StaffTypeChange* stc = new StaffTypeChange(gscore);
      sp->append(stc, QT_TRANSLATE_NOOP("Palette", "Staff type change"));

      RehearsalMark* rhm = new RehearsalMark(gscore);
      rhm->setXmlText("B1");
      sp->append(rhm, QT_TRANSLATE_NOOP("Palette", "Rehearsal mark"));

      SystemText* stxt = new SystemText(gscore, Tid::TEMPO);
      stxt->setXmlText(QT_TRANSLATE_NOOP("Palette", "Swing"));
      stxt->setSwing(true);
      sp->append(stxt, QT_TRANSLATE_NOOP("Palette", "Swing"))->setElementTranslated(true);

      stxt = new SystemText(gscore, Tid::TEMPO);
      /*: System text to switch from swing rhythm back to straight rhythm */
      stxt->setXmlText(QT_TRANSLATE_NOOP("Palette", "Straight"));
      // need to be true to enable the "Off" option
      stxt->setSwing(true);
      // 0 (swingUnit) turns of swing; swingRatio is set to default
      stxt->setSwingParameters(0, stxt->score()->styleI(Sid::swingRatio));
      /*: System text to switch from swing rhythm back to straight rhythm */
      sp->append(stxt, QT_TRANSLATE_NOOP("Palette", "Straight"))->setElementTranslated(true);

      stxt = new SystemText(gscore);
      stxt->setXmlText(QT_TRANSLATE_NOOP("Palette", "System Text"));
      sp->append(stxt, QT_TRANSLATE_NOOP("Palette", "System text"))->setElementTranslated(true);

      // Measure numbers, unlike other elements (but like most text elements),
      // are not copied directly into the score when drop.
      // Instead, they simply set the corresponding measure's MeasureNumberMode to SHOW
      // Because of that, the element shown in the palettes does not have to have any particular formatting.
      MeasureNumber* meaNum = new MeasureNumber(gscore);
      meaNum->setProperty(Pid::SUB_STYLE, int(Tid::STAFF)); // Make the element bigger in the palettes (using the default measure number style makes it too small)
      meaNum->setXmlText(QT_TRANSLATE_NOOP("Palette", "Measure Number"));
      sp->append(meaNum, QT_TRANSLATE_NOOP("Palette", "Measure Number"))->setElementTranslated(true);

      if (!defaultPalettePanel) {
            StaffText* pz = new StaffText(gscore);
            pz->setXmlText(QT_TRANSLATE_NOOP("Palette", "pizz."));
            pz->setChannelName(0, "pizzicato");
            pz->setChannelName(1, "pizzicato");
            pz->setChannelName(2, "pizzicato");
            pz->setChannelName(3, "pizzicato");
            sp->append(pz, QT_TRANSLATE_NOOP("Palette", "Pizzicato"))->setElementTranslated(true);

            StaffText* ar = new StaffText(gscore);
            ar->setXmlText(QT_TRANSLATE_NOOP("Palette", "arco"));
            ar->setChannelName(0, "arco");
            ar->setChannelName(1, "arco");
            ar->setChannelName(2, "arco");
            ar->setChannelName(3, "arco");
            sp->append(ar, QT_TRANSLATE_NOOP("Palette", "Arco"))->setElementTranslated(true);

            StaffText* tm = new StaffText(gscore, Tid::EXPRESSION);
            tm->setXmlText(QT_TRANSLATE_NOOP("Palette", "tremolo"));
            tm->setChannelName(0, "tremolo");
            tm->setChannelName(1, "tremolo");
            tm->setChannelName(2, "tremolo");
            tm->setChannelName(3, "tremolo");
            sp->append(tm, QT_TRANSLATE_NOOP("Palette", "Tremolo"))->setElementTranslated(true);

            StaffText* mu = new StaffText(gscore);
            /*: For brass and plucked string instruments: staff text that prescribes to use mute while playing, see https://en.wikipedia.org/wiki/Mute_(music) */
            mu->setXmlText(QT_TRANSLATE_NOOP("Palette", "mute"));
            mu->setChannelName(0, "mute");
            mu->setChannelName(1, "mute");
            mu->setChannelName(2, "mute");
            mu->setChannelName(3, "mute");
            /*: For brass and plucked string instruments: staff text that prescribes to use mute while playing, see https://en.wikipedia.org/wiki/Mute_(music) */
            sp->append(mu, QT_TRANSLATE_NOOP("Palette", "Mute"))->setElementTranslated(true);

            StaffText* no = new StaffText(gscore);
            /*: For brass and plucked string instruments: staff text that prescribes to play without mute, see https://en.wikipedia.org/wiki/Mute_(music) */
            no->setXmlText(QT_TRANSLATE_NOOP("Palette", "open"));
            no->setChannelName(0, "open");
            no->setChannelName(1, "open");
            no->setChannelName(2, "open");
            no->setChannelName(3, "open");
            /*: For brass and plucked string instruments: staff text that prescribes to play without mute, see https://en.wikipedia.org/wiki/Mute_(music) */
            sp->append(no, QT_TRANSLATE_NOOP("Palette", "Open"))->setElementTranslated(true);

            StaffText* sa = new StaffText(gscore);
            sa->setXmlText(QT_TRANSLATE_NOOP("Palette", "S/A"));
            sa->setChannelName(0, "Soprano");
            sa->setChannelName(1, "Alto");
            sa->setChannelName(2, "Soprano");
            sa->setChannelName(3, "Alto");
            sa->setVisible(false);
            sp->append(sa, QT_TRANSLATE_NOOP("Palette", "Soprano/Alto"))->setElementTranslated(true);

            StaffText* tb = new StaffText(gscore);
            tb->setXmlText(QT_TRANSLATE_NOOP("Palette", "T/B"));
            tb->setChannelName(0, "Tenor");
            tb->setChannelName(1, "Bass");
            tb->setChannelName(2, "Tenor");
            tb->setChannelName(3, "Bass");
            tb->setVisible(false);
            sp->append(tb, QT_TRANSLATE_NOOP("Palette", "Tenor/Bass"))->setElementTranslated(true);

            StaffText* tl = new StaffText(gscore);
            tl->setXmlText(QT_TRANSLATE_NOOP("Palette", "T/L"));
            tl->setChannelName(0, "TENOR");
            tl->setChannelName(1, "LEAD");
            tl->setChannelName(2, "TENOR");
            tl->setChannelName(3, "LEAD");
            tl->setVisible(false);
            sp->append(tl, QT_TRANSLATE_NOOP("Palette", "Tenor/Lead"))->setElementTranslated(true);

            StaffText* bb = new StaffText(gscore);
            bb->setXmlText(QT_TRANSLATE_NOOP("Palette", "B/B"));
            bb->setChannelName(0, "BARI");
            bb->setChannelName(1, "BASS");
            bb->setChannelName(2, "BARI");
            bb->setChannelName(3, "BASS");
            bb->setVisible(false);
            sp->append(bb, QT_TRANSLATE_NOOP("Palette", "Bari/Bass"))->setElementTranslated(true);
            }

      return sp;
      }

//---------------------------------------------------------
//   newTimePalettePanel
//    create default time signature palette
//---------------------------------------------------------

PalettePanel* MuseScore::newTimePalettePanel()
      {
      struct TS {
            int numerator;
            int denominator;
            TimeSigType type;
            QString name;
            };

      TS tsList[] = {
            { 2,  4, TimeSigType::NORMAL, "2/4" },
            { 3,  4, TimeSigType::NORMAL, "3/4" },
            { 4,  4, TimeSigType::NORMAL, "4/4" },
            { 5,  4, TimeSigType::NORMAL, "5/4" },
            { 6,  4, TimeSigType::NORMAL, "6/4" },
            { 3,  8, TimeSigType::NORMAL, "3/8" },
            { 6,  8, TimeSigType::NORMAL, "6/8" },
            { 7,  8, TimeSigType::NORMAL, "7/8" },
            { 9,  8, TimeSigType::NORMAL, "9/8" },
            { 12, 8, TimeSigType::NORMAL, "12/8" },
            { 4,  4, TimeSigType::FOUR_FOUR, qApp->translate("symUserNames", "Common time") },
            { 2,  2, TimeSigType::ALLA_BREVE, qApp->translate("symUserNames", "Cut time") },
            { 2,  2, TimeSigType::NORMAL, "2/2" },
            { 3,  2, TimeSigType::NORMAL, "3/2" },
            { 4,  2, TimeSigType::NORMAL, "4/2" },
            { 2,  2, TimeSigType::CUT_BACH, qApp->translate("symUserNames", "Cut time (Bach)") },
            { 9,  8, TimeSigType::CUT_TRIPLE, qApp->translate("symUserNames", "Cut triple time (9/8)") },
            };

      PalettePanel* sp = new PalettePanel(PalettePanel::Type::TimeSig);
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Time Signatures"));
      sp->setMag(.8);
      sp->setGrid(42, 38);

      for (unsigned i = 0; i < sizeof(tsList)/sizeof(*tsList); ++i) {
            TimeSig* ts;
            ts = new TimeSig(gscore);
            ts->setSig(Fraction(tsList[i].numerator, tsList[i].denominator), tsList[i].type);
            sp->append(ts, tsList[i].name);
            }
      return sp;
      }

//-----------------------------------
//    newFretboardDiagramPalettePanel
//-----------------------------------

PalettePanel* MuseScore::newFretboardDiagramPalettePanel()
      {
      PalettePanel* sp = new PalettePanel(PalettePanel::Type::FretboardDiagram);
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Fretboard Diagrams"));
      sp->setGrid(42, 45);
      sp->setDrawGrid(true);

      FretDiagram* fret = FretDiagram::fromString(gscore, "X32O1O");
      fret->setHarmony("C");
      sp->append(fret, "C");
      fret = FretDiagram::fromString(gscore, "X-554-");
      fret->setHarmony("Cm");
      sp->append(fret, "Cm");
      fret = FretDiagram::fromString(gscore, "X3231O");
      fret->setHarmony("C7");
      sp->append(fret, "C7");

      fret = FretDiagram::fromString(gscore, "XXO232");
      fret->setHarmony("D");
      sp->append(fret, "D");
      fret = FretDiagram::fromString(gscore, "XXO231");
      fret->setHarmony("Dm");
      sp->append(fret, "Dm");
      fret = FretDiagram::fromString(gscore, "XXO212");
      fret->setHarmony("D7");
      sp->append(fret, "D7");

      fret = FretDiagram::fromString(gscore, "O221OO");
      fret->setHarmony("E");
      sp->append(fret, "E");
      fret = FretDiagram::fromString(gscore, "O22OOO");
      fret->setHarmony("Em");
      sp->append(fret, "Em");
      fret = FretDiagram::fromString(gscore, "O2O1OO");
      fret->setHarmony("E7");
      sp->append(fret, "E7");

      fret = FretDiagram::fromString(gscore, "-332--");
      fret->setHarmony("F");
      sp->append(fret, "F");
      fret = FretDiagram::fromString(gscore, "-33---");
      fret->setHarmony("Fm");
      sp->append(fret, "Fm");
      fret = FretDiagram::fromString(gscore, "-3-2--");
      fret->setHarmony("F7");
      sp->append(fret, "F7");

      fret = FretDiagram::fromString(gscore, "32OOO3");
      fret->setHarmony("G");
      sp->append(fret, "G");
      fret = FretDiagram::fromString(gscore, "-55---");
      fret->setHarmony("Gm");
      sp->append(fret, "Gm");
      fret = FretDiagram::fromString(gscore, "32OOO1");
      fret->setHarmony("G7");
      sp->append(fret, "G7");

      fret = FretDiagram::fromString(gscore, "XO222O");
      fret->setHarmony("A");
      sp->append(fret, "A");
      fret = FretDiagram::fromString(gscore, "XO221O");
      fret->setHarmony("Am");
      sp->append(fret, "Am");
      fret = FretDiagram::fromString(gscore, "XO2O2O");
      fret->setHarmony("A7");
      sp->append(fret, "A7");

      fret = FretDiagram::fromString(gscore, "X-444-");
      fret->setHarmony("B");
      sp->append(fret, "B");
      fret = FretDiagram::fromString(gscore, "X-443-");
      fret->setHarmony("Bm");
      sp->append(fret, "Bm");
      fret = FretDiagram::fromString(gscore, "X212O2");
      fret->setHarmony("B7");
      sp->append(fret, "B7");

      return sp;
      }
//########END DEBUG: new palettes #########################

//---------------------------------------------------------
//   setDefaultPalette
//---------------------------------------------------------

void MuseScore::setDefaultPalette()
      {
      std::unique_ptr<PaletteTree> defaultPalette(new PaletteTree);

      defaultPalette->append(newClefsPalettePanel(true));
      defaultPalette->append(newKeySigPalettePanel());
      defaultPalette->append(newTimePalettePanel());
      defaultPalette->append(newBracketsPalettePanel());
      defaultPalette->append(newAccidentalsPalettePanel(true));
      defaultPalette->append(newArticulationsPalettePanel());
      defaultPalette->append(newOrnamentsPalettePanel());
      defaultPalette->append(newBreathPalettePanel());
      defaultPalette->append(newGraceNotePalettePanel());
      defaultPalette->append(newNoteHeadsPalettePanel());
      defaultPalette->append(newLinesPalettePanel());
      defaultPalette->append(newBarLinePalettePanel());
      defaultPalette->append(newArpeggioPalettePanel());
      defaultPalette->append(newTremoloPalettePanel());
      defaultPalette->append(newTextPalettePanel(true));
      defaultPalette->append(newTempoPalettePanel(true));
      defaultPalette->append(newDynamicsPalettePanel(true));
      defaultPalette->append(newFingeringPalettePanel());
      defaultPalette->append(newRepeatsPalettePanel());
      defaultPalette->append(newFretboardDiagramPalettePanel());
      defaultPalette->append(newAccordionPalettePanel());
      defaultPalette->append(newBagpipeEmbellishmentPalettePanel());
      defaultPalette->append(newBreaksPalettePanel());
      defaultPalette->append(newFramePalettePanel());
      defaultPalette->append(newBeamPalettePanel());

      mscore->getPaletteWorkspace()->setUserPaletteTree(std::move(defaultPalette));
      }

//---------------------------------------------------------
//   addTempo
//---------------------------------------------------------

void MuseScore::addTempo()
      {
      ChordRest* cr = cs->getSelectedChordRest();
      if (!cr)
            return;
//      double bps = 2.0;

      SigEvent event = cs->sigmap()->timesig(cr->tick());
      Fraction f = event.nominal();
      QString text("<sym>metNoteQuarterUp</sym> = 80");
      switch (f.denominator()) {
            case 1:
                  text = "<sym>metNoteWhole</sym> = 80";
                  break;
            case 2:
                  text = "<sym>metNoteHalfUp</sym> = 80";
                  break;
            case 4:
                  text = "<sym>metNoteQuarterUp</sym> = 80";
                  break;
            case 8:
                  if(f.numerator() % 3 == 0)
                        text = "<sym>metNoteQuarterUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80";
                  else
                        text = "<sym>metNote8thUp</sym> = 80";
                  break;
            case 16:
                  if(f.numerator() % 3 == 0)
                        text = "<sym>metNote8thUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80";
                  else
                        text = "<sym>metNote16thUp</sym> = 80";
                  break;
            case 32:
                  if(f.numerator() % 3 == 0)
                        text = "<sym>metNote16thUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80";
                  else
                        text = "<sym>metNote32ndUp</sym> = 80";
                  break;
            case 64:
                  if(f.numerator() % 3 == 0)
                        text = "<sym>metNote32ndUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80";
                  else
                        text = "<sym>metNote64thUp</sym> = 80";
                  break;
            case 128:
                  if(f.numerator() % 3 == 0)
                        text = "<sym>metNote64ndUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80";
                  else
                        text = "<sym>metNote128thUp</sym> = 80";
                  break;
            default:
                  break;
            }

      TempoText* tt = new TempoText(cs);
      cs->startCmd();
      tt->setParent(cr->segment());
      tt->setTrack(0);
      tt->setXmlText(text);
      tt->setFollowText(true);
      //tt->setTempo(bps);
      cs->undoAddElement(tt);
      cs->select(tt, SelectType::SINGLE, 0);
      cs->endCmd();
      Measure* m = tt->findMeasure();
      if (m && m->hasMMRest() && tt->links()) {
            Measure* mmRest = m->mmRest();
            for (ScoreElement* se : *tt->links()) {
                  TempoText* tt1 = toTempoText(se);
                  if (tt != tt1 && tt1->findMeasure() == mmRest) {
                        tt = tt1;
                        break;
                        }
                  }
            }
      cv->startEditMode(tt);
      }

//---------------------------------------------------------
//   smuflRanges
//    read smufl ranges.json file
//---------------------------------------------------------

QMap<QString, QStringList>* smuflRanges()
      {
      static QMap<QString, QStringList> ranges;
      QStringList allSymbols;

      if (ranges.empty()) {
            QFile fi(":fonts/smufl/ranges.json");
            if (!fi.open(QIODevice::ReadOnly))
                  qDebug("ScoreFont: open ranges file <%s> failed", qPrintable(fi.fileName()));
            QJsonParseError error;
            QJsonObject o = QJsonDocument::fromJson(fi.readAll(), &error).object();
            if (error.error != QJsonParseError::NoError)
                  qDebug("Json parse error in <%s>(offset: %d): %s", qPrintable(fi.fileName()),
                     error.offset, qPrintable(error.errorString()));

            for (auto s : o.keys()) {
                  QJsonObject range = o.value(s).toObject();
                  QString desc      = range.value("description").toString();
                  QJsonArray glyphs = range.value("glyphs").toArray();
                  if (glyphs.size() > 0) {
                        QStringList glyphNames;
                        for (QJsonValue g : glyphs)
                              glyphNames.append(g.toString());
                        ranges.insert(desc, glyphNames);
                        allSymbols << glyphNames;
                        }
                  }
            ranges.insert(SMUFL_ALL_SYMBOLS, allSymbols); // TODO: make translatable as well as ranges.json
            }
      return &ranges;
      }
}

