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

namespace mu::mpe {
struct PlaybackSetupData
{
    using ID = std::variant<SoundId, mu::String>;

    ID id = SoundId::Undefined;
    SoundCategory category = SoundCategory::Undefined;
    SoundSubCategories subCategorySet;

    std::optional<std::string> musicXmlSoundId;

    PlaybackSetupData() = default;

    PlaybackSetupData(ID id, SoundCategory category, SoundSubCategories&& subCategorySet = {})
        : id(id), category(category), subCategorySet(std::move(subCategorySet))
    {}

    SoundId soundId() const
    {
        if (std::holds_alternative<SoundId>(id)) {
            return std::get<SoundId>(id);
        }

        return SoundId::Undefined;
    }

    bool contains(const SoundSubCategory subcategory) const
    {
        return subCategorySet.find(subcategory) != subCategorySet.cend();
    }

    bool operator==(const PlaybackSetupData& other) const
    {
        return id == other.id
               && category == other.category
               && subCategorySet == other.subCategorySet;
    }

    bool operator<(const PlaybackSetupData& other) const
    {
        if (other.id > id) {
            return true;
        } else if (other.id == id) {
            if (other.category > category) {
                return true;
            } else if (other.category == category) {
                return other.subCategorySet > subCategorySet;
            }
        }

        return false;
    }

    bool isValid() const
    {
        if (category == SoundCategory::Undefined) {
            return false;
        }

        if (std::holds_alternative<SoundId>(id)) {
            return std::get<SoundId>(id) != SoundId::Undefined;
        } else if (std::holds_alternative<String>(id)) {
            return !std::get<String>(id).empty();
        }

        return false;
    }

    String toString() const
    {
        String result;

        String idAsStr;
        if (std::holds_alternative<SoundId>(id)) {
            idAsStr = soundIdToString(std::get<SoundId>(id));
        } else if (std::holds_alternative<String>(id)) {
            idAsStr = std::get<String>(id);
        }

        if (!subCategorySet.empty()) {
            result = String(u"%1.%2.%3")
                     .arg(soundCategoryToString(category))
                     .arg(idAsStr)
                     .arg(subCategorySet.toString());
        } else {
            result = String(u"%1.%2")
                     .arg(soundCategoryToString(category))
                     .arg(idAsStr);
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

        SoundSubCategories subCategories;
        if (subStrList.size() == 3) {
            subCategories = SoundSubCategories::fromString(subStrList.at(2));
        }

        ID id;

        SoundId soundId = soundIdFromString(subStrList.at(1));
        if (soundId != SoundId::Undefined) {
            id = soundId;
        } else {
            id = subStrList.at(1);
        }

        PlaybackSetupData result = {
            id,
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
