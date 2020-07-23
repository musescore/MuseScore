#ifndef SCORESTATEOBSERVER_H
#define SCORESTATEOBSERVER_H

#include <QObject>

#include "mscore/globals.h"

class ScoreStateObserver : public QObject
{
    Q_OBJECT

public:
    static ScoreStateObserver* instance()
    {
        static ScoreStateObserver obs;
        return &obs;
    }

    Ms::ScoreState currentState() const;

public slots:
    void setCurrentState(Ms::ScoreState currentState);

signals:
    void currentStateChanged(Ms::ScoreState currentState);

private:
    explicit ScoreStateObserver(QObject* parent = nullptr);

    Ms::ScoreState m_currentState = Ms::ScoreState::STATE_ALL;
};

Q_DECLARE_METATYPE(Ms::ScoreState)

#endif // SCORESTATEOBSERVER_H
