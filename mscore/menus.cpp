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
#include <tuple>
#include "libmscore/score.h"
#include "palette.h"
#include "palettebox.h"
#include "libmscore/note.h"
#include "libmscore/chordrest.h"
#include "libmscore/dynamic.h"
#include "libmscore/slur.h"
#include "libmscore/sym.h"
#include "libmscore/hairpin.h"
#include "scoreview.h"
#include "musescore.h"
#include "libmscore/select.h"
#include "libmscore/tempo.h"
#include "libmscore/segment.h"
#include "libmscore/undo.h"
#include "icons.h"
#include "libmscore/bracket.h"
#include "libmscore/ottava.h"
#include "libmscore/textline.h"
#include "libmscore/trill.h"
#include "libmscore/pedal.h"
#include "libmscore/clef.h"
#include "libmscore/timesig.h"
#include "libmscore/barline.h"
#include "libmscore/layoutbreak.h"
#include "symboldialog.h"
#include "libmscore/volta.h"
#include "libmscore/keysig.h"
#include "libmscore/breath.h"
#include "libmscore/arpeggio.h"
#include "libmscore/tremolo.h"
#include "libmscore/repeat.h"
#include "libmscore/tempotext.h"
#include "libmscore/glissando.h"
#include "libmscore/articulation.h"
#include "libmscore/chord.h"
#include "libmscore/drumset.h"
#include "libmscore/spacer.h"
#include "libmscore/measure.h"
#include "libmscore/fret.h"
#include "libmscore/staffstate.h"
#include "libmscore/fingering.h"
#include "libmscore/bend.h"
#include "libmscore/tremolobar.h"
#include "libmscore/chordline.h"
#include "libmscore/stafftext.h"
#include "libmscore/systemtext.h"
#include "libmscore/instrchange.h"
#include "workspace.h"
#include "libmscore/icon.h"
#include "libmscore/accidental.h"
#include "libmscore/harmony.h"
#include "libmscore/rehearsalmark.h"
#include "shortcut.h"
#include "libmscore/marker.h"
#include "libmscore/jump.h"
#include "libmscore/bagpembell.h"
#include "libmscore/ambitus.h"
#include "libmscore/stafftypechange.h"
#include "libmscore/letring.h"
#include "libmscore/vibrato.h"
#include "libmscore/palmmute.h"
#include "libmscore/fermata.h"

namespace Ms {

extern bool useFactorySettings;

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
      Palette* sp = new Palette;
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

      populateIconPalette(sp, bpa);
      return sp;
      }

//---------------------------------------------------------
//   newFramePalette
//---------------------------------------------------------

Palette* MuseScore::newFramePalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Frames && Measures"));
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
            populateIconPalette(sp, bpa);
            }
      else {
            static const IconAction bpa[] = {
                  { IconType::VFRAME,   "insert-vbox" },
                  { IconType::HFRAME,   "insert-hbox" },
                  { IconType::TFRAME,   "insert-textframe" },
                  { IconType::MEASURE,  "insert-measure" },
                  { IconType::NONE,     ""}
                };
            populateIconPalette(sp, bpa);
            }

      return sp;
      }

//---------------------------------------------------------
//   newDynamicsPalette
//---------------------------------------------------------

Palette* MuseScore::newDynamicsPalette(bool defaultPalette)
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Dynamics"));
      sp->setMag(.8);
      sp->setDrawGrid(true);

      static const std::vector<const char*> array1 = {
            "pppppp", "ppppp", "pppp",
            "ppp", "pp", "p", "mp", "mf", "f", "ff", "fff",
            "ffff", "fffff", "ffffff",
            "fp", "sf", "sfz", "sff", "sffz", "sfp", "sfpp",
            "rfz", "rf", "fz", "m", "r", "s", "z", "n"
            };
      static const std::vector<const char*> arrayDefault = {
            "ppp", "pp", "p", "mp", "mf", "f", "ff", "fff",
            "fp", "sf", "sfz", "sff", "sffz", "sfp", "sfpp",
            "rfz", "rf", "fz", "m", "r", "s", "z", "n"
            };

      const std::vector<const char*>* array = nullptr;
      if (defaultPalette) {
            array = &arrayDefault;
            sp->setGrid(42, 28);
            sp->setMoreElements(true);
            connect(sp, SIGNAL(displayMore(const QString&)), mscore, SLOT(showMasterPalette(const QString&)));
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
//   newKeySigPalette
//---------------------------------------------------------

Palette* MuseScore::newKeySigPalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Key Signatures"));
      sp->setMag(1.0);
      sp->setGrid(56, 55);
      sp->setYOffset(1.0);

      for (int i = 0; i < 7; ++i) {
            KeySig* k = new KeySig(gscore);
            k->setKey(Key(i + 1));
            sp->append(k, qApp->translate("MuseScore", keyNames[i*2]));
            }
      for (int i = -7; i < 0; ++i) {
            KeySig* k = new KeySig(gscore);
            k->setKey(Key(i));
            sp->append(k, qApp->translate("MuseScore", keyNames[(7 + i) * 2 + 1]));
            }
      KeySig* k = new KeySig(gscore);
      k->setKey(Key::C);
      sp->append(k, qApp->translate("MuseScore", keyNames[14]));

      // atonal key signature
      KeySigEvent nke;
      nke.setKey(Key::C);
      nke.setCustom(true);
      nke.setMode(KeyMode::NONE);
      KeySig* nk = new KeySig(gscore);
      nk->setKeySigEvent(nke);
      sp->append(nk, qApp->translate("MuseScore", keyNames[15]));

      return sp;
      }

