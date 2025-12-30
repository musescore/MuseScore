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

#ifndef MU_NOTATION_VOICINGSELECT_H
#define MU_NOTATION_VOICINGSELECT_H

#include "ui_voicing_select.h"

namespace mu::notation {
class VoicingSelect : public QWidget, public Ui::VoicingSelect
{
    Q_OBJECT

public:
    VoicingSelect(QWidget* parent);
    int getVoicing() { return voicingBox->currentIndex(); }
    bool getLiteral() { return interpretBox->currentIndex(); }
    int getDuration() { return durationBox->currentIndex(); }

    void setVoicing(int idx);
    void setLiteral(bool literal);
    void setDuration(int duration);

signals:
    void voicingChanged(bool, int, int);

private:
    void blockVoicingSignals(bool);

private slots:
    void _voicingChanged();
};
}

#endif // MU_NOTATION_VOICINGSELECT_H
