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
#include "paletteworkspacesetup.h"

#include <QBuffer>

#include "palette/paletteworkspace.h"
#include "palette/palettecreator.h"

#include "log.h"

using namespace mu::palette;
using namespace mu::workspace;

static const QString PALETTE_XML_TAG("PaletteBox");

static Ms::PaletteTreePtr readPalette(const QByteArray& data)
{
    QBuffer buf;
    buf.setData(data);
    Ms::XmlReader reader(&buf);

    while (!reader.atEnd()) {
        reader.readNextStartElement();

        if (reader.name() == PALETTE_XML_TAG) {
            Ms::PaletteTreePtr tree = std::make_shared<Ms::PaletteTree>();
            tree->read(reader);
            return tree;
        }
    }

    return nullptr;
}

static void writePalette(const Ms::PaletteTreePtr& tree, QByteArray& data)
{
    QBuffer buf(&data);
    Ms::XmlWriter writer(nullptr, &buf);
    tree->write(writer);
}

void PaletteWorkspaceSetup::setup()
{
    if (!workspacesDataProvider()) {
        return;
    }

    Ms::PaletteWorkspace* paletteWorkspace = adapter()->paletteWorkspace();
    auto updateWorkspaceConnection = std::make_shared<QMetaObject::Connection>();

    auto applyWorkspaceData = [this, paletteWorkspace, updateWorkspaceConnection]() {
        RetVal<QByteArray> data = workspacesDataProvider()->rawData(DataKey::Palettes);
        if (!data.ret) {
            LOGD() << "no palette data";
            return false;
        }

        Ms::PaletteTreePtr tree = readPalette(data.val);

        paletteWorkspace->setDefaultPaletteTree(tree);
        paletteWorkspace->setUserPaletteTree(tree);

        if (updateWorkspaceConnection) {
            QObject::disconnect(*updateWorkspaceConnection);
        }

        auto newConnection = QObject::connect(paletteWorkspace, &Ms::PaletteWorkspace::userPaletteChanged, [this, tree]() {
            QByteArray newData;
            writePalette(tree, newData);
            workspacesDataProvider()->setRawData(DataKey::Palettes, newData);
        });

        *updateWorkspaceConnection = newConnection;

        return true;
    };

    bool ok = applyWorkspaceData();
    if (!ok) {
        Ms::PaletteTreePtr tree(Ms::PaletteCreator::newDefaultPaletteTree());
        paletteWorkspace->setUserPaletteTree(tree);
    }

    workspacesDataProvider()->dataChanged(DataKey::Palettes).onNotify(this, [paletteWorkspace, applyWorkspaceData]() {
        bool ok = applyWorkspaceData();
        if (!ok) {
            Ms::PaletteTreePtr tree(Ms::PaletteCreator::newDefaultPaletteTree());
            paletteWorkspace->setUserPaletteTree(tree);
        }
    });
}
