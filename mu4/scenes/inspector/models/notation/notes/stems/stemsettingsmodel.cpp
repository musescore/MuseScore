#include "stemsettingsmodel.h"

#include "types/stemtypes.h"
#include "dataformatter.h"

StemSettingsModel::StemSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_STEM);
    setTitle(tr("Stem"));

    createProperties();
}

void StemSettingsModel::createProperties()
{
    m_isStemHidden = buildPropertyItem(Ms::Pid::VISIBLE, [this](const int pid, const QVariant& isStemHidden) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), !isStemHidden.toBool());
    });

    m_thickness = buildPropertyItem(Ms::Pid::LINE_WIDTH);
    m_length = buildPropertyItem(Ms::Pid::USER_LEN);
    m_stemDirection = buildPropertyItem(Ms::Pid::STEM_DIRECTION);

    m_horizontalOffset = buildPropertyItem(Ms::Pid::OFFSET, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), QPointF(newValue.toDouble(), m_verticalOffset->value().toDouble()));
    });

    m_verticalOffset = buildPropertyItem(Ms::Pid::OFFSET, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), QPointF(m_horizontalOffset->value().toDouble(), newValue.toDouble()));
    });
}

void StemSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::STEM);
}

void StemSettingsModel::loadProperties()
{
    loadPropertyItem(m_isStemHidden, [](const QVariant& isVisible) -> QVariant {
        return !isVisible.toBool();
    });

    loadPropertyItem(m_thickness, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toDouble());
    });

    loadPropertyItem(m_length, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toDouble());
    });

    loadPropertyItem(m_stemDirection);

    loadPropertyItem(m_horizontalOffset, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toPointF().x());
    });

    loadPropertyItem(m_verticalOffset, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toPointF().y());
    });
}

void StemSettingsModel::resetProperties()
{
    m_isStemHidden->resetToDefault();
    m_thickness->resetToDefault();
    m_length->resetToDefault();
    m_stemDirection->resetToDefault();
    m_horizontalOffset->resetToDefault();
    m_verticalOffset->resetToDefault();
}

PropertyItem* StemSettingsModel::isStemHidden() const
{
    return m_isStemHidden;
}

PropertyItem* StemSettingsModel::thickness() const
{
    return m_thickness;
}

PropertyItem* StemSettingsModel::length() const
{
    return m_length;
}

PropertyItem* StemSettingsModel::horizontalOffset() const
{
    return m_horizontalOffset;
}

PropertyItem* StemSettingsModel::verticalOffset() const
{
    return m_verticalOffset;
}

PropertyItem* StemSettingsModel::stemDirection() const
{
    return m_stemDirection;
}
