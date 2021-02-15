#include "temposettingsmodel.h"

#include "dataformatter.h"

#include "translation.h"

using namespace mu::inspector;

TempoSettingsModel::TempoSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_TEMPO);
    setTitle(qtrc("inspector", "Tempo"));
    createProperties();
}

void TempoSettingsModel::createProperties()
{
    m_isDefaultTempoForced = buildPropertyItem(Ms::Pid::TEMPO_FOLLOW_TEXT, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), newValue);

        emit requestReloadPropertyItems();
    });

    m_tempo = buildPropertyItem(Ms::Pid::TEMPO);
}

void TempoSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::TEMPO_TEXT);
}

void TempoSettingsModel::loadProperties()
{
    loadPropertyItem(m_isDefaultTempoForced);
    loadPropertyItem(m_tempo, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toDouble());
    });
}

void TempoSettingsModel::resetProperties()
{
    m_isDefaultTempoForced->resetToDefault();
    m_tempo->resetToDefault();
}

PropertyItem* TempoSettingsModel::isDefaultTempoForced() const
{
    return m_isDefaultTempoForced;
}

PropertyItem* TempoSettingsModel::tempo() const
{
    return m_tempo;
}
