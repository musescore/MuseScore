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

#ifndef MUSE_MPE_ARTICULATIONSPROFILEEDITORMODEL_H
#define MUSE_MPE_ARTICULATIONSPROFILEEDITORMODEL_H

#include <QObject>
#include <QList>

#include "modularity/ioc.h"
#include "global/iinteractive.h"
#include "io/path.h"

#include "iarticulationprofilesrepository.h"
#include "articulationpatternitem.h"

namespace muse::mpe {
class ArticulationsProfileEditorModel : public QObject, public Injectable
{
    Q_OBJECT

    Q_PROPERTY(QString currentPath READ currentPath WRITE setCurrentPath NOTIFY currentPathChanged)
    Q_PROPERTY(ArticulationPatternItem * selectedItem READ selectedItem WRITE setSelectedItem NOTIFY selectedItemChanged)

    Q_PROPERTY(bool isArrangementVisible READ isArrangementVisible WRITE setIsArrangementVisible NOTIFY isArrangementVisibleChanged)
    Q_PROPERTY(bool isPitchVisible READ isPitchVisible WRITE setIsPitchVisible NOTIFY isPitchVisibleChanged)
    Q_PROPERTY(bool isExpressionVisible READ isExpressionVisible WRITE setIsExpressionVisible NOTIFY isExpressionVisibleChanged)

    Q_PROPERTY(QList<ArticulationPatternItem*> singleNoteItems READ singleNoteItems CONSTANT)
    Q_PROPERTY(QList<ArticulationPatternItem*> multiNoteItems READ multiNoteItems CONSTANT)

    Inject<IInteractive> interactive = { this };
    Inject<IArticulationProfilesRepository> profilesRepository = { this };

public:
    enum RoleNames {
        PatternsScopeItem = Qt::UserRole + 1
    };

    explicit ArticulationsProfileEditorModel(QObject* parent = nullptr);

    Q_INVOKABLE void init();
    Q_INVOKABLE void requestToOpenProfile();
    Q_INVOKABLE bool requestToCreateProfile();
    Q_INVOKABLE void requestToSaveProfile();

    Q_INVOKABLE void copyPatternDataFromItem(ArticulationPatternItem* item);

    QString currentPath() const;
    void setCurrentPath(const QString& newCurrentPath);

    ArticulationPatternItem* selectedItem() const;
    void setSelectedItem(ArticulationPatternItem* newSelectedItem);

    QList<ArticulationPatternItem*> singleNoteItems() const;
    QList<ArticulationPatternItem*> multiNoteItems() const;

    bool isArrangementVisible() const;
    void setIsArrangementVisible(bool newIsArrangementVisible);

    bool isPitchVisible() const;
    void setIsPitchVisible(bool newIsPitchVisible);

    bool isExpressionVisible() const;
    void setIsExpressionVisible(bool newIsExpressionVisible);

signals:
    void currentPathChanged();
    void selectedItemChanged();

    void isArrangementVisibleChanged();
    void isPitchVisibleChanged();
    void isExpressionVisibleChanged();

private:
    void setProfile(const ArticulationsProfilePtr ptr);

    void loadItems();
    ArticulationPatternItem* buildItem(const ArticulationType type, const bool isSingleNoteType);

    io::path_t m_profilePath;

    ArticulationsProfilePtr m_profile = nullptr;

    QMap<ArticulationType, ArticulationPatternItem*> m_singleNoteItems;
    QMap<ArticulationType, ArticulationPatternItem*> m_multiNoteItems;

    ArticulationPatternItem* m_selectedItem = nullptr;

    bool m_isArrangementVisible = true;
    bool m_isPitchVisible = true;
    bool m_isExpressionVisible = true;
};
}

#endif // MUSE_MPE_ARTICULATIONSPROFILEEDITORMODEL_H
