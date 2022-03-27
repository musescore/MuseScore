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

    connect(this, &QQuickItem::widthChanged, this, &PianoKeyboardView::calculateKeyRects);
    connect(this, &QQuickItem::heightChanged, this, &PianoKeyboardView::calculateKeyRects);

    uiConfiguration()->fontChanged().onNotify(this, [this]() {
        determineOctaveLabelsFont();
        update();
    });
}

void PianoKeyboardView::calculateKeyRects()
{
    determineOctaveLabelsFont();

    m_spacing = std::min(2.0 * m_keyWidthScaling, 2.0);

    m_whiteKeyWidth = 32.0 * m_keyWidthScaling;
    m_blackKeyWidth = 20.0 * m_keyWidthScaling;

    m_blackKeyRects.clear();
    m_whiteKeyRects.clear();

    qreal whiteKeyHeight = std::min(height() - m_spacing, 8.0 * m_whiteKeyWidth);
    qreal blackKeyHeight = whiteKeyHeight * 0.625;

    qreal hPos = m_spacing / 2;

    key_t numKeys = std::min<key_t>(m_lowestKey + m_numberOfKeys, MAX_NUM_KEYS);
    for (key_t key = m_lowestKey; key < numKeys; ++key) {
        if (isBlackKey(key)) {
            constexpr qreal offsets[12] {
                0.0, -13.0, 0.0, -7.0, 0.0, 0.0, -13.0, 0.0, -10.0, 0.0, -7.0, 0.0
            };

            qreal offset = offsets[key % 12] * m_keyWidthScaling;

            m_blackKeyRects[key] = QRectF(hPos + offset, m_spacing, m_blackKeyWidth, blackKeyHeight);
        } else {
            m_whiteKeyRects[key] = QRectF(hPos, m_spacing, m_whiteKeyWidth, whiteKeyHeight);
            hPos += m_whiteKeyWidth;
        }
    }

    qreal keysAreaWidth = hPos + m_spacing / 2;
    qreal keysAreaHeight = m_spacing + whiteKeyHeight;

    m_keysAreaRect.setSize(QSizeF(keysAreaWidth, keysAreaHeight));
    adjustKeysAreaPosition();
}

void PianoKeyboardView::adjustKeysAreaPosition()
{
    qreal keysAreaTop = (height() - m_keysAreaRect.height()) / 2;

    qreal minScrollOffset = std::min(width() - m_keysAreaRect.width(), (width() - m_keysAreaRect.width()) / 2);
    qreal maxScrollOffset = std::max(0.0, (width() - m_keysAreaRect.width()) / 2);
    m_scrollOffset = std::clamp(m_scrollOffset, minScrollOffset, maxScrollOffset);

    m_keysAreaRect.moveTo(QPointF(m_scrollOffset, keysAreaTop));
}

void PianoKeyboardView::determineOctaveLabelsFont()
{
    m_octaveLabelsFont.setFamily(QString::fromStdString(uiConfiguration()->fontFamily()));
    m_octaveLabelsFont.setPixelSize(uiConfiguration()->fontSize() * m_keyWidthScaling);
}

void PianoKeyboardView::paint(QPainter* painter)
{
    painter->setRenderHint(QPainter::Antialiasing);

    paintBackground(painter);

    QPointF pos = m_keysAreaRect.topLeft();
    painter->translate(pos);

    QRectF viewport = QRectF(0.0, 0.0, width(), height()).translated(-pos);
    paintWhiteKeys(painter, viewport);
    paintBlackKeys(painter, viewport);
}

void PianoKeyboardView::paintBackground(QPainter* painter)
{
    painter->fillRect(m_keysAreaRect, backgroundColor);
}

