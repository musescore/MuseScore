#include "sampleobject.h"

using namespace mu::framework;

SampleObject::SampleObject(QObject* parent)
    : QObject(parent)
{
}

SampleObject::State SampleObject::state() const
{
    return _state;
}

void SampleObject::next()
{
    if (_state == Third) {
        _state = First;
    } else {
        _state = static_cast<State>(static_cast<int>(_state) + 1);
    }
    emit stateChanged(_state);
}
