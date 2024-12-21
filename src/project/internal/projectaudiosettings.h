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
#ifndef MU_PROJECT_PROJECTAUDIOSETTINGS_H
#define MU_PROJECT_PROJECTAUDIOSETTINGS_H

#include <memory>
#include <string>

#include "modularity/ioc.h"
#include "playback/iplaybackconfiguration.h"
#include "types/ret.h"
#include "engraving/infrastructure/mscreader.h"
#include "engraving/infrastructure/mscwriter.h"

#include "../iprojectaudiosettings.h"

namespace mu::project {
class ProjectAudioSettings : public IProjectAudioSettings
{
    INJECT_STATIC(playback::IPlaybackConfiguration, playbackConfig)
public:
    const muse::audio::AudioOutputParams& masterAudioOutputParams() const override;
    void setMasterAudioOutputParams(const muse::audio::AudioOutputParams& params) override;

    bool containsAuxOutputParams(muse::audio::aux_channel_idx_t index) const override;
    const muse::audio::AudioOutputParams& auxOutputParams(muse::audio::aux_channel_idx_t index) const override;
    void setAuxOutputParams(muse::audio::aux_channel_idx_t index, const muse::audio::AudioOutputParams& params) override;

    const muse::audio::AudioInputParams& trackInputParams(const engraving::InstrumentTrackId& partId) const override;
    void setTrackInputParams(const engraving::InstrumentTrackId& partId, const muse::audio::AudioInputParams& params) override;
    void clearTrackInputParams() override;
    muse::async::Channel<engraving::InstrumentTrackId> trackInputParamsChanged() const override;

    bool trackHasExistingOutputParams(const engraving::InstrumentTrackId& partId) const override;
    const muse::audio::AudioOutputParams& trackOutputParams(const engraving::InstrumentTrackId& partId) const override;
    void setTrackOutputParams(const engraving::InstrumentTrackId& partId, const muse::audio::AudioOutputParams& params) override;

    const SoloMuteState& auxSoloMuteState(muse::audio::aux_channel_idx_t index) const override;
    void setAuxSoloMuteState(muse::audio::aux_channel_idx_t index, const SoloMuteState& state) override;
    muse::async::Channel<muse::audio::aux_channel_idx_t, SoloMuteState> auxSoloMuteStateChanged() const override;

    void removeTrackParams(const engraving::InstrumentTrackId& partId) override;

    const playback::SoundProfileName& activeSoundProfile() const override;
    void setActiveSoundProfile(const playback::SoundProfileName& profileName) override;

    muse::async::Notification settingsChanged() const override;

    muse::Ret read(const engraving::MscReader& reader);
    muse::Ret write(engraving::MscWriter& writer, notation::INotationSoloMuteStatePtr masterSoloMuteStatePtr);

    //! NOTE Used for new or imported project (score)
    void makeDefault();

private:
    friend class NotationProject;
    ProjectAudioSettings() = default;

    muse::audio::AudioInputParams inputParamsFromJson(const QJsonObject& object) const;
    muse::audio::AudioOutputParams outputParamsFromJson(const QJsonObject& object) const;
    SoloMuteState soloMuteStateFromJson(const QJsonObject& object) const;
    muse::audio::AudioFxChain fxChainFromJson(const QJsonObject& fxChainObject) const;
    muse::audio::AudioFxParams fxParamsFromJson(const QJsonObject& object) const;
    muse::audio::AuxSendsParams auxSendsFromJson(const QJsonArray& objectList) const;
    muse::audio::AuxSendParams auxSendParamsFromJson(const QJsonObject& object) const;
    muse::audio::AudioResourceMeta resourceMetaFromJson(const QJsonObject& object) const;
    muse::audio::AudioUnitConfig unitConfigFromJson(const QJsonObject& object) const;
    muse::audio::AudioResourceAttributes attributesFromJson(const QJsonObject& object) const;

    QJsonObject inputParamsToJson(const muse::audio::AudioInputParams& params) const;
    QJsonObject outputParamsToJson(const muse::audio::AudioOutputParams& params) const;
    QJsonObject soloMuteStateToJson(const SoloMuteState& state) const;
    QJsonObject fxChainToJson(const muse::audio::AudioFxChain& fxChain) const;
    QJsonObject fxParamsToJson(const muse::audio::AudioFxParams& fxParams) const;
    QJsonArray auxSendsToJson(const muse::audio::AuxSendsParams& auxSends) const;
    QJsonObject auxSendParamsToJson(const muse::audio::AuxSendParams& auxParams) const;
    QJsonObject resourceMetaToJson(const muse::audio::AudioResourceMeta& meta) const;
    QJsonObject unitConfigToJson(const muse::audio::AudioUnitConfig& config) const;
    QJsonObject attributesToJson(const muse::audio::AudioResourceAttributes& attributes) const;

    muse::audio::AudioSourceType sourceTypeFromString(const QString& string) const;
    muse::audio::AudioResourceType resourceTypeFromString(const QString& string) const;

    QString sourceTypeToString(const muse::audio::AudioSourceType& type) const;
    QString resourceTypeToString(const muse::audio::AudioResourceType& type) const;

    QJsonObject buildAuxObject(muse::audio::aux_channel_idx_t index, const muse::audio::AudioOutputParams& params) const;
    QJsonObject buildTrackObject(notation::INotationSoloMuteStatePtr masterSoloMuteStatePtr, const engraving::InstrumentTrackId& id) const;

    muse::audio::AudioOutputParams m_masterOutputParams;

    std::map<muse::audio::aux_channel_idx_t, muse::audio::AudioOutputParams> m_auxOutputParams;
    std::unordered_map<muse::audio::aux_channel_idx_t, SoloMuteState> m_auxSoloMuteStatesMap;
    muse::async::Channel<muse::audio::aux_channel_idx_t, SoloMuteState> m_auxSoloMuteStateChanged;

    std::unordered_map<engraving::InstrumentTrackId, muse::audio::AudioInputParams> m_trackInputParamsMap;
    std::unordered_map<engraving::InstrumentTrackId, muse::audio::AudioOutputParams> m_trackOutputParamsMap;

    muse::async::Notification m_settingsChanged;
    muse::async::Channel<engraving::InstrumentTrackId> m_trackInputParamsChanged;

    mu::playback::SoundProfileName m_activeSoundProfileName;
};

using ProjectAudioSettingsPtr = std::shared_ptr<ProjectAudioSettings>;
}

#endif // MU_PROJECT_PROJECTAUDIOSETTINGS_H
