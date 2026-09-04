/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#pragma once

#include <QObject>
#include <qqmlintegration.h>

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

#include "notation/inotation.h"
#include "notation/inotationinteraction.h"
#include "notation/inotationselection.h"
#include "notation/notationtypes.h"

namespace mu::notation {
struct TransposeDialogState {
    bool chromaticChecked = true;
    bool transposeByKeyChecked = true;
    TransposeDirection direction = TransposeDirection::CLOSEST;
    int keyListIdx = 7;
    int chromaticIntervalIdx = 0;
    int diatonicIntervalIdx = 0;
    bool keepDegreeAlterationsChecked = true;
    bool needTransposeKeysChecked = true;
    bool needTransposeChordNamesChecked = true;
    int needTransposeDoubleSharpsFlatsIdx = 1;
};

class TransposeDialogModel : public QObject, public muse::Contextable
{
    Q_OBJECT
    QML_ELEMENT

    // which of the three mutually-exclusive top-level modes is active (TransposeMode)
    Q_PROPERTY(int mode READ mode WRITE setMode NOTIFY modeChanged)

    // "To Key" sub-options (chromatic / to-key mode)
    Q_PROPERTY(int keyIndex READ keyIndex WRITE setKeyIndex NOTIFY keyIndexChanged)

    // "By Interval" sub-options (chromatic / by-interval mode)
    Q_PROPERTY(int intervalIndex READ intervalIndex WRITE setIntervalIndex NOTIFY intervalIndexChanged)

    // diatonic mode sub-options
    Q_PROPERTY(int degreeIndex READ degreeIndex WRITE setDegreeIndex NOTIFY degreeIndexChanged)
    Q_PROPERTY(bool keepDegreeAlterations READ keepDegreeAlterations WRITE setKeepDegreeAlterations NOTIFY keepDegreeAlterationsChanged)

    // shared options
    Q_PROPERTY(int direction READ direction WRITE setDirection NOTIFY directionChanged)
    Q_PROPERTY(bool transposeKeys READ transposeKeys WRITE setTransposeKeys NOTIFY transposeKeysChanged)
    Q_PROPERTY(bool transposeChordNames READ transposeChordNames WRITE setTransposeChordNames NOTIFY transposeChordNamesChanged)
    Q_PROPERTY(bool useDoubleSharpsFlats READ useDoubleSharpsFlats WRITE setUseDoubleSharpsFlats NOTIFY useDoubleSharpsFlatsChanged)

    // enable/disable flags computed from the current selection
    Q_PROPERTY(bool enableTransposeToKey READ enableTransposeToKey NOTIFY enableTransposeToKeyChanged)
    Q_PROPERTY(bool enableTransposeKeys READ enableTransposeKeys NOTIFY enableTransposeKeysChanged)
    Q_PROPERTY(bool enableTransposeChordNames READ enableTransposeChordNames NOTIFY enableTransposeChordNamesChanged)

    muse::ContextInject<context::IGlobalContext> context = { this };

public:
    explicit TransposeDialogModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE void apply();

    int mode() const; // TransposeMode
    int keyIndex() const;
    int intervalIndex() const;
    int degreeIndex() const;
    int direction() const;
    bool keepDegreeAlterations() const;
    bool transposeKeys() const;
    bool transposeChordNames() const;
    bool useDoubleSharpsFlats() const;

    bool enableTransposeToKey() const;
    bool enableTransposeKeys() const;
    bool enableTransposeChordNames() const;

public slots:
    void setMode(int mode); // TransposeMode
    void setKeyIndex(int index);
    void setIntervalIndex(int index);
    void setDegreeIndex(int index);
    void setDirection(int direction);
    void setKeepDegreeAlterations(bool value);
    void setTransposeKeys(bool value);
    void setTransposeChordNames(bool value);
    void setUseDoubleSharpsFlats(bool value);

signals:
    void modeChanged();
    void keyIndexChanged();
    void intervalIndexChanged();
    void degreeIndexChanged();
    void directionChanged();
    void keepDegreeAlterationsChanged();
    void transposeKeysChanged();
    void transposeChordNamesChanged();
    void useDoubleSharpsFlatsChanged();

    void enableTransposeToKeyChanged();
    void enableTransposeKeysChanged();
    void enableTransposeChordNamesChanged();

private:
    static TransposeDialogState& previousState();
    void restorePreviousState();
    void saveState();

    TransposeMode m_mode = TransposeMode::TO_KEY;

    int m_keyIndex = 7;
    int m_intervalIndex = 0;
    int m_degreeIndex = 0;
    TransposeDirection m_direction = TransposeDirection::CLOSEST;
    bool m_keepDegreeAlterations = true;
    // NOTE: mode/keyDirection/intervalDirection/degreeDirection are stored here as their
    // real C++ enum types, but exposed to QML as plain int (see Q_PROPERTY above) since
    // TransposeMode (engraving/dom/mscore.h) is not Q_ENUM-registered for QML.

    bool m_transposeKeys = true;
    bool m_transposeChordNames = true;
    bool m_useDoubleSharpsFlats = true;

    bool m_enableTransposeToKey = true;
    bool m_enableTransposeKeys = true;
    bool m_enableTransposeChordNames = false;

    bool m_allSelected = false;
};
}
