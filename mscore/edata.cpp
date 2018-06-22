//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "musescore.h"
#include "libmscore/articulation.h"
#include "libmscore/score.h"
#include "libmscore/dynamic.h"
#include "libmscore/marker.h"
#include "libmscore/timesig.h"
#include "libmscore/key.h"
#include "libmscore/keysig.h"
#include "libmscore/chord.h"
#include "libmscore/rest.h"
#include "libmscore/clef.h"
#include "libmscore/hook.h"
#include "libmscore/xml.h"
#include "libmscore/accidental.h"
#include "libmscore/durationtype.h"
#include "libmscore/sym.h"
#include "libmscore/staff.h"
#include "libmscore/bracket.h"
#include "libmscore/tuplet.h"
#include "libmscore/barline.h"
#include "libmscore/stem.h"

//##  write out extended metadata about every painted symbol

namespace Ms {

//    TODO: spatium is not constant 20 but may depend on score

//---------------------------------------------------------
//   writeData
//---------------------------------------------------------

static void writeData(XmlWriter& xml, qreal spatium, qreal scale, const QString& name, const Element* e)
      {
      qreal interline = 20.0;
      if (e->staff()) {
            interline *= (e->staff()->spatium(e->tick()) / e->score()->spatium());
            scale     *= (e->score()->spatium() / e->staff()->spatium(e->tick()));
            }
      if (scale == 1.0)
            xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(interline).arg(name));
      else
            xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"%3\"").arg(interline).arg(scale).arg(name));
      QRectF rr(e->pageBoundingRect());
      QRectF r(rr.x() * spatium, rr.y() * spatium, rr.width() * spatium, rr.height() * spatium);
      xml.putLevel();
      xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
         .arg(r.x(),      0, 'f', 3)
         .arg(r.y(),      0, 'f', 3)
         .arg(r.width(),  0, 'f', 3)
         .arg(r.height(), 0, 'f', 3);
      xml.etag();
      }

//---------------------------------------------------------
//   writeRestData
//---------------------------------------------------------

static void writeRestData(XmlWriter& xml, qreal spatium, qreal scale, const QString& name, const Element* e)
      {
      qreal interline = 20.0;
      if (e->staff()) {
            interline *= (e->staff()->spatium(e->tick()) / e->score()->spatium());
            scale     *= (e->score()->spatium() / e->staff()->spatium(e->tick()));
            }
      if (scale == 1.0)
            xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(interline).arg(name));
      else
            xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"%3\"").arg(interline).arg(scale).arg(name));

      QRectF rr(e->pageBoundingRect());
      QRectF r(rr.x() * spatium, rr.y() * spatium, rr.width() * spatium, rr.height() * spatium);
      xml.putLevel();
      xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
         .arg(r.x(),      0, 'f', 3)
         .arg(r.y(),      0, 'f', 3)
         .arg(r.width(),  0, 'f', 3)
         .arg(r.height(), 0, 'f', 3);
      xml.etag();
      const Rest* rest = toRest(e);
      int dots = rest->durationType().dots();
      if (dots) {
            QString dotname = "augmentationDot";
            if (scale == 1.0)
                  xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(interline).arg(dotname));
            else
                  xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"%3\"").arg(interline).arg(scale).arg(dotname));
            QRectF rd(rest->symBbox(SymId::augmentationDot));
            QRectF dot((rr.x() + (3 * rr.width()) / 2) * spatium, (rr.y() - rd.y() + (rr.height() / 4)) * spatium, rd.width() * spatium, rd.height() * spatium);
            xml.putLevel();
            xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
               .arg(dot.x(),      0, 'f', 3)
               .arg(dot.y(),      0, 'f', 3)
               .arg(dot.width(),  0, 'f', 3)
               .arg(dot.height(), 0, 'f', 3);
            xml.etag();
            }
      }

//---------------------------------------------------------
//   writeTupletData
//---------------------------------------------------------

