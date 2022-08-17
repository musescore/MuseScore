#include "inputresourceitem.h"

#include <QList>

#include "log.h"
#include "translation.h"
#include "stringutils.h"

// Note that this isn't ideal to be a dependency here; however the audio resource structure is limited in data storage and
// so MuseSampler packs extra info into the resourceMeta.id field.  Including the helper here avoids code duplication.
#include "musesampler/internal/musesamplerutils.h"

using namespace mu::playback;
using namespace mu::audio;

static const QString VST_MENU_ITEM_ID("VST3");
static const QString SOUNDFONTS_MENU_ITEM_ID = QString::fromStdString(mu::trc("playback", "SoundFonts"));
static const QString MUSE_MENU_ITEM_ID("Muse Sounds");

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
            const QString& currentResourceId = QString::fromStdString(m_currentInputParams.resourceMeta.id);
            result << buildMenuItem(currentResourceId,
                                    currentResourceId,
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
    if (m_currentInputParams.type() == mu::audio::AudioSourceType::MuseSampler)
    {
        if (auto id = musesampler::getMuseInstrumentNameFromId(m_currentInputParams.resourceMeta.id); id.has_value())
            return QString::fromStdString(*id);
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
    std::string currentCategory;
    if (auto category = musesampler::getMuseInstrumentCategoryFromId(m_currentInputParams.resourceMeta.id); category.has_value())
        currentCategory = *category;
    QVariantList subItemsByType;

    for (const auto& pair : resourcesByVendor) {
        const QString& vendor = QString::fromStdString(pair.first);

        QVariantList subItemsByVendor;

        std::map<std::string, std::vector<std::tuple<int, std::string, const AudioResourceMeta*>>> categoryMap;
        for (const AudioResourceMeta& resourceMeta : pair.second) {
            auto category = musesampler::getMuseInstrumentCategoryFromId(resourceMeta.id);
            auto name = musesampler::getMuseInstrumentNameFromId(resourceMeta.id);
            auto unique_id = musesampler::getMuseInstrumentUniqueIdFromId(resourceMeta.id);
            if (category.has_value() && name.has_value() && unique_id.has_value())
                categoryMap[*category].push_back({*unique_id, *name, &resourceMeta});
        }

        for (const auto& category : categoryMap)
        {
            QVariantList subItemsByCategory;
            for (auto& inst : category.second)
            {
                std::string myId = musesampler::buildMuseInstrumentId(category.first, std::get<1>(inst), std::get<0>(inst));
                const QString& instName = QString::fromStdString(std::get<1>(inst));
                const QString& instId = QString::fromStdString(myId);
                subItemsByCategory << buildMenuItem(instId,
                                                    instName,
                                                    m_currentInputParams.resourceMeta.id == myId);
            }

            const QString& categoryString = QString::fromStdString(category.first);
            subItemsByVendor << buildMenuItem(categoryString,
                                              categoryString,
                                              currentCategory == category.first,
                                              subItemsByCategory);
        }

        subItemsByType << buildMenuItem(vendor,
                                        vendor,
                                        m_currentInputParams.resourceMeta.vendor == pair.first,
                                        subItemsByVendor);
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
    QVariantList subItemsByType;

    for (const auto& pair : resourcesByVendor) {
        for (const AudioResourceMeta& resourceMeta : pair.second) {
            const QString& resourceId = QString::fromStdString(resourceMeta.id);

            subItemsByType << buildMenuItem(resourceId,
                                            resourceId,
                                            m_currentInputParams.resourceMeta.id == resourceMeta.id);
        }
    }

    return buildMenuItem(SOUNDFONTS_MENU_ITEM_ID,
                         SOUNDFONTS_MENU_ITEM_ID,
                         m_currentInputParams.resourceMeta.type == AudioResourceType::FluidSoundfont,
                         subItemsByType);
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
