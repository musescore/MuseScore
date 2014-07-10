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

#ifndef __CHORDREST_H__
#define __CHORDREST_H__

#include "symbol.h"
#include "duration.h"
#include "beam.h"
#include "segment.h"

namespace Ms {

enum class CrossMeasure : signed char {
      UNKNOWN = -1,
      NONE = 0,
      FIRST,
      SECOND
      };

class Score;
class Measure;
class Tuplet;
class Segment;
class Slur;
class Articulation;
class Lyrics;
class TabDurationSymbol;
class Spanner;

//-------------------------------------------------------------------
//   @@ ChordRest
///    Virtual base class. Chords and rests can be part of a beam
//
//   @P durationType  int
//   @P beamMode      Ms::Beam::Mode (AUTO, BEGIN, MID, END, NONE, BEGIN32, BEGIN64, INVALID)
//   @P small         bool           small chord/rest
//-------------------------------------------------------------------

class ChordRest : public DurationElement {
      Q_OBJECT
      Q_PROPERTY(int            durationType  READ durationTypeTicks  WRITE setDurationType)
      Q_PROPERTY(Ms::Beam::Mode beamMode      READ beamMode           WRITE undoSetBeamMode)
      Q_PROPERTY(bool           small         READ small              WRITE undoSetSmall)
      Q_ENUMS(Ms::Beam::Mode)

      TDuration _durationType;
      int _staffMove;         // -1, 0, +1, used for crossbeaming

   protected:
      QList<Articulation*> _articulations;
      Beam* _beam;
      QList<Lyrics*> _lyricsList;
      TabDurationSymbol* _tabDur;         // stores a duration symbol in tablature staves

      Beam::Mode _beamMode;
      bool _up;                           // actual stem direction
      bool _small;

      // CrossMeasure: combine 2 tied notes if across a bar line and can be combined in a single duration
      CrossMeasure _crossMeasure;            ///< 0: no cross-measure modification; 1: 1st note of a mod.; -1: 2nd note
      TDuration _crossMeasureTDur;  ///< the total Duration type of the combined notes

      Space _space;                       // cached value from layout

   public:
      ChordRest(Score*);
      ChordRest(const ChordRest&, bool link = false);
      ChordRest &operator=(const ChordRest&) = delete;
      ~ChordRest();

      virtual Element::Type type() const = 0;
      virtual Element* drop(const DropData&);
      virtual void undoUnlink() override;

      virtual Segment* segment() const           { return (Segment*)parent(); }
      virtual Measure* measure() const = 0;

      virtual void writeProperties(Xml& xml) const;
      virtual bool readProperties(XmlReader&);
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);

      void setBeamMode(Beam::Mode m)            { _beamMode = m;    }
      void undoSetBeamMode(Beam::Mode m);
      Beam::Mode beamMode() const               { return _beamMode; }

      void setBeam(Beam* b);
      virtual Beam* beam() const                { return _beam; }
      int beams() const                         { return _durationType.hooks(); }
      virtual qreal upPos()   const = 0;
      virtual qreal downPos() const = 0;
      virtual qreal centerX() const = 0;

      int line(bool up) const                   { return up ? upLine() : downLine(); }
      int line() const                          { return _up ? upLine() : downLine(); }
      virtual int upLine() const = 0;
      virtual int downLine() const = 0;
      virtual QPointF stemPos() const = 0;
      virtual qreal stemPosX() const = 0;
      virtual QPointF stemPosBeam() const = 0;

      bool up() const                           { return _up;   }
      void setUp(bool val)                      { _up = val; }

      QList<Articulation*>& articulations()     { return _articulations; }
      const QList<Articulation*>& articulations() const { return _articulations; }
      Articulation* hasArticulation(const Articulation*);

      bool small() const                        { return _small; }
      void setSmall(bool val);
      void undoSetSmall(bool val);

      int staffMove() const                     { return _staffMove; }
      void setStaffMove(int val)                { _staffMove = val; }

      void layoutArticulations();

      const TDuration& durationType() const     { return _crossMeasure == CrossMeasure::FIRST ?
                                                      _crossMeasureTDur : _durationType;        }
      const TDuration& actualDurationType() const   { return _durationType; }
      void setDurationType(TDuration::DurationType t);
      void setDurationType(const QString& s);
      void setDurationType(int ticks);
      void setDurationType(const TDuration& v);
      void setDots(int n)                       { _durationType.setDots(n); }
      int dots() const        { return _crossMeasure == CrossMeasure::FIRST ? _crossMeasureTDur.dots()
                                    : (_crossMeasure == CrossMeasure::SECOND ? 0 : _durationType.dots()); }
      int actualDots() const  { return _durationType.dots(); }
      int durationTypeTicks() { return _crossMeasure == CrossMeasure::FIRST ? _crossMeasureTDur.ticks()
                                    : _durationType.ticks(); }
      QString durationUserName();

      virtual void setTrack(int val);
      virtual int tick() const;
      virtual int rtick() const;
      virtual Space space() const               { return _space; }

      const QList<Lyrics*>& lyricsList() const { return _lyricsList; }
      QList<Lyrics*>& lyricsList()             { return _lyricsList; }
      Lyrics* lyrics(int no)                   { return _lyricsList.value(no); }
      virtual void add(Element*);
      virtual void remove(Element*);
      void removeDeleteBeam(bool beamed = false);

      CrossMeasure crossMeasure() const            { return _crossMeasure; }
      void setCrossMeasure(CrossMeasure val)       { _crossMeasure = val;  }
      virtual void crossMeasureSetup(bool /*on*/)   { }

      virtual QVariant getProperty(P_ID propertyId) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);
      virtual QVariant propertyDefault(P_ID) const;
      bool isGrace() const;
      bool isGraceBefore() const;
      bool isGraceAfter() const;
      void writeBeam(Xml& xml);
      Segment* nextSegmentAfterCR(Segment::Type types) const;

      virtual Element* nextElement() override;
      virtual Element* prevElement() override;
      virtual QString accessibleExtraInfo() override;
      };


}     // namespace Ms
#endif

