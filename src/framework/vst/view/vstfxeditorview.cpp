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

#include "vstfxeditorview.h"

using namespace muse::vst;

VstFxEditorView::VstFxEditorView(QWidget* parent)
    : AbstractVstEditorView(parent)
{
}

bool VstFxEditorView::isAbleToWrapPlugin() const
{
    return !resourceId().isEmpty() && m_chainOrder != -1;
}

IVstPluginInstancePtr VstFxEditorView::determineInstance() const
{
    if (trackId() == -1) {
        return instancesRegister()->masterFxPlugin(resourceId().toStdString(), m_chainOrder);
    }

    return instancesRegister()->fxPlugin(resourceId().toStdString(), trackId(), m_chainOrder);
}

int VstFxEditorView::chainOrder() const
{
    return m_chainOrder;
}

void VstFxEditorView::setChainOrder(int newChainOrder)
{
    if (m_chainOrder == newChainOrder) {
        return;
    }

    m_chainOrder = newChainOrder;
    emit chainOrderChanged();

    if (isAbleToWrapPlugin()) {
        wrapPluginView();
    }
}
