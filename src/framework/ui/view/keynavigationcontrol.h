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
#ifndef MU_UI_KEYNAVIGATIONCONTROL_H
#define MU_UI_KEYNAVIGATIONCONTROL_H

#include <QObject>

#include "abstractkeynavigation.h"
#include "../ikeynavigation.h"
#include "keynavigationsubsection.h"
#include "async/asyncable.h"

namespace mu::ui {
class KeyNavigationControl : public AbstractKeyNavigation, public IKeyNavigationControl, public async::Asyncable
{
    Q_OBJECT
    Q_PROPERTY(KeyNavigationSubSection * subsection READ subsection WRITE setSubSection NOTIFY subsectionChanged)

public:
    explicit KeyNavigationControl(QObject* parent = nullptr);
    ~KeyNavigationControl() override;

    QString name() const override;
    int order() const override;
    bool enabled() const override;
    bool active() const override;
    void setActive(bool arg) override;
    async::Channel<bool> activeChanged() const override;
    void trigger() override;
    async::Channel<IKeyNavigationControl*> forceActiveRequested() const override;

    KeyNavigationSubSection* subsection() const;

    Q_INVOKABLE void forceActive();

public slots:
    void setSubSection(KeyNavigationSubSection* subsection);

signals:
    void subsectionChanged(KeyNavigationSubSection* subsection);
    void triggered();

private slots:
    void onSubSectionDestroyed();

private:

    void componentComplete() override;

    KeyNavigationSubSection* m_subsection = nullptr;
    async::Channel<IKeyNavigationControl*> m_forceActiveRequested;
};
}

#endif // MU_UI_KEYNAVIGATIONCONTROL_H
