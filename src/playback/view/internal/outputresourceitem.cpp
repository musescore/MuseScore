#include "outputresourceitem.h"

#include <QList>

#include "log.h"
#include "translation.h"

using namespace mu::playback;
using namespace mu::audio;

static const QString& NO_FX_MENU_ITEM_ID()
{
    static std::string id = mu::trc("playback", "No effect");
    static QString resultStr = QString::fromStdString(id);
    return resultStr;
}

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
    }

    const AudioResourceId& newSelectedResourceId = menuItemId.toStdString();

    for (auto& pair : m_fxByVendorMap) {
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

    m_currentFxParams.resourceMeta = newMeta;
    m_currentFxParams.active = newMeta.isValid();

    emit isActiveChanged();
    emit titleChanged();
    emit fxParamsChanged();
    emit isBlankChanged();

    requestToLaunchNativeEditorView();
}

void OutputResourceItem::updateAvailableFxVendorsMap(const audio::AudioResourceMetaList& availableFxResources)
{
    m_fxByVendorMap.clear();

    for (const auto& meta : availableFxResources) {
        AudioResourceMetaList& fxResourceList = m_fxByVendorMap[meta.vendor];
        fxResourceList.push_back(meta);
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
