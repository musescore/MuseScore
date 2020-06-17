#include "scoredisplaysettingsmodel.h"

#include "shortcut.h"

ScoreSettingsModel::ScoreSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setSectionType(SECTION_SCORE_DISPLAY);
    setTitle(tr("Show"));
    createProperties();
}

void ScoreSettingsModel::createProperties()
{
    m_shouldShowInvisible = Ms::Shortcut::getActionByName("show-invisible");
    m_shouldShowUnprintable = Ms::Shortcut::getActionByName("show-unprintable");
    m_shouldShowFrames = Ms::Shortcut::getActionByName("show-frames");
    m_shouldShowPageMargins = Ms::Shortcut::getActionByName("show-pageborders");

    connect(m_shouldShowInvisible, &QAction::toggled, this, &ScoreSettingsModel::shouldShowInvisibleChanged);
    connect(m_shouldShowUnprintable, &QAction::toggled, this, &ScoreSettingsModel::shouldShowUnprintableChanged);
    connect(m_shouldShowFrames, &QAction::toggled, this, &ScoreSettingsModel::shouldShowFramesChanged);
    connect(m_shouldShowPageMargins, &QAction::toggled, this, &ScoreSettingsModel::shouldShowPageMarginsChanged);
}

void ScoreSettingsModel::requestElements()
{
    //!Note the model work only with the parent score, no need to request other child elements
}

bool ScoreSettingsModel::hasAcceptableElements() const
{
    if (parentScore()) {
        return true;
    }

    return false;
}

void ScoreSettingsModel::loadProperties()
{
    emit shouldShowInvisibleChanged(shouldShowInvisible());
    emit shouldShowUnprintableChanged(shouldShowUnprintable());
    emit shouldShowFramesChanged(shouldShowFrames());
    emit shouldShowPageMarginsChanged(shouldShowPageMargins());
}

void ScoreSettingsModel::resetProperties()
{
    m_shouldShowInvisible->setChecked(false);
    m_shouldShowUnprintable->setChecked(false);
    m_shouldShowFrames->setChecked(false);
    m_shouldShowFrames->setChecked(false);
}

bool ScoreSettingsModel::shouldShowInvisible() const
{
    return m_shouldShowInvisible->isChecked();
}

bool ScoreSettingsModel::shouldShowUnprintable() const
{
    return m_shouldShowUnprintable->isChecked();
}

bool ScoreSettingsModel::shouldShowFrames() const
{
    return m_shouldShowFrames->isChecked();
}

bool ScoreSettingsModel::shouldShowPageMargins() const
{
    return m_shouldShowPageMargins->isChecked();
}

void ScoreSettingsModel::setShouldShowInvisible(bool shouldShowInvisible)
{
    if (m_shouldShowInvisible->isChecked() == shouldShowInvisible)
        return;

    m_shouldShowInvisible->trigger();
}

void ScoreSettingsModel::setShouldShowUnprintable(bool shouldShowUnprintable)
{
    if (m_shouldShowUnprintable->isChecked() == shouldShowUnprintable)
        return;

    m_shouldShowUnprintable->trigger();
}

void ScoreSettingsModel::setShouldShowFrames(bool shouldShowFrames)
{
    if (m_shouldShowFrames->isChecked() == shouldShowFrames)
        return;

    m_shouldShowFrames->trigger();
}

void ScoreSettingsModel::setShouldShowPageMargins(bool shouldShowPageMargins)
{
    if (m_shouldShowPageMargins->isChecked() == shouldShowPageMargins)
        return;

    m_shouldShowPageMargins->trigger();
}
