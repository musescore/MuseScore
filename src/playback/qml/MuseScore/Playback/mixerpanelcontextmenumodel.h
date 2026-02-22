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

#pragma once

#include "actions/actionable.h"
#include "uicomponents/qml/Muse/UiComponents/abstractmenumodel.h"

#include "actions/iactionsdispatcher.h"
#include "playback/iplaybackconfiguration.h"

#include "playback/playbacktypes.h"

namespace mu::playback {
class MixerPanelContextMenuModel : public muse::uicomponents::AbstractMenuModel, public muse::actions::Actionable
{
    Q_OBJECT

    Q_PROPERTY(bool labelsSectionVisible READ labelsSectionVisible NOTIFY labelsSectionVisibleChanged)
    Q_PROPERTY(bool soundSectionVisible READ soundSectionVisible NOTIFY soundSectionVisibleChanged)
    Q_PROPERTY(bool audioFxSectionVisible READ audioFxSectionVisible NOTIFY audioFxSectionVisibleChanged)
    Q_PROPERTY(bool auxSendsSectionVisible READ auxSendsSectionVisible NOTIFY auxSendsSectionVisibleChanged)
    Q_PROPERTY(bool balanceSectionVisible READ balanceSectionVisible NOTIFY balanceSectionVisibleChanged)
    Q_PROPERTY(bool volumeSectionVisible READ volumeSectionVisible NOTIFY volumeSectionVisibleChanged)
    Q_PROPERTY(bool faderSectionVisible READ faderSectionVisible NOTIFY faderSectionVisibleChanged)
    Q_PROPERTY(bool muteAndSoloSectionVisible READ muteAndSoloSectionVisible NOTIFY muteAndSoloSectionVisibleChanged)
    Q_PROPERTY(bool titleSectionVisible READ titleSectionVisible NOTIFY titleSectionVisibleChanged)
    Q_PROPERTY(bool highlightSelection READ highlightSelection WRITE setHighlightSelection NOTIFY highlightSelectionChanged)
    Q_PROPERTY(bool autoScrollToSelection READ autoScrollToSelection WRITE setAutoScrollToSelection NOTIFY autoScrollToSelectionChanged)

    QML_ELEMENT

    muse::GlobalInject<playback::IPlaybackConfiguration> configuration;
    muse::ContextInject<muse::actions::IActionsDispatcher> dispatcher = { this };

public:
    explicit MixerPanelContextMenuModel(QObject* parent = nullptr);

    bool labelsSectionVisible() const;
    bool soundSectionVisible() const;
    bool audioFxSectionVisible() const;
    bool auxSendsSectionVisible() const;
    bool balanceSectionVisible() const;
    bool volumeSectionVisible() const;
    bool faderSectionVisible() const;
    bool muteAndSoloSectionVisible() const;
    bool titleSectionVisible() const;

    bool highlightSelection() const { return m_highlightSelection; }
    void setHighlightSelection(bool v);
    bool autoScrollToSelection() const { return m_autoScrollToSelection; }
    void setAutoScrollToSelection(bool v);

    Q_INVOKABLE void load() override;

signals:
    void labelsSectionVisibleChanged();
    void soundSectionVisibleChanged();
    void audioFxSectionVisibleChanged();
    void auxSendsSectionVisibleChanged();
    void balanceSectionVisibleChanged();
    void volumeSectionVisibleChanged();
    void faderSectionVisibleChanged();
    void muteAndSoloSectionVisibleChanged();
    void titleSectionVisibleChanged();
    void highlightSelectionChanged();
    void autoScrollToSelectionChanged();

private:
    bool isSectionVisible(MixerSectionType sectionType) const;
    bool m_highlightSelection = false;
    bool m_autoScrollToSelection = false;

    muse::uicomponents::MenuItem* buildSectionVisibleItem(MixerSectionType sectionType);
    muse::uicomponents::MenuItem* buildAuxSendVisibleItem(muse::audio::aux_channel_idx_t index);
    muse::uicomponents::MenuItem* buildAuxChannelVisibleItem(muse::audio::aux_channel_idx_t index);
    muse::uicomponents::MenuItem* m_highlightMenuItem = nullptr;
    muse::uicomponents::MenuItem* m_autoScrollMenuItem = nullptr;

    void toggleMixerSection(const muse::actions::ActionData& args);
    void toggleAuxSend(const muse::actions::ActionData& args);
    void toggleAuxChannel(const muse::actions::ActionData& args);
    void toggleHighlight(const muse::actions::ActionData& args);
    void toggleAutoScroll(const muse::actions::ActionData& args);

    void emitMixerSectionVisibilityChanged(MixerSectionType sectionType);

    void setViewMenuItemChecked(const QString& itemId, bool checked);
};
}
