#include "scoremigrationdialogmodel.h"

#include <QDesktopServices>

#include "handlers/instrumentorderinghandler.h"
#include "handlers/staffverticaljustificationhandler.h"
#include "handlers/resetallelementspositionshandler.h"
#include "handlers/firstsystemindentationhandler.h"
#include "libmscore/cmd.cpp"

ScoreMigrationDialogModel::ScoreMigrationDialogModel(Ms::Score* score, QObject* parent)
    : QObject(parent), m_score(score), m_migrator(new ScoreMigrator_3_6())
{
}

ScoreMigrationDialogModel::~ScoreMigrationDialogModel()
{
    delete m_migrator;
}

void ScoreMigrationDialogModel::apply()
{
    setUpMigrationPolicy();

    m_migrator->migrateScore(m_score);

    emit closeRequested();
}

void ScoreMigrationDialogModel::ignore()
{
    emit closeRequested();
}

void ScoreMigrationDialogModel::showMoreDetails()
{
    QDesktopServices::openUrl(QUrl("https://musescore.com")); // Here will be the link for announcement
}

bool ScoreMigrationDialogModel::areStylingImprovementsAllowed() const
{
    return m_areStylingImprovementsAllowed;
}

bool ScoreMigrationDialogModel::areInstrumentsImprovementsAllowed() const
{
    return m_areInstrumentsImprovementsAllowed;
}

bool ScoreMigrationDialogModel::isApplyingAvailable() const
{
    return m_areInstrumentsImprovementsAllowed
            || m_areStylingImprovementsAllowed
            || shouldNeverAskForThisScoreAgain()
            || m_shouldNeverAskAgain;
}

bool ScoreMigrationDialogModel::shouldNeverAskForThisScoreAgain() const
{
    return m_score ? !m_score->isQuallityUpgradeAllowed() : false;
}

bool ScoreMigrationDialogModel::shouldNeverAskAgain() const
{
    return m_shouldNeverAskAgain;
}

void ScoreMigrationDialogModel::setAreStylingImprovementsAllowed(bool areStylingImprovementsAllowed)
{
    if (m_areStylingImprovementsAllowed == areStylingImprovementsAllowed)
        return;

    m_areStylingImprovementsAllowed = areStylingImprovementsAllowed;
    emit areStylingImprovementsAllowedChanged(m_areStylingImprovementsAllowed);
    emit isApplyingAvailableChanged(isApplyingAvailable());
}

void ScoreMigrationDialogModel::setAreInstrumentsImprovementsAllowed(bool areInstrumentsImprovementsAllowed)
{
    if (m_areInstrumentsImprovementsAllowed == areInstrumentsImprovementsAllowed)
        return;

    m_areInstrumentsImprovementsAllowed = areInstrumentsImprovementsAllowed;
    emit areInstrumentsImprovementsAllowedChanged(m_areInstrumentsImprovementsAllowed);
    emit isApplyingAvailableChanged(isApplyingAvailable());
}

void ScoreMigrationDialogModel::setShouldNeverAskForThisScoreAgain(bool _shouldNeverAskForThisScoreAgain)
{
    if (!m_score || shouldNeverAskForThisScoreAgain() == _shouldNeverAskForThisScoreAgain)
        return;

    m_score->setIsQuallityUpgradeAllowed(!_shouldNeverAskForThisScoreAgain);
    emit shouldNeverAskForThisScoreAgainChanged(_shouldNeverAskForThisScoreAgain);
    emit isApplyingAvailableChanged(isApplyingAvailable());
}

void ScoreMigrationDialogModel::setShouldNeverAskAgain(bool shouldNeverAskAgain)
{
    if (m_shouldNeverAskAgain == shouldNeverAskAgain)
        return;

    m_shouldNeverAskAgain = shouldNeverAskAgain;
    emit shouldNeverAskAgainChanged(m_shouldNeverAskAgain);
    emit isApplyingAvailableChanged(isApplyingAvailable());
}

void ScoreMigrationDialogModel::setUpMigrationPolicy()
{
    if (m_areStylingImprovementsAllowed) {
        m_migrator->registerHandler(new ResetAllElementsPositionsHandler());
        m_migrator->registerHandler(new StaffVerticalJustificationHandler());
        m_migrator->registerHandler(new FirstSystemIndentationHandler());
    }

    if (m_areInstrumentsImprovementsAllowed) {
        m_migrator->registerHandler(new InstrumentOrderingHandler());
    }
}
