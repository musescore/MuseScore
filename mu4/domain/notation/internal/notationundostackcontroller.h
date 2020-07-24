#ifndef NOTATIONCOMMANDER_H
#define NOTATIONCOMMANDER_H

#include "inotationundostack.h"

#include "modularity/ioc.h"
#include "igetscore.h"

namespace mu::domain::notation {
class NotationUndoStackController : public INotationUndoStack
{
public:
    NotationUndoStackController(IGetScore* getScore);

    void prepareChanges() override;
    void commitChanges() override;

private:
    IGetScore* m_getScore = nullptr;
};
}

#endif // NOTATIONCOMMANDER_H
