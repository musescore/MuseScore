#include "scorestateobserver.h"

ScoreStateObserver::ScoreStateObserver(QObject* parent)
    : QObject(parent)
{
}

Ms::ScoreState ScoreStateObserver::currentState() const
{
    return m_currentState;
}

void ScoreStateObserver::setCurrentState(Ms::ScoreState currentState)
{
    if (m_currentState == currentState) {
        return;
    }

    m_currentState = currentState;
    emit currentStateChanged(m_currentState);
}
