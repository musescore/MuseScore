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
#ifndef MU_NOTATION_EDITGRIDSIZEDIALOGMODEL_H
#define MU_NOTATION_EDITGRIDSIZEDIALOGMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "notation/inotationconfiguration.h"

namespace mu::notation {
class EditGridSizeDialogModel : public QObject, public muse::Injectable
{
    Q_OBJECT

    Q_PROPERTY(
        int verticalGridSizeSpatium READ verticalGridSizeSpatium WRITE setVerticalGridSizeSpatium NOTIFY verticalGridSizeSpatiumChanged)
    Q_PROPERTY(
        int horizontalGridSizeSpatium READ horizontalGridSizeSpatium WRITE setHorizontalGridSizeSpatium NOTIFY horizontalGridSizeSpatiumChanged)

    muse::Inject<INotationConfiguration> configuration = { this };

public:
    explicit EditGridSizeDialogModel(QObject* parent = nullptr);

    int verticalGridSizeSpatium() const;
    int horizontalGridSizeSpatium() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void apply();

public slots:
    void setVerticalGridSizeSpatium(int size);
    void setHorizontalGridSizeSpatium(int size);

signals:
    void verticalGridSizeSpatiumChanged(int size);
    void horizontalGridSizeSpatiumChanged(int size);

private:
    int m_verticalGridSizeSpatium = 0;
    int m_horizontalGridSizeSpatium = 0;
};
}

#endif // MU_NOTATION_EDITGRIDSIZEDIALOGMODEL_H
