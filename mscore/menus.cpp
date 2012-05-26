//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: menus.cpp 5651 2012-05-19 15:57:26Z lasconic $
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#include "libmscore/score.h"
#include "palette.h"
#include "libmscore/note.h"
#include "libmscore/chordrest.h"
#include "libmscore/dynamic.h"
#include "libmscore/slur.h"
#include "libmscore/sym.h"
#include "libmscore/hairpin.h"
#include "scoreview.h"
#include "musescore.h"
#include "edittempo.h"
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
#include "libmscore/instrchange.h"
#include "profile.h"
#include "libmscore/icon.h"
#include "libmscore/accidental.h"
#include "libmscore/harmony.h"
#include "libmscore/rehearsalmark.h"
#include "shortcut.h"

extern bool useFactorySettings;

//---------------------------------------------------------
//   populateIconPalette
//---------------------------------------------------------

void populateIconPalette(Palette* p, const IconAction* a)
      {
      while (a->subtype != -1) {
            Icon* ik = new Icon(gscore);
            ik->setSubtype(a->subtype);
            Shortcut* s = Shortcut::getShortcut(a->action);
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
      sp->setName(tr("Beam Properties"));
      sp->setGrid(27, 40);
      sp->setDrawGrid(true);

      static const IconAction bpa[] = {
            { ICON_SBEAM,    "beam-start" },
            { ICON_MBEAM,    "beam-mid" },
            { ICON_NBEAM,    "no-beam" },
            { ICON_BEAM32,   "beam32" },
            { ICON_BEAM64,   "beam64" },
            { ICON_AUTOBEAM, "auto-beam" },
            { ICON_FBEAM1,   "fbeam1" },
            { ICON_FBEAM2,   "fbeam2" },
            { -1, ""}
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
      sp->setName(tr("Frames"));
      sp->setGrid(27, 40);
      sp->setDrawGrid(true);

      static const IconAction bpa[] = {
            { ICON_VFRAME,   "vframe" },
            { ICON_HFRAME,   "hframe" },
            { ICON_TFRAME,   "tframe" },
            { ICON_FFRAME,   "fframe" },
            { ICON_MEASURE,  "measure" },
            { -1, ""}
            };
      populateIconPalette(sp, bpa);
      return sp;
      }

//---------------------------------------------------------
//   newDynamicsPalette
//---------------------------------------------------------

Palette* MuseScore::newDynamicsPalette()
      {
      Palette* sp = new Palette;
      sp->setName(tr("Dynamics"));
      sp->setMag(.8);
      sp->setGrid(42, 28);
      sp->setDrawGrid(true);

      static const char* dynS[] = {
            "ppp", "pp", "p", "mp", "mf", "f", "ff", "fff"
            };
      for (unsigned i = 0; i < sizeof(dynS)/sizeof(*dynS); ++i) {
            Dynamic* dynamic = new Dynamic(gscore);
            dynamic->setSubtype(dynS[i]);
            sp->append(dynamic, dynamic->subtypeName());
            }
      return sp;
      }

//---------------------------------------------------------
//   newKeySigPalette
//---------------------------------------------------------

Palette* MuseScore::newKeySigPalette()
      {
      Palette* sp = new Palette;
      sp->setName(qApp->translate("MuseScore", "Key Signatures"));
      sp->setMag(0.8);
      sp->setGrid(56, 45);
      sp->setYOffset(1.0);

      for (int i = 0; i < 7; ++i) {
            KeySig* k = new KeySig(gscore);
            k->setKeySigEvent(KeySigEvent(i+1));
            sp->append(k, qApp->translate("MuseScore", keyNames[i*2]));
            }
      for (int i = -7; i < 0; ++i) {
            KeySig* k = new KeySig(gscore);
            k->setKeySigEvent(KeySigEvent(i));
            sp->append(k, qApp->translate("MuseScore", keyNames[(7 + i) * 2 + 1]));
            }
      KeySig* k = new KeySig(gscore);
      k->setKeySigEvent(KeySigEvent(0));
      sp->append(k, qApp->translate("MuseScore", keyNames[14]));
      return sp;
      }

//---------------------------------------------------------
//   newAccidentalsPalette
//---------------------------------------------------------

Palette* MuseScore::newAccidentalsPalette()
      {
      Palette* sp = new Palette;
      sp->setName(qApp->translate("accidental", "Accidentals"));
      sp->setGrid(33, 36);
      sp->setDrawGrid(true);

      for (int i = ACC_SHARP; i < ACC_END; ++i) {
            Accidental* s = new Accidental(gscore);
            s->setSubtype(AccidentalType(i));
            sp->append(s, qApp->translate("accidental", s->subtypeUserName()));
            }
      AccidentalBracket* ab = new AccidentalBracket(gscore);
      ab->setSubtype(0);
      sp->append(ab, qApp->translate("accidental", "round bracket"));
      return sp;
      }

//---------------------------------------------------------
//   newBarLinePalette
//---------------------------------------------------------

Palette* MuseScore::newBarLinePalette()
      {
      Palette* sp = new Palette;
      sp->setName(qApp->translate("barlines", "Barlines"));
      sp->setMag(0.8);
      sp->setGrid(42, 38);

      struct {
            BarLineType type;
            const char* name;
            } t[] = {
            { NORMAL_BAR,       QT_TR_NOOP("Normal") },
            { BROKEN_BAR,       QT_TR_NOOP("Dashed") },
            { END_BAR,          QT_TR_NOOP("End Bar") },
            { DOUBLE_BAR,       QT_TR_NOOP("Double Bar") },
            { START_REPEAT,     QT_TR_NOOP("Start Repeat") },
            { END_REPEAT,       QT_TR_NOOP("End Repeat") },
            { END_START_REPEAT, QT_TR_NOOP("End-Start Repeat") },
            };
      for (unsigned i = 0; i < sizeof(t)/sizeof(*t); ++i) {
            BarLine* b  = new BarLine(gscore);
            b->setSubtype(t[i].type);
            sp->append(b, qApp->translate("barlines", t[i].name));
            }
      return sp;
      }

//---------------------------------------------------------
//   newRepeatsPalette
//---------------------------------------------------------

Palette* MuseScore::newRepeatsPalette()
      {
      Palette* sp = new Palette;
      sp->setName(tr("Repeats"));
      sp->setMag(0.65);
      sp->setGrid(84, 28);
      sp->setDrawGrid(true);

      RepeatMeasure* rm = new RepeatMeasure(gscore);
      sp->append(rm, tr("Repeat measure sign"));

      Marker* mk = new Marker(gscore);
      mk->setMarkerType(MARKER_SEGNO);
      sp->append(mk, tr("Segno"));

      mk = new Marker(gscore);
      mk->setMarkerType(MARKER_VARSEGNO);
      PaletteCell* cell = sp->append(mk, tr("Segno Variation"), "", 0.6);
      cell->yoffset = -2;

      mk = new Marker(gscore);
      mk->setMarkerType(MARKER_CODA);
      sp->append(mk, tr("Coda"));

      mk = new Marker(gscore);
      mk->setMarkerType(MARKER_VARCODA);
      sp->append(mk, tr("Varied coda"));

      mk = new Marker(gscore);
      mk->setMarkerType(MARKER_CODETTA);
      sp->append(mk, tr("Codetta"));

      mk = new Marker(gscore);
      mk->setMarkerType(MARKER_FINE);
      sp->append(mk, tr("Fine"));

      Jump* jp = new Jump(gscore);
      jp->setJumpType(JUMP_DC);
      sp->append(jp, tr("Da Capo"));

      jp = new Jump(gscore);
      jp->setJumpType(JUMP_DC_AL_FINE);
      sp->append(jp, tr("Da Capo al Fine"));

      jp = new Jump(gscore);
      jp->setJumpType(JUMP_DC_AL_CODA);
      sp->append(jp, tr("Da Capo al Coda"));

      jp = new Jump(gscore);
      jp->setJumpType(JUMP_DS_AL_CODA);
      sp->append(jp, tr("D.S al Coda"));

      jp = new Jump(gscore);
      jp->setJumpType(JUMP_DS_AL_FINE);
      sp->append(jp, tr("D.S al Fine"));

      jp = new Jump(gscore);
      jp->setJumpType(JUMP_DS);
      sp->append(jp, tr("D.S"));

      mk = new Marker(gscore);
      mk->setMarkerType(MARKER_TOCODA);
      sp->append(mk, tr("To Coda"));
      return sp;
      }

//---------------------------------------------------------
//   newBreaksPalette
//---------------------------------------------------------

Palette* MuseScore::newBreaksPalette()
      {
      qreal _spatium = gscore->spatium();
      Palette* sp = new Palette;
      sp->setName(tr("Breaks && Spacer"));
      sp->setMag(.7);
      sp->setGrid(42, 36);
      sp->setDrawGrid(true);

      LayoutBreak* lb = new LayoutBreak(gscore);
      lb->setSubtype(LAYOUT_BREAK_LINE);
      sp->append(lb, tr("Line break"));

      lb = new LayoutBreak(gscore);
      lb->setSubtype(LAYOUT_BREAK_PAGE);
      sp->append(lb, tr("Page break"));

      lb = new LayoutBreak(gscore);
      lb->setSubtype(LAYOUT_BREAK_SECTION);
      sp->append(lb, tr("Section break"));

      Spacer* spacer = new Spacer(gscore);
      spacer->setGap(3 * _spatium);
      spacer->setSubtype(SPACER_DOWN);
      sp->append(spacer, tr("Staff spacer down"));

      spacer = new Spacer(gscore);
      spacer->setGap(3 * _spatium);
      spacer->setSubtype(SPACER_UP);
      sp->append(spacer, tr("Staff spacer up"));
      return sp;
      }

//---------------------------------------------------------
//   newFingeringPalette
//---------------------------------------------------------

Palette* MuseScore::newFingeringPalette()
      {
      Palette* sp = new Palette;
      sp->setName(tr("Fingering"));
      sp->setMag(1.5);
      sp->setGrid(28, 30);
      sp->setDrawGrid(true);

      const char finger[] = "012345pimac";
      for (unsigned i = 0; i < strlen(finger); ++i) {
            Fingering* f = new Fingering(gscore);
            f->setText(QString(finger[i]));
            sp->append(f, tr("Fingering %1").arg(finger[i]));
            }
      const char stringnumber[] = "0123456";
      for (unsigned i = 0; i < strlen(stringnumber); ++i) {
            Fingering* f = new Fingering(gscore);
            f->setTextStyleType(TEXT_STYLE_STRING_NUMBER);
            f->setText(QString(stringnumber[i]));
            sp->append(f, tr("String number %1").arg(stringnumber[i]));
            }
      Symbol* symbol = new Symbol(gscore, thumbSym);
      sp->append(symbol, tr("Thumb"));
      return sp;
      }

//--------------------------------------------------------
//   newFallDoitPalett
//--------------------------------------------------------

Palette* MuseScore::newFallDoitPalette()
      {
      Palette* sp = new Palette;
      sp->setName(tr("Fall/Doit"));
      sp->setGrid(27, 40);
      sp->setDrawGrid(true);
      const char* scorelineNames[] = {
            QT_TR_NOOP("fall"),
            QT_TR_NOOP("doit"),
            };

      ChordLine* cl = new ChordLine(gscore);
      cl->setSubtype(CHORDLINE_FALL);
      sp->append(cl, tr(scorelineNames[0]));

      cl = new ChordLine(gscore);
      cl->setSubtype(CHORDLINE_DOIT);
      sp->append(cl, tr(scorelineNames[1]));
      return sp;
      }

//---------------------------------------------------------
//   newTremoloPalette
//---------------------------------------------------------

Palette* MuseScore::newTremoloPalette()
      {
      Palette* sp = new Palette;
      sp->setName(tr("Tremolo"));
      sp->setGrid(27, 40);
      sp->setDrawGrid(true);
      const char* tremoloName[] = {
            QT_TR_NOOP("1/8 through stem"),
            QT_TR_NOOP("1/16 through stem"),
            QT_TR_NOOP("1/32 through stem"),
            QT_TR_NOOP("1/64 through stem"),
            QT_TR_NOOP("1/8 between notes"),
            QT_TR_NOOP("1/16 between notes"),
            QT_TR_NOOP("1/32 between notes"),
            QT_TR_NOOP("1/64 between notes")
            };

      for (int i = TREMOLO_R8; i <= TREMOLO_C64; ++i) {
            Tremolo* tremolo = new Tremolo(gscore);
            tremolo->setSubtype(TremoloType(i));
            sp->append(tremolo, tr(tremoloName[i - TREMOLO_R8]));
            }
      return sp;
      }

//---------------------------------------------------------
//   newNoteHeadsPalette
//---------------------------------------------------------

Palette* MuseScore::newNoteHeadsPalette()
      {
      Palette* sp = new Palette;
      sp->setName(tr("Note Heads"));
      sp->setMag(1.3);
      sp->setGrid(33, 36);
      sp->setDrawGrid(true);

      for (int i = 0; i < HEAD_GROUPS; ++i) {
            int sym = noteHeads[0][i][1];
            if (i == HEAD_BREVIS_ALT)
                  sym = noteHeads[0][i][3];
            NoteHead* nh = new NoteHead(gscore);
            nh->setSym(sym);
            sp->append(nh, qApp->translate("symbol", symbols[0][sym].name()));
            }
      return sp;
      }

//---------------------------------------------------------
//   newArticulationsPalette
//---------------------------------------------------------

Palette* MuseScore::newArticulationsPalette()
      {
      Palette* sp = new Palette;
      sp->setName(tr("Articulations && Ornaments"));
      sp->setGrid(42, 25);
      sp->setDrawGrid(true);

      for (int i = 0; i < ARTICULATIONS; ++i) {
            Articulation* s = new Articulation(gscore);
            s->setSubtype(ArticulationType(i));
            sp->append(s, qApp->translate("articulation", qPrintable(s->subtypeUserName())));
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
      sp->append(tb, qApp->translate("articulation", "Tremolo Bar"));
      return sp;
      }

//---------------------------------------------------------
//   newBracketsPalette
//---------------------------------------------------------

Palette* MuseScore::newBracketsPalette()
      {
      Palette* sp = new Palette;
      sp->setName(tr("Brackets"));
      sp->setMag(0.7);
      sp->setGrid(42, 60);
      sp->setDrawGrid(true);

      Bracket* b1 = new Bracket(gscore);
      b1->setSubtype(BRACKET_NORMAL);
      Bracket* b2 = new Bracket(gscore);
      b2->setSubtype(BRACKET_AKKOLADE);

      sp->append(b1, tr("Square bracket"));
      sp->append(b2, tr("Curly bracket"));

      return sp;
      }

//---------------------------------------------------------
//   newBreathPalette
//---------------------------------------------------------

Palette* MuseScore::newBreathPalette()
      {
      Palette* sp = new Palette();
      sp->setName(tr("Breath && Pauses"));
      sp->setGrid(42, 40);
      sp->setDrawGrid(true);
      sp->setDrawGrid(true);

      for (int i = 0; i < 4; ++i) {
            Breath* a = new Breath(gscore);
            a->setSubtype(i);
            if (i < 2)
                  sp->append(a, tr("Breath"));
            else
                  sp->append(a, tr("Caesura"));
            }
      return sp;
      }

//---------------------------------------------------------
//   newArpeggioPalette
//---------------------------------------------------------

Palette* MuseScore::newArpeggioPalette()
      {
      Palette* sp = new Palette();
      sp->setName(tr("Arpeggio && Glissando"));
      sp->setGrid(27, 60);
      sp->setDrawGrid(true);

      for (int i = 0; i < 4; ++i) {
            Arpeggio* a = new Arpeggio(gscore);
            a->setSubtype(ArpeggioType(i));
            sp->append(a, tr("Arpeggio"));
            }
      for (int i = 0; i < 2; ++i) {
            Glissando* a = new Glissando(gscore);
            a->setSubtype(i);
            sp->append(a, tr("Glissando"));
            }
      return sp;
      }

//---------------------------------------------------------
//   newClefsPalette
//---------------------------------------------------------

Palette* MuseScore::newClefsPalette()
      {
      Palette* sp = new Palette;
      sp->setName(tr("Clefs"));
      sp->setMag(0.8);
      sp->setGrid(33, 60);
      sp->setYOffset(1.0);
      static const ClefType clefs[21] = {
            CLEF_G, CLEF_G1, CLEF_G2, CLEF_G3, CLEF_G4,
            CLEF_C1, CLEF_C2, CLEF_C3, CLEF_C4, CLEF_C5,
            CLEF_F, CLEF_F_8VA, CLEF_F_15MA, CLEF_F8, CLEF_F15, CLEF_F_B, CLEF_F_C,
            CLEF_PERC, CLEF_TAB, CLEF_TAB2, CLEF_PERC2
            };
      for (int i = 0; i < 20; ++i) {
            ClefType j = clefs[i];
            Clef* k = new ::Clef(gscore);
            k->setClefType(ClefTypeList(j, j));
            sp->append(k, qApp->translate("clefTable", clefTable[j].name));
            }
      return sp;
      }

//---------------------------------------------------------
//   newGraceNotePalette
//---------------------------------------------------------

Palette* MuseScore::newGraceNotePalette()
      {
      Palette* notePalette = new Palette;
      notePalette->setName(tr("Grace Notes"));
      notePalette->setGrid(32, 40);
      notePalette->setDrawGrid(true);

      static const IconAction gna[] = {
            { ICON_ACCIACCATURA, "acciaccatura" },
            { ICON_APPOGGIATURA, "appoggiatura" },
            { ICON_GRACE4,       "grace4" },
            { ICON_GRACE16,      "grace16" },
            { ICON_GRACE32,      "grace32" },
            { ICON_GRACE8B,      "grace8b" },
            { -1, "" }
            };
      populateIconPalette(notePalette, gna);
      return notePalette;
      }

//---------------------------------------------------------
//   newLinesPalette
//---------------------------------------------------------

Palette* MuseScore::newLinesPalette()
      {
      Palette* sp = new Palette;
      sp->setName(qApp->translate("lines", "Lines"));
      sp->setMag(.8);
      sp->setGrid(82, 23);
      sp->setDrawGrid(true);

      qreal w = gscore->spatium() * 7;
      Slur* slur = new Slur(gscore);
      slur->setLen(w);
      slur->setId(0);
      sp->append(slur, qApp->translate("lines", "Slur"));

      Hairpin* gabel0 = new Hairpin(gscore);
      gabel0->setSubtype(0);
      gabel0->setLen(w);
      sp->append(gabel0, qApp->translate("lines", "Crescendo"));

      Hairpin* gabel1 = new Hairpin(gscore);
      gabel1->setSubtype(1);
      gabel1->setLen(w);
      sp->append(gabel1, qApp->translate("lines", "Diminuendo"));

      Volta* volta = new Volta(gscore);
      volta->setSubtype(VOLTA_CLOSED);
      volta->setLen(w);
      volta->setText("1.");
      QList<int> il;
      il.append(1);
      volta->setEndings(il);
      sp->append(volta, qApp->translate("lines", "Prima volta"));

      volta = new Volta(gscore);
      volta->setSubtype(VOLTA_CLOSED);
      volta->setLen(w);
      volta->setText("2.");
      il.clear();
      il.append(2);
      volta->setEndings(il);
      sp->append(volta, qApp->translate("lines", "Seconda volta"));

      volta = new Volta(gscore);
      volta->setSubtype(VOLTA_CLOSED);
      volta->setLen(w);
      volta->setText("3.");
      il.clear();
      il.append(3);
      volta->setEndings(il);
      sp->append(volta, qApp->translate("lines", "Terza volta"));

      volta = new Volta(gscore);
      volta->setSubtype(VOLTA_OPEN);
      volta->setLen(w);
      volta->setText("2.");
      il.clear();
      il.append(2);
      volta->setEndings(il);
      sp->append(volta, qApp->translate("lines", "Seconda volta 2"));

      Ottava* ottava = new Ottava(gscore);
      ottava->setSubtype(0);
      ottava->setLen(w);
      sp->append(ottava, qApp->translate("lines", "8va"));

      ottava = new Ottava(gscore);
      ottava->setSubtype(1);
      ottava->setLen(w);
      sp->append(ottava, qApp->translate("lines", "15ma"));

      ottava = new Ottava(gscore);
      ottava->setSubtype(2);
      ottava->setLen(w);
      sp->append(ottava, qApp->translate("lines", "8vb"));

      ottava = new Ottava(gscore);
      ottava->setSubtype(3);
      ottava->setLen(w);
      sp->append(ottava, qApp->translate("lines", "15mb"));

      Pedal* pedal = new Pedal(gscore);
      pedal->setLen(w);
      sp->append(pedal, qApp->translate("lines", "Pedal"));

      pedal = new Pedal(gscore);
      pedal->setLen(w);
      pedal->setEndHookType(HOOK_45);
      sp->append(pedal, qApp->translate("lines", "Pedal"));

      pedal = new Pedal(gscore);
      pedal->setLen(w);
      pedal->setBeginSymbol(-1);
      pedal->setBeginHook(true);
      pedal->setBeginHookType(HOOK_45);
      pedal->setEndHookType(HOOK_45);
      sp->append(pedal, qApp->translate("lines", "Pedal"));

      pedal = new Pedal(gscore);
      pedal->setLen(w);
      pedal->setBeginSymbol(-1);
      pedal->setBeginHook(true);
      pedal->setBeginHookType(HOOK_45);
      sp->append(pedal, qApp->translate("lines", "Pedal"));

      Trill* trill = new Trill(gscore);
      trill->setLen(w);
      sp->append(trill, qApp->translate("lines", "Trill line"));

      trill = new Trill(gscore);
      trill->setSubtype("upprall");
      trill->setLen(w);
      sp->append(trill, qApp->translate("lines", "Upprall line"));

      trill = new Trill(gscore);
      trill->setSubtype("downprall");
      trill->setLen(w);
      sp->append(trill, qApp->translate("lines", "Downprall line"));

      trill = new Trill(gscore);
      trill->setSubtype("prallprall");
      trill->setLen(w);
      sp->append(trill, qApp->translate("lines", "Prallprall line"));

      TextLine* textLine = new TextLine(gscore);
      textLine->setLen(w);
      textLine->setBeginText("VII", gscore->textStyle(TEXT_STYLE_TEXTLINE));
      textLine->setEndHook(true);
      sp->append(textLine, qApp->translate("lines", "Text line"));

      TextLine* line = new TextLine(gscore);
      line->setLen(w);
      line->setDiagonal(true);
      sp->append(line, qApp->translate("lines", "Line"));
      return sp;
      }

//---------------------------------------------------------
//   showPalette
//---------------------------------------------------------

void MuseScore::showPalette(bool visible)
      {
      QAction* a = getAction("toggle-palette");
      if (paletteBox == 0)
            profile->read();
      if (paletteBox)   // read failed?
            paletteBox->setShown(visible);
      a->setChecked(visible);
      }

//---------------------------------------------------------
//   TempoPattern
//---------------------------------------------------------

struct TempoPattern {
      QString pattern;
      double f;

      TempoPattern(const QString& s, double v) : pattern(s), f(v) {}
      };

//---------------------------------------------------------
//   newTempoPalette
//---------------------------------------------------------

Palette* MuseScore::newTextPalette()
      {
      Palette* sp = new Palette;
      sp->setName(tr("Text"));
      sp->setMag(0.65);
      sp->setGrid(84, 28);
      sp->setDrawGrid(true);

      StaffText* st = new StaffText(gscore);
      st->setTextStyleType(TEXT_STYLE_STAFF);
      st->setText(tr("staff-text"));
      sp->append(st, tr("Staff Text"));

      st = new StaffText(gscore);
      st->setTextStyleType(TEXT_STYLE_SYSTEM);
      st->setText(tr("system-text"));
      sp->append(st, tr("System Text"));

      RehearsalMark* rhm = new RehearsalMark(gscore);
      rhm->setTrack(0);
      rhm->setText("B1");
      sp->append(rhm, tr("Rehearsal Mark"));

      InstrumentChange* is = new InstrumentChange(gscore);
      is->setText(tr("Instrument"));
      sp->append(is, tr("Instrument Change"));

      Text* text = new Text(gscore);
      text->setTrack(0);
      text->setTextStyleType(TEXT_STYLE_LYRICS_VERSE_NUMBER);
      text->setText(tr("1."));
      sp->append(text, tr("Lyrics Verse Number"));

      static const TempoPattern tp[] = {
            TempoPattern(QString("%1%2 = 80").    arg(QChar(0xd834)).arg(QChar(0xdd5f)), 80.0/60.0),      // 1/4
            TempoPattern(QString("%1%2 = 80").    arg(QChar(0xd834)).arg(QChar(0xdd5e)), 80.0/30.0),      // 1/2
            TempoPattern(QString("%1%2 = 80").    arg(QChar(0xd834)).arg(QChar(0xdd60)), 80.0/120.0),     // 1/8
            TempoPattern(QString("%1%2%3%4 = 80").arg(QChar(0xd834)).arg(QChar(0xdd5f)).arg(QChar(0xd834)).arg(QChar(0xdd6d)), 120.0/60.0),  // dotted 1/4
            TempoPattern(QString("%1%2%3%4 = 80").arg(QChar(0xd834)).arg(QChar(0xdd5e)).arg(QChar(0xd834)).arg(QChar(0xdd6d)), 120/30.0),    // dotted 1/2
            TempoPattern(QString("%1%2%3%4 = 80").arg(QChar(0xd834)).arg(QChar(0xdd60)).arg(QChar(0xd834)).arg(QChar(0xdd6d)), 120/120.0)    // dotted 1/8
            };
      for (unsigned i = 0; i < sizeof(tp)/sizeof(*tp); ++i) {
            TempoText* tt = new TempoText(gscore);
            tt->setFollowText(true);
            tt->setTrack(0);
            tt->setTempo(tp[i].f);
            tt->setText(tp[i].pattern);
            sp->append(tt, tr("Tempo Text"), QString(), 1.5);
            }

      Harmony* harmony = new Harmony(gscore);
      harmony->setText("c7");
      sp->append(harmony, tr("Chord Name"));
      return sp;
      }

//---------------------------------------------------------
//   populatePalette
//---------------------------------------------------------

void MuseScore::populatePalette()
      {
      paletteBox->addPalette(newGraceNotePalette());
      paletteBox->addPalette(newClefsPalette());
      paletteBox->addPalette(newKeySigPalette());

      //-----------------------------------
      //    Time
      //-----------------------------------

      Palette* sp = new Palette;
      sp->setName(tr("Time Signatures"));
      sp->setMag(.8);
      sp->setGrid(42, 38);

      TimeSig* ts;
      ts = new TimeSig(gscore);
      ts->setSig(2, 2);
      sp->append(ts, "2/2");

      sp->append(new TimeSig(gscore,  2, 4), "2/4");
      sp->append(new TimeSig(gscore,  3, 4), "3/4");
      sp->append(new TimeSig(gscore,  4, 4), "4/4");
      sp->append(new TimeSig(gscore,  5, 4), "5/4");
      sp->append(new TimeSig(gscore,  6, 4), "6/4");
      sp->append(new TimeSig(gscore,  3, 8), "3/8");
      sp->append(new TimeSig(gscore,  6, 8), "6/8");
      sp->append(new TimeSig(gscore,  9, 8), "9/8");
      sp->append(new TimeSig(gscore, 12, 8), "12/8");

      sp->append(new TimeSig(gscore, TSIG_FOUR_FOUR),  tr("4/4 common time"));
      sp->append(new TimeSig(gscore, TSIG_ALLA_BREVE), tr("2/2 alla breve"));
      paletteBox->addPalette(sp);

      paletteBox->addPalette(newBarLinePalette());
      paletteBox->addPalette(newLinesPalette());
      paletteBox->addPalette(newArpeggioPalette());
      paletteBox->addPalette(newBreathPalette());
      paletteBox->addPalette(newBracketsPalette());
      paletteBox->addPalette(newArticulationsPalette());
      paletteBox->addPalette(newAccidentalsPalette());
      paletteBox->addPalette(newDynamicsPalette());
      paletteBox->addPalette(newFingeringPalette());
      paletteBox->addPalette(newNoteHeadsPalette());
      paletteBox->addPalette(newTremoloPalette());
      paletteBox->addPalette(newFallDoitPalette());
      paletteBox->addPalette(newRepeatsPalette());
      paletteBox->addPalette(newTextPalette());
      paletteBox->addPalette(newBreaksPalette());

      //-----------------------------------
      //    staff state changes
      //-----------------------------------

#if 0
      sp = new Palette;
      sp->setName(tr("Staff Changes"));
      sp->setMag(.7);
      sp->setGrid(42, 36);
      sp->setDrawGrid(true);

      StaffState* st = new StaffState(gscore);
      st->setSubtype(STAFF_STATE_VISIBLE);
      sp->append(st, tr("set visible"));

      st = new StaffState(gscore);
      st->setSubtype(STAFF_STATE_INVISIBLE);
      sp->append(st, tr("set invisible"));

      st = new StaffState(gscore);
      st->setSubtype(STAFF_STATE_TYPE);
      sp->append(st, tr("change staff type"));

      st = new StaffState(gscore);
      st->setSubtype(STAFF_STATE_INSTRUMENT);
      sp->append(st, tr("change instrument"));

      paletteBox->addPalette(sp);
#endif

      paletteBox->addPalette(newBeamPalette());
      paletteBox->addPalette(newFramePalette());

      //-----------------------------------
      //    Symbols
      //-----------------------------------

      sp = new Palette;
      sp->setName(tr("Symbols"));
      sp->setGrid(42, 45);
      sp->setDrawGrid(true);

      sp->append(accDiscantSym);
      sp->append(accDotSym);
      sp->append(accFreebaseSym);
      sp->append(accStdbaseSym);
      sp->append(accBayanbaseSym);
      sp->append(accOldEESym);
      sp->append(accpushSym);
      sp->append(accpullSym);

      FretDiagram* fret = new FretDiagram(gscore);
      fret->setDot(5, 1);
      fret->setDot(2, 2);
      fret->setDot(1, 3);
      fret->setMarker(0, 'X');
      fret->setMarker(3, 'O');
      fret->setMarker(4, 'O');
      sp->append(fret, tr("Fret Diagram"));

      paletteBox->addPalette(sp);
      }

//---------------------------------------------------------
//   genCreateMenu
//---------------------------------------------------------

QMenu* MuseScore::genCreateMenu(QWidget* parent)
      {
      QMenu* popup = new QMenu(tr("&Create"), parent);
      popup->setObjectName("Create");

      popup->addAction(getAction("instruments"));

      QMenu* measures = popup->addMenu(tr("&Measures"));
      measures->addAction(getAction("insert-measure"));
      measures->addAction(getAction("insert-measures"));
      measures->addSeparator();
      measures->addAction(getAction("append-measure"));
      measures->addAction(getAction("append-measures"));

      QMenu* frames = popup->addMenu(tr("&Frames"));
      frames->addAction(getAction("insert-hbox"));
      frames->addAction(getAction("insert-vbox"));
      frames->addAction(getAction("insert-textframe"));
      frames->addAction(getAction("insert-fretframe"));
      frames->addSeparator();
      frames->addAction(getAction("append-hbox"));
      frames->addAction(getAction("append-vbox"));
      frames->addAction(getAction("append-textframe"));

      QMenu* text = popup->addMenu(tr("&Text"));
      text->addAction(getAction("title-text"));
      text->addAction(getAction("subtitle-text"));
      text->addAction(getAction("composer-text"));
      text->addAction(getAction("poet-text"));
      text->addSeparator();
      text->addAction(getAction("system-text"));
      text->addAction(getAction("staff-text"));
      text->addAction(getAction("chord-text"));
      text->addAction(getAction("rehearsalmark-text"));
      text->addSeparator();
      text->addAction(getAction("lyrics"));
      text->addAction(getAction("tempo"));

      return popup;
      }

//---------------------------------------------------------
//   addTempo
//---------------------------------------------------------

void MuseScore::addTempo()
      {
      ChordRest* cr = cs->getSelectedChordRest();
      if (!cr)
            return;
      if (editTempo == 0)
            editTempo = new EditTempo(0);
      int rv = editTempo->exec();
      if (rv == 1) {
            double bps = editTempo->bpm() / 60.0;
            TempoText* tt = new TempoText(cs);
            tt->setParent(cr->segment());
            tt->setTrack(cr->track());
            tt->setText(editTempo->text());
            tt->setTempo(bps);
            cs->undoAddElement(tt);
            cs->addRefresh(tt->abbox());  // ??
            }
      }

