//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_VST_PLUGINLISTMODEL_H
#define MU_VST_PLUGINLISTMODEL_H

#include <QAbstractListModel>

#include "internal/vstscanner.h"

namespace mu {
namespace vst {
class PluginListModel : public QAbstractListModel
{
    Q_OBJECT

    enum {
        UidRole     = Qt::UserRole,
        NameRole    = Qt::UserRole + 1
    };

public:
    PluginListModel(QObject* parent = nullptr);
    PluginListModel(std::shared_ptr<VSTScanner> scaner, QObject* parent = nullptr);

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE void update();
    const Plugin& item(unsigned int index);

private:
    std::shared_ptr<VSTScanner> m_scanner = nullptr;
    QList<Plugin> m_plugins = {};
    static const Plugin nullPlugin;
};
} // namespace vst
} // namespace mu

#endif // MU_VST_PLUGINLISTMODEL_H
