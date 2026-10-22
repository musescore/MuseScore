/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#pragma once

#include <QObject>
#include <QPointer>
#include <QVariantList>
#include <qqmlintegration.h>

namespace muse::uicomponents {
class TextInputMenuModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QObject* textInput READ textInput WRITE setTextInput NOTIFY textInputChanged)

public:
    explicit TextInputMenuModel(QObject* parent = nullptr);

    QObject* textInput() const;
    void setTextInput(QObject* textInput);

    Q_INVOKABLE QVariantList items();

    // Also saves the selection, as opening the menu resets it
    Q_INVOKABLE void handleMenuItem(const QString& itemId);

signals:
    void textInputChanged();

private:
    QPointer<QObject> m_textInput;
    int m_selectionStart = 0;
    int m_selectionEnd = 0;
};
}
