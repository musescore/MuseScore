//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2009-2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

//
//    q+d hack to create an font description file from
//    lilipond tables embedded in MScore.otf
//
//

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H

#include <QMap>
#include <QString>
#include <QPointF>
#include <QRectF>
#include <QList>
#include <QJsonObject>
#include <QJsonValue>
#include <QFile>
#include <QStringList>
#include <QJsonDocument>

QMap<int, int> codemap;
QMap<QString, int> namemap;

//---------------------------------------------------------
//   nmap
//    mapping glyph names between emmentaler and smufl
//---------------------------------------------------------

QMap<QString, QString> nmap {
//    { "space",        "space"    },
//    { "plus",         "plus"     },
//    { "comma",        "comma"    },
//    { "hyphen",       "hyphen"   },
//    { "period",       "period"   },

      { "zero",         "timeSig0" },
      { "one",          "timeSig1" },
      { "two",          "timeSig2" },
      { "three",        "timeSig3" },
      { "four",         "timeSig4" },
      { "five",         "timeSig5" },
      { "six",          "timeSig6" },
      { "seven",        "timeSig7" },
      { "eight",        "timeSig8" },
      { "nine",         "timeSig9" },

      { "f",            "dynamicForte"       },
      { "m",            "dynamicMezzo"       },
      { "p",            "dynamicPiano"       },
      { "r",            "dynamicRinforzando" },
      { "s",            "dynamicSubito"      },
      { "z",            "dynamicZ"           },

      { "rests.0",          "restWhole" },
      { "rests.1",          "restHalf"  },

//    { "rests.0o", "rests.0o" },   // outside staff
//    { "rests.1o", "rests.1o" },

//    { "rests.M3", "rests.M3" },   // longer than longa

      { "rests.M2",                 "restLonga" },
      { "rests.M1",                 "restDoubleWhole" },
      { "rests.2",                  "restQuarter" },
      { "rests.2classical",         "restQuarterOld" },
      { "rests.3",                  "rest8th" },
      { "rests.4",                  "rest16th" },
      { "rests.5",                  "rest32nd" },
      { "rests.6",                  "rest64th" },
      { "rests.7",                  "rest128th" },
//      { "rests.8",                  "rest256th" },
//      { "rests.9",                  "rest512th" },
//      { "rests.10",                 "rest1024th" },

      { "accidentals.sharp",                 "accidentalSharp"         },
      { "accidentals.sharp.slashslash.stem", "accidentalQuarterSharp4" },

//    { "accidentals.sharp.slashslashslash.stemstem", "accidentals.sharp.slashslashslash.stemstem" },

//      { "accidentals.sharp.slashslashslash.stem", "accidentals.sharp.slashslashslash.stem" },
      { "accidentals.natural",      "accidentalNatural"     },
      { "accidentals.flat",         "accidentalFlat"        },
      { "accidentals.flatflat",     "accidentalDoubleFlat"  },
      { "accidentals.doublesharp",  "accidentalDoubleSharp" },

//      { "accidentals.sharp.slashslash.stemstemstem", "accidentals.sharp.slashslash.stemstemstem" },
//      { "accidentals.flat.slash", "accidentals.flat.slash" },
//      { "accidentals.flat.slashslash", "accidentals.flat.slashslash" },
//      { "accidentals.mirroredflat.flat", "accidentals.mirroredflat.flat" },
//      { "accidentals.mirroredflat", "accidentals.mirroredflat" },
//      { "accidentals.mirroredflat.backslash", "accidentals.mirroredflat.backslash" },
//      { "accidentals.flatflat.slash", "accidentals.flatflat.slash" },

//      { "accidentals.rightparen", "accidentals.rightparen" },
//      { "accidentals.leftparen", "accidentals.leftparen" },
//      { "arrowheads.open.01", "arrowheads.open.01" },
//      { "arrowheads.open.0M1", "arrowheads.open.0M1" },
//      { "arrowheads.open.11", "arrowheads.open.11" },
//      { "arrowheads.open.1M1", "arrowheads.open.1M1" },
//      { "arrowheads.close.01", "arrowheads.close.01" },
//      { "arrowheads.close.0M1", "arrowheads.close.0M1" },
//      { "arrowheads.close.11", "arrowheads.close.11" },
//      { "arrowheads.close.1M1", "arrowheads.close.1M1" },

      { "dots.dot",      "augmentationDot" },

//      { "noteheads.uM2", "noteheads.uM2" },
//      { "noteheads.dM2", "noteheads.dM2" },

      { "noteheads.sM1",            "noteheadDoubleWhole" },
      { "noteheads.s0",             "noteheadWhole" },
      { "noteheads.s1",             "noteheadHalf" },
      { "noteheads.s2",             "noteheadBlack" },

      { "noteheads.s0diamond",      "noteheadDiamondWhole" },
      { "noteheads.s1diamond",      "noteheadDiamondHalf" },
      { "noteheads.s2diamond",      "noteheadDiamondBlack" },

      { "noteheads.s0triangle",     "noteheadTriangleUpWhole" },
      { "noteheads.d1triangle",     "noteheadTriangleUpHalf" },
//      { "noteheads.u1triangle",     "" },
      { "noteheads.u2triangle",     "noteheadTriangleUpBlack" },
//      { "noteheads.d2triangle",     "notehead" },

      { "noteheads.s0slash",        "noteheadSlashWhite" },
      { "noteheads.s1slash",        "noteheadSlashWhite" },
      { "noteheads.s2slash",        "noteheadSlashHorizontalEnds" },

      { "noteheads.s0cross",        "noteheadXWhole" },
      { "noteheads.s1cross",        "noteheadXHalf" },
      { "noteheads.s2cross",        "noteheadXBlack" },

      { "noteheads.s2xcircle",      "noteheadCircleX" },
#if 0
      { "noteheads.s0do",           "notehead" },
      { "noteheads.d1do",           "notehead" },
      { "noteheads.u1do",           "notehead" },
      { "noteheads.d2do",           "notehead" },
      { "noteheads.u2do",           "notehead" },

      { "noteheads.s0re",           "notehead" },
      { "noteheads.u1re",           "notehead" },
      { "noteheads.d1re",           "notehead" },
      { "noteheads.u2re",           "notehead" },
      { "noteheads.d2re",           "notehead" },

      { "noteheads.s0mi",           "notehead" },
      { "noteheads.s1mi",           "notehead" },
      { "noteheads.s2mi",           "notehead" },

      { "noteheads.u0fa",           "notehead" },
      { "noteheads.d0fa",           "notehead" },
      { "noteheads.u1fa",           "notehead" },
      { "noteheads.d1fa",           "notehead" },
      { "noteheads.u2fa",           "notehead" },
      { "noteheads.d2fa",           "notehead" },

      { "noteheads.s0la",           "notehead" },
      { "noteheads.s1la",           "notehead" },
      { "noteheads.s2la",           "notehead" },

      { "noteheads.s0ti",           "notehead" },
      { "noteheads.u1ti",           "notehead" },
      { "noteheads.d1ti",           "notehead" },
      { "noteheads.u2ti",           "notehead" },
      { "noteheads.d2ti",           "notehead" },
#endif

      { "scripts.ufermata",         "fermataAbove" },
      { "scripts.dfermata",         "fermataBelow" },
      { "scripts.ushortfermata",    "fermataShortAbove" },
      { "scripts.dshortfermata",    "fermataShortBelow" },
      { "scripts.ulongfermata",     "fermataLongAbove" },
      { "scripts.dlongfermata",     "fermataLongBelow" },
      { "scripts.uverylongfermata", "fermataVeryLongAbove" },
      { "scripts.dverylongfermata", "fermataVeryLongBelow" },
      { "scripts.thumb",            "stringsThumbPosition" },
      { "scripts.sforzato",         "articAccent" },
//    { "scripts.espr",             "scripts.espr" },     "<>" ?
      { "scripts.staccato",         "articStaccato" },
      { "scripts.ustaccatissimo",   "articStaccatissimoAbove" },
      { "scripts.dstaccatissimo",   "articStaccatissimoBelow" },
      { "scripts.tenuto",           "articTenuto" },
      { "scripts.uportato",         "articTenuroSlurBelow" },     // spelling bug!
      { "scripts.dportato",         "articTenutoSlurAbove" },
      { "scripts.umarcato",         "articMarcatoAbove" },
      { "scripts.dmarcato",         "articMarcatoBelow" },
      { "scripts.fadein",           "guitarFadeIn" },
      { "scripts.fadeout",          "guitarFadeOut" },
      { "scripts.wigglesawtooth",            "volumeSwell" },
      { "scripts.wigglesawtoothwide",        "wiggleSawtooth" },
      { "scripts.wigglevibratolargefaster",  "wiggleSawtoothWide" },
      { "scripts.wigglevibratolargeslowest", "wiggleVibratoLargeFaster" },
      { "scripts.widelefthandvibrato",   "wiggleVibratoLargeSlowest" },
      { "scripts.open",             "brassMuteOpen" },
      { "scripts.stopped",          "brassMuteClosed" },
      { "scripts.upbow",            "stringsUpBow" },
      { "scripts.downbow",          "stringsDownBow" },
      { "scripts.reverseturn",      "ornamentTurnInverted" },
      { "scripts.turn",             "ornamentTurn" },
      { "scripts.trill",            "ornamentTrill" },
      { "scripts.verticalturn",     "ornamentTurnUp" },
      { "scripts.reverseverticalturn",       "ornamentTurnUpS" },
      { "scripts.upedalheel",       "keyboardPedalHeel1" },
      { "scripts.dpedalheel",       "keyboardPedalHeel2" },
      { "scripts.upedaltoe",        "keyboardPedalToe1" },
      { "scripts.dpedaltoe",        "keyboardPedalToe2" },
      { "scripts.flageolet",        "stringsHarmonic" },
      { "scripts.segno",            "segno" },
      { "scripts.coda",             "coda" },
      { "scripts.varcoda",          "codaSquare" },
      { "scripts.rcomma",           "breathMark" },
//    { "scripts.lcomma",           "scripts.lcomma" },
//    { "scripts.rvarcomma",        "scripts.rvarcomma" },
//    { "scripts.lvarcomma",        "scripts.lvarcomma" },

      { "scripts.arpeggio",         "wiggleArpeggiatoUp" },

      { "scripts.trill_element",    "wiggleTrill" },

//      { "scripts.arpeggio.arrow.M1", "scripts.arpeggio.arrow.M1" },
//      { "scripts.arpeggio.arrow.1",  "scripts.arpeggio.arrow.1" },
//      { "scripts.trilelement",       "scripts.trilelement" },
      { "scripts.prall",             "ornamentShortTrill" },
      { "scripts.mordent",           "ornamentMordent" },
      { "scripts.prallprall",        "ornamentTremblement" },
      { "scripts.prallmordent",      "ornamentPrallMordent" },

      { "scripts.upprall",           "ornamentUpPrall",   }, // "ornamentPrecompSlideTrillAnglebert"
      { "scripts.upmordent",         "ornamentUpMordent", },  // "ornamentPrecompSlideTrillBach"

      { "scripts.pralldown",         "ornamentPrallDown"   },
      { "scripts.downprall",         "ornamentDownPrall"   },
      { "scripts.downmordent",       "ornamentDownMordent" },
      { "scripts.prallup",           "ornamentPrallUp"     },
      { "scripts.lineprall",         "ornamentLinePrall"   },


//    { "scripts.caesura.curved",   "scripts.caesura.curved" },
      { "scripts.caesura.straight", "caesura" },

      { "flags.u3",                 "flag8thUp" },
      { "flags.u4",                 "flag16thUp" },
      { "flags.u5",                 "flag32ndUp" },
      { "flags.u6",                 "flag64thUp" },
//      { "flags.u7",                 "flag128thUp" },
//      { "flags.u8",                 "flag256thUp" },
//      { "flags.u9",                 "flag512thUp" },
//      { "flags.u10",                "flag1024thUp" },
      { "flags.d3",                 "flag8thDown" },
//    { "flags.ugrace", "flags.ugrace" },
//    { "flags.dgrace", "flags.dgrace" },
      { "flags.d4",                 "flag16thDown" },
      { "flags.d5",                 "flag32ndDown" },
      { "flags.d6",                 "flag64thDown" },
//      { "flags.d7",                 "flag128thDown" },
//      { "flags.d8",                 "flag256thDown" },
//      { "flags.d9",                 "flag512thDown" },
//      { "flags.d10",                "flag1024thDown" },

      { "clefs.C",                  "cClef" },
//    { "clefs.C_change", "clefs.C_change" },
      { "clefs.F", "fClef" },
//    { "clefs.F_change", "clefs.F_change" },
      { "clefs.G", "gClef" },
//    { "clefs.G_change", "clefs.G_change" },
      { "clefs.percussion", "unpitchedPercussionClef1" },
//    { "clefs.percussion_change", "clefs.percussion_change" },
      { "clefs.tab", "6stringTabClefSerif" },
//    { "clefs.tab_change", "clefs.tab_change" },
      { "timesig.C44", "timeSigCommon" },
      { "timesig.C22", "timeSigCutCommon" },

      { "pedal.*", "keyboardPedalUp" },
//    { "pedal.M", "pedal.M" },
//    { "pedal..", "pedal.." },
      { "pedal.P", "keyboardPedalP" },
//    { "pedal.d", "pedal.d" },
//    { "pedal.e", "pedal.e" },
      { "pedal.Ped", "keyboardPedalPed" },

      { "brackettips.uright", "bracketTop" },
      { "brackettips.dright", "bracketBottom" },

//      { "accordion.accDiscant",   "accordion.accDiscant" },
//      { "accordion.accDot",       "accordion.accDot" },
//      { "accordion.accFreebase",  "accordion.accFreebase" },
//      { "accordion.accStdbase",   "accordion.accStdbase" },
//      { "accordion.accBayanbase", "accordion.accBayanbase" },
//      { "accordion.accOldEE",     "accordion.accOldEE" },

      { "brackettips.uleft", "reversedBracketTop" },
      { "brackettips.dleft", "reversedBracketBottom" },

      { "flags.d7", "flag128thDown" },
      { "flags.u7", "flag128thUp"   },

      { "scripts.snappizzicato",          "pluckedSnapPizzicatoAbove" },
#if 0
      { "noteheads.sM1double",            "noteheads.sM1double" },
      { "accidentals.flat.arrowup",       "accidentals.flat.arrowup" },
      { "accidentals.flat.arrowdown",     "accidentals.flat.arrowdown" },
      { "accidentals.flat.arrowboth",     "accidentals.flat.arrowboth" },
      { "accidentals.natural.arrowup",    "accidentals.natural.arrowup" },
      { "accidentals.natural.arrowdown",  "accidentals.natural.arrowdown" },
      { "accidentals.natural.arrowboth",  "accidentals.natural.arrowboth" },
      { "accidentals.sharp.arrowup",      "accidentals.sharp.arrowup" },
      { "accidentals.sharp.arrowdown",    "accidentals.sharp.arrowdown" },
      { "accidentals.sharp.arrowboth",    "accidentals.sharp.arrowboth" },
      { "noteheads.uM2alt",               "noteheads.uM2alt" },
      { "noteheads.dM2alt",               "noteheads.dM2alt" },
      { "noteheads.sM1alt",               "noteheads.sM1alt" },
      { "timesig.Cdot",                   "timesig.Cdot" },
      { "timesig.O",                      "timesig.O" },
      { "timesig.Ocut",                   "timesig.Ocut" },
      { "timesig.Odot",                   "timesig.Odot" },
#endif

//    { "uniE1CB", "uniE1CB" },
//    { "uniE1CC", "uniE1CC" },
//    { "uniE1CD", "uniE1CD" },
//    { "uniE1CE", "uniE1CE" },
//    { "uniE1CF", "uniE1CF" },
//    { "uniE1D0", "uniE1D0" },
//    { "uniE1D1", "uniE1D1" },
//    { "uniE1D2", "uniE1D2" },
      { "uniE1D3", "ornamentPrecompSlide" },

      { "uniE1D6", "gClef15mb" },     // new glyphs for smufl compatibility
      { "uniE1D7", "gClef8vb"  },
      { "uniE1D8", "gClef8va"  },
      { "uniE1D9", "gClef15ma" },
      { "uniE1DA", "fClef15mb" },
      { "uniE1DB", "fClef8vb"  },
      { "uniE1DC", "fClef8va"  },
      { "uniE1DD", "fClef15ma" },

      { "accidentals.sori", "accidentalSori" },
      { "accidentals.koron", "accidentalKoron" }

      };

