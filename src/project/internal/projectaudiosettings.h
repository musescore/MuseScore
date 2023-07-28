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
    audio::AudioOutputParams masterAudioOutputParams() const override;
    void setMasterAudioOutputParams(const audio::AudioOutputParams& params) override;

    bool containsAuxOutputParams(audio::aux_channel_idx_t index) const override;
    audio::AudioOutputParams auxOutputParams(audio::aux_channel_idx_t index) const override;
    void setAuxOutputParams(audio::aux_channel_idx_t index, const audio::AudioOutputParams& params) override;

    audio::AudioInputParams trackInputParams(const engraving::InstrumentTrackId& partId) const override;
    void setTrackInputParams(const engraving::InstrumentTrackId& partId, const audio::AudioInputParams& params) override;

    audio::AudioOutputParams trackOutputParams(const engraving::InstrumentTrackId& partId) const override;
    void setTrackOutputParams(const engraving::InstrumentTrackId& partId, const audio::AudioOutputParams& params) override;

    SoloMuteState trackSoloMuteState(const engraving::InstrumentTrackId& trackId) const override;
    void setTrackSoloMuteState(const engraving::InstrumentTrackId& trackId, const SoloMuteState& state) override;
    async::Channel<engraving::InstrumentTrackId, SoloMuteState> trackSoloMuteStateChanged() const override;

    SoloMuteState auxSoloMuteState(audio::aux_channel_idx_t index) const override;
    void setAuxSoloMuteState(audio::aux_channel_idx_t index, const SoloMuteState& state) override;
    async::Channel<audio::aux_channel_idx_t, SoloMuteState> auxSoloMuteStateChanged() const override;

    void removeTrackParams(const engraving::InstrumentTrackId& partId) override;

    mu::ValNt<bool> needSave() const override;
    void markAsSaved() override;

    const playback::SoundProfileName& activeSoundProfile() const override;
    void setActiveSoundProfile(const playback::SoundProfileName& profileName) override;

    Ret read(const engraving::MscReader& reader);
    Ret write(engraving::MscWriter& writer);

    //! NOTE Used for new or imported project (score)
    void makeDefault();

private:
    friend class NotationProject;
    ProjectAudioSettings() = default;

    audio::AudioInputParams inputParamsFromJson(const QJsonObject& object) const;
    audio::AudioOutputParams outputParamsFromJson(const QJsonObject& object) const;
    SoloMuteState soloMuteStateFromJson(const QJsonObject& object) const;
    audio::AudioFxChain fxChainFromJson(const QJsonObject& fxChainObject) const;
    audio::AudioFxParams fxParamsFromJson(const QJsonObject& object) const;
    audio::AuxSendsParams auxSendsFromJson(const QJsonArray& objectList) const;
    audio::AuxSendParams auxSendParamsFromJson(const QJsonObject& object) const;
    audio::AudioResourceMeta resourceMetaFromJson(const QJsonObject& object) const;
    audio::AudioUnitConfig unitConfigFromJson(const QJsonObject& object) const;
    audio::AudioResourceAttributes attributesFromJson(const QJsonObject& object) const;

    QJsonObject inputParamsToJson(const audio::AudioInputParams& params) const;
    QJsonObject outputParamsToJson(const audio::AudioOutputParams& params) const;
    QJsonObject soloMuteStateToJson(const SoloMuteState& state) const;
    QJsonObject fxChainToJson(const audio::AudioFxChain& fxChain) const;
    QJsonObject fxParamsToJson(const audio::AudioFxParams& fxParams) const;
    QJsonArray auxSendsToJson(const audio::AuxSendsParams& auxSends) const;
    QJsonObject auxSendParamsToJson(const audio::AuxSendParams& auxParams) const;
    QJsonObject resourceMetaToJson(const audio::AudioResourceMeta& meta) const;
    QJsonObject unitConfigToJson(const audio::AudioUnitConfig& config) const;
    QJsonObject attributesToJson(const audio::AudioResourceAttributes& attributes) const;

    audio::AudioSourceType sourceTypeFromString(const QString& string) const;
    audio::AudioResourceType resourceTypeFromString(const QString& string) const;

    QString sourceTypeToString(const audio::AudioSourceType& type) const;
    QString resourceTypeToString(const audio::AudioResourceType& type) const;

    QJsonObject buildAuxObject(audio::aux_channel_idx_t index, const audio::AudioOutputParams& params) const;
    QJsonObject buildTrackObject(const engraving::InstrumentTrackId& id) const;

    void setNeedSave(bool needSave);

    audio::AudioOutputParams m_masterOutputParams;

    std::map<audio::aux_channel_idx_t, audio::AudioOutputParams> m_auxOutputParams;
    std::unordered_map<audio::aux_channel_idx_t, SoloMuteState> m_auxSoloMuteStatesMap;
    async::Channel<audio::aux_channel_idx_t, SoloMuteState> m_auxSoloMuteStateChanged;

    std::unordered_map<engraving::InstrumentTrackId, audio::AudioInputParams> m_trackInputParamsMap;
    std::unordered_map<engraving::InstrumentTrackId, audio::AudioOutputParams> m_trackOutputParamsMap;
    std::unordered_map<engraving::InstrumentTrackId, SoloMuteState> m_trackSoloMuteStatesMap;
    async::Channel<engraving::InstrumentTrackId, SoloMuteState> m_trackSoloMuteStateChanged;

    bool m_needSave = false;
    async::Notification m_needSaveNotification;

    mu::playback::SoundProfileName m_activeSoundProfileName;
};

using ProjectAudioSettingsPtr = std::shared_ptr<ProjectAudioSettings>;
}

#endif // MU_PROJECT_PROJECTAUDIOSETTINGS_H
