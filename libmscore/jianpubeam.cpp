//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "jianpubeam.h"
#include "jianpunote.h"
#include "element.h"
#include "measure.h"
#include "system.h"
#include "chordrest.h"
#include "chord.h"
#include "rest.h"


namespace Ms {

JianpuBeam::JianpuBeam(Score* s)
   : Beam(s)
      {
      }

JianpuBeam::JianpuBeam(const Beam& b)
   : Beam(b)
      {
      }

JianpuBeam::JianpuBeam(const JianpuBeam& b)
   : Beam(b)
      {
      }

JianpuBeam::~JianpuBeam()
      {
      }

void JianpuBeam::read(XmlReader& xml)
      {
      QPointF p1, p2;
      //qreal _spatium = spatium();
      _id = xml.intAttribute("id");
      while (xml.readNextStartElement()) {
            const QStringRef& tag(xml.name());
#if 0
            // From Beam; not needed.

            if (tag == "StemDirection") {
                  setProperty(P_ID::STEM_DIRECTION, Ms::getProperty(P_ID::STEM_DIRECTION, e));
                  xml.readNext();
                  }
            else if (tag == "distribute")
                  setDistribute(xml.readInt());
            else if (tag == "noSlope") {
                  setNoSlope(xml.readInt());
                  noSlopeStyle = PropertyStyle::UNSTYLED;
                  }
            else if (tag == "growLeft")
                  setGrowLeft(xml.readDouble());
            else if (tag == "growRight")
                  setGrowRight(xml.readDouble());
            else if (tag == "y1") {
                  if (fragments.empty())
                        fragments.append(new BeamFragment);
                  BeamFragment* f = fragments.back();
                  int idx = (_direction == Direction::AUTO || _direction == Direction::DOWN) ? 0 : 1;
                  _userModified[idx] = true;
                  f->py1[idx] = xml.readDouble() * _spatium;
                  }
            else if (tag == "y2") {
                  if (fragments.empty())
                        fragments.append(new BeamFragment);
                  BeamFragment* f = fragments.back();
                  int idx = (_direction == Direction::AUTO || _direction == Direction::DOWN) ? 0 : 1;
                  _userModified[idx] = true;
                  f->py2[idx] = xml.readDouble() * _spatium;
                  }
            else if (tag == "Fragment") {
                  BeamFragment* f = new BeamFragment;
                  int idx = (_direction == Direction::AUTO || _direction == Direction::DOWN) ? 0 : 1;
                  _userModified[idx] = true;
                  qreal _spatium = spatium();

                  while (xml.readNextStartElement()) {
                        const QStringRef& tag(xml.name());
                        if (tag == "y1")
                              f->py1[idx] = xml.readDouble() * _spatium;
                        else if (tag == "y2")
                              f->py2[idx] = xml.readDouble() * _spatium;
                        else
                              xml.unknown();
                        }
                  fragments.append(f);
                  }
            else if (tag == "l1" || tag == "l2")      // ignore
                  xml.skipCurrentElement();
            else if (tag == "subtype")          // obsolete
                  xml.skipCurrentElement();
            else if (!Element::readProperties(xml))
                  xml.unknown();
#else
            if (!Element::readProperties(xml))
                  xml.unknown();
#endif
            }
      }

void JianpuBeam::write(XmlWriter& xml) const
      {
      if (_elements.empty())
            return;

      xml.stag(QString("Beam id=\"%1\"").arg(_id));
      Element::writeProperties(xml);

#if 0
        // From Beam; not needed.

      int idx = (_direction == Direction::AUTO || _direction == Direction::DOWN) ? 0 : 1;
      if (_userModified[idx]) {
            qreal _spatium = spatium();
            for (BeamFragment* f : fragments) {
                  xml.stag("Fragment");
                  xml.tag("y1", f->py1[idx] / _spatium);
                  xml.tag("y2", f->py2[idx] / _spatium);
                  xml.etag();
                  }
            }

      // this info is used for regression testing
      // l1/l2 is the beam position of the layout engine
      if (MScore::testMode) {
            qreal _spatium4 = spatium() * .25;
            for (BeamFragment* f : fragments) {
                  xml.tag("l1", int(lrint(f->py1[idx] / _spatium4)));
                  xml.tag("l2", int(lrint(f->py2[idx] / _spatium4)));
                  }
            }
#endif

      xml.etag();
      }

void JianpuBeam::layout()
      {
      // Always put the horizontal beams underneath the chords/rests.

      // Calculate and set beam's position.
      // Jianpu bar-line span: -4 to +4
      qreal x = 0.0;
      qreal y = JianpuNote::NOTE_BASELINE * spatium() * 0.5;
      setPos(x, y);

      System* system = _elements.front()->measure()->system();
      setParent(system);

      QPointF pagePosition(pagePos());
      setbbox(QRectF());

      // Get maximum beam level.
      int beamLevels = 0;
      for (const ChordRest* c : _elements)
            beamLevels = qMax(beamLevels, c->durationType().hooks());

      // Set the first beam's y position.
      const ChordRest* cr = _elements.front();
      y = cr->pos().y() - pagePosition.y();
      if (cr->isChord()) {
            const Note* note = toChord(cr)->notes().at(0);
            y = note->pos().y() + note->bbox().height();
            const JianpuNote* jn = dynamic_cast<const JianpuNote*>(note);
            if (jn && jn->noteOctave() >= 0) {
                  // Note's bounding box does not include space for lower-octave dots.
                  // Add octave-dot box y-offset to align with beams of other notes.
                  y += JianpuNote::OCTAVE_DOTBOX_Y_OFFSET + JianpuNote::OCTAVE_DOTBOX_HEIGHT;
                  }
            }
      else if (cr->isRest()){
            const Rest* rest = toRest(cr);
            y = rest->pos().y() + rest->bbox().height();
            // Rest's bounding box does not include space for lower-octave dots.
            // Add octave-dot box y-offset to align with beams of other notes.
            y += JianpuNote::OCTAVE_DOTBOX_Y_OFFSET + JianpuNote::OCTAVE_DOTBOX_HEIGHT;
            }
      qreal beamDistance = JianpuNote::BEAM_HEIGHT + JianpuNote::BEAM_Y_SPACE;

      // Create beam segments.
      int n = _elements.size();
      int c1 = 0;
      qreal x1 = 0;
      int c2 = 0;
      qreal x2 = 0;
      for (int beamLevel = 0; beamLevel < beamLevels; ++beamLevel) {
            // Loop through the different groups for this beam level.
            // Inner loop will advance through chordrests within each group.
            int i = 0;
            while ( i < n) {
                  ChordRest* cr1 = _elements[i];
                  int l1 = cr1->durationType().hooks() - 1;
                  if (l1 < beamLevel) {
                        i++;
                        continue;
                        }

                  // Found beginning of a group.
                  // Loop through chordrests looking for end of the group.
                  c1 = i;
                  int j = i + 1;
                  while (j < n) {
                        ChordRest* cr2 = _elements[j];
                        int l2 = cr2->durationType().hooks() - 1;
                        if (l2 < beamLevel) {
                              break;
                              }
                        j++;
                        }

                  // Found the beam-level discontinuity; chordrest at j-1 is end of the group.
                  c2 = j - 1;
                  // Calculate x-values of beam segment.
                  cr1 = _elements[c1];
                  x1 = cr1->pos().x() + cr1->pageX() - pagePosition.x();
                  ChordRest* cr2 = _elements[c2];
                  x2 = cr2->pos().x() + cr2->pageX() - pagePosition.x() + cr2->bbox().width();
                  // Add beam segment.
                  beamSegments.push_back(new QLineF(x1, y, x2, y));
                  // Update bounding box.
                  addbbox(QRectF(x1, y, x2 - x1, beamDistance));

                  // Update i index with last scanned location and loop again for more groups.
                  i = j;
                  }

            // Update y value for beams at next beam level.
            y += beamDistance;
            }
      //qDebug("bbox x=%.0f y=%.0f w=%.0f h=%.0f", bbox().x(), bbox().y(), bbox().width(), bbox().height());
      //Q_ASSERT(bbox().x() < 20000 && bbox().y() < 20000);
      //Q_ASSERT(bbox().width() < 20000 && bbox().height() < 20000);
      }

void JianpuBeam::draw(QPainter* painter) const
      {
      if (beamSegments.empty())
            return;

      // Draw beam(s) underneath the note/rest numbers.
      QBrush brush(curColor(), Qt::SolidPattern);
      painter->setBrush(brush);
      painter->setPen(Qt::NoPen);
      qreal height = JianpuNote::BEAM_HEIGHT;
      for (const QLineF* line : beamSegments) {
            QRectF beam(line->x1(), line->y1(), line->length(), height);
            painter->fillRect(beam, brush);
            }
      }

} // namespace Ms
