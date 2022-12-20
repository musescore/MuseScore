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

#include "errordetailsmodel.h"

#include <QApplication>
#include <QClipboard>
#include <QTextDocument>

using namespace mu::ui;

static QString toPlainText(const QString& html)
{
    QTextDocument doc;
    doc.setHtml(html);

    return doc.toPlainText();
}

ErrorDetailsModel::ErrorDetailsModel(QObject* parent)
    : QObject(parent)
{
}

QStringList ErrorDetailsModel::details() const
{
    return m_details;
}

void ErrorDetailsModel::load(const QString& detailedText)
{
    m_details = detailedText.split('\n');

    emit detailsChanged();
}

bool ErrorDetailsModel::copyDetailsToClipboard()
{
    QClipboard* clipboard = QGuiApplication::clipboard();
    if (!clipboard || m_details.empty()) {
        return false;
    }

    QString text;
    int lastIdx = m_details.size() - 1;

    for (int i = 0; i < m_details.size(); ++i) {
        text += toPlainText(m_details[i]);

        if (i != lastIdx) {
            text += "\n";
        }
    }

    clipboard->setText(text);

    return true;
}
