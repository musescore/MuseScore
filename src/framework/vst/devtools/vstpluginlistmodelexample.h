//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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

#ifndef MU_VST_VSTPLUGINLISTMODELEXAMPLE_H
#define MU_VST_VSTPLUGINLISTMODELEXAMPLE_H

#include <QAbstractListModel>

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "iinteractive.h"

#include "internal/vstplugin.h"
#include "ivstpluginrepository.h"

namespace mu::vst {
class VstPluginListModelExample : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(int selectedItemIndex READ selectedItemIndex WRITE setSelectedItemIndex NOTIFY selectedItemIndexChanged)

    INJECT(vst, framework::IInteractive, interactive)
    INJECT(vst, IVstPluginRepository, repository)
public:
    explicit VstPluginListModelExample(QObject* parent = nullptr);
    ~VstPluginListModelExample();

    Q_INVOKABLE void load();
    Q_INVOKABLE void showPluginEditor();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int selectedItemIndex() const;

public slots:
    void setSelectedItemIndex(int selectedItemIndex);

signals:
    void selectedItemIndexChanged(int selectedItemIndex);

private:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        IdRole,
        PathRole
    };

    void updatePluginMetaList(const VstPluginMetaList& newMetaList);

    VstPluginMetaList m_pluginMetaList;
    int m_selectedItemIndex = -1;
};
}

#endif // VSTPLUGINLISTMODELEXAMPLE_H
