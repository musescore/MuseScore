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

    m_blackKeyRects.clear();
    m_whiteKeyRects.clear();

    m_spacing = std::min(2.0 * m_keyWidthScaling, 2.0);

    qreal whiteKeyWidth = 32.0 * m_keyWidthScaling;
    qreal blackKeyWidth = 20.0 * m_keyWidthScaling;

    qreal whiteKeyHeight = std::min(height() - m_spacing, 4.27 * whiteKeyWidth);
    qreal blackKeyHeight = whiteKeyHeight * 0.625;

    qreal hPos = m_spacing / 2;

    key_t numKeys = std::min<key_t>(m_lowestKey + m_numberOfKeys, MAX_NUM_KEYS);
    for (key_t key = m_lowestKey; key < numKeys; ++key) {
        if (isBlackKey(key)) {
            constexpr qreal offsets[12] {
                0.0, -13.0, 0.0, -7.0, 0.0, 0.0, -13.0, 0.0, -10.0, 0.0, -7.0, 0.0
            };

            qreal offset = offsets[key % 12] * m_keyWidthScaling;

            m_blackKeyRects[key] = QRectF(hPos + offset, m_spacing, blackKeyWidth, blackKeyHeight);
        } else {
            m_whiteKeyRects[key] = QRectF(hPos, m_spacing, whiteKeyWidth, whiteKeyHeight);
            hPos += whiteKeyWidth;
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

    updateScrollBar();
}

void PianoKeyboardView::determineOctaveLabelsFont()
{
    m_octaveLabelsFont.setFamily(QString::fromStdString(uiConfiguration()->fontFamily()));
    m_octaveLabelsFont.setPixelSize(uiConfiguration()->fontSize() * std::min(m_keyWidthScaling, 1.5));
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
            qreal octaveLabelBottomOffset = std::min(8.0 * m_keyWidthScaling, 8.0);

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
    QLinearGradient topPieceGradient;
    QLinearGradient bottomPieceGradient;

    QPainterPath backgroundPath;
    QPainterPath topPiecePath;
    QPainterPath bottomPiecePath;

    for (auto [key, rect] : m_blackKeyRects) {
        if (!viewport.intersects(rect)) {
            continue;
        }

        if (backgroundPath.isEmpty()) {
            qreal cornerRadius = m_spacing;
            qreal bottomPieceHeight = 8.0 * m_keyWidthScaling;

            static const QColor lighterColor(78, 78, 78);

            // Make background
            qreal left = 0.0, top = 0.0,
                  right = rect.width(), bottom = rect.height();
            backgroundPath.moveTo(left, top);
            backgroundPath.lineTo(left, bottom - cornerRadius);
            backgroundPath.quadTo(left, bottom, left + cornerRadius, bottom);
            backgroundPath.lineTo(right - cornerRadius, bottom);
            backgroundPath.quadTo(right, bottom, right, bottom - cornerRadius);
            backgroundPath.lineTo(right, top);
            backgroundPath.closeSubpath();

            // Make top piece
            left += m_spacing;
            right -= m_spacing;
            top = m_spacing / 2;
            bottom = rect.height() - 1.5 * m_spacing - bottomPieceHeight;
            qreal center = rect.width() / 2;

            topPiecePath.moveTo(left, top);
            topPiecePath.lineTo(left, bottom - cornerRadius);
            topPiecePath.quadTo(left, bottom, center, bottom);
            topPiecePath.quadTo(right, bottom, right, bottom - cornerRadius);
            topPiecePath.lineTo(right, top);
            topPiecePath.closeSubpath();

            topPieceGradient.setColorAt(0.0, backgroundColor);
            topPieceGradient.setColorAt(1.0, lighterColor);
            topPieceGradient.setStart(0.0, top);
            topPieceGradient.setFinalStop(0.0, bottom);

            // Make bottom piece
            top = rect.height() - m_spacing - bottomPieceHeight;
            bottom = rect.height() - m_spacing;

            bottomPiecePath.moveTo(left, top + cornerRadius);
            bottomPiecePath.quadTo(left, top, center, top);
            bottomPiecePath.quadTo(right, top, right, top + cornerRadius);
            bottomPiecePath.lineTo(right, bottom);
            bottomPiecePath.lineTo(left, bottom);
            bottomPiecePath.closeSubpath();

            bottomPieceGradient.setColorAt(0.0, lighterColor);
            bottomPieceGradient.setColorAt(1.0, backgroundColor);
            bottomPieceGradient.setStart(0.0, top);
            bottomPieceGradient.setFinalStop(0.0, bottom);
        }

        painter->translate(rect.topLeft());
        painter->fillPath(backgroundPath, backgroundColor);
        painter->fillPath(topPiecePath, topPieceGradient);
        painter->fillPath(bottomPiecePath, bottomPieceGradient);
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

qreal PianoKeyboardView::keyWidthScaling() const
{
    return m_keyWidthScaling;
}

void PianoKeyboardView::scale(qreal factor, qreal x)
{
    setScaling(m_keyWidthScaling * factor, x);
}

void PianoKeyboardView::setScaling(qreal scaling, qreal x)
{
    qreal newScaling = std::clamp(scaling, 0.5, 2.0);
    qreal correctedFactor = newScaling / m_keyWidthScaling;

    m_keyWidthScaling = newScaling;
    m_scrollOffset *= correctedFactor;
    m_scrollOffset += x * (1 - correctedFactor);

    emit keyWidthScalingChanged();
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

qreal PianoKeyboardView::scrollBarPosition() const
{
    return m_scrollBarPosition;
}

qreal PianoKeyboardView::scrollBarSize() const
{
    return m_scrollBarSize;
}

void PianoKeyboardView::updateScrollBar()
{
    qreal newPosition = -m_scrollOffset / m_keysAreaRect.width();
    qreal newSize = width() / m_keysAreaRect.width();

    if (qFuzzyCompare(newPosition, m_scrollBarPosition)
        && qFuzzyCompare(newSize, m_scrollBarSize)) {
        return;
    }

    m_scrollBarPosition = newPosition;
    m_scrollBarSize = newSize;
    emit scrollBarChanged();
}

void PianoKeyboardView::setScrollBarPosition(qreal position)
{
    m_scrollOffset = -position* m_keysAreaRect.width();
    adjustKeysAreaPosition();
    update();
}
