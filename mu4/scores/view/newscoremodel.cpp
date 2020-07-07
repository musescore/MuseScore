#include "newscoremodel.h"

#include "actions/actiontypes.h"

using namespace mu::scores;
using namespace mu::actions;
using namespace mu::domain::notation;

NewScoreModel::NewScoreModel(QObject *parent) : QObject(parent)
{

}

void NewScoreModel::create()
{
    ScoreInfo score;
    score.title = m_title;
    score.composer = m_composer;
    score.templatePath = io::pathToQString(globalConfiguration()->sharePath() + "/templates/02-Choral/05-SATB_Closed_Score_+_Organ.mscx");

    fillDefault(score);

    dispatcher()->dispatch("file-newscore", ActionData::make_arg1<ScoreInfo>(score));

    emit close();
}

QString NewScoreModel::title() const
{
    return m_title;
}

QString NewScoreModel::composer() const
{
    return m_composer;
}

void NewScoreModel::setTitle(QString title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    emit titleChanged(m_title);
}

void NewScoreModel::setComposer(QString composer)
{
    if (m_composer == composer) {
        return;
    }

    m_composer = composer;
    emit composerChanged(m_composer);
}

void NewScoreModel::fillDefault(ScoreInfo& scoreInfo)
{
    scoreInfo.subtitle = "default subtitle";
    scoreInfo.poet = "default poet";
    scoreInfo.copyright = "default copyright";
    scoreInfo.tempo = 120;
    scoreInfo.timesigNumerator = 4;
    scoreInfo.timesigDenominator = 4;
    scoreInfo.measures = 32;
    scoreInfo.measureTimesigNumerator = 1;
    scoreInfo.measureTimesigDenominator = 4;
    scoreInfo.timesigType = Ms::TimeSigType::NORMAL;
    scoreInfo.key = Ms::Key::C;
}
