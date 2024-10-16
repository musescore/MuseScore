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
#include "diagnosticnavigationmodel.h"

#include <algorithm>

#include "keynavdevsection.h"
#include "keynavdevsubsection.h"
#include "keynavdevcontrol.h"

using namespace muse::diagnostics;
using namespace muse::ui;

template<class T>
static QList<T*> toQList(std::set<T*> set)
{
    QList<T*> l;
    l.reserve(int(set.size()));
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

DiagnosticNavigationModel::DiagnosticNavigationModel(QObject* parent)
    : QObject(parent), Injectable(muse::iocCtxForQmlObject(this))
{
    connect(&m_reloadDelayer, &QTimer::timeout, this, &DiagnosticNavigationModel::reload);
    m_reloadDelayer.setInterval(500);
    m_reloadDelayer.setSingleShot(true);
}

void DiagnosticNavigationModel::reload()
{
    emit beforeReload();
    m_sections.clear();
    qDeleteAll(m_memstore);
    m_memstore.clear();

    const std::set<INavigationSection*>& sectionsSet = navigationController()->sections();
    QList<INavigationSection*> sectionsList = toQList(sectionsSet);
    sortByIndex(sectionsList);

    for (INavigationSection* s : sectionsList) {
        m_sections << toWrap(s);
    }

    emit sectionsChanged();
    emit afterReload();
}

QVariant DiagnosticNavigationModel::toWrap(INavigationSection* s)
{
    KeyNavDevSection* qs = new KeyNavDevSection(s);
    m_memstore.append(qs);

    QVariantList subVarList;
    const std::set<INavigationPanel*>& subsectionsSet = s->panels();
    QList<INavigationPanel*> subsectionsList = toQList(subsectionsSet);
    sortByIndex(subsectionsList);
    for (INavigationPanel* sub : subsectionsList) {
        subVarList << toWrap(sub);
    }

    qs->setSubsections(subVarList);

    s->panelsListChanged().onNotify(this, [this]() {
        m_reloadDelayer.start();
    });

    return QVariant::fromValue(qs);
}

QVariant DiagnosticNavigationModel::toWrap(INavigationPanel* sub)
{
    KeyNavDevSubSection* qsub = new KeyNavDevSubSection(sub);
    m_memstore.append(qsub);

    QVariantList conVarList;
    const std::set<INavigationControl*>& controlsSet = sub->controls();
    QList<INavigationControl*> controlsList = toQList(controlsSet);
    sortByIndex(controlsList);
    for (INavigationControl* ctrl : controlsList) {
        conVarList << toWrap(ctrl);
    }

    qsub->setControls(conVarList);

    sub->controlsListChanged().onNotify(this, [this]() {
        m_reloadDelayer.start();
    });

    return QVariant::fromValue(qsub);
}

QVariant DiagnosticNavigationModel::toWrap(INavigationControl* ctrl)
{
    KeyNavDevControl* qc = new KeyNavDevControl(ctrl);
    m_memstore.append(qc);
    return QVariant::fromValue(qc);
}

QVariantList DiagnosticNavigationModel::sections() const
{
    return m_sections;
}
