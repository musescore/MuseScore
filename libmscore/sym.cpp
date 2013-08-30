//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "style.h"
#include "sym.h"
#include "utils.h"
#include "score.h"
#include "xml.h"
#include "mscore.h"

namespace Ms {

QVector<Sym> symbols[2];
static bool symbolsInitialized[2] = { false, false };

// static QReadWriteLock gLock;
QHash<QString, SymId> Sym::lnhash;
QVector<const char*> Sym::symNames;
QVector<QString> Sym::symUserNames;

//---------------------------------------------------------
//   SymbolNames
//---------------------------------------------------------

struct SymbolNames {
      SymId msIndex;
      const char* mname;      // user visible name
      const char* name;       // xml symbol name
      };

static SymbolNames lilypondNames[] = {
      { wholerestSym,         QT_TRANSLATE_NOOP("symbol", "whole rest"),           "rests.0" },
      { halfrestSym,          QT_TRANSLATE_NOOP("symbol", "half rest"),            "rests.1" },
      { outsidewholerestSym,  QT_TRANSLATE_NOOP("symbol", "outside whole rest"),   "rests.0o" },
      { outsidehalfrestSym,   QT_TRANSLATE_NOOP("symbol", "outside half rest"),    "rests.1o" },
      { rest_M3,              QT_TRANSLATE_NOOP("symbol", "rest M3"),              "rests.M3" },
      { breverestSym,         QT_TRANSLATE_NOOP("symbol", "breve rest"),           "rests.M1" },
      { longarestSym,         QT_TRANSLATE_NOOP("symbol", "longa rest"),           "rests.M2" },
      { rest4Sym,             QT_TRANSLATE_NOOP("symbol", "quart rest"),           "rests.2" },
      { clasquartrestSym,     QT_TRANSLATE_NOOP("symbol", "clas quart rest"),      "rests.2classical" },
      { rest8Sym,             QT_TRANSLATE_NOOP("symbol", "eight rest"),           "rests.3" },
      { rest16Sym,            QT_TRANSLATE_NOOP("symbol", "16' rest"),             "rests.4" },
      { rest32Sym,            QT_TRANSLATE_NOOP("symbol", "32' rest"),             "rests.5" },
      { rest64Sym,            QT_TRANSLATE_NOOP("symbol", "64' rest"),             "rests.6" },
      { rest128Sym,           QT_TRANSLATE_NOOP("symbol", "128' rest"),            "rests.7" },

      { sharpSym,             QT_TRANSLATE_NOOP("symbol", "sharp"),                "accidentals.sharp" },
      { sharpArrowUpSym,      QT_TRANSLATE_NOOP("symbol", "sharp arrow up"),       "accidentals.sharp.arrowup" },
      { sharpArrowDownSym,    QT_TRANSLATE_NOOP("symbol", "sharp arrow both"),     "accidentals.sharp.arrowdown" },
      { sharpArrowBothSym,    QT_TRANSLATE_NOOP("symbol", "sharp arrow both"),     "accidentals.sharp.arrowboth" },
      { sharpslashSym,        QT_TRANSLATE_NOOP("symbol", "sharp slash"),          "accidentals.sharp.slashslash.stem" },
      { sharpslash2Sym,       QT_TRANSLATE_NOOP("symbol", "sharp slash2"),         "accidentals.sharp.slashslashslash.stemstem" },
      { sharpslash3Sym,       QT_TRANSLATE_NOOP("symbol", "sharp slash3"),         "accidentals.sharp.slashslashslash.stem" },
      { sharpslash4Sym,       QT_TRANSLATE_NOOP("symbol", "sharp slash4"),         "accidentals.sharp.slashslash.stemstemstem" },
      { naturalSym,           QT_TRANSLATE_NOOP("symbol", "natural"),              "accidentals.natural" },
      { naturalArrowUpSym,    QT_TRANSLATE_NOOP("symbol", "natural arrow up"),     "accidentals.natural.arrowup" },
      { naturalArrowDownSym,  QT_TRANSLATE_NOOP("symbol", "natural arrow down"),   "accidentals.natural.arrowdown" },
      { naturalArrowBothSym,  QT_TRANSLATE_NOOP("symbol", "natural arrow both"),   "accidentals.natural.arrowboth" },
      { flatSym,              QT_TRANSLATE_NOOP("symbol", "flat"),                 "accidentals.flat" },
      { flatArrowUpSym,       QT_TRANSLATE_NOOP("symbol", "flat arrow up"),        "accidentals.flat.arrowup" },
      { flatArrowDownSym,     QT_TRANSLATE_NOOP("symbol", "flat arrow both"),      "accidentals.flat.arrowdown" },
      { flatArrowBothSym,     QT_TRANSLATE_NOOP("symbol", "flat arrow both"),      "accidentals.flat.arrowboth" },
      { flatslashSym,         QT_TRANSLATE_NOOP("symbol", "flat slash"),           "accidentals.flat.slash" },
      { flatslash2Sym,        QT_TRANSLATE_NOOP("symbol", "flat slash2"),          "accidentals.flat.slashslash" },
      { mirroredflat2Sym,     QT_TRANSLATE_NOOP("symbol", "mirrored flat2"),       "accidentals.mirroredflat.flat" },
      { mirroredflatSym,      QT_TRANSLATE_NOOP("symbol", "mirrored flat"),        "accidentals.mirroredflat" },
      { mirroredflatslashSym, QT_TRANSLATE_NOOP("symbol", "mirrored flat slash"),  "accidentals.mirroredflat.backslash" },
      { flatflatSym,          QT_TRANSLATE_NOOP("symbol", "flat flat"),            "accidentals.flatflat" },
      { flatflatslashSym,     QT_TRANSLATE_NOOP("symbol", "flat flat slash"),      "accidentals.flatflat.slash" },
      { sharpsharpSym,        QT_TRANSLATE_NOOP("symbol", "sharp sharp"),          "accidentals.doublesharp" },
      { soriSym,              QT_TRANSLATE_NOOP("symbol", "sori"),                 "accidentals.sori" },
      { koronSym,             QT_TRANSLATE_NOOP("symbol", "koron"),                "accidentals.koron" },

      { rightparenSym,        QT_TRANSLATE_NOOP("symbol", "right parenthesis"),    "accidentals.rightparen" },
      { leftparenSym,         QT_TRANSLATE_NOOP("symbol", "left parenthesis"),     "accidentals.leftparen"  },

      { open01arrowHeadSym,   QT_TRANSLATE_NOOP("symbol", "open arrowhead 01"),    "arrowheads.open.01"   },
      { open0M1arrowHeadSym,  QT_TRANSLATE_NOOP("symbol", "open arrowhead 0M1"),   "arrowheads.open.0M1"  },
      { open11arrowHeadSym,   QT_TRANSLATE_NOOP("symbol", "open arrowhead 11"),    "arrowheads.open.11"   },
      { open1M1arrowHeadSym,  QT_TRANSLATE_NOOP("symbol", "open arrowhead 1M1"),   "arrowheads.open.1M1"  },
      { close01arrowHeadSym,  QT_TRANSLATE_NOOP("symbol", "close arrowhead 01"),   "arrowheads.close.01"  },
      { close0M1arrowHeadSym, QT_TRANSLATE_NOOP("symbol", "close arrowhead 0M1"),  "arrowheads.close.0M1" },
      { close11arrowHeadSym,  QT_TRANSLATE_NOOP("symbol", "close arrowhead 11"),   "arrowheads.close.11"  },
      { close1M1arrowHeadSym, QT_TRANSLATE_NOOP("symbol", "close arrowhead 1M1"),  "arrowheads.close.1M1" },

      { dotSym,               QT_TRANSLATE_NOOP("symbol", "dot"),                  "dots.dot"      },
      { longaupSym,           QT_TRANSLATE_NOOP("symbol", "longa up"),             "noteheads.uM2" },
      { longadownSym,         QT_TRANSLATE_NOOP("symbol", "longa down"),           "noteheads.dM2" },
      { brevisheadSym,        QT_TRANSLATE_NOOP("symbol", "brevis head"),          "noteheads.sM1" },
      { brevisdoubleheadSym,  QT_TRANSLATE_NOOP("symbol", "brevis double head"),   "noteheads.sM1double" },
      { wholeheadSym,         QT_TRANSLATE_NOOP("symbol", "whole head"),           "noteheads.s0" },
      { halfheadSym,          QT_TRANSLATE_NOOP("symbol", "half head"),            "noteheads.s1" },
      { quartheadSym,         QT_TRANSLATE_NOOP("symbol", "quart head"),           "noteheads.s2" },
      { wholediamondheadSym,  QT_TRANSLATE_NOOP("symbol", "whole diamond head"),   "noteheads.s0diamond" },
      { halfdiamondheadSym,   QT_TRANSLATE_NOOP("symbol", "half diamond head"),    "noteheads.s1diamond" },
      { diamondheadSym,       QT_TRANSLATE_NOOP("symbol", "diamond head"),         "noteheads.s2diamond" },
      { s0triangleHeadSym,    QT_TRANSLATE_NOOP("symbol", "whole triangle head"),  "noteheads.s0triangle" },
      { d1triangleHeadSym,    QT_TRANSLATE_NOOP("symbol", "down half triangle head"), "noteheads.d1triangle" },
      { u1triangleHeadSym,    QT_TRANSLATE_NOOP("symbol", "up half triangle head"), "noteheads.u1triangle" },
      { u2triangleHeadSym,    QT_TRANSLATE_NOOP("symbol", "up quart triangle head"), "noteheads.u2triangle" },
      { d2triangleHeadSym,    QT_TRANSLATE_NOOP("symbol", "down quart triangle head"), "noteheads.d2triangle" },
      { wholeslashheadSym,    QT_TRANSLATE_NOOP("symbol", "whole slash head"),     "noteheads.s0slash" },
      { halfslashheadSym,     QT_TRANSLATE_NOOP("symbol", "half slash head"),      "noteheads.s1slash" },
      { quartslashheadSym,    QT_TRANSLATE_NOOP("symbol", "quart slash head"),     "noteheads.s2slash" },
      { wholecrossedheadSym,  QT_TRANSLATE_NOOP("symbol", "whole cross head"),     "noteheads.s0cross" },
      { halfcrossedheadSym,   QT_TRANSLATE_NOOP("symbol", "half cross head"),      "noteheads.s1cross" },
      { crossedheadSym,       QT_TRANSLATE_NOOP("symbol", "cross head"),           "noteheads.s2cross" },
      { xcircledheadSym,      QT_TRANSLATE_NOOP("symbol", "x circle head"),        "noteheads.s2xcircle" },
      { s0doHeadSym,          QT_TRANSLATE_NOOP("symbol", "s0do head"),            "noteheads.s0do" },
      { d1doHeadSym,          QT_TRANSLATE_NOOP("symbol", "d1do head"),            "noteheads.d1do" },
      { u1doHeadSym,          QT_TRANSLATE_NOOP("symbol", "u1do head"),            "noteheads.u1do" },
      { d2doHeadSym,          QT_TRANSLATE_NOOP("symbol", "d2do head"),            "noteheads.d2do" },
      { u2doHeadSym,          QT_TRANSLATE_NOOP("symbol", "u2do head"),            "noteheads.u2do" },
      { s0reHeadSym,          QT_TRANSLATE_NOOP("symbol", "s0re head"),            "noteheads.s0re" },
      { u1reHeadSym,          QT_TRANSLATE_NOOP("symbol", "u1re head"),            "noteheads.u1re" },
      { d1reHeadSym,          QT_TRANSLATE_NOOP("symbol", "d1re head"),            "noteheads.d1re" },
      { u2reHeadSym,          QT_TRANSLATE_NOOP("symbol", "u2re head"),            "noteheads.u2re" },
      { d2reHeadSym,          QT_TRANSLATE_NOOP("symbol", "d2re head"),            "noteheads.d2re" },
      { s0miHeadSym,          QT_TRANSLATE_NOOP("symbol", "s0mi head"),            "noteheads.s0mi" },
      { s1miHeadSym,          QT_TRANSLATE_NOOP("symbol", "s1mi head"),            "noteheads.s1mi" },
      { s2miHeadSym,          QT_TRANSLATE_NOOP("symbol", "s2mi head"),            "noteheads.s2mi" },
      { u0faHeadSym,          QT_TRANSLATE_NOOP("symbol", "u0fa head"),            "noteheads.u0fa" },
      { d0faHeadSym,          QT_TRANSLATE_NOOP("symbol", "d0fa head"),            "noteheads.d0fa" },
      { u1faHeadSym,          QT_TRANSLATE_NOOP("symbol", "u1fa head"),            "noteheads.u1fa" },
      { d1faHeadSym,          QT_TRANSLATE_NOOP("symbol", "d1fa head"),            "noteheads.d1fa" },
      { u2faHeadSym,          QT_TRANSLATE_NOOP("symbol", "u2fa head"),            "noteheads.u2fa" },
      { d2faHeadSym,          QT_TRANSLATE_NOOP("symbol", "d2fa head"),            "noteheads.d2fa" },
      { s0laHeadSym,          QT_TRANSLATE_NOOP("symbol", "s0la head"),            "noteheads.s0la" },
      { s1laHeadSym,          QT_TRANSLATE_NOOP("symbol", "s1la head"),            "noteheads.s1la" },
      { s2laHeadSym,          QT_TRANSLATE_NOOP("symbol", "s2la head"),            "noteheads.s2la" },
      { s0tiHeadSym,          QT_TRANSLATE_NOOP("symbol", "s0ti head"),            "noteheads.s0ti" },
      { u1tiHeadSym,          QT_TRANSLATE_NOOP("symbol", "u1ti head"),            "noteheads.u1ti" },
      { d1tiHeadSym,          QT_TRANSLATE_NOOP("symbol", "d1ti head"),            "noteheads.d1ti" },
      { u2tiHeadSym,          QT_TRANSLATE_NOOP("symbol", "u2ti head"),            "noteheads.u2ti" },
      { d2tiHeadSym,          QT_TRANSLATE_NOOP("symbol", "d2ti head"),            "noteheads.d2ti" },

      { s0solHeadSym,         QT_TRANSLATE_NOOP("symbol", "s0sol head"),           "noteheads.s0sol" },
      { s1solHeadSym,         QT_TRANSLATE_NOOP("symbol", "s1sol head"),           "noteheads.s1sol" },
      { s2solHeadSym,         QT_TRANSLATE_NOOP("symbol", "s2sol head"),           "noteheads.s2sol" },

      { ufermataSym,          QT_TRANSLATE_NOOP("symbol", "ufermata"),             "scripts.ufermata" },
      { dfermataSym,          QT_TRANSLATE_NOOP("symbol", "dfermata"),             "scripts.dfermata" },
      { snappizzicatoSym,     QT_TRANSLATE_NOOP("symbol", "snappizzicato"),        "scripts.snappizzicato" },
      { ushortfermataSym,     QT_TRANSLATE_NOOP("symbol", "ushortfermata"),        "scripts.ushortfermata" },
      { dshortfermataSym,     QT_TRANSLATE_NOOP("symbol", "dshortfermata"),        "scripts.dshortfermata" },
      { ulongfermataSym,      QT_TRANSLATE_NOOP("symbol", "ulongfermata"),         "scripts.ulongfermata" },
      { dlongfermataSym,      QT_TRANSLATE_NOOP("symbol", "dlongfermata"),         "scripts.dlongfermata" },
      { uverylongfermataSym,  QT_TRANSLATE_NOOP("symbol", "uverylongfermata"),     "scripts.uverylongfermata" },
      { dverylongfermataSym,  QT_TRANSLATE_NOOP("symbol", "dverylongfermata"),     "scripts.dverylongfermata" },
      { thumbSym,             QT_TRANSLATE_NOOP("symbol", "thumb"),                "scripts.thumb" },
      { sforzatoaccentSym,    QT_TRANSLATE_NOOP("symbol", "sforza to accent"),     "scripts.sforzato" },
      { esprSym,              QT_TRANSLATE_NOOP("symbol", "espressivo"),           "scripts.espr" },
      { staccatoSym,          QT_TRANSLATE_NOOP("symbol", "staccato"),             "scripts.staccato" },
      { ustaccatissimoSym,    QT_TRANSLATE_NOOP("symbol", "ustaccatissimo"),       "scripts.ustaccatissimo" },
      { dstaccatissimoSym,    QT_TRANSLATE_NOOP("symbol", "dstaccatissimo"),       "scripts.dstaccatissimo" },
      { tenutoSym,            QT_TRANSLATE_NOOP("symbol", "tenuto"),               "scripts.tenuto" },
      { uportatoSym,          QT_TRANSLATE_NOOP("symbol", "uportato"),             "scripts.uportato" },
      { dportatoSym,          QT_TRANSLATE_NOOP("symbol", "dportato"),             "scripts.dportato" },
      { umarcatoSym,          QT_TRANSLATE_NOOP("symbol", "umarcato"),             "scripts.umarcato" },
      { dmarcatoSym,          QT_TRANSLATE_NOOP("symbol", "dmarcato"),             "scripts.dmarcato" },
      { ouvertSym,            QT_TRANSLATE_NOOP("symbol", "ouvert"),               "scripts.open" },
      { halfopenSym,          QT_TRANSLATE_NOOP("symbol", "halfopen"),             "scripts.halfopen" },
      { plusstopSym,          QT_TRANSLATE_NOOP("symbol", "plus stop"),            "scripts.stopped" },
      { upbowSym,             QT_TRANSLATE_NOOP("symbol", "up bow"),               "scripts.upbow" },
      { downbowSym,           QT_TRANSLATE_NOOP("symbol", "down bow"),             "scripts.downbow" },
      { reverseturnSym,       QT_TRANSLATE_NOOP("symbol", "reverse turn"),         "scripts.reverseturn" },
      { turnSym,              QT_TRANSLATE_NOOP("symbol", "turn"),                 "scripts.turn"        },
      { trillSym,             QT_TRANSLATE_NOOP("symbol", "trill"),                "scripts.trill"       },
      { upedalheelSym,        QT_TRANSLATE_NOOP("symbol", "upedal heel"),          "scripts.upedalheel"  },
      { dpedalheelSym,        QT_TRANSLATE_NOOP("symbol", "dpedalheel"),           "scripts.dpedalheel"  },
      { upedaltoeSym,         QT_TRANSLATE_NOOP("symbol", "upedal toe"),           "scripts.upedaltoe"   },
      { dpedaltoeSym,         QT_TRANSLATE_NOOP("symbol", "dpedal toe"),           "scripts.dpedaltoe"   },
      { flageoletSym,         QT_TRANSLATE_NOOP("symbol", "flageolet"),            "scripts.flageolet"   },
      { segnoSym,             QT_TRANSLATE_NOOP("symbol", "segno"),                "scripts.segno"       },
      { varsegnoSym,          QT_TRANSLATE_NOOP("symbol", "var segno"),            "scripts.varsegno"    },
      { codaSym,              QT_TRANSLATE_NOOP("symbol", "coda"),                 "scripts.coda"        },
      { varcodaSym,           QT_TRANSLATE_NOOP("symbol", "varied coda"),          "scripts.varcoda"     },
      { rcommaSym,            QT_TRANSLATE_NOOP("symbol", "rcomma"),               "scripts.rcomma"      },
      { lcommaSym,            QT_TRANSLATE_NOOP("symbol", "lcomma"),               "scripts.lcomma"      },
      { rvarcommaSym,         QT_TRANSLATE_NOOP("symbol", "rcomma variation"),     "scripts.rvarcomma" },
      { lvarcommaSym,         QT_TRANSLATE_NOOP("symbol", "lcomma variation"),     "scripts.lvarcomma" },
      { arpeggioSym,          QT_TRANSLATE_NOOP("symbol", "arpeggio"),             "scripts.arpeggio" },
      { trillelementSym,      QT_TRANSLATE_NOOP("symbol", "trillelement"),         "scripts.trill_element" },
      { arpeggioarrowdownSym, QT_TRANSLATE_NOOP("symbol", "arpeggio arrow down"),  "scripts.arpeggio.arrow.M1" },
      { arpeggioarrowupSym,   QT_TRANSLATE_NOOP("symbol", "arpeggio arrow up"),    "scripts.arpeggio.arrow.1" },
      { trilelementSym,       QT_TRANSLATE_NOOP("symbol", "trill element"),        "scripts.trilelement" },
      { prallSym,             QT_TRANSLATE_NOOP("symbol", "prall"),                "scripts.prall" },
      { mordentSym,           QT_TRANSLATE_NOOP("symbol", "mordent"),              "scripts.mordent" },
      { prallprallSym,        QT_TRANSLATE_NOOP("symbol", "prall prall"),          "scripts.prallprall" },
      { prallmordentSym,      QT_TRANSLATE_NOOP("symbol", "prall mordent"),        "scripts.prallmordent" },
      { upprallSym,           QT_TRANSLATE_NOOP("symbol", "up prall"),             "scripts.upprall" },
      { upmordentSym,         QT_TRANSLATE_NOOP("symbol", "up mordent"),           "scripts.upmordent" },
      { pralldownSym,         QT_TRANSLATE_NOOP("symbol", "prall down"),           "scripts.pralldown" },
      { downprallSym,         QT_TRANSLATE_NOOP("symbol", "down prall"),           "scripts.downprall" },
      { downmordentSym,       QT_TRANSLATE_NOOP("symbol", "down mordent"),         "scripts.downmordent" },
      { prallupSym,           QT_TRANSLATE_NOOP("symbol", "prall up"),             "scripts.prallup" },
      { schleiferSym,         QT_TRANSLATE_NOOP("symbol", "schleifer"),            "scripts.schleifer" },

      { lineprallSym,         QT_TRANSLATE_NOOP("symbol", "line prall"),           "scripts.lineprall" },
      { caesuraCurvedSym,     QT_TRANSLATE_NOOP("symbol", "caesura curved"),       "scripts.caesura.curved" },
      { caesuraStraight,      QT_TRANSLATE_NOOP("symbol", "caesura straight"),     "scripts.caesura.straight" },
      { eighthflagSym,        QT_TRANSLATE_NOOP("symbol", "eight flag"),           "flags.u3" },
      { sixteenthflagSym,     QT_TRANSLATE_NOOP("symbol", "sixteenth flag"),       "flags.u4" },
      { thirtysecondflagSym,  QT_TRANSLATE_NOOP("symbol", "thirtysecond flag"),    "flags.u5" },
      { sixtyfourthflagSym,   QT_TRANSLATE_NOOP("symbol", "sixtyfour flag"),       "flags.u6" },
      { flag128Sym,           QT_TRANSLATE_NOOP("symbol", "128flag"),              "flags.u7" },
      { deighthflagSym,       QT_TRANSLATE_NOOP("symbol", "deight flag"),          "flags.d3" },
      { gracedashSym,         QT_TRANSLATE_NOOP("symbol", "grace dash"),           "flags.ugrace" },
      { dgracedashSym,        QT_TRANSLATE_NOOP("symbol", "dgrace dash"),          "flags.dgrace" },
      { dsixteenthflagSym,    QT_TRANSLATE_NOOP("symbol", "dsixteenth flag"),      "flags.d4" },
      { dthirtysecondflagSym, QT_TRANSLATE_NOOP("symbol", "dthirtysecond flag"),   "flags.d5" },
      { dsixtyfourthflagSym,  QT_TRANSLATE_NOOP("symbol", "dsixtyfourth flag"),    "flags.d6" },
      { dflag128Sym,          QT_TRANSLATE_NOOP("symbol", "d128flag"),             "flags.d7" },
      { altoclefSym,          QT_TRANSLATE_NOOP("symbol", "alto clef"),            "clefs.C" },
      { caltoclefSym,         QT_TRANSLATE_NOOP("symbol", "calto clef"),           "clefs.C_change" },
      { bassclefSym,          QT_TRANSLATE_NOOP("symbol", "bass clef"),            "clefs.F" },
      { cbassclefSym,         QT_TRANSLATE_NOOP("symbol", "cbass clef"),           "clefs.F_change" },
      { trebleclefSym,        QT_TRANSLATE_NOOP("symbol", "trebleclef"),           "clefs.G" },
      { ctrebleclefSym,       QT_TRANSLATE_NOOP("symbol", "ctrebleclef"),          "clefs.G_change" },
      { percussionclefSym,    QT_TRANSLATE_NOOP("symbol", "percussion clef"),      "clefs.percussion" },
      { cpercussionclefSym,   QT_TRANSLATE_NOOP("symbol", "cpercussion clef"),     "clefs.percussion_change" },
      { tabclefSym,           QT_TRANSLATE_NOOP("symbol", "tab clef"),             "clefs.tab"        },
      { ctabclefSym,          QT_TRANSLATE_NOOP("symbol", "ctab clef"),            "clefs.tab_change" },
      { fourfourmeterSym,     QT_TRANSLATE_NOOP("symbol", "four four meter"),      "timesig.C44"      },
      { allabreveSym,         QT_TRANSLATE_NOOP("symbol", "allabreve"),            "timesig.C22"      },
      { pedalasteriskSym,     QT_TRANSLATE_NOOP("symbol", "pedalasterisk"),        "pedal.*" },
      { pedaldashSym,         QT_TRANSLATE_NOOP("symbol", "pedaldash"),            "pedal.M" },
      { pedaldotSym,          QT_TRANSLATE_NOOP("symbol", "pedaldot"),             "pedal.." },
      { pedalPSym,            QT_TRANSLATE_NOOP("symbol", "pedalP"),               "pedal.P" },
      { pedaldSym,            QT_TRANSLATE_NOOP("symbol", "pedald"),               "pedal.d" },
      { pedaleSym,            QT_TRANSLATE_NOOP("symbol", "pedale"),               "pedal.e" },
      { pedalPedSym,          QT_TRANSLATE_NOOP("symbol", "pedal ped"),            "pedal.Ped" },
      { brackettipsRightUp,   QT_TRANSLATE_NOOP("symbol", "bracket tips up"),      "brackettips.uright"     },
      { brackettipsRightDown, QT_TRANSLATE_NOOP("symbol", "bracket tips down"),    "brackettips.dright"     },
      { brackettipsLeftUp,    QT_TRANSLATE_NOOP("symbol", "bracket tips left up"), "brackettips.uleft"      },
      { brackettipsLeftDown,  QT_TRANSLATE_NOOP("symbol", "bracket tips left down"), "brackettips.dleft"    },
      { accDotSym,            QT_TRANSLATE_NOOP("symbol", "accordion dot"),        "accordion.accDot"       },
      { accFreebaseSym,       QT_TRANSLATE_NOOP("symbol", "accordion freebase"),   "accordion.accFreebase"  },
      { accStdbaseSym,        QT_TRANSLATE_NOOP("symbol", "accordion stdbase"),    "accordion.accStdbase"   },
      { accBayanbaseSym,      QT_TRANSLATE_NOOP("symbol", "accordion bayanbase"),  "accordion.accBayanbase" },
      { accOldEESym,          QT_TRANSLATE_NOOP("symbol", "accordion old ee"),     "accordion.accOldEE"     },
      { accDiscantSym,        QT_TRANSLATE_NOOP("symbol", "accordion discant"),    "accordion.accDiscant"   },
      { accpushSym,           QT_TRANSLATE_NOOP("symbol", "accordion push"),       "accordion.push"         },
      { accpullSym,           QT_TRANSLATE_NOOP("symbol", "accordion pull"),       "accordion.pull"         },
      { plusSym,              QT_TRANSLATE_NOOP("symbol", "plus"),                 "plus"                  },
      { commaSym,             QT_TRANSLATE_NOOP("symbol", "comma"),                "comma"                 },
      { hyphenSym,            QT_TRANSLATE_NOOP("symbol", "hyphen"),               "hyphen"                },
      { periodSym,            QT_TRANSLATE_NOOP("symblo", "period"),               "period"                },
      { zeroSym,              QT_TRANSLATE_NOOP("symbol", "zero"),                 "zero" },
      { oneSym,               QT_TRANSLATE_NOOP("symbol", "one"),                  "one" },
      { twoSym,               QT_TRANSLATE_NOOP("symbol", "two"),                  "two" },
      { threeSym,             QT_TRANSLATE_NOOP("symbol", "three"),                "three" },
      { fourSym,              QT_TRANSLATE_NOOP("symbol", "four"),                 "four" },
      { fiveSym,              QT_TRANSLATE_NOOP("symbol", "five"),                 "five" },
      { sixSym,               QT_TRANSLATE_NOOP("symbol", "six"),                  "six" },
      { sevenSym,             QT_TRANSLATE_NOOP("symbol", "seven"),                "seven" },
      { eightSym,             QT_TRANSLATE_NOOP("symbol", "eight"),                "eight" },
      { nineSym,              QT_TRANSLATE_NOOP("symbol", "nine"),                 "nine" },
      { plusSym,              QT_TRANSLATE_NOOP("symbol", "plus"),                 "plus" },
      { spaceSym,             QT_TRANSLATE_NOOP("symbol", "space"),                "space" },
      { letterzSym,           QT_TRANSLATE_NOOP("symbol", "z"),                    "z" },
      { letterfSym,           QT_TRANSLATE_NOOP("symbol", "f"),                    "f" },
      { lettersSym,           QT_TRANSLATE_NOOP("symbol", "s"),                    "s" },
      { letterpSym,           QT_TRANSLATE_NOOP("symbol", "p"),                    "p" },
      { lettermSym,           QT_TRANSLATE_NOOP("symbol", "m"),                    "m" },
      { letterrSym,           QT_TRANSLATE_NOOP("symbol", "r"),                    "r" },
      { longaupaltSym,        QT_TRANSLATE_NOOP("symbol", "longa up alt"),         "noteheads.uM2alt" },
      { longadownaltSym,      QT_TRANSLATE_NOOP("symbol", "longa down alt"),       "noteheads.dM2alt" },
      { brevisheadaltSym,     QT_TRANSLATE_NOOP("symbol", "brevis head alt"),      "noteheads.sM1alt" },
      { timesigcdotSym,       QT_TRANSLATE_NOOP("symbol", "time sig C dot"),       "timesig.Cdot" },
      { timesigoSym,          QT_TRANSLATE_NOOP("symbol", "time sig O"),           "timesig.O" },
      { timesigocutSym,       QT_TRANSLATE_NOOP("symbol", "time sig O cut"),       "timesig.Ocut" },
      { timesigodotSym,       QT_TRANSLATE_NOOP("symbol", "time sig O dot"),       "timesig.Odot" },
      { tabclef2Sym,          QT_TRANSLATE_NOOP("symbol", "tab2 clef"),            "clefs.tab2" },
      };

//---------------------------------------------------------
//   symIdx2fontId
//---------------------------------------------------------

int symIdx2fontId(int symIdx)
      {
      return symIdx == 0 ? 0 : 3;
      }

//---------------------------------------------------------
//   fontId2font
//---------------------------------------------------------

QFont fontId2font(int fontId)
      {
      static QFont* fonts[5];       // cached values
      Q_ASSERT(fontId >= 0 && fontId < 5);

      QFont* f = fonts[fontId];
      if (f == 0) {
            f = fonts[fontId] = new QFont();
            f->setWeight(QFont::Normal);  // if not set we get system default
            f->setItalic(false);
#ifdef USE_GLYPHS
            qreal size = 20.0;
#else
            qreal size = 20.0 * MScore::DPI / PPI;
#endif
            if (fontId == 0)
                  f->setFamily("MScore");
            else if (fontId == 2) {
                  f->setFamily("FreeSerif");
                  size = 8.0; //  * MScore::DPI / PPI;
#ifndef USE_GLYPHS
                  size = size * MScore::DPI / PPI;
#endif
                  }
            else if (fontId == 3)
                  f->setFamily("Gonville-20");
            else if (fontId == 4)
                  f->setFamily("Bravura");
            else
                  qFatal("illegal font id %d", fontId);

            f->setStyleStrategy(QFont::NoFontMerging);

            // horizontal hinting is bad as note hooks do not attach to stems
            // properly at some magnifications
            f->setHintingPreference(QFont::PreferVerticalHinting);

#ifdef USE_GLYPHS
            f->setPixelSize(lrint(size));
            // f->setPointSizeF(size);
#else
            f->setPixelSize(lrint(size));
#endif
            }
      return *f;
      }

//---------------------------------------------------------
//   fontId2RawFont
//---------------------------------------------------------

QRawFont fontId2RawFont(int fontId)
      {
      static QRawFont* fonts[5];       // cached values
      Q_ASSERT(fontId >= 0 && fontId < 5);

      QRawFont* f = fonts[fontId];
      if (f == 0) {
            qreal size = 20.0 * MScore::DPI / PPI;
            QString name;

            if (fontId == 0)
                  name = ":/fonts/mscore-20.ttf";
            else if (fontId == 2) {
                  name = ":/fonts/FreeSerifMscore.ttf";
                  size = 8 * MScore::DPI / PPI;
                  }
            else if (fontId == 3)
                  name = ":/fonts/gonville-20.ttf";
            else if (fontId == 4)
                  name = ":/fonts/Bravura.otf";
            else
                  qFatal("illegal font id %d", fontId);

            // horizontal hinting is bad as note hooks do not attach to stems
            // properly at some magnifications
            f = fonts[fontId] = new QRawFont(name, size, QFont::PreferVerticalHinting);
            f->setPixelSize(size);
            }
      return *f;
      }

#ifdef USE_GLYPHS
//---------------------------------------------------------
//   genGlyphs
//---------------------------------------------------------

void Sym::genGlyphs(const QRawFont& rfont)
      {
      QVector<quint32> idx = rfont.glyphIndexesForString(toString());
      QVector<QPointF> adv;
      adv << QPointF();
      glyphs.setGlyphIndexes(idx);
      glyphs.setPositions(adv);
      glyphs.setRawFont(rfont);
      }
#endif

//---------------------------------------------------------
//   Sym
//---------------------------------------------------------

Sym::Sym(int c, int fid, qreal ax, qreal ay)
   : _code(c), fontId(fid), _attach(ax * MScore::DPI/PPI, ay * MScore::DPI/PPI)
      {
      QFont _font(fontId2font(fontId));
      QFontMetricsF fm(_font);
      if (!fm.inFont(_code)) {
            qDebug("Sym: character 0x%x(%d) are not in font <%s>", c, c, qPrintable(_font.family()));
            return;
            }
      w     = fm.width(_code);
      _bbox = fm.boundingRect(_code);
#ifdef USE_GLYPHS
      genGlyphs(fontId2RawFont(fontId));
#endif
      }

Sym::Sym(int c, int fid, const QPointF& a, const QRectF& b)
   : _code(c), fontId(fid)
      {
      qreal ds = MScore::DPI/PPI;
      _bbox.setRect(b.x() * ds, b.y() * ds, b.width() * ds, b.height() * ds);
      _attach = a * ds;
      w = _bbox.width();
#ifdef USE_GLYPHS
      genGlyphs(fontId2RawFont(fontId));
#endif
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

const QRectF Sym::bbox(qreal mag) const
      {
      return QRectF(_bbox.x() * mag, _bbox.y() * mag, _bbox.width() * mag, _bbox.height() * mag);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(QPainter* painter, qreal mag, const QPointF& pos) const
      {
      qreal imag = 1.0 / mag;
      painter->scale(mag, mag);
#ifdef USE_GLYPHS
      painter->drawGlyphRun(pos * imag, glyphs);
#else
      painter->setFont(font());
      painter->drawText(pos * imag, toString());
#endif
      painter->scale(imag, imag);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(QPainter* painter, qreal mag) const
      {
      qreal imag = 1.0 / mag;
      painter->scale(mag, mag);
#ifdef USE_GLYPHS
      painter->drawGlyphRun(QPointF(), glyphs);
#else
      painter->setFont(font());
      painter->drawText(QPointF(), toString());
#endif
      painter->scale(imag, imag);
      }

//---------------------------------------------------------
//   toString
//---------------------------------------------------------

QString Sym::toString() const
      {
      QString s;
      if (_code & 0xffff0000) {
            s = QChar(QChar::highSurrogate(_code));
            s += QChar(QChar::lowSurrogate(_code));
            }
      else
            s = QChar(_code);
      return s;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(QPainter* painter, qreal mag, const QPointF& pos, int n) const
      {
#ifdef USE_GLYPHS
      QVector<quint32> indexes(n);
      QVector<QPointF> positions(n);
      QGlyphRun nglyphs;
      nglyphs.setRawFont(glyphs.rawFont());

      positions[0] = QPointF();
      for (int i = 0; i < n; ++i) {
            indexes[i] = glyphs.glyphIndexes()[0];
            if (i)
                  positions[i] = QPointF(w * i, 0.0);
            }
      nglyphs.setGlyphIndexes(indexes);
      nglyphs.setPositions(positions);
#endif
      painter->scale(mag, mag);
      qreal imag = 1.0 / mag;
#ifdef USE_GLYPHS
      painter->drawGlyphRun(pos * imag, nglyphs);
#else
      painter->setFont(font());
      painter->drawText(pos * imag, QString(n, _code));
#endif
      painter->scale(imag, imag);
      }

//---------------------------------------------------------
//   symToHtml
//    transform symbol into html code suitable
//    for QDocument->setHtml()
//---------------------------------------------------------

QString symToHtml(const Sym& s, int leftMargin, const TextStyle* ts, qreal _spatium)
      {
      qreal size;
      if (ts) {
            size = ts->font(_spatium).pointSizeF();
            }
      else {
#ifdef USE_GLYPHS
            size = s.font().pointSizeF();
#else
            size = s.font().pixelSize();
#endif
            }

      QString family = s.font().family();
      return QString(
      "<data>"
        "<html>"
          "<head>"
            "<meta name=\"qrichtext\" content=\"1\" >"
            "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf8\" />"
            "<style type=\"text/css\">"
              "p, li { white-space: pre-wrap; }"
              "</style>"
            "</head>"
          "<body style=\" font-family:'%1'; font-size:%2pt;\">"
            "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:%3px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                "&#%4;"
              "</p>"
            "</body>"
          "</html>"
      "</data>").arg(family).arg(size).arg(leftMargin).arg(s.code());
      }

QString symToHtml(const Sym& s1, const Sym& s2, int leftMargin)
      {
      QFont f        = s1.font();
#ifdef USE_GLYPHS
      qreal size = s1.font().pointSizeF();
#else
      qreal size = s1.font().pixelSize();
#endif
      QString family = f.family();

      return QString(
      "<data>"
        "<html>"
          "<head>"
            "<meta name=\"qrichtext\" content=\"1\" >"
            "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf8\" />"
            "<style type=\"text/css\">"
              "p, li { white-space: pre-wrap; }"
              "</style>"
            "</head>"
          "<body style=\" font-family:'%1'; font-size:%2pt;\">"
            "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:%3px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                "&#%4;&#%5;"
              "</p>"
            "</body>"
          "</html>"
      "</data>").arg(family).arg(size).arg(leftMargin).arg(s1.code()).arg(s2.code());
      }

//---------------------------------------------------------
//   id2name
//---------------------------------------------------------

const char* Sym::id2name(SymId id)
      {
      if (id == noSym)
            return "noSym";
      return symNames[id];
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Sym::init()
      {
      symNames     = QVector<const char*>(lastSym);
      symUserNames = QVector<QString>(lastSym);

      for (unsigned int i = 0; i < sizeof(lilypondNames)/sizeof(*lilypondNames); ++i) {
            SymId id = lilypondNames[i].msIndex;
            lnhash[QString(lilypondNames[i].name)] = id;
            if (id != noSym) {
                  symNames[id]     = lilypondNames[i].name;
                  symUserNames[id] = lilypondNames[i].mname;
                  }
            }

// #define MT(a) QT_TRANSLATE_NOOP("symbol", a)

      symUserNames[clefEightSym]    = symNames[clefEightSym]  = "clef eight";
      symUserNames[clefOneSym]      = symNames[clefOneSym]    = "clef one";
      symUserNames[clefFiveSym]     = symNames[clefFiveSym]   = "clef five";
      symUserNames[letterTSym]      = symNames[letterTSym]    = "T";
      symUserNames[letterSSym]      = symNames[letterSSym]    = "S";
      symUserNames[letterPSym]      = symNames[letterPSym]    = "P";

      symUserNames[octave8]    = symNames[octave8]    = "octave8";
      symUserNames[octave8va]  = symNames[octave8va]  = "octave8va";
      symUserNames[octave8vb]  = symNames[octave8vb]  = "octave8vb";
      symUserNames[octave15]   = symNames[octave15]   = "octave15";
      symUserNames[octave15ma] = symNames[octave15ma] = "octave15ma";
      symUserNames[octave15mb] = symNames[octave15mb] = "octave15mb";
      symUserNames[octave22]   = symNames[octave22]   = "octave22";
      symUserNames[octave22ma] = symNames[octave22ma] = "octave22ma";
      symUserNames[octave22mb] = symNames[octave22mb] = "octave22mb";
      }

//---------------------------------------------------------
//   initSymbols
//---------------------------------------------------------

void initSymbols(int idx)
      {
      if (symbolsInitialized[idx])
            return;
      symbolsInitialized[idx] = true;
      symbols[idx] = QVector<Sym>(lastSym);

      symbols[idx][clefEightSym] = Sym(0x38, 2);
      symbols[idx][clefOneSym]   = Sym(0x31, 2);
      symbols[idx][clefFiveSym]  = Sym(0x35, 2);
      symbols[idx][letterTSym]   = Sym('T',  2);
      symbols[idx][letterSSym]   = Sym('S',  2);
      symbols[idx][letterPSym]   = Sym('P',  2);

      symbols[idx][octave8]      = Sym(0xe550, 4);
      symbols[idx][octave8va]    = Sym(0xe551, 4);
      symbols[idx][octave8vb]    = Sym(0xe552, 4);
      symbols[idx][octave15]     = Sym(0xe554, 4);
      symbols[idx][octave15ma]   = Sym(0xe555, 4);
      symbols[idx][octave15mb]   = Sym(0xe556, 4);
      symbols[idx][octave22]     = Sym(0xe557, 4);
      symbols[idx][octave22ma]   = Sym(0xe558, 4);
      symbols[idx][octave22mb]   = Sym(0xe559, 4);

      QString path;
#ifdef Q_OS_IOS
      {
      extern QString resourcePath();
      QString rpath = resourcePath();
      path = rpath + QString(idx == 0 ? "/mscore20.xml" : "/mscore/gonville.xml");
      }
#else
      path = idx == 0 ? ":/fonts/mscore20.xml" : ":/fonts/gonville.xml";
#endif
      QFile f(path);
      if (!f.open(QFile::ReadOnly)) {
            qDebug("cannot open symbols file %s", qPrintable(path));
            if (!MScore::debugMode)
                  exit(-1);
            }
      XmlReader e(&f);
      int fid = idx == 0 ? 0 : 3;

      while (e.readNextStartElement()) {
            if (e.name() == "museScore") {
                  while (e.readNextStartElement()) {
                        if (e.name() == "Glyph") {
                              QString name;
                              int code = -1;
                              QPointF p;
                              QRectF b;
                              while (e.readNextStartElement()) {
                                    const QStringRef& tag(e.name());
                                    if (tag == "name")
                                          name = e.readElementText();
                                    else if (tag == "code") {
                                          QString val(e.readElementText());
                                          bool ok;
                                          code = val.mid(2).toInt(&ok, 16);
                                          if (!ok)
                                                qDebug("cannot read code");
                                          }
                                    else if (tag == "attach")
                                          p = e.readPoint();
                                    else if (tag == "bbox")
                                          b = e.readRect();
                                    else
                                          e.unknown();
                                    }
                              if (code == -1)
                                    qDebug("no code for glyph <%s>", qPrintable(name));
                              SymId idx1 = Sym::name2id(name);
                              if (idx1 != noSym)
                                    symbols[idx][idx1] = Sym(code, fid, p, b);
                              else {
                                    qDebug("symbol <%s> declared in %s for symbol set %d not used",
                                       qPrintable(name), qPrintable(path), idx);
                                    }
                              }
                        else
                              e.unknown();
                        }
                  }
            else
                  e.unknown();
            }
      for (int i = 0; i < lastSym; ++i) {
            Sym* sym = &symbols[idx][i];
            if (sym->code() == -1) {
                  qDebug("no code for symbol %s", Sym::id2name(SymId(i)));
                  if (idx > 0) {
                        //fallback to default font
                        symbols[idx][i] = symbols[0][i];
                        }
                  }
            }
      }
}

