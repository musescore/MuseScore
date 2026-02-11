/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "modularity/imoduleinterface.h"
#include "async/notification.h"
#include "types/retval.h"

#include "notationscenetypes.h"

namespace mu::notation {
class INotationSceneConfiguration : MODULE_GLOBAL_INTERFACE
{
    INTERFACE_ID(INotationSceneConfiguration)

public:
    virtual ~INotationSceneConfiguration() = default;

    virtual bool isSmoothPanning() const = 0;
    virtual void setIsSmoothPanning(bool value) = 0;
    virtual muse::async::Notification isSmoothPanningChanged() const = 0;

    virtual bool isLimitCanvasScrollArea() const = 0;
    virtual void setIsLimitCanvasScrollArea(bool limited) = 0;
    virtual muse::async::Notification isLimitCanvasScrollAreaChanged() const = 0;

    virtual muse::ValCh<int> pianoKeyboardNumberOfKeys() const = 0;
    virtual void setPianoKeyboardNumberOfKeys(int number) = 0;

    virtual bool useNewPercussionPanel() const = 0;
    virtual void setUseNewPercussionPanel(bool use) = 0;
    virtual muse::async::Notification useNewPercussionPanelChanged() const = 0;

    virtual bool percussionPanelUseNotationPreview() const = 0;
    virtual void setPercussionPanelUseNotationPreview(bool use) = 0;
    virtual muse::async::Notification percussionPanelUseNotationPreviewChanged() const = 0;

    virtual PercussionPanelAutoShowMode percussionPanelAutoShowMode() const = 0;
    virtual void setPercussionPanelAutoShowMode(PercussionPanelAutoShowMode autoShowMode) = 0;
    virtual muse::async::Notification percussionPanelAutoShowModeChanged() const = 0;

    virtual bool autoClosePercussionPanel() const = 0;
    virtual void setAutoClosePercussionPanel(bool autoClose) = 0;
    virtual muse::async::Notification autoClosePercussionPanelChanged() const = 0;

    virtual bool showPercussionPanelPadSwapDialog() const = 0;
    virtual void setShowPercussionPanelPadSwapDialog(bool show) = 0;
    virtual muse::async::Notification showPercussionPanelPadSwapDialogChanged() const = 0;

    virtual bool percussionPanelMoveMidiNotesAndShortcuts() const = 0;
    virtual void setPercussionPanelMoveMidiNotesAndShortcuts(bool move) = 0;
    virtual muse::async::Notification percussionPanelMoveMidiNotesAndShortcutsChanged() const = 0;

    virtual int styleDialogLastPageIndex() const = 0;
    virtual void setStyleDialogLastPageIndex(int value) = 0;

    virtual int styleDialogLastSubPageIndex() const = 0;
    virtual void setStyleDialogLastSubPageIndex(int value) = 0;

    virtual void resetStyleDialogPageIndices() = 0;
};
}
