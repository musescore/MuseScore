//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2014 John Pirie
//  Parts of the GPX Import based on code contributed by J.Jørgen von Bargen
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "importgtp.h"

#include <libmscore/score.h>
#include <libmscore/measurebase.h>
#include <libmscore/text.h>
#include <libmscore/stafftext.h>
#include <libmscore/box.h>
#include <libmscore/staff.h>
#include <libmscore/part.h>
#include <libmscore/measure.h>
#include <libmscore/timesig.h>
#include <libmscore/tremolo.h>
#include <libmscore/rest.h>
#include <libmscore/chord.h>
#include <libmscore/note.h>
#include <libmscore/stringdata.h>
#include <libmscore/clef.h>
#include <libmscore/lyrics.h>
#include <libmscore/tempotext.h>
#include <libmscore/slur.h>
#include <libmscore/tie.h>
#include <libmscore/tuplet.h>
#include <libmscore/barline.h>
#include <libmscore/excerpt.h>
#include <libmscore/stafftype.h>
#include <libmscore/bracket.h>
#include <libmscore/articulation.h>
#include <libmscore/keysig.h>
#include <libmscore/harmony.h>
#include <libmscore/bend.h>
#include <libmscore/tremolobar.h>
#include <libmscore/segment.h>
#include <libmscore/rehearsalmark.h>
#include <libmscore/repeat.h>
#include <libmscore/glissando.h>
#include <libmscore/dynamic.h>
#include <libmscore/arpeggio.h>
#include <libmscore/volta.h>
#include <libmscore/instrtemplate.h>
#include <libmscore/hairpin.h>
#include <libmscore/fingering.h>
#include <libmscore/sym.h>
#include <libmscore/ottava.h>
#include <libmscore/marker.h>
#include <libmscore/notedot.h>
#include "libmscore/sym.h"
#include "libmscore/bracketItem.h"
#include "libmscore/textline.h"
#include <libmscore/repeat.h>
// #include <symtext.h>

