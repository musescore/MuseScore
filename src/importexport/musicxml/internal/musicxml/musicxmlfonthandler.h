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

#ifndef __MUSICXMLFONTHANDLER_H__
#define __MUSICXMLFONTHANDLER_H__

#include "engraving/dom/text.h"

namespace mu::engraving {
class XmlWriter;
//---------------------------------------------------------
//   MScoreTextToMXML
//---------------------------------------------------------

class MScoreTextToMXML
{
public:
    MScoreTextToMXML(const QString& tag, const QString& attr, const CharFormat& defFmt, const QString& mtf);
    static QString toPlainText(const QString& text);
    static QString toPlainTextPlusSymbols(const std::list<TextFragment>& list);
    static bool split(const std::list<TextFragment>& in, const int pos, const int len, std::list<TextFragment>& left,
                      std::list<TextFragment>& mid, std::list<TextFragment>& right);
    void writeTextFragments(const std::list<TextFragment>& fr, XmlWriter& xml);

private:
    QString updateFormat();
    QString attribs;
    QString tagname;
    CharFormat oldFormat;
    CharFormat newFormat;
    QString musicalTextFont;
};
} // namespace Ms

#endif