//---------------------------------------------------------
//   newAccidentalsPalette
//---------------------------------------------------------

Palette* MuseScore::newAccidentalsPalette(bool defaultPalette)
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Accidentals"));
      sp->setGrid(33, 36);
      sp->setDrawGrid(true);

      int end = 0;
      if (defaultPalette)
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

      if (defaultPalette) {
            sp->setMoreElements(true);
            connect(sp, SIGNAL(displayMore(const QString&)), mscore, SLOT(showMasterPalette(const QString&)));
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

      return sp;
      }

//---------------------------------------------------------
//   newBarLinePalette
//---------------------------------------------------------

Palette* MuseScore::newBarLinePalette()
      {
      Palette* sp = new Palette;
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
            { BARLINE_SPAN_TICK1_FROM, BARLINE_SPAN_TICK1_TO, QT_TRANSLATE_NOOP("Palette", "Tick 1 span") },
            { BARLINE_SPAN_TICK2_FROM, BARLINE_SPAN_TICK2_TO, QT_TRANSLATE_NOOP("Palette", "Tick 2 span") },
            { BARLINE_SPAN_SHORT1_FROM,BARLINE_SPAN_SHORT1_TO,QT_TRANSLATE_NOOP("Palette", "Short 1 span") },
            { BARLINE_SPAN_SHORT2_FROM,BARLINE_SPAN_SHORT2_TO,QT_TRANSLATE_NOOP("Palette", "Short 2 span") },
            };
      for (auto span : spans) {
            BarLine* b = new BarLine(gscore);
            b->setBarLineType(BarLineType::NORMAL);
            b->setSpanFrom(span.from);
            b->setSpanTo(span.to);
            sp->append(b, qApp->translate("Palette", span.userName));
            }
      return sp;
      }

//---------------------------------------------------------
//   newRepeatsPalette
//---------------------------------------------------------

Palette* MuseScore::newRepeatsPalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Repeats && Jumps"));
      sp->setMag(0.65);
      sp->setGrid(75, 28);
      sp->setDrawGrid(true);

      RepeatMeasure* rm = new RepeatMeasure(gscore);
      sp->append(rm, tr("Repeat measure sign"));

      for (int i = 0; i < markerTypeTableSize(); i++) {
            if (markerTypeTable[i].type == Marker::Type::CODETTA) //not in smufl
                  continue;

            Marker* mk = new Marker(gscore);
            mk->setMarkerType(markerTypeTable[i].type);
            sp->append(mk, qApp->translate("markerType", markerTypeTable[i].name.toUtf8().constData()));
            }

      for (int i = 0; i < jumpTypeTableSize(); i++) {
            Jump* jp = new Jump(gscore);
            jp->setJumpType(jumpTypeTable[i].type);
            sp->append(jp, qApp->translate("jumpType", jumpTypeTable[i].userText.toUtf8().constData()));
            }

      for (unsigned i = 0;; ++i) {
            const BarLineTableItem* bti = BarLine::barLineTableItem(i);
            if (!bti)
                  break;
            switch (bti->type) {
                  case BarLineType::START_REPEAT:
                  case BarLineType::END_REPEAT:
                        break;
                  default:
                        continue;
                  }

            BarLine* b = new BarLine(gscore);
            b->setBarLineType(bti->type);
            PaletteCell* cell= sp->append(b, BarLine::userTypeName(bti->type));
            cell->drawStaff = false;
            }

      return sp;
      }

//---------------------------------------------------------
//   newBreaksPalette
//---------------------------------------------------------

