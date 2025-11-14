/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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
#include <QString>
#include <QUrl>

#include "global/modularity/ioc.h"
#include "global/iinteractive.h"

namespace muse::extensions::apiv1 {
class FileDialog : public QObject, public Injectable
{
    Q_OBJECT
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY typeChanged FINAL)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged FINAL)
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged FINAL)
    Q_PROPERTY(QString folder READ folder WRITE setFolder NOTIFY folderChanged FINAL)
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged FINAL)
    Q_PROPERTY(QUrl fileUrl READ fileUrl WRITE setFileUrl FINAL)
    Q_PROPERTY(QList<QUrl> fileUrls READ fileUrls WRITE setFileUrls FINAL)

    Inject<IInteractive> interactive = { this };

public:
    FileDialog(QObject* parent = nullptr);

    enum Type {
        Load = 0,
        Save
    };
    Q_ENUM(Type)

    Type type() const;
    void setType(Type newType);

    QString title() const;
    void setTitle(const QString& newTitle);
    bool visible() const;
    void setVisible(bool newVisible);

    Q_INVOKABLE void open();
    Q_INVOKABLE void close();

    QString folder() const;
    void setFolder(const QString& newFolder);

    QString filePath() const;
    void setFilePath(const QString& newFilePath);

    QUrl fileUrl() const;
    void setFileUrl(const QUrl& newFileUrl);

    QList<QUrl> fileUrls() const;
    void setFileUrls(const QList<QUrl>& newFileUrls);

signals:
    void accepted();
    void rejected();

    void typeChanged();
    void titleChanged();
    void visibleChanged();
    void folderChanged();
    void filePathChanged();

private:

    QString doOpen(const QString& title, const QString& path);

    QString m_title;
    bool m_visible = false;
    QString m_folder;
    QString m_filePath;
    Type m_type = Type::Load;
};
}
