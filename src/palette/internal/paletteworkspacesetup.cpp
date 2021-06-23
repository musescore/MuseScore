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

static const QString PALETTE_XML_TAG("PaletteBox");
static const std::string PALETTE_DATA_NAME("palette");

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
    if (!workspaceManager()) {
        return;
    }

    Ms::PaletteWorkspace* paletteWorkspace = adapter()->paletteWorkspace();
    auto updateWorkspaceConnection = std::make_shared<QMetaObject::Connection>();

    auto applyWorkspaceData = [paletteWorkspace, updateWorkspaceConnection](workspace::IWorkspacePtr workspace) {
        RetVal<QByteArray> data = workspace->readRawData(PALETTE_DATA_NAME);
        if (!data.ret) {
            LOGW() << "no palette data in workspace: " << workspace->name();
            return false;
        }

        Ms::PaletteTreePtr tree = readPalette(data.val);

        paletteWorkspace->setDefaultPaletteTree(tree);
        paletteWorkspace->setUserPaletteTree(tree);

        if (updateWorkspaceConnection) {
            QObject::disconnect(*updateWorkspaceConnection);
        }

        auto newConnection = QObject::connect(paletteWorkspace, &Ms::PaletteWorkspace::userPaletteChanged, [workspace, tree]() {
            QByteArray newData;
            writePalette(tree, newData);
            workspace->writeRawData(PALETTE_DATA_NAME, newData);
        });

        *updateWorkspaceConnection = newConnection;

        return true;
    };

    RetValCh<workspace::IWorkspacePtr> workspace = workspaceManager()->currentWorkspace();
    if (workspace.ret) {
        bool ok = applyWorkspaceData(workspace.val);
        if (!ok) {
            Ms::PaletteTreePtr tree(Ms::PaletteCreator::newDefaultPaletteTree());
            paletteWorkspace->setUserPaletteTree(tree);
        }
    }

    workspace.ch.onReceive(nullptr, [paletteWorkspace, applyWorkspaceData](workspace::IWorkspacePtr w) {
        bool ok = applyWorkspaceData(w);
        if (!ok) {
            Ms::PaletteTreePtr tree(Ms::PaletteCreator::newDefaultPaletteTree());
            paletteWorkspace->setUserPaletteTree(tree);
        }
    });
}
