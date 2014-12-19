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
#ifndef QTWAITINGSPINNER_H
#define QTWAITINGSPINNER_H

#include <QWidget>

#include <QTimer>
#include <QColor>

class QtWaitingSpinner : public QWidget {
  Q_OBJECT
public:
  /*! Constructor for "standard" widget behaviour - use this
   * constructor if you wish to, e.g. embed your widget in another. */
  QtWaitingSpinner(QWidget *parent = 0);

  /*! Constructor - use this constructor to automatically create a modal
   * ("blocking") spinner on top of the calling widget/window.  If a valid
   * parent widget is provided, "centreOnParent" will ensure that
   * QtWaitingSpinner automatically centres itself on it, if not,
   * "centreOnParent" is ignored. */
  QtWaitingSpinner(Qt::WindowModality modality, QWidget *parent = 0,
                   bool centreOnParent = true);

public Q_SLOTS:
  void start();
  void stop();

public:
  void setColor(QColor color);
  void setRoundness(qreal roundness);
  void setMinimumTrailOpacity(qreal minOpacity);
  void setTrailFadePercentage(qreal trail);
  void setRevolutionsPerSecond(int rps);
  void setNumberOfLines(int lines);
  void setLineLength(int length);
  void setLineWidth(int width);
  void setInnerRadius(int radius);

  bool isSpinning() const;

private Q_SLOTS:
  void rotate();

protected:
  void paintEvent(QPaintEvent *ev);

private:
  static int calculateTimerInterval(int lines, int speed);
  static int lineCountDistanceFromPrimary(int current, int primary,
                                          int totalNrOfLines);
  static QColor currentLineColor(int distance, int totalNrOfLines,
                                 qreal trailFadePerc, qreal minOpacity,
                                 QColor color);

  void initialise();
  void updateSize();
  void updateTimer();
  void updatePosition();

private:
  // Configurable settings.
  QColor m_color;
  qreal m_roundness; // 0..100
  qreal m_minTrailOpacity;
  qreal m_trailFadePercentage;
  int m_revPerSec; // revolutions per second
  int m_numberOfLines;
  int m_lineLength;
  int m_lineWidth;
  int m_innerRadius;

private:
  QtWaitingSpinner(const QtWaitingSpinner&);
  QtWaitingSpinner& operator=(const QtWaitingSpinner&);

  QTimer *m_timer;
  QWidget *m_parent;
  bool m_centreOnParent;
  int m_currentCounter;
  bool m_isSpinning;
};

#endif // QTWAITINGSPINNER_H
