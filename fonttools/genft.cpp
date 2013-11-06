//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2009 Werner Schweer and others
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

//
//    q+d hack to create an font description file from
//    lilipond tables embedded in mscore-20.ttf
//
//    usage: genft mscore-20.ttf > symbols.xml
//

#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/tttables.h>

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
      { "space", "space" },
      { "plus", "plus" },
      { "comma", "comma" },
      { "hyphen", "hyphen" },
      { "period", "period" },
      { "zero", "zero" },
      { "one", "one" },
      { "two", "two" },
      { "three", "three" },
      { "four", "four" },
      { "five", "five" },
      { "six", "six" },
      { "seven", "seven" },
      { "eight", "eight" },
      { "nine", "nine" },
      { "f", "f" },
      { "m", "m" },
      { "p", "p" },
      { "r", "r" },
      { "s", "s" },
      { "z", "z" },
      { "rests.0", "rests.0" },
      { "rests.1", "rests.1" },
      { "rests.0o", "rests.0o" },
      { "rests.1o", "rests.1o" },
      { "rests.M3", "rests.M3" },
      { "rests.M2", "rests.M2" },
      { "rests.M1", "rests.M1" },
      { "rests.2", "rests.2" },
      { "rests.2classical", "rests.2classical" },
      { "rests.3", "rests.3" },
      { "rests.4", "rests.4" },
      { "rests.5", "rests.5" },
      { "rests.6", "rests.6" },
      { "rests.7", "rests.7" },
      { "accidentals.sharp", "accidentals.sharp" },
      { "accidentals.sharp.slashslash.stem", "accidentals.sharp.slashslash.stem" },
      { "accidentals.sharp.slashslashslash.stemstem", "accidentals.sharp.slashslashslash.stemstem" },
      { "accidentals.sharp.slashslashslash.stem", "accidentals.sharp.slashslashslash.stem" },
      { "accidentals.sharp.slashslash.stemstemstem", "accidentals.sharp.slashslash.stemstemstem" },
      { "accidentals.natural", "accidentals.natural" },
      { "accidentals.flat", "accidentals.flat" },
      { "accidentals.flat.slash", "accidentals.flat.slash" },
      { "accidentals.flat.slashslash", "accidentals.flat.slashslash" },
      { "accidentals.mirroredflat.flat", "accidentals.mirroredflat.flat" },
      { "accidentals.mirroredflat", "accidentals.mirroredflat" },
      { "accidentals.mirroredflat.backslash", "accidentals.mirroredflat.backslash" },
      { "accidentals.flatflat", "accidentals.flatflat" },
      { "accidentals.flatflat.slash", "accidentals.flatflat.slash" },
      { "accidentals.doublesharp", "accidentals.doublesharp" },
      { "accidentals.rightparen", "accidentals.rightparen" },
      { "accidentals.leftparen", "accidentals.leftparen" },
      { "arrowheads.open.01", "arrowheads.open.01" },
      { "arrowheads.open.0M1", "arrowheads.open.0M1" },
      { "arrowheads.open.11", "arrowheads.open.11" },
      { "arrowheads.open.1M1", "arrowheads.open.1M1" },
      { "arrowheads.close.01", "arrowheads.close.01" },
      { "arrowheads.close.0M1", "arrowheads.close.0M1" },
      { "arrowheads.close.11", "arrowheads.close.11" },
      { "arrowheads.close.1M1", "arrowheads.close.1M1" },
      { "dots.dot", "dots.dot" },
      { "noteheads.uM2", "noteheads.uM2" },
      { "noteheads.dM2", "noteheads.dM2" },
      { "noteheads.sM1", "noteheads.sM1" },
      { "noteheads.s0", "noteheads.s0" },
      { "noteheads.s1", "noteheads.s1" },
      { "noteheads.s2", "noteheadBlack" },
      { "noteheads.s0diamond", "noteheads.s0diamond" },
      { "noteheads.s1diamond", "noteheads.s1diamond" },
      { "noteheads.s2diamond", "noteheads.s2diamond" },
      { "noteheads.s0triangle", "noteheads.s0triangle" },
      { "noteheads.d1triangle", "noteheads.d1triangle" },
      { "noteheads.u1triangle", "noteheads.u1triangle" },
      { "noteheads.u2triangle", "noteheads.u2triangle" },
      { "noteheads.d2triangle", "noteheads.d2triangle" },
      { "noteheads.s0slash", "noteheads.s0slash" },
      { "noteheads.s1slash", "noteheads.s1slash" },
      { "noteheads.s2slash", "noteheads.s2slash" },
      { "noteheads.s0cross", "noteheads.s0cross" },
      { "noteheads.s1cross", "noteheads.s1cross" },
      { "noteheads.s2cross", "noteheads.s2cross" },
      { "noteheads.s2xcircle", "noteheads.s2xcircle" },
      { "noteheads.s0do", "noteheads.s0do" },
      { "noteheads.d1do", "noteheads.d1do" },
      { "noteheads.u1do", "noteheads.u1do" },
      { "noteheads.d2do", "noteheads.d2do" },
      { "noteheads.u2do", "noteheads.u2do" },
      { "noteheads.s0re", "noteheads.s0re" },
      { "noteheads.u1re", "noteheads.u1re" },
      { "noteheads.d1re", "noteheads.d1re" },
      { "noteheads.u2re", "noteheads.u2re" },
      { "noteheads.d2re", "noteheads.d2re" },
      { "noteheads.s0mi", "noteheads.s0mi" },
      { "noteheads.s1mi", "noteheads.s1mi" },
      { "noteheads.s2mi", "noteheads.s2mi" },
      { "noteheads.u0fa", "noteheads.u0fa" },
      { "noteheads.d0fa", "noteheads.d0fa" },
      { "noteheads.u1fa", "noteheads.u1fa" },
      { "noteheads.d1fa", "noteheads.d1fa" },
      { "noteheads.u2fa", "noteheads.u2fa" },
      { "noteheads.d2fa", "noteheads.d2fa" },
      { "noteheads.s0la", "noteheads.s0la" },
      { "noteheads.s1la", "noteheads.s1la" },
      { "noteheads.s2la", "noteheads.s2la" },
      { "noteheads.s0ti", "noteheads.s0ti" },
      { "noteheads.u1ti", "noteheads.u1ti" },
      { "noteheads.d1ti", "noteheads.d1ti" },
      { "noteheads.u2ti", "noteheads.u2ti" },
      { "noteheads.d2ti", "noteheads.d2ti" },
      { "scripts.ufermata", "scripts.ufermata" },
      { "scripts.dfermata", "scripts.dfermata" },
      { "scripts.ushortfermata", "scripts.ushortfermata" },
      { "scripts.dshortfermata", "scripts.dshortfermata" },
      { "scripts.ulongfermata", "scripts.ulongfermata" },
      { "scripts.dlongfermata", "scripts.dlongfermata" },
      { "scripts.uverylongfermata", "scripts.uverylongfermata" },
      { "scripts.dverylongfermata", "scripts.dverylongfermata" },
      { "scripts.thumb", "scripts.thumb" },
      { "scripts.sforzato", "scripts.sforzato" },
      { "scripts.espr", "scripts.espr" },
      { "scripts.staccato", "scripts.staccato" },
      { "scripts.ustaccatissimo", "scripts.ustaccatissimo" },
      { "scripts.dstaccatissimo", "scripts.dstaccatissimo" },
      { "scripts.tenuto",   "articTenuto" },
      { "scripts.uportato", "scripts.uportato" },
      { "scripts.dportato", "scripts.dportato" },
      { "scripts.umarcato", "scripts.umarcato" },
      { "scripts.dmarcato", "scripts.dmarcato" },
      { "scripts.open", "scripts.open" },
      { "scripts.stopped", "scripts.stopped" },
      { "scripts.upbow", "scripts.upbow" },
      { "scripts.downbow", "scripts.downbow" },
      { "scripts.reverseturn", "scripts.reverseturn" },
      { "scripts.turn", "scripts.turn" },
      { "scripts.trill", "scripts.trill" },
      { "scripts.upedalheel", "scripts.upedalheel" },
      { "scripts.dpedalheel", "scripts.dpedalheel" },
      { "scripts.upedaltoe", "scripts.upedaltoe" },
      { "scripts.dpedaltoe", "scripts.dpedaltoe" },
      { "scripts.flageolet", "scripts.flageolet" },
      { "scripts.segno", "scripts.segno" },
      { "scripts.coda", "scripts.coda" },
      { "scripts.varcoda", "scripts.varcoda" },
      { "scripts.rcomma", "scripts.rcomma" },
      { "scripts.lcomma", "scripts.lcomma" },
      { "scripts.rvarcomma", "scripts.rvarcomma" },
      { "scripts.lvarcomma", "scripts.lvarcomma" },
      { "scripts.arpeggio", "scripts.arpeggio" },
      { "scripts.trill_element", "scripts.trill_element" },
      { "scripts.arpeggio.arrow.M1", "scripts.arpeggio.arrow.M1" },
      { "scripts.arpeggio.arrow.1", "scripts.arpeggio.arrow.1" },
      { "scripts.trilelement", "scripts.trilelement" },
      { "scripts.prall", "scripts.prall" },
      { "scripts.mordent", "scripts.mordent" },
      { "scripts.prallprall", "scripts.prallprall" },
      { "scripts.prallmordent", "scripts.prallmordent" },
      { "scripts.upprall", "scripts.upprall" },
      { "scripts.upmordent", "scripts.upmordent" },
      { "scripts.pralldown", "scripts.pralldown" },
      { "scripts.downprall", "scripts.downprall" },
      { "scripts.downmordent", "scripts.downmordent" },
      { "scripts.prallup", "scripts.prallup" },
      { "scripts.lineprall", "scripts.lineprall" },
      { "scripts.caesura.curved", "scripts.caesura.curved" },
      { "scripts.caesura.straight", "scripts.caesura.straight" },
      { "flags.u3", "flags.u3" },
      { "flags.u4", "flags.u4" },
      { "flags.u5", "flags.u5" },
      { "flags.u6", "flags.u6" },
      { "flags.d3", "flags.d3" },
      { "flags.ugrace", "flags.ugrace" },
      { "flags.dgrace", "flags.dgrace" },
      { "flags.d4", "flags.d4" },
      { "flags.d5", "flags.d5" },
      { "flags.d6", "flags.d6" },
      { "clefs.C", "clefs.C" },
      { "clefs.C_change", "clefs.C_change" },
      { "clefs.F", "clefs.F" },
      { "clefs.F_change", "clefs.F_change" },
      { "clefs.G", "clefs.G" },
      { "clefs.G_change", "clefs.G_change" },
      { "clefs.percussion", "clefs.percussion" },
      { "clefs.percussion_change", "clefs.percussion_change" },
      { "clefs.tab", "clefs.tab" },
      { "clefs.tab_change", "clefs.tab_change" },
      { "timesig.C44", "timesig.C44" },
      { "timesig.C22", "timesig.C22" },
      { "pedal.*", "pedal.*" },
      { "pedal.M", "pedal.M" },
      { "pedal..", "pedal.." },
      { "pedal.P", "pedal.P" },
      { "pedal.d", "pedal.d" },
      { "pedal.e", "pedal.e" },
      { "pedal.Ped", "pedal.Ped" },
      { "brackettips.uright", "brackettips.uright" },
      { "brackettips.dright", "brackettips.dright" },
      { "accordion.accDiscant", "accordion.accDiscant" },
      { "accordion.accDot", "accordion.accDot" },
      { "accordion.accFreebase", "accordion.accFreebase" },
      { "accordion.accStdbase", "accordion.accStdbase" },
      { "accordion.accBayanbase", "accordion.accBayanbase" },
      { "accordion.accOldEE", "accordion.accOldEE" },
      { "brackettips.uleft", "brackettips.uleft" },
      { "brackettips.dleft", "brackettips.dleft" },
      { "flags.d7", "flags.d7" },
      { "flags.u7", "flags.u7" },
      { "scripts.snappizzicato", "scripts.snappizzicato" },
      { "noteheads.sM1double", "noteheads.sM1double" },
      { "accidentals.flat.arrowup", "accidentals.flat.arrowup" },
      { "accidentals.flat.arrowdown", "accidentals.flat.arrowdown" },
      { "accidentals.flat.arrowboth", "accidentals.flat.arrowboth" },
      { "accidentals.natural.arrowup", "accidentals.natural.arrowup" },
      { "accidentals.natural.arrowdown", "accidentals.natural.arrowdown" },
      { "accidentals.natural.arrowboth", "accidentals.natural.arrowboth" },
      { "accidentals.sharp.arrowup", "accidentals.sharp.arrowup" },
      { "accidentals.sharp.arrowdown", "accidentals.sharp.arrowdown" },
      { "accidentals.sharp.arrowboth", "accidentals.sharp.arrowboth" },
      { "noteheads.uM2alt", "noteheads.uM2alt" },
      { "noteheads.dM2alt", "noteheads.dM2alt" },
      { "noteheads.sM1alt", "noteheads.sM1alt" },
      { "timesig.Cdot", "timesig.Cdot" },
      { "timesig.O", "timesig.O" },
      { "timesig.Ocut", "timesig.Ocut" },
      { "timesig.Odot", "timesig.Odot" },
      { "uniE1CB", "uniE1CB" },
      { "uniE1CC", "uniE1CC" },
      { "uniE1CD", "uniE1CD" },
      { "uniE1CE", "uniE1CE" },
      { "uniE1CF", "uniE1CF" },
      { "uniE1D0", "uniE1D0" },
      { "uniE1D1", "uniE1D1" },
      { "uniE1D2", "uniE1D2" },
      { "uniE1D3", "uniE1D3" },
      { "accidentals.sori", "accidentals.sori" },
      { "accidentals.koron", "accidentals.koron" },
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
            qDebug("genft: cannot load table LILC\n");
            exit(-3);
            }
      FT_Byte* buffer = (FT_Byte*)malloc(length + 1);
      error = FT_Load_Sfnt_Table(face, tag, 0, buffer, &length);
      buffer[length] = 0;
      if (error) {
            qDebug("genft: cannot load font table LILC\n");
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
                  qDebug("genft: <%s> not in map\n", qPrintable(s));
            int code = 0;
            if (codemap.contains(idx))
                  code = codemap[idx];
            else
                  qDebug("codemap has no index %d\n", idx);
            g.code = code;

            s = sl[i+4];
            int val = ra.indexIn(s);
            if (val == -1 || ra.captureCount() != 2) {
                  qDebug("bad reg expr a\n");
                  exit(-5);
                  }
            g.attach.rx() = ra.cap(1).toDouble();
            g.attach.ry() = -ra.cap(2).toDouble();

            s = sl[i+1];
            val = rb.indexIn(s);
            if (val == -1 || rb.captureCount() != 4) {
                  qDebug("bad reg expr b\n");
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
                  printf("not found: <%s>\n", qPrintable(i.key()));
                  continue;
                  }

            int code = codemap[i.value()];

            QString smufl = nmap.value(i.key());

            QJsonObject jg;
            QString s = QString("U+%1").arg(code, 0, 16);
            jg.insert("codepoint", s);

            QJsonValue val(s);
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
            qDebug("init free type library failed\n");
            exit(-1);
            }
      FT_Face face;
      int error = FT_New_Face(library, argv[1], 0, &face);
      if (error) {
            qDebug("open font failed <%s>\n", argv[1]);
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
      // p = getTable("LILY", face);      // global values, not used now
      // p = getTable("LILF", face);      // subfont table, not used now
      genJson();
      return 0;
      }

