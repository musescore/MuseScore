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

#include "changeMap.h"
#include "measure.h"
#include "score.h"
#include "staff.h"
#include "style.h"
#include "system.h"
#include "tempo.h"
#include "text.h"
#include "xml.h"

#include <algorithm>

namespace Ms {

static const ElementStyle voltaStyle {
      { Sid::voltaFontFace,                      Pid::BEGIN_FONT_FACE         },
      { Sid::voltaFontFace,                      Pid::CONTINUE_FONT_FACE      },
      { Sid::voltaFontFace,                      Pid::END_FONT_FACE           },
      { Sid::voltaFontSize,                      Pid::BEGIN_FONT_SIZE         },
      { Sid::voltaFontSize,                      Pid::CONTINUE_FONT_SIZE      },
      { Sid::voltaFontSize,                      Pid::END_FONT_SIZE           },
      { Sid::voltaFontStyle,                     Pid::BEGIN_FONT_STYLE        },
      { Sid::voltaFontStyle,                     Pid::CONTINUE_FONT_STYLE     },
      { Sid::voltaFontStyle,                     Pid::END_FONT_STYLE          },
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
      { Sid::voltaPosAbove,                      Pid::OFFSET                  },
      };

//---------------------------------------------------------
//   VoltaSegment
//---------------------------------------------------------

VoltaSegment::VoltaSegment(Spanner* sp, Score* s) : TextLineBaseSegment(sp, s, ElementFlag::MOVABLE | ElementFlag::ON_STAFF | ElementFlag::SYSTEM)
      {
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void VoltaSegment::layout()
      {
      TextLineBaseSegment::layout();
      autoplaceSpannerSegment();
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

      setAnchor(VOLTA_ANCHOR);
      }

///
/// \brief sorts the provided list in ascending order
///
void Volta::setEndings(const QList<int>& l)
      {
      _endings = l;
      std::sort(_endings.begin(), _endings.end());
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
      eraseSpannerSegments();

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
            else if (!readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Volta::readProperties(XmlReader& e)
      {
      if (!TextLineBase::readProperties(e))
            return false;

      if (anchor() != VOLTA_ANCHOR) {
            // Volta strictly assumes that its anchor is measure, so don't let old scores override this.
            qWarning("Correcting volta anchor type from %d to %d", int(anchor()), int(VOLTA_ANCHOR));
            setAnchor(VOLTA_ANCHOR);
            }

      return true;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Volta::write(XmlWriter& xml) const
      {
      xml.stag(this);
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

static const ElementStyle voltaSegmentStyle {
      { Sid::voltaPosAbove,                      Pid::OFFSET                  },
      { Sid::voltaMinDistance,                   Pid::MIN_DISTANCE            },
      };

LineSegment* Volta::createLineSegment()
      {
      VoltaSegment* vs = new VoltaSegment(this, score());
      vs->setTrack(track());
      vs->initElementStyle(&voltaSegmentStyle);
      return vs;
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
//   firstEnding
//---------------------------------------------------------

int Volta::firstEnding() const
      {
      if (_endings.isEmpty())
            return 0;
      return _endings.front();
      }

//---------------------------------------------------------
//   lastEnding
//---------------------------------------------------------

int Volta::lastEnding() const
      {
      if (_endings.isEmpty())
            return 0;
      return _endings.back();
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
                  return int(VOLTA_ANCHOR);
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

            case Pid::PLACEMENT:
                  return int(Placement::ABOVE);

            default:
                  return TextLineBase::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   layoutSystem
//---------------------------------------------------------

SpannerSegment * Volta::layoutSystem(System * system)
      {
      SpannerSegment* voltaSegment= SLine::layoutSystem(system);

      // we need set tempo in layout because all tempos of score is set in layout
      // so fermata in seconda volta works correct because fermata apply itself tempo during layouting
      setTempo();

      return voltaSegment;
      }

//---------------------------------------------------------
//   setVelocity
//---------------------------------------------------------

void Volta::setVelocity() const
      {
      Measure* startMeasure = Spanner::startMeasure();
      Measure* endMeasure = Spanner::endMeasure();

      if (startMeasure && endMeasure) {
            if (!endMeasure->repeatEnd())
                  return;

            Fraction startTick  = Fraction::fromTicks(startMeasure->tick().ticks() - 1);
            Fraction endTick    = Fraction::fromTicks((endMeasure->tick() + endMeasure->ticks()).ticks() - 1);
            Staff* st      = staff();
            ChangeMap& velo = st->velocities();
            auto prevVelo  = velo.val(startTick);
            velo.addFixed(endTick, prevVelo);
            }
      }

//---------------------------------------------------------
//   setChannel
//---------------------------------------------------------

void Volta::setChannel() const
      {
      Measure* startMeasure = Spanner::startMeasure();
      Measure* endMeasure = Spanner::endMeasure();

      if (startMeasure && endMeasure) {
            if (!endMeasure->repeatEnd())
                  return;

            Fraction startTick = startMeasure->tick() - Fraction::fromTicks(1);
            Fraction endTick  = endMeasure->endTick() - Fraction::fromTicks(1);
            Staff* st = staff();
            for (int voice = 0; voice < VOICES; ++voice) {
                  int channel = st->channel(startTick, voice);
                  st->insertIntoChannelList(voice, endTick, channel);
                  }
            }
      }

//---------------------------------------------------------
//   setTempo
//---------------------------------------------------------

void Volta::setTempo() const
      {
      Measure* startMeasure = Spanner::startMeasure();
      Measure* endMeasure = Spanner::endMeasure();

      if (startMeasure && endMeasure) {
            if (!endMeasure->repeatEnd())
                  return;
            Fraction startTick = startMeasure->tick() - Fraction::fromTicks(1);
            Fraction endTick  = endMeasure->endTick() - Fraction::fromTicks(1);
            qreal tempoBeforeVolta = score()->tempomap()->tempo(startTick.ticks());
            score()->setTempo(endTick, tempoBeforeVolta);
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

