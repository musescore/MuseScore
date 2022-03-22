/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "templatesmodel.h"

#include "log.h"

using namespace mu::project;
using namespace mu::notation;

namespace mu::project {
inline bool operator==(const Template& templ1, const Template& templ2)
{
    return templ1.meta.filePath == templ2.meta.filePath;
}
}

TemplatesModel::TemplatesModel(QObject* parent)
    : QObject(parent)
{
}

void TemplatesModel::load()
{
    TRACEFUNC;

    RetVal<Templates> templates = repository()->templates();
    if (!templates.ret) {
        LOGE() << templates.ret.toString();
    }

    for (const Template& templ : templates.val) {
        if (!templ.meta.title.isEmpty()) {
            m_allTemplates << templ;
        }
    }

    loadAllCategories();
}

void TemplatesModel::loadAllCategories()
{
    TRACEFUNC;

    QStringList visibleCategories;

    for (const Template& templ: m_allTemplates) {
        if (!visibleCategories.contains(templ.categoryTitle)) {
            visibleCategories << templ.categoryTitle;
        }
    }

    setVisibleCategories(visibleCategories);
    updateTemplatesByCurrentCategory();
}

QStringList TemplatesModel::categoriesTitles() const
{
    return m_visibleCategoriesTitles;
}

int TemplatesModel::currentCategoryIndex() const
{
    return m_currentCategoryIndex;
}

int TemplatesModel::currentTemplateIndex() const
{
    return m_currentTemplateIndex;
}

QString TemplatesModel::currentTemplatePath() const
{
    return currentTemplate().meta.filePath.toQString();
}

QStringList TemplatesModel::templatesTitles() const
{
    QStringList titles;

    for (const Template& templ: m_visibleTemplates) {
        titles << templ.meta.title;
    }

    return titles;
}

void TemplatesModel::setCurrentCategoryIndex(int index)
{
    doSetCurrentCategoryIndex(index);
    updateTemplatesByCurrentCategory();
}

void TemplatesModel::setVisibleTemplates(const Templates& templates)
{
    if (m_visibleTemplates == templates) {
        return;
    }

    m_visibleTemplates = templates;
    emit templatesChanged();

    doSetCurrentTemplateIndex(0);
}

void TemplatesModel::doSetCurrentTemplateIndex(int index)
{
    if (m_currentTemplateIndex == index) {
        return;
    }

    m_currentTemplateIndex = index;
    emit currentTemplateChanged();
}

void TemplatesModel::setVisibleCategories(const QStringList& titles)
{
    if (m_visibleCategoriesTitles == titles) {
        return;
    }

    QString currentCategory = m_visibleCategoriesTitles.value(m_currentCategoryIndex, QString());

    m_visibleCategoriesTitles = titles;
    emit categoriesChanged();

    int currentCategoryIndex = 0;

    if (m_saveCurrentCategory) {
        currentCategoryIndex = std::max(titles.indexOf(currentCategory), currentCategoryIndex);
        m_saveCurrentCategory = false;
    }

    doSetCurrentCategoryIndex(currentCategoryIndex);
}

void TemplatesModel::doSetCurrentCategoryIndex(int index)
{
    if (m_currentCategoryIndex == index) {
        return;
    }

    m_currentCategoryIndex = index;
    emit currentCategoryChanged();
}

void TemplatesModel::updateTemplatesByCurrentCategory()
{
    TRACEFUNC;

    QStringList titles = categoriesTitles();
    if (titles.isEmpty()) {
        return;
    }

    QString currentCategoryTitle = titles[m_currentCategoryIndex];
    Templates newVisibleTemplates;

    for (const Template& templ: m_allTemplates) {
        if (templ.categoryTitle == currentCategoryTitle) {
            newVisibleTemplates << templ;
        }
    }

    setVisibleTemplates(newVisibleTemplates);
}

void TemplatesModel::setCurrentTemplateIndex(int index)
{
    if (m_currentTemplateIndex == index) {
        return;
    }

    doSetCurrentTemplateIndex(index);

    if (isSearching()) {
        const QString& currentCategory = currentTemplate().categoryTitle;
        int newCategoryIndex = m_visibleCategoriesTitles.indexOf(currentCategory);
        doSetCurrentCategoryIndex(newCategoryIndex);
    }
}

void TemplatesModel::saveCurrentCategory()
{
    m_saveCurrentCategory = true;
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
    TRACEFUNC;

    if (!isSearching()) {
        loadAllCategories();
        return;
    }

    QStringList newVisibleCategories;
    Templates newVisibleTemplates;

    for (const Template& templ: m_allTemplates) {
        if (titleAccepted(templ.meta.title) || titleAccepted(templ.categoryTitle)) {
            newVisibleTemplates << templ;

            if (!newVisibleCategories.contains(templ.categoryTitle)) {
                newVisibleCategories << templ.categoryTitle;
            }
        }
    }

    setVisibleCategories(newVisibleCategories);
    setVisibleTemplates(newVisibleTemplates);
}

bool TemplatesModel::titleAccepted(const QString& title) const
{
    if (isSearching()) {
        return title.contains(m_searchText, Qt::CaseInsensitive);
    }

    return true;
}

bool TemplatesModel::isSearching() const
{
    return !m_searchText.isEmpty();
}

const Template& TemplatesModel::currentTemplate() const
{
    if (m_currentTemplateIndex < 0 || m_currentTemplateIndex >= m_visibleTemplates.size()) {
        static Template dummy;
        return dummy;
    }

    return m_visibleTemplates[m_currentTemplateIndex];
}
