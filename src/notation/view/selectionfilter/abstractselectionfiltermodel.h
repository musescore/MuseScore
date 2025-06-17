/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

namespace mu::notation {
class AbstractSelectionFilterModel : public QAbstractListModel, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(bool enabled READ enabled NOTIFY enabledChanged)

    muse::Inject<context::IGlobalContext> globalContext = { this };

public:
    explicit AbstractSelectionFilterModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();

    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& data, int role) override;
    int rowCount(const QModelIndex& parent = {}) const override;
    QHash<int, QByteArray> roleNames() const override;

    virtual bool enabled() const;

signals:
    void enabledChanged();

protected:
    enum Roles {
        IsAllowedRole = Qt::UserRole + 1,
        TitleRole,
        IsSelectedRole,
        IsIndeterminateRole
    };

    virtual void loadTypes() = 0;

    virtual SelectionFilterTypesVariant getAllMask() const = 0;
    virtual SelectionFilterTypesVariant getNoneMask() const = 0;

    INotationInteractionPtr currentNotationInteraction() const;
    INotationSelectionFilterPtr currentNotationSelectionFilter() const;

    virtual bool isFiltered(const SelectionFilterTypesVariant& variant) const;
    virtual void setFiltered(const SelectionFilterTypesVariant& variant, bool filtered);

    virtual bool isAllowed(const SelectionFilterTypesVariant&) const { return true; }
    virtual QString titleForType(const SelectionFilterTypesVariant& variant) const = 0;
    bool isIndeterminate(const SelectionFilterTypesVariant& variant) const;

    virtual void onSelectionChanged();
    virtual void onNotationChanged();
    void notifyAboutDataChanged(const QModelIndex& index, const SelectionFilterTypesVariant& variant);

    QList<SelectionFilterTypesVariant> m_types;

private:
    INotationPtr currentNotation() const;
};
}
