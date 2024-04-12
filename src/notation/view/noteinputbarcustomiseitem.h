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
#ifndef MU_NOTATION_NOTEINPUTBARCUSTOMISEITEM_H
#define MU_NOTATION_NOTEINPUTBARCUSTOMISEITEM_H

#include <QString>

#include "ui/view/iconcodes.h"
#include "uicomponents/view/selectableitemlistmodel.h"

namespace mu::notation {
class NoteInputBarCustomiseItem : public muse::uicomponents::SelectableItemListModel::Item
{
    Q_OBJECT

    Q_PROPERTY(ItemType type READ type CONSTANT)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(int icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(bool checked READ checked WRITE setChecked NOTIFY checkedChanged)

public:
    enum ItemType {
        UNDEFINED = -1,
        ACTION,
        SEPARATOR
    };
    Q_ENUM(ItemType)

    explicit NoteInputBarCustomiseItem(const ItemType& type, QObject* parent = nullptr);

    ItemType type() const;
    QString title() const;
    int icon() const;
    bool checked() const;

    Q_INVOKABLE QString id() const;
    void setId(const QString& id);

public slots:
    void setTitle(QString title);
    void setIcon(muse::ui::IconCode::Code icon);
    void setChecked(bool checked);

signals:
    void titleChanged();
    void iconChanged();
    void checkedChanged(bool checked);

private:

    QString m_id;
    ItemType m_type = ItemType::UNDEFINED;
    QString m_title;
    muse::ui::IconCode::Code m_icon = muse::ui::IconCode::Code::NONE;
    bool m_checked = false;
};
}

#endif // MU_NOTATION_NOTEINPUTBARCUSTOMISEITEM_H
