/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
#pragma once

#include <QObject>
#include <QVariantList>
#include <qqmlintegration.h>

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "rcommand/icommanddispatcher.h"

#include "project/iprojectsession.h"

namespace mu::engraving {
class MasterScore;
class MeasureBase;
class PracticeEmphasisData;
}

namespace mu::playback {
class PracticeEmphasisPanelModel : public QObject, public muse::Contextable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(bool hasScore READ hasScore NOTIFY stateChanged)
    Q_PROPERTY(bool playbackActive READ playbackActive WRITE setPlaybackActive NOTIFY stateChanged)
    Q_PROPERTY(bool emphasisMode READ emphasisMode WRITE setEmphasisMode NOTIFY stateChanged)
    Q_PROPERTY(int emphasizedDb READ emphasizedDb WRITE setEmphasizedDb NOTIFY stateChanged)
    Q_PROPERTY(int otherDb READ otherDb WRITE setOtherDb NOTIFY stateChanged)
    Q_PROPERTY(int fromMeasure READ fromMeasure WRITE setFromMeasure NOTIFY stateChanged)
    Q_PROPERTY(int toMeasure READ toMeasure WRITE setToMeasure NOTIFY stateChanged)
    Q_PROPERTY(QVariantList parts READ parts NOTIFY stateChanged)
    Q_PROPERTY(QVariantList editingPartOptions READ editingPartOptions NOTIFY stateChanged)
    Q_PROPERTY(QString editingPartId READ editingPartId WRITE setEditingPartId NOTIFY stateChanged)
    Q_PROPERTY(QString editingPartName READ editingPartName NOTIFY stateChanged)
    Q_PROPERTY(bool editingPartBuiltin READ editingPartBuiltin NOTIFY stateChanged)
    Q_PROPERTY(QString editingPartParentId READ editingPartParentId NOTIFY stateChanged)
    Q_PROPERTY(int editingPartIndex READ editingPartIndex NOTIFY stateChanged)
    Q_PROPERTY(int editingPartParentIndex READ editingPartParentIndex NOTIFY stateChanged)
    Q_PROPERTY(bool solo READ solo WRITE setSolo NOTIFY stateChanged)
    Q_PROPERTY(QVariantList parentOptions READ parentOptions NOTIFY stateChanged)
    Q_PROPERTY(bool canDeleteEditingPart READ canDeleteEditingPart NOTIFY stateChanged)
    Q_PROPERTY(QString selectionButtonText READ selectionButtonText NOTIFY stateChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY stateChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY stateChanged)
    Q_PROPERTY(QVariantList copySourceOptions READ copySourceOptions NOTIFY stateChanged)
    Q_PROPERTY(QString copySourceId READ copySourceId WRITE setCopySourceId NOTIFY stateChanged)
    Q_PROPERTY(int copySourceIndex READ copySourceIndex NOTIFY stateChanged)
    Q_PROPERTY(bool canCopyFromPart READ canCopyFromPart NOTIFY stateChanged)

    QML_ELEMENT

    muse::ContextInject<project::IProjectSession> projectSession = { this };
    muse::ContextInject<muse::rcommand::ICommandDispatcher> commandDispatcher = { this };

public:
    explicit PracticeEmphasisPanelModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE void setPartSelected(const QString& partId, bool selected);
    Q_INVOKABLE void useSelectionRange();
    Q_INVOKABLE void clearMeasureRange();
    Q_INVOKABLE void addCustomPart();
    Q_INVOKABLE void deleteEditingPart();
    Q_INVOKABLE void resetEditingPartEdits();
    Q_INVOKABLE void applyEditingPartName(const QString& partId, const QString& name);
    Q_INVOKABLE void applyEditingPartParentId(const QString& partId, const QString& parentId);
    Q_INVOKABLE void togglePanel();
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    Q_INVOKABLE void copyFromPart();
    Q_INVOKABLE QString parentValueAt(int index) const;
    Q_INVOKABLE QString editingValueAt(int index) const;

    bool hasScore() const;
    bool playbackActive() const;
    bool emphasisMode() const;
    int emphasizedDb() const;
    int otherDb() const;
    int fromMeasure() const;
    int toMeasure() const;
    QVariantList parts() const;
    QVariantList editingPartOptions() const;
    QString editingPartId() const;
    QString editingPartName() const;
    bool editingPartBuiltin() const;
    QString editingPartParentId() const;
    int editingPartIndex() const;
    int editingPartParentIndex() const;
    bool solo() const;
    QVariantList parentOptions() const;
    bool canDeleteEditingPart() const;
    QString selectionButtonText() const;
    bool canUndo() const;
    bool canRedo() const;
    QVariantList copySourceOptions() const;
    QString copySourceId() const;
    int copySourceIndex() const;
    bool canCopyFromPart() const;

    void setPlaybackActive(bool active);
    void setEmphasisMode(bool enabled);
    void setEmphasizedDb(int db);
    void setOtherDb(int db);
    void setFromMeasure(int measure);
    void setToMeasure(int measure);
    void setEditingPartId(const QString& partId);
    void setSolo(bool enabled);
    void setCopySourceId(const QString& sourceId);

    //! Unit tests attach a MasterScore without a QML/IoC stack. Unused in the app.
    void attachScoreForTesting(engraving::MasterScore* score);
    void setSelectionMeasuresForTesting(engraving::MeasureBase* from, engraving::MeasureBase* to);

signals:
    void stateChanged();
    void editingPartChanged();

private:
    engraving::MasterScore* currentMaster() const;
    engraving::PracticeEmphasisData* data() const;
    void connectScore();
    void refresh();

    QString resolvedCopySourceId() const;
    int indexOfOptionValue(const QVariantList& options, const QString& value) const;

    bool m_loaded = false;
    QString m_copySourceId;
    engraving::MasterScore* m_testScore = nullptr;
    engraving::MeasureBase* m_testSelectionFrom = nullptr;
    engraving::MeasureBase* m_testSelectionTo = nullptr;
};
}
