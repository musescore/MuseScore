#ifndef MU_INSPECTOR_IELEMENTREPOSITORYSERVICE_H
#define MU_INSPECTOR_IELEMENTREPOSITORYSERVICE_H

#include "libmscore/element.h"
#include <QList>
#include <QObject>
#include <functional>

namespace mu::inspector {
class IElementRepositoryService
{
public:
    virtual ~IElementRepositoryService() = default;

    virtual QObject* getQObject() = 0;

    virtual void updateElementList(const QList<Ms::Element*>& newRawElementList) = 0;
    virtual QList<Ms::Element*> findElementsByType(const Ms::ElementType elementType) const = 0;
    virtual QList<Ms::Element*> findElementsByType(const Ms::ElementType elementType,
                                                   std::function<bool(const Ms::Element*)> filterFunc) const = 0;
    virtual QList<Ms::Element*> takeAllElements() const = 0;

signals:
    virtual void elementsUpdated() = 0;
};
}

#endif // MU_INSPECTOR_IELEMENTREPOSITORYSERVICE_H
