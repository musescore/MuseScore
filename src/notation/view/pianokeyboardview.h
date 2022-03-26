/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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
#ifndef MU_NOTATION_PIANOKEYBOARDVIEW_H
#define MU_NOTATION_PIANOKEYBOARDVIEW_H

#include <QQuickPaintedItem>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "ui/iuiconfiguration.h"

namespace mu::notation {
class PianoKeyboardView : public QQuickPaintedItem, public async::Asyncable
{
    Q_OBJECT

    INJECT(notation, ui::IUiConfiguration, uiConfiguration)

public:
    explicit PianoKeyboardView(QQuickItem* parent = nullptr);

    void paint(QPainter* painter) override;

private:
    using key_t = uint8_t;

    void calculateKeyRects();
    void initOctaveLabelsFont();

    void paintBackground(QPainter* painter);

    void paintWhiteKeys(QPainter* painter);
    void paintBlackKeys(QPainter* painter);

    static constexpr key_t MIN_KEY = 0;
    static constexpr key_t MAX_NUM_KEYS = 128;

    key_t m_lowestKey = MIN_KEY;
    key_t m_numberOfKeys = MAX_NUM_KEYS;

    std::map<key_t, QRectF> m_blackKeyRects;
    std::map<key_t, QRectF> m_whiteKeyRects;

    QFont m_octaveLabelsFont;
};
}

#endif // MU_NOTATION_PIANOKEYBOARDVIEW_H
