//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2014 John Pirie
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "importgtp.h"
#include "globals.h"
#include "libmscore/score.h"
#include "libmscore/measurebase.h"
#include "libmscore/text.h"
#include "libmscore/box.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/measure.h"
#include "libmscore/timesig.h"
#include "libmscore/tremolo.h"
#include "libmscore/rest.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/stringdata.h"
#include "libmscore/clef.h"
#include "libmscore/lyrics.h"
#include "libmscore/tempotext.h"
#include "libmscore/slur.h"
#include "libmscore/tie.h"
#include "libmscore/tuplet.h"
#include "libmscore/barline.h"
#include "libmscore/excerpt.h"
#include "libmscore/stafftype.h"
#include "libmscore/bracket.h"
#include "libmscore/articulation.h"
#include "libmscore/keysig.h"
#include "libmscore/harmony.h"
#include "libmscore/bend.h"
#include "libmscore/tremolobar.h"
#include "libmscore/segment.h"
#include "libmscore/rehearsalmark.h"
#include "libmscore/dynamic.h"
#include "libmscore/arpeggio.h"
#include "libmscore/volta.h"
#include "libmscore/instrtemplate.h"
#include "preferences.h"

namespace Ms {

//---------------------------------------------------------
//   readBit
//---------------------------------------------------------

int GuitarPro6::readBit() {
      // calculate the byte index by dividing the position in bits by the bits per byte
      int byteIndex = this->position / BITS_IN_BYTE;
      // calculate our offset so we know how much to bit shift
      int byteOffset = ((BITS_IN_BYTE-1) - (this->position % BITS_IN_BYTE));
      // calculate the bit which we want to read
      int bit = ((((*buffer)[byteIndex] & 0xff) >> byteOffset) & 0x01);
      // increment our curent position so we know this bit has been read
      this->position++;
      // return the bit we calculated
      return bit;
      }


//---------------------------------------------------------
//   readBits
//---------------------------------------------------------

int GuitarPro6::readBits(int bitsToRead) {
      int bits = 0;
      for (int i = (bitsToRead-1); i>=0; i--) {
            bits |= (readBit() << i);
            }
      return bits;
      }

//---------------------------------------------------------
//   readBitsReversed
//---------------------------------------------------------

int GuitarPro6::readBitsReversed(int bitsToRead) {
      int bits = 0;
      for( int i = 0 ; i < bitsToRead ; i ++ ) {
            bits |= (readBit() << i );
            }
      return bits;
      }

//---------------------------------------------------------
//   getBytes
//---------------------------------------------------------

QByteArray GuitarPro6::getBytes(QByteArray* buffer, int offset, int length) {
      QByteArray newBytes;
      // compute new bytes from our buffer and return byte array
      for (int i = 0; i < length; i++) {
            if (buffer->length() > offset + i) {
                  newBytes.insert(i, ((*buffer)[offset+i]));
                  }
            }
      return newBytes;
      }

//---------------------------------------------------------
//   readInteger
//---------------------------------------------------------

int GuitarPro6::readInteger(QByteArray* buffer, int offset) {
      // assign four bytes and take them from the buffer
      char bytes[4];
      bytes[0] = (*buffer)[offset+0];
      bytes[1] = (*buffer)[offset+1];
      bytes[2] = (*buffer)[offset+2];
      bytes[3] = (*buffer)[offset+3];
      // increment positioning so we keep track of where we are
      this->position+=sizeof(int)*BITS_IN_BYTE;
      // bit shift in order to compute our integer value and return
      return ((bytes[3] & 0xff) << 24) | ((bytes[2] & 0xff) << 16) | ((bytes[1] & 0xff) << 8) | (bytes[0] & 0xff);
      }

//---------------------------------------------------------
//   readString
//---------------------------------------------------------

QByteArray GuitarPro6::readString(QByteArray* buffer, int offset, int length) {
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
//   readGPX
//---------------------------------------------------------

void GuitarPro6::readGPX(QByteArray* buffer) {
      // start by reading the file header. It will tell us if the byte array is compressed.
      int fileHeader = readInteger(buffer, 0);

      if (fileHeader == GPX_HEADER_COMPRESSED) {
            // this is  a compressed file.
            int length = readInteger(buffer, this->position/BITS_IN_BYTE);
            QByteArray* bcfsBuffer = new QByteArray();
            int positionCounter = 0;
            while(!f->error() && (this->position/this->BITS_IN_BYTE) < length) {
                  // read the bit indicating compression information
                  int flag = this->readBits(1);

                  if (flag) {
                        int bits = this->readBits(4);
                        int offs = this->readBitsReversed(bits);
                        int size = this->readBitsReversed(bits);

                        QByteArray bcfsBufferCopy = *bcfsBuffer;
                        int pos = (bcfsBufferCopy.length() - offs );
                        for( int i = 0; i < (size > offs ? offs : size) ; i ++ ) {
                              bcfsBuffer->insert(positionCounter, bcfsBufferCopy[pos + i] ) ;
                              positionCounter++;
                              }
                        }
                  else  {
                        int size = this->readBitsReversed(2);
                        for(int i = 0; i < size; i++) {
                              bcfsBuffer->insert(positionCounter, this->readBits(8));
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
            *buffer = buffer->right(buffer->length()-sizeof(int));
            int sectorSize = 0x1000;
            int offset = 0;
            while ((offset = (offset + sectorSize)) + 3 < buffer->length()) {
                  int newInt = readInteger(buffer,offset);
                  if (newInt == 2) {
                        int indexFileName = (offset + 4);
                        int indexFileSize = (offset + 0x8C);
                        int indexOfBlock = (offset + 0x94);

                        // create a byte array and put information about files found in it
                        int block = 0;
                        int blockCount = 0;
                        QByteArray* fileBytes = new QByteArray();
                        while((block = (readInteger(buffer, (indexOfBlock + (4 * (blockCount ++)))))) != 0 ) {
                              fileBytes->push_back(getBytes(buffer, (offset = (block*sectorSize)), sectorSize));
                              }
                        // get file information and read the file
                        int fileSize = readInteger(buffer, indexFileSize);
                        if (fileBytes->length() >= fileSize) {
                              QByteArray filenameBytes = readString(buffer, indexFileName, 127);
                              char* filename = filenameBytes.data();
                              QByteArray data = getBytes(fileBytes, 0, fileSize);
                              parseFile(filename, &data);
                              }
                        delete fileBytes;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   parseFile
//---------------------------------------------------------

void GuitarPro6::parseFile(char* filename, QByteArray* data)
      {
      // test to check if we are dealing with the score
      if (!strcmp(filename, "score.gpif")) {
            readGpif(data);
            QFile file("/tmp/score.gpif");
            file.open(QIODevice::WriteOnly);
            file.write(*data);
            file.close();
            }
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
            else if (!nodeName.compare("Subtitle"))
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
            else
                  unhandledNode(nodeName);
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
                  while (!currentAutomation.isNull()) {
                        if (!currentAutomation.nodeName().compare("Automation")) {
                              if (!currentAutomation.firstChild().nodeName().compare("Tempo"))
                                    tempo = currentAutomation.lastChild().toElement().text().toInt();
                              else
                                    unhandledNode(currentAutomation.nodeName());
                              }
                        else
                              unhandledNode(currentAutomation.nodeName());
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
      // initialise a new fret diagram for our current track
      FretDiagram* fretDiagram = new FretDiagram(score);
      fretDiagram->setTrack(track);

      // get the identifier to set as the domain in the map
      int id = diagram->attributes().namedItem("id").toAttr().value().toInt();
      QDomNode diagramNode = diagram->firstChild();

      // set the number of strings on this part
      int stringCount = diagramNode.attributes().namedItem("stringCount").toAttr().value().toInt();
      fretDiagram->setStrings(stringCount);

      // set the fret offset
      int baseFret = diagramNode.attributes().namedItem("baseFret").toAttr().value().toInt();
      fretDiagram->setOffset(baseFret);

      QDomNode diagramEntity = diagramNode.firstChild();
      int counter = 0;
      while (!diagramEntity.isNull()) {
            QString nodeName = diagramEntity.nodeName();
            // new fret
            if (!nodeName.compare("Fret")) {
                  // get the string and fret numbers from the arguments to the node as integers
                  int string = diagramEntity.attributes().namedItem("string").toAttr().value().toInt();
                  int fret = diagramEntity.attributes().namedItem("fret").toAttr().value().toInt();

                  // if there are unspecified string values, add the X marker to that string
                  while (counter < string) {
                        fretDiagram->setMarker(counter, 'X');
                        counter++;
                        }

                  // look at the specified string/fret and add to diagram
                  if (fret == 0) {
                        fretDiagram->setMarker(string, '0');
                        counter++;
                        }
                  else  {
                        qDebug("Setting fret (string=%d,fret=%d)", string, fret-baseFret);
                        fretDiagram->setDot(string, fret);
                        counter++;
                        }
                  }
            // move to the next string/fret specification
            diagramEntity = diagramEntity.nextSibling();
            }

      // mark any missing strings as 'X'
      while (counter < stringCount) {
            fretDiagram->setMarker(counter, 'X');
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
      int trackCounter = 0;
      while (!nextTrack.isNull()) {
            QDomNode currentNode = nextTrack.firstChild();
            Part* part = new Part(score);
            Staff* s = new Staff(score, part, trackCounter);
            while (!currentNode.isNull()) {
                  QString nodeName = currentNode.nodeName();
                  if (nodeName == "Name")
                        part->setLongName(currentNode.toElement().text());
                  else if (nodeName == "Color") {}
                  else if (nodeName == "SystemsLayout") {}
                  // this is a typo is guitar pro - 'defaut' is correct here
                  else if (nodeName == "SystemsDefautLayout") {}
                  else if (nodeName == "RSE") {}
                  else if (nodeName == "GeneralMidi") {}
                  else if (nodeName == "PlaybackState") {}
                  else if (nodeName == "PlayingStyle") {}
                  else if (nodeName == "PageSetup") {}
                  else if (nodeName == "MultiVoice") {}
                  else if (nodeName == "ShortName")
                        part->setPartName(currentNode.toElement().text());
                  else if (nodeName == "Instrument") {
                        QString ref = currentNode.attributes().namedItem("ref").toAttr().value();
                        Instrument* instr = new Instrument;
                        // use an array as a map instead?
                        if (!ref.compare("e-gtr6")) {
                              *instr = instr->fromTemplate(Ms::searchTemplate("electric-guitar"));
                              part->setInstrument(*instr, 0);
                              }
                        else if (!ref.compare("tnr-s")) {
                              *instr = instr->fromTemplate(Ms::searchTemplate("voice"));
                              part->setInstrument(*instr, 0);
                              }
                        else if (!ref.compare("s-gtr6")) {
                              *instr = instr->fromTemplate(Ms::searchTemplate("guitar-steel"));
                              part->setInstrument(*instr, 0);
                              }
                        else if (!ref.compare("snt-lead-ss")) {
                              *instr = instr->fromTemplate(Ms::searchTemplate("poly-synth"));
                              part->setInstrument(*instr, 0);
                              }
                        else if (!ref.compare("f-bass5")) {
                              *instr = instr->fromTemplate(Ms::searchTemplate("bass-guitar"));
                              part->setInstrument(*instr, 0);
                              }
                        else if (!ref.compare("snt-bass-ss")) {
                              *instr = instr->fromTemplate(Ms::searchTemplate("metallic-synth"));
                              part->setInstrument(*instr, 0);
                              }
                        else if (!ref.compare("mrcs")) {
                              *instr = instr->fromTemplate(Ms::searchTemplate("maracas"));
                              part->setInstrument(*instr, 0);
                              }
                        else if (!ref.compare("drmkt")) {
                              *instr = instr->fromTemplate(Ms::searchTemplate("drumset"));
                              part->setInstrument(*instr, 0);
                              }
                        else
                              qDebug() << "WARINNG: unhandled playback instrument" << ref << "- defaulting.";
                        }
                  else if (nodeName == "Properties") {
                        QDomNode currentProperty = currentNode.firstChild();
                        while (!currentProperty.isNull()) {
                              QString propertyName = currentProperty.attributes().namedItem("name").toAttr().value();
                              if (!propertyName.compare("Tuning")) {
                                    // set up the tuning for the part
                                    QString tuningString = currentProperty.firstChild().toElement().text();
                                    QStringList tuningStringList = tuningString.split(" ");
                                    int strings = 0;
                                    int tuning[tuningStringList.length()];
                                    int frets   = 21;
                                    for (auto iter = tuningStringList.begin(); iter != tuningStringList.end(); ++iter) {
                                          int currentString = (*iter).toInt();
                                          tuning[strings] = currentString;
                                          strings++;
                                          }
                                          StringData* stringData = new StringData(frets, strings, tuning);
                                          Instrument* instr = part->instr();
                                          instr->setStringData(*stringData);
                                    }
                              else if (!propertyName.compare("DiagramCollection")) {
                                    QDomNode items = currentProperty.firstChild();
                                    QDomNode currentItem = items.firstChild();
                                    while (!currentItem.isNull()) {
                                          readChord(&currentItem, trackCounter);
                                          currentItem = currentItem.nextSibling();
                                          }
                                    }
                              else
                                    unhandledNode(nodeName);
                              currentProperty = currentProperty.nextSibling();
                              }
                        }
                  else
                        unhandledNode(nodeName);
                  currentNode = currentNode.nextSibling();
                  }

            // add in a new part
            part->insertStaff(s);
            score->staves().push_back(s);
            score->appendPart(part);
            trackCounter++;
            nextTrack = nextTrack.nextSibling();
            }
      // set the number of staves we need
      staves = score->staves().length();
      }

//---------------------------------------------------------
//   getNode
//---------------------------------------------------------

QDomNode GuitarPro6::getNode(QString id, QDomNode currentNode)
      {
      while (!(currentNode).isNull()) {
            QString currentId = currentNode.attributes().namedItem("id").toAttr().value();
            if (id.compare(currentId) == 0) {
                  return currentNode;
                  }
            currentNode = (currentNode).nextSibling();
            }
      qDebug("WARNING: A null node was returned when search for an identifier. Your Guitar Pro file may be corrupted.");
      return currentNode;
      }

//---------------------------------------------------------
//   findNumMeasures
//---------------------------------------------------------

int GuitarPro6::findNumMeasures(GPPartInfo* partInfo)
      {
      QDomNode finalMasterBar = partInfo->masterBars.nextSibling();
      while (!finalMasterBar.isNull()) {
            GpBar gpBar;
            gpBar.keysig = finalMasterBar.firstChild().toElement().text().toInt();
            QString timeSignature = finalMasterBar.firstChild().nextSibling().toElement().text();
            QList<QString> timeSignatureList = timeSignature.split("/");
            gpBar.timesig = Fraction(timeSignatureList.first().toInt(), timeSignatureList.last().toInt());
            bars.append(gpBar);
            if (finalMasterBar.nextSibling().isNull())
                  break;
            finalMasterBar = finalMasterBar.nextSibling();
            }
      QString bars = finalMasterBar.lastChildElement("Bars").toElement().text();
      //work out the number of measures (add 1 as couning from 0, and divide by number of parts)
      int numMeasures = (bars.split(" ").last().toInt() + 1) / score->parts().length();
      return numMeasures;
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
      int octaveInt;
      int toneInt;
      /* These numbers below were determined by creating all drum
       * notes in a GPX format file and then analyzing the score.gpif
       * file which specifies the score. */
      if (element == 11 && variation == 0) {
            octaveInt = 5;
            toneInt = 0;
            note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
      }
      else if (element == 0 && variation == 0) {
            octaveInt = 5;
            toneInt = 5;
      }
      else if (element == 5 && variation == 0) {
            octaveInt = 5;
            toneInt = 7;
      }
      else if (element == 6 && variation == 0) {
            octaveInt = 5;
            toneInt = 9;
      }
      else if (element == 7 && variation == 0) {
            octaveInt = 5;
            toneInt = 11;
      }
      else if (element == 1 && variation == 0) {
            octaveInt = 6;
            toneInt = 0;
      }
      else if (element == 1 && variation == 1) {
            octaveInt = 6;
            toneInt = 0;
            note->setHeadGroup(NoteHead::Group::HEAD_MI);
      }
      else if (element == 1 && variation == 2) {
            octaveInt = 6;
            toneInt = 0;
            note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
      }
      else if (element == 8 && variation == 0) {
            octaveInt = 6;
            toneInt = 2;
      }
      else if (element == 9 && variation == 0) {
            octaveInt = 6;
            toneInt = 4;
      }
      else if (element == 2 && variation == 0) {
            octaveInt = 6;
            toneInt = 4;
            note->setHeadGroup(NoteHead::Group::HEAD_TRIANGLE);
      }
      else if (element == 15 && variation == 0) {
            octaveInt = 6;
            toneInt = 5;
            note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
      }
      else if (element == 15 && variation == 1) {
            octaveInt = 6;
            toneInt = 5;
            note->setHeadGroup(NoteHead::Group::HEAD_DIAMOND);
      }
      else if (element == 15 && variation == 2) {
            octaveInt = 6;
            toneInt = 5;
            note->setHeadGroup(NoteHead::Group::HEAD_MI);
      }
      else if (element == 3 && variation == 0) {
            octaveInt = 6;
            toneInt = 5;
            note->setHeadGroup(NoteHead::Group::HEAD_TRIANGLE);
      }
      else if (element == 10 && variation == 0) {
            octaveInt = 6;
            toneInt = 7;
            note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
      }
      else if (element == 10 && variation == 1) {
            octaveInt = 6;
            toneInt = 7;
            note->setHeadGroup(NoteHead::Group::HEAD_SLASH);
      }
      else if (element == 10 && variation == 2) {
            octaveInt = 6;
            toneInt = 7;
            note->setHeadGroup(NoteHead::Group::HEAD_XCIRCLE);
      }
      else if (element == 12 && variation == 0) {
            octaveInt = 6;
            toneInt = 7;
            note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
      }
      else if (element == 4 && variation == 0) {
            octaveInt = 6;
            toneInt = 7;
            note->setHeadGroup(NoteHead::Group::HEAD_TRIANGLE);
      }
      else if (element == 14 && variation == 0) {
            octaveInt = 6;
            toneInt = 9;
            note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
      }
      else if (element == 13 && variation == 0) {
            octaveInt = 6;
            toneInt = 9;
            note->setHeadGroup(NoteHead::Group::HEAD_LA);
      }
      else if (element == 16 && variation == 0) {
            octaveInt = 6;
            toneInt = 11;
            note->setHeadGroup(NoteHead::Group::HEAD_NORMAL);
      }
      // multiply octaves by 12 as 12 semitones in octave
      note->setPitch((octaveInt * 12) + toneInt);
      }


void GuitarPro6::makeTie(Note* note) {
      bool found = false;
      Chord* chord     = note->chord();
      Segment* segment = chord->segment();
      segment = segment->prev1(Segment::Type::ChordRest);
      int track        = note->track();
      while (segment) {
            Element* e = segment->element(track);
            if (e) {
                  if (e->type() == Element::Type::CHORD) {
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
            segment = segment->prev1(Segment::Type::ChordRest);
      }
      if (!found)
            qDebug("tied note not found, pitch %d fret %d string %d", note->pitch(), note->fret(), note->string());
}

int GuitarPro6::readBeats(QString beats, GPPartInfo* partInfo, Measure* measure, int tick, int staffIdx, int voiceNum, Tuplet* tuplets[])
      {
            auto currentBeatList = beats.split(" ");
            for (auto currentBeat = currentBeatList.begin(); currentBeat != currentBeatList.end(); currentBeat++) {
                  int slide = -1;
                  if (slides->contains(staffIdx * VOICES + voiceNum))
                        slide = slides->take(staffIdx * VOICES + voiceNum);

                  Fraction l;
                  int dotted = 0;
                  QDomNode beat = getNode(*currentBeat, partInfo->beats);

                  Segment* segment = measure->getSegment(Segment::Type::ChordRest, tick);
                  QDomNode currentNode = beat.firstChild();
                  bool noteSpecified = false;
                  ChordRest* cr = segment->cr(staffIdx * VOICES + voiceNum);
                  bool tupletSet = false;
                  Tuplet* tuplet = tuplets[staffIdx * 2 + voiceNum];
                  bool grace = false;
                  int accent = 0;
                  while (!currentNode.isNull()) {
                        if (currentNode.nodeName() == "Notes") {
                              noteSpecified = true;
                              auto notesList = currentNode.toElement().text().split(" ");

                              // this could be set by rhythm if we dealt with a tuplet
                              if (!cr)
                                    cr = new Chord(score);
                              cr->setTrack(staffIdx * VOICES + voiceNum);
                              cr->setDuration(l);
                              TDuration d(l);
                              d.setDots(dotted);
                              cr->setDurationType(d);

                              if(!segment->cr(staffIdx * VOICES + voiceNum))
                                    segment->add(cr);


                              for (auto iter = notesList.begin(); iter != notesList.end(); ++iter) {
                                    // we have found a note
                                    QDomNode note = getNode(*iter, partInfo->notes);
                                    QDomNode currentNode = (note).firstChild();
                                    bool tie = false;
                                    // if a <Notes> tag is used but there is no <Note>, then we add a rest. This flag will allow us to check this.
                                    while (!currentNode.isNull()) {
                                          if (currentNode.nodeName() == "Properties") {
                                                QDomNode currentProperty = currentNode.firstChild();
                                                // these should not be in this scope - they may not even exist.
                                                QString stringNum;
                                                QString fretNum;
                                                QString tone;
                                                QString octave;
                                                QString midi;
                                                QString element;
                                                QString variation;
                                                while (!currentProperty.isNull()) {
                                                      QString argument = currentProperty.attributes().namedItem("name").toAttr().value();
                                                      if (argument == "String")
                                                            stringNum = currentProperty.firstChild().toElement().text();
                                                      else if (argument == "Element")
                                                            element = currentProperty.firstChild().toElement().text();
                                                      else if (argument == "Slide") {
                                                            int slideKind = currentProperty.firstChild().toElement().text().toInt();
                                                            if (slideKind >= 4)
                                                                  slide = slideKind;
                                                            else
                                                                  slides->insert(staffIdx * VOICES + voiceNum, slideKind);
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
                                                      else
                                                            qDebug() << "WARNING: Not handling node argument: " << argument << "in node" << currentNode.nodeName();
                                                      currentProperty = currentProperty.nextSibling();
                                                }

                                                Note* note = new Note(score);
                                                Chord* chord = static_cast<Chord*>(cr);
                                                chord->add(note);

                                                if (midi != "")
                                                      note->setPitch(midi.toInt());
                                                else if (element != "")
                                                      readDrumNote(note, element.toInt(), variation.toInt());
                                                else if (stringNum != "") {
                                                      Staff* staff = note->staff();
                                                      int fretNumber = fretNum.toInt();
                                                      int musescoreString = staff->part()->instr()->stringData()->strings() - 1 - stringNum.toInt();
                                                      auto pitch = staff->part()->instr()->stringData()->getPitch(musescoreString, fretNumber);
                                                      note->setFret(fretNumber);
                                                      // we need to turn this string number for GP to the the correct string number for musescore
                                                      note->setString(musescoreString);
                                                      note->setPitch(pitch);
                                                if (accent == 4) {
                                                      Articulation* art = new Articulation(note->score());
                                                      art->setArticulationType(ArticulationType::Marcato);
                                                      if (!note->score()->addArticulation(note, art))
                                                            delete art;
                                                      }
                                                }
                                                else if (tone != "")
                                                      note->setPitch((octave.toInt() * 12) + tone.toInt()); // multiply octaves by 12 as 12 semitones in octave
                                                if (grace) {
                                                      // chord->setDurationType(MScore::division/8);
                                                      // chord->setMag(note->chord()->staff()->mag() * score->styleD(StyleIdx::graceNoteMag));
                                                      // chord->setNoteType(NoteType::ACCIACCATURA);
                                                      }
                                                if (tie) {
                                                      makeTie(note);
                                                      tie = false;
                                                      }
                                                createSlide(slide, cr, staffIdx);

                                                note->setTpcFromPitch();
                                                currentNode = currentNode.nextSibling();
                                                }
                                          else if (!currentNode.nodeName().compare("Accent")) {
                                                // marcato
                                                if (!currentNode.toElement().text().compare("4"))
                                                      accent = 4;
                                                }
                                          else if (!currentNode.nodeName().compare("Tie")) {
                                                if (!currentNode.attributes().namedItem("destination").toAttr().value().compare("true"))
                                                      tie = true;
                                                }
                                          else
                                                unhandledNode(currentNode.nodeName());
                                          currentNode = currentNode.nextSibling();
                                    }
                              }
                        }
                        else if (!currentNode.nodeName().compare("Properties")) {
                              currentNode = currentNode.firstChild();
                              while (!currentNode.isNull()) {
                                    unhandledNode(currentNode.nodeName());
                                    }
                              }
                        else if (!currentNode.nodeName().compare("GraceNotes")) {
                              if (!currentNode.toElement().text().compare("BeforeBeat"))
                                    grace = true;
                              else
                                    qDebug("WARNING: New grace note type detected but unhandled.");
                              }
                        else if (currentNode.nodeName() == "Dynamic") {
                              }
                        else if (!currentNode.nodeName().compare("Chord")) {
                              int key = currentNode.toElement().text().toInt();
                              segment->add(fretDiagrams[key]);
                              }
                        else if (currentNode.nodeName() == "Rhythm") {
                              // we have found a rhythm
                              QString refString = currentNode.attributes().namedItem("ref").toAttr().value();
                              QDomNode rhythm = getNode(refString, partInfo->rhythms);
                              QDomNode currentNode = (rhythm).firstChild();
                              while (!currentNode.isNull()) {
                                    if (currentNode.nodeName() == "NoteValue") {
                                          l = rhythmToDuration(currentNode.toElement().text());
                                    }
                                    else if (currentNode.nodeName() == "AugmentationDot") {
                                          dotted = currentNode.attributes().namedItem("count").toAttr().value().toInt();
                                          for (int count = 1; count <= dotted; count++)
                                                l = l + (l / pow(2, count));
                                    }
                                    else if (currentNode.nodeName() == "PrimaryTuplet") {
                                          tupletSet = true;
                                          cr = new Chord(score);
                                          cr->setTrack(staffIdx * VOICES + voiceNum);
                                          if ((tuplet == 0) || (tuplet->elementsDuration() == tuplet->baseLen().fraction() * tuplet->ratio().numerator())) {
                                                tuplet = new Tuplet(score);
                                                tuplets[staffIdx * 2 + voiceNum] = tuplet;
                                                tuplet->setParent(measure);
                                          }
                                          tuplet->setTrack(cr->track());
                                          tuplet->setBaseLen(l);
                                          tuplet->setRatio(Fraction(currentNode.attributes().namedItem("num").toAttr().value().toInt(),currentNode.attributes().namedItem("den").toAttr().value().toInt()));
                                          tuplet->add(cr);
                                    }
                                    else
                                          qDebug() << "WARNING: Not handling node: " << currentNode.nodeName();
                                    currentNode = currentNode.nextSibling();
                              }
                        }
                        else
                              qDebug() << "WARNING: Not handling beat XML tag:" << currentNode.nodeName();
                        currentNode = currentNode.nextSibling();
                        dotted = 0;
                  }
                  // we have handled the note - was there a note?
                  if (!noteSpecified) {
                        // add a rest with length of l
                        cr = new Rest(score);
                        cr->setTrack(staffIdx * VOICES + voiceNum);
                        if (tupletSet)
                              tuplet->add(cr);
                        TDuration d(l);
                        cr->setDuration(l);
                        cr->setDurationType(d);
                        if(!segment->cr(staffIdx * VOICES + voiceNum))
                              segment->add(cr);
                  }
                  tick += cr->actualTicks();
            }
            return tick;
      }

//---------------------------------------------------------
//   readBars
//---------------------------------------------------------

void GuitarPro6::readBars(QDomNode* barList, Measure* measure, ClefType oldClefId[], GPPartInfo* partInfo, KeySig* /*t*/)
      {
      // unique bar identifiers are represented as a space separated string of numbers
      QStringList barsString = barList->toElement().text().split(" ");
      int staffIdx = 0;

      // used to keep track of tuplets
      Tuplet* tuplets[staves * 2];
      for (int track = 0; track < staves*2; ++track)
            tuplets[track] = 0;

      // iterate through all the bars that have been specified
      for (auto iter = barsString.begin(); iter != barsString.end(); ++iter) {
            int tick = measure->tick();

            QDomNode barNode = getNode(*iter, partInfo->bars);
            QDomNode currentNode = (barNode).firstChild();
            QDomNode voice;
            while (!currentNode.isNull()) {
                  voice.clear();
                  // get the clef of the bar and apply
                  if (!currentNode.nodeName().compare("Clef")) {
                        QString clefString = currentNode.toElement().text();
                        ClefType clefId;
                        if (!clefString.compare("F4"))
                              clefId = ClefType::F8;
                        else if (!clefString.compare("G2"))
                              clefId = ClefType::G3;
                        else if (!clefString.compare("Neutral"))
                              clefId = ClefType::PERC;
                        else
                              qDebug() << "WARNING: unhandled clef type: " << clefString;
                        Clef* newClef = new Clef(score);
                        newClef->setClefType(clefId);
                        newClef->setTrack(staffIdx * VOICES);
                        Segment* segment = measure->getSegment(Segment::Type::Clef, 0);
                        // only add the clef to the bar if it differs from previous measure
                        if (measure->prevMeasure()) {
                              if (clefId != oldClefId[staffIdx]) {
                                    segment->add(newClef);
                                    oldClefId[staffIdx] = clefId;
                                    }
                              }
                        else  {
                              segment->add(newClef);
                              oldClefId[staffIdx] = clefId;
                              }
                        }
                  // a repeated bar (simile marking)
                  else if (!currentNode.nodeName().compare("SimileMark")) {
                        if (!currentNode.toElement().text().compare("Simple") ||
                            !currentNode.toElement().text().compare("FirstOfDouble") ||
                            !currentNode.toElement().text().compare("SecondOfDouble"))
                              measure->cmdInsertRepeatMeasure(staffIdx);
                        else
                              qDebug() << "WARNING: unhandle similie mark type: " << currentNode.toElement().text();
                        }
                  // new voice specification
                  else if (!currentNode.nodeName().compare("Voices")) {
                        QString voicesString = currentNode.toElement().text();
                        auto currentVoice = voicesString.split(" ").first();
                        // if the voice is not -1 then we set voice
                        if (currentVoice.compare("-1"))
                              voice = getNode(currentVoice, partInfo->voices);
                        }
                  else if (!currentNode.nodeName().compare("XProperties")) {}
                  else
                        unhandledNode(currentNode.nodeName());
                  // go to the next node in the tree
                  currentNode = currentNode.nextSibling();
                  }

            int voiceNum = 0;
            if (voice.isNull()) {
                  Fraction l = Fraction(1,1);
                  // add a rest with length of l
                  ChordRest* cr = new Rest(score);
                  cr->setTrack(staffIdx * VOICES + voiceNum);
                  TDuration d(l);
                  cr->setDuration(l);
                  cr->setDurationType(d);
                  Segment* segment = measure->getSegment(Segment::Type::ChordRest, tick);
                  if(!segment->cr(staffIdx * VOICES + voiceNum))
                        segment->add(cr);
                  tick += cr->actualTicks();
                  staffIdx++;
                  continue;
                  }

            // read the beats that occur in the bar
            tick = readBeats(voice.firstChild().toElement().text(), partInfo, measure, tick, staffIdx, voiceNum, tuplets);
            qDebug("finished readBeats");
            // increment the counter for parts
            staffIdx++;
            }
      qDebug("finished read bars");
      }

//---------------------------------------------------------
//   readMasterBars
//---------------------------------------------------------

void GuitarPro6::readMasterBars(GPPartInfo* partInfo)
      {
      Measure* measure = score->firstMeasure();
      int bar = 0;
      QDomNode nextMasterBar = partInfo->masterBars;
      nextMasterBar = nextMasterBar.nextSibling();
      int measureCounter = 0;
      ClefType oldClefId[staves];
      do    {
            const GpBar& gpbar = bars[bar];

            if (!gpbar.marker.isEmpty()) {
                  Text* s = new RehearsalMark(score);
                  s->setText(gpbar.marker.trimmed());
                  s->setTrack(0);
                  Segment* segment = measure->getSegment(Segment::Type::ChordRest, measure->tick());
                  segment->add(s);
                  }

            QDomNode key = nextMasterBar.firstChild();
            KeySig* t = new KeySig(score);
            t->setKey(Key(key.firstChild().toElement().text().toInt()));
            t->setTrack(0);
            measure->getSegment(Segment::Type::KeySig, measure->tick())->add(t);

            QDomNode time = key.nextSibling();
            measure->setTimesig(bars[measureCounter].timesig);
            measure->setLen(bars[measureCounter].timesig);
            QDomNode barList = time.nextSibling();
            readBars(&barList, measure, oldClefId, partInfo, t);
            measureCounter++;
            nextMasterBar = nextMasterBar.nextSibling();
            measure = measure->nextMeasure();
            bar++;
            } while (!nextMasterBar.isNull());
      qDebug("finished reading master bars");
      }

//---------------------------------------------------------
//   readGpif
//---------------------------------------------------------

void GuitarPro6::readGpif(QByteArray* data)
      {
      QDomDocument qdomDoc;
      qdomDoc.setContent(*data);
      QDomElement qdomElem = qdomDoc.documentElement();
      // GPRevision node
      QDomNode revision = qdomElem.firstChild();
      // Score node
      QDomNode scoreNode = revision.nextSibling();
      readScore(&scoreNode);
      // MasterTrack node
      QDomNode masterTrack = scoreNode.nextSibling();
      readMasterTracks(&masterTrack);
      // Tracks node
      QDomNode eachTrack = masterTrack.nextSibling();
      readTracks(&eachTrack);

      // now we know how many staves there are from readTracks, we can initialise slurs
      slurs = new Slur*[staves];
      for (int i = 0; i < staves; ++i)
            slurs[i] = 0;

      // MasterBars node
      GPPartInfo partInfo;
      QDomNode masterBars = eachTrack.nextSibling();
      QDomNode bars = masterBars.nextSibling();
      QDomNode voices = bars.nextSibling();
      QDomNode beats = voices.nextSibling();
      QDomNode notes = beats.nextSibling();
      QDomNode rhythms = notes.nextSibling();

      // set up the partInfo struct to contain information from the file
      partInfo.masterBars = masterBars.firstChild();
      partInfo.bars = bars.firstChild();
      partInfo.voices = voices.firstChild();
      partInfo.beats = beats.firstChild();
      partInfo.notes = notes.firstChild();
      partInfo.rhythms = rhythms.firstChild();

      measures = findNumMeasures(&partInfo);

      createMeasures();
      readMasterBars(&partInfo);
      // set the starting tempo of the score
      setTempo(/*tempo*/120, score->firstMeasure());
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void GuitarPro6::read(QFile* fp)
      {
      qDebug("reading guitar pro v6 file (.gpx)...");
      f = fp;

      slides = new QMap<int,int>();

      previousDynamic = -1;
      previousTempo = -1;
      this->buffer = new QByteArray();
      *(this->buffer) = fp->readAll();
      // decompress and read files contained within GPX file
      readGPX(this->buffer);
      delete this->buffer;
      }

//---------------------------------------------------------
//   readBeatEffects
//---------------------------------------------------------

int GuitarPro6::readBeatEffects(int, Segment*)
      {
      qDebug("reading beat effects (.gpx)...\n");
      return 0;
      }

}