//---------------------------------------------------------
//   Glyph
//---------------------------------------------------------

struct Glyph {
      QString name;
      int code;
      QPointF attach;
      QRectF bbox;
      };

QList<Glyph> glyphs;

#if 0
//---------------------------------------------------------
//   getTable
//---------------------------------------------------------

static char* getTable(char* t, FT_Face face)
      {
      FT_ULong tag = FT_MAKE_TAG(t[0], t[1], t[2], t[3]);
      FT_ULong length = 0;
      int error = FT_Load_Sfnt_Table(face, tag, 0, NULL, &length);
      if (error) {
            qDebug("genft: cannot load table LILC");
            exit(-3);
            }
      FT_Byte* buffer = (FT_Byte*)malloc(length + 1);
      error = FT_Load_Sfnt_Table(face, tag, 0, buffer, &length);
      buffer[length] = 0;
      if (error) {
            qDebug("genft: cannot load font table LILC");
            exit(4);
            }
      return (char*)buffer;
      }

//---------------------------------------------------------
//   parseLILC
//    (rests.0 .
//    ((bbox . (-0.000000 -3.125000 7.500000 0.000000))
//    (subfont . "feta20")
//    (subfont-index . 33)
//    (attachment . (7.500000 . 0.000000))))
//---------------------------------------------------------

