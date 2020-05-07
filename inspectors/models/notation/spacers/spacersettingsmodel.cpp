#include "spacersettingsmodel.h"

SpacerSettingsModel::SpacerSettingsModel(QObject* parent, IElementRepositoryService* repository) :
    AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_SPACER);
    setTitle(tr("Spacer"));
    createProperties();
}

void SpacerSettingsModel::createProperties()
{
    m_spacerHeight = buildPropertyItem(Ms::Pid::SPACE);
}

void SpacerSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::SPACER);
}

void SpacerSettingsModel::loadProperties()
{
    loadPropertyItem(m_spacerHeight, [] (const QVariant& elementPropertyValue) -> QVariant {
        return QString::number(elementPropertyValue.toDouble(), 'f', 2).toDouble();
    });
}

void SpacerSettingsModel::resetProperties()
{
    m_spacerHeight->resetToDefault();
}

PropertyItem* SpacerSettingsModel::spacerHeight() const
{
    return m_spacerHeight;
}
