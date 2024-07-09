/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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

#include <QPainter>

#include "pianokeyboardcontroller.h"

#include "log.h"

using namespace mu::notation;

static const QColor backgroundColor(36, 36, 39);

static constexpr bool isBlackKey(piano_key_t key)
{
    constexpr bool isBlack[12] { false, true, false, true, false, false, true, false, true, false, true, false };

    return isBlack[key % 12];
}

static qreal abs2d(qreal x, qreal y)
{
    return sqrt(x * x + y * y) * (y > -x ? 1 : -1);
}

static QColor mixedColors(QColor background, QColor foreground, qreal opacity)
{
    QColor result;
    result.setRed(opacity * foreground.red() + (1 - opacity) * background.red());
    result.setGreen(opacity * foreground.green() + (1 - opacity) * background.green());
    result.setBlue(opacity * foreground.blue() + (1 - opacity) * background.blue());
    return result;
}

PianoKeyboardView::PianoKeyboardView(QQuickItem* parent)
    : muse::uicomponents::QuickPaintedView(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
    setAcceptedMouseButtons(Qt::LeftButton);
}

PianoKeyboardView::~PianoKeyboardView()
{
    delete m_controller;
}

void PianoKeyboardView::init()
{
    m_isInitialized = true;

    calculateKeyRects();

    connect(this, &QQuickItem::widthChanged, this, &PianoKeyboardView::calculateKeyRects);
    connect(this, &QQuickItem::heightChanged, this, &PianoKeyboardView::calculateKeyRects);

    m_controller = new PianoKeyboardController(iocContext());

    uiConfiguration()->fontChanged().onNotify(this, [this]() {
        determineOctaveLabelsFont();
        update();
    });

    updateKeyStateColors();
    uiConfiguration()->currentThemeChanged().onNotify(this, [this]() {
        updateKeyStateColors();
        update();
    });

    m_controller->keyStatesChanged().onNotify(this, [this]() {
        updateKeyStateColors();
        update();
    });

    update();
}

void PianoKeyboardView::calculateKeyRects()
{
    TRACEFUNC;

    determineOctaveLabelsFont();

    m_blackKeyRects.clear();
    m_whiteKeyRects.clear();

    constexpr qreal defaultSpacing = 2.0;
    constexpr qreal maxSpacing = 2.0;

    m_spacing = std::min(defaultSpacing * m_keyWidthScaling, maxSpacing);

    constexpr qreal defaultWhiteKeyWidth = 30.0;
    constexpr qreal defaultBlackKeyWidth = 20.0;

    qreal whiteKeyWidth = defaultWhiteKeyWidth * m_keyWidthScaling + m_spacing;
    qreal blackKeyWidth = defaultBlackKeyWidth * m_keyWidthScaling;

    constexpr qreal defaultWhiteKeyHeight = 128.0;
    constexpr qreal defaultBlackKeyHeight = 84.0;

    m_whiteKeyHeight = std::min(height(), defaultWhiteKeyHeight * m_keyWidthScaling + m_spacing * 2);
    qreal actualKeyHeightScaling = (m_whiteKeyHeight - m_spacing * 2) / defaultWhiteKeyHeight;
    m_blackKeyHeight = defaultBlackKeyHeight * actualKeyHeightScaling + m_spacing;

    qreal hPos = m_spacing / 2;

    piano_key_t numKeys = std::min<piano_key_t>(m_lowestKey + m_numberOfKeys, MAX_NUM_KEYS);
    for (piano_key_t key = m_lowestKey; key < numKeys; ++key) {
        if (isBlackKey(key)) {
            constexpr qreal offsets[12] {
                0.0, -13.0, 0.0, -7.0, 0.0, 0.0, -13.0, 0.0, -10.0, 0.0, -7.0, 0.0
            };

            qreal offset = offsets[key % 12] * m_keyWidthScaling;

            m_blackKeyRects[key] = QRectF(hPos + offset, 0.0, blackKeyWidth, m_blackKeyHeight);
        } else {
            m_whiteKeyRects[key] = QRectF(hPos, 0.0, whiteKeyWidth, m_whiteKeyHeight);
            hPos += whiteKeyWidth;
        }
    }

    m_keysAreaRect.setSize(QSizeF(hPos + m_spacing / 2, m_whiteKeyHeight));
    adjustKeysAreaPosition();
}

