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

#include "io/iodevice.h"
#include "io/buffer.h"

#include "notationbraille.h"

#include "translation.h"

#include "libmscore/masterscore.h"
#include "libmscore/spanner.h"
#include "libmscore/segment.h"
#include "libmscore/slur.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/sig.h"
#include "libmscore/measure.h"

#include "braille/internal/braille.h"
#include "braille/internal/louis.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::async;
using namespace mu::engraving;
using namespace mu::io;

void NotationBraille::init()
{
    setEnabled(brailleConfiguration()->braillePanelEnabled());
    setCurrentItemPosition(-1, -1);

    path_t tablesdir = tablesDefaultDirPath();
    setTablesDir(tablesdir.toStdString().c_str());
    initTables(tablesdir.toStdString());

    std::string welcome = braille_translate(table_for_literature.c_str(), "Welcome to MuseScore 4.0!");
    setBrailleInfo(QString(welcome.c_str()));

    brailleConfiguration()->braillePanelEnabledChanged().onNotify(this, [this]() {
        bool enabled = brailleConfiguration()->braillePanelEnabled();
        setEnabled(enabled);
    });

    updateTableForLyricsFromPreferences();
    brailleConfiguration()->brailleTableChanged().onNotify(this, [this]() {
        updateTableForLyricsFromPreferences();
    });

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        if (notation()) {
            doBraille(true);

            notation()->interaction()->selectionChanged().onNotify(this, [this]() {
                doBraille();
            });
        }
    });
}

void NotationBraille::updateTableForLyricsFromPreferences()
{
    QString table = brailleConfiguration()->brailleTable();
    int startPos = table.indexOf('[');
    int endPos = table.indexOf(']');
    if (startPos != -1 && endPos != -1) {
        table = table.mid(startPos + 1, endPos - startPos - 1);
        QString table_full_path = QString::fromStdString(tables_dir) + "/" + table;
        if (check_tables(table_full_path.toStdString().c_str()) == 0) {
            updateTableForLyrics(table.toStdString());
        } else {
            LOGD() << "Table check error!";
        }
    }
}

void NotationBraille::doBraille(bool force)
{
    if (brailleConfiguration()->braillePanelEnabled()) {
        EngravingItem* e = nullptr;
        Measure* m = nullptr;

        if (selection()->isSingle()) {
            e = selection()->element();
            m = e->findMeasure();
        } else if (selection()->isRange()) {
            for (auto el: selection()->elements()) {
                if (el->isMeasure()) {
                    m = toMeasure(el);
                    break;
                } else {
                    m = el->findMeasure();
                    if (m) {
                        break;
                    }
                }
            }
            e = m ? m : selection()->elements().front();
        } else if (selection()->isList()) {
            e = selection()->elements().front();
            m = e->findMeasure();
        }
        if (e) {
            if (!m) {
                brailleEngravingItems()->clear();
                Braille lb(score());
                bool res = lb.convertItem(e, brailleEngravingItems());
                if (!res) {
                    QString txt = e->accessibleInfo();
                    std::string braille = braille_long_translate(table_for_literature.c_str(), txt.toStdString());
                    brailleEngravingItems()->setBrailleStr(QString::fromStdString(braille));
                    setBrailleInfo(QString::fromStdString(braille));
                } else {
                    setBrailleInfo(brailleEngravingItems()->brailleStr());
                }
                current_measure = nullptr;
            } else {
                if (m != current_measure || force) {
                    brailleEngravingItems()->clear();
                    Braille lb(score());
                    lb.convertMeasure(m, brailleEngravingItems());
                    setBrailleInfo(brailleEngravingItems()->brailleStr());
                    current_measure = m;
                }
                std::pair<int, int> pos = brailleEngravingItems()->getBraillePos(e);
                if (pos.first != -1) {
                    setCurrentItemPosition(pos.first, pos.second);
                }
            }
        }
    }
}

mu::engraving::Score* NotationBraille::score()
{
    return notation()->elements()->msScore()->score();
}

mu::engraving::Selection* NotationBraille::selection()
{
    return &score()->selection();
}

mu::ValCh<std::string> NotationBraille::brailleInfo() const
{
    return m_brailleInfo;
}

mu::ValCh<int> NotationBraille::cursorPosition() const
{
    return m_cursorPosition;
}

mu::ValCh<int> NotationBraille::currentItemPositionStart() const
{
    return m_currentItemPositionStart;
}

mu::ValCh<int> NotationBraille::currentItemPositionEnd() const
{
    return m_currentItemPositionEnd;
}

mu::ValCh<std::string> NotationBraille::shortcut() const
{
    return m_shortcut;
}

mu::ValCh<bool> NotationBraille::enabled() const
{
    return m_enabled;
}

void NotationBraille::setEnabled(bool enabled)
{
    if (enabled == m_enabled.val) {
        return;
    }
    m_enabled.set(enabled);
}

mu::engraving::BrailleEngravingItems* NotationBraille::brailleEngravingItems()
{
    return &m_bei;
}

QString NotationBraille::getBrailleStr()
{
    return m_bei.brailleStr();
}

void NotationBraille::setBrailleInfo(const QString& info)
{
    std::string infoStd = info.toStdString();

    if (m_brailleInfo.val == infoStd) {
        return;
    }

    m_brailleInfo.set(infoStd);
}

void NotationBraille::setCursorPosition(const int pos)
{
    if (m_cursorPosition.val == pos) {
        return;
    }

    m_cursorPosition.set(pos);

    notation::EngravingItem* el = brailleEngravingItems()->getEngravingItem(pos);
    if (el != nullptr) {
        interaction()->select({ el });
    }
}

void NotationBraille::setCurrentItemPosition(const int start, const int end)
{
    if (m_currentItemPositionStart.val == start
        && m_currentItemPositionEnd.val == end) {
        return;
    }

    m_currentItemPositionStart.set(start);
    m_currentItemPositionEnd.set(end);
}

INotationPtr NotationBraille::notation()
{
    return globalContext()->currentNotation();
}

INotationInteractionPtr NotationBraille::interaction()
{
    return notation() ? notation()->interaction() : nullptr;
}

void NotationBraille::setShortcut(const QString& sequence)
{
    LOGD() << sequence;
    std::string seq = sequence.toStdString();
    m_shortcut.set(seq);

    if (seq == "Left") {
        interaction()->moveSelection(MoveDirection::Left, MoveSelectionType::Chord);
    } else if (seq == "Right") {
        interaction()->moveSelection(MoveDirection::Right, MoveSelectionType::Chord);
    } else if (seq == "Ctrl+Left") {
        interaction()->moveSelection(MoveDirection::Left, MoveSelectionType::Measure);
    } else if (seq == "Ctrl+Right") {
        interaction()->moveSelection(MoveDirection::Right, MoveSelectionType::Measure);
    } else if (seq == "Alt+Left") {
        interaction()->moveSelection(MoveDirection::Left, MoveSelectionType::EngravingItem);
    } else if (seq == "Alt+Right") {
        interaction()->moveSelection(MoveDirection::Right, MoveSelectionType::EngravingItem);
    } else if (seq == "Ctrl+End") {
        interaction()->selectLastElement();
    } else if (seq == "Ctrl+Home") {
        interaction()->selectFirstElement();
    }// else if(shortcutsController()->isRegistered(seq)) {
     //   shortcutsController()->activate(seq);
     //}
}

path_t NotationBraille::tablesDefaultDirPath() const
{
    return globalConfiguration()->appDataPath() + "tables";
}
