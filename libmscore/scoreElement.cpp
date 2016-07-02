//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2015 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "scoreElement.h"
#include "score.h"
#include "undo.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   ScoreElement
//---------------------------------------------------------

ScoreElement::ScoreElement(const ScoreElement& se)
      {
      _score = se._score;
      _links = 0;
      }

//---------------------------------------------------------
//   ~ScoreElement
//---------------------------------------------------------

ScoreElement::~ScoreElement()
      {
      if (_links) {
            _links->removeOne(this);
            if (_links->empty()) {
                  delete _links;
                  _links = 0;
                  }
            }
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void ScoreElement::resetProperty(P_ID id)
      {
      QVariant v = propertyDefault(id);
      if (v.isValid())
            setProperty(id, v);
      }

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void ScoreElement::undoChangeProperty(P_ID id, const QVariant& val)
      {
      score()->undoChangeProperty(this, id, val);
      }

//---------------------------------------------------------
//   undoPushProperty
//---------------------------------------------------------

void ScoreElement::undoPushProperty(P_ID id)
      {
      QVariant val = getProperty(id);
      score()->undoStack()->push1(new ChangeProperty(this, id, val));
      }

//---------------------------------------------------------
//   writeProperty
//---------------------------------------------------------

void ScoreElement::writeProperty(Xml& xml, P_ID id) const
      {
      xml.tag(id, getProperty(id), propertyDefault(id));
      }

//---------------------------------------------------------
//   linkTo
//---------------------------------------------------------

void ScoreElement::linkTo(ScoreElement* element)
      {
      Q_ASSERT(element != this);
      if (!_links) {
            if (element->links()) {
                  _links = element->_links;
                  Q_ASSERT(_links->contains(element));
                  }
            else {
                  _links = new LinkedElements(score());
                  _links->append(element);
                  element->_links = _links;
                  }
            Q_ASSERT(!_links->contains(this));
            _links->append(this);
            }
      else {
            _links->append(element);
            element->_links = _links;
            }
      }

//---------------------------------------------------------
//   unlink
//---------------------------------------------------------

void ScoreElement::unlink()
      {
      if (_links) {
            Q_ASSERT(_links->contains(this));
            _links->removeOne(this);

            // if link list is empty, remove list
            if (_links->size() <= 1) {
                  if (!_links->empty())         // abnormal case: only "this" is in list
                        _links->front()->_links = 0;
                  delete _links;
                  }
            _links = 0;
            }
      }

//---------------------------------------------------------
//   undoUnlink
//---------------------------------------------------------

void ScoreElement::undoUnlink()
      {
//      if (_links)
//TODO-ws            _score->undo(new Unlink(this));
      }

//---------------------------------------------------------
//   linkList
//---------------------------------------------------------

QList<ScoreElement*> ScoreElement::linkList() const
      {
      QList<ScoreElement*> el;
      if (links())
            el.append(*links());
      else
            el.append((Element*)this);
      return el;
      }

//---------------------------------------------------------
//   LinkedElements
//---------------------------------------------------------

LinkedElements::LinkedElements(Score* score)
      {
      _lid = score->linkId(); // create new unique id
      }

LinkedElements::LinkedElements(Score* score, int id)
      {
      _lid = id;
      score->linkId(id);      // remember used id
      }

//---------------------------------------------------------
//   setLid
//---------------------------------------------------------

void LinkedElements::setLid(Score* score, int id)
      {
      _lid = id;
      score->linkId(id);
      }

//---------------------------------------------------------
//   masterScore
//---------------------------------------------------------

MasterScore* ScoreElement::masterScore() const
      {
      return _score->masterScore();
      }

//---------------------------------------------------------
//   propertyStyle
//---------------------------------------------------------

PropertyStyle ScoreElement::propertyStyle(P_ID) const
      {
      return PropertyStyle::NOSTYLE;
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

StyleIdx ScoreElement::getPropertyStyle(P_ID) const
      {
      return StyleIdx::NOSTYLE;
      }
}