void PianoKeyboardView::adjustKeysAreaPosition()
{
    TRACEFUNC;

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

void PianoKeyboardView::updateKeyStateColors()
{
    if (!m_isInitialized) {
        return;
    }

    auto themeValues = uiConfiguration()->currentTheme().values;

    QColor accentColor = themeValues[muse::ui::ACCENT_COLOR].toString();
    bool isKeysFromMidiInput = m_controller->isFromMidi();

    m_whiteKeyStateColors[KeyState::None] = Qt::white;
    m_whiteKeyStateColors[KeyState::OtherInSelectedChord] = mixedColors(Qt::white, accentColor, 0.25);
    m_whiteKeyStateColors[KeyState::Selected] = mixedColors(Qt::white, accentColor, isKeysFromMidiInput ? 0.8 : 0.5);
    m_whiteKeyStateColors[KeyState::Played] = mixedColors(Qt::white, accentColor, 0.8);

    QColor blackKeyTopPieceBaseColor(78, 78, 78);
    m_blackKeyTopPieceStateColors[KeyState::None] = blackKeyTopPieceBaseColor;
    m_blackKeyTopPieceStateColors[KeyState::OtherInSelectedChord] = mixedColors(blackKeyTopPieceBaseColor, accentColor, 0.4);
    m_blackKeyTopPieceStateColors[KeyState::Selected] = mixedColors(blackKeyTopPieceBaseColor, accentColor, 0.8);
    m_blackKeyTopPieceStateColors[KeyState::Played] = mixedColors(blackKeyTopPieceBaseColor, accentColor, 1.0);

    QColor blackKeyBottomPieceBaseColor(56, 56, 58);
    m_blackKeyBottomPieceStateColors[KeyState::None] = blackKeyBottomPieceBaseColor;
    m_blackKeyBottomPieceStateColors[KeyState::OtherInSelectedChord] = mixedColors(blackKeyBottomPieceBaseColor, accentColor, 0.4);
    m_blackKeyBottomPieceStateColors[KeyState::Selected] = mixedColors(blackKeyBottomPieceBaseColor, accentColor, 0.8);
    m_blackKeyBottomPieceStateColors[KeyState::Played] = mixedColors(blackKeyBottomPieceBaseColor, accentColor, 1.0);
}

void PianoKeyboardView::paint(QPainter* painter)
{
    if (!m_isInitialized) {
        return;
    }

    TRACEFUNC;

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

        qreal left = inset, top = m_spacing,
              right = rect.width() - inset, bottom = rect.height() - m_spacing;

        if (path.isEmpty()) {
            qreal cornerRadius = 2.0 * m_keyWidthScaling;

            path.moveTo(left, top);
            path.lineTo(left, bottom - cornerRadius);
            path.quadTo(left, bottom, left + cornerRadius, bottom);
            path.lineTo(right - cornerRadius, bottom);
            path.quadTo(right, bottom, right, bottom - cornerRadius);
            path.lineTo(right, top);
            path.closeSubpath();
        }

        painter->translate(rect.topLeft());

        QColor fillColor = m_whiteKeyStateColors[m_controller->keyState(key)];

        painter->fillPath(path, fillColor);

        if (key % 12 == 0) {
            // Draw octave label
            qreal availableHeight = m_whiteKeyHeight - m_blackKeyHeight;
            qreal requiredHeight = m_octaveLabelsFont.pixelSize();
            qreal bottomOffsetToCenterTextInAvailableSpace = (availableHeight - requiredHeight) / 2;

            constexpr qreal maxBottomOffset = 8.0;
            constexpr qreal defaultBottomOffset = 8.0;

            qreal bottomOffset = std::min({
                bottomOffsetToCenterTextInAvailableSpace,
                defaultBottomOffset * m_keyWidthScaling,
                maxBottomOffset
            });

            int octaveNumber = (key / 12) - 1;

            QString octaveLabel = "C" + QString::number(octaveNumber);
            QRect octaveLabelRect(left, top, right - left, bottom - top - bottomOffset);

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

    QRectF backgroundRect;
    QPainterPath topPiecePath;
    QPainterPath bottomPiecePath;

    for (auto [key, rect] : m_blackKeyRects) {
        if (!viewport.intersects(rect)) {
            continue;
        }

        if (backgroundRect.isEmpty()) {
            qreal blackKeyInset = 2.0 * m_keyWidthScaling;
            qreal cornerRadius = blackKeyInset;
            qreal bottomPieceHeight = 8.0 * m_keyWidthScaling;

            // Make background rect
            // Adjust the top, to make sure that the rect fully covers the white keys,
            // but does not extend beyond the background of the whole keyboard (which
            // theoretically shouldn't happen, but in practice does, probably due to
            // antialiasing)
            backgroundRect.setSize(rect.size());
            backgroundRect.setTop(0.5 * m_spacing);

            // Make top piece
            qreal top = m_spacing + 0.5 * blackKeyInset,
                  left = blackKeyInset,
                  right = rect.width() - blackKeyInset,
                  bottom = rect.height() - blackKeyInset - bottomPieceHeight - 0.5 * blackKeyInset;

            qreal center = rect.width() / 2;

            topPiecePath.moveTo(left, top);
            topPiecePath.lineTo(left, bottom - cornerRadius);
            topPiecePath.quadTo(left, bottom, center, bottom);
            topPiecePath.quadTo(right, bottom, right, bottom - cornerRadius);
            topPiecePath.lineTo(right, top);
            topPiecePath.closeSubpath();

            topPieceGradient.setColorAt(0.0, backgroundColor);
            topPieceGradient.setStart(0.0, top);
            topPieceGradient.setFinalStop(0.0, bottom);

            // Make bottom piece
            top = rect.height() - blackKeyInset - bottomPieceHeight;
            bottom = rect.height() - blackKeyInset;

            bottomPiecePath.moveTo(left, top + cornerRadius);
            bottomPiecePath.quadTo(left, top, center, top);
            bottomPiecePath.quadTo(right, top, right, top + cornerRadius);
            bottomPiecePath.lineTo(right, bottom);
            bottomPiecePath.lineTo(left, bottom);
            bottomPiecePath.closeSubpath();

            bottomPieceGradient.setColorAt(1.0, backgroundColor);
            bottomPieceGradient.setStart(0.0, top);
            bottomPieceGradient.setFinalStop(0.0, bottom);
        }

        topPieceGradient.setColorAt(1.0, m_blackKeyTopPieceStateColors[m_controller->keyState(key)]);
        bottomPieceGradient.setColorAt(0.0, m_blackKeyBottomPieceStateColors[m_controller->keyState(key)]);

        painter->translate(rect.topLeft());
        painter->fillRect(backgroundRect, backgroundColor);
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
        m_lowestKey = 21;
        m_numberOfKeys = 88;
        break;
    case 61:
        m_lowestKey = 36;
        m_numberOfKeys = 61;
        break;
    case 49:
        m_lowestKey = 36;
        m_numberOfKeys = 49;
        break;
    case 25:
        m_lowestKey = 48;
        m_numberOfKeys = 25;
        break;
    default:
        return;
    }

    emit numberOfKeysChanged();

    if (!m_isInitialized) {
        return;
    }

    calculateKeyRects();
    update();
}

void PianoKeyboardView::moveCanvas(qreal dx)
{
    if (!m_isInitialized) {
        return;
    }

    if (qFuzzyIsNull(dx)) {
        return;
    }

    setScrollOffset(m_scrollOffset + dx);
}

void PianoKeyboardView::setScrollOffset(qreal offset)
{
    if (qFuzzyCompare(m_scrollOffset, offset)) {
        return;
    }

    m_scrollOffset = offset;
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
    if (!m_isInitialized) {
        return;
    }

    qreal newScaling = std::clamp(scaling, SMALL_KEY_WIDTH_SCALING, LARGE_KEY_WIDTH_SCALING);
    qreal correctedFactor = newScaling / m_keyWidthScaling;

    if (qFuzzyCompare(correctedFactor, 1.0)) {
        return;
    }

    m_keyWidthScaling = newScaling;
    m_scrollOffset *= correctedFactor;
    m_scrollOffset += x * (1 - correctedFactor);

    emit keyWidthScalingChanged();
    calculateKeyRects();
    update();
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
    if (!m_isInitialized) {
        return;
    }

    setScrollOffset(-position * m_keysAreaRect.width());
}

std::optional<piano_key_t> PianoKeyboardView::keyAt(const QPointF& position) const
{
    QPointF correctedPosition = position - m_keysAreaRect.topLeft();

    for (auto [key, rect] : m_blackKeyRects) {
        if (rect.contains(correctedPosition)) {
            return key;
        }
    }

    for (auto [key, rect] : m_whiteKeyRects) {
        if (rect.contains(correctedPosition)) {
            return key;
        }
    }

    return std::nullopt;
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
        // For every pixel being scrolled, zoom by a factor or 1.002 (value is found empirically)
        constexpr qreal zoomSpeed = 1.002;
        qreal abs = abs2d(delta.x(), delta.y());
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

void PianoKeyboardView::mousePressEvent(QMouseEvent* event)
{
    m_controller->setPressedKey(keyAt(event->pos()));
}

void PianoKeyboardView::mouseMoveEvent(QMouseEvent* event)
{
    m_controller->setPressedKey(keyAt(event->pos()));
}

void PianoKeyboardView::mouseReleaseEvent(QMouseEvent*)
{
    m_controller->setPressedKey(std::nullopt);
}
