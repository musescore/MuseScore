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
#ifndef MU_UI_KEYNAVIGATIONSUBSECTION_H
#define MU_UI_KEYNAVIGATIONSUBSECTION_H

#include <QObject>
#include <QList>

#include "abstractkeynavigation.h"
#include "../ikeynavigation.h"
#include "keynavigationsection.h"
#include "async/asyncable.h"

namespace mu::ui {
class KeyNavigationControl;
class KeyNavigationSubSection : public AbstractKeyNavigation, public IKeyNavigationSubSection, public async::Asyncable
{
    Q_OBJECT
    Q_PROPERTY(KeyNavigationSection * section READ section WRITE setSection NOTIFY sectionChanged)
    Q_PROPERTY(QmlDirection direction READ direction_property WRITE setDirection NOTIFY directionChanged)

public:
    explicit KeyNavigationSubSection(QObject* parent = nullptr);
    ~KeyNavigationSubSection() override;

    //! NOTE Please sync with IKeyNavigationSubSection::Direction
    enum QmlDirection {
        Horizontal = 0,
        Vertical,
        Both
    };
    Q_ENUM(QmlDirection)

    QString name() const override;

    const Index& index() const override;
    async::Channel<Index> indexChanged() const override;

    bool enabled() const override;
    async::Channel<bool> enabledChanged() const override;

    bool active() const override;
    void setActive(bool arg) override;
    async::Channel<bool> activeChanged() const override;

    QmlDirection direction_property() const;
    Direction direction() const override;

    const std::set<IKeyNavigationControl*>& controls() const override;
    async::Notification controlsListChanged() const override;

    async::Channel<SubSectionControl> forceActiveRequested() const override;

    KeyNavigationSection* section() const;

    void componentComplete() override;

    void addControl(KeyNavigationControl* control);
    void removeControl(KeyNavigationControl* control);

public slots:
    void setSection(KeyNavigationSection* section);
    void setDirection(QmlDirection direction);

signals:
    void sectionChanged(KeyNavigationSection* section);
    void directionChanged(QmlDirection direction);

private slots:
    void onSectionDestroyed();

private:
    KeyNavigationSection* m_section = nullptr;
    std::set<IKeyNavigationControl*> m_controls;
    async::Notification m_controlsListChanged;
    async::Channel<SubSectionControl> m_forceActiveRequested;
    QmlDirection m_direction = QmlDirection::Horizontal;
};
}

#endif // MU_UI_KEYNAVIGATIONSUBSECTION_H
