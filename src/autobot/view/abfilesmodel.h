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
#ifndef MU_AUTOBOT_ABFILESMODEL_H
#define MU_AUTOBOT_ABFILESMODEL_H

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "iautobot.h"
#include "async/asyncable.h"

namespace mu::autobot {
class AbFilesModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT
    INJECT(autobot, IAutobot, autobot)

public:
    explicit AbFilesModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int,QByteArray> roleNames() const override;

private:

    void update();
    void updateFile(const File& f);
    QString fileStatus(const File& f) const;

    enum Roles {
        FileTitle = Qt::UserRole + 1,
        FileIndex,
        IsCurrentFile,
        FileStatus
    };

    ValNt<Files> m_files;
    ValCh<int> m_fileIndex;
};
}

#endif // MU_AUTOBOT_ABFILESMODEL_H