Palette* MuseScore::newBreaksPalette()
      {
      qreal _spatium = gscore->spatium();
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Breaks && Spacers"));
      sp->setMag(1.0);
      sp->setGrid(42, 36);
      sp->setDrawGrid(true);

      struct BreakItem {
            LayoutBreak b;
            };
      LayoutBreak* lb = new LayoutBreak(gscore);
      lb->setLayoutBreakType(LayoutBreak::Type::LINE);
      PaletteCell* cell = sp->append(lb, tr("System break"));
      cell->mag = 1.2;

      lb = new LayoutBreak(gscore);
      lb->setLayoutBreakType(LayoutBreak::Type::PAGE);
      cell = sp->append(lb, tr("Page break"));
      cell->mag = 1.2;

      lb = new LayoutBreak(gscore);
      lb->setLayoutBreakType(LayoutBreak::Type::SECTION);
      cell = sp->append(lb, tr("Section break"));
      cell->mag = 1.2;

#if 0
      lb = new LayoutBreak(gscore);
      lb->setLayoutBreakType(LayoutBreak::Type::NOBREAK);
      cell = sp->append(lb, tr("Don't break"));
      cell->mag = 1.2;
#endif

      Spacer* spacer = new Spacer(gscore);
      spacer->setSpacerType(SpacerType::DOWN);
      spacer->setGap(3 * _spatium);
      cell = sp->append(spacer, tr("Staff spacer down"));
      cell->mag = .7;

      spacer = new Spacer(gscore);
      spacer->setSpacerType(SpacerType::UP);
      spacer->setGap(3 * _spatium);
      cell = sp->append(spacer, tr("Staff spacer up"));
      cell->mag = .7;

      spacer = new Spacer(gscore);
      spacer->setSpacerType(SpacerType::FIXED);
      spacer->setGap(3 * _spatium);
      cell = sp->append(spacer, tr("Staff spacer fixed down"));
      cell->mag = .7;

      return sp;
      }

//---------------------------------------------------------
//   newFingeringPalette
//---------------------------------------------------------

Palette* MuseScore::newFingeringPalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Fingering"));
      sp->setMag(1.5);
      sp->setGrid(28, 30);
      sp->setDrawGrid(true);

      const char* finger = "012345";
      for (unsigned i = 0; i < strlen(finger); ++i) {
            Fingering* f = new Fingering(gscore);
            f->setXmlText(QString(finger[i]));
            sp->append(f, tr("Fingering %1").arg(finger[i]));
            }
      finger = "pimac";
      for (unsigned i = 0; i < strlen(finger); ++i) {
            Fingering* f = new Fingering(gscore, Tid::RH_GUITAR_FINGERING);
            f->setXmlText(QString(finger[i]));
            sp->append(f, tr("RH Guitar Fingering %1").arg(finger[i]));
            }
      finger = "012345T";
      for (unsigned i = 0; i < strlen(finger); ++i) {
            Fingering* f = new Fingering(gscore, Tid::LH_GUITAR_FINGERING);
            f->setXmlText(QString(finger[i]));
            sp->append(f, tr("LH Guitar Fingering %1").arg(finger[i]));
            }
      finger = "0123456";
      for (unsigned i = 0; i < strlen(finger); ++i) {
            Fingering* f = new Fingering(gscore, Tid::STRING_NUMBER);
            f->setXmlText(QString(finger[i]));
            sp->append(f, tr("String number %1").arg(finger[i]));
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
//   newTremoloPalette
//---------------------------------------------------------

Palette* MuseScore::newTremoloPalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Tremolo"));
      sp->setGrid(27, 40);
      sp->setDrawGrid(true);

      for (int i = int(TremoloType::R8); i <= int(TremoloType::C64); ++i) {
            Tremolo* tremolo = new Tremolo(gscore);
            tremolo->setTremoloType(TremoloType(i));
            sp->append(tremolo, qApp->translate("Tremolo", tremolo->subtypeName().toUtf8().constData()));
            }
      return sp;
      }

//---------------------------------------------------------
//   newNoteHeadsPalette
//---------------------------------------------------------

Palette* MuseScore::newNoteHeadsPalette()
      {
      Palette* sp = new Palette;
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
//   newArticulationsPalette
//---------------------------------------------------------

Palette* MuseScore::newArticulationsPalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Articulations"));
      sp->setGrid(42, 25);
      sp->setDrawGrid(true);

      // do not include additional symbol-based fingerings (temporarily?) implemented as articulations
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
      sp->append(bend, qApp->translate("articulation", "Bend"));

      TremoloBar* tb = new TremoloBar(gscore);
      tb->points().append(PitchValue(0,     0, false));     // "Dip"
      tb->points().append(PitchValue(30, -100, false));
      tb->points().append(PitchValue(60,    0, false));
      sp->append(tb, qApp->translate("articulation", "Tremolo bar"));

      return sp;
      }

//---------------------------------------------------------
//   newOrnamentsPalette
//---------------------------------------------------------

Palette* MuseScore::newOrnamentsPalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Ornaments"));
      sp->setGrid(42, 25);
      sp->setDrawGrid(true);

      // do not include additional symbol-based fingerings (temporarily?) implemented as articulations
      static const std::vector<SymId> art {
            SymId::ornamentTurnInverted,
            SymId::ornamentTurn,
            SymId::ornamentTrill,
            SymId::ornamentMordent,
            SymId::ornamentMordentInverted,
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
            };
      for (auto i : art) {
            Articulation* s = new Articulation(i, gscore);
            sp->append(s, s->userName());
            }
      return sp;
      }

