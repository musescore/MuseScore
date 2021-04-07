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
#ifndef MU_UI_KEYNAVIGATIONSECTION_H
#define MU_UI_KEYNAVIGATIONSECTION_H

#include <QObject>
#include <QList>

#include "abstractkeynavigation.h"
#include "../ikeynavigation.h"

#include "modularity/ioc.h"
#include "../ikeynavigationcontroller.h"
#include "async/asyncable.h"

namespace mu::ui {
class KeyNavigationSubSection;
class KeyNavigationSection : public AbstractKeyNavigation, public IKeyNavigationSection, public async::Asyncable
{
    Q_OBJECT
    INJECT(ui, IKeyNavigationController, keyNavigationController)

public:
    explicit KeyNavigationSection(QObject* parent = nullptr);
    ~KeyNavigationSection() override;

    QString name() const override;
    int order() const override;
    bool enabled() const override;
    bool active() const override;
    void setActive(bool arg) override;
    async::Channel<bool> activeChanged() const override;
    const QSet<IKeyNavigationSubSection*>& subsections() const override;
    async::Channel<SectionSubSectionControl> forceActiveRequested() const override;

    void componentComplete() override;

    void addSubSection(KeyNavigationSubSection* s);
    void removeSubSection(KeyNavigationSubSection* s);

private:

    QSet<IKeyNavigationSubSection*> m_subsections;
    async::Channel<SectionSubSectionControl> m_forceActiveRequested;
};
}

#endif // MU_UI_KEYNAVIGATIONSECTION_H
