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

#include "editgridsizedialogmodel.h"

using namespace mu::notation;

EditGridSizeDialogModel::EditGridSizeDialogModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

int EditGridSizeDialogModel::verticalGridSizeSpatium() const
{
    return m_verticalGridSizeSpatium;
}

int EditGridSizeDialogModel::horizontalGridSizeSpatium() const
{
    return m_horizontalGridSizeSpatium;
}

void EditGridSizeDialogModel::setVerticalGridSizeSpatium(int size)
{
    if (m_verticalGridSizeSpatium == size) {
        return;
    }

    m_verticalGridSizeSpatium = size;
    emit verticalGridSizeSpatiumChanged(size);
}

void EditGridSizeDialogModel::setHorizontalGridSizeSpatium(int size)
{
    if (m_horizontalGridSizeSpatium == size) {
        return;
    }

    m_horizontalGridSizeSpatium = size;
    emit horizontalGridSizeSpatiumChanged(size);
}

void EditGridSizeDialogModel::load()
{
    setVerticalGridSizeSpatium(configuration()->gridSizeSpatium(muse::Orientation::Vertical));
    setHorizontalGridSizeSpatium(configuration()->gridSizeSpatium(muse::Orientation::Horizontal));
}

void EditGridSizeDialogModel::apply()
{
    configuration()->setGridSize(muse::Orientation::Vertical, m_verticalGridSizeSpatium);
    configuration()->setGridSize(muse::Orientation::Horizontal, m_horizontalGridSizeSpatium);
}
