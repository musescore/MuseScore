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
#ifndef MU_PROJECT_TEMPLATESMODEL_H
#define MU_PROJECT_TEMPLATESMODEL_H

#include "modularity/ioc.h"
#include "internal/itemplatesrepository.h"

namespace mu::project {
class TemplatesModel : public QObject
{
    Q_OBJECT

    INJECT(ITemplatesRepository, repository)

    Q_PROPERTY(QStringList categoriesTitles READ categoriesTitles NOTIFY categoriesChanged)
    Q_PROPERTY(QStringList templatesTitles READ templatesTitles NOTIFY templatesChanged)

    Q_PROPERTY(int currentCategoryIndex READ currentCategoryIndex WRITE setCurrentCategoryIndex NOTIFY currentCategoryChanged)
    Q_PROPERTY(int currentTemplateIndex READ currentTemplateIndex WRITE setCurrentTemplateIndex NOTIFY currentTemplateChanged)

    Q_PROPERTY(QString currentTemplatePath READ currentTemplatePath NOTIFY currentTemplateChanged)

public:
    TemplatesModel(QObject* parent = nullptr);

    QStringList categoriesTitles() const;
    QStringList templatesTitles() const;

    int currentCategoryIndex() const;
    int currentTemplateIndex() const;

    QString currentTemplatePath() const;

    Q_INVOKABLE void load();

    Q_INVOKABLE void saveCurrentCategory();
    Q_INVOKABLE void setSearchText(const QString& text);

public slots:
    void setCurrentCategoryIndex(int index);
    void setCurrentTemplateIndex(int index);

signals:
    void categoriesChanged();
    void templatesChanged();
    void currentCategoryChanged();
    void currentTemplateChanged();

private:
    void loadAllCategories();

    void setVisibleTemplates(const Templates& templates);
    void setVisibleCategories(const QStringList& titles);

    void setCurrentTemplate(const Template& templ);
    void setCurrentCategory(const QString& category);

    void updateTemplatesByCurrentCategory();
    void updateTemplatesAndCategoriesBySearch();

    bool titleAccepted(const QString& title) const;

    bool isSearching() const;

    Templates m_allTemplates;
    Templates m_visibleTemplates;
    QStringList m_visibleCategoriesTitles;

    QString m_searchText;

    QString m_currentCategory;
    Template m_currentTemplate;

    bool m_saveCurrentCategory = false;
};
}

#endif // MU_PROJECT_TEMPLATESMODEL_H
