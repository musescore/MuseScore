#include "bendsettingsmodel.h"

#include "types/bendtypes.h"
#include "dataformatter.h"

BendSettingsModel::BendSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_BEND);
    setTitle(tr("Bend"));
    createProperties();
}

void BendSettingsModel::createProperties()
{
    m_bendType = buildPropertyItem(Ms::Pid::BEND_TYPE, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), newValue);

        if (newValue.toInt() != static_cast<int>(BendTypes::BendType::TYPE_CUSTOM)) {
            emit requestReloadPropertyItems();
        }
    });

    m_bendCurve = buildPropertyItem(Ms::Pid::BEND_CURVE, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), newValue);

        emit requestReloadPropertyItems();
    });

    m_lineThickness = buildPropertyItem(Ms::Pid::LINE_WIDTH);
}

void BendSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::BEND);

    emit areSettingsAvailableChanged(areSettingsAvailable());
}

void BendSettingsModel::loadProperties()
{
    loadPropertyItem(m_bendType);
    loadPropertyItem(m_bendCurve);
    loadPropertyItem(m_lineThickness, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toDouble());
    });
}

void BendSettingsModel::resetProperties()
{
    m_bendType->resetToDefault();
    m_bendCurve->resetToDefault();
    m_lineThickness->resetToDefault();
}

PropertyItem* BendSettingsModel::bendType() const
{
    return m_bendType;
}

PropertyItem* BendSettingsModel::lineThickness() const
{
    return m_lineThickness;
}

bool BendSettingsModel::areSettingsAvailable() const
{
    return m_elementList.count() == 1; // Bend inspector doesn't support multiple selection
}

PropertyItem* BendSettingsModel::bendCurve() const
{
    return m_bendCurve;
}
