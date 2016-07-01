//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef SOUNDBANK_H
#define SOUNDBANK_H

#include "scoreElement.h"
#include "xml.h"
//#include "instrument.h"
#include <vector>
#include <QString>

namespace Ms {

enum class ccType : char;
enum class ccBind : char;
class Channel;
struct NamedEventList;
class InstrumentTemplate;

struct SoundBankMatch {
      QString type;
      QString value;
      void read(XmlReader &e);
      void write(Xml &xml) const;
      };

struct SoundBankSpannerDefault {
      int value;
      int from;
      int to;
      QString name;
      ccType type;
      ccBind bind;
      Element::Type spannerType;
      bool isDefault = false;
      SoundBankSpannerDefault(Element::Type t) : spannerType(t) {;}
      void read(XmlReader &e);
      void write(Xml &xml) const;
      };

struct SoundBankSpannerDefaults {
      std::vector<SoundBankSpannerDefault*> _spannerDefaults;

      std::vector<SoundBankSpannerDefault*> getSpannerDefaults(Element::Type type);
      SoundBankSpannerDefault* getSpannerDefault(Element::Type type);
      void setSelected(unsigned int v, Element::Type type);

      ~SoundBankSpannerDefaults();
      void operator=(const SoundBankSpannerDefaults&);
      void read(XmlReader &);
      void write(Xml &) const;
      };

struct SoundBankOptions {
      int fixedVelocity = 0;
      bool useExpression = false;
      int noteOffset = 0;

      void read(XmlReader &);
      void write(Xml &) const;
      };

class SoundBank {
      QString _name;
      std::vector<SoundBankMatch> _matches;
      std::vector<Channel*> _channel;
      std::vector<NamedEventList> _midiActions;

      SoundBankSpannerDefaults _spannerDefaults;
      SoundBankOptions _options;

public:
      SoundBank(QString name);
      std::vector<SoundBankMatch> getMatches() { return _matches; }
      QString getName() const { return _name; }

      const std::vector<NamedEventList>& midiActions() const { return _midiActions; }
      std::vector<Channel*> soundBankChannels() const { return _channel; }

      SoundBankOptions sbOptions() const { return _options; }
      const SoundBankSpannerDefaults& sbSpannerDefaults() const { return _spannerDefaults; }

      void readMatches(XmlReader &);
      void readSpanners(XmlReader &e);
      void read(XmlReader &);
      void write(Xml &) const;
      };

extern std::vector<SoundBank *> soundbanks;
extern void addToSoundBanks(QString filename);
extern void removeSoundBank(QString Bankname);
extern void soundbankMatchInstrumentTemplate(InstrumentTemplate itemp);
extern void soundbankMatchInstrumentTemplates();

}
#endif // SOUNDBANK_H
