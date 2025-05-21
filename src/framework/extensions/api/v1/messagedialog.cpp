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

using namespace muse::extensions::apiv1;

MessageDialog::MessageDialog(QObject* parent)
    : QObject(parent), Injectable(muse::iocCtxForQmlObject(this)) {}

void MessageDialog::doOpen(const QString& contentTitle, const QString& text, const QString& detailed, const QVariantList& buttons)
{
    //! NOTE Minimum compatibility for the current ones to work.
    //! It would be nice to change a lot of things.

    IInteractive::Buttons btns;
    for (const QVariant& b : buttons) {
        btns.push_back(static_cast<IInteractive::Button>(b.toInt()));
    }

    // info
    if (btns.size() <= 1) {
        IInteractive::Text t;
        t.text = text.toStdString();
        t.detailedText = detailed.toStdString();
        interactive()->error(contentTitle.toStdString(), t).onResolve(this, [this](const IInteractive::Result&) {
            emit accepted();
        });
    }
    //
    else {
        std::string txt = text.toStdString();
        if (!detailed.isEmpty()) {
            txt += "\n\n";
            txt += detailed.toStdString();
        }

        auto promise = interactive()->question(contentTitle.toStdString(), txt, btns);
        promise.onResolve(this, [this](const IInteractive::Result& res) {
            if (res.isButton(IInteractive::Button::Ok)) {
                emit accepted();
            } else {
                emit rejected();
            }
        });
    }
}

void MessageDialog::open()
{
    setVisible(true);
}

void MessageDialog::close()
{
    setVisible(false);
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
        doOpen(m_title, m_text, m_detailedText, m_standardButtons);
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

QString MessageDialog::detailedText() const
{
    return m_detailedText;
}

void MessageDialog::setDetailedText(const QString& newDetailedText)
{
    if (m_detailedText == newDetailedText) {
        return;
    }
    m_detailedText = newDetailedText;
    emit detailedTextChanged();
}

QVariantList MessageDialog::standardButtons() const
{
    return m_standardButtons;
}

void MessageDialog::setStandardButtons(const QVariantList& newStandardButtons)
{
    if (m_standardButtons == newStandardButtons) {
        return;
    }
    m_standardButtons = newStandardButtons;
    emit standardButtonsChanged();
}
