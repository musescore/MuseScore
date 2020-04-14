#ifndef ABSTRACTINSPECTORMODEL_H
#define ABSTRACTINSPECTORMODEL_H

#include <QList>
#include <functional>
#include "libmscore/element.h"
#include "libmscore/score.h"
#include "property.h"

#include "interfaces/ielementrepositoryservice.h"
#include "models/propertyitem.h"

class AbstractInspectorModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(InspectorModelType type READ type CONSTANT)
    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY isEmptyChanged)

    Q_ENUMS(InspectorModelType)
public:
    enum InspectorModelType {
        UNDEFINED = -1,
        GENERAL,
        NOTATION
    };

    explicit AbstractInspectorModel(QObject* parent, IElementRepositoryService* repository = nullptr);

    Q_INVOKABLE virtual void requestResetToDefaults();

    QString title() const;
    InspectorModelType type() const;
    bool isEmpty() const;

    static InspectorModelType modelTypeFromElementType(const Ms::ElementType elementType);

    virtual bool hasAcceptableElements() const;

    virtual void createProperties() = 0;
    virtual void requestElements() = 0;
    virtual void loadProperties() = 0;
    virtual void resetProperties() = 0;

public slots:
    void setTitle(QString title);
    void setType(InspectorModelType modelType);
    void setIsEmpty(bool isEmpty);

signals:
    void elementsModified();
    void modelReseted();
    void isEmptyChanged(bool isEmpty);

    void requestReloadPropertyItems();

protected:
    PropertyItem* buildPropertyItem(const Ms::Pid& pid,
                                    std::function<void(const int propertyId, const QVariant& newValue)> onPropertyChangedCallBack = nullptr);

    void loadPropertyItem(PropertyItem* propertyItem,
                          std::function<QVariant(const QVariant&)> convertElementPropertyValueFunc = nullptr);

    Ms::Score* parentScore() const;

    QVariant valueToElementUnits(const Ms::Pid& pid, const QVariant& value, const Ms::Element* element) const;
    QVariant valueFromElementUnits(const Ms::Pid& pid, const QVariant& value, const Ms::Element* element) const;

    IElementRepositoryService* m_repository;

    QList<Ms::Element*> m_elementList;

protected slots:
    void onResetToDefaults(const QList<Ms::Pid>& pidList);
    void onPropertyValueChanged(const Ms::Pid pid, const QVariant& newValue);
    void updateProperties();

private:
    QString m_title;
    InspectorModelType m_type;
    bool m_isEmpty = false;
};

#endif // ABSTRACTINSPECTORMODEL_H
