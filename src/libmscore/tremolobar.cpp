//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "tremolobar.h"
#include "score.h"
#include "undo.h"
#include "staff.h"
#include "chord.h"
#include "note.h"
#include "xml.h"

namespace Ms {
//---------------------------------------------------------
//   tremoloBarStyle
//---------------------------------------------------------

static const ElementStyle tremoloBarStyle {
    { Sid::tremoloBarLineWidth,  Pid::LINE_WIDTH },
};

static const QList<PitchValue> DIP_CURVE = { PitchValue(0, 0),
                                             PitchValue(30, -100),
                                             PitchValue(60, 0) };

static const QList<PitchValue> DIVE_CURVE = { PitchValue(0, 0),
                                              PitchValue(60, -150) };

static const QList<PitchValue> RELEASE_UP_CURVE = { PitchValue(0, -150),
                                                    PitchValue(60, 0) };

static const QList<PitchValue> INVERTED_DIP_CURVE = { PitchValue(0, 0),
                                                      PitchValue(30, 100),
                                                      PitchValue(60, 0) };

static const QList<PitchValue> RETURN_CURVE = { PitchValue(0, 0),
                                                PitchValue(60, 150) };

static const QList<PitchValue> RELEASE_DOWN_CURVE = { PitchValue(0, 150),
                                                      PitchValue(60, 0) };

//---------------------------------------------------------
//   TremoloBar
//---------------------------------------------------------

TremoloBar::TremoloBar(Score* s)
    : Element(s, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    initElementStyle(&tremoloBarStyle);
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TremoloBar::layout()
{
    qreal _spatium = spatium();
    if (parent()) {
        setPos(0.0, -_spatium * 3.0);
    } else {
        setPos(QPointF());
    }

    /* we place the tremolo bars starting slightly before the
     *  notehead, and end it slightly after, drawing above the
     *  note. The values specified in Guitar Pro are very large, too
     *  large for the scale used in Musescore. We used the
     *  timeFactor and pitchFactor below to reduce these values down
     *  consistently to values that make sense to draw with the
     *  Musescore scale. */

    qreal timeFactor  = m_userMag / 1.0;
    qreal pitchFactor = -_spatium * .02;

    m_polygon.clear();
    for (auto v : m_points) {
        m_polygon << QPointF(v.time * timeFactor, v.pitch * pitchFactor);
    }

    qreal w = m_lw.val();
    setbbox(m_polygon.boundingRect().adjusted(-w, -w, w, w));
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TremoloBar::draw(QPainter* painter) const
{
    QPen pen(curColor(), m_lw.val(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(pen);
    painter->drawPolyline(m_polygon);
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TremoloBar::write(XmlWriter& xml) const
{
    xml.stag(this);
    writeProperty(xml, Pid::MAG);
    writeProperty(xml, Pid::LINE_WIDTH);
    writeProperty(xml, Pid::PLAY);
    for (const PitchValue& v : m_points) {
        xml.tagE(QString("point time=\"%1\" pitch=\"%2\" vibrato=\"%3\"")
                 .arg(v.time).arg(v.pitch).arg(v.vibrato));
    }
    xml.etag();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TremoloBar::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        auto tag = e.name();
        if (tag == "point") {
            PitchValue pv;
            pv.time    = e.intAttribute("time");
            pv.pitch   = e.intAttribute("pitch");
            pv.vibrato = e.intAttribute("vibrato");
            m_points.append(pv);
            e.readNext();
        } else if (tag == "mag") {
            m_userMag = e.readDouble(0.1, 10.0);
        } else if (readStyledProperty(e, tag)) {
        } else if (tag == "play") {
            setPlay(e.readInt());
        } else if (readProperty(tag, e, Pid::LINE_WIDTH)) {
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant TremoloBar::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::LINE_WIDTH:
        return lineWidth();
    case Pid::MAG:
        return userMag();
    case Pid::PLAY:
        return play();
    case Pid::TREMOLOBAR_TYPE:
        return static_cast<int>(parseTremoloBarTypeFromCurve(m_points));
    case Pid::TREMOLOBAR_CURVE:
        return QVariant::fromValue(m_points);
    default:
        return Element::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool TremoloBar::setProperty(Pid propertyId, const QVariant& v)
{
    switch (propertyId) {
    case Pid::LINE_WIDTH:
        setLineWidth(v.value<Spatium>());
        break;
    case Pid::MAG:
        setUserMag(v.toDouble());
        break;
    case Pid::PLAY:
        setPlay(v.toBool());
        score()->setPlaylistDirty();
        break;
    case Pid::TREMOLOBAR_TYPE:
        updatePointsByTremoloBarType(static_cast<TremoloBarType>(v.toInt()));
        break;
    case Pid::TREMOLOBAR_CURVE:
        setPoints(v.value<QList<Ms::PitchValue> >());
        break;
    default:
        return Element::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant TremoloBar::propertyDefault(Pid pid) const
{
    switch (pid) {
    case Pid::MAG:
        return 1.0;
    case Pid::PLAY:
        return true;
    case Pid::TREMOLOBAR_TYPE:
        return static_cast<int>(TremoloBarType::DIP);
    case Pid::TREMOLOBAR_CURVE:
        return QVariant::fromValue(DIP_CURVE);
    default:
        for (const StyledProperty& p : *styledProperties()) {
            if (p.pid == pid) {
                if (propertyType(pid) == P_TYPE::SP_REAL) {
                    return score()->styleP(p.sid);
                }
                return score()->styleV(p.sid);
            }
        }
        return Element::propertyDefault(pid);
    }
}

TremoloBarType TremoloBar::parseTremoloBarTypeFromCurve(const QList<PitchValue>& curve) const
{
    if (curve == DIP_CURVE) {
        return TremoloBarType::DIP;
    } else if (curve == DIVE_CURVE) {
        return TremoloBarType::DIVE;
    } else if (curve == RELEASE_UP_CURVE) {
        return TremoloBarType::RELEASE_UP;
    } else if (curve == INVERTED_DIP_CURVE) {
        return TremoloBarType::INVERTED_DIP;
    } else if (curve == RETURN_CURVE) {
        return TremoloBarType::RETURN;
    } else if (curve == RELEASE_DOWN_CURVE) {
        return TremoloBarType::RELEASE_DOWN;
    } else {
        return TremoloBarType::CUSTOM;
    }
}

void TremoloBar::updatePointsByTremoloBarType(const TremoloBarType type)
{
    switch (type) {
    case TremoloBarType::DIP:
        m_points = DIP_CURVE;
        break;
    case TremoloBarType::DIVE:
        m_points = RELEASE_UP_CURVE;
        break;
    case TremoloBarType::RELEASE_UP:
        m_points = INVERTED_DIP_CURVE;
        break;
    case TremoloBarType::INVERTED_DIP:
        m_points = RETURN_CURVE;
        break;
    case TremoloBarType::RETURN:
        m_points = RELEASE_DOWN_CURVE;
        break;
    case TremoloBarType::RELEASE_DOWN:
        m_points = RELEASE_DOWN_CURVE;
        break;
    default:
        break;
    }
}
}
