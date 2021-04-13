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
#ifndef MU_UI_KEYNAVDEVSECTION_H
#define MU_UI_KEYNAVDEVSECTION_H

#include "abstractkeynavdevitem.h"

namespace mu::ui {
class KeyNavDevSection : public AbstractKeyNavDevItem
{
    Q_OBJECT
    Q_PROPERTY(QVariantList subsections READ subsections NOTIFY subsectionsChanged)

public:
    explicit KeyNavDevSection(IKeyNavigationSection* section);

    QVariantList subsections() const;

public slots:
    void setSubsections(const QVariantList& subsections);

signals:
    void subsectionsChanged();

private:
    IKeyNavigationSection* m_section = nullptr;
    QVariantList m_subsections;
};
}

#endif // MU_UI_KEYNAVDEVSECTION_H
