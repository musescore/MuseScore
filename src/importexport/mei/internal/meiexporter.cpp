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

#include "meiexporter.h"

#include "libmscore/articulation.h"
#include "libmscore/barline.h"
#include "libmscore/beam.h"
#include "libmscore/box.h"
#include "libmscore/bracket.h"
#include "libmscore/chord.h"
#include "libmscore/clef.h"
#include "libmscore/dynamic.h"
#include "libmscore/factory.h"
#include "libmscore/jump.h"
#include "libmscore/key.h"
#include "libmscore/keysig.h"
#include "libmscore/lyrics.h"
#include "libmscore/marker.h"
#include "libmscore/masterscore.h"
#include "libmscore/measure.h"
#include "libmscore/note.h"
#include "libmscore/part.h"
#include "libmscore/rest.h"
#include "libmscore/segment.h"
#include "libmscore/sig.h"
#include "libmscore/slur.h"
#include "libmscore/staff.h"
#include "libmscore/tempotext.h"
#include "libmscore/text.h"
#include "libmscore/timesig.h"
#include "libmscore/timesig.h"
#include "libmscore/tuplet.h"
#include "libmscore/volta.h"

#include "log.h"

#include "thirdparty/libmei/cmn.h"
#include "thirdparty/libmei/shared.h"

using namespace mu::iex::mei;
using namespace mu::engraving;

// Number of spaces for the XML indentation. Set to 0 for tabs
#define MEI_INDENT 3

/**
 * Write the Score to the destination file.
 * Return false on error.
 */

bool MeiExporter::write(QIODevice& destinationDevice)
{
    // Still using QTextStream since we have a QIODevice
    QTextStream out(&destinationDevice);

    try {
        pugi::xml_document meiDoc;

        pugi::xml_node decl = meiDoc.prepend_child(pugi::node_declaration);
        decl.append_attribute("version") = "1.0";
        decl.append_attribute("encoding") = "UTF-8";

        // schema processing instruction
        std::string schema = "https://music-encoding.org/schema/dev/mei-basic.rng";
        decl = meiDoc.append_child(pugi::node_declaration);
        decl.set_name("xml-model");
        decl.append_attribute("href") = schema.c_str();
        decl.append_attribute("type") = "application/xml";
        decl.append_attribute("schematypens") = "http://relaxng.org/ns/structure/1.0";

        decl = meiDoc.append_child(pugi::node_declaration);
        decl.set_name("xml-model");
        decl.append_attribute("href") = schema.c_str();
        decl.append_attribute("type") = "application/xml";
        decl.append_attribute("schematypens") = "http://purl.oclc.org/dsdl/schematron";

        m_mei = meiDoc.append_child("mei");
        m_mei.append_attribute("xmlns") = "http://www.music-encoding.org/ns/mei";

        libmei::AttConverter converter;
        libmei::meiVersion_MEIVERSION meiVersion = libmei::meiVersion_MEIVERSION_5_0_0_devplusbasic;
        m_mei.append_attribute("meiversion") = (converter.MeiVersionMeiversionToStr(meiVersion)).c_str();

        // TODO: actual export

        unsigned int output_flags = pugi::format_default;

        // Tabulation of MEI_INDENT * spaces (tabs if 0)
        std::string indent = MEI_INDENT ? std::string(MEI_INDENT, ' ') : "\t";
        std::stringstream strStream;
        meiDoc.save(strStream, indent.c_str(), output_flags);
        out << String::fromStdString(strStream.str());
    }
    catch (char* str) {
        // Do something with the error message
        return false;
    }

    out.flush();
    return true;
}
