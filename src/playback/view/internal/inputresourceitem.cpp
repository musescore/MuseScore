/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#include "inputresourceitem.h"

#include <optional>

#include <QList>

#include "log.h"
#include "stringutils.h"
#include "translation.h"
#include "types/string.h"

#include "audio/itracks.h"
#include "audio/soundfonttypes.h"

#include "msbasicpresetscategories.h"

using namespace mu;
using namespace mu::playback;
using namespace mu::audio;
using namespace mu::audio::synth;

static const QString VST_MENU_ITEM_ID("VST3");
static const QString SOUNDFONTS_MENU_ITEM_ID = mu::qtrc("playback", "SoundFonts");
static const QString MUSE_MENU_ITEM_ID("Muse Sounds");

static const String MS_BASIC_SOUNDFONT_NAME(u"MS Basic");

InputResourceItem::InputResourceItem(QObject* parent)
    : AbstractAudioResourceItem(parent)
{
}

void InputResourceItem::requestAvailableResources()
{
    playback()->tracks()->availableInputResources()
    .onResolve(this, [this](const AudioResourceMetaList& availableResources) {
        updateAvailableResources(availableResources);

        QVariantList result;

        if (!isBlank()) {
            QString currentResourceId = QString::fromStdString(m_currentInputParams.resourceMeta.id);

            result << buildMenuItem(currentResourceId,
                                    title(),
                                    true /*checked*/);

            result << buildSeparator();
        }

        auto museResourcesSearch = m_availableResourceMap.find(AudioResourceType::MuseSamplerSoundPack);
        if (museResourcesSearch != m_availableResourceMap.end()) {
            result << buildMuseMenuItem(museResourcesSearch->second);

            result << buildSeparator();
        }

        auto vstResourcesSearch = m_availableResourceMap.find(AudioResourceType::VstPlugin);
        if (vstResourcesSearch != m_availableResourceMap.end()) {
            result << buildVstMenuItem(vstResourcesSearch->second);

            result << buildSeparator();
        }

        auto sfResourcesSearch = m_availableResourceMap.find(AudioResourceType::FluidSoundfont);
        if (sfResourcesSearch != m_availableResourceMap.end()) {
            result << buildSoundFontsMenuItem(sfResourcesSearch->second);
        }

        emit availableResourceListResolved(result);
    })
    .onReject(this, [](const int errCode, const std::string& errText) {
        LOGE() << "Unable to resolve available output resources"
               << " , errCode:" << errCode
               << " , errText:" << errText;
    });
}

void InputResourceItem::handleMenuItem(const QString& menuItemId)
{
    const AudioResourceId& newSelectedResourceId = menuItemId.toStdString();

    for (const auto& pairByType : m_availableResourceMap) {
        for (const auto& pairByVendor : pairByType.second) {
            for (const AudioResourceMeta& resourceMeta : pairByVendor.second) {
                if (newSelectedResourceId != resourceMeta.id) {
                    continue;
                }

                updateCurrentParams(resourceMeta);
            }
        }
    }
}

const AudioInputParams& InputResourceItem::params() const
{
    return m_currentInputParams;
}

void InputResourceItem::setParams(const audio::AudioInputParams& newParams)
{
    m_currentInputParams = newParams;

    emit titleChanged();
    emit isBlankChanged();
    emit isActiveChanged();
}

QString InputResourceItem::title() const
{
    if (m_currentInputParams.type() == mu::audio::AudioSourceType::MuseSampler) {
        return m_currentInputParams.resourceMeta.attributeVal(u"museName").toQString();
    }

    if (m_currentInputParams.resourceMeta.type == audio::AudioResourceType::FluidSoundfont) {
        const String& presetName = m_currentInputParams.resourceMeta.attributeVal(PRESET_NAME_ATTRIBUTE);
        if (!presetName.empty()) {
            return presetName.toQString();
        }

        const String& soundFontName = m_currentInputParams.resourceMeta.attributeVal(SOUNDFONT_NAME_ATTRIBUTE);
        if (!soundFontName.empty()) {
            return soundFontName.toQString();
        }
    }

    return QString::fromStdString(m_currentInputParams.resourceMeta.id);
}

bool InputResourceItem::isBlank() const
{
    return !m_currentInputParams.isValid();
}

