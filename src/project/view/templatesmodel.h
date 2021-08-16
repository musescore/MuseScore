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
#ifndef MU_PROJECT_TEMPLATESMODEL_H
#define MU_PROJECT_TEMPLATESMODEL_H

#include "modularity/ioc.h"
#include "internal/itemplatesrepository.h"

namespace mu::project {
class TemplatesModel : public QObject
{
    Q_OBJECT

    INJECT(project, ITemplatesRepository, repository)

    Q_PROPERTY(QStringList categoriesTitles READ categoriesTitles NOTIFY categoriesChanged)
    Q_PROPERTY(QStringList templatesTitles READ templatesTitles NOTIFY templatesChanged)

    Q_PROPERTY(QString currentTemplatePath READ currentTemplatePath NOTIFY currentTemplateChanged)

public:
    TemplatesModel(QObject* parent = nullptr);

    QStringList categoriesTitles() const;
    QStringList templatesTitles() const;

    QString currentTemplatePath() const;

    Q_INVOKABLE void load();

    Q_INVOKABLE void setCurrentCategory(int index);
    Q_INVOKABLE void setCurrentTemplate(int index);
    Q_INVOKABLE void setSearchText(const QString& text);

signals:
    void categoriesChanged();
    void templatesChanged();
    void currentTemplateChanged();

private:
    void updateTemplatesByCategory();
    void updateTemplatesAndCategoriesBySearch();

    bool titleAccepted(const QString& title) const;

    Templates m_allTemplates;
    Templates m_visibleTemplates;
    QList<QString> m_visibleCategoriesTitles;

    QString m_searchText;

    int m_currentCategoryIndex = 0;
    int m_currentTemplateIndex = 0;
};
}

#endif // MU_PROJECT_TEMPLATESMODEL_H
