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
#include <QGuiApplication>
#include <QScreen>
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
    // m_controller->glissandoEndNotesChanged().onNotify(this, [this]() {
        
    // });
    // m_controller->glissandoTickChanged().onNotify(this, [this]() {
        
    // });
    m_controller->keyStatesChanged().onNotify(this, [this]() {
        // LOGALEX() << "m_controller->keyStatesChanged().onNotify call back";
        updateKeyStateColors();
        update();
    });

    m_controller->playbackKeyStatesChanged().onNotify(this, [this]() {
        // LOGALEX() << "m_controller->playbackKeyStatesChanged().onNotify call back";
        updatePlaybackKeyStateColors();
        update();
    });

    m_controller->clefKeySigsKeysChanged().onNotify(this, [this]() {
        m_clefKeySigsKeys.clear();
        for (auto key : m_controller->clefKeySigsKeys()) {
            m_clefKeySigsKeys.insert(key);
        }
        m_controller->clearClefKeySigsKeys();
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

bool PianoKeyboardView::containsKey(uint keyIndex, piano_key_t key) 
{
    // offset
    // G#8va    120x1
    // G#15va   120x2
    // Gb8va    120x3
    // F#8va    120x4
    // Fb8va    120x5
    if (keyIndex < 120) {
        for (uint i = 8 * keyIndex; i < 8 * (keyIndex + 1); i++) {
            const piano_key_t& _key = m_clefKeySigs[i];
            if (_key == key) {
                return true;
            }
        }
        return false;
    } 

    if (keyIndex / 120 == 1) {
        keyIndex -= 120;
        for (uint i = 8 * keyIndex; i < 8 * (keyIndex + 1); i++) {
            if (m_clefKeySigs[i] + 12 == key) {
                return true;
            }
        }
        return false;
    } 
    if (keyIndex / 120 == 2) {
        keyIndex -= 240;
        for (uint i = 8 * keyIndex; i < 8 * (keyIndex + 1); i++) {
            if (m_clefKeySigs[i] + 24 == key) {
                return true;
            }
        }
        return false;
    } 
    if (keyIndex / 120 == 3) {
        keyIndex -= 360;
        for (uint i = 8 * keyIndex; i < 8 * (keyIndex + 1); i++) {
            if (m_clefKeySigs[i] - 12 == key) {
                return true;
            }
        }
        return false;
    } 
    // if (keyIndex / 120 == 4) {
    //     keyIndex -= 480;
    //     for (uint i = 8 * keyIndex; i < 8 * (keyIndex + 1); i++) {
    //         if (m_clefKeySigs[i] + 12 == key) {
    //             return true;
    //         }
    //     }
    //     return false;
    // } 
    // if (keyIndex / 120 == 5) {
    //     keyIndex -= 600;
    //     for (uint i = 8 * keyIndex; i < 8 * (keyIndex + 1); i++) {
    //         if (m_clefKeySigs[i] - 12 == key) {
    //             return true;
    //         }
    //     }
    //     return false;
    // }
    return false;
}

void PianoKeyboardView::adjustKeysAreaPosition()
{
    TRACEFUNC;

    qreal screenWidth = QGuiApplication::primaryScreen()->geometry().width();
    qreal screenHeight = QGuiApplication::primaryScreen()->geometry().height();

    qreal _width = width();
    qreal _height = height();
    if (_width > screenWidth) {
        _width = screenWidth;
    }
    if (_height > screenHeight) {
        _height = screenHeight;
    }

    qreal keysAreaTop = (_height - m_keysAreaRect.height()) / 2;

    qreal minScrollOffset = std::min(_width - m_keysAreaRect.width(), (_width - m_keysAreaRect.width()) / 2);
    qreal maxScrollOffset = std::max(0.0, (_width - m_keysAreaRect.width()) / 2);
    m_scrollOffset = std::clamp(m_scrollOffset, minScrollOffset, maxScrollOffset);

    m_keysAreaRect.moveTo(QPointF(m_scrollOffset, keysAreaTop));

    updateScrollBar();
}

void PianoKeyboardView::checkResponseKeyOccluded() {
    if (!m_controller->isPlaying()) {
        return;
    }
    if (m_check_rects.empty()) {
        return;
    }

    qreal screenWidth = QGuiApplication::primaryScreen()->geometry().width();
    qreal screenHeight = QGuiApplication::primaryScreen()->geometry().height();

    qreal _width = width();
    qreal _height = height();
    if (_width > screenWidth) {
        _width = screenWidth;
    }
    if (_height > screenHeight) {
        _height = screenHeight;
    }
    
    // piano_key_t minKey =  m_check_rects.begin()->first;
    QRectF minRect = m_check_rects.begin()->second;
    // piano_key_t maxKey =  m_check_rects.begin()->first;
    QRectF maxRect = m_check_rects.begin()->second;
    for (const auto [key, rect]: m_check_rects) {
        if (rect.x() < minRect.x()) {
            // minKey = key;
            minRect = rect;
        } else if (rect.x() > maxRect.x()) {
            // maxKey = key;
            maxRect = rect;
        }
    }
    qreal keysAreaTop = (_height - m_keysAreaRect.height()) / 2;
    if (_width < m_keysAreaRect.width()) {
        if (m_scrollOffset + minRect.x() < 0) {
            if (minRect.width() <= minRect.x()) {
                m_scrollOffset = -minRect.x() + minRect.width();
            } else {
                m_scrollOffset = -minRect.x();
            }
            m_keysAreaRect.moveTo(QPointF(m_scrollOffset, keysAreaTop));
            updateScrollBar();
        } else if (m_scrollOffset + maxRect.x() + maxRect.width() > _width) {
            qreal offset = m_scrollOffset + maxRect.x() + maxRect.width() - _width;
            if (maxRect.x() + 2 * maxRect.width() <= m_keysAreaRect.width()) {
                offset += maxRect.width();
            } 
            m_scrollOffset -= offset;
            m_keysAreaRect.moveTo(QPointF(m_scrollOffset, keysAreaTop));
            updateScrollBar();
        } 
    }
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

void PianoKeyboardView::updatePlaybackKeyStateColors() 
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
    QColor whiteKeyRightHandBaseColor(255, 0, 0);
    QColor whiteKeyRightHandChangeColor(255, 100, 0);
    QColor whiteKeyPlayColor = whiteKeyRightHandBaseColor;
    if (playbackkey_state_base) {
        whiteKeyPlayColor = whiteKeyRightHandChangeColor;
    }
    m_whiteKeyStateColors[KeyState::RightHand] = mixedColors(Qt::white, whiteKeyPlayColor, 1.0);

    QColor whiteKeyTrillColor(255, 255, 0);
    m_whiteKeyStateColors[KeyState::Trill] = mixedColors(Qt::white, whiteKeyTrillColor, 1.0);

    QColor whiteKeyArpeggioColor(255, 255, 0);
    m_whiteKeyStateColors[KeyState::Arpeggio] = mixedColors(Qt::white, whiteKeyArpeggioColor, 1.0);

    QColor whiteKeyGlissandoColor(255, 255, 0);
    m_whiteKeyStateColors[KeyState::Glissando] = mixedColors(Qt::white, whiteKeyGlissandoColor, 1.0);

    QColor blackKeyTopPieceBaseColor(78, 78, 78);
    m_blackKeyTopPieceStateColors[KeyState::None] = blackKeyTopPieceBaseColor;
    m_blackKeyTopPieceStateColors[KeyState::OtherInSelectedChord] = mixedColors(blackKeyTopPieceBaseColor, accentColor, 0.4);
    m_blackKeyTopPieceStateColors[KeyState::Selected] = mixedColors(blackKeyTopPieceBaseColor, accentColor, 0.8);
    m_blackKeyTopPieceStateColors[KeyState::Played] = mixedColors(blackKeyTopPieceBaseColor, accentColor, 1.0);
    QColor blackKeyRightHandTopPieceBaseColor(255, 0, 0);
    QColor blackKeyRightHandTopPieceChangeColor(255, 100, 0);
    QColor balckKeyPlayTopPieceColor = blackKeyRightHandTopPieceBaseColor;
    if (playbackkey_state_base) {
        balckKeyPlayTopPieceColor = blackKeyRightHandTopPieceChangeColor;
    }
    m_blackKeyTopPieceStateColors[KeyState::RightHand] = mixedColors(blackKeyTopPieceBaseColor, balckKeyPlayTopPieceColor, 1.0);

    QColor blackKeyTrillTopPieceColor(255, 255, 0);
    m_blackKeyTopPieceStateColors[KeyState::Trill] = mixedColors(blackKeyTopPieceBaseColor, blackKeyTrillTopPieceColor, 1.0);

    QColor blackKeyArpeggioTopPieceColor(255, 255, 0);
    m_blackKeyTopPieceStateColors[KeyState::Arpeggio] = mixedColors(blackKeyTopPieceBaseColor, blackKeyArpeggioTopPieceColor, 1.0);

    QColor blackKeyGlissandoTopPieceColor(255, 255, 0);
    m_blackKeyTopPieceStateColors[KeyState::Glissando] = mixedColors(blackKeyTopPieceBaseColor, blackKeyGlissandoTopPieceColor, 1.0);

    QColor blackKeyBottomPieceBaseColor(56, 56, 58);
    m_blackKeyBottomPieceStateColors[KeyState::None] = blackKeyBottomPieceBaseColor;
    m_blackKeyBottomPieceStateColors[KeyState::OtherInSelectedChord] = mixedColors(blackKeyBottomPieceBaseColor, accentColor, 0.4);
    m_blackKeyBottomPieceStateColors[KeyState::Selected] = mixedColors(blackKeyBottomPieceBaseColor, accentColor, 0.8);
    m_blackKeyBottomPieceStateColors[KeyState::Played] = mixedColors(blackKeyBottomPieceBaseColor, accentColor, 1.0);
    QColor blackKeyRightHandBottomPieceBaseColor(255, 0, 0);
    QColor blackKeyRightHandBottomPieceChangeColor(255, 100, 0);
    QColor balckKeyPlayBottomPieceColor = blackKeyRightHandBottomPieceBaseColor;
    if (playbackkey_state_base) {
        balckKeyPlayBottomPieceColor = blackKeyRightHandBottomPieceChangeColor;
    }
    m_blackKeyBottomPieceStateColors[KeyState::RightHand] = mixedColors(blackKeyBottomPieceBaseColor, balckKeyPlayBottomPieceColor, 1.0);

    QColor blackKeyTrillBottomPieceColor(255, 255, 0);
    m_blackKeyBottomPieceStateColors[KeyState::Trill] = mixedColors(blackKeyBottomPieceBaseColor, blackKeyTrillBottomPieceColor, 1.0);

    QColor blackKeyArpeggioBottomPieceColor(255, 255, 0);
    m_blackKeyBottomPieceStateColors[KeyState::Arpeggio] = mixedColors(blackKeyBottomPieceBaseColor, blackKeyArpeggioBottomPieceColor, 1.0);

    QColor blackKeyGlissandoBottomPieceColor(255, 255, 0);
    m_blackKeyBottomPieceStateColors[KeyState::Glissando] = mixedColors(blackKeyBottomPieceBaseColor, blackKeyGlissandoBottomPieceColor, 1.0);

    if (!preRectUnchanged()) {
        playbackkey_state_base = !playbackkey_state_base;
    }
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

    shiftCheckRects();

    paintWhiteKeys(painter, viewport);
    paintBlackKeys(painter, viewport);

    checkResponseKeyOccluded();
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

        if (m_controller->keyState(key) == KeyState::None) {
            if (m_controller->isPlaying()) {
                for (const uint& keyIndex : m_clefKeySigsKeys) {
                    if (containsKey(keyIndex, key)) {
                        fillColor = Qt::green;
                        // m_check_rects.insert({ key, rect });
                    }
                }
            }
        }

        if (!m_controller->playbackKeyStatesEmpty()) {
            if (m_controller->playbackKeyState(key) == KeyState::RightHand) {
                fillColor = m_whiteKeyStateColors[m_controller->playbackKeyState(key)];
                m_check_rects.insert({ key, rect });
            }
        }

        if (m_controller->isPlaying()) {
            if (m_controller->trillKeyState(key) != KeyState::None) {
                fillColor = m_whiteKeyStateColors[m_controller->trillKeyState(key)];
                m_check_rects.insert({ key, rect });
            }

            if (m_controller->arpeggioKeyState(key) != KeyState::None) {
                fillColor = m_whiteKeyStateColors[m_controller->arpeggioKeyState(key)];
                m_check_rects.insert({ key, rect });
            }

            if (m_controller->glissandoKeyState(key) != KeyState::None) {
                fillColor = m_whiteKeyStateColors[m_controller->glissandoKeyState(key)];
                m_check_rects.insert({ key, rect });
            }
        }

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

        if (m_controller->keyState(key) == KeyState::None) {
            if (m_controller->isPlaying()) {
                for (const uint& keyIndex : m_clefKeySigsKeys) {
                    if (containsKey(keyIndex, key)) {
                        topPieceGradient.setColorAt(1.0, Qt::green);
                        bottomPieceGradient.setColorAt(0.0, Qt::green);
                        // m_check_rects.insert({ key, rect });
                    }
                }
            }
        }

        if (!m_controller->playbackKeyStatesEmpty()) {
            if (m_controller->playbackKeyState(key) == KeyState::RightHand) {
                topPieceGradient.setColorAt(1.0, m_blackKeyTopPieceStateColors[m_controller->playbackKeyState(key)]);
                bottomPieceGradient.setColorAt(0.0, m_blackKeyBottomPieceStateColors[m_controller->playbackKeyState(key)]);
                m_check_rects.insert({ key, rect });
            }
        }

        if (m_controller->isPlaying()) {
            if (m_controller->trillKeyState(key) != KeyState::None) {
                topPieceGradient.setColorAt(1.0, m_blackKeyTopPieceStateColors[m_controller->trillKeyState(key)]);
                bottomPieceGradient.setColorAt(0.0, m_blackKeyBottomPieceStateColors[m_controller->trillKeyState(key)]);
                m_check_rects.insert({ key, rect });
            }

            if (m_controller->arpeggioKeyState(key) != KeyState::None) {
                topPieceGradient.setColorAt(1.0, m_blackKeyTopPieceStateColors[m_controller->arpeggioKeyState(key)]);
                bottomPieceGradient.setColorAt(0.0, m_blackKeyBottomPieceStateColors[m_controller->arpeggioKeyState(key)]);
                m_check_rects.insert({ key, rect });
            }

            if (m_controller->glissandoKeyState(key) != KeyState::None) {
                topPieceGradient.setColorAt(1.0, m_blackKeyTopPieceStateColors[m_controller->glissandoKeyState(key)]);
                bottomPieceGradient.setColorAt(0.0, m_blackKeyBottomPieceStateColors[m_controller->glissandoKeyState(key)]);
                m_check_rects.insert({ key, rect });
            }
        }

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
    qreal screenWidth = QGuiApplication::primaryScreen()->geometry().width();

    qreal _width = width();
    if (_width > screenWidth) {
        _width = screenWidth;
    }

    qreal newPosition = -m_scrollOffset / m_keysAreaRect.width();
    qreal newSize = _width / m_keysAreaRect.width();

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

void PianoKeyboardView::shiftCheckRects() 
{
    m_pre_check_rects.swap(m_check_rects);
    m_check_rects.clear();
}

bool PianoKeyboardView::preRectUnchanged() 
{
    return m_check_rects == m_pre_check_rects;
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