void PianoKeyboardView::paintWhiteKeys(QPainter* painter, const QRectF& viewport)
{
    QPainterPath path;

    for (auto [key, rect] : m_whiteKeyRects) {
        if (!viewport.intersects(rect)) {
            continue;
        }

        qreal inset = m_spacing / 2;

        qreal left = inset, top = 0.0,
              right = rect.width() - inset, bottom = rect.height() - m_spacing;

        if (path.isEmpty()) {
            qreal cornerRadius = m_spacing;

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
            qreal octaveLabelBottomOffset = 8.0 * m_keyWidthScaling;

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

void PianoKeyboardView::paintBlackKeys(QPainter* painter, const QRectF& viewport)
{
    QLinearGradient gradient1;
    QLinearGradient gradient2;

    QPainterPath path1;
    QPainterPath path2;

    for (auto [key, rect] : m_blackKeyRects) {
        if (!viewport.intersects(rect)) {
            continue;
        }

        painter->fillRect(rect, backgroundColor);

        if (path1.isEmpty()) {
            qreal cornerRadius = m_spacing;
            qreal bottomPieceHeight = 8.0 * m_keyWidthScaling;

            qreal left = m_spacing, top1 = 0.0,
                  right = rect.width() - m_spacing, bottom1 = rect.height() - 2 * m_spacing - bottomPieceHeight,
                  center = rect.width() / 2;
            path1.moveTo(left, top1);
            path1.lineTo(left, bottom1 - cornerRadius);
            path1.quadTo(left, bottom1, center, bottom1);
            path1.quadTo(right, bottom1, right, bottom1 - cornerRadius);
            path1.lineTo(right, top1);
            path1.closeSubpath();

            qreal top2 = rect.height() - m_spacing - bottomPieceHeight,
                  bottom2 = rect.height() - m_spacing;
            path2.moveTo(left, top2 + cornerRadius);
            path2.quadTo(left, top2, center, top2);
            path2.quadTo(right, top2, right, top2 + cornerRadius);
            path2.lineTo(right, bottom2);
            path2.lineTo(left, bottom2);
            path2.closeSubpath();

            static const QColor lighterColor(78, 78, 78);

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

int PianoKeyboardView::numberOfKeys() const
{
    return m_numberOfKeys;
}

void PianoKeyboardView::setNumberOfKeys(int number)
{
    if (m_numberOfKeys == number) {
        return;
    }

    switch (number) {
    case 128:
        m_lowestKey = 0;
        m_numberOfKeys = 128;
        break;
    case 88:
        m_lowestKey = 33;
        m_numberOfKeys = 88;
        break;
    case 61:
        m_lowestKey = 48;
        m_numberOfKeys = 61;
        break;
    case 49:
        m_lowestKey = 48;
        m_numberOfKeys = 49;
        break;
    case 25:
        m_lowestKey = 60;
        m_numberOfKeys = 25;
        break;
    default:
        return;
    }

    emit numberOfKeysChanged();
    calculateKeyRects();
    update();
}

void PianoKeyboardView::moveCanvas(qreal dx)
{
    m_scrollOffset += dx;
    adjustKeysAreaPosition();
    update();
}

void PianoKeyboardView::scale(qreal factor, qreal x)
{
    qreal newScaling = std::clamp(m_keyWidthScaling * factor, 0.5, 2.0);
    qreal correctedFactor = newScaling / m_keyWidthScaling;

    m_keyWidthScaling = newScaling;
    m_scrollOffset *= correctedFactor;
    m_scrollOffset += x * (1 - correctedFactor);

    calculateKeyRects();
    update();
}

qreal abs2d(qreal x, qreal y)
{
    return sqrt(x * x + y * y) * (y > -x ? 1 : -1);
}

void PianoKeyboardView::wheelEvent(QWheelEvent* event)
{
    QPoint delta = event->pixelDelta();

    if (delta.isNull()) {
        delta = event->angleDelta();
    }

    Qt::KeyboardModifiers keyState = event->modifiers();

    // Windows touch pad pinches also execute this
    if (keyState & Qt::ControlModifier) {
        qreal abs = abs2d(delta.x(), delta.y());
        constexpr qreal zoomSpeed = 1.002;
        qreal factor = pow(zoomSpeed, abs);
        scale(factor, event->position().x());
        return;
    }

    if (delta.x() == 0) {
        // Make life easy for people who can only scroll vertically
        moveCanvas(delta.y());
    } else if (keyState & Qt::ShiftModifier) {
        qreal abs = abs2d(delta.x(), delta.y());
        moveCanvas(abs);
    } else {
        moveCanvas(delta.x());
    }
}
