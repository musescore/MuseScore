//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "volta.h"
#include "style.h"
#include "xml.h"
#include "score.h"
#include "text.h"
#include "system.h"

namespace Ms {

static const ElementStyle voltaStyle {
      { Sid::voltaFontFace,                      Pid::BEGIN_FONT_FACE         },
      { Sid::voltaFontFace,                      Pid::CONTINUE_FONT_FACE      },
      { Sid::voltaFontFace,                      Pid::END_FONT_FACE           },
      { Sid::voltaFontSize,                      Pid::BEGIN_FONT_SIZE         },
      { Sid::voltaFontSize,                      Pid::CONTINUE_FONT_SIZE      },
      { Sid::voltaFontSize,                      Pid::END_FONT_SIZE           },
      { Sid::voltaFontBold,                      Pid::BEGIN_FONT_BOLD         },
      { Sid::voltaFontBold,                      Pid::CONTINUE_FONT_BOLD      },
      { Sid::voltaFontBold,                      Pid::END_FONT_BOLD           },
      { Sid::voltaFontItalic,                    Pid::BEGIN_FONT_ITALIC       },
      { Sid::voltaFontItalic,                    Pid::CONTINUE_FONT_ITALIC    },
      { Sid::voltaFontItalic,                    Pid::END_FONT_ITALIC         },
      { Sid::voltaFontUnderline,                 Pid::BEGIN_FONT_UNDERLINE    },
      { Sid::voltaFontUnderline,                 Pid::CONTINUE_FONT_UNDERLINE },
      { Sid::voltaFontUnderline,                 Pid::END_FONT_UNDERLINE      },
      { Sid::voltaAlign,                         Pid::BEGIN_TEXT_ALIGN        },
      { Sid::voltaAlign,                         Pid::CONTINUE_TEXT_ALIGN     },
      { Sid::voltaAlign,                         Pid::END_TEXT_ALIGN          },
      { Sid::voltaOffset,                        Pid::BEGIN_TEXT_OFFSET       },
      { Sid::voltaOffset,                        Pid::CONTINUE_TEXT_OFFSET    },
      { Sid::voltaOffset,                        Pid::END_TEXT_OFFSET         },
      { Sid::voltaLineWidth,                     Pid::LINE_WIDTH              },
      { Sid::voltaLineStyle,                     Pid::LINE_STYLE              },
      { Sid::voltaHook,                          Pid::BEGIN_HOOK_HEIGHT       },
      { Sid::voltaHook,                          Pid::END_HOOK_HEIGHT         },
      };

//---------------------------------------------------------
//   VoltaSegment
//---------------------------------------------------------

VoltaSegment::VoltaSegment(Score* s) : TextLineBaseSegment(s, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
      {
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void VoltaSegment::layout()
      {
      TextLineBaseSegment::layout();
      autoplaceSpannerSegment(spatium() * 1.0, Sid::voltaY, Sid::voltaY);
      }

//---------------------------------------------------------
//   propertyDelegate
//---------------------------------------------------------

Element* VoltaSegment::propertyDelegate(Pid pid)
      {
      if (pid == Pid::BEGIN_HOOK_TYPE || pid == Pid::END_HOOK_TYPE || pid == Pid::VOLTA_ENDING)
            return spanner();
      return TextLineBaseSegment::propertyDelegate(pid);
      }

//---------------------------------------------------------
//   Volta
//---------------------------------------------------------

Volta::Volta(Score* s)
   : TextLineBase(s, ElementFlag::SYSTEM)
      {
      setPlacement(Placement::ABOVE);
      initElementStyle(&voltaStyle);

      setBeginTextPlace(PlaceText::BELOW);
      setContinueTextPlace(PlaceText::BELOW);
      setLineVisible(true);
      resetProperty(Pid::BEGIN_TEXT);
      resetProperty(Pid::CONTINUE_TEXT);
      resetProperty(Pid::END_TEXT);
      resetProperty(Pid::BEGIN_TEXT_PLACE);
      resetProperty(Pid::CONTINUE_TEXT_PLACE);
      resetProperty(Pid::END_TEXT_PLACE);
      resetProperty(Pid::BEGIN_HOOK_TYPE);
      resetProperty(Pid::END_HOOK_TYPE);

      setAnchor(Anchor::MEASURE);
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void Volta::setText(const QString& s)
      {
      setBeginText(s);
      }

//---------------------------------------------------------
//   text
//---------------------------------------------------------

QString Volta::text() const
      {
      return beginText();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Volta::read(XmlReader& e)
      {
      qDeleteAll(spannerSegments());
      spannerSegments().clear();

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "endings") {
                  QString s = e.readElementText();
                  QStringList sl = s.split(",", QString::SkipEmptyParts);
                  _endings.clear();
                  for (const QString& l : sl) {
                        int i = l.simplified().toInt();
                        _endings.append(i);
                        }
                  }
            else if (!TextLineBase::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Volta::write(XmlWriter& xml) const
      {
      xml.stag(name());
      TextLineBase::writeProperties(xml);
      QString s;
      for (int i : _endings) {
            if (!s.isEmpty())
                  s += ", ";
            s += QString("%1").arg(i);
            }
      xml.tag("endings", s);
      xml.etag();
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Volta::createLineSegment()
      {
      return new VoltaSegment(score());
      }

//---------------------------------------------------------
//   hasEnding
//---------------------------------------------------------

bool Volta::hasEnding(int repeat) const
      {
      for (int ending : endings()) {
            if (ending == repeat)
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   lastEnding
//---------------------------------------------------------

int Volta::lastEnding() const
      {
      if (_endings.isEmpty())
            return 0;
      return _endings.last();
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Volta::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::VOLTA_ENDING:
                  return QVariant::fromValue(endings());
            default:
                  break;
            }
      return TextLineBase::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Volta::setProperty(Pid propertyId, const QVariant& val)
      {
      switch (propertyId) {
            case Pid::VOLTA_ENDING:
                  setEndings(val.value<QList<int>>());
                  break;
            default:
                  if (!TextLineBase::setProperty(propertyId, val))
                        return false;
                  break;
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Volta::propertyDefault(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::VOLTA_ENDING:
                  return QVariant::fromValue(QList<int>());
            case Pid::ANCHOR:
                  return int(Anchor::MEASURE);
            case Pid::BEGIN_HOOK_TYPE:
                  return int(HookType::HOOK_90);
            case Pid::END_HOOK_TYPE:
                  return int(HookType::NONE);
            case Pid::BEGIN_TEXT:
            case Pid::CONTINUE_TEXT:
            case Pid::END_TEXT:
                  return "";
            case Pid::LINE_VISIBLE:
                  return true;
            case Pid::BEGIN_TEXT_PLACE:
            case Pid::CONTINUE_TEXT_PLACE:
            case Pid::END_TEXT_PLACE:
                  return int(PlaceText::ABOVE);

            default:
                  return TextLineBase::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Volta::accessibleInfo() const
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(text());
      }

//---------------------------------------------------------
//   setVoltaType
//    deprecated
//---------------------------------------------------------

void Volta::setVoltaType(Type val)
      {
      setEndHookType(Type::CLOSED == val ? HookType::HOOK_90 : HookType::NONE);
      }

//---------------------------------------------------------
//   voltaType
//    deprecated
//---------------------------------------------------------

Volta::Type Volta::voltaType() const
      {
      return endHookType() != HookType::NONE ? Type::CLOSED : Type::OPEN;
      }


}

