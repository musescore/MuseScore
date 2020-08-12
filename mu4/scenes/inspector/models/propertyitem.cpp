#include "propertyitem.h"

PropertyItem::PropertyItem(const int propertyId, QObject* parent)
    : QObject(parent)
{
    m_propertyId = propertyId;
}

void PropertyItem::fillValues(const QVariant& currentValue, const QVariant& defaultValue)
{
    updateCurrentValue(currentValue);

    setDefaultValue(defaultValue);

    emit isModifiedChanged(isModified());
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

void PropertyItem::applyToStyle()
{
    emit applyToStyleRequested(m_styleId, m_currentValue);
}

int PropertyItem::propertyId() const
{
    return m_propertyId;
}

QVariant PropertyItem::value() const
{
    return m_currentValue;
}

QVariant PropertyItem::defaultValue() const
{
    return m_defaultValue;
}

bool PropertyItem::isUndefined() const
{
    return !m_currentValue.isValid();
}

bool PropertyItem::isEnabled() const
{
    return m_isEnabled;
}

bool PropertyItem::isStyled() const
{
    return m_isStyled;
}

bool PropertyItem::isModified() const
{
    return m_currentValue != m_defaultValue;
}

void PropertyItem::setStyleId(const int styleId)
{
    m_styleId = styleId;
}

void PropertyItem::setValue(const QVariant& value)
{
    if (m_currentValue == value) {
        return;
    }

    updateCurrentValue(value);

    emit propertyModified(m_propertyId, m_currentValue);
    emit isModifiedChanged(isModified());
}

void PropertyItem::setDefaultValue(const QVariant& defaultValue)
{
    m_defaultValue = defaultValue;
    emit defaultValueChanged(m_defaultValue);
}

void PropertyItem::setIsEnabled(bool isEnabled)
{
    if (m_isEnabled == isEnabled) {
        return;
    }

    m_isEnabled = isEnabled;
    emit isEnabledChanged(m_isEnabled);
}

void PropertyItem::setIsStyled(bool isStyled)
{
    if (m_isStyled == isStyled) {
        return;
    }

    m_isStyled = isStyled;
    emit isStyledChanged(m_isStyled);
}
