#include "scoreappearancesettingsmodel.h"

#include <QPageSize>
#include <QSize>

#include "types/scoreappearancetypes.h"
#include "dataformatter.h"

using namespace mu::inspector;

ScoreAppearanceSettingsModel::ScoreAppearanceSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setSectionType(InspectorSectionType::SECTION_SCORE_APPEARANCE);
    setTitle(qtrc("inspector", "Score appearance"));
    createProperties();
}

void ScoreAppearanceSettingsModel::createProperties()
{
    setPageTypeListModel(new PageTypeListModel(this));
}

void ScoreAppearanceSettingsModel::requestElements()
{
    //!Note the model work only with the parent score, no need to request other child elements
}

bool ScoreAppearanceSettingsModel::hasAcceptableElements() const
{
    if (isNotationExisting()) {
        return true;
    }

    return false;
}

void ScoreAppearanceSettingsModel::loadProperties()
{
    QSizeF pageSize = QSizeF(styleValue(Ms::Sid::pageWidth).toDouble(), styleValue(Ms::Sid::pageHeight).toDouble());

    m_pageTypeListModel->setCurrentPageSizeId(static_cast<int>(QPageSize::id(pageSize, QPageSize::Inch, QPageSize::FuzzyOrientationMatch)));

    ScoreAppearanceTypes::OrientationType pageOrientationType = pageSize.width()
                                                                > pageSize.height() ? ScoreAppearanceTypes::OrientationType::
                                                                ORIENTATION_LANDSCAPE
                                                                : ScoreAppearanceTypes::OrientationType::ORIENTATION_PORTRAIT;

    setOrientationType(static_cast<int>(pageOrientationType));
    setStaffSpacing(DataFormatter::formatDouble(styleValue(Ms::Sid::spatium).toDouble() / Ms::DPMM, 3));
    setStaffDistance(styleValue(Ms::Sid::staffDistance).toDouble());
}

void ScoreAppearanceSettingsModel::resetProperties()
{
}

PageTypeListModel* ScoreAppearanceSettingsModel::pageTypeListModel() const
{
    return m_pageTypeListModel;
}

void ScoreAppearanceSettingsModel::showPageSettings()
{
    adapter()->showPageSettingsDialog();
}

void ScoreAppearanceSettingsModel::showStyleSettings()
{
    adapter()->showStyleSettingsDialog();
}

int ScoreAppearanceSettingsModel::orientationType() const
{
    return m_orientationType;
}

qreal ScoreAppearanceSettingsModel::staffSpacing() const
{
    return m_staffSpacing;
}

qreal ScoreAppearanceSettingsModel::staffDistance() const
{
    return m_staffDistance;
}

void ScoreAppearanceSettingsModel::setPageTypeListModel(PageTypeListModel* pageTypeListModel)
{
    m_pageTypeListModel = pageTypeListModel;

    connect(m_pageTypeListModel, &PageTypeListModel::currentPageSizeIdChanged, this, [this](const int newCurrentPageSizeId) {
        QSizeF pageSize = QPageSize::size(QPageSize::PageSizeId(newCurrentPageSizeId), QPageSize::Inch);

        updateStyleValue(Ms::Sid::pageWidth, pageSize.width());
        updateStyleValue(Ms::Sid::pageHeight, pageSize.height());
    });
}

void ScoreAppearanceSettingsModel::setOrientationType(int orientationType)
{
    if (m_orientationType == orientationType) {
        return;
    }

    m_orientationType = orientationType;

    QSizeF pageSize(styleValue(Ms::Sid::pageWidth).toDouble(), styleValue(Ms::Sid::pageHeight).toDouble());

    updateStyleValue(Ms::Sid::pageWidth, pageSize.height());
    updateStyleValue(Ms::Sid::pageHeight, pageSize.width());
    emit orientationTypeChanged(m_orientationType);
}

void ScoreAppearanceSettingsModel::setStaffSpacing(qreal staffSpacing)
{
    if (qFuzzyCompare(m_staffSpacing, staffSpacing)) {
        return;
    }

    m_staffSpacing = staffSpacing;
    updateStyleValue(Ms::Sid::spatium, staffSpacing * Ms::DPMM);
    emit staffSpacingChanged(m_staffSpacing);
}

void ScoreAppearanceSettingsModel::setStaffDistance(qreal staffDistance)
{
    if (qFuzzyCompare(m_staffDistance, staffDistance)) {
        return;
    }

    m_staffDistance = staffDistance;
    updateStyleValue(Ms::Sid::staffDistance, staffDistance);
    emit staffDistanceChanged(m_staffDistance);
}
