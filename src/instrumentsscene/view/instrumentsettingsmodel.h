/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

#include "notation/notationtypes.h"

namespace mu::instrumentsscene {
class InstrumentSettingsModel : public QObject, public muse::async::Asyncable
{
    Q_OBJECT

    INJECT(context::IGlobalContext, context)

    Q_PROPERTY(QString instrumentName READ instrumentName WRITE setInstrumentName NOTIFY dataChanged)
    Q_PROPERTY(QString abbreviature READ abbreviature WRITE setAbbreviature NOTIFY dataChanged)

    Q_PROPERTY(bool isMainScore READ isMainScore NOTIFY isMainScoreChanged)

public:
    explicit InstrumentSettingsModel(QObject* parent = nullptr);

    Q_INVOKABLE void load(const QVariant& instrument);

    QString instrumentName() const;
    QString abbreviature() const;

    bool isMainScore() const;

public slots:
    void setInstrumentName(const QString& name);
    void setAbbreviature(const QString& abbreviature);

signals:
    void dataChanged();
    bool isMainScoreChanged();

private:
    notation::INotationPtr currentNotation() const;
    notation::INotationPtr currentMasterNotation() const;
    notation::INotationPartsPtr notationParts() const;

    notation::InstrumentKey m_instrumentKey;
    QString m_instrumentName;
    QString m_instrumentAbbreviature;
};
}

#endif // MU_INSTRUMENTSSCENE_INSTRUMENTSETTINGSMODEL_H