//---------------------------------------------------------
//   newAccordionPalette
//---------------------------------------------------------

Palette* MuseScore::newAccordionPalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Accordion"));
      sp->setGrid(42, 25);
      sp->setDrawGrid(true);

      // do not include additional symbol-based fingerings (temporarily?) implemented as articulations
      static std::vector<SymId> art {
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
//   newBracketsPalette
//---------------------------------------------------------

Palette* MuseScore::newBracketsPalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Brackets"));
      sp->setMag(0.7);
      sp->setGrid(40, 60);
      sp->setDrawGrid(true);

      for (auto t : std::array<std::pair<BracketType,const char*>, 4> {
         {{ BracketType::NORMAL, "Bracket" },
         { BracketType::BRACE,  "Brace"   },
         { BracketType::SQUARE, "Square"  },
         { BracketType::LINE,   "Line"    }}
         } ) {
            Bracket* b1      = new Bracket(gscore);
            BracketItem* bi1 = new BracketItem(gscore);
            bi1->setBracketType(t.first);
            b1->setBracketItem(bi1);
            sp->append(b1, tr(t.second));      // Brace, Square, Line
            }
      return sp;
      }

//---------------------------------------------------------
//   newBreathPalette
//---------------------------------------------------------

Palette* MuseScore::newBreathPalette()
      {
      Palette* sp = new Palette();
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Breaths && Pauses"));
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
//   newArpeggioPalette
//---------------------------------------------------------

Palette* MuseScore::newArpeggioPalette()
      {
      Palette* sp = new Palette();
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Arpeggios && Glissandos"));
      sp->setGrid(27, 50);
      sp->setDrawGrid(true);

      for (int i = 0; i < 6; ++i) {
            Arpeggio* a = new Arpeggio(gscore);
            a->setArpeggioType(ArpeggioType(i));
            sp->append(a, tr("Arpeggio"));
            }
      for (int i = 0; i < 2; ++i) {
            Glissando* a = new Glissando(gscore);
            a->setGlissandoType(GlissandoType(i));
            sp->append(a, tr("Glissando"));
            }

      //fall and doits

      ChordLine* cl = new ChordLine(gscore);
      cl->setChordLineType(ChordLineType::FALL);
      sp->append(cl, tr(scorelineNames[0]));

      cl = new ChordLine(gscore);
      cl->setChordLineType(ChordLineType::DOIT);
      sp->append(cl, tr(scorelineNames[1]));

      cl = new ChordLine(gscore);
      cl->setChordLineType(ChordLineType::PLOP);
      sp->append(cl, tr(scorelineNames[2]));

      cl = new ChordLine(gscore);
      cl->setChordLineType(ChordLineType::SCOOP);
      sp->append(cl, tr(scorelineNames[3]));

      cl = new ChordLine(gscore);
      cl->setChordLineType(ChordLineType::FALL);
      cl->setStraight(true);
      sp->append(cl, qApp->translate("articulation", "Slide out down"));

      cl = new ChordLine(gscore);
      cl->setChordLineType(ChordLineType::DOIT);
      cl->setStraight(true);
      sp->append(cl, qApp->translate("articulation", "Slide out up"));

      cl = new ChordLine(gscore);
      cl->setChordLineType(ChordLineType::PLOP);
      cl->setStraight(true);
      sp->append(cl, qApp->translate("articulation", "Slide in above"));

      cl = new ChordLine(gscore);
      cl->setChordLineType(ChordLineType::SCOOP);
      cl->setStraight(true);
      sp->append(cl, qApp->translate("articulation", "Slide in below"));

      return sp;
      }

//---------------------------------------------------------
//   newClefsPalette
//---------------------------------------------------------

Palette* MuseScore::newClefsPalette(bool defaultPalette)
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Clefs"));
      sp->setMag(0.8);
      sp->setGrid(35, 50);
      sp->setYOffset(1.0);

      static std::vector<ClefType> clefsDefault  {
            ClefType::G,     ClefType::G8_VA,  ClefType::G15_MA,  ClefType::G8_VB, ClefType::G15_MB, ClefType::G8_VB_O,
            ClefType::G8_VB_P,    ClefType::G_1,  ClefType::C1,  ClefType::C2,    ClefType::C3,
            ClefType::C4,    ClefType::C5, ClefType::F,   ClefType::F_8VA, ClefType::F_15MA,
            ClefType::F8_VB,    ClefType::F15_MB, ClefType::F_B, ClefType::F_C, ClefType::PERC,
            ClefType::PERC2, ClefType::TAB, ClefType::TAB4
            };
      static std::vector<ClefType> clefsMaster  {
            ClefType::G,     ClefType::G8_VA,  ClefType::G15_MA,  ClefType::G8_VB, ClefType::G15_MB, ClefType::G8_VB_O,
            ClefType::G8_VB_P,    ClefType::G_1,  ClefType::C1,  ClefType::C2,    ClefType::C3,
            ClefType::C4,    ClefType::C5,  ClefType::C_19C, ClefType::C3_F18C, ClefType::C4_F18C, ClefType::C3_F20C, ClefType::C4_F20C,
             ClefType::F,   ClefType::F_8VA, ClefType::F_15MA,
            ClefType::F8_VB,    ClefType::F15_MB, ClefType::F_B, ClefType::F_C, ClefType::F_F18C, ClefType::F_19C,  ClefType::PERC,
            ClefType::PERC2, ClefType::TAB, ClefType::TAB4, ClefType::TAB_SERIF, ClefType::TAB4_SERIF
            };

      std::vector<ClefType>* items = nullptr;
      if (defaultPalette) {
            items = &clefsDefault;
            sp->setMoreElements(true);
            }
      else {
            items = &clefsMaster;
            sp->setMoreElements(false);
            }
      connect(sp, SIGNAL(displayMore(const QString&)), mscore, SLOT(showMasterPalette(const QString&)));

      for (ClefType j : *items) {
            Clef* k = new Ms::Clef(gscore);
            k->setClefType(ClefTypeList(j, j));
            sp->append(k, qApp->translate("clefTable", ClefInfo::name(j)));
            }
      return sp;
      }

