#include "tremolobarsettingsmodel.h"

#include "types/tremolobartypes.h"
#include "dataformatter.h"

TremoloBarSettingsModel::TremoloBarSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_TREMOLOBAR);
    setTitle(tr("Tremolo bar"));
    createProperties();
}

void TremoloBarSettingsModel::createProperties()
{
    m_type = buildPropertyItem(Ms::Pid::TREMOLOBAR_TYPE, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), newValue);

        if (newValue.toInt() != static_cast<int>(TremoloBarTypes::TremoloBarType::TYPE_CUSTOM)) {
            emit requestReloadPropertyItems();
        }
    });

    m_curve = buildPropertyItem(Ms::Pid::TREMOLOBAR_CURVE, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), newValue);

        emit requestReloadPropertyItems();
    });

    m_lineThickness = buildPropertyItem(Ms::Pid::LINE_WIDTH);
    m_scale = buildPropertyItem(Ms::Pid::MAG);
}

void TremoloBarSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::TREMOLOBAR);

    emit areSettingsAvailableChanged(areSettingsAvailable());
}

void TremoloBarSettingsModel::loadProperties()
{
    loadPropertyItem(m_type);
    loadPropertyItem(m_curve);
    loadPropertyItem(m_lineThickness, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toDouble());
    });

    loadPropertyItem(m_scale, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toDouble());
    });
}

void TremoloBarSettingsModel::resetProperties()
{
    m_type->resetToDefault();
    m_curve->resetToDefault();
    m_lineThickness->resetToDefault();
    m_scale->resetToDefault();
}

PropertyItem* TremoloBarSettingsModel::type() const
{
    return m_type;
}

PropertyItem* TremoloBarSettingsModel::curve() const
{
    return m_curve;
}

PropertyItem* TremoloBarSettingsModel::lineThickness() const
{
    return m_lineThickness;
}

PropertyItem* TremoloBarSettingsModel::scale() const
{
    return m_scale;
}

bool TremoloBarSettingsModel::areSettingsAvailable() const
{
    return m_elementList.count() == 1; // TremoloBar inspector doesn't support multiple selection
}
