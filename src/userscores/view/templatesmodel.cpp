#include "templatesmodel.h"

#include "log.h"

using namespace mu::userscores;
using namespace mu::notation;

TemplatesModel::TemplatesModel(QObject* parent)
    : QObject(parent)
{
}

void TemplatesModel::load()
{
    RetVal<Templates> templates = repository()->templates();
    if (!templates.ret) {
        LOGE() << templates.ret.toString();
    }

    for (const Template& templ : templates.val) {
        if (!templ.title.isEmpty()) {
            m_allTemplates << templ;
        }
    }

    for (const Template& templ: m_allTemplates) {
        if (!m_visibleCategoriesTitles.contains(templ.categoryTitle)) {
            m_visibleCategoriesTitles << templ.categoryTitle;
        }
    }

    updateTemplatesByCategory();
    emit categoriesChanged();
}

QStringList TemplatesModel::categoriesTitles() const
{
    return m_visibleCategoriesTitles;
}

QString TemplatesModel::currentTemplatePath() const
{
    if (m_visibleTemplates.isEmpty()) {
        return QString();
    }

    return m_visibleTemplates[m_currentTemplateIndex].filePath.toQString();
}

QStringList TemplatesModel::templatesTitles() const
{
    QStringList titles;

    for (const Template& templ: m_visibleTemplates) {
        titles << templ.title;
    }

    return titles;
}

void TemplatesModel::setCurrentCategory(int index)
{
    if (m_currentCategoryIndex == index) {
        return;
    }

    m_currentCategoryIndex = index;
    updateTemplatesByCategory();
}

void TemplatesModel::updateTemplatesByCategory()
{
    m_visibleTemplates.clear();
    m_currentTemplateIndex = 0;

    QString currentCategoryTitle = categoriesTitles()[m_currentCategoryIndex];

    for (const Template& templ: m_allTemplates) {
        if (templ.categoryTitle == currentCategoryTitle) {
            m_visibleTemplates << templ;
        }
    }

    emit templatesChanged();
    emit currentTemplateChanged();
}

void TemplatesModel::setCurrentTemplate(int index)
{
    if (m_currentTemplateIndex == index) {
        return;
    }

    m_currentTemplateIndex = index;
    emit currentTemplateChanged();
}

void TemplatesModel::setSearchText(const QString& text)
{
    if (m_searchText == text) {
        return;
    }

    m_searchText = text;
    updateTemplatesAndCategoriesBySearch();
}

void TemplatesModel::updateTemplatesAndCategoriesBySearch()
{
    m_visibleTemplates.clear();
    m_visibleCategoriesTitles.clear();

    m_currentCategoryIndex = 0;
    m_currentTemplateIndex = 0;

    for (const Template& templ: m_allTemplates) {
        if (titleAccepted(templ.title) || titleAccepted(templ.categoryTitle)) {
            m_visibleTemplates << templ;
            if (!m_visibleCategoriesTitles.contains(templ.categoryTitle)) {
                m_visibleCategoriesTitles << templ.categoryTitle;
            }
        }
    }

    emit categoriesChanged();
    emit templatesChanged();
    emit currentTemplateChanged();
}

bool TemplatesModel::titleAccepted(const QString& title) const
{
    if (m_searchText.isEmpty()) {
        return true;
    }

    return title.contains(m_searchText, Qt::CaseInsensitive);
}
