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
#pragma once

#include <QAbstractListModel>
#include <QSortFilterProxyModel>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

namespace mu::notation {
class SelectionFilterSection
{
    Q_GADGET
public:
    enum Section
    {
        UNDEFINED,
        VOICES,
        CHORDS,
        NOTATION_ELEMENTS
    };
    Q_ENUM(Section)
};

class SelectionFilterModel : public QAbstractListModel, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(bool enabled READ enabled NOTIFY enabledChanged)

    muse::Inject<context::IGlobalContext> globalContext = { this };

public:
    explicit SelectionFilterModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE QSortFilterProxyModel* proxyModelForSection(const SelectionFilterSection::Section& section);

    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& data, int role) override;
    int rowCount(const QModelIndex& parent = {}) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool enabled() const;
    SelectionFilterType typeForRow(int row) const;

signals:
    void enabledChanged();

private:
    enum Roles {
        TitleRole = Qt::UserRole + 1,
        IsSelectedRole,
        IsIndeterminateRole
    };

    INotationPtr currentNotation() const;
    INotationInteractionPtr currentNotationInteraction() const;

    unsigned int filteredTypes() const;
    bool isFiltered(SelectionFilterType type) const;
    void setFiltered(SelectionFilterType type, bool filtered);

    QString titleForType(SelectionFilterType type) const;

    QList<SelectionFilterType> m_types;
};

class SelectionFilterProxyModel : public QSortFilterProxyModel
{
public:
    explicit SelectionFilterProxyModel(const SelectionFilterSection::Section& section, QObject* parent = nullptr);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex&) const override;

private:
    SelectionFilterSection::Section m_section = SelectionFilterSection::Section::UNDEFINED;
};
}