//---------------------------------------------------------
//   newGraceNotePalette
//---------------------------------------------------------

Palette* MuseScore::newGraceNotePalette()
      {
      Palette* sp = new Palette;
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
      populateIconPalette(sp, gna);
      return sp;
      }

//---------------------------------------------------------
//   newBagpipeEmbellishmentPalette
//---------------------------------------------------------

Palette* MuseScore::newBagpipeEmbellishmentPalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Bagpipe Embellishments"));
      sp->setMag(0.8);
      sp->setYOffset(2.0);
      sp->setGrid(55, 55);
      for (int i = 0; i < BagpipeEmbellishment::nEmbellishments(); ++i) {
            BagpipeEmbellishment* b  = new BagpipeEmbellishment(gscore);
            b->setEmbelType(i);
            sp->append(b, qApp->translate("bagpipe", BagpipeEmbellishment::BagpipeEmbellishmentList[i].name));
            }

      return sp;
      }

//---------------------------------------------------------
//   newLinesPalette
//---------------------------------------------------------

Palette* MuseScore::newLinesPalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Lines"));
      sp->setMag(.8);
      sp->setGrid(75, 28);
      sp->setDrawGrid(true);

      qreal w = gscore->spatium() * 8;

      Slur* slur = new Slur(gscore);
      sp->append(slur, qApp->translate("lines", "Slur"));

      Hairpin* gabel0 = new Hairpin(gscore);
      gabel0->setHairpinType(HairpinType::CRESC_HAIRPIN);
      gabel0->setLen(w);
      sp->append(gabel0, qApp->translate("lines", "Crescendo hairpin"));

      Hairpin* gabel1 = new Hairpin(gscore);
      gabel1->setHairpinType(HairpinType::DECRESC_HAIRPIN);
      gabel1->setLen(w);
      sp->append(gabel1, QT_TRANSLATE_NOOP("Palette", "Diminuendo hairpin"));

      Hairpin* gabel2 = new Hairpin(gscore);
      gabel2->setHairpinType(HairpinType::CRESC_LINE);
      gabel2->setLen(w);
      sp->append(gabel2, qApp->translate("lines", "Crescendo line"));

      Hairpin* gabel3 = new Hairpin(gscore);
      gabel3->setHairpinType(HairpinType::DECRESC_LINE);
      gabel3->setLen(w);
      sp->append(gabel3, QT_TRANSLATE_NOOP("Palette", "Diminuendo line"));

      Hairpin* gabel4 = new Hairpin(gscore);
      gabel4->setHairpinType(HairpinType::CRESC_HAIRPIN);
      gabel4->setBeginText("<sym>dynamicMezzo</sym><sym>dynamicForte</sym>");
      gabel4->setBeginTextAlign(Align::VCENTER);
      gabel4->setLen(w);
      sp->append(gabel4, qApp->translate("lines", "Dynamic + hairpin"));

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
      sp->append(ottava, QT_TRANSLATE_NOOP("Palette", "8va"));

      ottava = new Ottava(gscore);
      ottava->setOttavaType(OttavaType::OTTAVA_8VB);
      ottava->setLen(w);
      ottava->setPlacement(Placement::BELOW);
      ottava->styleChanged();
      sp->append(ottava, QT_TRANSLATE_NOOP("Palette", "8vb"));

      ottava = new Ottava(gscore);
      ottava->setOttavaType(OttavaType::OTTAVA_15MA);
      ottava->setLen(w);
      ottava->styleChanged();
      sp->append(ottava, QT_TRANSLATE_NOOP("Palette", "15ma"));

      ottava = new Ottava(gscore);
      ottava->setOttavaType(OttavaType::OTTAVA_15MB);
      ottava->setLen(w);
      ottava->setPlacement(Placement::BELOW);
      ottava->styleChanged();
      sp->append(ottava, QT_TRANSLATE_NOOP("Palette", "15mb"));

      ottava = new Ottava(gscore);
      ottava->setOttavaType(OttavaType::OTTAVA_22MA);
      ottava->setLen(w);
      ottava->styleChanged();
      sp->append(ottava, QT_TRANSLATE_NOOP("Palette", "22ma"));

      ottava = new Ottava(gscore);
      ottava->setOttavaType(OttavaType::OTTAVA_22MB);
      ottava->setLen(w);
      ottava->styleChanged();
      sp->append(ottava, QT_TRANSLATE_NOOP("Palette", "22mb"));

      Pedal* pedal;
      pedal = new Pedal(gscore);
      pedal->setLen(w);
      pedal->setBeginText("<sym>keyboardPedalPed</sym>");
      pedal->setContinueText("(<sym>keyboardPedalPed</sym>)");
      pedal->setEndHookType(HookType::HOOK_90);
      sp->append(pedal, QT_TRANSLATE_NOOP("Palette", "Pedal"));

      pedal = new Pedal(gscore);
      pedal->setLen(w);
      pedal->setBeginText("<sym>keyboardPedalPed</sym>");
      pedal->setContinueText("(<sym>keyboardPedalPed</sym>)");
      pedal->setEndText("<sym>keyboardPedalUp</sym>");
      pedal->setLineVisible(false);
      sp->append(pedal, QT_TRANSLATE_NOOP("Palette", "Pedal"));

      pedal = new Pedal(gscore);
      pedal->setLen(w);
      pedal->setBeginHookType(HookType::HOOK_90);
      pedal->setEndHookType(HookType::HOOK_90);
      sp->append(pedal, QT_TRANSLATE_NOOP("Palette", "Pedal"));

      pedal = new Pedal(gscore);
      pedal->setLen(w);
      pedal->setBeginHookType(HookType::HOOK_90);
      pedal->setEndHookType(HookType::HOOK_45);
      sp->append(pedal, QT_TRANSLATE_NOOP("Palette", "Pedal"));

      pedal = new Pedal(gscore);
      pedal->setLen(w);
      pedal->setBeginHookType(HookType::HOOK_45);
      pedal->setEndHookType(HookType::HOOK_45);
      sp->append(pedal, QT_TRANSLATE_NOOP("Palette", "Pedal"));

      pedal = new Pedal(gscore);
      pedal->setLen(w);
      pedal->setBeginHookType(HookType::HOOK_45);
      pedal->setEndHookType(HookType::HOOK_90);
      sp->append(pedal, QT_TRANSLATE_NOOP("Palette", "Pedal"));

      for (int i = 0; i < trillTableSize(); i++) {
            Trill* trill = new Trill(gscore);
            trill->setTrillType(trillTable[i].type);
            trill->setLen(w);
            sp->append(trill, qApp->translate("trillType", trillTable[i].userName.toUtf8().constData()));
            }

      TextLine* textLine = new TextLine(gscore);
      textLine->setLen(w);
      textLine->setBeginText("VII");
      textLine->setEndHookType(HookType::HOOK_90);
      sp->append(textLine, QT_TRANSLATE_NOOP("Palette", "Text line"));

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
            sp->append(vibrato, qApp->translate("vibratoType", vibratoTable[i].userName.toUtf8().constData()));
            }

      PalmMute* pm = new PalmMute(gscore);
      pm->setLen(w);
      sp->append(pm, QT_TRANSLATE_NOOP("Palette", "Palm Mute"));

      return sp;
      }