static void writeTupletData(XmlWriter& xml, qreal spatium, qreal scale, int num, Element* e)
      {
      qreal interline = 20.0;
      if (e->staff()) {
            interline *= (e->staff()->spatium(e->tick()) / e->score()->spatium());
            scale     *= (e->score()->spatium() / e->staff()->spatium(e->tick()));
      }
      if (scale == 1.0)
            xml.stag(QString("Symbol interline=\"%1\" shape=\"bracketedTuplet%2\"").arg(interline).arg(num));
      else
            xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"bracketedTuplet%3\"").arg(interline).arg(scale).arg(num));
      QRectF rr(e->pageBoundingRect());
      Tuplet* t = toTuplet(e);
      QPointF* bracketLeft = t->getLeftBracket();
      QPointF* bracketRight = t->getRightBracket();

      qreal widthInner = bracketLeft[2].x() - bracketLeft[1].x();
      qreal heightInner = bracketLeft[0].y() - bracketLeft[1].y();

      int flag = 0;                 // 0 for no slant, 1 for +ve slope, -1 for negative slope

      if (bracketLeft[2].y() == bracketLeft[1].y())
            flag = 0;
      else if (t->isUp())
            flag = 1;
      else
            flag = -1;

      QRectF outerElement = QRectF();
      if (flag == 0) {
            outerElement.setX(rr.x() * spatium);
            outerElement.setY(rr.y() * spatium);
            outerElement.setWidth(rr.width() * spatium);
            outerElement.setHeight(rr.height() * spatium);
            }
      else if (flag == 1) {
            outerElement.setX(rr.x() * spatium);
            outerElement.setY((rr.y() - heightInner) * spatium);
            outerElement.setWidth(rr.width() * spatium);
            outerElement.setHeight((rr.height() + 2 * heightInner) * spatium);
            }
      else {
            outerElement.setX(rr.x() * spatium);
            outerElement.setY((rr.y() +  heightInner) * spatium);
            outerElement.setWidth(rr.width() * spatium);
            outerElement.setHeight((rr.height() - 2 * heightInner) * spatium);
            }

      xml.putLevel();
      xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
         .arg(outerElement.x(),      0, 'f', 3)
         .arg(outerElement.y(),      0, 'f', 3)
         .arg(outerElement.width(),  0, 'f', 3)
         .arg(outerElement.height(), 0, 'f', 3);

      // Calculation for the left bracket

      if (scale == 1.0)
            xml.stag(QString("Symbol interline=\"%1\" shape=\"tupletBracketStart\"").arg(interline));
      else
            xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"tupletBracketStart\"").arg(interline).arg(scale));

      QRectF innerBracketStart = QRectF();
      if (flag == 0) {
            innerBracketStart.setX(rr.x() * spatium);
            innerBracketStart.setY((rr.y() + (rr.height() + (bracketLeft[1].y() - bracketLeft[0].y()))) * spatium);
            innerBracketStart.setWidth(widthInner * spatium);
            innerBracketStart.setHeight(heightInner * spatium);
            }
      else if (flag == 1) {
            innerBracketStart.setX(rr.x() * spatium);
            innerBracketStart.setY((rr.y() + rr.height() + heightInner - (bracketLeft[0].y() - bracketLeft[2].y())) * spatium);
            innerBracketStart.setWidth(widthInner * spatium);
            innerBracketStart.setHeight((bracketLeft[0].y() - bracketLeft[2].y()) * spatium);
            }
      else {
            innerBracketStart.setX(rr.x() * spatium);
            innerBracketStart.setY((rr.y() + heightInner) * spatium);
            innerBracketStart.setWidth(widthInner * spatium);
            innerBracketStart.setHeight((bracketLeft[2].y() - bracketLeft[0].y()) * spatium);
            }

      xml.putLevel();
      xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
         .arg(innerBracketStart.x(),      0, 'f', 3)
         .arg(innerBracketStart.y(),      0, 'f', 3)
         .arg(innerBracketStart.width(),  0, 'f', 3)
         .arg(innerBracketStart.height(), 0, 'f', 3);
      xml.etag();

      // Calculation for the center text element

      if (scale == 1.0)
            xml.stag(QString("Symbol interline=\"%1\" shape=\"tuplet%2\"").arg(interline).arg(num));
      else
            xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"tuplet%3\"").arg(interline).arg(scale).arg(num));
      QRectF centralBounds = t->getText()->pageBoundingRect();
      QRectF tupletInfo(centralBounds.x() * spatium, centralBounds.y() * spatium, centralBounds.width() * spatium, centralBounds.height() * spatium);
      xml.putLevel();
      xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
         .arg(tupletInfo.x(),      0, 'f', 3)
         .arg(tupletInfo.y(),      0, 'f', 3)
         .arg(tupletInfo.width(),  0, 'f', 3)
         .arg(tupletInfo.height(), 0, 'f', 3);
      xml.etag();

      // Calculation for the right bracket

      if (scale == 1.0)
            xml.stag(QString("Symbol interline=\"%1\" shape=\"tupletBracketEnd\"").arg(interline));
      else
            xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"tupletBracketEnd\"").arg(interline).arg(scale));

      QRectF innerBracketEnd = QRectF();
      if (flag == 0) {
            innerBracketEnd.setX((rr.x() + rr.width() - (bracketRight[1].x() - bracketRight[0].x())) * spatium);
            innerBracketEnd.setY((rr.y() + (rr.height() + (bracketRight[1].y() - bracketRight[2].y()))) * spatium);
            innerBracketEnd.setWidth((bracketRight[1].x() - bracketRight[0].x()) * spatium);
            innerBracketEnd.setHeight(heightInner * spatium);
            }
      else if (flag == 1) {
            int height = ((bracketRight[0].y() > bracketRight[2].y()) ? (bracketRight[0].y() - bracketRight[1].y()) : (bracketRight[2].y() - bracketRight[1].y()));
            innerBracketEnd.setX((rr.x() + rr.width() - widthInner) * spatium);
            innerBracketEnd.setY((rr.y() - heightInner + ((rr.height() + 2 * heightInner) + (bracketRight[1].y() - bracketLeft[0].y()))) * spatium);
            innerBracketEnd.setWidth(widthInner * spatium);
            innerBracketEnd.setHeight(height * spatium);
            }
      else {
            int height = (bracketRight[0].y() < bracketRight[2].y() ? (bracketRight[1].y() - bracketRight[0].y()) : (bracketRight[1].y() - bracketRight[2].y()));
            innerBracketEnd.setX((rr.x() + rr.width() - widthInner) * spatium);
            innerBracketEnd.setY((rr.y() + heightInner + ((bracketRight[0].y() < bracketRight[2].y()) ? (bracketRight[0].y() - bracketLeft[0].y()) : bracketRight[2].y() - bracketLeft[0].y())) * spatium);
            innerBracketEnd.setWidth(widthInner * spatium);
            innerBracketEnd.setHeight(height * spatium);
            }

      xml.putLevel();
      xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
         .arg(innerBracketEnd.x(),      0, 'f', 3)
         .arg(innerBracketEnd.y(),      0, 'f', 3)
         .arg(innerBracketEnd.width(),  0, 'f', 3)
         .arg(innerBracketEnd.height(), 0, 'f', 3);
      xml.etag();
      xml.etag();
      }

//---------------------------------------------------------
//   writeTimeSigData
//---------------------------------------------------------

static void writeTimeSigData(XmlWriter& xml, qreal spatium, qreal scale, int num, int denom, const Element* e)
      {
      qreal interline = 20.0;
      if (e->staff()) {
            interline *= (e->staff()->spatium(e->tick()) / e->score()->spatium());
            scale     *= (e->score()->spatium() / e->staff()->spatium(e->tick()));
      }
      QString name = QString("timeSig%1over%2").arg(num).arg(denom);
      if (scale == 1.0)
            xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(interline).arg(name));
      else
            xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"%3\"").arg(interline).arg(scale).arg(name));
      QRectF rr(e->pageBoundingRect());
      QRectF outerElement(rr.x() * spatium, rr.y() * spatium, rr.width() * spatium, rr.height() * spatium);
      xml.putLevel();
      xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
         .arg(outerElement.x(),      0, 'f', 3)
         .arg(outerElement.y(),      0, 'f', 3)
         .arg(outerElement.width(),  0, 'f', 3)
         .arg(outerElement.height(), 0, 'f', 3);

      // Nested Symbol start

      ScoreFont* font = e->score()->scoreFont();
      const TimeSig* ts = toTimeSig(e);
      QRectF numRect = font->bbox(ts->getNs(), spatium);
      QRectF denomRect = font->bbox(ts->getDs(), spatium);

      qreal midHorizontal = rr.x() + (rr.width()) / 2;
      qreal midVertical = rr.y() + (rr.height()) / 2;

      // Numerator Info

      name = QString("timeSig%1").arg(num);
      if (scale == 1.0)
            xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(interline).arg(name));
      else
            xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"%3\"").arg(interline).arg(scale).arg(name));
      QRectF numInfo((midHorizontal * spatium - (numRect.width() / 2)), rr.y() * spatium, numRect.width(), numRect.height());
      xml.putLevel();
      xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
         .arg(numInfo.x(),      0, 'f', 3)
         .arg(numInfo.y(),      0, 'f', 3)
         .arg(numInfo.width(),  0, 'f', 3)
         .arg(numInfo.height(), 0, 'f', 3);
      xml.etag();

      // Denominator Info

      name = QString("timeSig%1").arg(denom);
      if (scale == 1.0)
             xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(interline).arg(name));
      else
             xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"%3\"").arg(interline).arg(scale).arg(name));
      QRectF denomInfo((midHorizontal * spatium - (denomRect.width() / 2)), midVertical * spatium, denomRect.width(), denomRect.height());
      xml.putLevel();
      xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
         .arg(denomInfo.x(),      0, 'f', 3)
         .arg(denomInfo.y(),      0, 'f', 3)
         .arg(denomInfo.width(),  0, 'f', 3)
         .arg(denomInfo.height(), 0, 'f', 3);
      xml.etag();

      // End

      xml.etag();
      }

