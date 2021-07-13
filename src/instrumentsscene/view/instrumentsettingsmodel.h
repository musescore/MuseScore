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
#ifndef MU_INSTRUMENTSSCENE_INSTRUMENTSETTINGSMODEL_H
#define MU_INSTRUMENTSSCENE_INSTRUMENTSETTINGSMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "context/iglobalcontext.h"
#include "notation/notationtypes.h"
#include "notation/iselectinstrumentscenario.h"

namespace mu::instrumentsscene {
class InstrumentSettingsModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(instruments, notation::ISelectInstrumentsScenario, selectInstrumentsScenario)
    INJECT(instruments, context::IGlobalContext, context)

    Q_PROPERTY(QString instrumentName READ instrumentName WRITE setInstrumentName NOTIFY dataChanged)
    Q_PROPERTY(QString partName READ partName WRITE setPartName NOTIFY dataChanged)
    Q_PROPERTY(QString abbreviature READ abbreviature WRITE setAbbreviature NOTIFY dataChanged)

public:
    explicit InstrumentSettingsModel(QObject* parent = nullptr);

    Q_INVOKABLE void load(const QVariant& instrument);
    Q_INVOKABLE void replaceInstrument();

    QString instrumentName() const;
    QString partName() const;
    QString abbreviature() const;

public slots:
    void setInstrumentName(const QString& name);
    void setPartName(const QString& name);
    void setAbbreviature(const QString& abbreviature);

signals:
    void dataChanged();

private:
    notation::INotationPartsPtr parts() const;

    QString m_partId;
    QString m_partName;
    QString m_instrumentId;
    QString m_instrumentName;
    QString m_instrumentAbbreviature;
};
}

#endif // MU_INSTRUMENTSSCENE_INSTRUMENTSETTINGSMODEL_H
