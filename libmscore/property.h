//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PROPERTY_H__
#define __PROPERTY_H__

//------------------------------------------------------------------------
//   Element Properties
//    accessible through
//    virtual QVariant Element::getProperty(int propertyId)
//    virtual void Element::setProperty(int propertyId, const QVariant&)
//------------------------------------------------------------------------

enum P_ID {
      P_SUBTYPE,
      P_SELECTED,
      P_COLOR,
      P_VISIBLE,
      P_SMALL,
      P_SHOW_COURTESY,
      P_LINE_TYPE,
      P_PITCH,
      P_TPC,
      P_HEAD_TYPE,
      P_HEAD_GROUP,
      P_VELO_TYPE,
      P_VELO_OFFSET,
      P_ARTICULATION_ANCHOR,
      P_DIRECTION,
      P_STEM_DIRECTION,
      P_NO_STEM,
      P_SLUR_DIRECTION,
      P_LEADING_SPACE,
      P_TRAILING_SPACE,
      P_DISTRIBUTE,
      P_MIRROR_HEAD,
      P_DOT_POSITION,
      P_ONTIME_OFFSET,
      P_OFFTIME_OFFSET,
      P_TUNING,
      P_PAUSE,
      P_BARLINE_SPAN,
      P_USER_OFF,
      P_FRET,
      P_STRING,
      P_GHOST,
      P_TIMESIG_NOMINAL,
      P_TIMESIG_ACTUAL,
      P_NUMBER_TYPE,
      P_BRACKET_TYPE,
      P_NORMAL_NOTES,
      P_ACTUAL_NOTES,
      P_P1,
      P_P2,
      P_GROW_LEFT,
      P_GROW_RIGHT,
      P_BOX_HEIGHT,
      P_BOX_WIDTH,
      P_TOP_GAP,
      P_BOTTOM_GAP,
      P_LEFT_MARGIN,
      P_RIGHT_MARGIN,
      P_TOP_MARGIN,
      P_BOTTOM_MARGIN,
      P_LAYOUT_BREAK,
      P_AUTOSCALE,
      P_SIZE,
      P_SCALE,
      P_LOCK_ASPECT_RATIO,
      P_SIZE_IS_SPATIUM,
      P_TEXT_STYLE,
      P_USER_MODIFIED,
      P_BEAM_POS,
      P_BEAM_MODE,
      P_USER_LEN,       // used for stems
      P_SPACE,          // used for spacer
      P_TEMPO,
      P_TEMPO_FOLLOW_TEXT,
      P_ACCIDENTAL_BRACKET,
      P_NUMERATOR_STRING,
      P_DENOMINATOR_STRING,
      P_SHOW_NATURALS,

      P_END
      };

enum P_TYPE {
      T_SUBTYPE,
      T_BOOL,
      T_INT,
      T_REAL,
      T_SREAL,
      T_FRACTION,
      T_POINT,
      T_SIZE,
      T_STRING,
      T_SCALE,
      T_COLOR,
      T_DIRECTION,      // enum Direction
      T_DIRECTION_H,    // enum DirectionH
      T_LAYOUT_BREAK,
      T_VALUE_TYPE,
      T_BEAM_MODE,
      };

extern QVariant getProperty(P_ID type, const QDomElement& e);
extern P_TYPE propertyType(P_ID);
extern const char* propertyName(P_ID);

//---------------------------------------------------------
//   template Property
//---------------------------------------------------------

template <class T>
class Property {
   public:
      P_ID id;
      void* (T::*data)();   // member function returns pointer to data
      void* defaultVal;     // pointer to default data
      };

//---------------------------------------------------------
//   property
//---------------------------------------------------------

template <class T>
Property<T>* property(Property<T>* list, int id)
      {
      for (int i = 0; ; ++i) {
            if (list[i].id == P_END)
                  break;
            else if (list[i].id == id)
                  return &list[i];
            }
      return 0;
      }

template <class T>
Property<T>* property(Property<T>* list, const QString& name)
      {
      for (int i = 0; ; ++i) {
            if (list[i].id == P_END)
                  break;
            else if (propertyName((P_ID)i) == name)
                  return &list[i];
            }
      return 0;
      }

#include "xml.h"

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

#define WRITE_PROPERTIES(T) \
      for (int i = 0;; ++i) {  \
            const Property<T>& p = propertyList[i]; \
            if (p.id == P_END) \
                  break;       \
            xml.tag(p.id, ((*(T*)this).*(p.data))(), p.defaultVal); \
            }

#define PROPERTY_DECLARATIONS(T)                                                 \
      virtual QVariant getProperty(P_ID propertyId) const;                       \
      virtual bool setProperty(P_ID propertyId, const QVariant&);                \
      virtual bool setProperty(const QString&, const QDomElement&);              \
      Property<T>* property(P_ID id) const;                                      \
      virtual QVariant propertyDefault(P_ID) const;                              \
      static Property<T> propertyList[];


//---------------------------------------------------------
//   PROPERTY_FUNCTIONS
//---------------------------------------------------------

#define PROPERTY_FUNCTIONS(T)                                                    \
                                                                                 \
Property<T>* T::property(P_ID id) const                                          \
      {                                                                          \
      for (int i = 0;; ++i) {                                                    \
            if (propertyList[i].id == P_END)                                     \
                  break;                                                         \
            if (propertyList[i].id == id)                                        \
                  return &propertyList[i];                                       \
            }                                                                    \
      return 0;                                                                  \
      }                                                                          \
QVariant T::getProperty(P_ID propertyId) const                                   \
      {                                                                          \
      Property<T>* p = property(propertyId);                                     \
      if (p)                                                                     \
            return getVariant(propertyId, ((*(T*)this).*(p->data))());           \
      return Element::getProperty(propertyId);                                   \
      }                                                                          \
bool T::setProperty(P_ID propertyId, const QVariant& v)                          \
      {                                                                          \
      score()->addRefresh(canvasBoundingRect());                                 \
      Property<T>* p = property(propertyId);                                     \
      bool rv = true;                                                            \
      if (p) {                                                                   \
            setVariant(propertyId, ((*this).*(p->data))(), v);                   \
            setGenerated(false);                                                 \
            }                                                                    \
      else                                                                       \
            rv = Element::setProperty(propertyId, v);                            \
      score()->setLayoutAll(true);                                               \
      return rv;                                                                 \
      }                                                                          \
bool T::setProperty(const QString& name, const QDomElement& e)                   \
      {                                                                          \
      for (int i = 0;; ++i) {                                                    \
            P_ID id = propertyList[i].id;                                        \
            if (id == P_END)                                                     \
                  break;                                                         \
            if (propertyName(id) == name) {                                      \
                  QVariant v = ::getProperty(id, e);                             \
                  setVariant(id, ((*this).*(propertyList[i].data))(), v);        \
                  setGenerated(false);                                           \
                  return true;                                                   \
                  }                                                              \
            }                                                                    \
      return Element::setProperty(name, e);                                      \
      }                                                                          \
QVariant T::propertyDefault(P_ID id) const                                       \
      {                                                                          \
      Property<T>* p = property(id);                                             \
      return getVariant(id, p->defaultVal);                                      \
      }

#endif

