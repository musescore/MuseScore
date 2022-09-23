#include "inputresourceitem.h"

#include <QList>

#include "log.h"
#include "translation.h"
#include "stringutils.h"

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
