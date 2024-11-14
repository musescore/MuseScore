#include "outputresourceitem.h"

#include <QList>

#include "log.h"
#include "translation.h"
#include "stringutils.h"

using namespace mu::playback;
using namespace muse;
using namespace muse::audio;

static const QString& NO_FX_MENU_ITEM_ID()
{
    static std::string id = muse::trc("playback", "No effect");
    static QString resultStr = QString::fromStdString(id);
    return resultStr;
}

static const QString GET_MORE_EFFECTS("getMoreEffects");

OutputResourceItem::OutputResourceItem(QObject* parent, const audio::AudioFxParams& params)
    : AbstractAudioResourceItem(parent),
    m_currentFxParams(params)
{
}

void OutputResourceItem::requestAvailableResources()
{
    playback()->audioOutput()->availableOutputResources()
    .onResolve(this, [this](const AudioResourceMetaList& availableFxResources) {
        updateAvailableFxVendorsMap(availableFxResources);

        QVariantList result;

        if (!isBlank()) {
            const QString& currentResourceId = QString::fromStdString(m_currentFxParams.resourceMeta.id);
            result << buildMenuItem(currentResourceId,
                                    currentResourceId,
                                    true /*checked*/);

            result << buildSeparator();
        }

        // add "no fx" item
        result << buildMenuItem(NO_FX_MENU_ITEM_ID(),
                                NO_FX_MENU_ITEM_ID(),
                                m_currentFxParams.resourceMeta.id.empty());

        if (!m_fxByVendorMap.empty()) {
            result << buildSeparator();
        }

        for (const auto& pair : m_fxByVendorMap) {
            const QString& vendor = QString::fromStdString(pair.first);

            QVariantList subItems;

            for (const AudioResourceMeta& fxResourceMeta : pair.second) {
                const QString& resourceId = QString::fromStdString(fxResourceMeta.id);
                subItems << buildMenuItem(resourceId,
                                          resourceId,
                                          m_currentFxParams.resourceMeta.id == fxResourceMeta.id);
            }

            result << buildMenuItem(vendor,
                                    vendor,
                                    m_currentFxParams.resourceMeta.vendor == pair.first,
                                    subItems);
        }

        result << buildSeparator();
        result << buildExternalLinkMenuItem(GET_MORE_EFFECTS, muse::qtrc("playback", "Get more effects"));

        emit availableResourceListResolved(result);
    })
    .onReject(this, [](const int errCode, const std::string& errText) {
        LOGE() << "Unable to resolve available output resources"
               << " , errCode:" << errCode
               << " , errText:" << errText;
    });
}

void OutputResourceItem::handleMenuItem(const QString& menuItemId)
{
    if (menuItemId == NO_FX_MENU_ITEM_ID()) {
        updateCurrentFxParams(AudioResourceMeta());
        return;
    } else if (menuItemId == GET_MORE_EFFECTS) {
        const QString url = QString::fromStdString(globalConfiguration()->museHubWebUrl());
        const QString urlParams("plugins?utm_source=mss-mixer-fx&utm_medium=mh-fx&utm_campaign=mss-mixer-fx-mainpage");
        interactive()->openUrl(url + urlParams);
        return;
    }

    const AudioResourceId& newSelectedResourceId = menuItemId.toStdString();

    for (const auto& pair : m_fxByVendorMap) {
        for (const AudioResourceMeta& fxResourceMeta : pair.second) {
            if (newSelectedResourceId != fxResourceMeta.id) {
                continue;
            }

            updateCurrentFxParams(fxResourceMeta);
        }
    }
}

const AudioFxParams& OutputResourceItem::params() const
{
    return m_currentFxParams;
}

void OutputResourceItem::setParams(const audio::AudioFxParams& params)
{
    if (m_currentFxParams == params) {
        return;
    }

    bool activeChanged = m_currentFxParams.active != params.active;
    bool resourceChanged = m_currentFxParams.resourceMeta.id != params.resourceMeta.id;
    bool blankChanged = m_currentFxParams.isValid() != params.isValid();

    m_currentFxParams = params;
    emit fxParamsChanged();

    if (activeChanged) {
        emit isActiveChanged();
    }

    if (resourceChanged) {
        emit titleChanged();
    }

    if (blankChanged) {
        emit isBlankChanged();
    }
}

QString OutputResourceItem::title() const
{
    return QString::fromStdString(m_currentFxParams.resourceMeta.id);
}

bool OutputResourceItem::isActive() const
{
    return m_currentFxParams.active;
}

QString OutputResourceItem::id() const
{
    return QString::number(m_currentFxParams.chainOrder);
}

void OutputResourceItem::setIsActive(bool newIsActive)
{
    if (m_currentFxParams.active == newIsActive) {
        return;
    }

    m_currentFxParams.active = newIsActive;

    emit isActiveChanged();
    emit fxParamsChanged();
}

void OutputResourceItem::updateCurrentFxParams(const AudioResourceMeta& newMeta)
{
    if (m_currentFxParams.resourceMeta == newMeta) {
        return;
    }

    audio::AudioFxParams newParams = m_currentFxParams;
    newParams.resourceMeta = newMeta;
    newParams.active = newMeta.isValid();

    setParams(newParams);
    updateNativeEditorView();
}

void OutputResourceItem::updateAvailableFxVendorsMap(const audio::AudioResourceMetaList& availableFxResources)
{
    m_fxByVendorMap.clear();

    for (const auto& meta : availableFxResources) {
        AudioResourceMetaList& fxResourceList = m_fxByVendorMap[meta.vendor];
        fxResourceList.push_back(meta);
    }

    for (auto& [vendor, fxResourceList] : m_fxByVendorMap) {
        sortResourcesList(fxResourceList);
    }
}

bool OutputResourceItem::isBlank() const
{
    return !m_currentFxParams.isValid();
}

bool OutputResourceItem::hasNativeEditorSupport() const
{
    return m_currentFxParams.resourceMeta.hasNativeEditorSupport;
}
