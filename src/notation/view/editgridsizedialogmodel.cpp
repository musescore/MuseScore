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

#include "editgridsizedialogmodel.h"

using namespace mu::notation;
using namespace mu::framework;

EditGridSizeDialogModel::EditGridSizeDialogModel(QObject* parent)
    : QObject(parent)
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
    setVerticalGridSizeSpatium(configuration()->gridSizeSpatium(mu::framework::Orientation::Vertical));
    setHorizontalGridSizeSpatium(configuration()->gridSizeSpatium(mu::framework::Orientation::Horizontal));
}

void EditGridSizeDialogModel::apply()
{
    configuration()->setGridSize(mu::framework::Orientation::Vertical, m_verticalGridSizeSpatium);
    configuration()->setGridSize(mu::framework::Orientation::Horizontal, m_horizontalGridSizeSpatium);
}
