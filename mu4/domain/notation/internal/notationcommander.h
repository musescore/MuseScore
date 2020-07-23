#ifndef NOTATIONCOMMANDER_H
#define NOTATIONCOMMANDER_H

#include "inotationcommander.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "igetscore.h"

namespace mu::domain::notation {
class NotationCommander : public INotationCommander
{
    INJECT(notation, mu::context::IGlobalContext, context)

public:
    NotationCommander(IGetScore* getScore);

    void beginCommand() override;
    void endCommand() override;

private:
    IGetScore* m_getScore = nullptr;
};
}

#endif // NOTATIONCOMMANDER_H
