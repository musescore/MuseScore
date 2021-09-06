/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#ifndef MU_PIANOROLL_PIANOROLLAUTOMATIONEDITOR_H
#define MU_PIANOROLL_PIANOROLLAUTOMATIONEDITOR_H

#include <QQuickPaintedItem>
#include <QPixmap>
#include <QColor>

#include "async/asyncable.h"
#include "context/iglobalcontext.h"
#include "pianoroll/ipianorollcontroller.h"
#include "audio/iplayback.h"

namespace mu::pianoroll {


class PianorollAutomationEditor : public QQuickPaintedItem, public async::Asyncable
{
    Q_OBJECT

    INJECT(pianoroll, context::IGlobalContext, globalContext)
    INJECT(pianoroll, IPianorollController, controller)
    INJECT(playback, audio::IPlayback, playback)

    Q_PROPERTY(double wholeNoteWidth READ wholeNoteWidth WRITE setWholeNoteWidth NOTIFY wholeNoteWidthChanged)
    Q_PROPERTY(double centerX READ centerX WRITE setCenterX NOTIFY centerXChanged)
    Q_PROPERTY(double displayObjectWidth READ displayObjectWidth WRITE setDisplayObjectWidth NOTIFY displayObjectWidthChanged)
    Q_PROPERTY(int tuplet READ tuplet WRITE setTuplet NOTIFY tupletChanged)
    Q_PROPERTY(int subdivision READ subdivision WRITE setSubdivision NOTIFY subdivisionChanged)

public:
    enum class AutomationAttribute { VELOCITY, EXPRESSION, PAN };
    Q_ENUM(AutomationAttribute)

    PianorollAutomationEditor(QQuickItem* parent = nullptr);

    Q_INVOKABLE void load();

    double wholeNoteWidth() const { return m_wholeNoteWidth; }
    void setWholeNoteWidth(double value);
    double centerX() const { return m_centerX; }
    void setCenterX(double value);
    double displayObjectWidth() const { return m_displayObjectWidth; }
    void setDisplayObjectWidth(double value);
    int tuplet() const { return m_tuplet; }
    void setTuplet(int value);
    int subdivision() const { return m_subdivision; }
    void setSubdivision(int value);
    Ms::Fraction playbackPosition() { return m_playbackPosition; }
    void setPlaybackPosition(Ms::Fraction value);
    AutomationAttribute automationAttribute() { return m_automationAttribute; }
    void setAutomationAttribute(AutomationAttribute value);

    void paint(QPainter*) override;

    int wholeNoteToPixelX(Ms::Fraction tick) const { return wholeNoteToPixelX(tick.numerator() / (double)tick.denominator()); }
    int wholeNoteToPixelX(double tick) const;
    double pixelXToWholeNote(int pixelX) const;

    Ms::Score* score();

signals:
    void wholeNoteWidthChanged();
    void centerXChanged();
    void displayObjectWidthChanged();
    void playbackPositionChanged();
    void automationAttributeChanged();
    void tupletChanged();
    void subdivisionChanged();


private:
    void onNotationChanged();
    void onCurrentNotationChanged();
    void updateBoundingSize();

    AutomationAttribute m_automationAttribute = AutomationAttribute::VELOCITY;

    double m_centerX = 0;  //fraction of note grid camera is focused on
    double m_displayObjectWidth = 0;  //Set to note grid in pixels
    double m_wholeNoteWidth;

    double m_marginY = 8;

    int m_tuplet = 1;
    int m_subdivision = 0;

    Ms::Fraction m_playbackPosition;

    QColor m_colorBackground = Qt::gray;
    QColor m_colorGridBackground = QColor(0xdddddd);
    QColor m_colorPlaybackLine = QColor(0xff0000);
    QColor m_colorGridLine = QColor(0xa2a2a6);
    QColor m_colorText = Qt::black;
};

}

#endif // MU_PIANOROLL_PIANOROLLAUTOMATIONEDITOR_H
