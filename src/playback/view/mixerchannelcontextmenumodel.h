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

#ifndef MU_PLAYBACK_MIXERCHANNELCONTEXTMENUMODEL_H
#define MU_PLAYBACK_MIXERCHANNELCONTEXTMENUMODEL_H

#include "uicomponents/qml/Muse/UiComponents/abstractmenumodel.h"

namespace mu::playback {
class MixerPanelModel;

class MixerChannelContextMenuModel : public muse::uicomponents::AbstractMenuModel
{
    Q_OBJECT

    Q_PROPERTY(bool canPaste READ canPaste NOTIFY canPasteChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY undoStateChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY undoStateChanged)

public:
    explicit MixerChannelContextMenuModel(QObject* parent = nullptr);

    Q_INVOKABLE void loadForChannel(int channelIndex, QObject* mixerModel);

    bool canPaste() const;
    bool canUndo() const;
    bool canRedo() const;

signals:
    void canPasteChanged();
    void undoStateChanged();

private:
    void buildMenu();

    int m_channelIndex = -1;
    MixerPanelModel* m_mixerModel = nullptr;
};
}

#endif // MU_PLAYBACK_MIXERCHANNELCONTEXTMENUMODEL_H