bool InputResourceItem::isActive() const
{
    return m_currentInputParams.isValid();
}

bool InputResourceItem::hasNativeEditorSupport() const
{
    return m_currentInputParams.resourceMeta.hasNativeEditorSupport;
}

QVariantMap InputResourceItem::buildMuseMenuItem(const ResourceByVendorMap& resourcesByVendor) const
{
    String currentCategory = m_currentInputParams.resourceMeta.attributeVal(u"museCategory");

    QVariantList subItemsByType;

    for (const auto& pair : resourcesByVendor) {
        std::map<String, std::vector<std::tuple<int, String, const AudioResourceMeta&> > > categoryMap;
        for (const AudioResourceMeta& resourceMeta : pair.second) {
            const String& category = resourceMeta.attributeVal(u"museCategory");
            const String& name = resourceMeta.attributeVal(u"museName");
            int unique_id = resourceMeta.attributeVal(u"museUID").toInt();

            categoryMap[category].push_back({ unique_id, name, resourceMeta });
        }

        for (const auto& category : categoryMap) {
            QVariantList subItemsByCategory;
            for (const auto& inst : category.second) {
                QString instName = std::get<1>(inst).toQString();
                auto instId = std::get<2>(inst).id;
                subItemsByCategory << buildMenuItem(QString::fromStdString(instId),
                                                    instName,
                                                    m_currentInputParams.resourceMeta.id == instId);
            }

            QString categoryString = category.first.toQString();
            subItemsByType << buildMenuItem(categoryString,
                                            categoryString,
                                            currentCategory == category.first,
                                            subItemsByCategory);
        }
    }

    return buildMenuItem(MUSE_MENU_ITEM_ID,
                         MUSE_MENU_ITEM_ID,
                         m_currentInputParams.resourceMeta.type == AudioResourceType::MuseSamplerSoundPack,
                         subItemsByType);
}

QVariantMap InputResourceItem::buildVstMenuItem(const ResourceByVendorMap& resourcesByVendor) const
{
    QVariantList subItemsByType;

    for (const auto& pair : resourcesByVendor) {
        const QString& vendor = QString::fromStdString(pair.first);

        QVariantList subItemsByVendor;

        for (const AudioResourceMeta& resourceMeta : pair.second) {
            const QString& resourceId = QString::fromStdString(resourceMeta.id);
            subItemsByVendor << buildMenuItem(resourceId,
                                              resourceId,
                                              m_currentInputParams.resourceMeta.id == resourceMeta.id);
        }

        subItemsByType << buildMenuItem(vendor,
                                        vendor,
                                        m_currentInputParams.resourceMeta.vendor == pair.first,
                                        subItemsByVendor);
    }

    return buildMenuItem(VST_MENU_ITEM_ID,
                         VST_MENU_ITEM_ID,
                         m_currentInputParams.resourceMeta.type == AudioResourceType::VstPlugin,
                         subItemsByType);
}

