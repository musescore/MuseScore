#ifndef INSPECTORPROPERTY_H
#define INSPECTORPROPERTY_H

#include <QObject>
#include <QVariant>

class PropertyItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(QVariant defaultValue READ defaultValue NOTIFY defaultValueChanged)
    Q_PROPERTY(bool isUndefined READ isUndefined NOTIFY isUndefinedChanged)
    Q_PROPERTY(bool isEnabled READ isEnabled NOTIFY isEnabledChanged)
    Q_PROPERTY(bool isStyled READ isStyled NOTIFY isStyledChanged)
    Q_PROPERTY(bool isModified READ isModified NOTIFY isModifiedChanged)

public:
    explicit PropertyItem(const int propertyId, QObject* parent = nullptr);

    void fillValues(const QVariant& currentValue, const QVariant& defaultValue);
    void updateCurrentValue(const QVariant& currentValue);

    Q_INVOKABLE void resetToDefault();
    Q_INVOKABLE void applyToStyle();

    int propertyId() const;
    QVariant value() const;
    QVariant defaultValue() const;
    bool isUndefined() const;
    bool isEnabled() const;
    bool isStyled() const;
    bool isModified() const;

    void setStyleId(const int styleId);

public slots:
    void setValue(const QVariant& value);
    void setDefaultValue(const QVariant& defaultValue);
    void setIsEnabled(bool isEnabled);
    void setIsStyled(bool isStyled);

signals:
    void valueChanged(QVariant value);
    void defaultValueChanged(QVariant defaultValue);
    void isUndefinedChanged(bool isUndefined);
    void isEnabledChanged(bool isEnabled);
    void isStyledChanged(bool isStyled);
    void isModifiedChanged(bool isModified);

    void propertyModified(int propertyId, QVariant newValue);
    void applyToStyleRequested(int styledId, QVariant newStyleValue);

private:
    int m_propertyId = -1;
    int m_styleId = -1;

    QVariant m_defaultValue = 0;
    QVariant m_currentValue = 0;
    bool m_isEnabled = false;
    bool m_isStyled = false;
};

#endif // INSPECTORPROPERTY_H
