/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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

#include "soundprofilesmodel.h"

#include "project/inotationproject.h"
#include "project/iprojectaudiosettings.h"

using namespace mu::playback;
using namespace mu::project;

SoundProfilesModel::SoundProfilesModel(QObject* parent)
    : QAbstractListModel(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void SoundProfilesModel::init()
{
    beginResetModel();

    const SoundProfilesMap& availableProfiles = profilesRepo()->availableProfiles();
    for (const auto& pair : availableProfiles) {
        m_profiles.push_back(pair.second);
    }

    std::sort(m_profiles.begin(), m_profiles.end(), [this](const SoundProfile& left, const SoundProfile& right) {
        if (left.name == config()->basicSoundProfileName()
            && right.name == config()->museSoundsProfileName()) {
            return true;
        }

        return false;
    });

    if (INotationProjectPtr project = context()->currentProject()) {
        m_activeProfile = project->audioSettings()->activeSoundProfile().toQString();
        m_currentlySelectedProfile = m_activeProfile;
    }

    m_defaultProjectsProfile = config()->defaultProfileForNewProjects().toQString();

    endResetModel();
}

int SoundProfilesModel::rowCount(const QModelIndex& /*parent*/) const
{
    return static_cast<int>(m_profiles.size());
}

QVariant SoundProfilesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()
        || m_profiles.empty()
        || index.row() > static_cast<int>(m_profiles.size() - 1)) {
        return QVariant();
    }

    const SoundProfile& profile = m_profiles.at(index.row());

    switch (role) {
    case RoleTitle:
        return profile.name.toQString();
    case RoleEnabled:
        return profile.isEnabled();
    }

    return QVariant();
}

QHash<int, QByteArray> SoundProfilesModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { RoleTitle, "titleRole" },
        { RoleEnabled, "isEnabledRole" }
    };

    return roles;
}

const QString& SoundProfilesModel::activeProfile() const
{
    return m_activeProfile;
}

void SoundProfilesModel::setActiveProfile(const QString& newActiveProfile)
{
    if (m_activeProfile == newActiveProfile) {
        return;
    }

    if (!askAboutChangingSounds()) {
        return;
    }

    m_activeProfile = newActiveProfile;

    if (INotationProjectPtr project = context()->currentProject()) {
        SoundProfileName activeProfileName = SoundProfileName::fromQString(newActiveProfile);
        project->audioSettings()->setActiveSoundProfile(activeProfileName);
        controller()->applyProfile(activeProfileName);
    }

    emit activeProfileChanged();
}

const QString& SoundProfilesModel::defaultProjectsProfile() const
{
    return m_defaultProjectsProfile;
}

void SoundProfilesModel::setDefaultProjectsProfile(const QString& newDefaultProjectsProfile)
{
    if (m_defaultProjectsProfile == newDefaultProjectsProfile) {
        return;
    }

    m_defaultProjectsProfile = newDefaultProjectsProfile;
    config()->setDefaultProfileForNewProjects(SoundProfileName::fromQString(newDefaultProjectsProfile));

    emit defaultProjectsProfileChanged();
}

mu::notation::INotationPlaybackPtr SoundProfilesModel::notationPlayback() const
{
    project::INotationProjectPtr project = context()->currentProject();
    return project ? project->masterNotation()->playback() : nullptr;
}

bool SoundProfilesModel::askAboutChangingSounds()
{
    if (!config()->needToShowResetSoundFlagsWhenChangePlaybackProfileWarning()) {
        return true;
    }

    if (!notationPlayback()->hasSoundFlags(notationPlayback()->existingTrackIdSet())) {
        return true;
    }

    int changeBtn = int(muse::IInteractive::Button::Apply);
    muse::IInteractive::Options options = muse::IInteractive::Option::WithIcon | muse::IInteractive::Option::WithDontShowAgainCheckBox;
    muse::IInteractive::ButtonDatas buttons = {
        interactive()->buttonData(muse::IInteractive::Button::Cancel),
        muse::IInteractive::ButtonData(changeBtn, muse::trc("playback", "Change sounds"), true /*accent*/)
    };

    muse::IInteractive::Result result = interactive()->warning(muse::trc("playback", "Are you sure you want to change sounds?"),
                                                               muse::trc("playback",
                                                                         "Sound flags may be reset, but staff text will remain. This action can’t be undone."),
                                                               buttons, changeBtn, options);

    if (result.button() == changeBtn) {
        if (!result.showAgain()) {
            config()->setNeedToShowResetSoundFlagsWhenChangePlaybackProfileWarning(false);
        }

        return true;
    } else {
        return false;
    }
}

const QString& SoundProfilesModel::currentlySelectedProfile() const
{
    return m_currentlySelectedProfile;
}

void SoundProfilesModel::setCurrentlySelectedProfile(const QString& newCurrentlySelectedProfile)
{
    if (m_currentlySelectedProfile == newCurrentlySelectedProfile) {
        return;
    }
    m_currentlySelectedProfile = newCurrentlySelectedProfile;
    emit currentlySelectedProfileChanged();
}
