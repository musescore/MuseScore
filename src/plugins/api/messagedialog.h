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
#ifndef MU_PLUGINS_MESSAGEDIALOG_H
#define MU_PLUGINS_MESSAGEDIALOG_H

#include <QObject>
#include <QString>

#include "global/modularity/ioc.h"
#include "global/iinteractive.h"

namespace mu::plugins::api {
class MessageDialog : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged FINAL)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged FINAL)
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged FINAL)

    Inject<framework::IInteractive> interactive;

public:
    MessageDialog();
    QString text() const;
    void setText(const QString& newText);

    bool visible() const;
    void setVisible(bool newVisible);

    QString title() const;
    void setTitle(const QString& newTitle);

    Q_INVOKABLE void open();

signals:
    void textChanged();
    void visibleChanged();
    void titleChanged();

    void accepted();

private:

    void doOpen(const QString& title, const QString& text);

    QString m_text;
    bool m_visible = false;
    QString m_title;
};
}

#endif // MU_PLUGINS_MESSAGEDIALOG_H
