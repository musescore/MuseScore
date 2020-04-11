#include "propertyitem.h"

PropertyItem::PropertyItem(const int propertyId, QObject* parent) : QObject(parent)
{
    m_propertyId = propertyId;
}

void PropertyItem::fillValues(const QVariant& currentValue, const QVariant& defaultValue)
{
    updateCurrentValue(currentValue);

    m_defaultValue = defaultValue;
}

void PropertyItem::updateCurrentValue(const QVariant& currentValue)
{
    m_currentValue = currentValue;

    emit isUndefinedChanged(isUndefined());
    emit valueChanged(m_currentValue);
}

void PropertyItem::resetToDefault()
{
    setValue(m_defaultValue);
}

int PropertyItem::propertyId() const
{
    return m_propertyId;
}

QVariant PropertyItem::value() const
{
    return m_currentValue;
}

bool PropertyItem::isUndefined() const
{
    return !m_currentValue.isValid();
}

void PropertyItem::setValue(const QVariant& value)
{
    if (m_currentValue == value)
        return;

    updateCurrentValue(value);

    emit propertyModified(m_propertyId, m_currentValue);
}

void PropertyItem::setDefaultValue(const QVariant& defaultValue)
{
    m_defaultValue = defaultValue;
}
