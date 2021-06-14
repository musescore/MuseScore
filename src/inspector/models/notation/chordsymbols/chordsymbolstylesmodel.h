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
#ifndef CHORDSYMBOLSTYLESMODEL_H
#define CHORDSYMBOLSTYLESMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include "modularity/ioc.h"
#include "context/iglobalcontext.h"


namespace mu::inspector {
class ChordSymbolStylesModel : public QAbstractListModel
{

    INJECT(inspector, context::IGlobalContext, globalContext)

    Q_OBJECT

public:
    explicit ChordSymbolStylesModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void setChordStyle(QString styleName) const;

private:
    enum RoleNames {
        StyleNameRole = Qt::UserRole + 1,
        FileRole
    };
    // TODO: Reconsider the required roles (attributes)
    struct ChordSymbolStyle{
        QString styleName;
        QString file;
    };
    // TODO: Make the list dynamic (get info from styles)
    QList<ChordSymbolStyle> m_styles = {
        {"Standard","chords_std.xml"},
        {"Jazz","chords_jazz.xml"},
        {"Jazz","chord_jazz.xml"}
    };
};
}
#endif // CHORDSYMBOLSTYLESMODEL_H
