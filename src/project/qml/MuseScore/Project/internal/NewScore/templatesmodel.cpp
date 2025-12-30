/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
using namespace muse;

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
    return m_visibleCategoriesTitles.indexOf(m_currentCategory);
}

int TemplatesModel::currentTemplateIndex() const
{
    return m_visibleTemplates.indexOf(m_currentTemplate);
}

QString TemplatesModel::currentTemplatePath() const
{
    return m_currentTemplate.meta.filePath.toQString();
}

QStringList TemplatesModel::templatesTitles() const
{
    QStringList titles;

    for (const Template& templ: m_visibleTemplates) {
        QString titleToAdd = templ.isCustom ? templ.meta.fileName(false).toQString() : templ.meta.title;
        titles << titleToAdd;
    }

    return titles;
}

void TemplatesModel::setCurrentCategoryIndex(int index)
{
    setCurrentCategory(m_visibleCategoriesTitles.value(index));
    updateTemplatesByCurrentCategory();
}

void TemplatesModel::setVisibleTemplates(const Templates& templates)
{
    if (m_visibleTemplates == templates) {
        return;
    }

    m_visibleTemplates = templates;
    emit templatesChanged();

    setCurrentTemplate(templates.value(0));
}

void TemplatesModel::setCurrentTemplate(const Template& templ)
{
    if (m_currentTemplate == templ) {
        return;
    }

    m_currentTemplate = templ;
    emit currentTemplateChanged();
}

void TemplatesModel::setVisibleCategories(const QStringList& titles)
{
    if (m_visibleCategoriesTitles == titles) {
        return;
    }

    QString currentCategory = titles.value(0);
    if (m_saveCurrentCategory) {
        m_saveCurrentCategory = false;
        currentCategory = m_currentCategory;
    }

    m_visibleCategoriesTitles = titles;
    emit categoriesChanged();

    setCurrentCategory(currentCategory);
}

void TemplatesModel::setCurrentCategory(const QString& category)
{
    if (m_currentCategory == category) {
        return;
    }

    m_currentCategory = category;
    emit currentCategoryChanged();
}

void TemplatesModel::updateTemplatesByCurrentCategory()
{
    TRACEFUNC;

    QStringList titles = categoriesTitles();
    if (titles.isEmpty()) {
        return;
    }

    Templates newVisibleTemplates;

    for (const Template& templ: m_allTemplates) {
        if (templ.categoryTitle == m_currentCategory) {
            newVisibleTemplates << templ;
        }
    }

    setVisibleTemplates(newVisibleTemplates);
}

void TemplatesModel::setCurrentTemplateIndex(int index)
{
    setCurrentTemplate(m_visibleTemplates.value(index));

    if (isSearching()) {
        setCurrentCategory(m_currentTemplate.categoryTitle);
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

    setCurrentCategory(m_currentTemplate.categoryTitle);
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
