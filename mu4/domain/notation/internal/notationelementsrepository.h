#ifndef NOTATIONELEMENTS_H
#define NOTATIONELEMENTS_H

#include "inotationelementsrepository.h"
#include "igetscore.h"

namespace mu {
namespace domain {
namespace notation {
class NotationElementsRepository : public INotationElementsRepository
{
public:
    NotationElementsRepository(IGetScore* getScore);

    Ms::Measure* measureByIndex(const int measureIndex) const override;

private:
    IGetScore* m_getScore = nullptr;
};
}
}
}

#endif // NOTATIONELEMENTS_H
