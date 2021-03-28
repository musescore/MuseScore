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

#ifndef MU_USERSCORES_EXPORTSCORESETTINGSMODEL_H
#define MU_USERSCORES_EXPORTSCORESETTINGSMODEL_H

#include "modularity/ioc.h"

#include "settings.h"
#include "async/asyncable.h"

#include <QAbstractListModel>

namespace mu::userscores {
class ExportScoreSettingsModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT

public:
    explicit ExportScoreSettingsModel(QObject* parent = nullptr);

    Q_INVOKABLE void load(QString suffix);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void setValue(int index, QVariant newVal);

private:
    enum Roles {
        LabelRole = Qt::UserRole + 1,
        TypeRole,
        ValRole,
        InfoRole
    };

    QString typeToString(const framework::Settings::Item& item) const;

    using KeyList = QList<framework::Settings::Key>;

    KeyList pdfKeys() const;
    KeyList pngKeys() const;
    KeyList mp3Keys() const;
    KeyList audioKeys() const;
    KeyList midiKeys() const;
    KeyList xmlKeys() const;

    KeyList m_keys;
};
}

#endif // MU_USERSCORES_EXPORTSCORESETTINGSMODEL_H
