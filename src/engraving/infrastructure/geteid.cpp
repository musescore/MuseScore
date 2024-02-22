#include "geteid.h"

using namespace mu::engraving;

void GetEID::init(uint32_t val)
{
    m_lastID = val;
}

EID GetEID::newEID(ElementType type)
{
    return EID(type, ++m_lastID);
}
