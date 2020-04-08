#include "generalsettingsmodel.h"

GeneralSettingsModel::GeneralSettingsModel(QObject* parent, IElementRepositoryService* repository) : AbstractInspectorModel(parent, repository)
{
    createProperties();

    setTitle(tr("General"));
    setType(GENERAL);
    setPlaybackProxyModel(new PlaybackProxyModel(this, repository));
}

void GeneralSettingsModel::createProperties()
{
    m_isVisible = buildPropertyItem(Ms::Pid::VISIBLE);
    m_isAutoPlaceAllowed = buildPropertyItem(Ms::Pid::AUTOPLACE);
    m_isPlayable = buildPropertyItem(Ms::Pid::PLAY);
    m_isPlayModeAvailable = buildPropertyItem(Ms::Pid::PLAY);
    m_isSmall = buildPropertyItem(Ms::Pid::SMALL);
    m_isSmallModeAvailable = buildPropertyItem(Ms::Pid::SMALL);
}

void GeneralSettingsModel::requestElements()
{
    m_elementList = m_repository->takeAllElements();
}

void GeneralSettingsModel::loadProperties()
{
    loadPropertyItem(m_isVisible);
    loadPropertyItem(m_isAutoPlaceAllowed);
    loadPropertyItem(m_isPlayable);
    loadPropertyItem(m_isSmall);

    //@note Some elements support "Play" property, some don't. If element doesn't support property it'll return invalid value.
    //      So we use that knowledge here
    loadPropertyItem(m_isPlayModeAvailable, [this] (const QVariant& elementPropertyValue) -> QVariant {
        return elementPropertyValue.isValid();
    });

    //@note Some elements support "Play" property, some don't. If element doesn't support property it'll return invalid value.
    //      So we use that knowledge here
    loadPropertyItem(m_isSmallModeAvailable, [this] (const QVariant& elementPropertyValue) -> QVariant {
        return elementPropertyValue.isValid();
    });
}

void GeneralSettingsModel::resetProperties()
{
    m_isVisible->resetToDefault();
    m_isAutoPlaceAllowed->resetToDefault();
}

PropertyItem* GeneralSettingsModel::isVisible() const
{
    return m_isVisible;
}

PropertyItem* GeneralSettingsModel::isAutoPlaceAllowed() const
{
    return m_isAutoPlaceAllowed;
}

PropertyItem* GeneralSettingsModel::isPlayable() const
{
    return m_isPlayable;
}

PropertyItem* GeneralSettingsModel::isPlayModeAvailable() const
{
    return m_isPlayModeAvailable;
}

PropertyItem* GeneralSettingsModel::isSmall() const
{
    return m_isSmall;
}

PropertyItem* GeneralSettingsModel::isSmallModeAvailable() const
{
    return m_isSmallModeAvailable;
}

QObject* GeneralSettingsModel::playbackProxyModel() const
{
    return m_playbackProxyModel;
}

void GeneralSettingsModel::setPlaybackProxyModel(PlaybackProxyModel *playbackProxyModel)
{
    m_playbackProxyModel = playbackProxyModel;
    emit playbackProxyModelChanged(m_playbackProxyModel);
}
