#include "mu3inspectoradapter.h"

#include <QAction>

#include "shortcut.h"
#include "libmscore/musescoreCore.h"
#include "libmscore/score.h"
#include "global/context/scorestateobserver.h"
#include "log.h"

bool MU3InspectorAdapter::isNotationExisting() const
{
    return score() != nullptr;
}

MU3InspectorAdapter::MU3InspectorAdapter()
{
    QObject::connect(ScoreStateObserver::instance(), &ScoreStateObserver::currentStateChanged, [this](const Ms::ScoreState) {
        m_isTextEditingNotification.notify();
    });
}

bool MU3InspectorAdapter::isTextEditingStarted() const
{
    Ms::ScoreState state = ScoreStateObserver::instance()->currentState();

    return state == Ms::ScoreState::STATE_TEXT_EDIT
           || state == Ms::ScoreState::STATE_LYRICS_EDIT
           || state == Ms::ScoreState::STATE_HARMONY_FIGBASS_EDIT;
}

mu::async::Notification MU3InspectorAdapter::isTextEditingChanged() const
{
    return m_isTextEditingNotification;
}

void MU3InspectorAdapter::beginCommand()
{
    score()->startCmd();
}

void MU3InspectorAdapter::endCommand()
{
    score()->endCmd(true /*isCmdFromInspector*/);
}

void MU3InspectorAdapter::updateStyleValue(const Ms::Sid& styleId, const QVariant& newValue)
{
    score()->undoChangeStyleVal(styleId, newValue);
}

QVariant MU3InspectorAdapter::styleValue(const Ms::Sid& styleId)
{
    return score()->styleV(styleId);
}

void MU3InspectorAdapter::showSpecialCharactersDialog()
{
    QAction* action = Ms::Shortcut::getActionByName("show-keys");

    if (!action) {
        return;
    }

    action->trigger();
}

void MU3InspectorAdapter::showStaffTextPropertiesDialog()
{
    QAction* action = Ms::Shortcut::getActionByName("show-staff-text-properties");

    if (!action) {
        return;
    }

    action->trigger();
}

void MU3InspectorAdapter::showPageSettingsDialog()
{
    QAction* action = Ms::Shortcut::getActionByName("page-settings");

    if (!action) {
        return;
    }

    action->trigger();
}

void MU3InspectorAdapter::showStyleSettingsDialog()
{
    QAction* action = Ms::Shortcut::getActionByName("edit-style");

    if (!action) {
        return;
    }

    action->trigger();
}

void MU3InspectorAdapter::showTimeSignaturePropertiesDialog()
{
    QAction* action = Ms::Shortcut::getActionByName("show-time-signature-properties");

    if (!action) {
        return;
    }

    action->trigger();
}

void MU3InspectorAdapter::showArticulationPropertiesDialog()
{
    QAction* action = Ms::Shortcut::getActionByName("show-articulation-properties");

    if (!action) {
        return;
    }

    action->trigger();
}

void MU3InspectorAdapter::showGridConfigurationDialog()
{
    QAction* action = Ms::Shortcut::getActionByName("config-raster");

    if (!action) {
        return;
    }

    action->trigger();
}

void MU3InspectorAdapter::updatePageMarginsVisibility(const bool isVisible)
{
    QAction* action = Ms::Shortcut::getActionByName("show-pageborders");

    if (!action || action->isChecked() == isVisible) {
        return;
    }

    action->trigger();
}

void MU3InspectorAdapter::updateFramesVisibility(const bool isVisible)
{
    QAction* action = Ms::Shortcut::getActionByName("show-frames");

    if (!action || action->isChecked() == isVisible) {
        return;
    }

    action->trigger();
}

void MU3InspectorAdapter::updateHorizontalGridSnapping(const bool isSnapped)
{
    QAction* action = Ms::Shortcut::getActionByName("hraster");

    if (!action || action->isChecked() == isSnapped) {
        return;
    }

    action->trigger();
}

void MU3InspectorAdapter::updateVerticalGridSnapping(const bool isSnapped)
{
    QAction* action = Ms::Shortcut::getActionByName("vraster");

    if (!action || action->isChecked() == isSnapped) {
        return;
    }

    action->trigger();
}

void MU3InspectorAdapter::updateUnprintableElementsVisibility(const bool isVisible)
{
    QAction* action = Ms::Shortcut::getActionByName("show-unprintable");

    if (!action || action->isChecked() == isVisible) {
        return;
    }

    action->trigger();
}

void MU3InspectorAdapter::updateInvisibleElementsDisplaying(const bool isVisible)
{
    QAction* action = Ms::Shortcut::getActionByName("show-invisible");

    if (!action || action->isChecked() == isVisible) {
        return;
    }

    action->trigger();
}

void MU3InspectorAdapter::updateNotation()
{
    return score()->doLayout();
}

Ms::Score* MU3InspectorAdapter::score() const
{
    IF_ASSERT_FAILED(Ms::MuseScoreCore::mscoreCore) {
        return nullptr;
    }

    return Ms::MuseScoreCore::mscoreCore->currentScore();
}
