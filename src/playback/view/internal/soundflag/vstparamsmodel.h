/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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

#ifndef MU_PLAYBACK_VSTPARAMSMODEL_H
#define MU_PLAYBACK_VSTPARAMSMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

#include "notation/notationtypes.h"

namespace mu::playback {
class VSTParamsModel : public QObject
{
    Q_OBJECT

    INJECT(context::IGlobalContext, globalContext)

    Q_PROPERTY(int keySwitch READ keySwitch NOTIFY keySwitchChanged FINAL)
    Q_PROPERTY(QString keySwitchStr READ keySwitchStr WRITE setKeySwitchStr NOTIFY keySwitchStrChanged FINAL)

public:
    VSTParamsModel(QObject* parent = nullptr);

    Q_INVOKABLE void init();

    Q_INVOKABLE QString increaseKeySwitch(const QString& keyStr);
    Q_INVOKABLE QString decreaseKeySwitch(const QString& keyStr);

    Q_INVOKABLE QString defaultKeySwitchStr() const;

    int keySwitch() const;

    QString keySwitchStr() const;
    void setKeySwitchStr(const QString& str);

signals:
    void keySwitchChanged();
    void keySwitchStrChanged();

private:
    notation::INotationSelectionPtr selection() const;
    notation::INotationUndoStackPtr undoStack() const;

    engraving::EngravingItem* m_item = nullptr;
    bool m_useFlat = false;
};
}

#endif // MU_PLAYBACK_VSTPARAMSMODEL_H
