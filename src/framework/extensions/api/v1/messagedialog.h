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
#ifndef MUSE_EXTENSIONS_APIV1_MESSAGEDIALOG_H
#define MUSE_EXTENSIONS_APIV1_MESSAGEDIALOG_H

#include <QObject>
#include <QString>

#include "global/modularity/ioc.h"
#include "global/iinteractive.h"

namespace muse::extensions::apiv1 {
class StandardButton
{
    Q_GADGET
public:
    enum Button {
        Ok = static_cast<int>(IInteractive::Button::Ok),
        Cancel = static_cast<int>(IInteractive::Button::Cancel),
    };
    Q_ENUM(Button)
};

class MessageDialog : public QObject, public Injectable
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged FINAL)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged FINAL)
    Q_PROPERTY(QString detailedText READ detailedText WRITE setDetailedText NOTIFY detailedTextChanged FINAL)
    Q_PROPERTY(QVariantList standardButtons READ standardButtons WRITE setStandardButtons NOTIFY standardButtonsChanged FINAL)
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged FINAL)

    Inject<IInteractive> interactive = { this };

public:

    MessageDialog(QObject* parent = nullptr);

    QString text() const;
    void setText(const QString& newText);

    bool visible() const;
    void setVisible(bool newVisible);

    QString title() const;
    void setTitle(const QString& newTitle);

    QString detailedText() const;
    void setDetailedText(const QString& newDetailedText);

    QVariantList standardButtons() const;
    void setStandardButtons(const QVariantList& newStandardButtons);

    Q_INVOKABLE void open();
    Q_INVOKABLE void close();

signals:
    void textChanged();
    void visibleChanged();
    void titleChanged();
    void detailedTextChanged();
    void standardButtonsChanged();

    void accepted();
    void rejected();

private:

    void doOpen(const QString& title, const QString& text, const QString& detailed, const QVariantList& buttons);

    QString m_text;
    bool m_visible = false;
    QString m_title;
    QString m_detailedText;
    QVariantList m_standardButtons;
};
}

#endif // MUSE_EXTENSIONS_APIV1_MESSAGEDIALOG_H
