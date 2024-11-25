#include "eidregister.h"

using namespace mu::engraving;

void EIDRegister::init(uint32_t val)
{
    m_lastID = val;
}

EID EIDRegister::newEID(ElementType type)
{
    return EID(type, ++m_lastID);
}

void EIDRegister::registerItemEID(EID eid, EngravingObject* item)
{
    bool inserted = m_register.emplace(eid.toUint64(), item).second;
    assert(inserted);
}

EngravingObject* EIDRegister::itemFromEID(EID eid)
{
    auto iter = m_register.find(eid.toUint64());
    assert(iter != m_register.end());
    return (*iter).second;
}
