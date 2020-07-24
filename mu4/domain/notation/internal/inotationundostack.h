#ifndef INOTATIONCOMMANDER_H
#define INOTATIONCOMMANDER_H

#include "modularity/imoduleexport.h"

namespace mu::domain::notation {
class INotationUndoStack
{
public:
    ~INotationUndoStack() = default;

    virtual void prepareChanges() = 0;
    virtual void commitChanges() = 0;
};
}

#endif // INOTATIONCOMMANDER_H