//---------------------------------------------------------
//   writeRepeatDotData
//---------------------------------------------------------

static void writeRepeatDotData(XmlWriter& xml, qreal spatium, qreal scale, QString name, const Element* e)
      {
      qreal interline = 20.0;
      if (e->staff()) {
            interline *= (e->staff()->spatium(e->tick()) / e->score()->spatium());
            scale     *= (e->score()->spatium() / e->staff()->spatium(e->tick()));
      }
      if (scale == 1.0)
            xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(interline).arg(name));
      else
            xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"%3\"").arg(interline).arg(scale).arg(name));
      QRectF rr(e->pageBoundingRect());
      QRectF outerElement(rr.x() * spatium, rr.y() * spatium, rr.width() * spatium, rr.height() * spatium);
      xml.putLevel();
      xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
         .arg(outerElement.x(),      0, 'f', 3)
         .arg(outerElement.y(),      0, 'f', 3)
         .arg(outerElement.width(),  0, 'f', 3)
         .arg(outerElement.height(), 0, 'f', 3);

      // Nested Symbol start

      const BarLine* bl = toBarLine(e);
      bool revFlag = false;
      if (bl->barLineType() == BarLineType::END_REPEAT)
            revFlag = true;

      QRectF thickBar(e->symBbox(SymId::barlineHeavy));
      QRectF thinBar(e->symBbox(SymId::barlineSingle));
      QRectF aDot(e->symBbox(SymId::augmentationDot));

      if (!revFlag)
            {
                // Thick Barline

                QString n = "barlineHeavy";
                if (scale == 1.0)
                    xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(interline).arg(n));
                else
                    xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"%3\"").arg(interline).arg(scale).arg(n));

                QRectF hBar = QRectF(rr.x() * spatium, rr.y() * spatium, (4 * thickBar.width() / 5) * spatium, thickBar.height() * spatium);
                xml.putLevel();
                xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
                .arg(hBar.x(),      0, 'f', 3)
                .arg(hBar.y(),      0, 'f', 3)
                .arg(hBar.width(),  0, 'f', 3)
                .arg(hBar.height(), 0, 'f', 3);
                xml.etag();

                // Thin Barline

                n = "barlineSingle";
                if (scale == 1.0)
                    xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(interline).arg(n));
                else
                    xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"%3\"").arg(interline).arg(scale).arg(n));

                QRectF lBar = QRectF((rr.x() + rr.width() - aDot.width() - (3 * thinBar.width() / 2)) * spatium, rr.y() * spatium, (thinBar.width() / 2) * spatium, thinBar.height() * spatium);
                xml.putLevel();
                xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
                .arg(lBar.x(),      0, 'f', 3)
                .arg(lBar.y(),      0, 'f', 3)
                .arg(lBar.width(),  0, 'f', 3)
                .arg(lBar.height(), 0, 'f', 3);
                xml.etag();

                // Dots

                n = "augmentationDot";
                if (scale == 1.0)
                    xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(interline).arg(n));
                else
                    xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"%3\"").arg(interline).arg(scale).arg(n));

                QRectF dotOne = QRectF();
                if (revFlag) {
                    dotOne = QRectF(rr.x() * spatium, (rr.y() + (rr.height() / 2) - (3 * aDot.height() / 2)) * spatium, aDot.width() * spatium, aDot.height() * spatium);
                }
                else {
                    dotOne = QRectF((rr.x() + rr.width() - aDot.width() + 1) * spatium, (rr.y() + (rr.height() / 2) - (3 * aDot.height() / 2)) * spatium, (aDot.width() - 1) * spatium, aDot.height() * spatium);
                }

                xml.putLevel();
                xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
                .arg(dotOne.x(),      0, 'f', 3)
                .arg(dotOne.y(),      0, 'f', 3)
                .arg(dotOne.width(),  0, 'f', 3)
                .arg(dotOne.height(), 0, 'f', 3);
                xml.etag();

                if (scale == 1.0)
                    xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(interline).arg(n));
                else
                    xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"%3\"").arg(interline).arg(scale).arg(n));

                QRectF dotTwo = QRectF();
                if (revFlag)
                      dotTwo = QRectF(rr.x() * spatium, (rr.y() + (rr.height() + (3 * aDot.height() / 2)) / 2) * spatium, aDot.width() * spatium, aDot.height() * spatium);
                else
                      dotTwo = QRectF((rr.x() + rr.width() - aDot.width() + 1) * spatium, (rr.y() + (rr.height() + (3 * aDot.height() / 2)) / 2) * spatium, (aDot.width() - 1) * spatium, aDot.height() * spatium);

                xml.putLevel();
                xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
                .arg(dotTwo.x(),      0, 'f', 3)
                .arg(dotTwo.y(),      0, 'f', 3)
                .arg(dotTwo.width(),  0, 'f', 3)
                .arg(dotTwo.height(), 0, 'f', 3);
                xml.etag();

            }
      else
            {

                // Dots

                QString n = "augmentationDot";
                if (scale == 1.0)
                    xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(interline).arg(n));
                else
                    xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"%3\"").arg(interline).arg(scale).arg(n));

                QRectF dotOne = QRectF();
                if (revFlag)
                    dotOne = QRectF(rr.x() * spatium, (rr.y() + (rr.height() / 2) - (3 * aDot.height() / 2)) * spatium, aDot.width() * spatium, aDot.height() * spatium);
                else
                    dotOne = QRectF((rr.x() + rr.width() - aDot.width() + 1) * spatium, (rr.y() + (rr.height() / 2) - (3 * aDot.height() / 2)) * spatium, (aDot.width() - 1) * spatium, aDot.height() * spatium);

                xml.putLevel();
                xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
                .arg(dotOne.x(),      0, 'f', 3)
                .arg(dotOne.y(),      0, 'f', 3)
                .arg(dotOne.width(),  0, 'f', 3)
                .arg(dotOne.height(), 0, 'f', 3);
                xml.etag();

                if (scale == 1.0)
                    xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(interline).arg(n));
                else
                    xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"%3\"").arg(interline).arg(scale).arg(n));

                QRectF dotTwo = QRectF();
                if (revFlag)
                    dotTwo = QRectF(rr.x() * spatium, (rr.y() + (rr.height() + (3 * aDot.height() / 2)) / 2) * spatium, aDot.width() * spatium, aDot.height() * spatium);
                else
                    dotTwo = QRectF((rr.x() + rr.width() - aDot.width() + 1) * spatium, (rr.y() + (rr.height() + (3 * aDot.height() / 2)) / 2) * spatium, (aDot.width() - 1) * spatium, aDot.height() * spatium);

                xml.putLevel();
                xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
                .arg(dotTwo.x(),      0, 'f', 3)
                .arg(dotTwo.y(),      0, 'f', 3)
                .arg(dotTwo.width(),  0, 'f', 3)
                .arg(dotTwo.height(), 0, 'f', 3);
                xml.etag();

                // Thin BarLine

                n = "barlineSingle";
                if (scale == 1.0)
                    xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(interline).arg(n));
                else
                    xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"%3\"").arg(interline).arg(scale).arg(n));

                QRectF lBar = QRectF();
                if (revFlag)
                    lBar = QRectF((rr.x() + aDot.width() + (3 * thinBar.width() / 2)) * spatium, rr.y() * spatium, (thinBar.width() / 2) * spatium, thinBar.height() * spatium);
                else
                    lBar = QRectF((rr.x() + rr.width() - aDot.width() - (3 * thinBar.width() / 2)) * spatium, rr.y() * spatium, (thinBar.width() / 2) * spatium, thinBar.height() * spatium);

                xml.putLevel();
                xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
                .arg(lBar.x(),      0, 'f', 3)
                .arg(lBar.y(),      0, 'f', 3)
                .arg(lBar.width(),  0, 'f', 3)
                .arg(lBar.height(), 0, 'f', 3);
                xml.etag();

                // Thick BarLine

                n = "barlineHeavy";
                if (scale == 1.0)
                    xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(interline).arg(n));
                else
                    xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"%3\"").arg(interline).arg(scale).arg(n));

                QRectF hBar = QRectF();
                if (revFlag)
                    hBar = QRectF((rr.x() + rr.width() - (4 * thickBar.width() / 5)) * spatium, rr.y() * spatium, (4 * thickBar.width() / 5) * spatium, thickBar.height() * spatium);
                else
                    hBar = QRectF(rr.x() * spatium, rr.y() * spatium, (4 * thickBar.width() / 5) * spatium, thickBar.height() * spatium);

                xml.putLevel();
                xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
                .arg(hBar.x(),      0, 'f', 3)
                .arg(hBar.y(),      0, 'f', 3)
                .arg(hBar.width(),  0, 'f', 3)
                .arg(hBar.height(), 0, 'f', 3);
                xml.etag();

            }
      xml.etag();
      }

