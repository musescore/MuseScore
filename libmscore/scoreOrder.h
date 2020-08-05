//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2014 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SCOREORDER_H__
#define __SCOREORDER_H__

#include "mscore.h"
#include "part.h"

namespace Ms {

class InstrumentTemplate;

//---------------------------------------------------------
//   ScoreGroup
//---------------------------------------------------------

class ScoreGroup final {
      int static counter;
      QString _id;
      QString _section;
      bool _soloists;
      QString _unsorted;  // isNull()   : not an unsorted group
                          // isEmpty()  : equal to <unsorted/>
                          // !isEmpty() : equal to <unsorted group="_unsorted"/>
      int _index;

   public:
      bool bracket;
      bool showSystemMarkings;
      bool barLineSpan;
      bool thinBracket;

      ScoreGroup(const QString id, const QString section, const QString unsorted=QString(), bool soloists=false);
      ~ScoreGroup();
      ScoreGroup* clone();

      void write(XmlWriter& xml) const;

      const QString& id() const;
      const QString& section() const;
      bool isSoloists() const;
      bool isUnsorted(const QString& group=QString()) const;
      int index() const;

      virtual void dump() const;
      };

//---------------------------------------------------------
//   InstrumentOverwrite
//---------------------------------------------------------

class InstrumentOverwrite {
   public:
      QString id;
      QString name;

      InstrumentOverwrite(const QString id=QString(), const QString name=QString());
};

//---------------------------------------------------------
//   ScoreOrder
//---------------------------------------------------------

class ScoreOrder {
      QString _id { "" };
      QString _name { "" };
      ScoreGroup* _soloists;
      ScoreGroup* _unsorted;
      int _groupMultiplier;
      bool _customised { false };

   protected:
      QMap<QString, InstrumentOverwrite> instrumentMap;

      void init();
      bool readBoolAttribute(XmlReader& e, const char* name, bool defValue);
      void readName(XmlReader& e);
      void readInstrument(XmlReader& e);
      void readSoloists(XmlReader& e, const QString section);
      void readUnsorted(XmlReader& e, const QString section, bool br, bool ssm, bool bls, bool tbr);
      void readFamily(XmlReader& e, const QString section, bool br, bool ssm, bool bls, bool tbr);
      void readSection(XmlReader& e);
      QString getFamilyName(const InstrumentTemplate *instrTemplate, bool soloist) const;


   public:
      QList<ScoreGroup*> groups;

      ScoreOrder(const QString id, const QString name=QString());
      ~ScoreOrder();
      ScoreOrder* clone();

      QString getId() const;
      QString getName() const;
      QString getFullName() const;
      bool isCustom() const;
      void setOwner(Score *score);
      Score* getOwner() const;
      bool isCustomised() const;
      void setCustomised();

      ScoreGroup* getGroup(const QString family, const QString instrumentGroup) const;
      ScoreGroup* getGroup(const QString instrumentName, bool soloist) const;

      void read(XmlReader& e);
      void write(XmlWriter& xml) const;

      int instrumentIndex(const QString name, bool soloist) const;
      bool instrumentInUnsortedSection(const QString name, bool soloist) const;

      void updateInstruments(const Score* score);
      void setBracketsAndBarlines(Score* score);
      bool isScoreOrder(const QList<int>& indices) const;
      bool isScoreOrder(const Score* score) const;

      void debug(const QString& name, bool soloist) const;
      void dump() const;
      };

//---------------------------------------------------------
//   ScoreOrderList
//---------------------------------------------------------

class ScoreOrderList {
   protected:
      QList<ScoreOrder*> _orders;

   public:
      ScoreOrderList();
      ~ScoreOrderList();

      ScoreOrder* findById(const QString& orderId) const;
      ScoreOrder* getById(const QString& orderId);
      ScoreOrder* findByName(const QString& orderName, bool customised=false);
      int getScoreOrderIndex(const ScoreOrder* order) const;
      QList<ScoreOrder*> searchScoreOrders(const QList<int>& indices) const;
      QList<ScoreOrder*> searchScoreOrders(const Score* score) const;
      void addScoreOrder(ScoreOrder* order);
      void removeScoreOrder(ScoreOrder* order);

      void read(XmlReader& e);
      void write(XmlWriter& xml) const;

      int size() const;

      ScoreOrder* operator[](const int) const;

      void dump() const;
      };

extern ScoreOrderList scoreOrders;

extern bool loadScoreOrders(const QString& scoreOrderFileName);
extern bool saveScoreOrders(const QString& scoreOrderFileName);

} // namespace Ms

#endif
