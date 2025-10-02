/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "editstyle.h"

#include "../dom/chordlist.h"
#include "../dom/score.h"
#include "editing/editsystemlocks.h"

using namespace mu::engraving;

static void changeChordStyle(Score* score)
{
    const MStyle& style = score->style();
    score->chordList()->unload();
    double emag = style.styleD(Sid::chordExtensionMag);
    double eadjust = style.styleD(Sid::chordExtensionAdjust);
    double mmag = style.styleD(Sid::chordModifierMag);
    double madjust = style.styleD(Sid::chordModifierAdjust);
    double stackedmmag = style.styleD(Sid::chordStackedModifierMag);
    bool mstackModifiers = style.styleB(Sid::verticallyStackModifiers);
    bool mexcludeModsHAlign = style.styleB(Sid::chordAlignmentExcludeModifiers);
    String msymbolFont = style.styleSt(Sid::musicalTextFont);
    ChordStylePreset preset = style.styleV(Sid::chordStyle).value<ChordStylePreset>();
    score->chordList()->configureAutoAdjust(emag, eadjust, mmag, madjust, stackedmmag, mstackModifiers, mexcludeModsHAlign, msymbolFont,
                                            preset);
    if (score->style().styleB(Sid::chordsXmlFile)) {
        score->chordList()->read(u"chords.xml");
    }
    score->chordList()->read(style.styleSt(Sid::chordDescriptionFile));
    score->chordList()->setCustomChordList(style.styleV(Sid::chordStyle).value<ChordStylePreset>() == ChordStylePreset::CUSTOM);
}

//---------------------------------------------------------
//   ChangeStyle
//----------------------------------------------------------

ChangeStyle::ChangeStyle(Score* s, const MStyle& st, const bool overlapOnly)
    : score(s), style(st), overlap(overlapOnly)
{
}

StyleIdSet ChangeStyle::changedIds() const
{
    StyleIdSet result;
    for (int _sid = 0; _sid < static_cast<int>(Sid::STYLES); ++_sid) {
        Sid sid = static_cast<Sid>(_sid);
        if (score->style().styleV(sid) != style.value(sid)) {
            result.insert(sid);
        }
    }
    return result;
}

void ChangeStyle::flip(EditData*)
{
    MStyle tmp = score->style();

    if (score->style().styleV(Sid::concertPitch) != style.value(Sid::concertPitch)) {
        score->cmdConcertPitchChanged(style.value(Sid::concertPitch).toBool());
    }
    if (score->style().styleV(Sid::musicalSymbolFont) != style.value(Sid::musicalSymbolFont)) {
        score->setEngravingFont(score->engravingFonts()->fontByName(style.styleSt(Sid::musicalSymbolFont).toStdString()));
    }

    score->setStyle(style, overlap);
    changeChordStyle(score);
    if (tmp.spatium() != style.spatium()) {
        score->spatiumChanged(tmp.spatium(), style.spatium());
    }
    score->styleChanged();
    style = tmp;
}

void ChangeStyle::undo(EditData* ed)
{
    overlap = false;
    UndoCommand::undo(ed);
}

//---------------------------------------------------------
//   ChangeStyleValues
//----------------------------------------------------------

static void changeStyleValue(Score* score, Sid idx, const PropertyValue& oldValue, const PropertyValue& newValue)
{
    score->style().set(idx, newValue);
    switch (idx) {
    case Sid::chordExtensionMag:
    case Sid::chordExtensionAdjust:
    case Sid::chordModifierMag:
    case Sid::chordModifierAdjust:
    case Sid::chordDescriptionFile:
    case Sid::verticallyStackModifiers:
    case Sid::chordAlignmentExcludeModifiers:
    case Sid::musicalTextFont: {
        changeChordStyle(score);
    }
    break;
    case Sid::spatium:
        score->spatiumChanged(oldValue.toDouble(), newValue.toDouble());
        break;
    case Sid::defaultsVersion:
        score->style().setDefaultStyleVersion(newValue.toInt());
        break;
    case Sid::createMultiMeasureRests:
        if (oldValue.toBool() == true && newValue.toBool() == false) {
            EditSystemLocks::removeSystemLocksContainingMMRests(score);
        }
        break;
    default:
        break;
    }
}

void ChangeStyleValues::flip(EditData*)
{
    if (!m_score) {
        return;
    }

    bool styleChanged = false;
    const MStyle& style = m_score->style();

    for (auto& pair : m_values) {
        PropertyValue oldValue = style.styleV(pair.first);
        if (oldValue == pair.second) {
            continue;
        }

        changeStyleValue(m_score, pair.first, oldValue, pair.second);
        pair.second = oldValue;

        styleChanged = true;
    }

    if (styleChanged) {
        m_score->styleChanged();
    }
}