//---------------------------------------------------------
//   writeGraceData
//---------------------------------------------------------

static void writeGraceData(XmlWriter& xml, qreal spatium, qreal scale, const QString& name, const Element* e, qreal smag)
    {
        const Note* n = toNote(e);
        const Chord* ch = n->chord();
        const Hook* h = ch->hook();

        if (ch->hook() == NULL)
              return;

        QString addString = "StemUp";
        bool stemFlag = false;   // false if Stem is Up

        if (n->chord()->stemDirection() == Direction::DOWN) {
              addString = "StemDown";
              stemFlag = true;
              }

        qreal interline = 20.0;
        if (e->staff()) {
            interline *= (e->staff()->spatium(e->tick()) / e->score()->spatium());
            scale     *= (e->score()->spatium() / e->staff()->spatium(e->tick()));
        }

        if (scale == 1.0)
            xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(interline).arg(name + addString));
        else
            xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"%3\"").arg(interline).arg(scale).arg(name + addString));

        QRectF stemEl(e->symBbox(SymId::stem));
        QRectF hookEl(h->pageBoundingRect());
        QRectF noteEl(e->pageBoundingRect());

        QRectF nEl(noteEl.x() * spatium, noteEl.y() * spatium, noteEl.width() * spatium, noteEl.height() * spatium);
        QRectF hEl(hookEl.x() * spatium, hookEl.y() * spatium, hookEl.width() * spatium, hookEl.height() * spatium);

        QRectF outerElement = QRectF();
        if (!stemFlag) {
              outerElement.setX(nEl.x());
              outerElement.setY(hEl.y());
              if (name == "graceNoteAcciaccatura") {
                    outerElement.setWidth(nEl.width() + hEl.width() + (stemEl.width() * spatium));
                    }
              else {
                    outerElement.setWidth(nEl.width() + hEl.width() - (stemEl.width() * spatium));
                    }
              outerElement.setHeight(nEl.y() + nEl.height() - hEl.y());
              }
        else {
              if (name == "graceNoteAcciaccatura")
                  outerElement.setX(nEl.x() - (nEl.width() / 2));
              else
                  outerElement.setX(nEl.x());
              outerElement.setY(nEl.y());
              if (name == "graceNoteAcciaccatura")
                  outerElement.setWidth(nEl.width() + nEl.width() / 2);
              else
                  outerElement.setWidth(nEl.width());
              outerElement.setHeight(nEl.height() + hEl.height());
              }

        xml.putLevel();
        xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
        .arg(outerElement.x(),      0, 'f', 3)
        .arg(outerElement.y(),      0, 'f', 3)
        .arg(outerElement.width(),  0, 'f', 3)
        .arg(outerElement.height(), 0, 'f', 3);

        if (!stemFlag) {
              QString str;

              // Note Element

              SymId sym = n->noteHead();
              str = Sym::id2name(sym);
              if (scale == 1.0)
                  xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(interline).arg(str));
              else
                  xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"%3\"").arg(interline).arg(scale).arg(str));
              xml.putLevel();
              xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
              .arg(nEl.x(),      0, 'f', 3)
              .arg(nEl.y(),      0, 'f', 3)
              .arg(nEl.width(),  0, 'f', 3)
              .arg(nEl.height(), 0, 'f', 3);
              xml.etag();

              // Stem

              str = "stem";
              if ((scale / smag) == 1.0)
                  xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(interline).arg(str));
              else
                  xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"%3\"").arg(interline).arg(scale / smag).arg(str));
              xml.putLevel();
              QRectF sEl(hEl.x(), outerElement.y(), nEl.x() + nEl.width() - hEl.x(), stemEl.height() * spatium);
              xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
              .arg(sEl.x(),      0, 'f', 3)
              .arg(sEl.y(),      0, 'f', 3)
              .arg(sEl.width(),  0, 'f', 3)
              .arg(sEl.height(), 0, 'f', 3);
              xml.etag();

              // Hook

              sym = h->sym();
              str = Sym::id2name(sym);
              if (scale == 1.0)
                  xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(interline).arg(str));
              else
                  xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"%3\"").arg(interline).arg(scale).arg(str));
              xml.putLevel();
              xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
              .arg(hEl.x(),      0, 'f', 3)
              .arg(hEl.y(),      0, 'f', 3)
              .arg(hEl.width(),  0, 'f', 3)
              .arg(hEl.height(), 0, 'f', 3);
              xml.etag();

              // Check for Slash

              if (name == "graceNoteAcciaccatura") {
                    str = "graceNoteSlashStemUp";
                    QRectF slashEl(e->symBbox(SymId::graceNoteSlashStemUp));
                    if (scale == 1.0)
                        xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(interline).arg(str));
                    else
                        xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"%3\"").arg(interline).arg(scale).arg(str));
                    xml.putLevel();
                    QRectF stemUpEl(hEl.x() - (nEl.width() / 2), hEl.y() + (0.25 * outerElement.height()), outerElement.x() + outerElement. width() - hEl.x() + (nEl.width() / 2), slashEl.height() * spatium);
                    xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
                    .arg(stemUpEl.x(),      0, 'f', 3)
                    .arg(stemUpEl.y(),      0, 'f', 3)
                    .arg(stemUpEl.width(),  0, 'f', 3)
                    .arg(stemUpEl.height(), 0, 'f', 3);
                    xml.etag();
                    }
              }
        else {
              QString str;

              // Stem

              str = "stem";
              if ((scale / smag) == 1.0)
                  xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(interline).arg(str));
              else
                  xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"%3\"").arg(interline).arg(scale / smag).arg(str));
              xml.putLevel();
              QRectF sEl(hEl.x(), nEl.y() + (nEl.height() / 2), stemEl.width() * spatium, stemEl.height() * spatium);
              xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
              .arg(sEl.x(),      0, 'f', 3)
              .arg(sEl.y(),      0, 'f', 3)
              .arg(sEl.width(),  0, 'f', 3)
              .arg(sEl.height(), 0, 'f', 3);
              xml.etag();

              // Note Element

              SymId sym = n->noteHead();
              str = Sym::id2name(sym);
              if (scale == 1.0)
                  xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(interline).arg(str));
              else
                  xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"%3\"").arg(interline).arg(scale).arg(str));
              xml.putLevel();
              xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
              .arg(nEl.x(),      0, 'f', 3)
              .arg(nEl.y(),      0, 'f', 3)
              .arg(nEl.width(),  0, 'f', 3)
              .arg(nEl.height(), 0, 'f', 3);
              xml.etag();

              // Hook

              sym = h->sym();
              str = Sym::id2name(sym);
              if (scale == 1.0)
                  xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(interline).arg(str));
              else
                  xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"%3\"").arg(interline).arg(scale).arg(str));
              xml.putLevel();
              xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
              .arg(hEl.x(),      0, 'f', 3)
              .arg(hEl.y(),      0, 'f', 3)
              .arg(hEl.width(),  0, 'f', 3)
              .arg(hEl.height(), 0, 'f', 3);
              xml.etag();

              // Check for Slash

              if (name == "graceNoteAcciaccatura") {
                    str = "graceNoteSlashStemUp";
                    QRectF slashEl(e->symBbox(SymId::graceNoteSlashStemUp));
                    if (scale == 1.0)
                        xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(interline).arg(str));
                    else
                        xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"%3\"").arg(interline).arg(scale).arg(str));
                    xml.putLevel();
                    QRectF stemUpEl(hEl.x() - (nEl.width() / 2), nEl.y() + (0.4 * outerElement.height()), outerElement.x() + outerElement. width() - hEl.x() + (nEl.width() / 2), slashEl.height() * spatium);
                    xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
                    .arg(stemUpEl.x(),      0, 'f', 3)
                    .arg(stemUpEl.y(),      0, 'f', 3)
                    .arg(stemUpEl.width(),  0, 'f', 3)
                    .arg(stemUpEl.height(), 0, 'f', 3);
                    xml.etag();
                    }
              }
        xml.etag();
    }


