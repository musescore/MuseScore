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
#ifndef MU_UI_KEYNAVDEVSUBSECTION_H
#define MU_UI_KEYNAVDEVSUBSECTION_H

#include "abstractkeynavdevitem.h"

namespace mu::ui {
class KeyNavDevSubSection : public AbstractKeyNavDevItem
{
    Q_OBJECT
    Q_PROPERTY(QString direction READ direction CONSTANT)
    Q_PROPERTY(QVariantList controls READ controls NOTIFY controlsChanged)

public:
    explicit KeyNavDevSubSection(IKeyNavigationSubSection* subsection);

    QString direction() const;
    QVariantList controls() const;

public slots:
    void setControls(const QVariantList& controls);

signals:
    void controlsChanged();

private:
    IKeyNavigationSubSection* m_subsection = nullptr;
    QVariantList m_controls;
};
}

#endif // MU_UI_KEYNAVDEVSUBSECTION_H