namespace Ms {


const std::map<QString, QString> GuitarPro6::instrumentMapping = {
            {"2Mrcs",           "maracas"},
            {"a-bass4",         "acoustic-bass"},
            {"a-bass5",         "acoustic-bass"},
            {"a-bass6",         "acoustic-bass"},
            {"alt-c",           "alto"},
            {"alt-s",           "alto"},
            {"a-piano-gs",      "piano"},
            {"a-piano-ss",      "piano"},
            {"bass-c",          "bass"},
            {"bass-flt-c",      "bass-flute"},
            {"bassn",           "bassoon"},
            {"bass-s",          "bass"},
            {"basstuba-eb",     "bass-eb-tuba"},
            {"bnj4",            "banjo"},
            {"bnj5",            "banjo"},
            {"bnj6",            "banjo"},
            {"bongo",           "bongos"},
            {"brthns",          "baritone-horn"},
            {"brtn-c",          "baritone"},
            {"brtn-s",          "baritone"},
            {"cbs",             "cabasa"},
            {"cello",           "violoncello"},
            {"china",           "chinese-tom-toms"},
            {"clrnt-a",         "a-clarinet"},
            {"clrnt-bb-bass",   "bass-clarinet"},
            {"clrnt-bb",        "bb-clarinet"},
            {"clrnt-c",         "c-clarinet"},
            {"clrnt-d",         "d-clarinet"},
            {"clrnt-eb",        "eb-clarinet"},
            {"clrnt",           "clarinet"},
            {"clst-gs",         "celesta"},
            {"clst-ss",         "celesta"},
            {"clvs",            "claves"},
            {"cngKit",          "congas"},
            {"conga",           "congas"},
            {"cowbell",         "cowbell"},
            {"crash",           "crash-cymbal"},
            {"cstnt",           "castanets"},
            {"ctbassn",         "contrabassoon"},
            {"ctbass",          "contrabass"},
            {"cuicaKit",        "cuica"},
            {"cuica",           "cuica"},
            {"drmkt",           "drumset"},
            {"e-bass4",         "bass-guitar"},
            {"e-bass5",         "bass-guitar"},
            {"e-bass6",         "bass-guitar"},
            {"e-gtr12",         "electric-guitar-treble-clef"},
            {"e-gtr6",          "electric-guitar-treble-clef"},
            {"e-gtr7",          "electric-guitar-treble-clef"},
            {"e-gtr8",          "electric-guitar-treble-clef"},
            {"em-organ-gs",     "organ"},
            {"em-organ-ss",     "organ"},
            {"en-horn",         "english-horn"},
            {"e-piano-gs",      "electric-piano"},
            {"e-piano-ss",      "electric-piano"},
            {"flt-c",           "flute"},
            {"flt-g",           "alto-flute"},
            {"flt-whstl",       "tin-whistle"},
            {"fr-horn",         "horn"},
            {"grcss",           "bass-drum"},   //grancassa is an alterantive name for bass drum
            {"guiro",           "guiro"},
            {"harp-gs",         "harp"},
            {"harp-ss",         "harp"},
            {"hclap",           "hand-clap"},
            {"hihat",           "hi-hat"},
            {"hrpch-gs",        "harpsichord"},
            {"hrpch-ss",        "harpsichord"},
            {"jngl-bell",       "sleigh-bells"},
            {"klmb",            "kalimba"},
            {"mrcs",            "maracas"},
            {"n-gtr6",          "guitar-nylon-treble-clef"},
            {"n-gtr7",          "guitar-nylon-treble-clef"},
            {"n-gtr8",          "guitar-nylon-treble-clef"},
            {"oboe",            "oboe"},
            {"ocrn",            "ocarina"},
            {"pccl",            "piccolo"},
            {"pedalhihat",      "hi-hat"},
            {"pnflt",           "pan-flute"},
            {"ptt",             "cymbal"},  //piatti is cymbal in italian
            {"rec",             "recorder"},
            {"ride",            "ride-cymbal"},
            {"rvs-cymb",        "cymbal"},
            {"sax-alt-eb",      "alto-saxophone"},
            {"sax-bar-eb",      "baritone-saxophone"},
            {"sax-bass-eb",     "bass-saxophone"},
            {"sax-ms-f",        "mezzo-soprano-saxophone"},
            {"sax-sop-bb",      "soprano-saxophone"},
            {"sax-ten-bb",      "tenor-saxophone"},
            {"sax-ten-c",       "melody-saxophone"},
            {"s-bass4",         "electric-bass"},
            {"s-bass5",         "5-string-electric-bass"},
            {"s-gtr12",         "guitar-steel-treble-clef"},
            {"s-gtr6",          "guitar-steel-treble-clef"},
            {"s-gtr7",          "guitar-steel-treble-clef"},
            {"s-gtr8",          "guitar-steel-treble-clef"},
            {"shkr",            "percussion"},
            {"shmsn",           "shamisen"},
            {"shn",             "sheng"},
            {"snare",           "snare-drum"},
            {"snr",             "snare-drum"},
            {"snt-brass-gs",    "brass-synthesizer"},
            {"snt-brass-ss",    "brass-synthesizer"},
            //{"snt-key-gs",    ""},
            //{"snt-key-ss",    ""},
            //{"snt-seq-gs",    ""},
            //{"snt-seq-ss",    ""},
            {"snt-lead-gs",     "poly-synth"},
            {"snt-lead-ss",     "poly-synth"},
            {"snt-pad",         "pad-synth"},
            {"snt-pad-gs",      "pad-synth"},
            {"snt-pad-ss",      "pad-synth"},
            {"splash",          "splash-cymbal"},
            {"sprn-c",          "soprano"},
            {"sprn-s",          "soprano"},
            {"tmbrn",           "tambourine"}, // to be mapped
            {"Tambourine-Perc", "tambourine"},
            {"Tambourine",      "tambourine"},
            {"tmblKit",         "timbales"},
            {"tmbl",            "timbales"},
            {"tmpn",            "timpani"},
            {"tnklbll",         "tubular-bells"}, //The short form does not match but this is very likely due to the description
            {"tnr-c",           "tenor"},
            {"tnr-s",           "tenor"},
            {"Triangle-Percu",  "triangle"},
            {"trmbn-bb-bass",   "bass-trombone"},
            {"trmbn-bb",        "tenor-trombone"},
            {"trmbn-bb-treble", "trombone-treble"},
            {"trmbn-eb",        "alto-trombone"},
            {"trmpt-a",         "a-trumpet"},
            {"trmpt-bb",        "bb-trumpet"},
            {"trmpt-c-bass",    "c-bass-trumpet"},
            {"trmpt-c",         "c-trumpet"},
            {"trmpt-d",         "d-trumpet"},
            {"trmpt-eb-bass",   "eb-bass-trumpet"},
            {"trmpt-eb",        "eb-trumpet"},
            {"trmpt-e",         "e-trumpet"},
            {"trmpt-f",         "f-trumpet"},
            {"trmpt-flgh",      "flugelhorn"},
            {"trngl",           "triangle"},
            {"ukll4",           "ukulele"},
            {"vbrphn",          "vibraphone"},
            {"vbrslp",          "vibraslap"}, // to be mapped
            {"vla",             "viola"},
            {"vln",             "violin"},
            {"wdblckKit",       "wood-blocks"},
            {"wdblck",          "wood-blocks"},
            {"whstlKit",        "slide-whistle"},
            {"whstl",           "tin-whistle"},
            {"xlphn",           "xylophone"}
            };

//---------------------------------------------------------
//   readBit
//---------------------------------------------------------

int GuitarPro6::readBit(QByteArray* buffer)
      {
      // calculate the byte index by dividing the position in bits by the bits per byte
      int byteIndex = position / BITS_IN_BYTE;

      // calculate our offset so we know how much to bit shift
      int byteOffset = ((BITS_IN_BYTE - 1) - (position % BITS_IN_BYTE));

      // calculate the bit which we want to read
      int bit = ((((*buffer)[byteIndex] & 0xff) >> byteOffset) & 0x01);

      // increment our current position so we know this bit has been read
      position++;
      return bit;       // return the bit we calculated
      }

//---------------------------------------------------------
//   readBits
//---------------------------------------------------------

int GuitarPro6::readBits(QByteArray* buffer, int bitsToRead)
      {
      int bits = 0;
      for (int i = (bitsToRead - 1); i >= 0; i--)
            bits |= (readBit(buffer) << i);
      return bits;
      }

//---------------------------------------------------------
//   readBitsReversed
//---------------------------------------------------------

int GuitarPro6::readBitsReversed(QByteArray* buffer, int bitsToRead)
      {
      int bits = 0;
      for( int i = 0; i < bitsToRead; i++)
            bits |= readBit(buffer) << i;
      return bits;
      }

//---------------------------------------------------------
//   getBytes
//---------------------------------------------------------

QByteArray GuitarPro6::getBytes(QByteArray* buffer, int offset, int length)
      {
      QByteArray newBytes;
      // compute new bytes from our buffer and return byte array
      for (int i = 0; i < length; i++) {
            if (buffer->length() > offset + i)
                  newBytes.insert(i, ((*buffer)[offset + i]));
            }
      return newBytes;
      }

//---------------------------------------------------------
//   readInteger
//---------------------------------------------------------

int GuitarPro6::readInteger(QByteArray* buffer, int offset)
      {
      // assign four bytes and take them from the buffer
      char bytes[4];
      bytes[0] = (*buffer)[offset + 0];
      bytes[1] = (*buffer)[offset + 1];
      bytes[2] = (*buffer)[offset + 2];
      bytes[3] = (*buffer)[offset + 3];
      // increment positioning so we keep track of where we are
      position += sizeof(int) * BITS_IN_BYTE;
      // bit shift in order to compute our integer value and return
      return ((bytes[3] & 0xff) << 24) | ((bytes[2] & 0xff) << 16) | ((bytes[1] & 0xff) << 8) | (bytes[0] & 0xff);
      }

//---------------------------------------------------------
//   readString
//---------------------------------------------------------

QByteArray GuitarPro6::readString(QByteArray* buffer, int offset, int length)
      {
      QByteArray filename;
      // compute the string by iterating through the buffer
      for (int i = 0; i < length; i++) {
            int charValue = (((*buffer)[offset + i]) & 0xff);
            if (charValue == 0)
                  break;
            filename.push_back((char)charValue);
            }
      return filename;
      }


//---------------------------------------------------------
//   unhandledNode
//---------------------------------------------------------

void GuitarPro6::unhandledNode(QString nodeName)
      {
      qDebug() << "WARNING: Discovered unhandled node name" << nodeName;
      }

//---------------------------------------------------------
//   readScore
//---------------------------------------------------------

void GuitarPro6::readScore(QDomNode* scoreNode)
      {
      // loop through the score meta-info, grabbing title, artist, etc
      QDomNode currentNode = scoreNode->firstChild();
      while (!currentNode.isNull()) {
            QString nodeName = currentNode.nodeName();
            if (!nodeName.compare("Title"))
                  title = currentNode.toElement().text();
            if (!nodeName.compare("Copyright"))
                  score->setMetaTag("copyright", currentNode.toElement().text());
            else if (!nodeName.compare("SubTitle"))
                  subtitle = currentNode.toElement().text();
            else if (!nodeName.compare("Artist"))
                  artist = currentNode.toElement().text();
            else if (!nodeName.compare("Album"))
                  album = currentNode.toElement().text();
            else if (nodeName == "FirstPageHeader") {}
            else if (nodeName == "FirstPageFooter") {}
            else if (nodeName == "PageHeader") {}
            else if (nodeName == "PageFooter") {}
            else if (nodeName == "ScoreSystemsDefaultLayout") {}
            else if (nodeName == "ScoreSystemsLayout") {}
            currentNode = currentNode.nextSibling();
            }
      }

//---------------------------------------------------------
//   readMasterTracks
//---------------------------------------------------------

void GuitarPro6::readMasterTracks(QDomNode* masterTrack)
      {
      // inspects MasterTrack, gives information applying to start of score such as tempo
      QDomNode currentNode = masterTrack->firstChild();
      while (!currentNode.isNull()) {
            QString nodeName = currentNode.nodeName();
            if (!nodeName.compare("Automations")) {
                  QDomNode currentAutomation = currentNode.firstChild();
                  bool linearTemp { false };
                  while (!currentAutomation.isNull()) {
                        if (!currentAutomation.nodeName().compare("Automation")) {
                              auto ln = currentAutomation.firstChildElement("Linear");
                              if (!ln.isNull())
                                    linearTemp = ln.text() == "true";
                              auto first_name = currentAutomation.firstChild().nodeName();
                              if (first_name == "Type")
                                    first_name = currentAutomation.firstChild().toElement().text();
                              if (!first_name.compare("Tempo")) {
                                    QString t = currentAutomation.lastChild().toElement().text();
                                    QStringList sa = t.split(" ");
                                    int curtempo = 120;
                                    if (sa.length() >= 1)
                                          curtempo = sa[0].toInt();
                                    auto barnode = currentAutomation.firstChildElement("Bar");
                                    if (!barnode.isNull()) {
                                          tempoMap[barnode.text().toInt()] = std::make_pair(curtempo, linearTemp);
                                          }
                                    }
                              }
                        currentAutomation = currentAutomation.nextSibling();
                        }
                  }
            currentNode = currentNode.nextSibling();
            }
      }

//---------------------------------------------------------
//   readChord
//---------------------------------------------------------

void GuitarPro6::readChord(QDomNode* diagram, int track)
      {
      // initialize a new fret diagram for our current track
      FretDiagram* fretDiagram = new FretDiagram(score);
      fretDiagram->setTrack(track);

      // get the identifier to set as the domain in the map
      int id    = diagram->attributes().namedItem("id").toAttr().value().toInt();
//TODO-ws      auto name = diagram->attributes().namedItem("name").toAttr().value();

//TODO-ws      fretDiagram->setChordName(name);
      QDomNode diagramNode = diagram->firstChild();

      // set the number of strings on this part
      int stringCount = diagramNode.attributes().namedItem("stringCount").toAttr().value().toInt();
      fretDiagram->setStrings(stringCount);

      // set the fret offset
      int baseFret = diagramNode.attributes().namedItem("baseFret").toAttr().value().toInt();
      fretDiagram->setFretOffset(baseFret);

      QDomNode diagramEntity = diagramNode.firstChild();
      int counter            = 0;
      while (!diagramEntity.isNull()) {
            QString nodeName = diagramEntity.nodeName();
            // new fret
            if (!nodeName.compare("Fret")) {
                  // get the string and fret numbers from the arguments to the node as integers
                  int string = diagramEntity.attributes().namedItem("string").toAttr().value().toInt();
                  int fret   = diagramEntity.attributes().namedItem("fret").toAttr().value().toInt();

                  // if there are unspecified string values, add the X marker to that string
                  while (counter < string) {
                        fretDiagram->setMarker(counter, FretMarkerType::CROSS);
                        counter++;
                        }

                  // look at the specified string/fret and add to diagram
                  if (fret == 0) {
                        fretDiagram->setMarker(string, FretMarkerType::CIRCLE);
                        counter++;
                        }
                  else {
                        fretDiagram->setDot(string, fret, true);
                        counter++;
                        }
                  }
            // move to the next string/fret specification
            diagramEntity = diagramEntity.nextSibling();
            }

      // mark any missing strings as 'X'
      while (counter < stringCount) {
            fretDiagram->setMarker(counter, FretMarkerType::CROSS);
            counter++;
            }

      // insert the fret diagram into the map of diagrams
      fretDiagrams.insert(id, fretDiagram);
      }

//---------------------------------------------------------
//   readTracks
//---------------------------------------------------------

void GuitarPro6::readTracks(QDomNode* track)
      {
      QDomNode nextTrack = track->firstChild();
      int trackCounter   = 0;
      while (!nextTrack.isNull()) {
            QDomNode currentNode = nextTrack.firstChild();
            Part* part           = new Part(score);
            bool hasTuning       = false;
            Staff* s             = new Staff(score);
            s->setPart(part);
            part->insertStaff(s, -1);
            score->staves().push_back(s);
            while (!currentNode.isNull()) {
                  QString nodeName = currentNode.nodeName();
                  if (nodeName == "Name")
                        part->setPartName(currentNode.toElement().text());
                  else if (nodeName == "Color") {}
                  // this is a typo is guitar pro - 'defaut' is correct here
                  else if (nodeName == "SystemsDefautLayout") {}
                  else if (nodeName == "SystemsLayout") {}
                  else if (nodeName == "RSE") {}
                  else if (nodeName == "GeneralMidi") {
                        if (currentNode.toElement().hasChildNodes()) {
                              auto prog = currentNode.firstChildElement("Program");
                              if (!prog.isNull()) {
                                    auto p = prog.text().toInt();
                                    part->instrument(Fraction(0,1))->channel(0)->setProgram(p);
                                    }
                              int midiChannel = currentNode.firstChildElement("PrimaryChannel").text().toInt();
                              //if (!prog.isNull() && midiChannel != GP_DEFAULT_PERCUSSION_CHANNEL)
                              // midiChannel += prog.text().toInt();
                              part->setMidiChannel(midiChannel);
                              if (midiChannel == GP_DEFAULT_PERCUSSION_CHANNEL) {
                                    part->instrument()->setDrumset(gpDrumset);
                                    s->setStaffType(Fraction(0,1), *StaffType::preset(StaffTypes::PERC_DEFAULT));
                                    }
                              }
                        }
                  else if (nodeName == "PlaybackState") {}
                  else if (nodeName == "PlayingStyle") {}
                  else if (nodeName == "PageSetup") {}
                  else if (nodeName == "MultiVoice") {}
                  else if (nodeName == "ShortName") {}
                  else if (nodeName == "Instrument") {
                        QString ref = currentNode.attributes().namedItem("ref").toAttr().value();
                        auto it     = instrumentMapping.find(ref);
                        if (it != instrumentMapping.end()) {
                              part->setInstrument(Instrument::fromTemplate(Ms::searchTemplate(it->second)));
                              }
                        else
                              qDebug() << "Unknown instrument: " << ref;
                        if (ref.endsWith("-gs") || ref.startsWith("2")) { // grand staff
                              Staff* s2 = new Staff(score);
                              s2->setPart(part);
                              part->insertStaff(s2, -1);
                              score->staves().push_back(s2);
                              s->addBracket(new BracketItem(s->score(), BracketType::BRACE, 2));
                              s->setBarLineSpan(2);
                              }
                        }
                  else if (nodeName == "PartSounding")
                        part->instrument()->setTranspose(Interval(currentNode.firstChildElement("TranspositionPitch").text().toInt()));
                  else if (nodeName == "Properties")
                        readTrackProperties(currentNode, part, trackCounter, hasTuning);
                  currentNode = currentNode.nextSibling();
                  }

            // add in a new part
            score->appendPart(part);
            trackCounter++;
            nextTrack = nextTrack.nextSibling();

            if (!hasTuning) {
                  tunings.push_back("");
                  }
            }

      previousDynamic = new int[score->staves().length() * VOICES];
      // initialise the dynamics to 0
      for (int i = 0; i < score->staves().length() * VOICES; i++)
            previousDynamic[i] = 0;
      // set the number of staves we need
      staves = score->staves().length();
      }

//---------------------------------------------------------
//   readTrackProperties
//---------------------------------------------------------

void GuitarPro6::readTrackProperties(const QDomNode& currentNode, Part* part, int trackCounter, bool& hasTuning)
      {
      QDomNode currentProperty = currentNode.firstChild();
      while (!currentProperty.isNull()) {
            QString propertyName = currentProperty.attributes().namedItem("name").toAttr().value();
            if (!propertyName.compare("Tuning")) {
                  // set up the tuning for the part
                  QString tuningString = currentProperty.firstChild().toElement().text();
                  QStringList tuningStringList = tuningString.split(" ");
                  int strings = 0;
                  std::vector<int> tuning(tuningStringList.length());
                  //int tuning[tuningStringList.length()];
                  int frets = 24;
                  for (auto iter = tuningStringList.begin(); iter != tuningStringList.end(); ++iter) {
                        int currentString = (*iter).toInt();
                        tuning[strings] = currentString;
                        strings++;
                        }
                  StringData* stringData = new StringData(frets, strings, &tuning[0]);
                  Instrument* instr = part->instrument();
                  instr->setStringData(*stringData);
                  instr->setSingleNoteDynamics(false);
                  hasTuning = true;
                  createTuningString(strings, &tuning[0]);
                  }
            else if (!propertyName.compare("DiagramCollection")) {
                  QDomNode items = currentProperty.firstChild();
                  QDomNode currentItem = items.firstChild();
                  while (!currentItem.isNull()) {
                        readChord(&currentItem, trackCounter);
                        currentItem = currentItem.nextSibling();
                        }
                  }
            currentProperty = currentProperty.nextSibling();
            }
      }

//---------------------------------------------------------
//   getNode
//---------------------------------------------------------

QDomNode GuitarPro6::getNode(const QString& id, QDomNode currentDomNode)
      {
      while (!(currentDomNode).isNull()) {
            QString currentId = currentDomNode.attributes().namedItem("id").toAttr().value();
            if (id.compare(currentId) == 0) {
                  return currentDomNode;
                  }
            currentDomNode = currentDomNode.nextSibling();
            }
      qDebug() << "WARNING: A null node was returned when search for the identifier" << id << ". Your Guitar Pro file may be corrupted.";
      return currentDomNode;
      }

//---------------------------------------------------------
//   findNumMeasures
//---------------------------------------------------------

int GuitarPro6::findNumMeasures(GPPartInfo* partInfo)
      {
      QDomNode masterBar = partInfo->masterBars.nextSibling();
      QDomNode masterBarElement;
      while (!masterBar.isNull()) {
            GpBar gpBar;
            gpBar.freeTime       = false;
            gpBar.direction      = "";
            gpBar.directionStyle = "";
            masterBarElement     = masterBar.firstChild();
            while (!masterBarElement.isNull()) {
                  if (!masterBarElement.nodeName().compare("Key"))
                        gpBar.keysig = masterBarElement.firstChild().toElement().text().toInt();
                  else if (!masterBarElement.nodeName().compare("Time")) {
                        QString timeSignature            = masterBarElement.toElement().text();
                        QList<QString> timeSignatureList = timeSignature.split("/");
                        gpBar.timesig = Fraction(timeSignatureList.first().toInt(), timeSignatureList.last().toInt());
                        }
                  else if (!masterBarElement.nodeName().compare("Directions")) {
                        auto element = masterBarElement.firstChild();
                        while (!element.isNull()) {
                              gpBar.directions.push_back(element.toElement().text());
                              element = element.nextSibling();
                              }
                        //gpBar.direction = masterBarElement.firstChild().toElement().text();
                        gpBar.directionStyle = masterBarElement.firstChild().nodeName();
                        }
                  else if (!masterBarElement.nodeName().compare("FreeTime")) {
                        gpBar.freeTime = true;
                        gpBar.barLine  = BarLineType::BROKEN;
                        }
                  else if (!masterBarElement.nodeName().compare("DoubleBar"))
                        gpBar.barLine = BarLineType::DOUBLE;
                  else if (!masterBarElement.nodeName().compare("Section")) {
                        auto section = masterBarElement.firstChild();
                        while (!section.isNull())
                              {
                              if (!section.nodeName().compare("Letter"))
                                    gpBar.section[0] = section.toElement().text();
                              else if (!section.nodeName().compare("Text"))
                                    gpBar.section[1] = section.toElement().text();
                              section = section.nextSibling();
                              }
                        }
                  masterBarElement = masterBarElement.nextSibling();
                  }
            bars.append(gpBar);
            if (masterBar.nextSibling().isNull())
                  break;
            masterBar = masterBar.nextSibling();
            }
      QString b = masterBar.lastChildElement("Bars").toElement().text();
      //work out the number of measures (add 1 as couning from 0, and divide by number of parts)
      int numMeasures = (b.split(" ").last().toInt() + 1) / score->parts().length();

      if (numMeasures > b.size()) {
            qDebug("GuitarPro6:findNumMeasures: bars %d < numMeasures %d\n", b.size(), numMeasures);
            // HACK (ws)
            numMeasures = b.size();
            }
      return numMeasures;
      }

//---------------------------------------------------------
//   fermataToFraction
//---------------------------------------------------------

Fraction GuitarPro6::fermataToFraction(int numerator, int denominator)
      {
      numerator++;
      // minim through to hemidemisemiquaver
      if (denominator == 0)       { denominator = 2; }
      else if (denominator == 1) {
            denominator = 4;
            }
      else if (denominator == 2) {
            denominator = 8;
            }
      else if (denominator == 3) {
            denominator = 12;
            }
      else if (denominator == 4) {
            denominator = 16;
            }
      else if (denominator == 5) {
            denominator = 32;
            }

      return Fraction(numerator, denominator);
      }

//---------------------------------------------------------
//   rhythmToDuration
//---------------------------------------------------------

Fraction GuitarPro6::rhythmToDuration(QString value)
      {
      Fraction l;
      if (value.compare("Whole") == 0)
            l.set(1, 1);
      else if (value.compare("Half") == 0)
            l.set(1, 2);
      else if (value.compare("Quarter") == 0)
            l.set(1, 4);
      else if (value.compare("Eighth") == 0)
            l.set(1,8);
      else if (value.compare("16th") == 0)
            l.set(1,16);
      else if (value.compare("32nd") == 0)
            l.set(1,32);
      else if (value.compare("64th") == 0)
            l.set(1,64);
      else if (value.compare("128th") == 0)
            l.set(1,128);
      else
            qFatal( "Error - unknown note length: %s", qPrintable(value));
      return l;
      }

//---------------------------------------------------------
//   readDrumNote
//---------------------------------------------------------

void GuitarPro6::readDrumNote(Note* note, int element, int variation)
      {
      int pitch = 44;
      /* These numbers below were determined by creating all drum
       * notes in a GPX format file and then analyzing the score.gpif
       * file which specifies the score and then matching as much
       * as possible with the gpDrumset...   */
      if (element == 11 && variation == 0)  // pedal hihat
            pitch = 44;
      else if (element == 0 && variation == 0) // Kick (hit)
            pitch = 36; // or 36
      else if (element == 5 && variation == 0) // Tom very low (hit)
            pitch = 41;
      else if (element == 6 && variation == 0) // Tom low (hit)
            pitch = 43;
      else if (element == 7 && variation == 0) // Tom medium (hit)
            pitch = 45;
      else if (element == 1 && variation == 0) // Snare (hit)
            pitch = 40; //or 40
      else if (element == 1 && variation == 1) // Snare (rim shot)
            pitch = 91;
      else if (element == 1 && variation == 2) // Snare (side stick)
            pitch = 37;
      else if (element == 8 && variation == 0) // Tom high (hit)
            pitch = 48;
      else if (element == 9 && variation == 0) // Tom very high (hit)
            pitch = 50;
      else if (element == 15 && variation == 0) // Ride (middle)
            pitch = 51;
      else if (element == 15 && variation == 1) // Ride (edge)
            pitch = 59;
      else if (element == 15 && variation == 2) // Ride (bell)
            pitch = 59;
      else if (element == 10 && variation == 0) // Hihat (closed)
            pitch = 42;
      else if (element == 10 && variation == 1) // Hihat (half)
            pitch = 46;
      else if (element == 10 && variation == 2) // Hihat (open)
            pitch = 46;
      else if (element == 12 && variation == 0) // Crash medium (hit)
            pitch = 49;
      else if (element == 14 && variation == 0) // Splash (hit)
            pitch = 55;
      else if (element == 13 && variation == 0) // Crash high (hit)
            pitch = 57;
      else if (element == 16 && variation == 0) // China (hit)
            pitch = 52;
      else if (element == 4 && variation == 0) // Cowbell high (hit)
            pitch = 102;
      else if (element == 3 && variation == 0) // Cowbell medium (hit)
            pitch = 56;
      else if (element == 2 && variation == 0) // Cowbell low (hit)
            pitch = 99;

      note->setPitch(pitch);
      }


//---------------------------------------------------------
//   makeTie
//---------------------------------------------------------

void GuitarPro6::makeTie(Note* note)
      {
      bool found       = false;
      Chord* chord     = note->chord();
      Segment* segment = chord->segment();
      if (!segment)
            return;
      segment = segment->prev1(SegmentType::ChordRest);
      int track = note->track();
      while (segment) {
            Element* e = segment->element(track);
            if (e) {
                  if (e->type() == ElementType::CHORD) {
                        Chord* chord2 = static_cast<Chord*>(e);
                        foreach(Note* note2, chord2->notes()) {
                              if (note2->string() == note->string()) {
                                    Tie* tie = new Tie(score);
                                    tie->setEndNote(note);
                                    note2->add(tie);
                                    note->setFret(note2->fret());
                                    note->setPitch(note2->pitch());
                                    found = true;
                                    break;
                                    }
                              }
                        }
                  if (found)
                        break;
                  }
            segment = segment->prev1(SegmentType::ChordRest);
            }
      }

//---------------------------------------------------------
//   readBeats
//---------------------------------------------------------

Fraction GuitarPro6::readBeats(QString beats, GPPartInfo* partInfo, Measure* measure, const Fraction& startTick, int staffIdx, int voiceNum, Tuplet* tuplets[], int measureCounter)
      {
      bool wrong_pause = false;
      Lyrics* lyric    = nullptr;
      Fraction beatsTick    = {0,1};

      // we must count from the start of the bar, so declare a fraction to track this
      Fraction fermataIndex(0,1);
      int track            = staffIdx * VOICES + voiceNum;
      auto currentBeatList = beats.split(" ");
      bool startSlur = false;
      bool endSlur = false;
      for (auto currentBeat = currentBeatList.begin(); currentBeat != currentBeatList.end(); currentBeat++) {
            int sl = -1;
            if (slides.contains(staffIdx * VOICES + voiceNum))
                  sl = slides.take(staffIdx * VOICES + voiceNum);

            Fraction l;
            int dotted           = 0;
            QDomNode beat        = getNode(*currentBeat, partInfo->beats);
            Fraction currentTick = startTick + beatsTick;
            Segment* segment     = measure->getSegment(SegmentType::ChordRest, currentTick);
            QDomNode currentNode = beat.firstChild();
            bool noteSpecified   = false;
            ChordRest* cr        = segment->cr(track);
            bool tupletSet       = false;
            Tuplet* tuplet       = tuplets[staffIdx * VOICES + voiceNum];
            int whammyOrigin     = -1;
            int whammyMiddle     = -1;
            int whammyEnd        = -1;
            bool graceNote       = false;
            Note* lyrNote(nullptr);
            std::map<int, QString> lyrics;
            while (!currentNode.isNull()) {
                  if (currentNode.nodeName() == "GraceNotes") {
                        graceNote = true;
                        }
                  else if (currentNode.nodeName() == "Notes") {
                        noteSpecified = true;
                        auto notesList = currentNode.toElement().text().split(" ");

                        // this could be set by rhythm if we dealt with a tuplet
                        if (!cr)
                              cr = new Chord(score);
                        if (lyric) {
                              cr->add(lyric);
                              lyric = nullptr;
                              }
                        cr->setTrack(track);
                        cr->setTicks(l);
                        TDuration d(l);
                        d.setDots(dotted);

                        cr->setDurationType(d);

                        if (cr->isChord()) {
                              auto lyrchord = toChord(cr);
                              if (lyrchord && lyrchord->notes().size())
                                    lyrNote = lyrchord->notes().front();
                              }

                        if (!segment->cr(track))
                              segment->add(cr);

                        if (startSlur) {
                                    Slur* slur = new Slur(score);
                                    slur->setParent(0);
                                    slur->setTrack(track);
                                    slur->setTrack2(track);
                                    legatos[track] = slur;
                                    slur->setTick(cr->tick());
                                    slur->setTick2(cr->tick());
                                    startSlur = false;
                                    }
                        if (endSlur) {
                              Slur* slur = legatos[track];
                              if (slur) {
                                    slur->setTrack2(track);
                                    slur->setTick2(cr->tick());
                                    score->addElement(slur);
                                    legatos[track] = 0;
                                    }
                              endSlur = false;
                              }

                        Chord* lastChord { nullptr };
                        for (auto iter = notesList.begin(); iter != notesList.end(); ++iter) {
                              // we have found a note
                              QDomNode dnote = getNode(*iter, partInfo->notes);
                              int id        = -1;
                              auto idx      = dnote.attributes().namedItem("id");
                              if (!idx.isNull())
                                    id = idx.nodeValue().toInt() - 1;
                              QDomNode currentNote = (dnote).firstChild();
                              bool tie             = false;
                              bool trill           = false;
                              // if a <Notes> tag is used but there is no <Note>, then we add a rest. This flag will allow us to check this.
                              while (!currentNote.isNull()) {
                                    if (!currentNote.nodeName().compare("Tie")) {
                                          if (!currentNote.attributes().namedItem("destination").toAttr().value().compare("true"))
                                                tie = true;
                                          }

                                    if (currentNote.nodeName() == "Properties") {
                                          QDomNode currentProperty = currentNote.firstChild();
                                          // these should not be in this scope - they may not even exist.
                                          QString stringNum;
                                          QString fretNum;
                                          QString tone;
                                          QString octave;
                                          QString midi;
                                          QString element;
                                          QString variation;

                                          Note* note = new Note(score);
                                          if (graceNote)
                                                lyrNote = note;
                                          if (id != -1) {
                                                auto iter1 = lyrics.find(id);
                                                if (iter1 != lyrics.end()) {
                                                      auto lyr = new Lyrics(score);
                                                      lyr->setPlainText(iter1->second);
                                                      cr->add(lyr);
                                                      }
                                                }
                                          lyrNote = note;
                                          if (dotted) {
                                                // there is at most one dotted note in this guitar pro version
                                                NoteDot* dot = new NoteDot(score);
                                                dot->setParent(note);
                                                dot->setTrack(track);  // needed to know the staff it belongs to (and detect tablature)
                                                dot->setVisible(true);
                                                note->add(dot);
                                                }

                                          Chord* chord = static_cast<Chord*>(cr);
                                          chord->add(note);
                                          QString harmonicText = "";
                                          bool use_harmonic    = true;
                                          bool hasSlur         = false;
                                          bool check_slide_map = true;

                                          while (!currentProperty.isNull()) {
                                                QString argument = currentProperty.attributes().namedItem("name").toAttr().value();
                                                if (argument == "String") {
                                                      stringNum = currentProperty.firstChild().toElement().text();
                                                      if (check_slide_map) {
                                                            int string = stringNum.toInt();
                                                            if (slideMap.find({ string, staffIdx }) != slideMap.end()) {
                                                                  Note* start  = slideMap[{ string, staffIdx }];
                                                                  Glissando* s = new Glissando(score);
                                                                  s->setGlissandoType(GlissandoType::STRAIGHT);
                                                                  note->chord()->add(s);
                                                                  s->setAnchor(Spanner::Anchor::NOTE);
                                                                  s->setStartElement(start);
                                                                  s->setTick(start->chord()->tick());
                                                                  s->setTrack(staffIdx);
                                                                  s->setParent(start);
                                                                  s->setEndElement(note);
                                                                  s->setTick2(note->chord()->tick());
                                                                  s->setTrack2(staffIdx);
                                                                  score->addElement(s);
                                                                  slideMap.erase({ string, staffIdx });
                                                                  if (slurs[staffIdx])
                                                                        createSlur(false, track, note->chord());
                                                                  }
                                                            }
                                                      }
                                                else if (argument == "Element")
                                                      element = currentProperty.firstChild().toElement().text();
                                                else if (argument == "Slide") {
                                                      int slideKind = currentProperty.firstChild().toElement().text().toInt();
                                                      if (slideKind & (SHIFT_SLIDE | LEGATO_SLIDE)) {
                                                            auto string = note->string();
                                                            if (string == -1) {
                                                                  for (auto node = currentNote.firstChild(); !node.isNull(); node = node.nextSibling()) {
                                                                        auto arg = node.attributes().namedItem("name").toAttr().value();
                                                                        if (arg == "String") {
                                                                              string = node.firstChild().toElement().text().toInt();
                                                                              break;
                                                                              }
                                                                        }
                                                                  }
                                                            if (slideKind & LEGATO_SLIDE)
                                                                  note->setFlag(ElementFlag::HAS_TAG, true);
                                                            slideMap.insert({ { string, staffIdx }, note });
                                                            slideKind      &= ~(SHIFT_SLIDE | LEGATO_SLIDE);
                                                            check_slide_map = false;
                                                            }
                                                      if (slideKind) {
                                                            createSlide(slideKind, note->chord(), staffIdx, note);
#if 0
                                                            if (slideKind >= 4)
                                                                  slide = slideKind;
                                                            else
                                                                  slides->insert(staffIdx * VOICES + voiceNum, slideKind);
#endif
                                                            }
                                                      }
                                                else if (!argument.compare("HopoOrigin")) {
                                                      hasSlur = true;
                                                      createSlur(true, track, cr);
                                                      }
                                                else if (!argument.compare("HopoDestination") && !hasSlur) {
                                                      createSlur(false, track, cr);
                                                      }
                                                else if (argument == "Variation")
                                                      variation = currentProperty.firstChild().toElement().text();
                                                else if (argument == "Fret")
                                                      fretNum = currentProperty.firstChild().toElement().text();
                                                else if (argument == "Tone")
                                                      tone = currentProperty.firstChild().toElement().text();
                                                else if (argument == "Octave")
                                                      octave = currentProperty.firstChild().toElement().text();
                                                else if (argument == "Midi")
                                                      midi = currentProperty.firstChild().toElement().text();
                                                else if (argument == "Muted") {
                                                      if (!currentProperty.firstChild().nodeName().compare("Enable")) {
                                                            note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
                                                            note->setGhost(true);
                                                            }
                                                      }
                                                else if (argument == "Tapped") {
                                                      if (!currentProperty.firstChild().nodeName().compare("Enable"))
                                                            addTap(note);
                                                      }
                                                else if (argument == "LeftHandTapped") {
                                                      if (!currentProperty.firstChild().nodeName().compare("Enable")) {
                                                            Symbol* sym = new Symbol(note->score());
                                                            sym->setSym(SymId::articLaissezVibrerAbove);
                                                            sym->setParent(note);
                                                            note->add(sym);
                                                            //note->setStemThrough(true);
                                                            //articLaissezVibrerAbove
                                                            /*
                                                            Symbol* leftSym = new Symbol(note->score());
                                                            Symbol* rightSym = new Symbol(note->score());
                                                            leftSym->setSym(SymId::noteheadParenthesisLeft);
                                                            rightSym->setSym(SymId::noteheadParenthesisRight);
                                                            leftSym->setParent(note);
                                                            rightSym->setParent(note);
                                                            note->add(leftSym);
                                                            note->add(rightSym);
                                                            */
                                                            }
                                                      }
                                                else if (argument == "Bended") {
                                                      if (!currentProperty.firstChild().nodeName().compare("Enable")) {
                                                            auto props = currentProperty.nextSibling();
                                                            int origin(0);
                                                            int destination(0);
                                                            int off1(15);
                                                            int off2(0);
                                                            int offdest(60);
                                                            int middleval(0);
                                                            bool has_middle = false;
                                                            while (!props.isNull()) {
                                                                  auto name = props.attributes().namedItem("name").toAttr().value();
                                                                  if (name == "BendOriginValue") {
                                                                        origin = props.firstChildElement("Float").toElement().text().toInt();
                                                                        }
                                                                  else if (name == "BendDestinationValue") {
                                                                        destination = props.firstChildElement("Float").toElement().text().toInt();
                                                                        }
                                                                  else if (name == "BendMiddleOffset1") {
                                                                        off1 = props.firstChildElement("Float").toElement().text().toInt();
                                                                        }
                                                                  else if (name == "BendMiddleOffset2") {
                                                                        off2 = props.firstChildElement("Float").toElement().text().toInt();
                                                                        }
                                                                  else if (name == "BendDestinationOffset") {
                                                                        offdest = props.firstChildElement("Float").toElement().text().toInt();
                                                                        }
                                                                  else if (name == "BendMiddleValue") {
                                                                        middleval  = props.firstChildElement("Float").toElement().text().toInt();
                                                                        has_middle = true;
                                                                        }
                                                                  props = props.nextSibling();
                                                                  }

                                                            Bend* bend = new Bend(note->score());
                                                            //bend->setNote(note); //TODO
                                                            bend->points().append(PitchValue(0, origin));
                                                            bend->points().append(PitchValue(off1, has_middle ? middleval : destination));
                                                            if (has_middle)
                                                                  bend->points().append(PitchValue(off2, middleval));
                                                            bend->points().append(PitchValue(offdest, destination));
                                                            note->add(bend);
                                                            }
                                                      }
                                                else if (argument == "PalmMuted") {
                                                      if (!currentProperty.firstChild().nodeName().compare("Enable"))
                                                            addPalmMute(note);
                                                      }
                                                else if (!argument.compare("HarmonicType")) {
                                                      QString type;
                                                      auto inner = currentProperty.firstChildElement("HType");
                                                      if (inner.isNull())
                                                            type = currentProperty.toElement().text();
                                                      else
                                                            type = inner.toElement().text();
                                                      // add the same text to the note that Guitar Pro does
                                                      if (!type.compare("Feedback"))
                                                            harmonicText = "Fdbk.";
                                                      else if (!type.compare("Semi"))
                                                            harmonicText = "S.H.";
                                                      else if (!type.compare("Pinch"))
                                                            harmonicText = "P.H.";
                                                      else if (!type.compare("Tap"))
                                                            harmonicText = "T.H.";
                                                      else if (!type.compare("Artificial"))
                                                            harmonicText = "A.H.";
                                                      else {
                                                            harmonicText = type;
                                                            use_harmonic = false;
                                                            }
                                                      }
                                                else if (!argument.compare("HarmonicFret")) {
                                                      QString value;
                                                      auto inner = currentProperty.firstChildElement("HFret");
                                                      if (inner.isNull())
                                                            value = currentProperty.toElement().text();
                                                      else
                                                            value = inner.toElement().text();
                                                      Note* harmonicNote = nullptr;

                                                      // natural harmonic = artificial harmonic?
                                                      if (harmonicText.length()) {
                                                            if (!harmonicText.compare("Natural")) {
                                                                  harmonicNote = note;
                                                                  //note->setHarmonic(true); //TODO
                                                                  }
                                                            else {
                                                                  harmonicNote = new Note(score);
                                                                  // harmonicNote->setHarmonic(true);
                                                                  //harmonicNote->setTrillFret(11);
                                                                  chord->add(harmonicNote);

                                                                  //harmonicNote->setVisible(false);
                                                                  }
                                                            }

                                                      if (harmonicNote) {
                                                      #if 0
                                                            if (harmonicNote->harmonic())
                                                                  value = "";
                                                            if (harmonicText == "A.H.")
                                                                  harmonicNote->setHarmonic(true);
                                                      #endif
                                                            Staff* staff        = note->staff();
                                                            int harmonicFret    = fretNum.toInt();
                                                            int musescoreString = staff->part()->instrument()->stringData()->strings() - 1 - stringNum.toInt();
                                                            harmonicNote->setString(musescoreString);
                                                            harmonicNote->setFret(harmonicFret);                     // add the octave for the harmonic
                                                            harmonicNote->setHeadGroup(NoteHead::Group::HEAD_DIAMOND);
                                                            if (!value.compare("12"))
                                                                  harmonicFret += 12;
                                                            else if (!value.compare("7") || !value.compare("19"))
                                                                  harmonicFret += 19;
                                                            else if (!value.compare("5") || !value.compare("24"))
                                                                  harmonicFret += 24;
                                                            else if (!value.compare("3.9") || !value.compare("4") || !value.compare("9") || !value.compare("16"))
                                                                  harmonicFret += 28;
                                                            else if (!value.compare("3.2"))
                                                                  harmonicFret += 31;
                                                            else if (!value.compare("2.7"))
                                                                  harmonicFret += 34;
                                                            else if (!value.compare("2.3") || !value.compare("2.4"))
                                                                  harmonicFret += 36;
                                                            else if (!value.compare("2"))
                                                                  harmonicFret += 38;
                                                            else if (!value.compare("1.8"))
                                                                  harmonicFret += 40;
                                                            //harmonicNote->setFret(harmonicFret);
                                                            harmonicNote->setPitch(staff->part()->instrument()->stringData()->getPitch(musescoreString, harmonicFret, nullptr, Fraction(0,1)));
                                                            harmonicNote->setTpcFromPitch();
                                                            if (harmonicText.length() && harmonicText.compare("Natural")) {
                                                                  harmonicNote->setFret(fretNum.toInt());
                                                                  if (use_harmonic)
                                                                        harmonicText += "\\";
                                                                  addTextToNote(harmonicText, Align::CENTER, harmonicNote);
                                                                  }
                                                            }
                                                      }
                                                currentProperty = currentProperty.nextSibling();
                                                }

                                          if (midi != "")
                                                note->setPitch(midi.toInt());
                                          else if (element != "")
                                                readDrumNote(note, element.toInt(), variation.toInt());
                                          else if (stringNum != "" && stringNum.toInt() >= 0 && note->headGroup() != NoteHead::Group::HEAD_DIAMOND) {
                                                Staff* staff        = note->staff();
                                                int fretNumber      = fretNum.toInt();
                                                int musescoreString = staff->part()->instrument()->stringData()->strings() - 1 - stringNum.toInt();
                                                auto pitch          = staff->part()->instrument()->stringData()->getPitch(musescoreString, fretNumber, nullptr, Fraction(0,1));
                                                note->setFret(fretNumber);
                                                // we need to turn this string number for GP to the correct string number for musescore
                                                note->setString(musescoreString);
                                                note->setPitch(pitch);
                                                }
                                          else if (tone != "")
                                                note->setPitch((octave.toInt() * 12) + tone.toInt()); // multiply octaves by 12 as 12 semitones in octave
                                          if (tie) {
                                                makeTie(note);
                                                tie = false;
                                                }
                                          if (trill) {
                                                Articulation* art = new Articulation(note->score());
                                                art->setSymId(SymId::ornamentTrill);
                                                if (!note->score()->addArticulation(note, art))
                                                      delete art;
                                                }
                                          QDomNode tremoloNode = currentNode.parentNode().firstChildElement("Tremolo");
                                          if (!tremoloNode.isNull()) {
                                                QString value = tremoloNode.toElement().text();
                                                Tremolo* t    = new Tremolo(chord->score());
                                                if (!value.compare("1/2")) {
                                                      t->setTremoloType(TremoloType::R8);
                                                      chord->add(t);
                                                      }
                                                else if (!value.compare("1/4")) {
                                                      t->setTremoloType(TremoloType::R16);
                                                      chord->add(t);
                                                      }
                                                else if (!value.compare("1/8")) {
                                                      t->setTremoloType(TremoloType::R32);
                                                      chord->add(t);
                                                      }
                                                }
                                          QDomNode wahNode = currentNode.parentNode().firstChildElement("Wah");
                                          if (!wahNode.isNull()) {
                                                QString value = wahNode.toElement().text();
                                                if (!value.compare("Open")) {
                                                      Articulation* art = new Articulation(note->score());
                                                      art->setSymId(SymId::brassMuteOpen);
                                                      if (!note->score()->addArticulation(note, art))
                                                            delete art;
                                                      }
                                                else if (!value.compare("Closed")) {
                                                      Articulation* art = new Articulation(note->score());
                                                      art->setSymId(SymId::brassMuteClosed);
                                                      if (!note->score()->addArticulation(note, art))
                                                            delete art;
                                                      }
                                                }
                                          QDomNode accentNode = currentNote.parentNode().firstChildElement("Accent");
                                          if (!accentNode.isNull()) {
                                                int value   = accentNode.toElement().text().toInt();
                                                SymId symId = SymId::articStaccatoAbove;
                                                switch (value) {
                                                      case 1: symId  = SymId::articStaccatoAbove; break;
                                                      case 2: symId  = SymId::articStaccatissimoAbove; break;
                                                      case 4: symId  = SymId::articMarcatoAbove; break;
                                                      case 8: symId  = SymId::articAccentAbove; break;
                                                      case 16: symId = SymId::articTenutoAbove; break;
                                                      }
                                                Articulation* art = new Articulation(note->score());
                                                art->setSymId(symId);
                                                if (!note->score()->addArticulation(note, art))
                                                      delete art;
                                                }
                                          QDomNode ornamentNode = currentNote.parentNode().firstChildElement("Ornament");
                                          if (!ornamentNode.isNull()) {
                                                QString value = ornamentNode.toElement().text();
                                                // guitar pro represents the turns the other way to what we do
                                                if (!value.compare("InvertedTurn")) {
                                                      Articulation* art = new Articulation(note->score());
                                                      art->setSymId(SymId::ornamentTurn);
                                                      if (!note->score()->addArticulation(note, art))
                                                            delete art;
                                                      }
                                                else if (!value.compare("Turn")) {
                                                      Articulation* art = new Articulation(note->score());
                                                      art->setSymId(SymId::ornamentTurnInverted);
                                                      if (!note->score()->addArticulation(note, art))
                                                            delete art;
                                                      }
                                                else if (!value.compare("LowerMordent")) {
                                                      Articulation* art = new Articulation(note->score());
                                                      art->setSymId(SymId::ornamentMordent);
                                                      if (!note->score()->addArticulation(note, art))
                                                            delete art;
                                                      }
                                                else if (!value.compare("UpperMordent")) {
                                                      Articulation* art = new Articulation(note->score());
                                                      art->setSymId(SymId::ornamentShortTrill);
                                                      if (!note->score()->addArticulation(note, art))
                                                            delete art;
                                                      }
                                                }
                                          QDomNode propertiesNode = currentNode.parentNode().firstChildElement("Properties");
                                          if (!propertiesNode.isNull()) {
                                                QDomNode currentProperty1 = propertiesNode.firstChild();
                                                QString barreFret        = "";
                                                bool halfBarre           = false;
                                                while (!currentProperty1.isNull()) {
                                                      QString argument = currentProperty1.attributes().namedItem("name").toAttr().value();
                                                      if (!argument.compare("PickStroke")) {
                                                            if (!currentProperty1.firstChild().toElement().text().compare("Up")) {
                                                                  Articulation* art = new Articulation(note->score());
                                                                  art->setSymId(SymId::stringsUpBow);
                                                                  if (!note->score()->addArticulation(note, art))
                                                                        delete art;
                                                                  }
                                                            else if (!currentProperty1.firstChild().toElement().text().compare("Down")) {
                                                                  Articulation* art = new Articulation(note->score());
                                                                  art->setSymId(SymId::stringsDownBow);
                                                                  if (!note->score()->addArticulation(note, art))
                                                                        delete art;
                                                                  }
                                                            }
                                                      else if (!argument.compare("Brush")) {
                                                            Arpeggio* a = new Arpeggio(score);
                                                            // directions in arpeggion type are reversed, they are correct below
                                                            if (!currentProperty1.firstChild().toElement().text().compare("Up"))
                                                                  a->setArpeggioType(ArpeggioType::DOWN_STRAIGHT);
                                                            else if (!currentProperty1.firstChild().toElement().text().compare("Down"))
                                                                  a->setArpeggioType(ArpeggioType::UP_STRAIGHT);
                                                            chord->add(a);
                                                            }
                                                      else if (!argument.compare("Slapped")) {
                                                            if (!currentProperty1.firstChild().nodeName().compare("Enable"))
                                                                  addSlap(note);
                                                            }
                                                      else if (!argument.compare("Popped")) {
                                                            if (!currentProperty1.firstChild().nodeName().compare("Enable"))
                                                                  addPop(note);
                                                            }
                                                      else if (!argument.compare("VibratoWTremBar")) {
                                                            if (!currentProperty1.firstChild().toElement().text().compare("Slight"))
                                                                  addVibrato(note, Vibrato::Type::VIBRATO_SAWTOOTH);
                                                            else
                                                                  addVibrato(note, Vibrato::Type::VIBRATO_SAWTOOTH_WIDE);
                                                            }
                                                      else if (!argument.compare("BarreFret")) {
                                                            // target can be anywhere from 1 to 36
                                                            int target = currentProperty1.firstChild().toElement().text().toInt();
                                                            for (int i = 0; i < (target / 10); i++)
                                                                  barreFret += "X";
                                                            int targetMod10 = target % 10;
                                                            if (targetMod10 == 9)
                                                                  barreFret += "IX";
                                                            else if (targetMod10 == 4)
                                                                  barreFret += "IV";
                                                            else {
                                                                  if (targetMod10 >= 5) {
                                                                        barreFret   += "V";
                                                                        targetMod10 -= 5;
                                                                        }
                                                                  for (int j = 0; j < targetMod10; j++)
                                                                        barreFret += "I";
                                                                  }
                                                            }
                                                      else if (!argument.compare("BarreString"))
                                                            halfBarre = true;
                                                      else if (!argument.compare("WhammyBarOriginValue"))
                                                            whammyOrigin = currentProperty1.firstChild().toElement().text().toInt();
                                                      else if (!argument.compare("WhammyBarMiddleValue"))
                                                            whammyMiddle = currentProperty1.firstChild().toElement().text().toInt();
                                                      else if (!argument.compare("WhammyBarDestinationValue"))
                                                            whammyEnd = currentProperty1.firstChild().toElement().text().toInt();
                                                      currentProperty1 = currentProperty1.nextSibling();
                                                      }

                                                if (whammyOrigin != -1) {
                                                      // a whammy bar has been detected
                                                      addTremoloBar(segment, track, whammyOrigin, whammyMiddle, whammyEnd);
                                                      }

                                                // if a barre fret has been specified
                                                if (barreFret.compare("") && lastChord != note->chord()) {
                                                      lastChord = note->chord();
                                                      if (halfBarre)
                                                            addTextToNote("1/2B " + barreFret, Align::CENTER, note);
                                                      else
                                                            addTextToNote("B " + barreFret, Align::CENTER, note);
                                                      }
                                                }
                                          QDomNode dynamicsNode = currentNode.parentNode().firstChildElement("Dynamic");
                                          if (!dynamicsNode.isNull()) {
                                                QString dynamicStr = dynamicsNode.toElement().text();
                                                int dynamic        = 0;
                                                if (!dynamicStr.compare("PPP"))
                                                      dynamic = 1;
                                                else if (!dynamicStr.compare("PP"))
                                                      dynamic = 2;
                                                else if (!dynamicStr.compare("P"))
                                                      dynamic = 3;
                                                else if (!dynamicStr.compare("MP"))
                                                      dynamic = 4;
                                                else if (!dynamicStr.compare("MF"))
                                                      dynamic = 5;
                                                else if (!dynamicStr.compare("F"))
                                                      dynamic = 6;
                                                else if (!dynamicStr.compare("FF"))
                                                      dynamic = 7;
                                                else if (!dynamicStr.compare("FFF"))
                                                      dynamic = 8;
                                                if (previousDynamic[track] != dynamic) {
                                                      previousDynamic[track] = dynamic;
                                                      addDynamic(note, dynamic);
                                                      }
                                                }
                                          /* while left and right fingering nodes have distinct values, they are represented
                                           * the same way w.r.t. identifying digit/char in the score file. */
                                          QDomNode leftFingeringNode  = currentNote.parentNode().firstChildElement("LeftFingering");
                                          QDomNode rightFingeringNode = currentNote.parentNode().firstChildElement("RightFingering");
                                          if (!leftFingeringNode.isNull() || !rightFingeringNode.isNull()) {
                                                QDomNode fingeringNode = leftFingeringNode.isNull() ? rightFingeringNode : leftFingeringNode;
                                                QString finger         = fingeringNode.toElement().text();
                                                Fingering* fi          = new Fingering(score);
                                                if (!leftFingeringNode.isNull()) {
                                                      if (!finger.compare("Open"))
                                                            finger = "O";
                                                      else if (!finger.compare("P"))
                                                            finger = "t";
                                                      else if (!finger.compare("I"))
                                                            finger = "1";
                                                      else if (!finger.compare("M"))
                                                            finger = "2";
                                                      else if (!finger.compare("A"))
                                                            finger = "3";
                                                      else if (!finger.compare("C"))
                                                            finger = "4";
                                                      }
                                                fi->setPlainText(finger);
                                                note->add(fi);
                                                fi->reset();
                                                }
                                          QDomNode arpeggioNode = currentNode.parentNode().firstChildElement("Arpeggio");
                                          if (!arpeggioNode.isNull()) {
                                                QString arpeggioStr = arpeggioNode.toElement().text();
                                                Arpeggio* a         = new Arpeggio(score);
                                                if (!arpeggioStr.compare("Up"))
                                                      a->setArpeggioType(ArpeggioType::DOWN);
                                                else
                                                      a->setArpeggioType(ArpeggioType::NORMAL);
                                                chord->add(a);
                                                }
                                          QDomNode letRingNode = currentNote.parentNode().firstChildElement("LetRing");
                                          if (!letRingNode.isNull())
                                                addLetRing(note);
#if 0
                                          QDomNode timerNode = currentNode.parentNode().firstChildElement("Timer");
                                          if (!timerNode.isNull()) {
                                                int time = timerNode.toElement().text().toInt();
                                                TextStyle textStyle;
                                                textStyle.setAlign(Align::CENTER);
                                                textStyle.setUnderline(true);
                                                int minutes = time / 60;
                                                int seconds = time % 60;
                                                addTextToNote(QString::number(minutes) + ":" + (seconds < 10 ? "0" + QString::number(seconds) : QString::number(seconds)), textStyle, note);
                                                }
#endif
                                          QDomNode textNode = currentNode.parentNode().firstChildElement("FreeText");
                                          if (!textNode.isNull()) {
                                                QString text      = textNode.toElement().text();
                                                bool t            = false;
                                                // do not add twice the same text per staff
                                                int strack = staffIdx * VOICES;
                                                int etrack = staffIdx * VOICES + VOICES;
                                                for (const Element* e : segment->annotations()) {
                                                      if (e->type() == ElementType::STAFF_TEXT && e->track() >= strack && e->track() < etrack) {
                                                            const StaffText* st = static_cast<const StaffText*>(e);
                                                            if (!st->xmlText().compare(text)) {
                                                                  t = true;
                                                                  break;
                                                                  }
                                                            }
                                                      }
                                                if (!t && !text.isEmpty()) {
                                                      StaffText* s = new StaffText(score);
                                                      s->setPlainText(text);
                                                      s->setTrack(track);
                                                      segment->add(s);
                                                      }
                                                }
                                          QDomNode ghostNode = currentNote.parentNode().firstChildElement("AntiAccent");
                                          if (!ghostNode.isNull()) {
                                                note->setGhost(true);
                                                /*Symbol* leftSym = new Symbol(note->score());
                                                Symbol* rightSym = new Symbol(note->score());
                                                leftSym->setSym(SymId::noteheadParenthesisLeft);
                                                rightSym->setSym(SymId::noteheadParenthesisRight);
                                                leftSym->setParent(note);
                                                rightSym->setParent(note);
                                                note->add(leftSym);
                                                note->add(rightSym);*/
                                                }
                                          QDomNode swellNode = currentNode.parentNode().firstChildElement("Fadding");
                                          if (!swellNode.isNull()) {
                                                auto str          = swellNode.toElement().text();
                                                Articulation* art = new Articulation(note->score());
                                                if (str == "FadeIn")
                                                      art->setSymId(SymId::guitarFadeIn);
                                                else if (str == "FadeOut")
                                                      art->setSymId(SymId::guitarFadeOut);
                                                else if (str == "VolumeSwell")
                                                      art->setSymId(SymId::guitarVolumeSwell);
                                                art->setAnchor(ArticulationAnchor::TOP_STAFF);
                                                art->setPropertyFlags(Pid::ARTICULATION_ANCHOR, PropertyFlags::UNSTYLED);
                                                if (!note->score()->addArticulation(note, art))
                                                      delete art;
                                                }
                                          QDomNode noteVibrato = currentNote.parentNode().firstChildElement("Vibrato");
                                          if (!noteVibrato.isNull()) {
                                                if (!noteVibrato.toElement().text().compare("Slight"))
                                                      addVibrato(note, Vibrato::Type::GUITAR_VIBRATO);
                                                else
                                                      addVibrato(note, Vibrato::Type::GUITAR_VIBRATO_WIDE);
                                                }

                                          if (cr && (cr->type() == ElementType::CHORD) && sl > 0)
                                                createSlide(sl, cr, staffIdx);
                                          note->setTpcFromPitch();

                                          /* if the ottava is a continuation (need to end old one), or we don't
                                          * see one in the current note when we are tracking one then end the ottava. */
                                          if (ottavaFound.at(track) == 2 || (ottavaFound.at(track) == 1 && currentNode.parentNode().firstChildElement("Ottavia").isNull())) {
                                                createOttava(false, track, cr, ottavaValue.at(track));
                                                if (ottavaFound.at(track) == 2)
                                                      ottavaFound.at(track) = 1;
                                                else
                                                      ottavaFound.at(track) = 0;
                                                }
                                          if (ottavaFound.at(track)) {
                                                createOttava(true, track, cr, ottavaValue.at(track));
                                                int pitch       = note->pitch();
                                                OttavaType type = ottava.at(track)->ottavaType();
                                                if (type == OttavaType::OTTAVA_8VA)
                                                      note->setPitch( (pitch - 12 > 0) ? pitch - 12 : pitch);
                                                else if (type == OttavaType::OTTAVA_8VB)
                                                      note->setPitch( (pitch + 12 < 127) ? pitch + 12 : pitch);
                                                else if (type == OttavaType::OTTAVA_15MA)
                                                      note->setPitch( (pitch - 24 > 0) ? pitch - 24 : (pitch - 12 > 0 ? pitch - 12 : pitch));
                                                else if (type == OttavaType::OTTAVA_15MB)
                                                      note->setPitch( (pitch + 24 < 127) ? pitch + 24 : ( (pitch + 12 < 127) ? pitch + 12 :  pitch));
                                                }

                                          currentNote = currentNote.nextSibling();
                                          }
                                    else if (!currentNote.nodeName().compare("Trill")) {
                                          trill = true;
                                          }
                                    currentNote = currentNote.nextSibling();
                                    }

                              if (graceNote) {
                                    QDomNode graceNode = currentNode.parentNode().firstChildElement("GraceNotes");
                                    if (!graceNode.isNull()) {
                                          lyrNote->setTpcFromPitch();
                                          auto chord = lyrNote->chord();
                                          // before beat grace notes have to be handled after the Tpc is set from pitch
                                          if (!graceNode.toElement().text().compare("OnBeat")) {
                                                auto gNote = score->setGraceNote(chord, lyrNote->pitch(), NoteType::GRACE4, MScore::division / 2);
                                                auto iter1  = slideMap.end();
                                                for (auto beg = slideMap.begin(); beg != slideMap.end(); ++beg) {
                                                      if (beg->second == lyrNote) {
                                                            iter1 = beg;
                                                            break;
                                                            }
                                                      }
                                                if (iter1 != slideMap.end()) {
                                                      iter1->second = gNote;
                                                      createSlur(true, staffIdx, gNote->chord());
                                                      }
                                                if (lyrNote->chord()->notes().size() > 1) {
                                                      lyrNote->chord()->remove(lyrNote);
                                                      delete lyrNote;
                                                      lyrNote = nullptr;
                                                      }
                                                }
                                          else if (!graceNode.toElement().text().compare("BeforeBeat") && chord->type() == ElementType::CHORD) {
                                                auto gNote = score->setGraceNote(chord, lyrNote->pitch(), NoteType::ACCIACCATURA, MScore::division / 2);
                                                auto iter1  = slideMap.end();
                                                for (auto beg = slideMap.begin(); beg != slideMap.end(); ++beg) {
                                                      if (beg->second == lyrNote) {
                                                            iter1 = beg;
                                                            break;
                                                            }
                                                      }
                                                if (iter1 != slideMap.end()) {
                                                      iter1->second = gNote;
                                                      //slideMap.erase(iter);
                                                      //slideMap.insert({ { lyrNote->string(), lyrNote->staffIdx() }, gNote });
                                                      }
                                                //lyrNote->chord()->remove(lyrNote);
                                                //delete lyrNote;
                                                lyrNote = nullptr;
                                                }
                                          }
                                    }
                              continue;
                              }
                        }
                  else if (currentNode.nodeName() == "Dynamic") {}
                  else if (!currentNode.nodeName().compare("Chord")) {
                        int k = currentNode.toElement().text().toInt();
                        if (fretDiagrams[k]) {
                              // TODO: free fretDiagrams
                              segment->add(new FretDiagram(*fretDiagrams[k]));
                              }
                        }
                  else if (currentNode.nodeName() == "Timer") {
                        //int time    = currentNode.toElement().text().toInt();
                        //int minutes = time / 60;
                        //int seconds = time % 60;
                        //addTextToNote(QString::number(minutes) + ":" + (seconds < 10 ? "0" + QString::number(seconds) : QString::number(seconds)), Align::CENTER, note); //TODO
                        }
                  else if (currentNode.nodeName() == "Rhythm") {
                        // we have found a rhythm
                        QString refString     = currentNode.attributes().namedItem("ref").toAttr().value();
                        QDomNode rhythm       = getNode(refString, partInfo->rhythms);
                        QDomNode currentNode1 = (rhythm).firstChild();
                        while (!currentNode1.isNull()) {
                              if (currentNode1.nodeName() == "NoteValue") {
                                    l = rhythmToDuration(currentNode1.toElement().text());
                                    }
                              else if (currentNode1.nodeName() == "AugmentationDot") {
                                    dotted = currentNode1.attributes().namedItem("count").toAttr().value().toInt();
                                    Fraction tmp = l;
                                    for (int count = 1; count <= dotted; count++)
                                          l = l + (tmp / Fraction(pow(2, count),1));
                                    }
                              else if (currentNode1.nodeName() == "PrimaryTuplet") {
                                    tupletSet = true;
                                    cr        = new Chord(score);
                                    cr->setParent(segment);
                                    cr->setTrack(track);
                                    if ((tuplet == 0) || (tuplet->elementsDuration() == tuplet->baseLen().fraction() * tuplet->ratio().numerator())) {
                                          tuplet                           = new Tuplet(score);
                                          tuplet->setTick(currentTick);
                                          tuplets[staffIdx * VOICES + voiceNum] = tuplet;
                                          tuplet->setParent(measure);
                                          }
                                    tuplet->setTrack(cr->track());
                                    tuplet->setBaseLen(l);
                                    tuplet->setRatio(Fraction(currentNode1.attributes().namedItem("num").toAttr().value().toInt(),currentNode1.attributes().namedItem("den").toAttr().value().toInt()));
                                    setupTupletStyle(tuplet);
                                    tuplet->setTicks(l * tuplet->ratio().denominator());
                                    tuplet->add(cr);
                                    }
                              else
                                    qDebug() << "WARNING: Not handling node: " << currentNode1.nodeName();
                              currentNode1 = currentNode1.nextSibling();
                              }
                        fermataIndex += l;
                        }
                  else if (currentNode.nodeName() == "Legato") {
                        QString origin = currentNode.attributes().namedItem("origin").toAttr().value();
                        QString destination = currentNode.attributes().namedItem("destination").toAttr().value();
                        if (!destination.compare("false") && !origin.compare("true")) {
                              qDebug() << "origin";
                              startSlur = true;
                              }
                        else if (!destination.compare("true") && !origin.compare("false")) {
                              qDebug() << "destination";
                              endSlur = true;
                              }
                        }
                  else if (currentNode.nodeName() == "Hairpin") {
                        Segment* seg = segment->prev1(SegmentType::ChordRest);
                        bool isCrec  = !currentNode.toElement().text().compare("Crescendo");
                        if (seg && hairpins[staffIdx]) {
                              if (hairpins[staffIdx]->tick2() == seg->tick() &&
                                 ((isCrec && hairpins[staffIdx]->hairpinType() == HairpinType::CRESC_HAIRPIN) ||
                                 (!isCrec && hairpins[staffIdx]->hairpinType() == HairpinType::DECRESC_HAIRPIN)))
                                    hairpins[staffIdx]->setTick2(currentTick);
                              else
                                    createCrecDim(staffIdx, track, currentTick, isCrec);
                              }
                        else
                              createCrecDim(staffIdx, track, currentTick, isCrec);
                        }
                  else if (!currentNode.nodeName().compare("Properties")) {
                        QDomNode currentProperty = currentNode.firstChild();
                        while (!currentProperty.isNull()) {
                              QString argument = currentProperty.attributes().namedItem("name").toAttr().value();
                              if (!argument.compare("Rasgueado")) {
                                    ChordRest* cr1 = segment->cr(track);
                                    if (cr1 && cr1->isChord()) {
                                          Chord* c = toChord(cr1);
                                          addTextToNote("rasg.", Align::LEFT, c->upNote());
                                          }
#if 0
                                    StaffText* st = new StaffText(score);
                                    st->setTextStyleType(TextStyleType::STAFF);
                                    st->setXmlText("rasg.");
                                    st->setParent(segment);
                                    st->setTrack(track);
                                    score->addElement(st);
#endif
                                    }
                              currentProperty = currentProperty.nextSibling();
                              }
                        }
                  else if (!currentNode.nodeName().compare("Ottavia")) {
                        /* if we saw an ottava and have an updated
                        * information string, set to 2 indicating that. */
                        if (ottavaFound.at(track) == 1 && ottavaValue.at(track).compare(currentNode.toElement().text()))
                              ottavaFound.at(track) = 2;
                        else
                              ottavaFound.at(track) = 1;
                        ottavaValue.at(track) = currentNode.toElement().text();
                        }
                  else if (currentNode.nodeName() == "Lyrics") {
                        auto lyrNode = currentNode.firstChildElement("Line");
                        auto str     = lyrNode.toElement().text();
                        //if (lyrNote) lyrNote->setLyric(str);
                        auto lyr = new Lyrics(score);
                        lyr->setPlainText(str);
                        if (cr)
                              cr->add(lyr);
                        else
                              lyric = lyr;
                        }
                  else if (currentNode.nodeName() == "FreeText") {
                        //TODO::
#if 0
                        auto str = currentNode.toElement().text();
                        auto lyr = new Lyrics(score);
                        lyr->setPlainText(str);
                        cr->add(lyr);
                        if (lyrNote)
                              lyrNote->setLyric(str);
                        else {
                              auto nt = beat.firstChildElement("Notes");
                              if (!nt.isNull()) {
                                    auto notelist = nt.toElement().text().split(' ');
                                    lyrics[notelist.first().toInt()] = str;
                                    }
                              }
#endif
                        }
                  currentNode = currentNode.nextSibling();
                  }
            dotted = 0;
            if (graceNote)
                  continue;
            // we have handled the beat - was there a note?
            if (!noteSpecified) {
                  // add a rest with length of l
                  // we already have a chord, delete it first
                  ChordRest* prevCr = cr; // added in "Rhythm"
                  cr = new Rest(score);
                  if (track % VOICES != 0)
                        wrong_pause = true;
                  cr->setTrack(track);
                  if (tupletSet) {
                        tuplet->remove(prevCr);
                        delete prevCr;
                        tuplet->add(cr);
                        }
                  cr->setTicks(l);
                  if (cr->type() == ElementType::REST && l >= measure->ticks()) {
                        cr->setDurationType(TDuration::DurationType::V_MEASURE);
                        cr->setTicks(measure->ticks());
                        }
                  else {
                        TDuration d(l);
                        cr->setDurationType(d);
                        }
                  if (!segment->cr(track))
                        segment->add(cr);
                  }
            auto fermataList = fermatas.find(measureCounter);
            if (fermataList != fermatas.end()) {
                  // iterator is a list of GPFermata values
                  //for (auto fermataIter = (*fermataList)->begin(); fermataIter != (*fermataList)->end(); fermataIter++) {

                  auto fermataIter     = (*fermataList)->begin();
                  Fraction targetIndex = fermataToFraction((*fermataIter).index, ((*fermataIter).timeDivision));
                  if (fermataIndex == targetIndex) {
                        Articulation* art = new Articulation(score);
                        art->setSymId(SymId::fermataAbove);
                        if (fermataIter->type == "Long")
                              art->setSymId(SymId::fermataLongAbove);
                        art->setUp(true);
                        art->setAnchor(ArticulationAnchor::TOP_STAFF);
                        art->setPropertyFlags(Pid::ARTICULATION_ANCHOR, PropertyFlags::UNSTYLED);
                        auto seg = cr->segment()->prev1(SegmentType::ChordRest);
                        if (seg && seg->cr(track)->isChord()) {
                              seg->cr(track)->add(art);
                              }
                        else
                              cr->add(art);
                        }
                  //}
                  }
            beatsTick += cr->actualTicks();
            }

      if (wrong_pause) {
            _lastTick = beatsTick;
            return Fraction(-1,1);
            }
      return beatsTick;
      }

//---------------------------------------------------------
//   readBars
//---------------------------------------------------------

void GuitarPro6::readBars(QDomNode* barList, Measure* measure, ClefType oldClefId[], GPPartInfo* partInfo, int measureCounter)
      {
      // unique bar identifiers are represented as a space separated string of numbers
      QStringList barsString = barList->toElement().text().split(" ");
      int staffIdx           = 0;

      // used to keep track of tuplets
      std::vector<Tuplet*> tuplets(staves * VOICES);
      for (int track = 0; track < staves * VOICES; ++track)
            tuplets[track] = 0;

      // iterate through all the bars that have been specified
      for (auto iter = barsString.begin(); iter != barsString.end(); ++iter) {
            Fraction tick = measure->tick();

            QDomNode barNode     = getNode(*iter, partInfo->bars);
            QDomNode currentNode = (barNode).firstChild();
            QDomNode voice;
            while (!currentNode.isNull()) {
                  voice.clear();
                  // get the clef of the bar and apply
                  if (!currentNode.nodeName().compare("Clef")) {
                        QString clefString = currentNode.toElement().text();
                        QDomNode nextNode  = currentNode.nextSibling();
                        QString clefOctave;
                        if (!nextNode.nodeName().compare("Ottavia"))
                              clefOctave = nextNode.toElement().text();
                        ClefType clefId = ClefType::G;
                        if (!clefString.compare("F4")) {
                              clefId = ClefType::F;
                              if (clefOctave == "8va")
                                    clefId = ClefType::F_8VA;
                              else if (clefOctave == "8vb")
                                    clefId = ClefType::F8_VB;
                              else if (clefOctave == "15ma")
                                    clefId = ClefType::F_15MA;
                              else if (clefOctave == "15mb")
                                    clefId = ClefType::F15_MB;
                              }
                        else if (!clefString.compare("G2")) {
                              clefId = ClefType::G;
                              if (clefOctave == "8va")
                                    clefId = ClefType::G8_VA;
                              else if (clefOctave == "8vb")
                                    clefId = ClefType::G8_VB;
                              else if (clefOctave == "15ma")
                                    clefId = ClefType::G15_MA;
                              else if (clefOctave == "15mb")
                                    clefId = ClefType::G15_MB;
                              }
                        else if (!clefString.compare("C3"))
                              clefId = ClefType::C3;
                        else if (!clefString.compare("C4")) {
                              clefId = ClefType::C4;
                              if (!clefString.compare("C4") && clefOctave == "8vb")
                                  clefId = ClefType::C4_8VB;
                              }
                        else if (!clefString.compare("Neutral"))
                              clefId = ClefType::PERC;
                        else
                              qDebug() << "WARNING: unhandled clef type: " << clefString;
                        Clef* newClef = new Clef(score);
                        newClef->setClefType(clefId);
                        newClef->setTrack(staffIdx * VOICES);
                        // only add the clef to the bar if it differs from previous measure
                        if (measure->prevMeasure()) {
                              if (clefId != oldClefId[staffIdx]) {
                                    Segment* segment = measure->getSegment(SegmentType::Clef, tick);
                                    segment->add(newClef);
                                    oldClefId[staffIdx] = clefId;
                                    }
                              else
                                    delete newClef;
                              }
                        else {
                              Segment* segment = measure->getSegment(SegmentType::HeaderClef, Fraction(0,1));
                              segment->add(newClef);
                              oldClefId[staffIdx] = clefId;
                              }
                        }
                  // a repeated bar (simile marking)
                  else if (!currentNode.nodeName().compare("SimileMark")) {
                        if (!currentNode.toElement().text().compare("Simple") ||
                           !currentNode.toElement().text().compare("FirstOfDouble") ||
                           !currentNode.toElement().text().compare("SecondOfDouble")) {
                              RepeatMeasure* rm = new RepeatMeasure(score);
                              rm->setTrack(staffIdx * VOICES);
                              rm->setTicks(measure->ticks());
                              rm->setDurationType(TDuration::DurationType::V_MEASURE);
                              Segment* segment = measure->getSegment(SegmentType::ChordRest, tick);
                              segment->add(rm);
                              }
                        else
                              qDebug() << "WARNING: unhandle similie mark type: " << currentNode.toElement().text();
                        }
                  // new voice specification
                  else if (!currentNode.nodeName().compare("Voices")) {
                        QString voicesString = currentNode.toElement().text();
                        auto voices          = voicesString.split(" ");
                        bool contentAdded    = false;
                        int voiceNum         = -1;
                        for (const auto &currentVoice : qAsConst(voices)) {
                              // if the voice is not -1 then we set voice
                              if (currentVoice.compare("-1"))
                                    voice = getNode(currentVoice, partInfo->voices);
                              voiceNum += 1;
                              if (currentVoice.toInt() == -1) {
                                    if (contentAdded) continue;
                                    Fraction l = measure->ticks();
                                    // add a rest with length of l
                                    ChordRest* cr = new Rest(score);
                                    cr->setTrack(staffIdx * VOICES + voiceNum);
                                    cr->setTicks(l);
                                    cr->setDurationType(TDuration::DurationType::V_MEASURE);
                                    Segment* segment = measure->getSegment(SegmentType::ChordRest, tick);
                                    if(!segment->cr(staffIdx * VOICES + voiceNum))
                                          segment->add(cr);
                                    contentAdded = true;
                                    continue;
                                    }
                              // read the beats that occur in the bar
                              Fraction ticks = readBeats(voice.firstChild().toElement().text(), partInfo, measure, tick, staffIdx, voiceNum, &tuplets[0], measureCounter);
                              if (ticks > Fraction(0,1))
                                    contentAdded = true;
                              // deal with possible anacrusis
                              if (ticks < measure->ticks() && voiceNum == 0) {
                                    Fraction mticks = measure->ticks();
                                    Fraction tickOffSet = mticks - ticks;
                                    int track            = staffIdx * VOICES + voiceNum;
                                    score->setRest(ticks + measure->tick(), track, tickOffSet, true, nullptr, true);
                                    }
                              }
                        }
                  else if (!currentNode.nodeName().compare("XProperties")) {}
                  // go to the next node in the tree
                  currentNode = currentNode.nextSibling();
                  }
            // increment the counter for parts
            staffIdx++;
            }
      }

//---------------------------------------------------------
//   checkForHeld
//---------------------------------------------------------

bool checkForHold(Segment* segment, QList<PitchValue> points)
      {
      bool same        = false;
      Segment* prevSeg = segment->prev1(SegmentType::ChordRest);
      if (!prevSeg)
            return false;
      foreach (Element* e, prevSeg->annotations()) {
            if (e->type() == ElementType::TREMOLOBAR) {
                  QList<PitchValue> prevPoints = ((TremoloBar*)e)->points();
                  if (prevPoints.length() != points.length())
                        break;

                  auto iter = points.begin();
                  for (auto prevIter = prevPoints.begin(); prevIter != prevPoints.end(); ++prevIter) {
                        if (*prevIter == *iter)
                              same = true;
                        else {
                              same = false;
                              break;
                              }
                        ++iter;
                        }
                  }
            }
      return same;
      }

//---------------------------------------------------------
//   addTremoloBar
//---------------------------------------------------------

void GuitarPro6::addTremoloBar(Segment* segment, int track, int whammyOrigin, int whammyMiddle, int whammyEnd)
      {
      if (whammyOrigin == 0 && whammyMiddle == 0 && whammyEnd == 0)
            return;
      if ((whammyOrigin == whammyEnd) && (whammyOrigin != whammyMiddle) && whammyMiddle != -1) {
            /* we are dealing with a dip. We need the check for whammy middle
             * to be set as a predive has the same characteristics. */
            QList<PitchValue> points;
            points.append(PitchValue(0, whammyOrigin, false));
            points.append(PitchValue(50, whammyMiddle, false));
            points.append(PitchValue(100, whammyEnd, false));
            TremoloBar* b = new TremoloBar(score);
            b->setPoints(points);
            b->setTrack(track);
            segment->add(b);
            }
      else if (whammyOrigin == 0) {
            // we're dealing with a dive that does not continue from a previous marking
            QList<PitchValue> points;
            points.append(PitchValue(0, whammyOrigin, false));
            points.append(PitchValue(50, whammyMiddle, false));
            points.append(PitchValue(100, whammyEnd, false));
            TremoloBar* b = new TremoloBar(score);
            b->setPoints(points);
            b->setTrack(track);
            segment->add(b);
            }
      else if (whammyOrigin == whammyEnd && whammyMiddle == -1) {
            // this deals with a pre-dive
            QList<PitchValue> points;
            points.append(PitchValue(0, 0, false));
            points.append(PitchValue(50, whammyOrigin, false));
            points.append(PitchValue(100, whammyEnd, false));
            TremoloBar* b = new TremoloBar(score);
            b->setPoints(points);
            b->setTrack(track);
            segment->add(b);
            }
      else if (whammyMiddle != -1) {
            // dive starting from pre-existing point
            Segment* prevSeg = segment->prev1(SegmentType::ChordRest);
            if (!prevSeg)
                  return;
            foreach (Element* e, prevSeg->annotations()) {
                  if (e->type() == ElementType::TREMOLOBAR) {
                        QList<PitchValue> prevPoints = ((TremoloBar*)e)->points();
                        QList<PitchValue> points;
                        points.append(PitchValue(0, prevPoints[prevPoints.length() - 1].pitch, false));
                        points.append(PitchValue(50, whammyOrigin, false));
                        points.append(PitchValue(100, whammyEnd, false));
                        TremoloBar* b = new TremoloBar(score);
                        b->setPoints(points);
                        b->setTrack(track);
                        segment->add(b);
                        }
                  }
            }
      else {
            // a predive/dive
            QList<PitchValue> points;
            points.append(PitchValue(0, 0, false));
            points.append(PitchValue(50, whammyOrigin, false));
            points.append(PitchValue(100, whammyEnd, false));

            if (checkForHold(segment, points)) {
                  points.clear();
                  points.append(PitchValue(0, whammyEnd, false));
                  points.append(PitchValue(100, whammyEnd, false));
                  }

            TremoloBar* b = new TremoloBar(score);
            b->setPoints(points);
            b->setTrack(track);
            segment->add(b);
            }
      }

//---------------------------------------------------------
//   readMasterBars
//---------------------------------------------------------

void GuitarPro6::readMasterBars(GPPartInfo* partInfo)
      {
      Measure* measure       = score->firstMeasure();
      int bar                = 0;
      QDomNode nextMasterBar = partInfo->masterBars;
      nextMasterBar = nextMasterBar.nextSibling();
      int measureCounter = 0;
      //int last_counter   = -1, last_counter2 = -1, max_counter = -1;
      std::vector<ClefType> oldClefId(staves);
      //ClefType oldClefId[staves];
      hairpins = new Hairpin*[staves];
      for (int i = 0; i < staves; i++)
            hairpins[i] = 0;
      do    {
            const GpBar& gpbar = bars[bar];

            if (!gpbar.marker.isEmpty()) {
                  RehearsalMark* s = new RehearsalMark(score);
                  s->setPlainText(gpbar.marker.trimmed());
                  s->setTrack(0);
                  Segment* segment = measure->getSegment(SegmentType::ChordRest, measure->tick());
                  segment->add(s);
                  }

            QDomNode masterBarElementTemplate = nextMasterBar.firstChild();
            for (int stave = 0; stave < staves; stave++) {
                  QDomNode masterBarElement = masterBarElementTemplate;
                  bool first                = true;
                  while (!masterBarElement.isNull()) {
                        if (first) {
                              if (bars[measureCounter].freeTime /*&& last_counter != measureCounter*/) {
                                    //last_counter = measureCounter;
                                    bool previousFreeTime = (measureCounter > 0 && bars[measureCounter - 1].freeTime);
                                    bool sameTimeSig = measureCounter > 0 && (bars[measureCounter - 1].timesig == bars[measureCounter].timesig);
                                    if (!sameTimeSig) {
                                          TimeSig* ts = new TimeSig(score);
                                          ts->setSig(bars[measureCounter].timesig);
                                          ts->setTrack(stave);
                                          Measure* m = score->getCreateMeasure(measure->tick());
                                          Segment* s = m->getSegment(SegmentType::TimeSig, measure->tick());
                                          ts->setLargeParentheses(true);
                                          s->add(ts);
                                          // no text for two consecutive freetime timesig
                                          if (!previousFreeTime) {
                                                StaffText* st = new StaffText(score);
                                                st->setXmlText("Free time");
                                                s = m->getSegment(SegmentType::ChordRest, measure->tick());
                                                st->setParent(s);
                                                st->setTrack(stave);
                                                score->addElement(st);
                                                }
                                          }
                                    }
                              else if (measureCounter > 0 && bars[measureCounter - 1].freeTime) {
                                    TimeSig* ts = new TimeSig(score);
                                    ts->setSig(bars[measureCounter].timesig);
                                    ts->setTrack(stave);
                                    Measure* m = score->getCreateMeasure(measure->tick());
                                    Segment* s = m->getSegment(SegmentType::TimeSig, measure->tick());
                                    ts->setLargeParentheses(false);
                                    s->add(ts);
                                    }
                              else
                                    measure->setTimesig(bars[measureCounter].timesig);
                              measure->setTicks(bars[measureCounter].timesig);
                              }

                        if (!bars[measureCounter].direction.compare("Fine") || (bars[measureCounter].direction.compare("") && !bars[measureCounter].directionStyle.compare("Jump"))) {
                              Segment* s    = measure->getSegment(SegmentType::KeySig, measure->tick());
                              StaffText* st = new StaffText(score);
                              if (!bars[measureCounter].direction.compare("Fine"))
                                    st->setXmlText("fine");
                              else if (!bars[measureCounter].direction.compare("DaCapo"))
                                    st->setXmlText("Da Capo");
                              else if (!bars[measureCounter].direction.compare("DaCapoAlCoda"))
                                    st->setXmlText("D.C. al Coda");
                              else if (!bars[measureCounter].direction.compare("DaCapoAlDoubleCoda"))
                                    st->setXmlText("D.C. al Double Coda");
                              else if (!bars[measureCounter].direction.compare("DaCapoAlFine"))
                                    st->setXmlText("D.C. al Fine");
                              else if (!bars[measureCounter].direction.compare("DaSegnoAlCoda"))
                                    st->setXmlText("D.S. al Coda");
                              else if (!bars[measureCounter].direction.compare("DaSegno"))
                                    st->setXmlText("Da Segno");
                              else if (!bars[measureCounter].direction.compare("DaSegnoAlDoubleCoda"))
                                    st->setXmlText("D.S. al Double Coda");
                              else if (!bars[measureCounter].direction.compare("DaSegnoAlFine"))
                                    st->setXmlText("D.S. al Fine");
                              else if (!bars[measureCounter].direction.compare("DaSegnoSegno"))
                                    st->setXmlText("Da Segno Segno");
                              else if (!bars[measureCounter].direction.compare("DaSegnoSegnoAlCoda"))
                                    st->setXmlText("D.S.S. al Coda");
                              else if (!bars[measureCounter].direction.compare("DaSegnoSegnoAlDoubleCoda"))
                                    st->setXmlText("D.S.S. al Double Coda");
                              else if (!bars[measureCounter].direction.compare("DaSegnoSegnoAlFine"))
                                    st->setXmlText("D.S.S. al Fine");
                              else if (!bars[measureCounter].direction.compare("DaCoda"))
                                    st->setXmlText("Da Coda");
                              else if (!bars[measureCounter].direction.compare("DaDoubleCoda"))
                                    st->setXmlText("Da Double Coda");
                              st->setParent(s);
                              st->setTrack(stave);
                              score->addElement(st);
                              bars[measureCounter].direction = "";
                              }
                        else if (bars[measureCounter].direction.compare("") && !bars[measureCounter].directionStyle.compare("Target")) {
                              Segment* s  = measure->getSegment(SegmentType::BarLine, measure->tick());
                              Symbol* sym = new Symbol(score);
                              if (!bars[measureCounter].direction.compare("Segno"))
                                    sym->setSym(SymId::segno);
                              else if (!bars[measureCounter].direction.compare("SegnoSegno")) {
                                    sym->setSym(SymId::segno);
                                    Segment* s2  = measure->getSegment(SegmentType::ChordRest, measure->tick());
                                    Symbol* sym2 = new Symbol(score);
                                    sym2->setSym(SymId::segno);
                                    sym2->setParent(measure);
                                    sym2->setTrack(stave);
                                    s2->add(sym2);
                                    }
                              else if (!bars[measureCounter].direction.compare("Coda"))
                                    sym->setSym(SymId::coda);
                              else if (!bars[measureCounter].direction.compare("DoubleCoda")) {
                                    sym->setSym(SymId::coda);
                                    Segment* s2  = measure->getSegment(SegmentType::ChordRest, measure->tick());
                                    Symbol* sym2 = new Symbol(score);
                                    sym2->setSym(SymId::coda);
                                    sym2->setParent(measure);
                                    sym2->setTrack(stave);
                                    s2->add(sym2);
                                    }
                              sym->setParent(measure);
                              sym->setTrack(stave);
                              s->add(sym);
                              bars[measureCounter].direction = "";
                              }
#if 0
                        if (first && measureCounter > max_counter) {
                              max_counter = measureCounter;
                              first       = false;
                              int counter = -1;
                              for (auto& dir : bars[measureCounter].directions) {
                                    ++counter;
                                    if (!dir.compare("Fine") || !bars[measureCounter].directionStyle.compare("Jump")) {
                                          Segment* s    = measure->getSegment(SegmentType::KeySig, measure->tick());
                                          StaffText* st = new StaffText(score);
                                          if (!dir.compare("Fine"))
                                                st->setXmlText("fine");
                                          else if (!dir.compare("DaCapo"))
                                                st->setXmlText("Da Capo");
                                          else if (!dir.compare("DaCapoAlCoda"))
                                                st->setXmlText("D.C. al Coda");
                                          else if (!dir.compare("DaCapoAlDoubleCoda"))
                                                st->setXmlText("D.C. al Double Coda");
                                          else if (!dir.compare("DaCapoAlFine"))
                                                st->setXmlText("D.C. al Fine");
                                          else if (!dir.compare("DaSegnoAlCoda"))
                                                st->setXmlText("D.S. al Coda");
                                          else if (!dir.compare("DaSegno"))
                                                st->setXmlText("Da Segno");
                                          else if (!dir.compare("DaSegnoAlDoubleCoda"))
                                                st->setXmlText("D.S. al Double Coda");
                                          else if (!dir.compare("DaSegnoAlFine"))
                                                st->setXmlText("D.S. al Fine");
                                          else if (!dir.compare("DaSegnoSegno"))
                                                st->setXmlText("Da Segno Segno");
                                          else if (!dir.compare("DaSegnoSegnoAlCoda"))
                                                st->setXmlText("D.S.S. al Coda");
                                          else if (!dir.compare("DaSegnoSegnoAlDoubleCoda"))
                                                st->setXmlText("D.S.S. al Double Coda");
                                          else if (!dir.compare("DaSegnoSegnoAlFine"))
                                                st->setXmlText("D.S.S. al Fine");
                                          else if (!dir.compare("DaCoda"))
                                                st->setXmlText("Da Coda");
                                          else if (!dir.compare("DaDoubleCoda"))
                                                st->setXmlText("Da Double Coda");
                                          st->setParent(s);
                                          st->setTrack(stave);
                                          score->addElement(st);
                                          //bars[measureCounter].direction = "";
                                          }
                                    else if (!bars[measureCounter].directionStyle.compare("Target")) {
                                          Segment* s  = measure->getSegment(SegmentType::BarLine, measure->tick());
                                          Symbol* sym = new Symbol(score);
                                          if (!dir.compare("Segno"))
                                                sym->setSym(SymId::segno);
                                          else if (!dir.compare("SegnoSegno")) {
                                                sym->setSym(SymId::segnoSerpent2);
                                                /* Segment* s2 = measure->getSegment(SegmentType::ChordRest, measure->tick());
                                                 Symbol* sym2 = new Symbol(score);
                                                 sym2->setSym(SymId::segno);
                                                 sym2->setParent(measure);
                                                 sym2->setTrack(stave);
                                                 sym2->setXoffset(5.5f);
                                                 sym2->setElYOffset(-7.0f * counter);
                                                 s2->add(sym2);*/
                                                }
                                          else if (!dir.compare("Coda"))
                                                sym->setSym(SymId::coda);
                                          else if (!dir.compare("DoubleCoda")) {
                                                sym->setSym(SymId::codaSquare);
                                                /*  Segment* s2 = measure->getSegment(SegmentType::ChordRest, measure->tick());
                                                  Symbol* sym2 = new Symbol(score);
                                                  sym2->setSym(SymId::coda);
                                                  sym2->setParent(measure);
                                                  sym2->setTrack(stave);
                                                  sym2->setXoffset(8.0f);
                                                  sym2->setElYOffset(-7.0f * counter);
                                                  s2->add(sym2);*/
                                                }
                                          sym->setParent(measure);
                                          sym->setTrack(stave);
                                          s->add(sym);
                                          bars[measureCounter].direction = "";
                                          }
                                    }
                              }
#endif
                        // we no longer set the key here, the gpbar has the information stored in it
                        if (!masterBarElement.nodeName().compare("Fermatas")) {
                              QDomNode currentFermata = masterBarElement.firstChild();
                              while (!currentFermata.isNull()) {
                                    QString fermata = currentFermata.lastChildElement("Offset").toElement().text();
                                    auto type       = currentFermata.lastChildElement("Type");
                                    currentFermata = currentFermata.nextSibling();

                                    // get the fermata information and construct a gpFermata from them
                                    QStringList fermataComponents = fermata.split("/", QString::SkipEmptyParts);
                                    GPFermata gpFermata;
                                    gpFermata.index        = fermataComponents.at(0).toInt();
                                    gpFermata.timeDivision = fermataComponents.at(1).toInt();

                                    if (!type.isNull())
                                          gpFermata.type = type.toElement().text();
                                    else
                                          gpFermata.type = "Medium";

                                    if (fermatas.contains(measureCounter)) {
                                          QList<GPFermata>* fermataList = fermatas.value(measureCounter);
                                          fermataList->push_back(gpFermata);
                                          }
                                    else {
                                          QList<GPFermata>* fermataList = new QList<GPFermata>;
                                          fermataList->push_back(gpFermata);
                                          fermatas.insert(measureCounter, fermataList);
                                          }
                                    }
                              }
                        else if (!masterBarElement.nodeName().compare("Repeat")) {
                              bool start = !masterBarElement.attributes().namedItem("start").toAttr().value().compare("true");
                              int count  = masterBarElement.attributes().namedItem("count").toAttr().value().toInt();
                              if (start)
                                    measure->setRepeatStart(true);
                              else
                                    measure->setRepeatEnd(true);
                              measure->setRepeatCount(count);
                              }
                        else if (!masterBarElement.nodeName().compare("AlternateEndings") /*&& measureCounter != last_counter2*/) {
                              //last_counter2 = measureCounter;
                              QString endNumbers = masterBarElement.toElement().text().replace(" ", ",");
                              bool create        = true;
                              if (_lastVolta) {
                                    auto prevm = measure->prevMeasure();
                                    if (prevm->endBarLineType() != BarLineType::START_REPEAT && (_lastVolta->tick2() == prevm->tick() + prevm->ticks() )
                                       &&(_lastVolta->text() == endNumbers) ) {
                                          create = false;
                                          _lastVolta->setTick2(measure->tick() + measure->ticks());
                                          }
                                    }
                              if (create) {
                                    Ms::Volta* volta = new Ms::Volta(score);
                                    volta->endings().clear();
                                    volta->setText(endNumbers);
                                    volta->setTick(measure->tick());
                                    volta->setTick2(measure->tick() + measure->ticks());

                                    const char* c = endNumbers.toUtf8().constData();
                                    while (c && *c)
                                          {
                                          if (*c >= '0' && *c <= '9')
                                                volta->endings().push_back(int(*c - '0'));
                                          ++c;
                                          }

                                    _lastVolta = volta;
                                    score->addElement(volta);
                                    }
                              }
                        else if (!masterBarElement.nodeName().compare("Bars") && stave == staves - 1) {
                              readBars(&masterBarElement, measure, &oldClefId[0], partInfo, measureCounter);
                              for (int i = 0; i < staves * VOICES; ++i) {
                                    Ottava* o = ottava.at(i);
                                    if (o && o->ticks().isZero())
                                          o->setTick2(score->endTick());
                                    Slur* slur = legatos[i];
                                    if (slur) {
                                          if (measure->prevMeasure() && !measure->hasVoice(i)) {
                                                //find last chord in track
                                                Chord* c = nullptr;
                                                for (const Segment* seg = measure->prevMeasure()->last(); seg; seg = seg->prev1()) {
                                                      Element* el = seg->element(i);
                                                      if (el && el->isChord()) {
                                                            c = static_cast<Chord*>(el);
                                                            break;
                                                            }
                                                      }
                                                if (c) {
                                                      slur->setTick2(c->tick());
                                                      score->addElement(slur);
                                                      legatos[slur->track()] = 0;
                                                      }
                                                }
                                          }
                                    }
                              }
                        masterBarElement = masterBarElement.nextSibling();
                        first = false;
                        }
                  }
            if (bars[measureCounter].section[0].length() || bars[measureCounter].section[1].length()) {
                  Segment* s = measure->getSegment(SegmentType::ChordRest, measure->tick());
                  if (bars[measureCounter].section[0].length()) {
                        RehearsalMark* t = new RehearsalMark(score);
                        t->setFrameType(FrameType::SQUARE);
                        t->setPlainText(bars[measureCounter].section[0]);
                        t->setTrack(0);
                        s->add(t);
                        }
                  if (bars[measureCounter].section[1].length()) {
                        RehearsalMark* t = new RehearsalMark(score);
                        t->setFrameType(FrameType::NO_FRAME);
                        t->setPlainText(bars[measureCounter].section[1]);
                        t->setTrack(0);
                        s->add(t);
                        }
                  }
            measureCounter++;
            nextMasterBar = nextMasterBar.nextSibling();
            measure       = measure->nextMeasure();
            bar++;
            } while (!nextMasterBar.isNull());
      }

//---------------------------------------------------------
//   readGpif
//---------------------------------------------------------

void GuitarPro6::readGpif(QByteArray* data)
      {
      // qDebug() << QString(*data);
      QDomDocument qdomDoc;
      qdomDoc.setContent(*data);
      QDomElement qdomElem = qdomDoc.documentElement();
      // GPRevision node
      QDomNode revision = qdomElem.firstChildElement("GPRevision");
      // Score node
      QDomNode scoreNode = qdomElem.firstChildElement("Score");
      readScore(&scoreNode);
      // MasterTrack node
      QDomNode masterTrack = qdomElem.firstChildElement("MasterTrack");
      readMasterTracks(&masterTrack);
      // Tracks node
      QDomNode eachTrack = qdomElem.firstChildElement("Tracks");
      readTracks(&eachTrack);

      // now we know how many staves there are from readTracks, we can initialise slurs (for hammer/pulloff)
       // and legatos
      slurs = new Slur*[staves * VOICES];
      legatos = new Slur*[staves * VOICES];
      ottava.assign(staves * VOICES, 0);
      ottavaFound.assign(staves * VOICES, 0);
      ottavaValue.assign(staves * VOICES, "");
      for (int i = 0; i < staves * VOICES; ++i) {
            slurs[i] = 0;
            legatos[i] = 0;
            }

      // MasterBars node
      GPPartInfo partInfo;
      QDomNode masterBars = eachTrack.nextSibling();
      QDomNode b          = masterBars.nextSibling();
      QDomNode voices     = b.nextSibling();
      QDomNode beats      = voices.nextSibling();
      QDomNode notes      = beats.nextSibling();
      QDomNode rhythms    = notes.nextSibling();

      // set up the partInfo struct to contain information from the file
      partInfo.masterBars = masterBars.firstChild();
      partInfo.bars       = b.firstChild();
      partInfo.voices     = voices.firstChild();
      partInfo.beats      = beats.firstChild();
      partInfo.notes      = notes.firstChild();
      partInfo.rhythms    = rhythms.firstChild();

      measures = findNumMeasures(&partInfo);

      createMeasures();
      fermatas.clear();
      readMasterBars(&partInfo);
      // complete slurs (GP6 sometimes output destination=true even for last beat)
      for (int i = 0; i < staves * VOICES; ++i) {
            Slur* slur = legatos[i];
            if (slur) {
                  //find last chord in track
                  Chord* c = nullptr;
                  for (const Segment* seg = score->lastSegment(); seg; seg = seg->prev1()) {
                        Element* el = seg->element(i);
                        if (el && el->isChord()) {
                              c = static_cast<Chord*>(el);
                              break;
                              }
                        }
                  if (c) {
                        slur->setTick2(c->tick());
                        score->addElement(slur);
                        legatos[slur->track()] = nullptr;
                        }
                  else {
                        legatos[slur->track()] = nullptr;
                        delete slur;
                        }
                  }
            }
      // change the tuning to deal with transposition
      // It's needed to create correct tabs
      for (Part * p : score->parts()) {
            Instrument* instr = p->instrument();
            if (instr->transpose().chromatic == 0)
                  continue;
            const StringData* sd = instr->stringData();
            if (sd) {
#if (!defined (_MSCVER) && !defined (_MSC_VER))
               int tuning[sd->strings()];
#else
               // MSVC does not support VLA. Replace with std::vector. If profiling determines that the
               //    heap allocation is slow, an optimization might be used.
               std::vector<int> vTuning(sd->strings());
               int* tuning = vTuning.data();
#endif
                  int frets   = sd->frets();
                  int strings;
                  for (strings = 0; strings < sd->strings(); strings++) {
                        tuning[strings] = sd->stringList()[strings].pitch - instr->transpose().chromatic;
                        }
                  StringData* stringData = new StringData(frets, strings, tuning);
                  instr->setStringData(*stringData);
                  }
            }

      // set the starting tempo of the score
      bool zero_set = false;
//      int linearTemp = -1;
      for (auto iter = tempoMap.begin(); iter != tempoMap.end(); ++iter) {
            if (iter->first == 0)
                  zero_set = true;
            Measure* measure = toMeasure(score->measure(iter->first));
            if (measure)
                  setTempo(iter->second.first, measure);
#if 0 // TODO-ws   what's linearTemp ?
            if (linearTemp != -1) {
                  auto siter = iter;
                  siter--;
                  auto val = iter->second.first - siter->second.first;
                  if (val != 0) {
                        for (int i = linearTemp; i <= iter->first; ++i) {
                              Measure* ms = toMeasure(score->measure(i));
                              if (ms)
                                    ms->setLinearTemp(val);
                              }
                        }
                  linearTemp = -1;
                  }
            if (iter->second.second)
                  linearTemp = iter->first;
#endif
            }
      if (!zero_set)
            setTempo(120, score->firstMeasure());
      }

//---------------------------------------------------------
//   parseFile
//---------------------------------------------------------

void GuitarPro6::parseFile(const char* filename, QByteArray* data)
      {
      // test to check if we are dealing with the score
      if (!strcmp(filename, "score.gpif"))
            readGpif(data);
      }

//---------------------------------------------------------
//   readGPX
//---------------------------------------------------------

void GuitarPro6::readGPX(QByteArray* buffer)
      {
      // start by reading the file header. It will tell us if the byte array is compressed.
      int fileHeader = readInteger(buffer, 0);

      if (fileHeader == GPX_HEADER_COMPRESSED) {
            // this is  a compressed file.
            int length             = readInteger(buffer, position / BITS_IN_BYTE);
            QByteArray* bcfsBuffer = new QByteArray();
            int positionCounter    = 0;
            while(!f->error() && (position / BITS_IN_BYTE) < length) {
                  // read the bit indicating compression information
                  int flag = readBits(buffer, 1);

                  if (flag) {
                        int bits = readBits(buffer, 4);
                        int offs = readBitsReversed(buffer, bits);
                        int size = readBitsReversed(buffer, bits);

                        QByteArray bcfsBufferCopy = *bcfsBuffer;
                        int pos                   = (bcfsBufferCopy.length() - offs);
                        for( int i = 0; i < (size > offs ? offs : size); i++ ) {
                              bcfsBuffer->insert(positionCounter, bcfsBufferCopy[pos + i] );
                              positionCounter++;
                              }
                        }
                  else {
                        int size = readBitsReversed(buffer, 2);
                        for(int i = 0; i < size; i++) {
                              bcfsBuffer->insert(positionCounter, readBits(buffer, 8));
                              positionCounter++;
                              }
                        }
                  }
            // recurse on the decompressed file stored as a byte array
            readGPX(bcfsBuffer);
            delete bcfsBuffer;
            }
      else if (fileHeader == GPX_HEADER_UNCOMPRESSED) {
            // this is an uncompressed file - strip the header off
            *buffer = buffer->right(buffer->length() - sizeof(int));
            int sectorSize = 0x1000;
            int offset     = 0;
            while ((offset = (offset + sectorSize)) + 3 < buffer->length()) {
                  int newInt = readInteger(buffer,offset);
                  if (newInt == 2) {
                        int indexFileName = (offset + 4);
                        int indexFileSize = (offset + 0x8C);
                        int indexOfBlock  = (offset + 0x94);

                        // create a byte array and put information about files found in it
                        int block             = 0;
                        int blockCount        = 0;
                        QByteArray* fileBytes = new QByteArray();
                        while((block = (readInteger(buffer, (indexOfBlock + (4 * (blockCount++)))))) != 0 ) {
                              fileBytes->push_back(getBytes(buffer, (offset = (block * sectorSize)), sectorSize));
                              }
                        // get file information and read the file
                        int fileSize = readInteger(buffer, indexFileSize);
                        if (fileBytes->length() >= fileSize) {
                              QByteArray filenameBytes = readString(buffer, indexFileName, 127);
                              char* filename           = filenameBytes.data();
                              QByteArray data          = getBytes(fileBytes, 0, fileSize);
                              parseFile(filename, &data);
                              }
                        delete fileBytes;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   readBeatEffects
//---------------------------------------------------------

int GuitarPro6::readBeatEffects(int, Segment*)
      {
      qDebug("reading beat effects (.gpx)...\n");
      return 0;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

bool GuitarPro6::read(QFile* fp)
      {
      f = fp;
      previousTempo = -1;
      QByteArray buffer = fp->readAll();

      // decompress and read files contained within GPX file
      readGPX(&buffer);

      return true;
      }
}
