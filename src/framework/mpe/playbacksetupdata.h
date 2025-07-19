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

#ifndef MUSE_MPE_PLAYBACKSETUPDATA_H
#define MUSE_MPE_PLAYBACKSETUPDATA_H

#include <variant>
#include <optional>

#include "soundid.h"

namespace muse::mpe {
struct PlaybackSetupData
{
    String id;
    SoundCategory category = SoundCategory::Undefined;
    StringList subCategories;

    bool supportsSingleNoteDynamics = false;
    std::optional<std::string> musicXmlSoundId;

    PlaybackSetupData() = default;

    PlaybackSetupData(SoundId id, SoundCategory category, SoundSubCategories&& soundSubCategories = {}, bool supportsSND = false)
        : id(soundIdToString(id)), category(category), supportsSingleNoteDynamics(supportsSND)
    {
        for (SoundSubCategory subCategory : soundSubCategories) {
            subCategories.push_back(soundSubCategoryToString(subCategory));
        }
    }

    PlaybackSetupData(String id, SoundCategory category, StringList&& subCategories = {})
        : id(std::move(id)), category(category), subCategories(std::move(subCategories))
    {}

    SoundId soundId() const
    {
        return soundIdFromString(id);
    }

    SoundSubCategories soundSubCategories() const
    {
        SoundSubCategories result;
        for (const String& subCategory : subCategories) {
            result.insert(soundSubCategoryFromString(subCategory));
        }

        return result;
    }

    bool isKnownSound() const
    {
        if (soundIdFromString(id) == SoundId::Unknown) {
            return false;
        }

        for (const String& subCategory : subCategories) {
            if (soundSubCategoryFromString(subCategory) == SoundSubCategory::Unknown) {
                return false;
            }
        }

        return true;
    }

    bool contains(const SoundSubCategory subcategory) const
    {
        return muse::contains(subCategories, soundSubCategoryToString(subcategory));
    }

    void add(const SoundSubCategory subcategory)
    {
        auto insertPos = std::find_if(subCategories.cbegin(), subCategories.cend(), [subcategory](const String& str) {
            SoundSubCategory existingSubCategory = soundSubCategoryFromString(str);
            return subcategory < existingSubCategory;
        });

        subCategories.insert(insertPos, soundSubCategoryToString(subcategory));
    }

    bool operator==(const PlaybackSetupData& other) const
    {
        return id == other.id
               && category == other.category
               && subCategories == other.subCategories
               && supportsSingleNoteDynamics == other.supportsSingleNoteDynamics;
    }

    bool operator<(const PlaybackSetupData& other) const
    {
        if (other.id > id) {
            return true;
        } else if (other.id == id) {
            if (other.category > category) {
                return true;
            } else if (other.category == category) {
                return other.subCategories > subCategories;
            }
        }

        return false;
    }

    bool isValid() const
    {
        if (category == SoundCategory::Undefined) {
            return false;
        }

        if (id.empty()) {
            return false;
        }

        if (id == ID_STRINGS.at(SoundId::Undefined)) {
            return false;
        }

        return true;
    }

    String toString() const
    {
        String result;

        if (!subCategories.empty()) {
            result = String(u"%1.%2.%3")
                     .arg(soundCategoryToString(category))
                     .arg(id)
                     .arg(subCategories.join(u":"));
        } else {
            result = String(u"%1.%2")
                     .arg(soundCategoryToString(category))
                     .arg(id);
        }

        return result;
    }

    static PlaybackSetupData fromString(const String& str)
    {
        if (str.empty()) {
            return PlaybackSetupData();
        }

        StringList subStrList = str.split(u".");

        if (subStrList.size() < 2) {
            return PlaybackSetupData();
        }

        StringList subCategories;
        if (subStrList.size() == 3) {
            subCategories = subStrList.at(2).split(u":");
        }

        PlaybackSetupData result = {
            subStrList.at(1),
            soundCategoryFromString(subStrList.at(0)),
            std::move(subCategories)
        };

        return result;
    }
};

static const PlaybackSetupData GENERIC_SETUP_DATA = {
    SoundId::Last,
    SoundCategory::Last,
    { SoundSubCategory::Last }
};

static const String GENERIC_SETUP_DATA_STRING = GENERIC_SETUP_DATA.toString();
}

#endif // MUSE_MPE_PLAYBACKSETUPDATA_H
