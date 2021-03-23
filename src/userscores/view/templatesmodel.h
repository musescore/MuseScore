//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_USERSCORES_TEMPLATESMODEL_H
#define MU_USERSCORES_TEMPLATESMODEL_H

#include "modularity/ioc.h"
#include "internal/itemplatesrepository.h"

namespace mu::userscores {
class TemplatesModel : public QObject
{
    Q_OBJECT

    INJECT(userscores, ITemplatesRepository, repository)

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

#endif // MU_USERSCORES_TEMPLATESMODEL_H