static void parseLILC(char* buffer)
      {
      QString s(buffer);
      QStringList sl = s.split("\n");
      QRegExp ra("\\(attachment\\s\\.\\s\\(([0-9\\+\\-\\.]{1,})\\s\\.\\s([0-9\\+\\-\\.]{1,})\\)\\)\\)\\)");
      QRegExp rb("\\(\\(bbox\\s\\.\\s\\(([0-9\\+\\-\\.]{1,})\\s([0-9\\+\\-\\.]{1,})\\s([0-9\\+\\-\\.]{1,})\\s([0-9\\+\\-\\.]{1,})\\)\\)");
      int n = sl.size();
      for (int i = 0; (i+4) < n; i += 5) {
            Glyph g;
            QString s = sl[i];
            int nn = s.size();
            s = s.mid(1, nn - 3);
            g.name = s;
            int idx = 0;
            if (namemap.contains(s))
                  idx = namemap[s];
            else
                  qDebug("genft: <%s> not in map", qPrintable(s));
            int code = 0;
            if (codemap.contains(idx))
                  code = codemap[idx];
            else
                  qDebug("codemap has no index %d", idx);
            g.code = code;

            s = sl[i+4];
            int val = ra.indexIn(s);
            if (val == -1 || ra.captureCount() != 2) {
                  qDebug("bad reg expr a");
                  exit(-5);
                  }
            g.attach.rx() = ra.cap(1).toDouble();
            g.attach.ry() = -ra.cap(2).toDouble();

            s = sl[i+1];
            val = rb.indexIn(s);
            if (val == -1 || rb.captureCount() != 4) {
                  qDebug("bad reg expr b");
                  exit(-5);
                  }
            double a = rb.cap(1).toDouble();
            double b = rb.cap(2).toDouble();
            double c = rb.cap(3).toDouble();
            double d = rb.cap(4).toDouble();
            g.bbox = QRectF(a, -d, c - a, d - b);
            glyphs.append(g);
            }
      }
