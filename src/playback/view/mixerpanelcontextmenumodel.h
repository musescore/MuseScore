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

#ifndef MU_PLAYBACK_MIXERPANELCONTEXTMENUMODEL_H
#define MU_PLAYBACK_MIXERPANELCONTEXTMENUMODEL_H

#include "uicomponents/view/abstractmenumodel.h"
#include "actions/actionable.h"

#include "actions/iactionsdispatcher.h"
#include "playback/iplaybackconfiguration.h"

#include "playback/playbacktypes.h"

namespace mu::playback {
class MixerPanelContextMenuModel : public uicomponents::AbstractMenuModel, public actions::Actionable
{
    Q_OBJECT

    INJECT(playback, actions::IActionsDispatcher, dispatcher)
    INJECT(playback, playback::IPlaybackConfiguration, configuration)

    Q_PROPERTY(bool labelsSectionVisible READ labelsSectionVisible NOTIFY labelsSectionVisibleChanged)
    Q_PROPERTY(bool soundSectionVisible READ soundSectionVisible NOTIFY soundSectionVisibleChanged)
    Q_PROPERTY(bool audioFxSectionVisible READ audioFxSectionVisible NOTIFY audioFxSectionVisibleChanged)
    Q_PROPERTY(bool balanceSectionVisible READ balanceSectionVisible NOTIFY balanceSectionVisibleChanged)
    Q_PROPERTY(bool volumeSectionVisible READ volumeSectionVisible NOTIFY volumeSectionVisibleChanged)
    Q_PROPERTY(bool faderSectionVisible READ faderSectionVisible NOTIFY faderSectionVisibleChanged)
    Q_PROPERTY(bool muteAndSoloSectionVisible READ muteAndSoloSectionVisible NOTIFY muteAndSoloSectionVisibleChanged)
    Q_PROPERTY(bool titleSectionVisible READ titleSectionVisible NOTIFY titleSectionVisibleChanged)

public:
    explicit MixerPanelContextMenuModel(QObject* parent = nullptr);

    bool labelsSectionVisible() const;
    bool soundSectionVisible() const;
    bool audioFxSectionVisible() const;
    bool balanceSectionVisible() const;
    bool volumeSectionVisible() const;
    bool faderSectionVisible() const;
    bool muteAndSoloSectionVisible() const;
    bool titleSectionVisible() const;

    Q_INVOKABLE void load() override;

signals:
    void labelsSectionVisibleChanged();
    void soundSectionVisibleChanged();
    void audioFxSectionVisibleChanged();
    void balanceSectionVisibleChanged();
    void volumeSectionVisibleChanged();
    void faderSectionVisibleChanged();
    void muteAndSoloSectionVisibleChanged();
    void titleSectionVisibleChanged();

private:
    bool isSectionVisible(MixerSectionType sectionType) const;

    uicomponents::MenuItem* buildViewMenuItem(MixerSectionType sectionType);

    void toggleMixerSection(const actions::ActionData& args);
};
}

#endif // MU_PLAYBACK_MIXERPANELCONTEXTMENUMODEL_H
