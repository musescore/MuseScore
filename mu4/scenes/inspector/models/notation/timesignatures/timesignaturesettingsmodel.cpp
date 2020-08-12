#include "timesignaturesettingsmodel.h"

#include <QSizeF>

#include "dataformatter.h"

TimeSignatureSettingsModel::TimeSignatureSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_TIME_SIGNATURE);
    setTitle(tr("Time signature"));
    createProperties();
}

void TimeSignatureSettingsModel::createProperties()
{
    m_horizontalScale = buildPropertyItem(Ms::Pid::SCALE, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), QSizeF(newValue.toDouble() / 100, m_verticalScale->value().toDouble() / 100));
    });

    m_verticalScale = buildPropertyItem(Ms::Pid::SCALE, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), QSizeF(m_horizontalScale->value().toDouble() / 100, newValue.toDouble() / 100));
    });

    m_shouldShowCourtesy = buildPropertyItem(Ms::Pid::SHOW_COURTESY);
}

void TimeSignatureSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::TIMESIG);
}

void TimeSignatureSettingsModel::loadProperties()
{
    loadPropertyItem(m_horizontalScale, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toSizeF().width()) * 100;
    });

    loadPropertyItem(m_verticalScale, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toSizeF().height()) * 100;
    });

    loadPropertyItem(m_shouldShowCourtesy);
}

void TimeSignatureSettingsModel::resetProperties()
{
    m_horizontalScale->resetToDefault();
    m_verticalScale->resetToDefault();
    m_shouldShowCourtesy->resetToDefault();
}

void TimeSignatureSettingsModel::showTimeSignatureProperties()
{
    adapter()->showTimeSignaturePropertiesDialog();
}

PropertyItem* TimeSignatureSettingsModel::horizontalScale() const
{
    return m_horizontalScale;
}

PropertyItem* TimeSignatureSettingsModel::verticalScale() const
{
    return m_verticalScale;
}

PropertyItem* TimeSignatureSettingsModel::shouldShowCourtesy() const
{
    return m_shouldShowCourtesy;
}
