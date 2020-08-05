#include "newscoremodel.h"

#include "actions/actiontypes.h"

#include "ret.h"
#include "log.h"

using namespace mu::userscores;
using namespace mu::actions;
using namespace mu::domain::notation;

NewScoreModel::NewScoreModel(QObject* parent)
    : QObject(parent)
{
}

bool NewScoreModel::create()
{
    ScoreCreateOptions scoreOptions;
    scoreOptions.title = m_title;
    scoreOptions.composer = m_composer;

    // TODO: Temporary solution
    scoreOptions.templatePath = io::pathToQString(
        globalConfiguration()->sharePath() + "/templates/02-Choral/05-SATB_Closed_Score_+_Organ.mscx");

    fillDefault(scoreOptions);

    auto notation = notationCreator()->newNotation();
    IF_ASSERT_FAILED(notation) {
        return false;
    }

    Ret ret = notation->createNew(scoreOptions);

    if (!ret) {
        LOGE() << "failed create new score ret:" << ret.toString();
        return false;
    }

    io::path filePath = notation->path();

    if (!globalContext()->containsMasterNotation(filePath)) {
        globalContext()->addMasterNotation(notation);
    }

    globalContext()->setCurrentMasterNotation(notation);

    interactive()->open("musescore://notation");

    return true;
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

void NewScoreModel::fillDefault(ScoreCreateOptions& scoreOptions)
{
    // TODO: Temporary solution
    scoreOptions.subtitle = "default subtitle";
    scoreOptions.lyricist = "default lyricist";
    scoreOptions.copyright = "default copyright";
    scoreOptions.tempo = 120;
    scoreOptions.timesigNumerator = 4;
    scoreOptions.timesigDenominator = 4;
    scoreOptions.measures = 32;
    scoreOptions.measureTimesigNumerator = 1;
    scoreOptions.measureTimesigDenominator = 4;
    scoreOptions.timesigType = Ms::TimeSigType::NORMAL;
    scoreOptions.key = Ms::Key::C;
}
