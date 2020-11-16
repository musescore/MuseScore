//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2008-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <iostream>

#include "scoreOrder.h"
#include "score.h"
#include "part.h"
#include "staff.h"
#include "bracketItem.h"
#include "instrtemplate.h"
#include "undo.h"
#include "xml.h"

namespace Ms {

ScoreOrderList scoreOrders;

//---------------------------------------------------------
//   ScoreGroup
//---------------------------------------------------------

ScoreGroup::ScoreGroup(const QString id, const QString section, const QString unsorted, bool soloists)
   : _id(id), _section(section), _soloists(soloists), _unsorted(unsorted)
      {
      _index = counter++;
      bracket = false;
      showSystemMarkings = false;
      barLineSpan = true;
      thinBracket = true;
      }

ScoreGroup::~ScoreGroup()
      {
      }

//---------------------------------------------------------
//   clone
//---------------------------------------------------------

ScoreGroup* ScoreGroup::clone()
      {
      ScoreGroup* sg = new ScoreGroup(_id, _section, _unsorted, _soloists);
      sg->bracket = bracket;
      sg->showSystemMarkings = showSystemMarkings;
      sg->barLineSpan = barLineSpan;
      sg->thinBracket = thinBracket;
      return sg;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void ScoreGroup::write(XmlWriter& xml) const
      {
      if (_soloists)
            xml.tagE("soloists");
      else if (_unsorted.isNull())
            xml.tag("family", _id);
      else if (_unsorted.isEmpty())
            xml.tagE("unsorted");
      else
            xml.tagE(QString("unsorted group=\"%1\"").arg(_unsorted));
      }

//---------------------------------------------------------
//   id
//---------------------------------------------------------

const QString& ScoreGroup::id() const
      {
      return _id;
      }

//---------------------------------------------------------
//   section
//---------------------------------------------------------

const QString& ScoreGroup::section() const
      {
      return _section;
      }

//---------------------------------------------------------
//   isSoloists
//---------------------------------------------------------

bool ScoreGroup::isSoloists() const
      {
      return _soloists;
      }

//---------------------------------------------------------
//   isUnsorted
//---------------------------------------------------------

bool ScoreGroup::isUnsorted(const QString& group) const
      {
      if (_unsorted.isNull())
            return false;
      return group.isNull() || (_unsorted == group);
      }

//---------------------------------------------------------
//   index
//---------------------------------------------------------

int ScoreGroup::index() const
      {
      return _index;
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void ScoreGroup::dump() const
      {
      QString fullName = _section.isEmpty() ? _id : QString("%1/%2").arg(_section).arg(_id);
      std::cout << "      " << _index << " : ";
      if (_soloists)
            std::cout << fullName.toStdString();
      else if (!_unsorted.isNull()) {
            std::cout << fullName.toStdString();
            if (!_unsorted.isEmpty())
                  std::cout << ", group = " << _unsorted.toStdString();
            }
      else
            std::cout << fullName.toStdString();

      std::cout << " : "
                << (showSystemMarkings ? "" : "no ") << "showSystemMarkings, "
                << (barLineSpan ? "" : "no ") << "barLineSpan, "
                << (thinBracket ? "" : "no ") << "thinBrackets, "
                << (bracket ? "" : " no ") << "brackets";
      std::cout << std::endl;
      }

int ScoreGroup::counter { 0 };

//---------------------------------------------------------
//   InstrumentOverwrite
//---------------------------------------------------------

InstrumentOverwrite::InstrumentOverwrite(const QString instrId, const QString instrName)
      {
      id   = instrId;
      name = instrName;
      }

//---------------------------------------------------------
//   ScoreOrder
//---------------------------------------------------------

ScoreOrder::ScoreOrder(const QString orderId, const QString name)
      {
      _id = orderId;
      init();
      _name = name.isEmpty() ? orderId : name;
      }

ScoreOrder::~ScoreOrder()
      {
      while (!groups.isEmpty())
            delete groups.takeFirst();
      }

//---------------------------------------------------------
//   clone
//---------------------------------------------------------

ScoreOrder* ScoreOrder::clone()
      {
      ScoreOrder* order = new ScoreOrder(_id, _name);
      for (ScoreGroup* sg : groups)
            {
            ScoreGroup* sgc = sg->clone();
            if (sg->isSoloists())
                  order->_soloists = sgc;
            if (sg->isUnsorted())
                  order->_unsorted = sgc;
            order->groups.append(sgc);
            }
      order->instrumentMap = instrumentMap;
      order->_groupMultiplier = _groupMultiplier;
      order->_customised = true;
      return order;
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void ScoreOrder::init()
      {
      _name = _id;
      _soloists = nullptr;
      _unsorted = nullptr;
      _groupMultiplier = 1;
      _customised = false;
      if (!isCustom()) {
            for (auto ig : instrumentGroups)
                  _groupMultiplier += ig->instrumentTemplates.size();
            }

      while (!groups.isEmpty())
            delete groups.takeFirst();
      }

//---------------------------------------------------------
//   readBoolAttribute
//---------------------------------------------------------

bool ScoreOrder::readBoolAttribute(XmlReader& e, const char* name, bool defvalue)
      {
      if (!e.hasAttribute(name))
            return defvalue;
      QString attr { e.attribute(name) };
      if (attr.toLower() == "false")
            return false;
      if (attr.toLower() == "true")
            return true;
      qDebug("invalid value \"%s\" for attribute \"%s\", using default \"%d\"", qPrintable(attr), qPrintable(name), defvalue);
      return defvalue;
      }

//---------------------------------------------------------
//   readName
//---------------------------------------------------------

void ScoreOrder::readName(XmlReader& e)
      {
      _name = qApp->translate("OrderXML", e.readElementText().toUtf8().data());
      }

//---------------------------------------------------------
//   readInstrument
//---------------------------------------------------------

void ScoreOrder::readInstrument(XmlReader& e)
      {
      QString instrumentId { e.attribute("id") };
      if (!searchTemplate(instrumentId)) {
            qDebug("cannot find instrument templates for <%s>", qPrintable(instrumentId));
            e.skipCurrentElement();
            return;
            }
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "family") {
                  const QString id { e.attribute("id") };
                  const QString name = qApp->translate("OrderXML", e.readElementText().toUtf8().data());
                  instrumentMap.insert(instrumentId, InstrumentOverwrite(id, name));
                  }
            else {
                  e.unknown();
                  }
            }
      }

//---------------------------------------------------------
//   readSoloists
//---------------------------------------------------------

void ScoreOrder::readSoloists(XmlReader& e, const QString section)
      {
      e.skipCurrentElement();
      if (_soloists)
            return;
      _soloists = new ScoreGroup(QString("<soloists>"), section, QString(), true);
      groups.append(_soloists);
      }

//---------------------------------------------------------
//   readUnsorted
//---------------------------------------------------------

void ScoreOrder::readUnsorted(XmlReader& e, const QString section, bool br, bool ssm, bool bls, bool tbr)
      {
      QString group { e.attribute("group", QString("")) };
      e.skipCurrentElement();
      for (auto sg : groups) {
            if (sg->isUnsorted(group))
                  return;
            };
      ScoreGroup* sg = new ScoreGroup(QString("<unsorted>"), section, group);
      sg->bracket            = br;
      sg->showSystemMarkings = ssm;
      sg->barLineSpan        = bls;
      sg->thinBracket        = tbr;
      groups.append(sg);
      if (!_unsorted && group.isEmpty())
            _unsorted = sg;
      }

//---------------------------------------------------------
//   readFamily
//---------------------------------------------------------

void ScoreOrder::readFamily(XmlReader& e, const QString section, bool br, bool ssm, bool bls, bool tbr)
      {
      const QString id { e.readElementText().toUtf8().data() };
      for (auto sg : groups) {
            if (sg->id() == id)
                  return;
            }
      ScoreGroup* sg = new ScoreGroup(id, section);
      sg->bracket            = br;
      sg->showSystemMarkings = ssm;
      sg->barLineSpan        = bls;
      sg->thinBracket        = tbr;
      groups.append(sg);
      }

//---------------------------------------------------------
//   readSection
//---------------------------------------------------------

void ScoreOrder::readSection(XmlReader& e)
      {
      QString id { e.attribute("id") };
      bool ssm = readBoolAttribute(e, "showSystemMarkings", false);
      bool bls = readBoolAttribute(e, "barLineSpan",        true );
      bool tbr = readBoolAttribute(e, "thinBrackets",       true );
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "family")
                  readFamily(e, id, true, ssm, bls, tbr);
            else if (tag == "unsorted")
                  readUnsorted(e, id, true, ssm, bls, tbr);
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   getFamilyName
//---------------------------------------------------------

QString ScoreOrder::getFamilyName(const InstrumentTemplate* instrTemplate, bool soloist) const
      {
      if (!instrTemplate)
            return QString("<unsorted>");
      if (soloist)
           return QString("<soloists>");
      else if (instrumentMap.contains(instrTemplate->trackName.toLower()))
            return instrumentMap[instrTemplate->trackName.toLower()].id;
      else if (instrTemplate->family)
            return instrTemplate->family->id;
      else
            return QString("<unsorted>");
      }

//---------------------------------------------------------
//   getId
//---------------------------------------------------------

QString ScoreOrder::getId() const
      {
      if (_customised)
            return QString("%1-%2").arg(_id).arg(reinterpret_cast<quintptr>(this), 0, 16);
      else
            return _id;
      }

//---------------------------------------------------------
//   getName
//---------------------------------------------------------

QString ScoreOrder::getName() const
      {
      return _name;
      }

//---------------------------------------------------------
//   getFullName
//---------------------------------------------------------

QString ScoreOrder::getFullName() const
      {
      if (_customised)
            return QString(QT_TRANSLATE_NOOP("OrderXML", "%1 (Customised)")).arg(_name);
      else
            return getName();
      }

//---------------------------------------------------------
//   isCustom
//---------------------------------------------------------

bool ScoreOrder::isCustom() const
      {
      return _id == QString("<custom>");
      }

//---------------------------------------------------------
//   isCustomised
//---------------------------------------------------------

bool ScoreOrder::isCustomised() const
      {
      return _customised;
      }

//---------------------------------------------------------
//   setCustomised
//---------------------------------------------------------

void ScoreOrder::setCustomised()
      {
      if (!isCustom())
            _customised = true;
      }

//---------------------------------------------------------
//   getGroup
//---------------------------------------------------------

ScoreGroup* ScoreOrder::getGroup(const QString family, const QString instrumentGroup) const
      {
      if (family.isEmpty())
            return nullptr;

      ScoreGroup* unsorted { _unsorted };
      for (ScoreGroup* sg : groups) {
            if (sg->id() == family)
                  return sg;
            if (sg->isUnsorted(instrumentGroup))
                  unsorted = sg;
            if (sg->isUnsorted() && !unsorted)
                  unsorted = sg;
            }
      return unsorted;
      }

ScoreGroup* ScoreOrder::getGroup(const QString id, const bool soloist) const
      {
      InstrumentIndex ii = searchTemplateIndexForId(id);
      QString family { getFamilyName(ii.instrTemplate, soloist) };
      return getGroup(family, instrumentGroups[ii.groupIndex]->id);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void ScoreOrder::read(XmlReader& e)
      {
      init();
      const QString id { "" };
      _customised = e.intAttribute("customised");
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "name")
                  readName(e);
            else if (tag == "section")
                  readSection(e);
            else if (tag == "instrument")
                  readInstrument(e);
            else if (tag == "family")
                  readFamily(e, id, false, false, false, false);
            else if (tag == "soloists")
                  readSoloists(e, id);
            else if (tag == "unsorted")
                  readUnsorted(e, id, false, false, false, false);
            else
                  e.unknown();
            }
      if (!_unsorted) {
            _unsorted  = new ScoreGroup(QString("<unsorted>"), "", "");
            _unsorted->bracket            = false;
            _unsorted->showSystemMarkings = false;
            _unsorted->barLineSpan        = false;
            _unsorted->thinBracket        = false;
            groups.append(_unsorted);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void ScoreOrder::write(XmlWriter& xml) const
      {
      if (isCustom())
            return;

      xml.stag(QString("Order id=\"%1\" customised=\"%2\"").arg(_id).arg(_customised));
      xml.tag("name", _name);

      QMapIterator<QString, InstrumentOverwrite> i(instrumentMap);
      while (i.hasNext()) {
            i.next();
            xml.stag(QString("instrument id=\"%1\"").arg(i.key()));
            xml.tag(QString("family id=\"%1\"").arg(i.value().id), i.value().name);
            xml.etag();
            }

      QString section { "" };
      for (ScoreGroup* sg : groups)
            {
            if (sg->section() != section)
                  {
                  if (!section.isEmpty())
                        xml.etag();
                  if (!sg->section().isEmpty())
                        xml.stag(QString("section id=\"%1\" brackets=\"%2\" showSystemMarkings=\"%3\" barLineSpan=\"%4\" thinBrackets=\"%5\"")
                                 .arg(sg->section())
                                 .arg(sg->bracket ? "true" : "false")
                                 .arg(sg->showSystemMarkings ? "true" : "false")
                                 .arg(sg->barLineSpan ? "true" : "false")
                                 .arg(sg->thinBracket ? "true" : "false"));
                  section = sg->section();
                  }
            sg->write(xml);
            }
      if (!section.isEmpty())
            xml.etag();
      xml.etag();
      }

//---------------------------------------------------------
//   instrumentIndex
//---------------------------------------------------------

int ScoreOrder::instrumentIndex(const QString id, bool soloist) const
      {
      ScoreGroup* sg = getGroup(id, soloist);
      int groupIndex = sg ? sg->index() : _groupMultiplier;
      bool unsorted = sg && sg->isUnsorted();

      return _groupMultiplier * groupIndex + (unsorted ?  0 : searchTemplateIndexForId(id).instrIndex);
      }

//---------------------------------------------------------
//   instrumentInUnsortedSection
//---------------------------------------------------------

bool ScoreOrder::instrumentInUnsortedSection(const QString id, bool soloist) const
      {
      return soloist || getGroup(id, soloist)->isUnsorted();
      }

//---------------------------------------------------------
//   updateInstruments
//---------------------------------------------------------

void ScoreOrder::updateInstruments(const Score* score)
      {
      for (Part* part : score->parts())
            {
            InstrumentIndex ii = searchTemplateIndexForId(part->instrument()->getId());
            if (!ii.instrTemplate || !ii.instrTemplate->family)
                  continue;

            InstrumentFamily* family = ii.instrTemplate->family;
            instrumentMap.insert(ii.instrTemplate->id, InstrumentOverwrite(family->id, family->name));
            }
      }

//---------------------------------------------------------
//   setBracketsAndBarlines
//---------------------------------------------------------

void ScoreOrder::setBracketsAndBarlines(Score* score)
      {
      if (isCustom())
            return;

      ScoreGroup* prvScoreGroup   { nullptr };
      int         prvInstrument   { 0       };
      Staff*      prvStaff        { nullptr };

      Staff*      thkBracketStaff { nullptr };
      Staff*      thnBracketStaff { nullptr };
      int         thkBracketSpan  { 0       };
      int         thnBracketSpan  { 0       };

      for (Part* part : score->parts())
            {
            InstrumentIndex ii = searchTemplateIndexForId(part->instrument()->getId());
            QString family { getFamilyName(ii.instrTemplate, part->soloist()) };
            ScoreGroup* sg = getGroup(family, instrumentGroups[ii.groupIndex]->id);

            int staffIdx { 0 };
            bool blockThinBracket { false };
            for (Staff* staff : *part->staves())
                  {
                  for (BracketItem* bi : staff->brackets())
                        score->undo(new RemoveBracket(staff, bi->column(), bi->bracketType(), bi->bracketSpan()));
                  staff->undoChangeProperty(Pid::STAFF_BARLINE_SPAN, 0);

                  if (!prvScoreGroup || (sg->section() != prvScoreGroup->section())) {
                        if (thkBracketStaff && (thkBracketSpan > 1))
                              score->undoAddBracket(thkBracketStaff, 0, BracketType::NORMAL, thkBracketSpan);
                        if (sg->bracket && !staffIdx)
                              {
                              thkBracketStaff = sg->bracket ? staff : nullptr;
                              thkBracketSpan  = 0;
                              }
                        }
                  if (sg->bracket && !staffIdx)
                        thkBracketSpan += part->nstaves();

                  if (!staffIdx || (ii.instrIndex != prvInstrument))
                        {
                        if (thnBracketStaff && (thnBracketSpan > 1))
                              score->undoAddBracket(thnBracketStaff, 1, BracketType::SQUARE, thnBracketSpan);
                        if (ii.instrIndex != prvInstrument)
                              {
                              thnBracketStaff = (sg->thinBracket && !blockThinBracket) ? staff : nullptr;
                              thnBracketSpan  = 0;
                              }
                        }
                  if (sg->thinBracket && !blockThinBracket && !staffIdx)
                        thnBracketSpan += part->nstaves();

                  if (ii.instrTemplate->nstaves() > 1)
                        {
                        blockThinBracket = true;
                        if (ii.instrTemplate->bracket[staffIdx] != BracketType::NO_BRACKET)
                              score->undoAddBracket(staff, 2, ii.instrTemplate->bracket[staffIdx], ii.instrTemplate->bracketSpan[staffIdx]);
                        staff->undoChangeProperty(Pid::STAFF_BARLINE_SPAN, ii.instrTemplate->barlineSpan[staffIdx]);
                        if (staffIdx < ii.instrTemplate->nstaves())
                              ++staffIdx;
                        prvStaff = nullptr;
                        }
                  else
                        {
                        if (prvStaff)
                              prvStaff->undoChangeProperty(Pid::STAFF_BARLINE_SPAN, (sg->barLineSpan && (!prvScoreGroup || (sg->section() == prvScoreGroup->section()))));
                        prvStaff = staff;
                        ++staffIdx;
                        }
                  prvScoreGroup = sg;
                  }

            prvInstrument = ii.instrIndex;
            }

      if (thkBracketStaff && (thkBracketSpan > 1))
            score->undoAddBracket(thkBracketStaff, 0, BracketType::NORMAL, thkBracketSpan);
      if (thnBracketStaff && (thnBracketSpan > 1) && prvScoreGroup->thinBracket)
            score->undoAddBracket(thnBracketStaff, 1, BracketType::SQUARE, thnBracketSpan);
      }

//---------------------------------------------------------
//   isScoreOrder
//---------------------------------------------------------

bool ScoreOrder::isScoreOrder(const QList<int>& indices) const
      {
      if (isCustom())
            return true;

      int prvIndex { -1 };
      for (int curIndex : indices)
            {
            if (curIndex < prvIndex)
                  return false;
            prvIndex = curIndex;
            }
      return true;
      }

bool ScoreOrder::isScoreOrder(const Score* score) const
      {
      QList<int> indices;
      for (Part* part : score->parts())
            indices << instrumentIndex(part->instrument()->getId(), part->soloist());

      return isScoreOrder(indices);
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void ScoreOrder::dump() const
      {
      std::cout << "   order : " << _id.toStdString() << ", name = " << _name.toStdString() << std::endl;
      if (instrumentMap.isEmpty()) {
            std::cout << "      no instrument mapping" << std::endl;
            }
      else {
            std::cout << "      instrument mapping:" << std::endl;
            QMapIterator<QString, InstrumentOverwrite> i(instrumentMap);
            while (i.hasNext()) {
                  i.next();
                  std::cout << "         " << i.key().toStdString() << " => " << i.value().id.toStdString() << std::endl;
                  }
            }
      std::cout << "   sections:" << std::endl;
      for (auto group : groups)
            group->dump();
      }

//---------------------------------------------------------
//   ScoreOrderList
//---------------------------------------------------------

ScoreOrderList::ScoreOrderList()
      {
      _orders.clear();
      ScoreOrder* custom = new ScoreOrder(QString("<custom>"), qApp->translate("OrderXML", "Custom"));
      custom->groups.append(new ScoreGroup(QString("<unsorted>"), QString(""), QString("")));
      addScoreOrder(custom);
      }

ScoreOrderList::~ScoreOrderList()
      {
      while (!_orders.isEmpty())
            delete _orders.takeFirst();
      }

//---------------------------------------------------------
//   append
//---------------------------------------------------------

void ScoreOrderList::append(ScoreOrder* order)
      {
      if (_orders.empty() || !_orders.last()->isCustom())
            _orders.append(order);
      else
            _orders.insert(_orders.size()-1, order);
      }

//---------------------------------------------------------
//   findById
//      searches for a ScoreOrder with specified id
//      return nullptr if no matching ScoreOrder is found
//---------------------------------------------------------

ScoreOrder* ScoreOrderList::findById(const QString& id) const
      {
      for (ScoreOrder* order : _orders) {
            if (order->getId() == id)
                  return order;
            }
      return nullptr;
      }

//---------------------------------------------------------
//   getById
//      searches for a ScoreOrder with specified id
//      create a new ScoreOrder if no matching ScoreOrder is found
//---------------------------------------------------------

ScoreOrder* ScoreOrderList::getById(const QString& id)
      {
      ScoreOrder* order = findById(id);
      if (!order) {
            order = new ScoreOrder(id);
            addScoreOrder(order);
            }
      return order;
      }

//---------------------------------------------------------
//   findByName
//      searches for a ScoreOrder with specified name
//      return nullptr if no matching ScoreOrder is found
//---------------------------------------------------------

ScoreOrder* ScoreOrderList::findByName(const QString& name, bool customised)
      {
      ScoreOrder* customisedOrder { nullptr };
      for (ScoreOrder* order : _orders) {
            if (order->getName() == name) {
                  if (customised)
                        {
                        if (order->isCustomised())
                              return order;
                        }
                  else
                        {
                        if (order->isCustomised())
                              customisedOrder = order;
                        else
                              return order;
                        }
                  }
            }
      return customisedOrder;
      }

//---------------------------------------------------------
//   searchScoreOrder
//      return the index of the ScoreOrder or 0 (= Custom)
//      if the order is not found.
//---------------------------------------------------------

int ScoreOrderList::getScoreOrderIndex(const ScoreOrder* order) const
      {
      int index { 0 };
      for (ScoreOrder* so : _orders)
            {
            if (so == order)
                  return index;
            ++index;
            }
      return 0;
      }

//---------------------------------------------------------
//   searchScoreOrder
//---------------------------------------------------------

QList<ScoreOrder*> ScoreOrderList::searchScoreOrders(const QList<int>& indices) const
      {
      QList<ScoreOrder*> orders;
      for (ScoreOrder* order : _orders)
            {
            if (!order->isCustom() && order->isScoreOrder(indices))
                  orders << order;
            }
      return orders;
      }

QList<ScoreOrder*> ScoreOrderList::searchScoreOrders(const Score* score) const
      {
      QList<ScoreOrder*> orders;
      for (Part* part : score->parts()) {
            QList<int> indices;
            for (ScoreOrder* order : _orders)
                  {
                  if (order->isCustom())
                        continue;
                  indices << order->instrumentIndex(part->instrument()->getId(), part->soloist());
                  if (order->isScoreOrder(indices))
                        orders << order;
                  }
            }

      return orders;
      }

//---------------------------------------------------------
//   addScoreOrder
//---------------------------------------------------------

void ScoreOrderList::addScoreOrder(ScoreOrder* order)
      {
      if (!order || _orders.contains(order))
            return;

      if (!order->isCustomised()) {
            append(order);
            return;
            }

      if (order->isCustomised()) {
            for (int index { 0 }; index < _orders.size(); ++index) {
                  if (_orders[index]->getName() == order->getName()) {
                        _orders.insert(index+1, order);
                        return;
                        }
                  }
            }
      append(order);
      }

//---------------------------------------------------------
//   removeScoreOrder
//---------------------------------------------------------

void ScoreOrderList::removeScoreOrder(ScoreOrder* order)
      {
      if (!order)
            return;
      int index { getScoreOrderIndex(order) };
      if (index)
            _orders.removeAt(index);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void ScoreOrderList::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Order")
                  scoreOrders.getById(e.attribute("id"))->read(e);
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void ScoreOrderList::write(XmlWriter& xml) const
      {
      for (auto so : _orders)
            so->write(xml);
      }

//---------------------------------------------------------
//   size
//---------------------------------------------------------

int ScoreOrderList::size() const
      {
      return _orders.size();
      }

//---------------------------------------------------------
//   opeator[
//---------------------------------------------------------

ScoreOrder* ScoreOrderList::operator[](const int index) const
      {
      return _orders[index];
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void ScoreOrderList::dump() const
      {
      std::cout << "Dump of ScoreOrders:" << std::endl;
      for (auto order : _orders)
            order->dump();
      }

//---------------------------------------------------------
//   loadScoreOrders
//---------------------------------------------------------

bool loadScoreOrders(const QString& scoreOrderFileName)
      {
      QFile qf(scoreOrderFileName);
      if (!qf.open(QIODevice::Text | QIODevice::ReadOnly)) {
            qDebug("cannot load score orders at <%s>", qPrintable(scoreOrderFileName));
            return false;
            }

      XmlReader e(&qf);
      while (e.readNextStartElement()) {
            if (e.name() == "museScore")
                  scoreOrders.read(e);
            }

      return true;
      }


//---------------------------------------------------------
//   saveScoreOrders
//---------------------------------------------------------

bool saveScoreOrders(const QString& scoreOrderFileNameName)
      {
      QFile qf(scoreOrderFileNameName);
      if (!qf.open(QIODevice::WriteOnly)) {
            qDebug("cannot save instrument templates at <%s>", qPrintable(scoreOrderFileNameName));
            return false;
            }
      XmlWriter xml(0, &qf);
      xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      xml.stag("museScore");
      scoreOrders.write(xml);
      xml.etag();
      qf.close();
      return true;
      }
}
