//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_APPSHELL_NOTATIONPAGEMODEL_H
#define MU_APPSHELL_NOTATIONPAGEMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "actions/actionable.h"
#include "actions/iactionsdispatcher.h"
#include "inotationpagestate.h"

namespace mu::appshell {
class NotationPageModel : public QObject, public async::Asyncable, public actions::Actionable
{
    Q_OBJECT

    INJECT(appshell, INotationPageState, pageState)
    INJECT(appshell, actions::IActionsDispatcher, dispatcher)

    Q_PROPERTY(bool isPalettePanelVisible READ isPalettePanelVisible WRITE setIsPalettePanelVisible NOTIFY isPalettePanelVisibleChanged)
    Q_PROPERTY(
        bool isInstrumentsPanelVisible READ isInstrumentsPanelVisible WRITE setIsInstrumentsPanelVisible NOTIFY isInstrumentsPanelVisibleChanged)
    Q_PROPERTY(
        bool isInspectorPanelVisible READ isInspectorPanelVisible WRITE setIsInspectorPanelVisible NOTIFY isInspectorPanelVisibleChanged)
    Q_PROPERTY(bool isStatusBarVisible READ isStatusBarVisible WRITE setIsStatusBarVisible NOTIFY isStatusBarVisibleChanged)
    Q_PROPERTY(bool isNoteInputBarVisible READ isNoteInputBarVisible WRITE setIsNoteInputBarVisible NOTIFY isNoteInputBarVisibleChanged)
    Q_PROPERTY(
        bool isNotationToolBarVisible READ isNotationToolBarVisible WRITE setIsNotationToolBarVisible NOTIFY isNotationToolBarVisibleChanged)
    Q_PROPERTY(
        bool isPlaybackToolBarVisible READ isPlaybackToolBarVisible WRITE setIsPlaybackToolBarVisible NOTIFY isPlaybackToolBarVisibleChanged)
    Q_PROPERTY(
        bool isUndoRedoToolBarVisible READ isUndoRedoToolBarVisible WRITE setIsUndoRedoToolBarVisible NOTIFY isUndoRedoToolBarVisibleChanged)
    Q_PROPERTY(
        bool isNotationNavigatorVisible READ isNotationNavigatorVisible WRITE setIsNotationNavigatorVisible NOTIFY isNotationNavigatorVisibleChanged)

public:
    explicit NotationPageModel(QObject* parent = nullptr);

    Q_INVOKABLE void init();
    Q_INVOKABLE void setPanelsState(const QVariantList& states);

    bool isPalettePanelVisible() const;
    bool isInstrumentsPanelVisible() const;
    bool isInspectorPanelVisible() const;
    bool isStatusBarVisible() const;
    bool isNoteInputBarVisible() const;
    bool isNotationToolBarVisible() const;
    bool isPlaybackToolBarVisible() const;
    bool isUndoRedoToolBarVisible() const;
    bool isNotationNavigatorVisible() const;

public slots:
    void setIsPalettePanelVisible(bool visible);
    void setIsInstrumentsPanelVisible(bool visible);
    void setIsInspectorPanelVisible(bool visible);
    void setIsStatusBarVisible(bool visible);
    void setIsNoteInputBarVisible(bool visible);
    void setIsNotationToolBarVisible(bool visible);
    void setIsPlaybackToolBarVisible(bool visible);
    void setIsUndoRedoToolBarVisible(bool visible);
    void setIsNotationNavigatorVisible(bool visible);

signals:
    void isPalettePanelVisibleChanged();
    void isInstrumentsPanelVisibleChanged();
    void isInspectorPanelVisibleChanged();
    void isStatusBarVisibleChanged();
    void isNotationToolBarVisibleChanged();
    void isNoteInputBarVisibleChanged();
    void isPlaybackToolBarVisibleChanged();
    void isUndoRedoToolBarVisibleChanged();
    void isNotationNavigatorVisibleChanged();

private:
    void notifyAboutPanelChanged(PanelType type);

    void togglePanel(PanelType type);

    PanelType panelTypeFromString(const QString& string) const;
};
}

#endif // MU_APPSHELL_NOTATIONPAGEMODEL_H
