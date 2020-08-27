#include "scoredisplaysettingsmodel.h"

ScoreSettingsModel::ScoreSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setSectionType(SECTION_SCORE_DISPLAY);
    setTitle(tr("Show"));
    createProperties();
}

void ScoreSettingsModel::createProperties()
{
}

void ScoreSettingsModel::requestElements()
{
    //!Note the model work only with the parent score, no need to request other child elements
}

bool ScoreSettingsModel::hasAcceptableElements() const
{
    if (isNotationExisting()) {
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
    setShouldShowInvisible(false);
    setShouldShowFrames(false);
    setShouldShowPageMargins(false);
    setShouldShowUnprintable(false);
}

bool ScoreSettingsModel::shouldShowInvisible() const
{
    return m_shouldShowInvisible;
}

bool ScoreSettingsModel::shouldShowUnprintable() const
{
    return m_shouldShowUnprintable;
}

bool ScoreSettingsModel::shouldShowFrames() const
{
    return m_shouldShowFrames;
}

bool ScoreSettingsModel::shouldShowPageMargins() const
{
    return m_shouldShowPageMargins;
}

void ScoreSettingsModel::setShouldShowInvisible(bool shouldShowInvisible)
{
    if (m_shouldShowInvisible == shouldShowInvisible) {
        return;
    }

    adapter()->updateInvisibleElementsDisplaying(shouldShowInvisible);

    m_shouldShowInvisible = shouldShowInvisible;
    emit shouldShowInvisibleChanged(shouldShowInvisible);
}

void ScoreSettingsModel::setShouldShowUnprintable(bool shouldShowUnprintable)
{
    if (m_shouldShowUnprintable == shouldShowUnprintable) {
        return;
    }

    adapter()->updateUnprintableElementsVisibility(shouldShowUnprintable);

    m_shouldShowUnprintable = shouldShowUnprintable;
    emit shouldShowUnprintableChanged(shouldShowUnprintable);
}

void ScoreSettingsModel::setShouldShowFrames(bool shouldShowFrames)
{
    if (m_shouldShowFrames == shouldShowFrames) {
        return;
    }

    adapter()->updateFramesVisibility(shouldShowFrames);

    m_shouldShowFrames = shouldShowFrames;
    emit shouldShowFramesChanged(shouldShowFrames);
}

void ScoreSettingsModel::setShouldShowPageMargins(bool shouldShowPageMargins)
{
    if (m_shouldShowPageMargins == shouldShowPageMargins) {
        return;
    }

    adapter()->updatePageMarginsVisibility(shouldShowPageMargins);

    m_shouldShowPageMargins = shouldShowPageMargins;
    emit shouldShowPageMarginsChanged(shouldShowPageMargins);
}