//---------------------------------------------------------
//   showPalette
//---------------------------------------------------------

void MuseScore::showPalette(bool visible)
      {
      QAction* a = getAction("toggle-palette");
      if (paletteBox == 0) {
            Workspace::currentWorkspace->read();
            preferencesChanged();
            updateIcons();
            }
      if (paletteBox)   // read failed?
            paletteBox->setVisible(visible);
      a->setChecked(visible);
      }

//---------------------------------------------------------
//   TempoPattern
//---------------------------------------------------------

struct TempoPattern {
      QString pattern;
      double f;
      bool relative;
      bool italian;
      bool followText;
      bool basic;
      bool masterOnly;

      TempoPattern(const QString& s, double v, bool r, bool i, bool f, bool b, bool m) : pattern(s), f(v), relative(r), italian(i), followText(f), basic(b), masterOnly(m) {}
      };

//---------------------------------------------------------
//   newTempoPalette
//---------------------------------------------------------

Palette* MuseScore::newTempoPalette(bool defaultPalette)
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Tempo"));
      sp->setMag(0.65);
      if (defaultPalette)
            sp->setGrid(66, 28);
      else
            sp->setGrid(116, 28);
      sp->setDrawGrid(true);

      static const TempoPattern tps[] = {
            TempoPattern("<sym>metNoteHalfUp</sym> = 80",    80.0/ 30.0, false, false, true, true, false),                // 1/2
            TempoPattern("<sym>metNoteQuarterUp</sym> = 80", 80.0/ 60.0, false, false, true, true, false),                // 1/4
            TempoPattern("<sym>metNote8thUp</sym> = 80",     80.0/120.0, false, false, true, true, false),                // 1/8
            TempoPattern("<sym>metNoteHalfUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80",    120/ 30.0, false, false, true, false, false),   // dotted 1/2
            TempoPattern("<sym>metNoteQuarterUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80", 120/ 60.0, false, false, true, true, false),   // dotted 1/4
            TempoPattern("<sym>metNote8thUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80",     120/120.0, false, false, true, false, false),   // dotted 1/8

            TempoPattern("Grave",             35.0/60.0, false, true, false, false, false),
            TempoPattern("Largo",             50.0/60.0, false, true, false, false, false),
            TempoPattern("Lento",             52.5/60.0, false, true, false, false, false),
            TempoPattern("Larghetto",         63.0/60.0, false, true, false, false, true),
            TempoPattern("Adagio",            71.0/60.0, false, true, false, false, false),
            TempoPattern("Andante",           92.0/60.0, false, true, false, false, false),
            TempoPattern("Andantino",         94.0/60.0, false, true, false, false, true),
            TempoPattern("Moderato",         114.0/60.0, false, true, false, false, false),
            TempoPattern("Allegretto",       116.0/60.0, false, true, false, false, false),
            TempoPattern("Allegro moderato", 118.0/60.0, false, true, false, false, true),
            TempoPattern("Allegro",          144.0/60.0, false, true, false, false, false),
            TempoPattern("Vivace",           172.0/60.0, false, true, false, false, false),
            TempoPattern("Presto",           187.0/60.0, false, true, false, false, false),
            TempoPattern("Prestissimo",      200.0/60.0, false, true, false, false, true),

            TempoPattern("<sym>metNoteQuarterUp</sym> = <sym>metNoteQuarterUp</sym><sym>space</sym><sym>metAugmentationDot</sym>", 3.0/2.0, true, false, true, false, false),
            TempoPattern("<sym>metNoteQuarterUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = <sym>metNoteQuarterUp</sym>", 2.0/3.0, true, false, true, false, false),
            TempoPattern("<sym>metNoteHalfUp</sym> = <sym>metNoteQuarterUp</sym>",    1.0/2.0, true, false, true, false, false),
            TempoPattern("<sym>metNoteQuarterUp</sym> = <sym>metNoteHalfUp</sym>",    2.0/1.0, true, false, true, false, false),
            TempoPattern("<sym>metNote8thUp</sym> = <sym>metNote8thUp</sym>",         1.0/1.0, true, false, true, false, false),
            TempoPattern("<sym>metNoteQuarterUp</sym> = <sym>metNoteQuarterUp</sym>", 1.0/1.0, true, false, true, false, false),
            };
      for (TempoPattern tp : tps) {
            TempoText* tt = new TempoText(gscore);
            tt->setFollowText(tp.followText);
            tt->setXmlText(tp.pattern);
            if (tp.relative) {
                  tt->setRelative(tp.f);
                  sp->append(tt, tr("Metric modulation"), QString(), 1.5);
                  }
            else if (tp.italian) {
                  tt->setTempo(tp.f);
                  sp->append(tt, tr("Tempo text"), QString(), 1.3);
                  }
            else {
                  tt->setTempo(tp.f);
                  sp->append(tt, tr("Tempo text"), QString(), 1.5);
                  }
            }
      sp->setMoreElements(false);
      connect(sp, SIGNAL(displayMore(const QString&)), mscore, SLOT(showMasterPalette(const QString&)));

      return sp;
      }

