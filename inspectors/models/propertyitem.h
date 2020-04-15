#ifndef INSPECTORPROPERTY_H
#define INSPECTORPROPERTY_H

#include <QObject>
#include <QVariant>

class PropertyItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(bool isUndefined READ isUndefined NOTIFY isUndefinedChanged)
    Q_PROPERTY(bool isEnabled READ isEnabled NOTIFY isEnabledChanged)

public:
    explicit PropertyItem(const int propertyId, QObject* parent = nullptr);

    void fillValues(const QVariant& currentValue, const QVariant& defaultValue);
    void updateCurrentValue(const QVariant& currentValue);
    void resetToDefault();

    int propertyId() const;
    QVariant value() const;
    bool isUndefined() const;
    bool isEnabled() const;

public slots:
    void setValue(const QVariant& value);
    void setDefaultValue(const QVariant& defaultValue);
    void setIsEnabled(bool isEnabled);

signals:
    void valueChanged(QVariant value);
    void isUndefinedChanged(bool isUndefined);
    void isEnabledChanged(bool isEnabled);

    void propertyModified(int propertyId, QVariant newValue);

private:
    int m_propertyId = -1;

    QVariant m_defaultValue = 0;
    QVariant m_currentValue = 0;
    bool m_isEnabled = false;
};

#endif // INSPECTORPROPERTY_H
