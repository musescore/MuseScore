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
#include "keynavdevmodel.h"

#include <algorithm>

#include "keynavdevsection.h"
#include "keynavdevsubsection.h"
#include "keynavdevcontrol.h"

using namespace mu::ui;

template<class T>
static QList<T*> toQList(std::set<T*> set)
{
    QList<T*> l;
    l.reserve(set.size());
    for (T* s : set) {
        l.append(s);
    }
    return l;
}

template<class T>
static void sortByIndex(QList<T*>& list)
{
    std::sort(list.begin(), list.end(), [](T* f, T* s) {
        if (f->index().row != s->index().row) {
            return f->index().row < s->index().row;
        }
        return f->index().column < s->index().column;
    });
}

KeyNavDevModel::KeyNavDevModel(QObject* parent)
    : QObject(parent)
{
    connect(&m_reloadDelayer, &QTimer::timeout, this, &KeyNavDevModel::reload);
    m_reloadDelayer.setInterval(500);
    m_reloadDelayer.setSingleShot(true);
}

void KeyNavDevModel::reload()
{
    m_sections.clear();
    qDeleteAll(m_memstore);
    m_memstore.clear();

    const std::set<IKeyNavigationSection*>& sectionsSet = keynavController()->sections();
    QList<IKeyNavigationSection*> sectionsList = toQList(sectionsSet);
    sortByIndex(sectionsList);

    for (IKeyNavigationSection* s : sectionsList) {
        m_sections << toWrap(s);
    }

    emit sectionsChanged();
}

QVariant KeyNavDevModel::toWrap(IKeyNavigationSection* s)
{
    KeyNavDevSection* qs = new KeyNavDevSection(s);
    m_memstore.append(qs);

    QVariantList subVarList;
    const std::set<IKeyNavigationSubSection*>& subsectionsSet = s->subsections();
    QList<IKeyNavigationSubSection*> subsectionsList = toQList(subsectionsSet);
    sortByIndex(subsectionsList);
    for (IKeyNavigationSubSection* sub : subsectionsList) {
        subVarList << toWrap(sub);
    }

    qs->setSubsections(subVarList);

    s->subsectionsListChanged().onNotify(this, [this]() {
        m_reloadDelayer.start();
    });

    return QVariant::fromValue(qs);
}

QVariant KeyNavDevModel::toWrap(IKeyNavigationSubSection* sub)
{
    KeyNavDevSubSection* qsub = new KeyNavDevSubSection(sub);
    m_memstore.append(qsub);

    QVariantList conVarList;
    const std::set<IKeyNavigationControl*>& controlsSet = sub->controls();
    QList<IKeyNavigationControl*> controlsList = toQList(controlsSet);
    sortByIndex(controlsList);
    for (IKeyNavigationControl* ctrl : controlsList) {
        conVarList << toWrap(ctrl);
    }

    qsub->setControls(conVarList);

    sub->controlsListChanged().onNotify(this, [this]() {
        m_reloadDelayer.start();
    });

    return QVariant::fromValue(qsub);
}

QVariant KeyNavDevModel::toWrap(IKeyNavigationControl* ctrl)
{
    KeyNavDevControl* qc = new KeyNavDevControl(ctrl);
    m_memstore.append(qc);
    return QVariant::fromValue(qc);
}

QVariantList KeyNavDevModel::sections() const
{
    return m_sections;
}