//---------------------------------------------------------
//   newTextPalette
//---------------------------------------------------------

Palette* MuseScore::newTextPalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Text"));
      sp->setMag(0.85);
      sp->setGrid(84, 28);
      sp->setDrawGrid(true);

      StaffText* st = new StaffText(gscore);
      st->setXmlText(tr("Staff Text"));
      sp->append(st, tr("Staff text"));

      st = new StaffText(gscore, Tid::EXPRESSION);
      st->setXmlText(tr("Expression"));
      st->setPlacement(Placement::BELOW);
      st->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
      sp->append(st, tr("Expression text"));

      InstrumentChange* is = new InstrumentChange(gscore);
      is->setXmlText(tr("Change Instr."));
      sp->append(is, tr("Instrument change"));

      StaffTypeChange* stc = new StaffTypeChange(gscore);
      sp->append(stc, tr("Staff type change"));

      RehearsalMark* rhm = new RehearsalMark(gscore);
      rhm->setXmlText("B1");
      sp->append(rhm, tr("Rehearsal mark"));

      SystemText* stxt = new SystemText(gscore, Tid::TEMPO);
      stxt->setXmlText(tr("Swing"));
      stxt->setSwing(true);
      sp->append(stxt, tr("Swing"));

      stxt = new SystemText(gscore);
      stxt->setXmlText(tr("System Text"));
      sp->append(stxt, tr("System text"));

      return sp;
      }

