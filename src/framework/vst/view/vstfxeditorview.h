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

#ifndef MUSE_VST_VSTFXEDITORVIEW_H
#define MUSE_VST_VSTFXEDITORVIEW_H

#include "abstractvsteditorview.h"

namespace muse::vst {
class VstFxEditorView : public AbstractVstEditorView
{
    Q_OBJECT

    Q_PROPERTY(int chainOrder READ chainOrder WRITE setChainOrder NOTIFY chainOrderChanged)

public:
    explicit VstFxEditorView(QWidget* parent = nullptr);

    int chainOrder() const;
    void setChainOrder(int newChainOrder);

signals:
    void chainOrderChanged();

private:
    bool isAbleToWrapPlugin() const override;
    VstPluginPtr getPluginPtr() const override;

    muse::audio::AudioFxChainOrder m_chainOrder = -1;
};
}

Q_DECLARE_METATYPE(muse::vst::VstFxEditorView)

#endif // MUSE_VST_VSTFXEDITORVIEW_H
