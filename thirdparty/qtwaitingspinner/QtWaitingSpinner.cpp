/* Original Work Copyright (c) 2012-2014 Alexander Turkin
   Modified 2014 by William Hallatt

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <cmath>
#include <algorithm>

#include <QPainter>
#include <QTimer>

#include "QtWaitingSpinner.h"

/*----------------------------------------------------------------------------*/

// Defaults
const QColor c_color(Qt::black);
const qreal c_roundness(70.0);
const qreal c_minTrailOpacity(15.0);
const qreal c_trailFadePercentage(70.0);
const int c_lines(12);
const int c_lineLength(10);
const int c_lineWidth(5);
const int c_innerRadius(10);
const int c_revPerSec(1);

/*----------------------------------------------------------------------------*/

QtWaitingSpinner::QtWaitingSpinner(QWidget *parent)
    : QWidget(parent),

      // Configurable settings.
      m_color(c_color), m_roundness(c_roundness),
      m_minTrailOpacity(c_minTrailOpacity),
      m_trailFadePercentage(c_trailFadePercentage), m_revPerSec(c_revPerSec),
      m_numberOfLines(c_lines), m_lineLength(c_lineLength + c_lineWidth),
      m_lineWidth(c_lineWidth), m_innerRadius(c_innerRadius),

      // Other
      m_timer(NULL), m_parent(parent), m_centreOnParent(false),
      m_currentCounter(0), m_isSpinning(false) {
  initialise();
}

/*----------------------------------------------------------------------------*/

QtWaitingSpinner::QtWaitingSpinner(Qt::WindowModality modality, QWidget *parent,
                                   bool centreOnParent)
    : QWidget(parent, Qt::Dialog | Qt::FramelessWindowHint),

      // Configurable settings.
      m_color(c_color), m_roundness(c_roundness),
      m_minTrailOpacity(c_minTrailOpacity),
      m_trailFadePercentage(c_trailFadePercentage), m_revPerSec(c_revPerSec),
      m_numberOfLines(c_lines), m_lineLength(c_lineLength + c_lineWidth),
      m_lineWidth(c_lineWidth), m_innerRadius(c_innerRadius),

      // Other
      m_timer(NULL), m_parent(parent), m_centreOnParent(centreOnParent),
      m_currentCounter(0) {
  initialise();

  // We need to set the window modality AFTER we've hidden the
  // widget for the first time since changing this property while
  // the widget is visible has no effect.
  this->setWindowModality(modality);
  this->setAttribute(Qt::WA_TranslucentBackground);
}

/*----------------------------------------------------------------------------*/

void QtWaitingSpinner::initialise() {
  m_timer = new QTimer(this);
  connect(m_timer, SIGNAL(timeout()), this, SLOT(rotate()));
  updateSize();
  updateTimer();
  this->hide();
}

/*----------------------------------------------------------------------------*/

void QtWaitingSpinner::paintEvent(QPaintEvent * /*ev*/) {
  QPainter painter(this);
  painter.fillRect(this->rect(), Qt::transparent);
  painter.setRenderHint(QPainter::Antialiasing, true);

  if (m_currentCounter >= m_numberOfLines) {
    m_currentCounter = 0;
  }
  painter.setPen(Qt::NoPen);
  for (int i = 0; i < m_numberOfLines; ++i) {
    painter.save();
    painter.translate(m_innerRadius + m_lineLength,
                      m_innerRadius + m_lineLength);
    qreal rotateAngle =
        static_cast<qreal>(360 * i) / static_cast<qreal>(m_numberOfLines);
    painter.rotate(rotateAngle);
    painter.translate(m_innerRadius, 0);
    int distance =
        lineCountDistanceFromPrimary(i, m_currentCounter, m_numberOfLines);
    QColor color =
        currentLineColor(distance, m_numberOfLines, m_trailFadePercentage,
                         m_minTrailOpacity, m_color);
    painter.setBrush(color);
    // TODO improve the way rounded rect is painted
    painter.drawRoundedRect(
        QRect(0, -m_lineWidth / 2, m_lineLength, m_lineWidth), m_roundness,
        m_roundness, Qt::RelativeSize);
    painter.restore();
  }
}

/*----------------------------------------------------------------------------*/

void QtWaitingSpinner::start() {
  updatePosition();
  m_isSpinning = true;
  this->show();
  if (!m_timer->isActive()) {
    m_timer->start();
    m_currentCounter = 0;
  }
}

