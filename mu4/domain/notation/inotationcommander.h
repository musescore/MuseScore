#ifndef INOTATIONCOMMANDER_H
#define INOTATIONCOMMANDER_H

#include "modularity/imoduleexport.h"

namespace mu::domain::notation {
class INotationCommander
{
public:
    ~INotationCommander() = default;

    virtual void beginCommand() = 0;
    virtual void endCommand() = 0;
};
}

#endif // INOTATIONCOMMANDER_H
