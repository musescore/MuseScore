#include "newscoremodel.h"

#include "actions/actiontypes.h"

using namespace mu::scores;
using namespace mu::actions;

NewScoreModel::NewScoreModel(QObject *parent) : QObject(parent)
{

}

void NewScoreModel::create()
{
    QVariantMap scoreInfo;
    scoreInfo["title"] = m_title;
    scoreInfo["composer"] = m_composer;

    io::path templatePath = globalConfiguration()->sharePath() + "/templates/02-Choral/05-SATB_Closed_Score_+_Organ.mscx";
    scoreInfo["template"] = io::pathToQString(templatePath);

    dispatcher()->dispatch("file-newscore", ActionData::make_arg1<QVariantMap>(scoreInfo));

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
