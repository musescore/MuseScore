#include "hooksettingsmodel.h"

#include "dataformatter.h"

HookSettingsModel::HookSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_HOOK);
    setTitle(tr("Hook"));

    createProperties();
}

void HookSettingsModel::createProperties()
{
    m_horizontalOffset = buildPropertyItem(Ms::Pid::OFFSET, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), QPointF(newValue.toDouble(), m_verticalOffset->value().toDouble()));
    });

    m_verticalOffset = buildPropertyItem(Ms::Pid::OFFSET, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), QPointF(m_horizontalOffset->value().toDouble(), newValue.toDouble()));
    });
}

void HookSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::HOOK);
}

void HookSettingsModel::loadProperties()
{
    loadPropertyItem(m_horizontalOffset, [this](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toPointF().x());
    });

    loadPropertyItem(m_verticalOffset, [this](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toPointF().y());
    });
}

void HookSettingsModel::resetProperties()
{
    m_horizontalOffset->resetToDefault();
    m_verticalOffset->resetToDefault();
}

PropertyItem* HookSettingsModel::horizontalOffset() const
{
    return m_horizontalOffset;
}

PropertyItem* HookSettingsModel::verticalOffset() const
{
    return m_verticalOffset;
}
