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
#ifndef MU_NOTATION_PIANOKEYBOARDVIEW_H
#define MU_NOTATION_PIANOKEYBOARDVIEW_H

#include "async/asyncable.h"

#include "uicomponents/view/quickpaintedview.h"
#include "modularity/ioc.h"
#include "inotationconfiguration.h"
#include "ui/iuiconfiguration.h"

#include "pianokeyboardtypes.h"

namespace mu::notation {
class PianoKeyboardController;
class PianoKeyboardView : public muse::uicomponents::QuickPaintedView, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(int numberOfKeys READ numberOfKeys WRITE setNumberOfKeys NOTIFY numberOfKeysChanged)
    Q_PROPERTY(qreal keyWidthScaling READ keyWidthScaling WRITE setScaling NOTIFY keyWidthScalingChanged)

    Q_PROPERTY(qreal scrollBarPosition READ scrollBarPosition WRITE setScrollBarPosition NOTIFY scrollBarChanged)
    Q_PROPERTY(qreal scrollBarSize READ scrollBarSize NOTIFY scrollBarChanged)

    muse::Inject<INotationConfiguration> configuration = { this };
    muse::Inject<muse::ui::IUiConfiguration> uiConfiguration = { this };

public:
    explicit PianoKeyboardView(QQuickItem* parent = nullptr);
    ~PianoKeyboardView() override;

    Q_INVOKABLE void init();

    void paint(QPainter* painter) override;

    int numberOfKeys() const;
    void setNumberOfKeys(int number);

    qreal keyWidthScaling() const;
    Q_INVOKABLE void scale(qreal factor, qreal x);
    void setScaling(qreal scaling, qreal x = 0.0);

    qreal scrollBarPosition() const;
    qreal scrollBarSize() const;
    void setScrollBarPosition(qreal position);

signals:
    void numberOfKeysChanged();
    void keyWidthScalingChanged();

    void scrollBarChanged();

private:
    void calculateKeyRects();
    void adjustKeysAreaPosition();
    void determineOctaveLabelsFont();
    void updateKeyStateColors();
    void updatePlaybackKeyStateColors();

    void paintBackground(QPainter* painter);

    void paintWhiteKeys(QPainter* painter, const QRectF& viewport);
    void paintBlackKeys(QPainter* painter, const QRectF& viewport);

    void moveCanvas(qreal dx);
    void setScrollOffset(qreal offset);
    void updateScrollBar();

    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

    std::optional<piano_key_t> keyAt(const QPointF& position) const;

    static constexpr piano_key_t MIN_KEY = 0;
    static constexpr piano_key_t MAX_NUM_KEYS = 128;

    bool m_isInitialized = false;

    piano_key_t m_lowestKey = MIN_KEY;
    piano_key_t m_numberOfKeys = MAX_NUM_KEYS;

    PianoKeyboardController* m_controller = nullptr;

    QRectF m_keysAreaRect;
    std::map<piano_key_t, QRectF> m_blackKeyRects;
    std::map<piano_key_t, QRectF> m_whiteKeyRects;

    QFont m_octaveLabelsFont;

    std::map<KeyState, QColor> m_whiteKeyStateColors;
    std::map<KeyState, QColor> m_blackKeyTopPieceStateColors;
    std::map<KeyState, QColor> m_blackKeyBottomPieceStateColors;

    qreal m_keyWidthScaling = 1.0;
    qreal m_scrollOffset = 0.0;
    qreal m_spacing = 0.0;

    qreal m_whiteKeyHeight = 0.0;
    qreal m_blackKeyHeight = 0.0;

    qreal m_scrollBarPosition = 0.0;
    qreal m_scrollBarSize = 0.0;
};
}

#endif // MU_NOTATION_PIANOKEYBOARDVIEW_H
