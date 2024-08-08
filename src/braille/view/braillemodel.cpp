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

#include "braillemodel.h"

using namespace mu::braille;
using namespace mu::notation;

namespace mu::engraving {
BrailleModel::BrailleModel(QObject* parent)
    : QObject(parent)
{
}

QString BrailleModel::brailleInfo() const
{
    return notationBraille() ? QString::fromStdString(notationBraille()->brailleInfo().val) : QString();
}

QString BrailleModel::keys() const
{
    return notationBraille() ? QString::fromStdString(notationBraille()->keys().val) : QString();
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

void BrailleModel::setCursorPosition(int pos)
{
    if (!notationBraille()) {
        return;
    }

    if (notationBraille()->cursorPosition().val == pos) {
        return;
    }

    notationBraille()->setCursorPosition(pos);
}

void BrailleModel::setKeys(const QString& sequence)
{
    if (!notationBraille()) {
        return;
    }
    notationBraille()->setKeys(sequence);
}

bool BrailleModel::enabled() const
{
    return brailleConfiguration()->braillePanelEnabled();
}

void BrailleModel::setEnabled(bool e)
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

int BrailleModel::mode() const
{
    return notationBraille()->mode().val;
}

void BrailleModel::setMode(int m)
{
    if (!notationBraille()) {
        return;
    }

    if (notationBraille()->mode().val == m) {
        return;
    }

    notationBraille()->setMode(static_cast<BrailleMode>(m));

    emit brailleModeChanged();
}

bool BrailleModel::isNavigationMode()
{
    if (!notationBraille()) {
        return false;
    }
    return notationBraille()->isNavigationMode();
}

bool BrailleModel::isBrailleInputMode()
{
    if (!notationBraille()) {
        return false;
    }
    return notationBraille()->isBrailleInputMode();
}

QString BrailleModel::cursorColor() const
{
    QString color = notationBraille() ? QString::fromStdString(notationBraille()->cursorColor().val) : QString();
    return color;
}

void BrailleModel::load()
{
    TRACEFUNC;

    if (notationBraille()) {
        notationBraille()->setMode(BrailleMode::Navigation);
    }

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
    listenKeys();
    listenBraillePanelEnabledChanges();
    listenBrailleModeChanges();
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

void BrailleModel::listenKeys()
{
    if (!notationBraille()) {
        return;
    }

    notationBraille()->keys().ch.onReceive(this, [this](const std::string&) {
        emit keysFired();
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

void BrailleModel::listenBrailleModeChanges()
{
    if (!notationBraille()) {
        return;
    }
    notationBraille()->mode().ch.onReceive(this, [this](const int) {
        emit brailleModeChanged();
    });
}

INotationPtr BrailleModel::notation() const
{
    return context()->currentNotation();
}
}
