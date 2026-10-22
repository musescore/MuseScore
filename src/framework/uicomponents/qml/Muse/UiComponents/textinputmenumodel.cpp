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

#include "textinputmenumodel.h"

#include <QHash>
#include <QKeySequence>
#include <QVariantMap>

#include "translation.h"
#include "ui/view/iconcodes.h"

using namespace muse::uicomponents;
using namespace muse::ui;

static const QString CUT_ID("cut");
static const QString COPY_ID("copy");
static const QString PASTE_ID("paste");
static const QString SELECT_ALL_ID("selectAll");

static QVariantMap makeItem(const QString& id, const QString& title, IconCode::Code icon, QKeySequence::StandardKey key, bool enabled)
{
    return {
        { "id", id },
        { "title", title },
        { "icon", static_cast<int>(icon) },
        { "shortcuts", QKeySequence(key).toString(QKeySequence::NativeText) },
        { "enabled", enabled }
    };
}

TextInputMenuModel::TextInputMenuModel(QObject* parent)
    : QObject(parent)
{
}

QObject* TextInputMenuModel::textInput() const
{
    return m_textInput;
}

void TextInputMenuModel::setTextInput(QObject* textInput)
{
    if (m_textInput == textInput) {
        return;
    }

    m_textInput = textInput;
    emit textInputChanged();
}

QVariantList TextInputMenuModel::items()
{
    if (!m_textInput) {
        return {};
    }

    m_selectionStart = m_textInput->property("selectionStart").toInt();
    m_selectionEnd = m_textInput->property("selectionEnd").toInt();

    const bool readOnly = m_textInput->property("readOnly").toBool();
    const bool hasSelection = !m_textInput->property("selectedText").toString().isEmpty();
    const bool canPaste = m_textInput->property("canPaste").toBool();
    const bool hasText = !m_textInput->property("text").toString().isEmpty();

    return {
        makeItem(CUT_ID, muse::qtrc("action", "Cut"), IconCode::Code::CUT, QKeySequence::Cut, !readOnly && hasSelection),
        makeItem(COPY_ID, muse::qtrc("action", "Copy"), IconCode::Code::COPY, QKeySequence::Copy, hasSelection),
        makeItem(PASTE_ID, muse::qtrc("action", "Paste"), IconCode::Code::PASTE, QKeySequence::Paste, !readOnly && canPaste),
        QVariantMap(),
        makeItem(SELECT_ALL_ID, muse::qtrc("action", "Select all"), IconCode::Code::NONE, QKeySequence::SelectAll, hasText)
    };
}

void TextInputMenuModel::handleMenuItem(const QString& itemId)
{
    static const QHash<QString, const char*> METHOD_BY_ITEM_ID {
        { CUT_ID, "cut" },
        { COPY_ID, "copy" },
        { PASTE_ID, "paste" },
        { SELECT_ALL_ID, "selectAll" },
    };

    const char* method = METHOD_BY_ITEM_ID.value(itemId, nullptr);
    if (!m_textInput || !method) {
        return;
    }

    QMetaObject::invokeMethod(m_textInput, "select", Q_ARG(int, m_selectionStart), Q_ARG(int, m_selectionEnd));
    QMetaObject::invokeMethod(m_textInput, method);
}
