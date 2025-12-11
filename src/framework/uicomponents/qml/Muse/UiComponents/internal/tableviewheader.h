/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include <qqmlintegration.h>

#include <QObject>

#include "uicomponents/qml/Muse/UiComponents/menuitem.h"

namespace muse::uicomponents {
namespace TableViewCellType {
Q_NAMESPACE;
QML_ELEMENT;

enum class Type {
    Undefined = 0,
    Bool,
    Int,
    Double,
    String,
    List,
#ifndef NO_QT_SUPPORT
    Color,
#endif

    UserType = 100,
    UserTypeEnd = 1000
};

Q_ENUM_NS(Type)
}

namespace TableViewCellEditMode {
Q_NAMESPACE;
QML_ELEMENT;

enum class Mode {
    StartInEdit = 0,
    DoubleClick
};

Q_ENUM_NS(Mode)
}

class TableViewHeader : public QObject
{
    Q_OBJECT
    QML_ELEMENT;

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)

    Q_PROPERTY(muse::uicomponents::TableViewCellType::Type cellType READ cellType NOTIFY cellTypeChanged FINAL)
    Q_PROPERTY(muse::uicomponents::TableViewCellEditMode::Mode cellEditMode READ cellEditMode NOTIFY cellEditModeChanged FINAL)

    Q_PROPERTY(int preferredWidth READ preferredWidth WRITE setPreferredWidth NOTIFY preferredWidthChanged FINAL)

    Q_PROPERTY(
        muse::uicomponents::MenuItemList availableFormats READ availableFormats WRITE setAvailableFormats NOTIFY availableFormatsChanged FINAL)
    Q_PROPERTY(QString currentFormatId READ currentFormatId WRITE setCurrentFormatId NOTIFY currentFormatIdChanged FINAL)

public:
    explicit TableViewHeader(QObject* parent = nullptr);

    QString title() const;
    void setTitle(const QString& title);

    int preferredWidth() const;
    void setPreferredWidth(int width);

    TableViewCellType::Type cellType() const;
    void setCellType(TableViewCellType::Type type);

    TableViewCellEditMode::Mode cellEditMode() const;
    void setCellEditMode(TableViewCellEditMode::Mode mode);

    MenuItemList availableFormats() const;
    void setAvailableFormats(const MenuItemList& formats);

    QString currentFormatId() const;
    void setCurrentFormatId(const QString& id);

signals:
    void titleChanged();

    void cellTypeChanged();
    void cellEditModeChanged();

    void preferredWidthChanged();

    void availableFormatsChanged();
    void currentFormatIdChanged();

    void selectedChanged();

private:
    QString m_title;

    TableViewCellType::Type m_cellType = TableViewCellType::Type::Undefined;
    TableViewCellEditMode::Mode m_cellEditMode = TableViewCellEditMode::Mode::StartInEdit;

    int m_preferredWidth = 0;

    MenuItemList m_availableFormats;
    QString m_currentFormatId;
};
}