//---------------------------------------------------------
//   newTimePalette
//    create default time signature palette
//---------------------------------------------------------

Palette* MuseScore::newTimePalette()
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
            { 9,  8, TimeSigType::NORMAL, "9/8" },
            { 12, 8, TimeSigType::NORMAL, "12/8" },
            { 4,  4, TimeSigType::FOUR_FOUR,  tr("4/4 common time") },
            { 2,  2, TimeSigType::ALLA_BREVE, tr("2/2 alla breve") }
            };

      Palette* sp = new Palette;
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
//    newFretboardDiagramPalette
//-----------------------------------

Palette* MuseScore::newFretboardDiagramPalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Fretboard Diagrams"));
      sp->setGrid(42, 45);
      sp->setDrawGrid(true);

      FretDiagram* fret = FretDiagram::fromString(gscore, "X32O1O");
      sp->append(fret, "C");
      fret = FretDiagram::fromString(gscore, "X-554-");
      sp->append(fret, "Cm");
      fret = FretDiagram::fromString(gscore, "X3231O");
      sp->append(fret, "C7");

      fret = FretDiagram::fromString(gscore, "XXO232");
      sp->append(fret, "D");
      fret = FretDiagram::fromString(gscore, "XXO231");
      sp->append(fret, "Dm");
      fret = FretDiagram::fromString(gscore, "XXO212");
      sp->append(fret, "D7");

      fret = FretDiagram::fromString(gscore, "O221OO");
      sp->append(fret, "E");
      fret = FretDiagram::fromString(gscore, "O22OOO");
      sp->append(fret, "Em");
      fret = FretDiagram::fromString(gscore, "O2O1OO");
      sp->append(fret, "E7");

      fret = FretDiagram::fromString(gscore, "-332--");
      sp->append(fret, "F");
      fret = FretDiagram::fromString(gscore, "-33---");
      sp->append(fret, "Fm");
      fret = FretDiagram::fromString(gscore, "-3-2--");
      sp->append(fret, "F7");

      fret = FretDiagram::fromString(gscore, "32OOO3");
      sp->append(fret, "G");
      fret = FretDiagram::fromString(gscore, "-55---");
      sp->append(fret, "Gm");
      fret = FretDiagram::fromString(gscore, "32OOO1");
      sp->append(fret, "G7");

      fret = FretDiagram::fromString(gscore, "XO222O");
      sp->append(fret, "A");
      fret = FretDiagram::fromString(gscore, "XO221O");
      sp->append(fret, "Am");
      fret = FretDiagram::fromString(gscore, "XO2O2O");
      sp->append(fret, "A7");

      fret = FretDiagram::fromString(gscore, "X-444-");
      sp->append(fret, "B");
      fret = FretDiagram::fromString(gscore, "X-443-");
      sp->append(fret, "Bm");
      fret = FretDiagram::fromString(gscore, "X212O2");
      sp->append(fret, "B7");

      return sp;
      }

//---------------------------------------------------------
//   setDefaultPalette
//---------------------------------------------------------

void MuseScore::setDefaultPalette()
      {
      mscore->getPaletteBox();
      paletteBox->clear();
      paletteBox->addPalette(newClefsPalette(true));
      paletteBox->addPalette(newKeySigPalette());
      paletteBox->addPalette(newTimePalette());
      paletteBox->addPalette(newBracketsPalette());
      paletteBox->addPalette(newAccidentalsPalette(true));
      paletteBox->addPalette(newArticulationsPalette());
      paletteBox->addPalette(newOrnamentsPalette());
      paletteBox->addPalette(newBreathPalette());
      paletteBox->addPalette(newGraceNotePalette());
      paletteBox->addPalette(newNoteHeadsPalette());
      paletteBox->addPalette(newLinesPalette());
      paletteBox->addPalette(newBarLinePalette());
      paletteBox->addPalette(newArpeggioPalette());
      paletteBox->addPalette(newTremoloPalette());
      paletteBox->addPalette(newTextPalette());
      paletteBox->addPalette(newTempoPalette(true));
      paletteBox->addPalette(newDynamicsPalette(true));
      paletteBox->addPalette(newFingeringPalette());
      paletteBox->addPalette(newRepeatsPalette());
      paletteBox->addPalette(newFretboardDiagramPalette());
      paletteBox->addPalette(newBagpipeEmbellishmentPalette());
      paletteBox->addPalette(newBreaksPalette());
      paletteBox->addPalette(newFramePalette());
      paletteBox->addPalette(newBeamPalette());
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

