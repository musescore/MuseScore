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

#include "braillemodel.h"
#include "types/translatablestring.h"
#include "log.h"

namespace mu::notation {
BrailleModel::BrailleModel(QObject* parent)
    : QObject(parent)
{
    load();
}

QString BrailleModel::brailleInfo() const
{
    return notationBraille() ? QString::fromStdString(notationBraille()->brailleInfo().val) : QString();
}

QString BrailleModel::shortcut() const
{
    return notationBraille() ? QString::fromStdString(notationBraille()->shortcut().val) : QString();
}

int BrailleModel::cursorPosition() const
{
    return notationBraille() ? notationBraille()->cursorPosition().val : 0;
}

int BrailleModel::currentItemPositionStart() const
{
    return notationBraille() ? notationBraille()->currentItemPositionStart().val : 0;
}

int BrailleModel::currentItemPositionEnd() const
{
    return notationBraille() ? notationBraille()->currentItemPositionEnd().val : 0;
}

void BrailleModel::setCursorPosition(int pos) const
{
    if (!notationBraille()) {
        return;
    }

    if (notationBraille()->cursorPosition().val == pos) {
        return;
    }

    notationBraille()->setCursorPosition(pos);
}

void BrailleModel::setShortcut(const QString& sequence) const
{
    if (!notationBraille()) {
        return;
    }
    notationBraille()->setShortcut(sequence);
}

bool BrailleModel::enabled() const
{
    //if(!notationBraille()) return false;
    //return notationBraille()->enabled().val;
    return brailleConfiguration()->braillePanelEnabled();
}

void BrailleModel::setEnabled(bool e) const
{
    if (!notationBraille()) {
        return;
    }
    if (notationBraille()->enabled().val == e) {
        return;
    }
    notationBraille()->setEnabled(e);
    emit braillePanelEnabledChanged();
}

void BrailleModel::load()
{
    TRACEFUNC;

    onCurrentNotationChanged();
    context()->currentNotationChanged().onNotify(this, [this]() {
        onCurrentNotationChanged();
    });
}

void BrailleModel::onCurrentNotationChanged()
{
    if (!notation()) {
        return;
    }

    listenChangesInnotationBraille();
    listenCurrentItemChanges();
    listenShortcuts();
    listenBraillePanelEnabledChanges();
    listenCursorPositionChanges();
}

void BrailleModel::listenChangesInnotationBraille()
{
    if (!notationBraille()) {
        return;
    }

    notationBraille()->brailleInfo().ch.onReceive(this, [this](const std::string&) {
        emit brailleInfoChanged();
    });
}

void BrailleModel::listenShortcuts()
{
    if (!notationBraille()) {
        return;
    }

    notationBraille()->shortcut().ch.onReceive(this, [this](const std::string&) {
        emit shortcutFired();
    });
}

void BrailleModel::listenCursorPositionChanges()
{
    if (!notationBraille()) {
        return;
    }
    notationBraille()->cursorPosition().ch.onReceive(this, [this](const int) {
        emit cursorPositionChanged();
    });
}

void BrailleModel::listenCurrentItemChanges()
{
    if (!notationBraille()) {
        return;
    }
    notationBraille()->currentItemPositionStart().ch.onReceive(this, [this](int) {
        emit currentItemChanged();
    });

    notationBraille()->currentItemPositionEnd().ch.onReceive(this, [this](int) {
        emit currentItemChanged();
    });
}

void BrailleModel::listenBraillePanelEnabledChanges()
{
    if (!notationBraille()) {
        return;
    }
    notationBraille()->enabled().ch.onReceive(this, [this](const int) {
        emit braillePanelEnabledChanged();
    });
}

INotationPtr BrailleModel::notation() const
{
    return context()->currentNotation();
}
}
