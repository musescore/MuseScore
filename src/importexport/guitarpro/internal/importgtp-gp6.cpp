/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "importgtp.h"

#include <cmath>

#include "serialization/xmldom.h"

#include "gtp/gp6dombuilder.h"
#include "gtp/gpconverter.h"

#include "engraving/dom/factory.h"
#include "engraving/dom/arpeggio.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/barline.h"
#include "engraving/dom/box.h"
#include "engraving/dom/bracket.h"
#include "engraving/dom/bracketItem.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/fingering.h"
#include "engraving/dom/fret.h"
#include "engraving/dom/glissando.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/marker.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/measurebase.h"
#include "engraving/dom/measurerepeat.h"
#include "engraving/dom/note.h"
#include "engraving/dom/notedot.h"
#include "engraving/dom/ottava.h"
#include "engraving/dom/part.h"
#include "engraving/dom/rehearsalmark.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/stringdata.h"
#include "types/symid.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/text.h"
#include "engraving/dom/textline.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/tremolobar.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/volta.h"

#include "log.h"

using namespace mu;
using namespace muse;
using namespace muse::io;
using namespace mu::engraving;

namespace mu::iex::guitarpro {
const std::map<std::string, std::string> GuitarPro6::instrumentMapping = {
    { "2Mrcs",           "maracas" },
    { "a-bass4",         "acoustic-bass" },
    { "a-bass5",         "acoustic-bass" },
    { "a-bass6",         "acoustic-bass" },
    { "alt-c",           "alto" },
    { "alt-s",           "alto" },
    { "a-piano-gs",      "piano" },
    { "a-piano-ss",      "piano" },
    { "bass-c",          "bass" },
    { "bass-flt-c",      "bass-flute" },
    { "bassn",           "bassoon" },
    { "bass-s",          "bass" },
    { "basstuba-eb",     "bass-eb-tuba" },
    { "bnj4",            "banjo" },
    { "bnj5",            "banjo" },
    { "bnj6",            "banjo" },
    { "bongo",           "bongos" },
    { "brthns",          "baritone-horn" },
    { "brtn-c",          "baritone" },
    { "brtn-s",          "baritone" },
    { "cbs",             "cabasa" },
    { "cello",           "violoncello" },
    { "china",           "chinese-tom-toms" },
    { "clrnt-a",         "a-clarinet" },
    { "clrnt-bb-bass",   "bass-clarinet" },
    { "clrnt-bb",        "bb-clarinet" },
    { "clrnt-c",         "c-clarinet" },
    { "clrnt-d",         "d-clarinet" },
    { "clrnt-eb",        "eb-clarinet" },
    { "clrnt",           "clarinet" },
    { "clst-gs",         "celesta" },
    { "clst-ss",         "celesta" },
    { "clvs",            "claves" },
    { "cngKit",          "congas" },
    { "conga",           "congas" },
    { "cowbell",         "cowbell" },
    { "crash",           "crash-cymbal" },
    { "cstnt",           "castanets" },
    { "ctbassn",         "contrabassoon" },
    { "ctbass",          "contrabass" },
    { "cuicaKit",        "cuica" },
    { "cuica",           "cuica" },
    { "drmkt",           "drumset" },
    { "e-bass4",         "bass-guitar" },
    { "e-bass5",         "bass-guitar" },
    { "e-bass6",         "bass-guitar" },
    { "e-gtr12",         "electric-guitar-treble-clef" },
    { "e-gtr6",          "electric-guitar-treble-clef" },
    { "e-gtr7",          "electric-guitar-treble-clef" },
    { "e-gtr8",          "electric-guitar-treble-clef" },
    { "em-organ-gs",     "organ" },
    { "em-organ-ss",     "organ" },
    { "en-horn",         "english-horn" },
    { "e-piano-gs",      "electric-piano" },
    { "e-piano-ss",      "electric-piano" },
    { "flt-c",           "flute" },
    { "flt-g",           "alto-flute" },
    { "flt-whstl",       "tin-whistle" },
    { "fr-horn",         "horn" },
    { "grcss",           "bass-drum" },         //grancassa is an alternative name for bass drum
    { "guiro",           "guiro" },
    { "harp-gs",         "harp" },
    { "harp-ss",         "harp" },
    { "hclap",           "hand-clap" },
    { "hihat",           "hi-hat" },
    { "hrpch-gs",        "harpsichord" },
    { "hrpch-ss",        "harpsichord" },
    { "jngl-bell",       "sleigh-bells" },
    { "klmb",            "kalimba" },
    { "mrcs",            "maracas" },
    { "n-gtr6",          "guitar-nylon-treble-clef" },
    { "n-gtr7",          "guitar-nylon-treble-clef" },
    { "n-gtr8",          "guitar-nylon-treble-clef" },
    { "oboe",            "oboe" },
    { "ocrn",            "ocarina" },
    { "pccl",            "piccolo" },
    { "pedalhihat",      "hi-hat" },
    { "pnflt",           "pan-flute" },
    { "ptt",             "cymbal" },        //piatti is cymbal in italian
    { "rec",             "recorder" },
    { "ride",            "ride-cymbal" },
    { "rvs-cymb",        "cymbal" },
    { "sax-alt-eb",      "alto-saxophone" },
    { "sax-bar-eb",      "baritone-saxophone" },
    { "sax-bass-eb",     "bass-saxophone" },
    { "sax-ms-f",        "mezzo-soprano-saxophone" },
    { "sax-sop-bb",      "soprano-saxophone" },
    { "sax-ten-bb",      "tenor-saxophone" },
    { "sax-ten-c",       "melody-saxophone" },
    { "s-bass4",         "electric-bass" },
    { "s-bass5",         "5-string-electric-bass" },
    { "s-gtr12",         "guitar-steel-treble-clef" },
    { "s-gtr6",          "guitar-steel-treble-clef" },
    { "s-gtr7",          "guitar-steel-treble-clef" },
    { "s-gtr8",          "guitar-steel-treble-clef" },
    { "shkr",            "percussion" },
    { "shmsn",           "shamisen" },
    { "shn",             "sheng" },
    { "snare",           "snare-drum" },
    { "snr",             "snare-drum" },
    { "snt-brass-gs",    "brass-synthesizer" },
    { "snt-brass-ss",    "brass-synthesizer" },
    //{"snt-key-gs",    ""},
    //{"snt-key-ss",    ""},
    //{"snt-seq-gs",    ""},
    //{"snt-seq-ss",    ""},
    { "snt-lead-gs",     "poly-synth" },
    { "snt-lead-ss",     "poly-synth" },
    { "snt-pad",         "pad-synth" },
    { "snt-pad-gs",      "pad-synth" },
    { "snt-pad-ss",      "pad-synth" },
    { "splash",          "splash-cymbal" },
    { "sprn-c",          "soprano" },
    { "sprn-s",          "soprano" },
    { "tmbrn",           "tambourine" },       // to be mapped
    { "Tambourine-Perc", "tambourine" },
    { "Tambourine",      "tambourine" },
    { "tmblKit",         "timbales" },
    { "tmbl",            "timbales" },
    { "tmpn",            "timpani" },
    { "tnklbll",         "tubular-bells" },       //The short form does not match but this is very likely due to the description
    { "tnr-c",           "tenor" },
    { "tnr-s",           "tenor" },
    { "Triangle-Percu",  "triangle" },
    { "trmbn-bb-bass",   "bass-trombone" },
    { "trmbn-bb",        "tenor-trombone" },
    { "trmbn-bb-treble", "trombone-treble" },
    { "trmbn-eb",        "alto-trombone" },
    { "trmpt-a",         "a-trumpet" },
    { "trmpt-bb",        "bb-trumpet" },
    { "trmpt-c-bass",    "c-bass-trumpet" },
    { "trmpt-c",         "c-trumpet" },
    { "trmpt-d",         "d-trumpet" },
    { "trmpt-eb-bass",   "eb-bass-trumpet" },
    { "trmpt-eb",        "eb-trumpet" },
    { "trmpt-e",         "e-trumpet" },
    { "trmpt-f",         "f-trumpet" },
    { "trmpt-flgh",      "flugelhorn" },
    { "trngl",           "triangle" },
    { "ukll4",           "ukulele" },
    { "vbrphn",          "vibraphone" },
    { "vbrslp",          "vibraslap" },       // to be mapped
    { "vla",             "viola" },
    { "vln",             "violin" },
    { "wdblckKit",       "wood-blocks" },
    { "wdblck",          "wood-blocks" },
    { "whstlKit",        "slide-whistle" },
    { "whstl",           "tin-whistle" },
    { "xlphn",           "xylophone" }
};

std::unique_ptr<IGPDomBuilder> GuitarPro6::createGPDomBuilder() const
{
    return std::make_unique<GP6DomBuilder>();
}

//---------------------------------------------------------
//   readBit
//---------------------------------------------------------

int GuitarPro6::readBit(ByteArray* buffer)
{
    // calculate the byte index by dividing the position in bits by the bits per byte
    size_t byteIndex = position / BITS_IN_BYTE;

    // calculate our offset so we know how much to bit shift
    int byteOffset = ((BITS_IN_BYTE - 1) - (position % BITS_IN_BYTE));

    // calculate the bit which we want to read
    char byte = (byteIndex < buffer->size()) ? (*buffer)[byteIndex] : char(0);
    int bit = (((byte & 0xff) >> byteOffset) & 0x01);

    // increment our current position so we know this bit has been read
    position++;
    return bit;         // return the bit we calculated
}

//---------------------------------------------------------
//   readBits
//---------------------------------------------------------

int GuitarPro6::readBits(ByteArray* buffer, int bitsToRead)
{
    int bits = 0;
    for (int i = (bitsToRead - 1); i >= 0; i--) {
        bits |= (readBit(buffer) << i);
    }
    return bits;
}

//---------------------------------------------------------
//   readBitsReversed
//---------------------------------------------------------

int GuitarPro6::readBitsReversed(ByteArray* buffer, int bitsToRead)
{
    int bits = 0;
    for (int i = 0; i < bitsToRead; i++) {
        bits |= readBit(buffer) << i;
    }
    return bits;
}

//---------------------------------------------------------
//   getBytes
//---------------------------------------------------------

ByteArray GuitarPro6::getBytes(ByteArray* buffer, int offset, int length)
{
    ByteArray newBytes;
    // compute new bytes from our buffer and return byte array
    for (int i = 0; i < length; i++) {
        if (static_cast<int>(buffer->size()) > offset + i) {
            newBytes.insert(i, ((*buffer)[offset + i]));
        }
    }
    return newBytes;
}

//---------------------------------------------------------
//   readInteger
//---------------------------------------------------------

int GuitarPro6::readInteger(ByteArray* buffer, int offset)
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

ByteArray GuitarPro6::readString(ByteArray* buffer, int offset, int length)
{
    ByteArray filename;
    // compute the string by iterating through the buffer
    for (int i = 0; i < length; i++) {
        int charValue = (((*buffer)[offset + i]) & 0xff);
        if (charValue == 0) {
            break;
        }
        filename.push_back((char)charValue);
    }
    return filename;
}

//---------------------------------------------------------
//   unhandledNode
//---------------------------------------------------------

void GuitarPro6::unhandledNode(String nodeName)
{
    LOGD() << "WARNING: Discovered unhandled node name" << nodeName;
}

//---------------------------------------------------------
//   getNode
//---------------------------------------------------------

XmlDomNode GuitarPro6::getNode(const String& id, XmlDomNode currentDomNode)
{
    while (!(currentDomNode).isNull()) {
        String currentId = currentDomNode.toElement().attribute("id").value();
        if (currentId == id) {
            return currentDomNode;
        }
        currentDomNode = currentDomNode.nextSibling();
    }
    LOGD() << "WARNING: A null node was returned when search for the identifier" << id << ". Your Guitar Pro file may be corrupted.";
    return currentDomNode;
}

//---------------------------------------------------------
//   readGpif
//---------------------------------------------------------

void GuitarPro6::readGpif(ByteArray* data)
{
    XmlDomDocument domDoc;
    domDoc.setContent(*data);
    XmlDomElement domElem = domDoc.rootElement();

    auto builder = createGPDomBuilder();
    builder->buildGPDomModel(&domElem);

    GPConverter scoreBuilder(score, builder->getGPDomModel(), iocContext());
    scoreBuilder.convertGP();
}

//---------------------------------------------------------
//   parseFile
//---------------------------------------------------------

void GuitarPro6::parseFile(const char* filename, ByteArray* data)
{
    // test to check if we are dealing with the score
    if (!strcmp(filename, "score.gpif")) {
        readGpif(data);
    }
}

//---------------------------------------------------------
//   readGPX
//---------------------------------------------------------

void GuitarPro6::readGPX(ByteArray* buffer)
{
    // start by reading the file header. It will tell us if the byte array is compressed.
    int fileHeader = readInteger(buffer, 0);

    if (fileHeader == GPX_HEADER_COMPRESSED) {
        // this is  a compressed file.
        int length             = readInteger(buffer, position / BITS_IN_BYTE);
        ByteArray bcfsBuffer;
        bcfsBuffer.reserve(length);
        int positionCounter    = 0;
        while ((position / BITS_IN_BYTE) < length) {
            // read the bit indicating compression information
            int flag = readBits(buffer, 1);

            if (flag) {
                int bits = readBits(buffer, 4);
                int offs = readBitsReversed(buffer, bits);
                int size = readBitsReversed(buffer, bits);

                int pos = (static_cast<int>(bcfsBuffer.size()) - offs);
                for (int i = 0; i < (size > offs ? offs : size); i++) {
                    bcfsBuffer.insert(positionCounter, bcfsBuffer[pos + i]);
                    positionCounter++;
                }
            } else {
                int size = readBitsReversed(buffer, 2);
                for (int i = 0; i < size; i++) {
                    bcfsBuffer.insert(positionCounter, readBits(buffer, 8));
                    positionCounter++;
                }
            }
        }
        // recurse on the decompressed file stored as a byte array
        readGPX(&bcfsBuffer);
    } else if (fileHeader == GPX_HEADER_UNCOMPRESSED) {
        // this is an uncompressed file - strip the header off
        *buffer = buffer->right(buffer->size() - sizeof(int));
        size_t sectorSize = 0x1000;
        int offset        = 0;
        while ((offset = (offset + static_cast<int>(sectorSize))) + 3 < static_cast<int>(buffer->size())) {
            int newInt = readInteger(buffer, offset);
            if (newInt == 2) {
                int indexFileName = (offset + 4);
                int indexFileSize = (offset + 0x8C);
                int indexOfBlock  = (offset + 0x94);

                // create a byte array and put information about files found in it
                int block             = 0;
                int blockCount        = 0;
                ByteArray* fileBytes = new ByteArray();
                while ((block = (readInteger(buffer, (indexOfBlock + (4 * (blockCount++)))))) != 0) {
                    fileBytes->push_back(getBytes(buffer, (offset = (block * static_cast<int>(sectorSize))), static_cast<int>(sectorSize)));
                }
                // get file information and read the file
                size_t fileSize = readInteger(buffer, indexFileSize);
                if (fileBytes->size() >= fileSize) {
                    ByteArray filenameBytes = readString(buffer, indexFileName, 127);
                    const char* filename = filenameBytes.constChar();
                    ByteArray data = getBytes(fileBytes, 0, static_cast<int>(fileSize));
                    parseFile(filename, &data);
                }
                delete fileBytes;
            }
        }
    }
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

bool GuitarPro6::read(IODevice* io)
{
    f = io;
    previousTempo = -1;
    // decompress and read files contained within GPX file
    ByteArray ba = io->readAll();

    readGPX(&ba);

    return true;
}
} // namespace mu::iex::guitarpro
