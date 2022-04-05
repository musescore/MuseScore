/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include <QDebug>
#include <cmath>

#include "gtp/gp6dombuilder.h"
#include "gtp/gp7dombuilder.h"
#include "gtp/gpconverter.h"

#include "libmscore/factory.h"
#include "libmscore/arpeggio.h"
#include "libmscore/articulation.h"
#include "libmscore/barline.h"
#include "libmscore/bend.h"
#include "libmscore/box.h"
#include "libmscore/bracket.h"
#include "libmscore/bracketItem.h"
#include "libmscore/chord.h"
#include "libmscore/clef.h"
#include "libmscore/dynamic.h"
#include "libmscore/excerpt.h"
#include "libmscore/fingering.h"
#include "libmscore/fret.h"
#include "libmscore/glissando.h"
#include "libmscore/hairpin.h"
#include "libmscore/harmony.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/keysig.h"
#include "libmscore/lyrics.h"
#include "libmscore/marker.h"
#include "libmscore/masterscore.h"
#include "libmscore/measure.h"
#include "libmscore/measurebase.h"
#include "libmscore/measurerepeat.h"
#include "libmscore/note.h"
#include "libmscore/notedot.h"
#include "libmscore/ottava.h"
#include "libmscore/part.h"
#include "libmscore/rehearsalmark.h"
#include "libmscore/rest.h"
#include "libmscore/segment.h"
#include "libmscore/slur.h"
#include "libmscore/staff.h"
#include "libmscore/stafftext.h"
#include "libmscore/stafftype.h"
#include "libmscore/stringdata.h"
#include "types/symid.h"
#include "libmscore/tempotext.h"
#include "libmscore/text.h"
#include "libmscore/textline.h"
#include "libmscore/tie.h"
#include "libmscore/timesig.h"
#include "libmscore/tremolo.h"
#include "libmscore/tremolobar.h"
#include "libmscore/tuplet.h"
#include "libmscore/volta.h"

using namespace mu::engraving;

namespace Ms {
const std::map<QString, QString> GuitarPro6::instrumentMapping = {
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
    { "grcss",           "bass-drum" },         //grancassa is an alterantive name for bass drum
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

int GuitarPro6::readBit(QByteArray* buffer)
{
    // calculate the byte index by dividing the position in bits by the bits per byte
    int byteIndex = position / BITS_IN_BYTE;

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

int GuitarPro6::readBits(QByteArray* buffer, int bitsToRead)
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

int GuitarPro6::readBitsReversed(QByteArray* buffer, int bitsToRead)
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

QByteArray GuitarPro6::getBytes(QByteArray* buffer, int offset, int length)
{
    QByteArray newBytes;
    // compute new bytes from our buffer and return byte array
    for (int i = 0; i < length; i++) {
        if (buffer->length() > offset + i) {
            newBytes.insert(i, ((*buffer)[offset + i]));
        }
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

void GuitarPro6::unhandledNode(QString nodeName)
{
    qDebug() << "WARNING: Discovered unhandled node name" << nodeName;
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
//   readGpif
//---------------------------------------------------------

void GuitarPro6::readGpif(QByteArray* data)
{
    QDomDocument qdomDoc;
    qdomDoc.setContent(*data);
    QDomElement qdomElem = qdomDoc.documentElement();

    auto builder = createGPDomBuilder();
    builder->buildGPDomModel(&qdomElem);

    GPConverter scoreBuilder(score, builder->getGPDomModel());
    scoreBuilder.convertGP();
}

//---------------------------------------------------------
//   parseFile
//---------------------------------------------------------

void GuitarPro6::parseFile(const char* filename, QByteArray* data)
{
    // test to check if we are dealing with the score
    if (!strcmp(filename, "score.gpif")) {
        readGpif(data);
    }
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
        while (!f->error() && (position / BITS_IN_BYTE) < length) {
            // read the bit indicating compression information
            int flag = readBits(buffer, 1);

            if (flag) {
                int bits = readBits(buffer, 4);
                int offs = readBitsReversed(buffer, bits);
                int size = readBitsReversed(buffer, bits);

                QByteArray bcfsBufferCopy = *bcfsBuffer;
                int pos                   = (bcfsBufferCopy.length() - offs);
                for (int i = 0; i < (size > offs ? offs : size); i++) {
                    bcfsBuffer->insert(positionCounter, bcfsBufferCopy[pos + i]);
                    positionCounter++;
                }
            } else {
                int size = readBitsReversed(buffer, 2);
                for (int i = 0; i < size; i++) {
                    bcfsBuffer->insert(positionCounter, readBits(buffer, 8));
                    positionCounter++;
                }
            }
        }
        // recurse on the decompressed file stored as a byte array
        readGPX(bcfsBuffer);
        delete bcfsBuffer;
    } else if (fileHeader == GPX_HEADER_UNCOMPRESSED) {
        // this is an uncompressed file - strip the header off
        *buffer = buffer->right(buffer->length() - sizeof(int));
        int sectorSize = 0x1000;
        int offset     = 0;
        while ((offset = (offset + sectorSize)) + 3 < buffer->length()) {
            int newInt = readInteger(buffer, offset);
            if (newInt == 2) {
                int indexFileName = (offset + 4);
                int indexFileSize = (offset + 0x8C);
                int indexOfBlock  = (offset + 0x94);

                // create a byte array and put information about files found in it
                int block             = 0;
                int blockCount        = 0;
                QByteArray* fileBytes = new QByteArray();
                while ((block = (readInteger(buffer, (indexOfBlock + (4 * (blockCount++)))))) != 0) {
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
