#include "spacersettingsmodel.h"

#include "dataformatter.h"

#include "translation.h"

using namespace mu::inspector;

SpacerSettingsModel::SpacerSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_SPACER);
    setTitle(qtrc("inspector", "Spacer"));
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
    loadPropertyItem(m_spacerHeight, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toDouble());
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
