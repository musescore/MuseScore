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
#include "pianokeyboardview.h"

#include "log.h"

using namespace mu::notation;

static const QColor backgroundColor(36, 36, 39);

static constexpr bool isBlackKey(uint8_t key)
{
    constexpr bool isBlack[12] { false, true, false, true, false, false, true, false, true, false, true, false };

    return isBlack[key % 12];
}

PianoKeyboardView::PianoKeyboardView(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    calculateKeyRects();

    connect(this, &QQuickItem::heightChanged, this, &PianoKeyboardView::calculateKeyRects);

    initOctaveLabelsFont();

    uiConfiguration()->fontChanged().onNotify(this, [this]() {
        initOctaveLabelsFont();
        update();
    });
}

void PianoKeyboardView::calculateKeyRects()
{
    constexpr qreal spacing = 2.0;

    constexpr qreal whiteKeyWidth = 32.0;
    constexpr qreal blackKeyWidth = 20.0;

    m_blackKeyRects.clear();
    m_whiteKeyRects.clear();

    qreal whiteKeyHeight = height() - spacing;
    qreal blackKeyHeight = whiteKeyHeight * 0.625;

    qreal hPos = spacing / 2;

    key_t numKeys = std::min<key_t>(m_lowestKey + m_numberOfKeys, MAX_NUM_KEYS);
    for (key_t key = m_lowestKey; key < numKeys; ++key) {
        if (isBlackKey(key)) {
            constexpr qreal offsets[12] {
                0.0, -13.0, 0.0, -7.0, 0.0, 0.0, -13.0, 0.0, -10.0, 0.0, -7.0, 0.0
            };

            qreal offset = offsets[key % 12];

            m_blackKeyRects[key] = QRectF(hPos + offset, spacing, blackKeyWidth, blackKeyHeight);
        } else {
            m_whiteKeyRects[key] = QRectF(hPos, spacing, whiteKeyWidth, whiteKeyHeight);
            hPos += whiteKeyWidth;
        }
    }
}

void PianoKeyboardView::initOctaveLabelsFont()
{
    m_octaveLabelsFont.setFamily(QString::fromStdString(uiConfiguration()->fontFamily()));
    m_octaveLabelsFont.setPixelSize(uiConfiguration()->fontSize());
}

void PianoKeyboardView::paint(QPainter* painter)
{
    painter->setRenderHint(QPainter::Antialiasing);

    paintBackground(painter);

    paintWhiteKeys(painter);
    paintBlackKeys(painter);
}

void PianoKeyboardView::paintBackground(QPainter* painter)
{
    painter->fillRect(QRectF(0.0, 0.0, width(), height()), backgroundColor);
}

void PianoKeyboardView::paintWhiteKeys(QPainter* painter)
{
    QPainterPath path;

    for (auto [key, rect] : m_whiteKeyRects) {
        constexpr qreal spacing = 2.0;
        constexpr qreal inset = spacing / 2;

        qreal left = inset, top = 0.0,
              right = rect.width() - inset, bottom = rect.height() - spacing;

        if (path.isEmpty()) {
            constexpr qreal cornerRadius = 2.0;

            path.moveTo(left, top);
            path.lineTo(left, bottom - cornerRadius);
            path.quadTo(left, bottom, left + cornerRadius, bottom);
            path.lineTo(right - cornerRadius, bottom);
            path.quadTo(right, bottom, right, bottom - cornerRadius);
            path.lineTo(right, top);
            path.closeSubpath();
        }

        painter->translate(rect.topLeft());

        painter->fillPath(path, Qt::white);

        if (key % 12 == 0) {
            // Draw octave label
            constexpr qreal octaveLabelBottomOffset = 8.0;

            int octaveNumber = (key / 12) - 2;

            QString octaveLabel = "C" + QString::number(octaveNumber);
            QRect octaveLabelRect(left, top, right - left, bottom - top - octaveLabelBottomOffset);

            painter->setPen(backgroundColor);
            painter->setFont(m_octaveLabelsFont);
            painter->drawText(octaveLabelRect, Qt::AlignHCenter | Qt::AlignBottom, octaveLabel);
        }

        painter->translate(-rect.topLeft());
    }
}

void PianoKeyboardView::paintBlackKeys(QPainter* painter)
{
    QLinearGradient gradient1;
    QLinearGradient gradient2;

    QPainterPath path1;
    QPainterPath path2;

    for (auto [key, rect] : m_blackKeyRects) {
        painter->fillRect(rect, backgroundColor);

        if (path1.isEmpty()) {
            constexpr qreal spacing = 2.0;
            constexpr qreal cornerRadius = 2.0;
            constexpr qreal bottomPieceHeight = 8.0;

            qreal left = spacing, top1 = 0.0,
                  right = rect.width() - spacing, bottom1 = rect.height() - 2 * spacing - bottomPieceHeight,
                  center = rect.width() / 2;
            path1.moveTo(left, top1);
            path1.lineTo(left, bottom1 - cornerRadius);
            path1.quadTo(left, bottom1, center, bottom1);
            path1.quadTo(right, bottom1, right, bottom1 - cornerRadius);
            path1.lineTo(right, top1);
            path1.closeSubpath();

            qreal top2 = rect.height() - spacing - bottomPieceHeight,
                  bottom2 = rect.height() - spacing;
            path2.moveTo(left, top2 + cornerRadius);
            path2.quadTo(left, top2, center, top2);
            path2.quadTo(right, top2, right, top2 + cornerRadius);
            path2.lineTo(right, bottom2);
            path2.lineTo(left, bottom2);
            path2.closeSubpath();

            static const QColor lighterColor = QColor::fromRgb(78, 78, 78);

            gradient1.setColorAt(0.0, backgroundColor);
            gradient1.setColorAt(1.0, lighterColor);
            gradient1.setStart(0.0, top1);
            gradient1.setFinalStop(0.0, bottom1);

            gradient2.setColorAt(0.0, lighterColor);
            gradient2.setColorAt(1.0, backgroundColor);
            gradient2.setStart(0.0, top2);
            gradient2.setFinalStop(0.0, bottom2);
        }

        painter->translate(rect.topLeft());
        painter->fillPath(path1, gradient1);
        painter->fillPath(path2, gradient2);
        painter->translate(-rect.topLeft());
    }
}
