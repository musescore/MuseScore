/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#pragma once

#include <qqmlintegration.h>

#include "abstractnotationpaintview.h"
#include "modifierholdwatcher.h"

#include "shortcuts_v2/icommandshortcutsregister.h"

namespace mu::notation {
class NotationPaintView : public AbstractNotationPaintView
{
    Q_OBJECT
    QML_ELEMENT

    muse::GlobalInject<muse::shortcuts::ICommandShortcutsRegister> commandShortcutsRegister;

public:
    explicit NotationPaintView(QQuickItem* parent = nullptr);

private:
    void onLoadNotation(INotationPtr notation) override;
    void onUnloadNotation(INotationPtr notation) override;

    void initZoomAndPosition() override;

    void onMatrixChanged(const muse::draw::Transform& oldMatrix, const muse::draw::Transform& newMatrix,
                         bool overrideZoomType = true) override;

    //! While the highlight keys are held: color everything that affects the selected note's playback (if a single
    //! note is selected); otherwise nothing. The view doesn't move.
    void updateHeldPlaybackInfluence();

    //! The keys of the "Highlight playback influences" shortcut (none if it has no modifiers-only sequence)
    Qt::KeyboardModifiers playbackInfluenceModifiers() const;

    bool m_isLocalMatrixUpdate = false;

    ModifierHoldWatcher m_influenceKeys;
    INotationPtr m_heldInfluenceNotation; // the notation the held highlight was set on
};
}
