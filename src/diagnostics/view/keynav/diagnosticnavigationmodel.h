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
#ifndef MU_DIAGNOSTICS_KEYNAVDEVMODEL_H
#define MU_DIAGNOSTICS_KEYNAVDEVMODEL_H

#include <QObject>
#include <QTimer>

#include "modularity/ioc.h"
#include "ui/inavigationcontroller.h"
#include "async/asyncable.h"

namespace mu::diagnostics {
class AbstractKeyNavDevItem;
class DiagnosticNavigationModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QVariantList sections READ sections NOTIFY sectionsChanged)

    INJECT(ui::INavigationController, navigationController)
public:
    explicit DiagnosticNavigationModel(QObject* parent = nullptr);

    QVariantList sections() const;

    Q_INVOKABLE void reload();

signals:
    void beforeReload();
    void afterReload();
    void sectionsChanged();

private:

    QVariant toWrap(ui::INavigationSection* s);
    QVariant toWrap(ui::INavigationPanel* sub);
    QVariant toWrap(ui::INavigationControl* ctrl);

    QVariantList m_sections;
    mutable QList<AbstractKeyNavDevItem*> m_memstore;
    QTimer m_reloadDelayer;
};
}

#endif // MU_DIAGNOSTICS_KEYNAVDEVMODEL_H
