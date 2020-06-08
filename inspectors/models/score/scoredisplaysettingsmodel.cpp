#include "scoresettingsmodel.h"

#include "shortcut.h"

scoreSettingsModel::scoreSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setSectionType(SECTION_SCORE_DISPLAY);
    setTitle(tr("Show"));
    createProperties();
}

void scoreSettingsModel::createProperties()
{
    m_shouldShowInvisible = Ms::Shortcut::getActionByName("show-invisible");
    m_shouldShowUnprintable = Ms::Shortcut::getActionByName("show-unprintable");
    m_shouldShowFrames = Ms::Shortcut::getActionByName("show-frames");
    m_shouldShowPageMargins = Ms::Shortcut::getActionByName("show-pageborders");

    connect(m_shouldShowInvisible, &QAction::toggled, this, &scoreSettingsModel::shouldShowInvisibleChanged);
    connect(m_shouldShowUnprintable, &QAction::toggled, this, &scoreSettingsModel::shouldShowUnprintableChanged);
    connect(m_shouldShowFrames, &QAction::toggled, this, &scoreSettingsModel::shouldShowFramesChanged);
    connect(m_shouldShowPageMargins, &QAction::toggled, this, &scoreSettingsModel::shouldShowPageMarginsChanged);
}

void scoreSettingsModel::requestElements()
{
    //!Note the model work only with the parent score, no need to request other child elements
}

bool scoreSettingsModel::hasAcceptableElements() const
{
    if (parentScore()) {
        return true;
    }

    return false;
}

void scoreSettingsModel::loadProperties()
{
    emit shouldShowInvisibleChanged(shouldShowInvisible());
    emit shouldShowUnprintableChanged(shouldShowUnprintable());
    emit shouldShowFramesChanged(shouldShowFrames());
    emit shouldShowPageMarginsChanged(shouldShowPageMargins());
}

void scoreSettingsModel::resetProperties()
{
    m_shouldShowInvisible->setChecked(false);
    m_shouldShowUnprintable->setChecked(false);
    m_shouldShowFrames->setChecked(false);
    m_shouldShowFrames->setChecked(false);
}

bool scoreSettingsModel::shouldShowInvisible() const
{
    return m_shouldShowInvisible->isChecked();
}

bool scoreSettingsModel::shouldShowUnprintable() const
{
    return m_shouldShowUnprintable->isChecked();
}

bool scoreSettingsModel::shouldShowFrames() const
{
    return m_shouldShowFrames->isChecked();
}

bool scoreSettingsModel::shouldShowPageMargins() const
{
    return m_shouldShowPageMargins->isChecked();
}

void scoreSettingsModel::setShouldShowInvisible(bool shouldShowInvisible)
{
    if (m_shouldShowInvisible->isChecked() == shouldShowInvisible)
        return;

    m_shouldShowInvisible->trigger();
}

void scoreSettingsModel::setShouldShowUnprintable(bool shouldShowUnprintable)
{
    if (m_shouldShowUnprintable->isChecked() == shouldShowUnprintable)
        return;

    m_shouldShowUnprintable->trigger();
}

void scoreSettingsModel::setShouldShowFrames(bool shouldShowFrames)
{
    if (m_shouldShowFrames->isChecked() == shouldShowFrames)
        return;

    m_shouldShowFrames->trigger();
}

void scoreSettingsModel::setShouldShowPageMargins(bool shouldShowPageMargins)
{
    if (m_shouldShowPageMargins->isChecked() == shouldShowPageMargins)
        return;

    m_shouldShowPageMargins->trigger();
}
