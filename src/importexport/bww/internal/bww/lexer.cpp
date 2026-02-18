/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 \file
 BWW file lexical analysis.

 Loosely based on BWW file format - BNF specification by tjm found in
 http://forums.bobdunsire.com/forums/showthread.php?t=123219
 */

#include <QRegularExpression>

#include "lexer.h"

#include "log.h"

namespace Bww {
/**
   Lexer constructor, \a inDevice is the input.
   */

Lexer::Lexer(QIODevice* inDevice)
    : in(inDevice)
{
    LOGD() << "Lexer::Lexer() begin";

    // Initialize the grace note translation table

    // Single Grace notes
    graceMap["ag"] = "LA";
    graceMap["bg"] = "B";
    graceMap["cg"] = "C";
    graceMap["dg"] = "D";
    graceMap["eg"] = "E";
    graceMap["fg"] = "F";
    graceMap["gg"] = "HG";
    graceMap["tg"] = "HA";

    // Regular Doublings
    graceMap["dblg"] = "HG LG D";
    graceMap["dbla"] = "HG LA D";
    graceMap["dbb"]  = "HG B D";
    graceMap["dbc"]  = "HG C D";
    graceMap["dbd"]  = "HG D E";
    graceMap["dbe"]  = "HG E F";
    graceMap["dbf"]  = "HG F HG";
    graceMap["dbhg"] = "HG F";
    graceMap["dbha"] = "HA HG";

    // Thumb Doublings
/*
    graceMap["thdblg"] = "HA LG D";
    graceMap["thdbla"] = "HA LA D";
    graceMap["thdbb"]  = "HA B D";
    graceMap["thdbc"]  = "HA C D";
    graceMap["thdbd"]  = "HA D E";
    graceMap["thdbe"]  = "HA E F";
    graceMap["thdbf"]  = "HA F HG";
    graceMap["thdbhg"] = "HA HG F";
*/
    graceMap["tdblg"] = "HA LG D";
    graceMap["tdbla"] = "HA LA D";
    graceMap["tdbb"]  = "HA B D";
    graceMap["tdbc"]  = "HA C D";
    graceMap["tdbd"]  = "HA D E";
    graceMap["tdbe"]  = "HA E F";
    graceMap["tdbf"]  = "HA F HG";

    // Half Doublings
    graceMap["hdblg"] = "LG D";
    graceMap["hdbla"] = "LA D";
    graceMap["hdbb"]  = "B D";
    graceMap["hdbc"]  = "C D";
    graceMap["hdbd"]  = "D E";
    graceMap["hdbe"]  = "E F";
    graceMap["hdbf"]  = "F HG";
/*
    graceMap["hdbhg"] = "HG F";
    graceMap["hdbha"] = "HA HG";
*/

    // Single Strikes (same as single grace notes)
    graceMap["strlg"] = "LG";
    graceMap["strla"] = "LA";
    graceMap["strb"]  = "B";
    graceMap["strc"]  = "C";
    graceMap["strd"]  = "D";
    graceMap["stre"]  = "E";
    graceMap["strf"]  = "F";
    graceMap["strhg"] = "HG";
/*
    graceMap["strha"] = "HA";
*/

    // G Grace note, Thumb and Half Strikes
    graceMap["gstla"]  = "HG LA LG";
    graceMap["gstb"]   = "HG B LG";
    graceMap["gstc"]   = "HG C LG";
    graceMap["gstd"]   = "HG D LG";
    graceMap["lgstd"]  = "HG D C";
    graceMap["gste"]   = "HG E LA";
    graceMap["gstf"]   = "HG F E";

    graceMap["tstla"]  = "HA LA LG";
    graceMap["tstb"]   = "HA B LG";
    graceMap["tstc"]   = "HA C LG";
    graceMap["tstd"]   = "HA D LG";
    graceMap["ltstd"]  = "HA D C";
    graceMap["tste"]   = "HA E LA";
    graceMap["tstf"]   = "HA F E";
    graceMap["tsthg"]  = "HA HG F";

    graceMap["hstla"]  = "LA LG";
    graceMap["hstb"]   = "B LG";
    graceMap["hstc"]   = "C LG";
    graceMap["hstd"]   = "D LG";
    graceMap["lhstd"]  = "D C";
    graceMap["hste"]   = "E LA";
    graceMap["hstf"]   = "F E";
    graceMap["hsthg"]  = "HG F";

    // Regular Grips
    graceMap["grp"]  = "LG D LG";
    graceMap["hgrp"] = "D LG";
    graceMap["grpb"] = "LG B LG";

    // G Grace note, Thumb and Half Grips
    graceMap["ggrpla"]  = "HG LA LG D LG";
    graceMap["ggrpb"]   = "HG B LG D LG";
    graceMap["ggrpc"]   = "HG C LG D LG";
    graceMap["ggrpd"]   = "HG D LG D LG";
    graceMap["ggrpdb"]  = "HG D LG B LG";
    graceMap["ggrpe"]   = "HG E LG D LG";
    graceMap["ggrpf"]   = "HG F LG F LG";

    graceMap["tgrpla"]  = "HA LA LG D LG";
    graceMap["tgrpb"]   = "HA B LG D LG";
    graceMap["tgrpc"]   = "HA C LG D LG";
    graceMap["tgrpd"]   = "HA D LG D LG";
    graceMap["tgrpdb"]  = "HA D LG B LG";
    graceMap["tgrpe"]   = "HA E LG D LG";
    graceMap["tgrpf"]   = "HA F LG F LG";
    graceMap["tgrphg"]  = "HA HG LG F LG";

    graceMap["hgrpla"]  = "LA LG D LG";
    graceMap["hgrpb"]   = "B LG D LG";
    graceMap["hgrpc"]   = "C LG D LG";
    graceMap["hgrpd"]   = "D LG D LG";
    graceMap["hgrpdb"]  = "D LG B LG";
    graceMap["hgrpe"]   = "E LG D LG";
    graceMap["hgrpf"]   = "F LG F LG";
    graceMap["hgrphg"]  = "HG LG D LG";
    graceMap["hgrpha"]  = "HA LG D LG";

    // Taorluaths and Bublys
    graceMap["tar"]    = "LG D LG E";
    graceMap["tarb"]   = "LG B LG E";
    graceMap["htar"]   = "D LG E";
    graceMap["bubly"]  = "LG D LG C LG";
    graceMap["hbubly"] = "D LG C LG";

    //  Birls
    graceMap["brl"] = "LG LA LG";
    graceMap["abr"] = "LA LG LA LG";
    graceMap["gbr"] = "HG LA LG LA LG";
    graceMap["tbr"] = "HA LA LG LA LG";

    // Light, Heavy and Half D Throws
    graceMap["thrd"]    = "LG D C";
    graceMap["hvthrd"]  = "LG D LG C";
    graceMap["hthrd"]   = "D C";
    graceMap["hhvthrd"] = "D LG C";

    // Regular, Thumb Grace Note and Half Peles
    graceMap["pella"]  = "HG LA E LA LG";
    graceMap["pelb"]   = "HG B E B LG";
    graceMap["pelc"]   = "HG C E C LG";
    graceMap["peld"]   = "HG D E D LG";
    graceMap["lpeld"]  = "HG D E D C";
    graceMap["pele"]   = "HG E F E LA";
    graceMap["pelf"]   = "HG F HG F E";

    graceMap["tpella"]  = "HA LA E LA LG";
    graceMap["tpelb"]   = "HA B E B LG";
    graceMap["tpelc"]   = "HA C E C LG";
    graceMap["tpeld"]   = "HA D E D LG";
    graceMap["ltpeld"]  = "HA D E D C";
    graceMap["tpele"]   = "HA E F E LA";
    graceMap["tpelf"]   = "HA F HG F E";
    graceMap["tpelhg"]  = "HA HG HA HG F";

    graceMap["hpella"]  = "LA E LA LG";
    graceMap["hpelb"]   = "B E B LG";
    graceMap["hpelc"]   = "C E C LG";
    graceMap["hpeld"]   = "D E D LG";
    graceMap["lhpeld"]  = "D E D C";
    graceMap["hpele"]   = "E F E LA";
    graceMap["hpelf"]   = "F HG F E";
    graceMap["hpelhg"]  = "HG HA HG F";

    // Regular Double Strikes
    graceMap["st2la"]  = "LG LA LG";
    graceMap["st2b"]   = "LG B LG";
    graceMap["st2c"]   = "LG C LG";
    graceMap["st2d"]   = "LG D LG";
    graceMap["lst2d"]  = "C D C";
    graceMap["st2e"]   = "LA E LA";
    graceMap["st2f"]   = "E F E";
    graceMap["st2hg"]  = "F HG F";
    graceMap["st2ha"]  = "HG HA HG";

    // G Grace note, Thumb and Half Double Strikes
    graceMap["gst2la"]  = "HG LA LG LA LG";
    graceMap["gst2b"]   = "HG B LG B LG";
    graceMap["gst2c"]   = "HG C LG C LG";
    graceMap["gst2d"]   = "HG D LG D LG";
    graceMap["lgst2d"]  = "HG D C D C";
    graceMap["gst2e"]   = "HG E LA E LA";
    graceMap["gst2f"]   = "HG F E F E";

    graceMap["tst2la"]   = "HA LA LG LA LG";
    graceMap["tst2b"]    = "HA B LG B LG";
    graceMap["tst2c"]    = "HA C LG C LG";
    graceMap["tst2d"]    = "HA D LG D LG";
    graceMap["ltst2d"]   = "HA D C D C";
    graceMap["tst2e"]    = "HA E LA E LA";
    graceMap["tst2f"]    = "HA F E F E";
    graceMap["tst2hg"]   = "HA HG F HG F";

    graceMap["hst2la"]   = "LA LG LA LG";
    graceMap["hst2b"]    = "B LG B LG";
    graceMap["hst2c"]    = "C LG C LG";
    graceMap["hst2d"]    = "D LG D LG";
    graceMap["lhst2d"]   = "D C D C";
    graceMap["hst2e"]    = "E LA E LA";
    graceMap["hst2f"]    = "F E F E";
    graceMap["hst2hg"]   = "HG F HG F";
    graceMap["hst2ha"]   = "HA HG HA HG";

    // Regular Triple Strikes
    graceMap["st3la"]  = "LG LA LG LA LG";
    graceMap["st3b"]   = "LG B LG B LG";
    graceMap["st3c"]   = "LG C LG C LG";
    graceMap["st3d"]   = "LG D LG D LG";
    graceMap["lst3d"]  = "C D C D C";
    graceMap["st3e"]   = "LA E LA E LA";
    graceMap["st3f"]   = "E F E F E";
    graceMap["st3hg"]  = "F HG F HG F";
    graceMap["st3ha"]  = "HG HA HG HA HG";

    // G Grace note, Thumb and Half Triple Strikes
    graceMap["gst3la"]  = "HG LA LG LA LG LA LG";
    graceMap["gst3b"]   = "HG B LG B LG B LG";
    graceMap["gst3c"]   = "HG C LG C LG C LG";
    graceMap["gst3d"]   = "HG D LG D LG D LG";
    graceMap["lgst3d"]  = "HG D C D C D C";
    graceMap["gst3e"]   = "HG E LA E LA E LA";
    graceMap["gst3f"]   = "HG F E F E F E";

    graceMap["tst3la"]   = "HA LA LG LA LG LA LG";
    graceMap["tst3b"]    = "HA B LG B LG B LG";
    graceMap["tst3c"]    = "HA C LG C LG C LG";
    graceMap["tst3d"]    = "HA D LG D LG D LG";
    graceMap["ltst3d"]   = "HA D C D C D C";
    graceMap["tst3e"]    = "HA E LA E LA E LA";
    graceMap["tst3f"]    = "HA F E F E F E";
    graceMap["tst3hg"]   = "HA HG F HG F HG F";

    graceMap["hst3la"]   = "LA LG LA LG LA LG";
    graceMap["hst3b"]    = "B LG B LG B LG";
    graceMap["hst3c"]    = "C LG C LG C LG";
    graceMap["hst3d"]    = "D LG D LG D LG";
    graceMap["lhst3d"]   = "D C D C D C";
    graceMap["hst3e"]    = "E LA E LA E LA";
    graceMap["hst3f"]    = "F E F E F E";
    graceMap["hst3hg"]   = "HG F HG F HG F";
    graceMap["hst3ha"]   = "HA HG HA HG HA HG";

    // Double Grace notes
    graceMap["dlg"] = "D LG";
    graceMap["dla"] = "D LA";
    graceMap["db"]  = "D B";
    graceMap["dc"]  = "D C";
    graceMap["elg"] = "E LG";
    graceMap["ela"] = "E LA";
    graceMap["eb"]  = "E B";
    graceMap["ec"]  = "E C";
    graceMap["ed"]  = "E D";

    graceMap["flg"] = "F LG";
    graceMap["fla"] = "F LA";
    graceMap["fb"]  = "F B";
    graceMap["fc"]  = "F C";
    graceMap["fd"]  = "F D";
    graceMap["fe"]  = "F E";

    graceMap["glg"] = "HG LG";
    graceMap["gla"] = "HG LA";
    graceMap["gb"]  = "HG B";
    graceMap["gc"]  = "HG C";
    graceMap["gd"]  = "HG D";
    graceMap["ge"]  = "HG E";
    graceMap["gf"]  = "HG F";

    graceMap["tlg"] = "HA LG";
    graceMap["tla"] = "HA LA";
    graceMap["tb"]  = "HA B";
    graceMap["tc"]  = "HA C";
    graceMap["td"]  = "HA D";
    graceMap["te"]  = "HA E";
    graceMap["tf"]  = "HA F";
    graceMap["thg"] = "HA HG";

    // piobraich
    graceMap["endari"] = "E LA F LA";
    graceMap["embari"] = "E LG F LG";
    graceMap["dare"]   = "F E HG E";
    graceMap["crunl"]  = "LG D LG E LA F LA";
    graceMap["crunlb"] = "LG B LG E LA F LA";

    getSym();

    LOGD() << "Lexer::Lexer() end";
}

/**
   Get the next symbol, update type and value.
   */

void Lexer::getSym()
{
    LOGD() << "Lexer::getSym()";

    // if unparsed words remaining, use these
    if (list.size() > 0) {
        categorizeWord(list.at(0));
        list.removeFirst();
        return;
    }

    // read the next non-empty line
    do{
        line = in.readLine();
        ++lineNumber;
        if (line.isNull()) {
            // end of file
            LOGD() << "-> end of file";
            type = NONE;
            value = "";
            return;
        }
    }while (line == "");

    LOGD() << "getSym: read line" << line;
    QRegularExpression rHeaderIgnore("^Bagpipe Reader|^MIDINoteMappings|^FrequencyMappings"
                                     "|^InstrumentMappings|^GracenoteDurations|^FontSizes"
                                     "|^TuneFormat");
    QRegularExpression rTuneTempo("^TuneTempo");
    if (rHeaderIgnore.match(line).capturedStart() == 0) {
        type = COMMENT;
        value = "";
        LOGD()
            << "-> header ignore,"
            << "type:" << symbolToString(type)
            << "value:" << value
        ;
        line = "";
        return;
    } else if (rTuneTempo.match(line).capturedStart() == 0) {
        type = TEMPO;
        value = line;
        LOGD()
            << "-> tempo,"
            << "type:" << symbolToString(type)
            << "value:" << value
        ;
        line = "";
        return;
    } else if (line.at(0) == QChar('"')) {
        type = STRING;
        value = line;
        LOGD()
            << "-> quoted string,"
            << "type:" << symbolToString(type)
            << "value:" << value
        ;
        line = "";
    } else {
        // split line into space-separated words
        list = line.trimmed().split(QRegularExpression("\\s+"));
        LOGD()
            << "-> words"
            << list
        ;
        categorizeWord(list.at(0));
        list.removeFirst();
    }
    line = "";
}

/**
   Return the current symbols type.
   */

Symbol Lexer::symType() const
{
    return type;
}

/**
   Return the current symbols value.
   */

QString Lexer::symValue() const
{
    return value;
}

/**
   Determine the symbol type for \a word.
   */

void Lexer::categorizeWord(QString word)
{
    LOGD() << "Lexer::categorizeWord(" << word << ")";

    // default values
    type = NONE;
    value = word;

    QRegularExpression rClef(QRegularExpression::anchoredPattern("&"));
    QRegularExpression rKey(QRegularExpression::anchoredPattern("sharp[cf]"));
    QRegularExpression rTSig(QRegularExpression::anchoredPattern("\\d+_(1|2|4|8|16|32)"));
    QRegularExpression rPart(QRegularExpression::anchoredPattern("I!''|I!|''!I|!I|'intro|[2-9]|'[12]|_'"));
    QRegularExpression rBar(QRegularExpression::anchoredPattern("!|!t|!!t"));
    QRegularExpression rNote(QRegularExpression::anchoredPattern("(LG|LA|[B-F]|HG|HA)[lr]?_(1|2|4|8|16|32)"));
    QRegularExpression rTie(QRegularExpression::anchoredPattern("\\^t[es]"));
    QRegularExpression rTriplet(QRegularExpression::anchoredPattern("\\^3[es]"));
    QRegularExpression rDot(QRegularExpression::anchoredPattern("'([hl][ag]|[b-f])"));

    if (rClef.match(word).hasMatch()) {
        type = CLEF;
    } else if (rKey.match(word).hasMatch()) {
        type = KEY;
    } else if (rTSig.match(word).hasMatch()) {
        type = TSIG;
    } else if (rPart.match(word).hasMatch()) {
        type = PART;
    } else if (rBar.match(word).hasMatch()) {
        type = BAR;
    } else if (rNote.match(word).hasMatch()) {
        type = NOTE;
    } else if (rTie.match(word).hasMatch()) {
        type = TIE;
    } else if (rTriplet.match(word).hasMatch()) {
        type = TRIPLET;
    } else if (rDot.match(word).hasMatch()) {
        type = DOT;
    } else if (graceMap.contains(word)) {
        type  = GRACE;
        value = graceMap.value(word);
    } else {
        type = UNKNOWN;
    }

    LOGD()
        << " type: " << qPrintable(symbolToString(type))
        << " value: '" << qPrintable(value) << "'"
    ;
}
} // namespace Bww
