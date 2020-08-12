#include "generalsettingsmodel.h"

GeneralSettingsModel::GeneralSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    createProperties();

    setTitle(tr("General"));
    setSectionType(SECTION_GENERAL);
    setPlaybackProxyModel(new PlaybackProxyModel(this, repository));
    setAppearanceSettingsModel(new AppearanceSettingsModel(this, repository));
}

void GeneralSettingsModel::createProperties()
{
    m_isVisible = buildPropertyItem(Ms::Pid::VISIBLE);
    m_isAutoPlaceAllowed = buildPropertyItem(Ms::Pid::AUTOPLACE);
    m_isPlayable = buildPropertyItem(Ms::Pid::PLAY);
    m_isSmall = buildPropertyItem(Ms::Pid::SMALL);
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
}

void GeneralSettingsModel::resetProperties()
{
    m_isVisible->resetToDefault();
    m_isAutoPlaceAllowed->resetToDefault();
    m_isPlayable->resetToDefault();
    m_isSmall->resetToDefault();
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

PropertyItem* GeneralSettingsModel::isSmall() const
{
    return m_isSmall;
}

QObject* GeneralSettingsModel::playbackProxyModel() const
{
    return m_playbackProxyModel;
}

QObject* GeneralSettingsModel::appearanceSettingsModel() const
{
    return m_appearanceSettingsModel;
}

void GeneralSettingsModel::setPlaybackProxyModel(PlaybackProxyModel* playbackProxyModel)
{
    m_playbackProxyModel = playbackProxyModel;
    emit playbackProxyModelChanged(m_playbackProxyModel);
}

void GeneralSettingsModel::setAppearanceSettingsModel(AppearanceSettingsModel* appearanceSettingsModel)
{
    m_appearanceSettingsModel = appearanceSettingsModel;
    emit appearanceSettingsModelChanged(m_appearanceSettingsModel);
}