QVariantMap InputResourceItem::buildSoundFontsMenuItem(const ResourceByVendorMap& resourcesByVendor) const
{
    // Get info about current resource
    const String& currentSoundFontName = m_currentInputParams.resourceMeta.attributeVal(SOUNDFONT_NAME_ATTRIBUTE);
    std::optional<midi::Program> currentPreset = std::nullopt;
    {
        if (!currentSoundFontName.empty()) {
            bool bankOk = false, programOk = false;
            int currentPresetBank = m_currentInputParams.resourceMeta.attributeVal(PRESET_BANK_ATTRIBUTE).toInt(&bankOk);
            int currentPresetProgram = m_currentInputParams.resourceMeta.attributeVal(PRESET_PROGRAM_ATTRIBUTE).toInt(&programOk);

            if (bankOk && programOk) {
                currentPreset = midi::Program(currentPresetBank, currentPresetProgram);
            }
        }
    }

    // Group resources by SoundFont name
    std::map<String, AudioResourceMetaList> resourcesBySoundFont;

    for (const auto& pair : resourcesByVendor) {
        for (const AudioResourceMeta& resourceMeta : pair.second) {
            const String& soundFontName = resourceMeta.attributeVal(SOUNDFONT_NAME_ATTRIBUTE);

            resourcesBySoundFont[soundFontName].push_back(resourceMeta);
        }
    }

    // Sort SoundFonts by name and add them to the menu
    std::vector<String> soundFonts = mu::keys(resourcesBySoundFont);
    std::sort(soundFonts.begin(), soundFonts.end(), [](const String& s1, const String& s2) {
        return strings::lessThanCaseInsensitive(s1, s2);
    });

    QVariantList soundFontItems;
    std::string currentSoundFontId = m_currentInputParams.resourceMeta.id;

    for (const String& soundFont : soundFonts) {
        // currentSoundFontId will be equal to soundFont in the case of "choose automatically" for older files (this is a temporary fix)
        // See: https://github.com/musescore/MuseScore/pull/20316#issuecomment-1841326774
        bool isCurrentSoundFont = currentSoundFontName == soundFont || currentSoundFontId == soundFont.toStdString();

        if (soundFont == MS_BASIC_SOUNDFONT_NAME) {
            soundFontItems << buildMsBasicMenuItem(resourcesBySoundFont[soundFont], isCurrentSoundFont, currentPreset);
        } else {
            soundFontItems << buildSoundFontMenuItem(soundFont, resourcesBySoundFont[soundFont], isCurrentSoundFont, currentPreset);
        }
    }

    return buildMenuItem(SOUNDFONTS_MENU_ITEM_ID,
                         SOUNDFONTS_MENU_ITEM_ID,
                         m_currentInputParams.resourceMeta.type == AudioResourceType::FluidSoundfont,
                         soundFontItems);
}

QVariantMap InputResourceItem::buildMsBasicMenuItem(const AudioResourceMetaList& availableResources, bool isCurrentSoundFont,
                                                    const std::optional<midi::Program>& currentPreset) const
{
    std::map<midi::Program, AudioResourceMeta> resourcesByProgram;
    AudioResourceMeta chooseAutomaticMeta;

    for (const AudioResourceMeta& resourceMeta : availableResources) {
        bool bankOk = false, programOk = false;
        int presetBank = resourceMeta.attributeVal(PRESET_BANK_ATTRIBUTE).toInt(&bankOk);
        int presetProgram = resourceMeta.attributeVal(PRESET_PROGRAM_ATTRIBUTE).toInt(&programOk);

        if (bankOk && programOk) {
            resourcesByProgram[midi::Program(presetBank, presetProgram)] = resourceMeta;
        } else {
            chooseAutomaticMeta = resourceMeta;
        }
    }

    std::function<QVariantMap(const MsBasicItem&, const QString&, bool&, bool&)> buildMsBasicItem
        = [&](const MsBasicItem& item, const QString& parentMenuId, bool& ok, bool& isCurrent) {
        ok = true;
        if (item.subItems.empty()) {
            auto it = resourcesByProgram.find(item.preset);
            if (it == resourcesByProgram.cend()) {
                LOGW() << "Preset specified in MS_BASIC_PRESET_CATEGORIES not found in SoundFont: bank " << item.preset.bank
                       << ", program " << item.preset.program;

                ok = false;
                return QVariantMap();
            }

            const AudioResourceMeta& resourceMeta = it->second;

            isCurrent = isCurrentSoundFont && currentPreset.has_value() && currentPreset.value() == item.preset;

            QString presetName = resourceMeta.attributeVal(PRESET_NAME_ATTRIBUTE);
            if (presetName.isEmpty()) {
                presetName = qtrc("playback", "Bank %1, preset %2").arg(item.preset.bank).arg(item.preset.program);
            }

            return buildMenuItem(QString::fromStdString(resourceMeta.id),
                                 presetName,
                                 isCurrent);
        }

        QString menuId = parentMenuId + "\\" + item.title + "\\menu";

        QVariantList subItems;

        for (const MsBasicItem& subItem : item.subItems) {
            bool _ok = false;
            bool isSubItemCurrent = false;

            QVariantMap menuItem = buildMsBasicItem(subItem, parentMenuId, _ok, isSubItemCurrent);
            if (!_ok) {
                continue;
            }

            subItems << menuItem;

            if (isSubItemCurrent) {
                isCurrent = true;
            }
        }

        return buildMenuItem(menuId,
                             item.title,
                             isCurrent,
                             subItems);
    };

    QString menuId = MS_BASIC_SOUNDFONT_NAME.toQString() + "\\menu";

    QVariantList categoryItems;

    for (const MsBasicItem& category : MS_BASIC_PRESET_CATEGORIES) {
        bool ok = false;
        bool isCurrent = false;

        categoryItems << buildMsBasicItem(category, menuId, ok, isCurrent);
    }

    // Prepend the "Choose automatically" item
    categoryItems.prepend(buildSeparator());
    categoryItems.prepend(buildMenuItem(QString::fromStdString(chooseAutomaticMeta.id),
                                        qtrc("playback", "Choose automatically"),
                                        isCurrentSoundFont && !currentPreset.has_value()));

    return buildMenuItem(menuId,
                         MS_BASIC_SOUNDFONT_NAME,
                         isCurrentSoundFont,
                         categoryItems);
}

