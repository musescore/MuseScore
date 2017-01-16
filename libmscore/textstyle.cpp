//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "mscore.h"
#include "textstyle.h"
#include "xml.h"
#include "score.h"
#include "articulation.h"
#include "harmony.h"
#include "chordlist.h"
#include "page.h"
#include "mscore.h"
#include "clef.h"

namespace Ms {

//---------------------------------------------------------
//   TextStyle
//---------------------------------------------------------

TextStyle::TextStyle()
      {
      _family                 = "FreeSerif";
      _size                   = 10.0;
      _bold                   = false;
      _italic                 = false;
      _underline              = false;
      _hasFrame               = false;
      _square                 = false;
      _sizeIsSpatiumDependent = false;
      _frameWidth             = Spatium(0);
      _paddingWidth           = Spatium(0);
      _frameWidthMM           = 0.0;
      _paddingWidthMM         = 0.0;
      _frameRound             = 25;
      _frameColor             = MScore::defaultColor;
      _circle                 = false;
      _systemFlag             = false;
      _foregroundColor        = Qt::black;
      _backgroundColor        = QColor(255, 255, 255, 0);
      }

TextStyle::TextStyle(
   QString family,
   qreal size,
   bool bold,
   bool italic,
   bool underline,
   Align align,
   const QPointF& off,
   OffsetType ot,
   bool sd,
   bool hasFrame,
   bool square,
   Spatium fw,
   Spatium pw,
   int fr,
   QColor co,
   bool circle,
   bool systemFlag,
   QColor fg,
   QColor bg)
   :
   ElementLayout(align, off, ot),
   _bold(bold),
   _italic(italic),
   _underline(underline),
   _systemFlag(systemFlag),
   _sizeIsSpatiumDependent(sd),
   _hasFrame(hasFrame),
   _square(square),
   _circle(circle),
   _size(size),
   _frameWidth(fw),
   _paddingWidth(pw),
   _frameRound(fr),
   _frameColor(co),
   _foregroundColor(fg),
   _backgroundColor(bg)
      {
      //hasFrame       = (fw.val() != 0.0) || (bg.alpha() != 0);
      _family         = family;
      _frameWidthMM   = 0.0;
      _paddingWidthMM = 0.0;
      }

//---------------------------------------------------------
//   font
//---------------------------------------------------------

QFont TextStyle::font(qreal _spatium) const
      {
      qreal m = _size;
      QFont f(_family);
      f.setBold(_bold);
      f.setItalic(_italic);
      f.setUnderline(_underline);

      if (_sizeIsSpatiumDependent)
            m *= _spatium / SPATIUM20;
      f.setPointSizeF(m);
      return f;
      }

QString TextStyle::family() const                         { return _family;             }
void    TextStyle::setFamily(const QString& s)            { _family = s;                }

qreal   TextStyle::size() const                           { return _size;               }
void    TextStyle::setSize(qreal v)                       { _size = v;                  }

bool    TextStyle::bold() const                           { return _bold;               }
void    TextStyle::setBold(bool v)                        { _bold = v;                  }

bool    TextStyle::italic() const                         { return _italic;             }
void    TextStyle::setItalic(bool v)                      { _italic = v;                }

bool    TextStyle::underline() const                      { return _underline;          }
void    TextStyle::setUnderline(bool v)                   { _underline = v;             }

bool    TextStyle::hasFrame() const                       { return _hasFrame;           }
void    TextStyle::setHasFrame(bool v)                    { _hasFrame = v;              }

bool    TextStyle::square() const                         { return _square;             }
void    TextStyle::setSquare(bool val)                    { _square = val;              }

bool    TextStyle::circle() const                         { return _circle;             }
void    TextStyle::setCircle(bool v)                      { _circle = v;                }

Spatium TextStyle::frameWidth()  const                    { return _frameWidth;         }
void    TextStyle::setFrameWidth(Spatium v)               { _frameWidth = v;            }

Spatium TextStyle::paddingWidth() const                   { return _paddingWidth;       }
void    TextStyle::setPaddingWidth(Spatium v)             { _paddingWidth = v;          }

int     TextStyle::frameRound() const                     { return _frameRound;         }
void    TextStyle::setFrameRound(int v)                   { _frameRound = v;            }

QColor  TextStyle::frameColor() const                     { return _frameColor;         }
void    TextStyle::setFrameColor(const QColor& v)         { _frameColor = v;            }

QColor  TextStyle::foregroundColor() const                { return _foregroundColor;    }
void    TextStyle::setForegroundColor(const QColor& v)    { _foregroundColor = v;       }

QColor  TextStyle::backgroundColor() const                { return _backgroundColor;    }
void    TextStyle::setBackgroundColor(const QColor& v)    { _backgroundColor = v;       }

bool TextStyle::sizeIsSpatiumDependent() const            { return _sizeIsSpatiumDependent; }

qreal TextStyle::frameWidthMM()  const                    { return _frameWidthMM;        }
qreal TextStyle::paddingWidthMM() const                   { return _paddingWidthMM;      }

bool TextStyle::systemFlag() const                        { return _systemFlag;          }
void TextStyle::setSizeIsSpatiumDependent(bool v)         { _sizeIsSpatiumDependent = v; }
void TextStyle::setSystemFlag(bool v)                     { _systemFlag = v;             }
QFontMetricsF TextStyle::fontMetrics(qreal space) const   { return QFontMetricsF(font(space), MScore::paintDevice()); }
QRectF TextStyle::bbox(qreal space, const QString& s) const { return fontMetrics(space).boundingRect(s); }

}
