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
#ifndef MU_NOTATION_NOTATIONPAGEMODEL_H
#define MU_NOTATION_NOTATIONPAGEMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "inotationconfiguration.h"

namespace mu::notation {
class NotationPageModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(notation, INotationConfiguration, configuration)

    Q_PROPERTY(bool isPalettePanelVisible READ isPalettePanelVisible WRITE setIsPalettePanelVisible NOTIFY isPalettePanelVisibleChanged)
    Q_PROPERTY(
        bool isInstrumentsPanelVisible READ isInstrumentsPanelVisible WRITE setIsInstrumentsPanelVisible NOTIFY isInstrumentsPanelVisibleChanged)
    Q_PROPERTY(
        bool isInspectorPanelVisible READ isInspectorPanelVisible WRITE setIsInspectorPanelVisible NOTIFY isInspectorPanelVisibleChanged)
    Q_PROPERTY(bool isStatusBarVisible READ isStatusBarVisible WRITE setIsStatusBarVisible NOTIFY isStatusBarVisibleChanged)

public:
    explicit NotationPageModel(QObject* parent = nullptr);

    Q_INVOKABLE void init();

    bool isPalettePanelVisible() const;
    bool isInstrumentsPanelVisible() const;
    bool isInspectorPanelVisible() const;
    bool isStatusBarVisible() const;

public slots:
    void setIsPalettePanelVisible(bool visible);
    void setIsInstrumentsPanelVisible(bool visible);
    void setIsInspectorPanelVisible(bool visible);
    void setIsStatusBarVisible(bool visible);

signals:
    void isPalettePanelVisibleChanged(bool isPalettePanelVisible);
    void isInstrumentsPanelVisibleChanged(bool isInstrumentsPanelVisible);
    void isInspectorPanelVisibleChanged(bool isInspectorPanelVisible);
    void isStatusBarVisibleChanged(bool isStatusBarVisible);
};
}

#endif // MU_NOTATION_NOTATIONPAGEMODEL_H
