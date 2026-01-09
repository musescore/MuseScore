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
#include "paletteworkspacesetup.h"

#include "io/buffer.h"

#include "palettecreator.h"
#include "../palettetypes.h"

#include "engraving/dom/masterscore.h"
#include "engraving/rw/xmlreader.h"
#include "engraving/rw/xmlwriter.h"

#include "log.h"

using namespace mu;
using namespace mu::palette;
using namespace muse;
using namespace muse::io;
using namespace muse::workspace;

static const AsciiStringView PALETTE_XML_TAG("PaletteBox");

PaletteTreePtr PaletteWorkspaceSetup::readPalette(const ByteArray& data, const muse::modularity::ContextPtr& iocCtx)
{
    ByteArray ba = ByteArray::fromRawData(data.constData(), data.size());
    Buffer buf(&ba);
    buf.open(IODevice::ReadOnly);
    mu::engraving::XmlReader reader(&buf);

    while (!reader.atEnd()) {
        reader.readNextStartElement();

        if (reader.name() == PALETTE_XML_TAG) {
            int mscVersion = 0;
            if (reader.hasAttribute("version")) {
                const String version = reader.attribute("version");
                const StringList sl = version.split(u'.');
                mscVersion = sl.size() == 2 ? sl[0].toInt() * 100 + sl[1].toInt() : 0;
            } else {
                // Versioning workspace palette files started in 4.7. All unversioned files should be treated like 4.6 files
                mscVersion = 460;
            }
            engraving::gpaletteScore->setMscVersion(mscVersion);

            PaletteTreePtr tree = std::make_shared<PaletteTree>();
            tree->read(reader, false, iocCtx);
            return tree;
        }
    }

    return nullptr;
}

void PaletteWorkspaceSetup::writePalette(const PaletteTreePtr& tree, QByteArray& data)
{
    Buffer buf;
    buf.open(IODevice::WriteOnly);
    mu::engraving::XmlWriter writer(&buf);
    tree->write(writer, false);
    writer.flush();
    data = buf.data().toQByteArray();
}

void PaletteWorkspaceSetup::setup()
{
    if (!workspacesDataProvider()) {
        return;
    }

    paletteProvider()->setDefaultPaletteTree(PaletteCreator(iocContext()).newDefaultPaletteTree());

    auto saveData = [this]() {
        PaletteTreePtr tree = paletteProvider()->userPaletteTree();

        QByteArray newData;
        writePalette(tree, newData);

        workspacesDataProvider()->setRawData(WS_Palettes, newData);
    };

    paletteProvider()->userPaletteTreeChanged().onNotify(this, saveData);

    auto loadData = [this, saveData]() {
        RetVal<QByteArray> data = workspacesDataProvider()->rawData(WS_Palettes);
        PaletteTreePtr tree;
        if (data.ret && !data.val.isEmpty()) {
            LOGD() << "there is palette data in the workspace, we will use it";
            ByteArray ba = ByteArray::fromQByteArrayNoCopy(data.val);
            tree = readPalette(ba, iocContext());
        } else {
            LOGD() << "no palette data in workspace, will use default";
            tree = PaletteCreator(iocContext()).newDefaultPaletteTree();
        }
        paletteProvider()->setUserPaletteTree(tree);

        if (engraving::gpaletteScore->mscVersion() < engraving::Constants::MSC_VERSION) {
            LOGD() << "Workspace file found with palette file version " << engraving::gpaletteScore->mscVersion() <<
                ". Migrating palette file to " << engraving::Constants::MSC_VERSION;
            saveData();
        }
    };

    workspacesDataProvider()->workspaceChanged().onNotify(this, loadData);

    loadData();
}
