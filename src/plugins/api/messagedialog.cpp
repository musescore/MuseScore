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
#include "messagedialog.h"

using namespace mu::plugins::api;

MessageDialog::MessageDialog() {}

void MessageDialog::doOpen(const QString& title, const QString& text)
{
    interactive()->error(title.toStdString(), text.toStdString());
    emit accepted();
}

void MessageDialog::open()
{
    setVisible(true);
}

QString MessageDialog::text() const
{
    return m_text;
}

void MessageDialog::setText(const QString& newText)
{
    if (m_text == newText) {
        return;
    }
    m_text = newText;
    emit textChanged();
}

bool MessageDialog::visible() const
{
    return m_visible;
}

void MessageDialog::setVisible(bool newVisible)
{
    if (m_visible == newVisible) {
        return;
    }
    m_visible = newVisible;
    emit visibleChanged();

    if (m_visible) {
        doOpen(m_title, m_text);
    }
}

QString MessageDialog::title() const
{
    return m_title;
}

void MessageDialog::setTitle(const QString& newTitle)
{
    if (m_title == newTitle) {
        return;
    }
    m_title = newTitle;
    emit titleChanged();
}
