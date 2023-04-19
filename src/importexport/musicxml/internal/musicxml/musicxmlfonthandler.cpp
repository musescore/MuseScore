/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
 MusicXML font handling support.
 */

#include "musicxmlfonthandler.h"

namespace mu::engraving {
//---------------------------------------------------------
//   MScoreTextToMXML
//---------------------------------------------------------

MScoreTextToMXML::MScoreTextToMXML(const QString& tag, const QString& attr, const CharFormat& defFmt, const QString& mtf)
    : attribs(attr), tagname(tag), oldFormat(defFmt), musicalTextFont(mtf)
{
    // set MusicXML defaults
    oldFormat.setBold(false);
    oldFormat.setItalic(false);
    oldFormat.setUnderline(false);
}

//---------------------------------------------------------
//   toPlainText
//    convert to plain text
//    naive implementation: simply remove all chars from '<' to '>'
//    typically used to remove formatting info from fields read
//    from MuseScore 1.3 file where they are stored as html, such as
//    part name and shortName
//---------------------------------------------------------

QString MScoreTextToMXML::toPlainText(const QString& text)
{
    QString res;
    bool inElem = false;
    foreach (QChar ch, text) {
        if (ch == '<') {
            inElem = true;
        } else if (ch == '>') {
            inElem = false;
        } else {
            if (!inElem) {
                res += ch;
            }
        }
    }
    //LOGD("MScoreTextToMXML::toPlainText('%s') res '%s'", qPrintable(text), qPrintable(res));
    return res;
}

//---------------------------------------------------------
//   toPlainTextPlusSymbols
//    convert to plain text plus <sym>[name]</sym> encoded symbols
//---------------------------------------------------------

QString MScoreTextToMXML::toPlainTextPlusSymbols(const std::list<TextFragment>& list)
{
    QString res;
    for (const TextFragment& f : list) {
        res += f.text;
    }
    return res;
}

//---------------------------------------------------------
//   plainTextPlusSymbolsSize
//---------------------------------------------------------

static int plainTextPlusSymbolsFragmentSize(const TextFragment& f)
{
    return f.columns();
}

//---------------------------------------------------------
//   plainTextPlusSymbolsSize
//---------------------------------------------------------

static int plainTextPlusSymbolsListSize(const std::list<TextFragment>& list)
{
    int res = 0;
    for (const TextFragment& f : list) {
        res += plainTextPlusSymbolsFragmentSize(f);
    }
    return res;
}

//---------------------------------------------------------
//   split
//---------------------------------------------------------

/**
 Split \a in into \a left, \a mid and \a right. Mid starts at \a pos and is \a len characters long.
 Pos and len refer to the representation returned by toPlainTextPlusSymbols().
 TODO Make sure surrogate pairs are handled correctly
 Return true if OK, false on error.
 */

bool MScoreTextToMXML::split(const std::list<TextFragment>& in, const int pos, const int len,
                             std::list<TextFragment>& left, std::list<TextFragment>& mid, std::list<TextFragment>& right)
{
    //LOGD("MScoreTextToMXML::split in size %d pos %d len %d", plainTextPlusSymbolsListSize(in), pos, len);
    //LOGD("-> in");
    //dumpText(in);

    if (pos < 0 || len < 0) {
        return false;
    }

    // ensure output is empty at start
    left.clear();
    mid.clear();
    right.clear();

    // set pos to begin of first fragment
    std::list<TextFragment>::const_iterator fragmentNr = in.begin();
    TextFragment fragment;
    if (fragmentNr != in.end()) {
        fragment = *fragmentNr;
    }
    std::list<TextFragment>* currentDest = &left;
    int currentMaxSize = pos;

    // while text left
    while (fragmentNr != in.end()) {
        int destSize = plainTextPlusSymbolsListSize(*currentDest);
        int fragSize = plainTextPlusSymbolsFragmentSize(fragment);
        // if no room left in current destination (check applies only to left and mid)
        if ((currentDest != &right && destSize >= currentMaxSize)
            || currentDest == &right) {
            // move to next destination
            if (currentDest == &left) {
                currentDest = &mid;
                currentMaxSize = len;
            } else if (currentDest == &mid) {
                currentDest = &right;
            }
        }
        // if current fragment fits in current destination (check applies only to left and mid)
        if ((currentDest != &right && destSize + fragSize <= currentMaxSize)
            || currentDest == &right) {
            // add it
            currentDest->push_back(fragment);
            // move to next fragment
            fragmentNr++;
            if (fragmentNr != in.end()) {
                fragment = *fragmentNr;
            }
        } else {
            // split current fragment
            TextFragment rightPart = fragment.split(currentMaxSize - plainTextPlusSymbolsListSize(*currentDest));
            // add first part to current destination
            currentDest->push_back(fragment);
            fragment = rightPart;
        }
    }

    /*
    LOGD("-> left");
    dumpText(left);
    LOGD("-> mid");
    dumpText(mid);
    LOGD("-> right");
    dumpText(right);
     */

    return true;
}

//---------------------------------------------------------
//   writeTextFragments
//---------------------------------------------------------

void MScoreTextToMXML::writeTextFragments(const std::list<TextFragment>& fr, XmlWriter& xml)
{
    //LOGD("MScoreTextToMXML::writeTextFragments defFmt %s", qPrintable(charFormat2QString(oldFormat)));
    //dumpText(fr);
    bool firstTime = true;   // write additional attributes only the first time characters are written
    for (const TextFragment& f : fr) {
        newFormat = f.format;
        QString formatAttr = updateFormat();
        xml.tagRaw(tagname + (firstTime ? attribs : "") + formatAttr, f.text);
        firstTime = false;
    }
}

//---------------------------------------------------------
//   attribute
//    add one attribute if necessary
//---------------------------------------------------------

static QString attribute(bool needed, bool value, QString trueString, QString falseString)
{
    QString res;
    if (needed) {
        res = value ? trueString : falseString;
    }
    if (res != "") {
        res = " " + res;
    }
    return res;
}

//---------------------------------------------------------
//   updateFormat
//    update the text format by generating attributes
//    corresponding to the difference between old- and newFormat
//    copy newFormat to oldFormat
//---------------------------------------------------------

QString MScoreTextToMXML::updateFormat()
{
    if (newFormat.fontFamily() == "ScoreText") {
        newFormat.setFontFamily(musicalTextFont);
    }
    QString res;
    res += attribute(newFormat.bold() != oldFormat.bold(), newFormat.bold(), "font-weight=\"bold\"", "font-weight=\"normal\"");
    res += attribute(newFormat.italic() != oldFormat.italic(), newFormat.italic(), "font-style=\"italic\"", "font-style=\"normal\"");
    res += attribute(newFormat.underline() != oldFormat.underline(), newFormat.underline(), "underline=\"1\"", "underline=\"0\"");
    res += attribute(newFormat.strike() != oldFormat.strike(), newFormat.strike(), "line-through=\"1\"", "line-though=\"0\"");
    res += attribute(newFormat.fontFamily() != oldFormat.fontFamily(), true, QString("font-family=\"%1\"").arg(newFormat.fontFamily()), "");
    bool needSize = newFormat.fontSize() < 0.99 * oldFormat.fontSize() || newFormat.fontSize() > 1.01 * oldFormat.fontSize();
    res += attribute(needSize, true, QString("font-size=\"%1\"").arg(newFormat.fontSize()), "");
    //LOGD("updateFormat() res '%s'", qPrintable(res));
    oldFormat = newFormat;
    return res;
}
} // namespace Ms
