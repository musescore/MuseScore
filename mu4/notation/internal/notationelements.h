#ifndef NOTATIONELEMENTS_H
#define NOTATIONELEMENTS_H

#include "inotationelements.h"
#include "igetscore.h"

namespace mu {
namespace notation {
class NotationElements : public INotationElements
{
public:
    NotationElements(IGetScore* getScore);

    Ms::Measure* measureByIndex(const int measureIndex) const override;

private:
    IGetScore* m_getScore = nullptr;
};
}
}

#endif // NOTATIONELEMENTS_H