/*----------------------------------------------------------------------------*/

void QtWaitingSpinner::stop() {
  m_isSpinning = false;
  this->hide();
  if (m_timer->isActive()) {
    m_timer->stop();
    m_currentCounter = 0;
  }
}

/*----------------------------------------------------------------------------*/

void QtWaitingSpinner::setNumberOfLines(int lines) {
  m_numberOfLines = lines;
  m_currentCounter = 0;
  updateTimer();
}

/*----------------------------------------------------------------------------*/

void QtWaitingSpinner::setLineLength(int length) {
  m_lineLength = length;
  updateSize();
}

/*----------------------------------------------------------------------------*/

void QtWaitingSpinner::setLineWidth(int width) {
  m_lineWidth = width;
  updateSize();
}

/*----------------------------------------------------------------------------*/

void QtWaitingSpinner::setInnerRadius(int radius) {
  m_innerRadius = radius;
  updateSize();
}

/*----------------------------------------------------------------------------*/

bool QtWaitingSpinner::isSpinning() const { return m_isSpinning; }

/*----------------------------------------------------------------------------*/

void QtWaitingSpinner::setRoundness(qreal roundness) {
  m_roundness = std::max(0.0, std::min(100.0, roundness));
}

/*----------------------------------------------------------------------------*/

void QtWaitingSpinner::setColor(QColor color) { m_color = color; }

/*----------------------------------------------------------------------------*/

void QtWaitingSpinner::setRevolutionsPerSecond(int rps) {
  m_revPerSec = rps;
  updateTimer();
}

/*----------------------------------------------------------------------------*/

void QtWaitingSpinner::setTrailFadePercentage(qreal trail) {
  m_trailFadePercentage = trail;
}

/*----------------------------------------------------------------------------*/

void QtWaitingSpinner::setMinimumTrailOpacity(qreal minOpacity) {
  m_minTrailOpacity = minOpacity;
}

/*----------------------------------------------------------------------------*/

void QtWaitingSpinner::rotate() {
  ++m_currentCounter;
  if (m_currentCounter >= m_numberOfLines) {
    m_currentCounter = 0;
  }
  update();
}

/*----------------------------------------------------------------------------*/

void QtWaitingSpinner::updateSize() {
  int size = (m_innerRadius + m_lineLength) * 2;
  setFixedSize(size, size);
}

/*----------------------------------------------------------------------------*/

void QtWaitingSpinner::updateTimer() {
  m_timer->setInterval(calculateTimerInterval(m_numberOfLines, m_revPerSec));
}

/*----------------------------------------------------------------------------*/

void QtWaitingSpinner::updatePosition() {
  if (m_parent && m_centreOnParent) {
    this->move(m_parent->frameGeometry().topLeft() + m_parent->rect().center() -
               this->rect().center());
  }
}

/*----------------------------------------------------------------------------*/

int QtWaitingSpinner::calculateTimerInterval(int lines, int speed) {
  return 1000 / (lines * speed);
}

/*----------------------------------------------------------------------------*/

int QtWaitingSpinner::lineCountDistanceFromPrimary(int current, int primary,
                                                   int totalNrOfLines) {
  int distance = primary - current;
  if (distance < 0) {
    distance += totalNrOfLines;
  }
  return distance;
}

/*----------------------------------------------------------------------------*/

QColor QtWaitingSpinner::currentLineColor(int countDistance, int totalNrOfLines,
                                          qreal trailFadePerc, qreal minOpacity,
                                          QColor color) {
  if (countDistance == 0) {
    return color;
  }
  const qreal minAlphaF = minOpacity / 100.0;
  int distanceThreshold =
      static_cast<int>(ceil((totalNrOfLines - 1) * trailFadePerc / 100.0));
  if (countDistance > distanceThreshold) {
    color.setAlphaF(minAlphaF);
  } else {
    qreal alphaDiff = color.alphaF() - minAlphaF;
    qreal gradient = alphaDiff / static_cast<qreal>(distanceThreshold + 1);
    qreal resultAlpha = color.alphaF() - gradient * countDistance;

    // If alpha is out of bounds, clip it.
    resultAlpha = std::min(1.0, std::max(0.0, resultAlpha));
    color.setAlphaF(resultAlpha);
  }
  return color;
}

/*----------------------------------------------------------------------------*/
