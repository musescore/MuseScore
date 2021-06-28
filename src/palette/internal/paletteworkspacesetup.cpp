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
    buf.open(QIODevice::ReadOnly);
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
    buf.open(QIODevice::WriteOnly);
    Ms::XmlWriter writer(nullptr, &buf);
    tree->write(writer);
}

void PaletteWorkspaceSetup::setup()
{
    if (!workspacesDataProvider()) {
        return;
    }

    Ms::PaletteWorkspace* paletteWorkspace = adapter()->paletteWorkspace();
    QObject::connect(paletteWorkspace, &Ms::PaletteWorkspace::userPaletteChanged, [this, paletteWorkspace]() {
        Ms::PaletteTreeModel* treeModel = paletteWorkspace->userPaletteModel();
        Ms::PaletteTreePtr tree = treeModel->paletteTreePtr();

        QByteArray newData;
        writePalette(tree, newData);

        workspacesDataProvider()->setRawData(DataKey::Palettes, newData);
    });

    auto loadData = [this, paletteWorkspace]() {
        RetVal<QByteArray> data = workspacesDataProvider()->rawData(DataKey::Palettes);
        Ms::PaletteTreePtr tree;
        if (data.ret) {
            LOGD() << "there is palette data in the workspace, we will use it";
            tree = readPalette(data.val);
        } else {
            LOGD() << "no palette data in workspace, will use default";
            tree = std::shared_ptr<Ms::PaletteTree>(Ms::PaletteCreator::newDefaultPaletteTree());
        }

        paletteWorkspace->setDefaultPaletteTree(tree);
        paletteWorkspace->setUserPaletteTree(tree);
    };

    workspacesDataProvider()->workspaceChanged().onNotify(this, loadData);

    loadData();
}