#endif

//---------------------------------------------------------
//   genJson
//---------------------------------------------------------

static void genJson()
      {
      QJsonObject o;

      QMapIterator<QString, int> i(namemap);
      while (i.hasNext()) {
            i.next();

            if (!nmap.contains(i.key())) {
                  fprintf(stderr, "not found: <%s>\n", qPrintable(i.key()));
                  continue;
                  }

            int code = codemap[i.value()];

            QString smufl = nmap.value(i.key());

            QJsonObject jg;
            QString s = QString("U+%1").arg(code, 0, 16);
            jg.insert("codepoint", s);

            o.insert(smufl, jg);
            }
      QJsonDocument d(o);
      QByteArray ba = d.toJson();
      QFile f("glyphnames.json");
      f.open(QIODevice::WriteOnly);
      f.write(ba);
      f.close();
      }

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int, char* argv[])
      {
      FT_Library library;

      if (FT_Init_FreeType(&library)) {
            qDebug("init free type library failed");
            exit(-1);
            }
      FT_Face face;
      int error = FT_New_Face(library, argv[1], 0, &face);
      if (error) {
            qDebug("open font failed <%s>", argv[1]);
            exit(-2);
            }

      FT_Select_Charmap(face, FT_ENCODING_UNICODE);

      FT_ULong charcode;
      FT_UInt gindex;

      for (charcode = FT_Get_First_Char(face, &gindex); gindex;
         charcode = FT_Get_Next_Char(face, charcode, &gindex)) {
            char name[256];
            FT_Get_Glyph_Name(face, gindex, name, 256);
            codemap[gindex] = charcode;
            namemap[name] = gindex;
            }
//      char* p = getTable((char*)"LILC", face);
//      parseLILC(p);
      // p = getTable("LILY", face);      // global values, not used
      // p = getTable("LILF", face);      // subfont table, not used
      genJson();
      return 0;
      }

