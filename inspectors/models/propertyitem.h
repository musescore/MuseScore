#ifndef INSPECTORPROPERTY_H
#define INSPECTORPROPERTY_H

#include <QObject>
#include <QVariant>

class PropertyItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(bool isUndefined READ isUndefined NOTIFY isUndefinedChanged)

public:
    explicit PropertyItem(const int propertyId, QObject* parent = nullptr);

    void fillValues(const QVariant& currentValue, const QVariant& defaultValue);
    void updateCurrentValue(const QVariant& currentValue);
    void resetToDefault();

    int propertyId() const;
    QVariant value() const;
    bool isUndefined() const;

public slots:
    void setValue(const QVariant& value);
    void setDefaultValue(const QVariant& defaultValue);

signals:
    void valueChanged(QVariant value);
    void isUndefinedChanged(bool isUndefined);

    void propertyModified(int propertyId, QVariant newValue);

private:
    int m_propertyId = -1;

    QVariant m_defaultValue = 0;
    QVariant m_currentValue = 0;
};

#endif // INSPECTORPROPERTY_H