//---------------------------------------------------------
//   writeSymbol
//---------------------------------------------------------

static void writeSymbol(XmlWriter& xml, qreal mag, const QString& name, const Element* e, SymId sym, QPointF p)
      {
      xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(20).arg(name));
      QRectF bbox = e->score()->scoreFont()->bbox(sym, e->magS());
      QRectF rr   = bbox.translated(p);
      QRectF r(rr.x() * mag, rr.y() * mag, rr.width() * mag, rr.height() * mag);

      xml.putLevel();
      xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
         .arg(r.x(),      0, 'f', 3)
         .arg(r.y(),      0, 'f', 3)
         .arg(r.width(),  0, 'f', 3)
         .arg(r.height(), 0, 'f', 3);
      xml.etag();
      }

//---------------------------------------------------------
//   writeEdata
//---------------------------------------------------------

void MuseScore::writeEdata(const QString& edataName, const QString& imageName, Score* score, qreal gmag, const QList<Element*>& pel)
      {
      QFile f(edataName);
      if (!f.open(QIODevice::WriteOnly)) {
            MScore::lastError = tr("Open Element metadata xml file\n%1\nfailed: %2").arg(f.fileName().arg(QString(strerror(errno))));
            return;
            }
      XmlWriter xml(score, &f);
      xml.header();
      xml.stag("Annotations version=\"1.0\"");
      xml.tag("Source", "MuseScore 3.0");

      for (Element* e : pel) {
          if (e->isPage()) {
                  xml.stag("Page");
                  QUrl url = QUrl::fromLocalFile(imageName);
//                  xml.tag("Image", url.toDisplayString());
                  xml.tag("Image", QString("%1").arg(imageName));  // no path
                  QRectF r(e->pageBoundingRect());
                  xml.tag("Size", QVariant(QSize(lrint(r.width() * gmag), lrint(r.height() * gmag))));
                  xml.etag();
                  break;
                  }
            }
      for (Element* e : pel) {
            if (!e->visible())
                  continue;
            Staff* staff = e->staff();

            qreal smag = staff ? score->spatium() / staff->spatium(e->tick()) : gmag;
            qreal mag  = gmag * smag;
            switch (e->type()) {
                  case ElementType::NOTEDOT:
                        writeData(xml, mag, e->mag(), "augmentationDot", e);
                        break;

                  case ElementType::STEM: {
                        const Stem* s = toStem(e);
                        const Chord* ch = s->chord();
                        if (ch->isGrace())
                              break;
                        writeData(xml, mag, e->mag() / smag, "stem", e);
                        }
                        break;
#if 0
                  case ElementType::HAIRPIN_SEGMENT:
                        writeData(xml, mag, e->mag(), e->name(), e);
                        break;
#endif
                  case ElementType::ARTICULATION: {
                        const Articulation* a = toArticulation(e);
                        SymId symId = a->symId();
                        QString symName = Sym::id2name(symId);
                        writeData(xml, mag, a->mag(), symName, a);
                        }
                        break;

                  case ElementType::CLEF: {
                        const Clef* c = toClef(e);
                        switch (c->clefType()) {
                              case ClefType::C1:
                              case ClefType::C2:
                              case ClefType::C3:
                              case ClefType::C4:
                              case ClefType::C5:
                                    writeData(xml, mag, c->mag(), "cClef", c);
                                    break;
                              case ClefType::C_19C:
                                    writeData(xml, mag, c->mag(), "cClefSquare", c);
                                    break;
                              case ClefType::C3_F18C:
                              case ClefType::C4_F18C:
                                    writeData(xml, mag, c->mag(), "cClefFrench", c);
                                    break;
                              case ClefType::G:
                              case ClefType::G_1:
                                    writeData(xml, mag, c->mag(), "gClef", c);
                                    break;
                              case ClefType::G8_VA:
                                    writeData(xml, mag, c->mag(), "gClef8va", c);
                                    break;
                              case ClefType::G8_VB:
                                    writeData(xml, mag, c->mag(), "gClef8vb", c);
                                    break;
                              case ClefType::G15_MA:
                                    writeData(xml, mag, c->mag(), "gClef15ma", c);
                                    break;
                              case ClefType::G15_MB:
                                    writeData(xml, mag, c->mag(), "gClef15mb", c);
                                    break;
                              case ClefType::F:
                              case ClefType::F_B:
                              case ClefType::F_C:
                                    writeData(xml, mag, c->mag(), "fClef", c);
                                    break;
                              case ClefType::F_8VA:
                                    writeData(xml, mag, c->mag(), "fClef8va", c);
                                    break;
                              case ClefType::F8_VB:
                                    writeData(xml, mag, c->mag(), "fClef8vb", c);
                                    break;
                              case ClefType::F_15MA:
                                    writeData(xml, mag, c->mag(), "fClef15ma", c);
                                    break;
                              case ClefType::F15_MB:
                                    writeData(xml, mag, c->mag(), "fClef15mb", c);
                                    break;
                              case ClefType::PERC:
                                    writeData(xml, mag, c->mag(), "unpitchedPercussionClef1", c);
                                    break;
                              default:
                                    break;
                              }
                        }
                        break;

                  case ElementType::NOTE: {
                        const Note* n = toNote(e);
                        SymId symId = n->noteHead();
                        QString symName = Sym::id2name(symId);
                        switch (n->noteType()) {
                              case NoteType::NORMAL:
                                    writeData(xml, mag, n->mag(), symName , n);
                                    break;
                              case NoteType::ACCIACCATURA:
                                    writeGraceData(xml, mag, n->mag(), "graceNoteAcciaccatura", n, smag);
                                    break;
                              case NoteType::APPOGGIATURA:
                                    writeGraceData(xml, mag, n->mag(), "graceNoteAppoggiatura", n, smag);
                                    break;
                              case NoteType::GRACE4:
                                    writeData(xml, mag, n->mag(), "graceNote4", n);
                                    break;
                              case NoteType::GRACE16:
                                    writeData(xml, mag, n->mag(), "graceNote16", n);
                                    break;
                              case NoteType::GRACE32:
                                    writeData(xml, mag, n->mag(), "graceNote32", n);
                                    break;
                              case NoteType::GRACE8_AFTER:
                                    writeData(xml, mag, n->mag(), "graceNote8_After", n);
                                    break;
                              case NoteType::GRACE16_AFTER:
                                    writeData(xml, mag, n->mag(), "graceNote16_After", n);
                                    break;
                              case NoteType::GRACE32_AFTER:
                                    writeData(xml, mag, n->mag(), "graceNote32_After", n);
                                    break;
                              case NoteType::INVALID:
                                    break;
                              }
                        }
                        break;

                  case ElementType::ACCIDENTAL: {
                        const Accidental* a = toAccidental(e);
                        switch (a->accidentalType()) {
                              case AccidentalType::SHARP:
                                    writeData(xml, mag, a->mag(), "accidentalSharp", a);
                                    break;
                              case AccidentalType::FLAT:
                                    writeData(xml, mag, a->mag(), "accidentalFlat", a);
                                    break;
                              case AccidentalType::SHARP2:
                                    writeData(xml, mag, a->mag(), "accidentalDoubleSharp", a);
                                    break;
                              case AccidentalType::FLAT2:
                                    writeData(xml, mag, a->mag(), "accidentalDoubleFlat", a);
                                    break;
                              case AccidentalType::NATURAL:
                                    writeData(xml, mag, a->mag(), "accidentalNatural", a);
                                    break;
                              default:
                                    break;
                              }
                        }
                        break;

                  case ElementType::KEYSIG: {
                        const KeySig* k = toKeySig(e);
                        QPointF p (k->pagePos());
                        for (const KeySym& ks: k->keySigEvent().keySymbols()) {
                              QString name;
                              switch (ks.sym) {
                                    case SymId::accidentalSharp:
                                          name = "keySharp";
                                          break;
                                    case SymId::accidentalFlat:
                                          name = "keyFlat";
                                          break;
                                    case SymId::accidentalNatural:
                                          name = "keyNatural";
                                          break;
                                    default:
                                          name = "unknown";
                                          break;
                                    }
                              writeSymbol(xml, mag, name, k, ks.sym, p + ks.pos);
                              }
                        };
                        break;

                  case ElementType::HOOK: {
                        const Hook* h = toHook(e);
                        const Chord* c = h->chord();
                        if (c->isGrace())
                              break;
                        switch (h->hookType()) {
                              default:
                              case 0:
                                    break;
                              case -1:
                                    writeData(xml, mag, h->mag(), "flag8thDown", h);
                                    break;
                              case -2:
                                    writeData(xml, mag, h->mag(), "flag16thDown", h);
                                    break;
                              case -3:
                                    writeData(xml, mag, h->mag(), "flag32ndDown", h);
                                    break;
                              case -4:
                                    writeData(xml, mag, h->mag(), "flag64thDown", h);
                                    break;
                              case -5:
                                    writeData(xml, mag, h->mag(), "flag128thDown", h);
                                    break;
                              case -6:
                                    writeData(xml, mag, h->mag(), "flag256thDown", h);
                                    break;
                              case -7:
                                    writeData(xml, mag, h->mag(), "flag512thDown", h);
                                    break;
                              case -8:
                                    writeData(xml, mag, h->mag(), "flag1024thDown", h);
                                    break;
                              case 1:
                                    writeData(xml, mag, h->mag(), "flag8thUp", h);
                                    break;
                              case 2:
                                    writeData(xml, mag, h->mag(), "flag16thUp", h);
                                    break;
                              case 3:
                                    writeData(xml, mag, h->mag(), "flag32ndUp", h);
                                    break;
                              case 4:
                                    writeData(xml, mag, h->mag(), "flag64thUp", h);
                                    break;
                              case 5:
                                    writeData(xml, mag, h->mag(), "flag128thUp", h);
                                    break;
                              case 6:
                                    writeData(xml, mag, h->mag(), "flag256thUp", h);
                                    break;
                              case 7:
                                    writeData(xml, mag, h->mag(), "flag512thUp", h);
                                    break;
                              case 8:
                                    writeData(xml, mag, h->mag(), "flag1024thUp", h);
                                    break;
                              }
                        }
                        break;

                  case ElementType::REST: {
                        const Rest* r = toRest(e);
                        switch (r->durationType().type()) {
                              case TDuration::DurationType::V_1024TH:
                                    writeRestData(xml, mag, r->mag(), "rest1024th", r);
                                    break;
                              case TDuration::DurationType::V_512TH:
                                    writeRestData(xml, mag, r->mag(), "rest512th", r);
                                    break;
                              case TDuration::DurationType::V_256TH:
                                    writeRestData(xml, mag, r->mag(), "rest256th", r);
                                    break;
                              case TDuration::DurationType::V_128TH:
                                    writeRestData(xml, mag, r->mag(), "rest128th", r);
                                    break;
                              case TDuration::DurationType::V_64TH:
                                    writeRestData(xml, mag, r->mag(), "rest64th", r);
                                    break;
                              case TDuration::DurationType::V_32ND:
                                    writeRestData(xml, mag, r->mag(), "rest32nd", r);
                                    break;
                              case TDuration::DurationType::V_16TH:
                                    writeRestData(xml, mag, r->mag(), "rest16th", r);
                                    break;
                              case TDuration::DurationType::V_EIGHTH:
                                    writeRestData(xml, mag, r->mag(), "rest8th", r);
                                    break;
                              case TDuration::DurationType::V_QUARTER:
                                    writeRestData(xml, mag, r->mag(), "restQuarter", r);
                                    break;
                              case TDuration::DurationType::V_HALF:
                                    writeRestData(xml, mag, r->mag(), "restHalf", r);
                                    break;
                              case TDuration::DurationType::V_WHOLE:
                              case TDuration::DurationType::V_MEASURE:
                                    writeRestData(xml, mag, r->mag(), "restWhole", r);
                                    break;
                              case TDuration::DurationType::V_BREVE:
                                    writeRestData(xml, mag, r->mag(), "restDoubleWhole", r);
                                    break;
                              case TDuration::DurationType::V_LONG:
                                    writeRestData(xml, mag, r->mag(), "restLonga", r);
                                    break;
                              default:
                                    break;
                              }
                        }
                        break;

                  case ElementType::TIMESIG: {
                        const TimeSig* ts = toTimeSig(e);
                        Fraction sig      = ts->sig();
                        TimeSigType t     = ts->timeSigType();
                        switch (t) {
                              case TimeSigType::NORMAL:
                                    writeTimeSigData(xml, mag, ts->mag(), sig.numerator(), sig.denominator(), ts);
                                    break;
                              case TimeSigType::FOUR_FOUR:
                                    writeData(xml, mag, ts->mag(), "timeSigCommon", ts);
                                    break;
                              case TimeSigType::ALLA_BREVE:
                                    writeData(xml, mag, ts->mag(), "timeSigCutCommon", ts);
                                    break;
                              }
                        }
                        break;

                  case ElementType::BAR_LINE: {
                        const BarLine* bl = toBarLine(e);
                        switch (bl->barLineType()) {
                              case BarLineType::NORMAL:
                                    writeData(xml, mag, bl->mag(), "barlineSingle", bl);
                                    break;
                              case BarLineType::DOUBLE:
                                    writeData(xml, mag, bl->mag(), "barlineDouble", bl);
                                    break;
                              case BarLineType::START_REPEAT:
                                    writeRepeatDotData(xml, mag, bl->mag(), "repeatLeft", bl);
                                    break;
                              case BarLineType::END_REPEAT:
                                    writeRepeatDotData(xml, mag, bl->mag(), "repeatRight", bl);
                                    break;
                              case BarLineType::BROKEN:
                                    writeData(xml, mag, bl->mag(), "barlineDashed", bl);
                                    break;
                              case BarLineType::END:
                                    writeData(xml, mag, bl->mag(), "barlineSingle", bl);
                                    break;
                              case BarLineType::DOTTED:
                                    writeData(xml, mag, bl->mag(), "barlineDotted", bl);
                                    break;
                              default:
                                    break;
                              }
                        }
                        break;

                  case ElementType::MARKER: {
                        const Marker* m = toMarker(e);
                        switch (m->markerType()) {
                              case Marker::Type::SEGNO:
                              case Marker::Type::VARSEGNO:
                                    writeData(xml, mag, m->mag(), "segno", m);
                                    break;
                              case Marker::Type::VARCODA:
                                    writeData(xml, mag, m->mag(), "codaSquare", m);
                                    break;
                              case Marker::Type::CODA:
                                    writeData(xml, mag, m->mag(), "coda", m);
                                    break;
                              case Marker::Type::FINE:
                                    writeData(xml, mag, m->mag(), "fine", m);
                                    break;
                              case Marker::Type::TOCODA:
                                    writeData(xml, mag, m->mag(), "toCoda", m);
                                    break;
                              case Marker::Type::CODETTA:
                                    writeData(xml, mag, m->mag(), "unknown", m);
                                    break;
                              default:
                                    break;
                              }
                        }
                        break;

                  case ElementType::TUPLET: {
                        Tuplet* t = toTuplet(e);
                        Fraction f = t->ratio();
                        bool hasBracket = t->hasBracket();
                        QString s = QString("tuplet%1").arg(f.numerator());
                        switch (f.numerator()) {
                              case 2:
                                    if (hasBracket)
                                          writeTupletData(xml, mag, t->mag(), 2, t);
                                    else
                                          writeData(xml, mag, t->mag(), s, t);
                              case 3:
                                    if (hasBracket)
                                          writeTupletData(xml, mag, t->mag(), 3, t);
                                    else
                                          writeData(xml, mag, t->mag(), s, t);
                                    break;
                              case 4:
                                    if (hasBracket)
                                          writeTupletData(xml, mag, t->mag(), 4, t);
                                    else
                                          writeData(xml, mag, t->mag(), s, t);
                                    break;
                              case 5:
                                    if (hasBracket)
                                          writeTupletData(xml, mag, t->mag(), 5, t);
                                    else
                                          writeData(xml, mag, t->mag(), s, t);
                                    break;
                              case 6:
                                    if (hasBracket)
                                          writeTupletData(xml, mag, t->mag(), 6, t);
                                    else
                                          writeData(xml, mag, t->mag(), s, t);
                                    break;
                              case 7:
                                    if (hasBracket)
                                          writeTupletData(xml, mag, t->mag(), 7, t);
                                    else
                                          writeData(xml, mag, t->mag(), s, t);
                                    break;
                              case 8:
                                    if (hasBracket)
                                          writeTupletData(xml, mag, t->mag(), 8, t);
                                    else
                                          writeData(xml, mag, t->mag(), s, t);
                                    break;
                              case 9:
                                    if (hasBracket)
                                          writeTupletData(xml, mag, t->mag(), 9, t);
                                    else
                                          writeData(xml, mag, t->mag(), s, t);
                                    break;
                              default:
                                    break;
                              }
                        }
                        break;

                  case ElementType::BRACKET: {
                        const Bracket* b = toBracket(e);
                        switch (b->bracketType()) {
                              case BracketType::BRACE:
                                    writeData(xml, mag, b->mag(), "brace", b);
                                    break;
                              case BracketType::NORMAL:
                                    writeData(xml, mag, b->mag(), "bracketNormal", b);
                                    break;
                              case BracketType::SQUARE:
                                    writeData(xml, mag, b->mag(), "bracketSquare", b);
                                    break;
                              case BracketType::LINE:
                                    writeData(xml, mag, b->mag(), "bracketLine", b);
                                    break;
                              default:
                                    break;
                              }
                        }
                        break;

                  case ElementType::DYNAMIC: {
                        const Dynamic* d = toDynamic(e);
                        switch (d->dynamicType()) {
                              case Dynamic::Type::PPPPPP:
                                    writeData(xml, mag, d->mag(), "dynamicPPPPPP", d);
                                    break;
                              case Dynamic::Type::PPPPP:
                                    writeData(xml, mag, d->mag(), "dynamicPPPPP", d);
                                    break;
                              case Dynamic::Type::PPPP:
                                    writeData(xml, mag, d->mag(), "dynamicPPPP", d);
                                    break;
                              case Dynamic::Type::PPP:
                                    writeData(xml, mag, d->mag(), "dynamicPPP", d);
                                    break;
                              case Dynamic::Type::PP:
                                    writeData(xml, mag, d->mag(), "dynamicPP", d);
                                    break;
                              case Dynamic::Type::P:
                                    writeData(xml, mag, d->mag(), "dynamicPiano", d);
                                    break;
                              case Dynamic::Type::MP:
                                    writeData(xml, mag, d->mag(), "dynamicMP", d);
                                    break;
                              case Dynamic::Type::MF:
                                    writeData(xml, mag, d->mag(), "dynamicMF", d);
                                    break;
                              case Dynamic::Type::F:
                                    writeData(xml, mag, d->mag(), "dynamicForte", d);
                                    break;
                              case Dynamic::Type::FF:
                                    writeData(xml, mag, d->mag(), "dynamicFF", d);
                                    break;
                              case Dynamic::Type::FFF:
                                    writeData(xml, mag, d->mag(), "dynamicFFF", d);
                                    break;
                              case Dynamic::Type::FFFF:
                                    writeData(xml, mag, d->mag(), "dynamicFFFF", d);
                                    break;
                              case Dynamic::Type::FFFFF:
                                    writeData(xml, mag, d->mag(), "dynamicFFFF", d);
                                    break;
                              case Dynamic::Type::FFFFFF:
                                    writeData(xml, mag, d->mag(), "dynamicFFFFF", d);
                                    break;
                              case Dynamic::Type::FP:
                                    writeData(xml, mag, d->mag(), "dynamicFortePiano", d);
                                    break;
                              case Dynamic::Type::SF:
                                    writeData(xml, mag, d->mag(), "dynamicSforzando1", d);
                                    break;
                              case Dynamic::Type::SFZ:
                                    writeData(xml, mag, d->mag(), "dynamicSforzato", d);
                                    break;
                              case Dynamic::Type::SFF:
                                    writeData(xml, mag, d->mag(), "unknown", d);
                                    break;
                              case Dynamic::Type::SFFZ:
                                    writeData(xml, mag, d->mag(), "dynamicSforzatoFF", d);
                                    break;
                              case Dynamic::Type::SFP:
                                    writeData(xml, mag, d->mag(), "dynamicSforzatoPiano", d);
                                    break;
                              case Dynamic::Type::SFPP:
                                    writeData(xml, mag, d->mag(), "dynamicSforzatoPianissimo", d);
                                    break;
                              case Dynamic::Type::RFZ:
                                    writeData(xml, mag, d->mag(), "dynamicRinforzando2", d);
                                    break;
                              case Dynamic::Type::RF:
                                    writeData(xml, mag, d->mag(), "unknown", d);
                                    break;
                              case Dynamic::Type::FZ:
                                    writeData(xml, mag, d->mag(), "dynamicForzando", d);
                                    break;
                              case Dynamic::Type::M:
                                    writeData(xml, mag, d->mag(), "unknown", d);
                                    break;
                              case Dynamic::Type::R:
                                    writeData(xml, mag, d->mag(), "dynamicRinforzando", d);
                                    break;
                              case Dynamic::Type::S:
                                    writeData(xml, mag, d->mag(), "dynamicSforzando", d);
                                    break;
                              case Dynamic::Type::Z:
                                    writeData(xml, mag, d->mag(), "dynamicZ", d);
                                    break;
                              default:
                                    writeData(xml, mag, d->mag(), "unknown", d);
                                    break;
                              }
                        }
                        break;

                  default:
                        break;      // ignore
                  }
            }
      xml.etag();
      if (f.error() != QFile::NoError)
            MScore::lastError = tr("Write failed: %1").arg(f.errorString());
      }

}