QVariantMap InputResourceItem::buildSoundFontMenuItem(const String& soundFont, const audio::AudioResourceMetaList& availableResources,
                                                      bool isCurrentSoundFont, const std::optional<midi::Program>& currentPreset) const
{
    // Group resources by bank, and use this to sort them
    std::map<int, std::map<int, AudioResourceMeta> > resourcesByBank;
    AudioResourceMeta chooseAutomaticMeta;

    for (const AudioResourceMeta& resourceMeta : availableResources) {
        bool bankOk = false, programOk = false;
        int presetBank = resourceMeta.attributeVal(PRESET_BANK_ATTRIBUTE).toInt(&bankOk);
        int presetProgram = resourceMeta.attributeVal(PRESET_PROGRAM_ATTRIBUTE).toInt(&programOk);

        if (bankOk && programOk) {
            resourcesByBank[presetBank][presetProgram] = resourceMeta;
        } else {
            chooseAutomaticMeta = resourceMeta;
        }
    }

    QVariantList bankItems;

    for (const auto& bankPair : resourcesByBank) {
        bool isCurrentBank = isCurrentSoundFont && currentPreset.has_value() && currentPreset.value().bank == bankPair.first;

        QVariantList presetItems;

        for (const auto& presetPair : bankPair.second) {
            bool isCurrentPreset = isCurrentBank && currentPreset.value().program == presetPair.first;

            QString presetName = presetPair.second.attributeVal(PRESET_NAME_ATTRIBUTE);
            if (presetName.isEmpty()) {
                presetName = qtrc("playback", "Preset %1").arg(presetPair.first);
            }

            presetItems << buildMenuItem(QString::fromStdString(presetPair.second.id),
                                         presetName,
                                         isCurrentPreset);
        }

        bankItems << buildMenuItem(soundFont + u"\\" + String::number(bankPair.first),
                                   qtrc("playback", "Bank %1").arg(bankPair.first),
                                   isCurrentBank,
                                   presetItems);
    }

    // Prepend the "Choose automatically" item
    bankItems.prepend(buildSeparator());
    bankItems.prepend(buildMenuItem(QString::fromStdString(chooseAutomaticMeta.id),
                                    qtrc("playback", "Choose automatically"),
                                    isCurrentSoundFont && !currentPreset.has_value()));

    return buildMenuItem(soundFont + u"\\menu",
                         soundFont,
                         isCurrentSoundFont,
                         bankItems);
}

void InputResourceItem::updateCurrentParams(const AudioResourceMeta& newMeta)
{
    m_currentInputParams.resourceMeta = newMeta;

    emit titleChanged();
    emit isBlankChanged();
    emit isActiveChanged();
    emit inputParamsChanged();

    updateNativeEditorView();
}

void InputResourceItem::updateAvailableResources(const AudioResourceMetaList& availableResources)
{
    m_availableResourceMap.clear();

    for (const AudioResourceMeta& meta : availableResources) {
        ResourceByVendorMap& resourcesByVendor = m_availableResourceMap[meta.type];
        AudioResourceMetaList& resourcesMetaList = resourcesByVendor[meta.vendor];
        resourcesMetaList.push_back(meta);
    }

    for (auto& [type, resourcesByVendor] : m_availableResourceMap) {
        for (auto& [vendor, resourceMetaList] : resourcesByVendor) {
            sortResourcesList(resourceMetaList);
        }
    }
}
