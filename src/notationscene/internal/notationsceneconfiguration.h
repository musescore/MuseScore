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

#include "async/asyncable.h"
#include "context/iglobalcontext.h"
#include "modularity/ioc.h"

#include "inotationsceneconfiguration.h"

namespace mu::notation {
class NotationSceneConfiguration : public INotationSceneConfiguration, public muse::async::Asyncable, public muse::Contextable
{
    muse::ContextInject<context::IGlobalContext> context = { this };

public:
    explicit NotationSceneConfiguration(const muse::modularity::ContextPtr& ctx);

    void init();

    bool isSmoothPanning() const override;
    void setIsSmoothPanning(bool value) override;
    muse::async::Notification isSmoothPanningChanged() const override;

    bool isLimitCanvasScrollArea() const override;
    void setIsLimitCanvasScrollArea(bool limited) override;
    muse::async::Notification isLimitCanvasScrollAreaChanged() const override;

    muse::ValCh<int> pianoKeyboardNumberOfKeys() const override;
    void setPianoKeyboardNumberOfKeys(int number) override;

    bool useNewPercussionPanel() const override;
    void setUseNewPercussionPanel(bool use) override;
    muse::async::Notification useNewPercussionPanelChanged() const override;

    bool percussionPanelUseNotationPreview() const override;
    void setPercussionPanelUseNotationPreview(bool use) override;
    muse::async::Notification percussionPanelUseNotationPreviewChanged() const override;

    PercussionPanelAutoShowMode percussionPanelAutoShowMode() const override;
    void setPercussionPanelAutoShowMode(PercussionPanelAutoShowMode percussionPanelAutoShowMode) override;
    muse::async::Notification percussionPanelAutoShowModeChanged() const override;

    bool autoClosePercussionPanel() const override;
    void setAutoClosePercussionPanel(bool autoClose) override;
    muse::async::Notification autoClosePercussionPanelChanged() const override;

    bool showPercussionPanelPadSwapDialog() const override;
    void setShowPercussionPanelPadSwapDialog(bool show) override;
    muse::async::Notification showPercussionPanelPadSwapDialogChanged() const override;

    bool percussionPanelMoveMidiNotesAndShortcuts() const override;
    void setPercussionPanelMoveMidiNotesAndShortcuts(bool move) override;
    muse::async::Notification percussionPanelMoveMidiNotesAndShortcutsChanged() const override;

    int styleDialogLastPageIndex() const override;
    void setStyleDialogLastPageIndex(int value) override;

    int styleDialogLastSubPageIndex() const override;
    void setStyleDialogLastSubPageIndex(int value) override;

    void resetStyleDialogPageIndices() override;

private:
    muse::async::Notification m_isSmoothPanningChanged;
    muse::async::Notification m_isLimitCanvasScrollAreaChanged;
    muse::ValCh<int> m_pianoKeyboardNumberOfKeys;
    muse::async::Notification m_useNewPercussionPanelChanged;
    muse::async::Notification m_percussionPanelUseNotationPreviewChanged;
    muse::async::Notification m_percussionPanelAutoShowModeChanged;
    muse::async::Notification m_autoClosePercussionPanelChanged;
    muse::async::Notification m_showPercussionPanelPadSwapDialogChanged;
    muse::async::Notification m_percussionPanelMoveMidiNotesAndShortcutsChanged;

    int m_styleDialogLastPageIndex = 0;
    int m_styleDialogLastSubPageIndex = 0;
};
}
