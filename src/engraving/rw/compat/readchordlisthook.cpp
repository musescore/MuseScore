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
#include "readchordlisthook.h"

#include "style/style.h"
#include "rw/xml.h"
#include "libmscore/masterscore.h"

#include "log.h"

using namespace mu::engraving::compat;
using namespace mu::engraving;

ReadChordListHook::ReadChordListHook(Score* score)
    : m_score(score)
{
    if (m_score) {
        m_oldChordDescriptionFile = m_score->style().styleSt(Sid::chordDescriptionFile);
    }
}

void ReadChordListHook::read(XmlReader& e)
{
    if (!m_score) {
        return;
    }

    m_score->chordList()->clear();
    m_score->chordList()->read(e);
    m_score->chordList()->setCustomChordList(true);

    m_chordListTag = true;
}

void ReadChordListHook::validate()
{
    if (!m_score) {
        return;
    }

    // if we just specified a new chord description file
    // and didn't encounter a ChordList tag
    // then load the chord description file

    MStyle& style = m_score->style();
    ChordList* chordList = m_score->chordList();

    String newChordDescriptionFile = style.styleSt(Sid::chordDescriptionFile);
    if (newChordDescriptionFile != m_oldChordDescriptionFile && !m_chordListTag) {
        if (!newChordDescriptionFile.startsWith(u"chords_") && style.styleSt(Sid::chordStyle) == "std") {
            // should not normally happen,
            // but treat as "old" (114) score just in case
            style.set(Sid::chordStyle, String(u"custom"));
            style.set(Sid::chordsXmlFile, true);
            LOGD("StyleData::load: custom chord description file %s with chordStyle == std", muPrintable(newChordDescriptionFile));
        }

        bool custom = style.styleSt(Sid::chordStyle) == "custom";
        chordList->setCustomChordList(custom);

        chordList->unload();
    }

    // make sure we have a chordlist
    if (!m_chordListTag) {
        chordList->checkChordList(style);
    }
}
