#ifndef INOTATIONSTYLE_H
#define INOTATIONSTYLE_H

#include <QVariant>

#include "modularity/imoduleexport.h"
#include "libmscore/style.h"

using StyleId = Ms::Sid;

namespace mu {
namespace domain {
namespace notation {
class INotationStyle : MODULE_EXPORT_INTERFACE
{
public:
    ~INotationStyle() = default;

    virtual void updateStyleValue(const StyleId& styleId, const QVariant& newValue) = 0;
    virtual QVariant styleValue(const StyleId& styleId) const = 0;
};
}
}
}

#endif // INOTATIONSTYLE_H
