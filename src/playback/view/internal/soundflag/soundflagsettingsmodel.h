/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#ifndef MU_PLAYBACK_SOUNDFLAGSETTINGSMODEL_H
#define MU_PLAYBACK_SOUNDFLAGSETTINGSMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "playback/iplaybackcontroller.h"
#include "playback/iplaybackconfiguration.h"

#include "uicomponents/view/menuitem.h"

#include "notation/view/abstractelementpopupmodel.h"

namespace mu::playback {
class SoundFlagSettingsModel : public notation::AbstractElementPopupModel
{
    Q_OBJECT

    Q_PROPERTY(bool inited READ inited NOTIFY initedChanged FINAL)

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged FINAL)

    Q_PROPERTY(QRectF iconRect READ iconRect NOTIFY iconRectChanged FINAL)

    Q_PROPERTY(QVariantList availablePresets READ availablePresets NOTIFY availablePresetsChanged FINAL)
    Q_PROPERTY(QStringList selectedPresetCodes READ selectedPresetCodes NOTIFY selectedPresetCodesChanged FINAL)

    Q_PROPERTY(QVariantList availablePlayingTechniques READ availablePlayingTechniques NOTIFY availablePlayingTechniquesChanged FINAL)
    Q_PROPERTY(QString selectedPlayingTechniqueCode READ selectedPlayingTechniqueCode NOTIFY selectedPlayingTechniqueCodeChanged FINAL)

    Q_PROPERTY(QVariantList contextMenuModel READ contextMenuModel NOTIFY contextMenuModelChanged FINAL)

    muse::Inject<IPlaybackController> playbackController = { this };
    muse::Inject<IPlaybackConfiguration> playbackConfiguration = { this };

public:
    explicit SoundFlagSettingsModel(QObject* parent = nullptr);

    bool inited() const;

    Q_INVOKABLE void init() override;
    Q_INVOKABLE void togglePreset(const QString& presetCode);
    Q_INVOKABLE void togglePlayingTechnique(const QString& playingTechniqueCode);

    QVariantList contextMenuModel();
    Q_INVOKABLE void handleContextMenuItem(const QString& menuId);

    QString title() const;
    void setTitle(const QString& title);

    QRectF iconRect() const;

    QVariantList availablePresets() const;
    QStringList selectedPresetCodes() const;

    QVariantList availablePlayingTechniques() const;
    QString selectedPlayingTechniqueCode() const;

signals:
    void initedChanged();

    void iconRectChanged(const QRectF& rect);

    void titleChanged();

    void availablePresetsChanged();
    void selectedPresetCodesChanged();

    void availablePlayingTechniquesChanged();
    void selectedPlayingTechniqueCodeChanged();

    void contextMenuModelChanged();

private:
    void load();

    project::IProjectAudioSettingsPtr audioSettings() const;
    const muse::audio::AudioInputParams& currentAudioInputParams() const;

    void initTitle();
    void initAvailablePresets();
    void initAvailablePlayingTechniques();

    void setAvailableSoundPresets(const muse::audio::SoundPresetList& presets);
    void loadAvailablePlayingTechniques();

    muse::uicomponents::MenuItem* buildMenuItem(const QString& actionCode, const muse::TranslatableString& title, bool enabled = true);

    QString defaultPresetCode() const;
    QString defaultPlayingTechniqueCode() const;

    bool updateStaffText();

    QString m_title;

    muse::audio::SoundPresetList m_availablePresets;

    QVariantList m_availablePresetsModel;
    QVariantList m_availablePlayingTechniquesModel;

    bool m_availablePresetsInited = false;
};
}

#endif // MU_PLAYBACK_SOUNDFLAGSETTINGSMODEL_H
