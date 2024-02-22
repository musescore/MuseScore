/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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

namespace mu::engraving {
class StaffText;
}

namespace mu::playback {
class SoundFlagSettingsModel : public notation::AbstractElementPopupModel
{
    Q_OBJECT

    INJECT(IPlaybackController, playbackController)
    INJECT(IPlaybackConfiguration, playbackConfiguration)

    Q_PROPERTY(bool inited READ inited NOTIFY initedChanged FINAL)

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged FINAL)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged FINAL)

    Q_PROPERTY(QRect iconRect READ iconRect NOTIFY iconRectChanged FINAL)

    Q_PROPERTY(QVariantList availablePresets READ availablePresets NOTIFY availablePresetsChanged FINAL)
    Q_PROPERTY(QStringList presetCodes READ presetCodes NOTIFY presetCodesChanged FINAL)

    Q_PROPERTY(QVariantList availablePlayingTechniques READ availablePlayingTechniques NOTIFY availablePlayingTechniquesChanged FINAL)
    Q_PROPERTY(QStringList playingTechniquesCodes READ playingTechniquesCodes NOTIFY playingTechniquesCodesChanged FINAL)

    Q_PROPERTY(QVariantList contextMenuModel READ contextMenuModel NOTIFY contextMenuModelChanged FINAL)

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

    QString text() const;
    void setText(const QString& text);

    QRect iconRect() const;

    QVariantList availablePresets() const;
    void setAvailablePresets(const audio::SoundPresetList& presets);

    QStringList presetCodes() const;

    QVariantList availablePlayingTechniques() const;
    void setAvailablePlayingTechniques(const audio::SoundPreset::PlayingTechniqueList& playingTechniques);

    QStringList playingTechniquesCodes() const;

signals:
    void titleChanged();
    void showTextChanged();
    void textChanged();

    void iconRectChanged();

    void availablePresetsChanged();
    void presetCodesChanged();

    void availablePlayingTechniquesChanged();
    void playingTechniquesCodesChanged();

    void contextMenuModelChanged();

    void initedChanged();

private:
    project::IProjectAudioSettingsPtr audioSettings() const;

    const audio::AudioInputParams& currentAudioInputParams() const;

    void initTitle();
    void initAvailablePresets();
    void initAvailablePlayingTechniques();

    engraving::StaffText* staffText() const;

    uicomponents::MenuItem* buildMenuItem(const QString& actionCode, const TranslatableString& title);

    bool updateStaffText();

    QString m_title;

    QVariantList m_availablePresets;
    QVariantList m_availablePlayingTechniques;

    bool m_isPresetsInited = false;
    bool m_isPlayingTechniquesInited = false;
};
}

#endif // MU_PLAYBACK_SOUNDFLAGSETTINGSMODEL_H
