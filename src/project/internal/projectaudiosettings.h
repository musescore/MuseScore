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

#include "../iprojectaudiosettings.h"

#include "ret.h"
#include "engraving/infrastructure/io/mscreader.h"
#include "engraving/infrastructure/io/mscwriter.h"

namespace mu::project {
class ProjectAudioSettings : public IProjectAudioSettings
{
public:

    audio::AudioOutputParams masterAudioOutputParams() const override;
    void setMasterAudioOutputParams(const audio::AudioOutputParams& params) override;

    audio::AudioInputParams trackInputParams(const ID& partId) const override;
    void setTrackInputParams(const ID& partId, const audio::AudioInputParams& params) override;

    audio::AudioOutputParams trackOutputParams(const ID& partId) const override;
    void setTrackOutputParams(const ID& partId, const audio::AudioOutputParams& params) override;

    void removeTrackParams(const ID& partId) override;

    Ret read(const engraving::MscReader& reader);
    Ret write(engraving::MscWriter& writer);

    //! NOTE Used for new or imported project (score)
    void makeDefault();

private:
    friend class NotationProject;
    ProjectAudioSettings() = default;

    audio::AudioInputParams inputParamsFromJson(const QJsonObject& object) const;
    audio::AudioOutputParams outputParamsFromJson(const QJsonObject& object) const;
    audio::AudioFxChain fxChainFromJson(const QJsonObject& fxChainObject) const;
    audio::AudioFxParams fxParamsFromJson(const QJsonObject& object) const;
    audio::AudioResourceMeta resourceMetaFromJson(const QJsonObject& object) const;

    QJsonObject inputParamsToJson(const audio::AudioInputParams& params) const;
    QJsonObject outputParamsToJson(const audio::AudioOutputParams& params) const;
    QJsonObject fxChainToJson(const audio::AudioFxChain& fxChain) const;
    QJsonObject fxParamsToJson(const audio::AudioFxParams& fxParams) const;
    QJsonObject resourceMetaToJson(const audio::AudioResourceMeta& meta) const;

    audio::AudioSourceType sourceTypeFromString(const QString& string) const;
    audio::AudioResourceType resourceTypeFromString(const QString& string) const;

    QString sourceTypeToString(const audio::AudioSourceType& type) const;
    QString resourceTypeToString(const audio::AudioResourceType& type) const;


    QJsonObject buildTrackObject(const ID& id) const;

    audio::AudioOutputParams m_masterOutputParams;

    std::map<ID /*partId*/, audio::AudioInputParams> m_trackInputParamsMap;
    std::map<ID /*partId*/, audio::AudioOutputParams> m_trackOutputParamsMap;
};

using ProjectAudioSettingsPtr = std::shared_ptr<ProjectAudioSettings>;
}

#endif // MU_PROJECT_PROJECTAUDIOSETTINGS_H
