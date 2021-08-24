#include "pianorollgeneral.h"

#include "internal/pianorollcontroller.h"

using namespace mu::pianoroll;

PianorollGeneral::PianorollGeneral(QObject* parent)
    : QObject(parent),
      m_xZoom(X_ZOOM_INITIAL),
      m_noteHeight(DEFAULT_KEY_HEIGHT)
{
}

void PianorollGeneral::load()
{
}


void PianorollGeneral::setXZoom(double value)
{
    if (value == m_xZoom)
        return;
    m_xZoom = value;
    emit xZoomChanged();
}

void PianorollGeneral::setNoteHeight(int value)
{
    if (value == m_noteHeight)
        return;
    m_noteHeight = value;
    emit noteHeightChanged();
}

