#include "sampleobject.h"

using namespace mu::uicomponents;

SampleObject::SampleObject(QObject* parent)
    : QObject(parent)
{
}

SampleObject::State SampleObject::state() const
{
    return m_state;
}

void SampleObject::next()
{
    if (m_state == Third) {
        m_state = First;
    } else {
        m_state = static_cast<State>(static_cast<int>(m_state) + 1);
    }
    emit stateChanged(m_state);
}
