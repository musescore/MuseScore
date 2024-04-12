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

#include "audiogenerationsettingsmodel.h"

using namespace mu::project;

AudioGenerationSettingsModel::AudioGenerationSettingsModel(QObject* parent)
    : QObject(parent)
{
}

int AudioGenerationSettingsModel::timePeriodType() const
{
    return static_cast<int>(configuration()->generateAudioTimePeriodType());
}

int AudioGenerationSettingsModel::numberOfSaves() const
{
    return configuration()->numberOfSavesToGenerateAudio();
}

void AudioGenerationSettingsModel::setTimePeriodType(int type)
{
    if (timePeriodType() == type) {
        return;
    }

    configuration()->setGenerateAudioTimePeriodType(static_cast<GenerateAudioTimePeriodType>(type));
    emit timePeriodTypeChanged();
}

void AudioGenerationSettingsModel::setNumberOfSaves(int number)
{
    if (numberOfSaves() == number) {
        return;
    }

    configuration()->setNumberOfSavesToGenerateAudio(number);
    emit numberOfSavesChanged();
}
