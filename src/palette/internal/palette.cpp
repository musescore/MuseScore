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
#include "palette.h"

#include <QMetaEnum>

#include "io/buffer.h"
#include "io/file.h"
#include "serialization/zipreader.h"
#include "serialization/zipwriter.h"

#include "translation.h"
#include "mimedatautils.h"

#include "draw/types/geometry.h"

#include "engraving/rw/400/writecontext.h"
#include "engraving/layout/tlayout.h"

#include "engraving/libmscore/actionicon.h"
#include "engraving/libmscore/articulation.h"
#include "engraving/libmscore/bracket.h"
#include "engraving/libmscore/engravingitem.h"
#include "engraving/libmscore/image.h"
#include "engraving/libmscore/imageStore.h"
#include "engraving/libmscore/masterscore.h"

#include "palettecell.h"

#include "log.h"

using namespace mu;
using namespace mu::io;
using namespace mu::palette;
using namespace mu::engraving;

Palette::Palette(Type t, QObject* parent)
    : QObject(parent), m_type(t)
{
    static int id = 0;
    m_id = QString::number(++id);
}

Palette::~Palette()
{
    //! NOTE Prevent double free
    //! 1. Deleting by deleting children of QObject
    //! 2. Deleting by deleting share pointer
    for (PaletteCellPtr& c : m_cells) {
        if (c->parent() == this) {
            c->setParent(nullptr);
        }
    }
}

QString Palette::id() const
{
    return m_id;
}

QString Palette::translatedName() const
{
    return qtrc("palette", m_name.toUtf8());
}

void Palette::retranslate()
{
    for (PaletteCellPtr cell : m_cells) {
        cell->retranslate();
    }
}

/// Returns palette type if it is defined or deduces it from the palette content for
/// custom palettes.
Palette::Type Palette::contentType() const
{
    Type t = type();
    if (t == Type::Unknown || t == Type::Custom) {
        t = guessType();
    }

    if (t == Type::Unknown || t == Type::Custom) {
        return Type::Clef; // if no type can be deduced, use Clef type by default
    }
    return t;
}

PaletteCellPtr Palette::insertElement(size_t idx, ElementPtr element, const QString& name, qreal mag, const QPointF& offset,
                                      const QString& tag)
{
    if (element) {
        // layout may be important for comparing cells, e.g. filtering "More" popup content
        layout::v0::LayoutContext lctx(element->score());
        layout::v0::TLayout::layoutItem(element.get(), lctx);
    }

    PaletteCellPtr cell = std::make_shared<PaletteCell>(element, name, mag, offset, tag, this);

    auto cellHandler = cellHandlerByPaletteType(m_type);
    if (cellHandler) {
        cellHandler(cell);
    }

    m_cells.emplace(m_cells.begin() + idx, cell);
    return cell;
}

PaletteCellPtr Palette::insertElement(size_t idx, ElementPtr element, const TranslatableString& name, qreal mag,
                                      const QPointF& offset, const QString& tag)
{
    return insertElement(idx, element, name.str, mag, offset, tag);
}

PaletteCellPtr Palette::appendElement(ElementPtr element, const QString& name, qreal mag, const QPointF& offset, const QString& tag)
{
    if (element) {
        // layout may be important for comparing cells, e.g. filtering "More" popup content
        layout::v0::LayoutContext ctx(element->score());
        layout::v0::TLayout::layoutItem(element.get(), ctx);
    }

    PaletteCellPtr cell = std::make_shared<PaletteCell>(element, name, mag, offset, tag, this);

    auto cellHandler = cellHandlerByPaletteType(m_type);
    if (cellHandler) {
        cellHandler(cell);
    }

    m_cells.emplace_back(cell);
    return cell;
}

PaletteCellPtr Palette::appendElement(ElementPtr element, const TranslatableString& name, qreal mag, const QPointF& offset,
                                      const QString& tag)
{
    return appendElement(element, name.str, mag, offset, tag);
}

PaletteCellPtr Palette::appendActionIcon(ActionIconType type, actions::ActionCode code)
{
    const ui::UiAction& action = actionsRegister()->action(code);
    QString name = !action.description.isEmpty() ? action.description.qTranslated() : action.title.qTranslatedWithoutMnemonic();
    auto icon = std::make_shared<ActionIcon>(gpaletteScore->dummy());
    icon->setActionType(type);
    icon->setAction(code, static_cast<char16_t>(action.iconCode));

    return appendElement(icon, name);
}

bool Palette::insertCell(size_t idx, PaletteCellPtr cell)
{
    return insertCells(idx, { cell });
}

bool Palette::insertCells(size_t idx, std::vector<PaletteCellPtr> cells)
{
    if (idx > m_cells.size()) {
        return false;
    }

    for (PaletteCellPtr& c : cells) {
        c->setParent(this);
    }

    m_cells.insert(m_cells.begin() + idx, std::make_move_iterator(cells.begin()),
                   std::make_move_iterator(cells.end()));

    return true;
}

PaletteCellPtr Palette::takeCell(size_t idx)
{
    std::vector<PaletteCellPtr> cells = takeCells(idx, 1);
    return !cells.empty() ? cells.front() : nullptr;
}

std::vector<PaletteCellPtr> Palette::takeCells(size_t idx, size_t count)
{
    std::vector<PaletteCellPtr> removedCells;
    removedCells.reserve(count);

    if (idx + count > m_cells.size()) {
        return removedCells;
    }

    auto removeBegin = m_cells.begin() + idx;
    auto removeEnd = removeBegin + count;

    removedCells.insert(removedCells.end(), std::make_move_iterator(removeBegin), std::make_move_iterator(removeEnd));
    m_cells.erase(removeBegin, removeEnd);

    for (PaletteCellPtr& c : removedCells) {
        c->setParent(nullptr);
    }

    return removedCells;
}

// Helper function to compare two Elements
static bool isEquivalent(const EngravingItem& e1, const EngravingItem& e2)
{
    return e1.type() == e2.type()
           && e1.subtype() == e2.subtype()
           && e1.mimeData() == e2.mimeData();
}

int Palette::indexOfCell(const PaletteCell& cell, bool matchName) const
{
    const EngravingItem* el = cell.element.get();
    if (!el) {
        return -1;
    }

    for (size_t i = 0; i < m_cells.size(); ++i) {
        const PaletteCell& localCell = *m_cells[i];
        if (matchName && localCell.name != cell.name) {
            continue;
        }

        const EngravingItem* exElement = localCell.element.get();
        if (exElement && !isEquivalent(*exElement, *el)) {
            continue;
        }

        if (localCell.drawStaff != cell.drawStaff
            || localCell.xoffset != cell.xoffset
            || localCell.yoffset != cell.yoffset
            || localCell.mag != cell.mag
            || localCell.readOnly != cell.readOnly
            || localCell.visible != cell.visible
            || localCell.custom != cell.custom
            ) {
            continue;
        }

        return int(i);
    }

    return -1;
}

QSize Palette::scaledGridSize() const
{
    return gridSize() * configuration()->paletteScaling();
}

bool Palette::read(XmlReader& e)
{
    m_name = e.attribute("name");
    m_type = Type::Unknown;
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "gridWidth") {
            m_gridSize.setWidth(e.readDouble());
        } else if (tag == "gridHeight") {
            m_gridSize.setHeight(e.readDouble());
        } else if (tag == "mag") {
            m_mag = e.readDouble();
        } else if (tag == "grid") {
            m_drawGrid = e.readInt();
        } else if (tag == "yoffset") {
            m_yOffset = e.readDouble();
        } else if (tag == "drumPalette") { // obsolete
            e.skipCurrentElement();
        } else if (tag == "type") {
            ByteArray ba = e.readText().toAscii();
            bool ok = true;
            const int t = QMetaEnum::fromType<Type>().keyToValue(ba.constChar(), &ok);
            if (ok) {
                m_type = Type(t);
            }
        } else if (tag == "visible") {
            m_isVisible = e.readBool();
        } else if (e.context()->pasteMode() && tag == "expanded") {
            m_isExpanded = e.readBool();
        } else if (tag == "editable") {
            m_isEditable = e.readBool();
        } else if (tag == "Cell") {
            PaletteCellPtr cell = std::make_shared<PaletteCell>(this);
            if (!cell->read(e)) {
                continue;
            }

            auto cellHandler = cellHandlerByPaletteType(m_type);
            if (cellHandler) {
                cellHandler(cell);
            }

            m_cells.push_back(cell);
        } else {
            e.unknown();
        }
    }
    // (from old palette): make sure hgrid and vgrid are not 0, we divide by them later
    if (m_gridSize.width() <= 0) {
        m_gridSize.setWidth(28);
    }
    if (m_gridSize.height() <= 0) {
        m_gridSize.setHeight(28);
    }

    if (m_type == Type::Unknown) {
        m_type = guessType();
    }

    return true;
}

QByteArray Palette::toMimeData() const
{
    return ::toMimeData(this);
}

void Palette::write(XmlWriter& xml) const
{
    xml.startElement("Palette", { { "name", m_name } });
    xml.tag("type", QMetaEnum::fromType<Type>().valueToKey(int(m_type)));
    xml.tag("gridWidth", m_gridSize.width());
    xml.tag("gridHeight", m_gridSize.height());
    xml.tag("mag", m_mag);
    if (m_drawGrid) {
        xml.tag("grid", m_drawGrid);
    }

    if (m_yOffset != 0.0) {
        xml.tag("yoffset", m_yOffset);
    }

    xml.tag("visible", m_isVisible, true);
    xml.tag("editable", m_isEditable, true);

    if (xml.context()->clipboardmode()) {
        xml.tag("expanded", m_isExpanded, false);
    }

    for (PaletteCellPtr cell : m_cells) {
        if (!cell) { // from old palette, not sure if it is still needed
            xml.tag("Cell");
            continue;
        }
        cell->write(xml);
    }
    xml.endElement();
}

PalettePtr Palette::fromMimeData(const QByteArray& data)
{
    return ::fromMimeData<Palette>(data, "Palette");
}

bool Palette::readFromFile(const QString& p)
{
    QString path(p);
    if (!path.endsWith(".mpal", Qt::CaseInsensitive)) {
        path += ".mpal";
    }

    ZipReader f(path);
    if (!f.exists()) {
        LOGD("palette <%s> not found", qPrintable(path));
        return false;
    }
    m_cells.clear();

    ByteArray ba = f.fileData("META-INF/container.xml");

    XmlReader e(ba);
    // extract first rootfile
    QString rootfile = "";
    QList<QString> images;
    while (e.readNextStartElement()) {
        if (e.name() != "container") {
            e.unknown();
            break;
        }
        while (e.readNextStartElement()) {
            if (e.name() != "rootfiles") {
                e.unknown();
                break;
            }
            while (e.readNextStartElement()) {
                const AsciiStringView tag(e.name());

                if (tag == "rootfile") {
                    if (rootfile.isEmpty()) {
                        rootfile = e.attribute("full-path");
                    }
                    e.readNext();
                } else if (tag == "file") {
                    images.append(e.readText());
                } else {
                    e.unknown();
                }
            }
        }
    }
    //
    // load images
    //
    for (const QString& s : images) {
        imageStore.add(s, f.fileData(s.toStdString()));
    }

    if (rootfile.isEmpty()) {
        LOGD("can't find rootfile in: %s", qPrintable(path));
        return false;
    }

    ba = f.fileData(rootfile.toStdString());
    e.setData(ba);
    while (e.readNextStartElement()) {
        if (e.name() == "museScore") {
            QString version = e.attribute("version");
            QStringList sl = version.split('.');
            int versionId = sl[0].toInt() * 100 + sl[1].toInt();
            gpaletteScore->setMscVersion(versionId); // TODO: what is this?

            while (e.readNextStartElement()) {
                if (e.name() == "Palette") {
                    read(e);
                } else {
                    e.unknown();
                }
            }
        } else {
            e.unknown();
        }
    }
    return true;
}

bool Palette::writeToFile(const QString& p) const
{
    QSet<ImageStoreItem*> images;
    size_t n = m_cells.size();
    for (size_t i = 0; i < n; ++i) {
        if (m_cells[i] == 0 || m_cells[i]->element == 0 || m_cells[i]->element->type() != ElementType::IMAGE) {
            continue;
        }
        images.insert(toImage(m_cells[i]->element.get())->storeItem());
    }

    QString path(p);
    if (!path.endsWith(".mpal", Qt::CaseInsensitive)) {
        path += ".mpal";
    }

    ZipWriter f(path);
    if (f.hasError()) {
        showWritingPaletteError(path);
        return false;
    }

    Buffer cbuf;
    cbuf.open(IODevice::ReadWrite);
    XmlWriter xml(&cbuf);
    xml.startDocument();
    xml.startElement("container");
    xml.startElement("rootfiles");
    xml.startElement("rootfile", { { "full-path", "palette.xml" } });
    xml.endElement();
    foreach (ImageStoreItem* ip, images) {
        QString ipath = QString("Pictures/") + ip->hashName();
        xml.tag("file", ipath);
    }
    xml.endElement();
    xml.endElement();
    cbuf.seek(0);
    //f.addDirectory("META-INF");
    //f.addDirectory("Pictures");
    f.addFile("META-INF/container.xml", cbuf.data());

    // save images
    for (ImageStoreItem* ip : images) {
        QString ipath = QString("Pictures/") + ip->hashName();
        f.addFile(ipath.toStdString(), ip->buffer());
    }
    {
        Buffer cbuf1;
        cbuf1.open(IODevice::ReadWrite);
        XmlWriter xml1(&cbuf1);
        xml1.startDocument();
        xml1.startElement("museScore", { { "version", MSC_VERSION } });
        write(xml1);
        xml1.endElement();
        cbuf1.close();
        f.addFile("palette.xml", cbuf1.data());
    }
    f.close();
    if (f.hasError()) {
        showWritingPaletteError(path);
        return false;
    }

    File::setPermissionsAllowedForAll(path);
    return true;
}

void Palette::showWritingPaletteError(const QString& path) const
{
    std::string title = trc("palette", "Writing Palette file");
    std::string message = qtrc("palette", "Writing Palette file\n%1\nfailed.").arg(path).toStdString();
    interactive()->error(title, message);
}

Palette::Type Palette::guessType() const
{
    if (m_cells.empty()) {
        return Type::Custom;
    }

    const EngravingItem* e = nullptr;
    for (PaletteCellPtr cell : m_cells) {
        if (cell->element) {
            e = cell->element.get();
            break;
        }
    }

    if (!e) {
        return Type::Custom;
    }

    switch (e->type()) {
    case ElementType::CLEF:
        return Type::Clef;
    case ElementType::KEYSIG:
        return Type::KeySig;
    case ElementType::TIMESIG:
        return Type::TimeSig;
    case ElementType::BRACKET:
    case ElementType::BRACKET_ITEM:
        return Type::Bracket;
    case ElementType::ACCIDENTAL:
        return Type::Accidental;
    case ElementType::ARTICULATION:
    case ElementType::BEND:
        return toArticulation(e)->isOrnament() ? Type::Ornament : Type::Articulation;
    case ElementType::FERMATA:
        return Type::Articulation;
    case ElementType::BREATH:
        return Type::Breath;
    case ElementType::NOTEHEAD:
        return Type::NoteHead;
    case ElementType::BAR_LINE:
        return Type::BarLine;
    case ElementType::ARPEGGIO:
    case ElementType::GLISSANDO:
        return Type::Arpeggio;
    case ElementType::TREMOLO:
        return Type::Tremolo;
    case ElementType::TEMPO_TEXT:
        return Type::Tempo;
    case ElementType::DYNAMIC:
        return Type::Dynamic;
    case ElementType::FINGERING:
        return Type::Fingering;
    case ElementType::MARKER:
    case ElementType::JUMP:
    case ElementType::MEASURE_REPEAT:
        return Type::Repeat;
    case ElementType::FRET_DIAGRAM:
        return Type::FretboardDiagram;
    case ElementType::BAGPIPE_EMBELLISHMENT:
        return Type::BagpipeEmbellishment;
    case ElementType::LAYOUT_BREAK:
    case ElementType::SPACER:
        return Type::Layout;
    case ElementType::SYMBOL:
        return Type::Accordion;
    case ElementType::HARP_DIAGRAM:
        return Type::Harp;
    case ElementType::ACTION_ICON: {
        const ActionIcon* action = toActionIcon(e);
        QString actionCode = QString::fromStdString(action->actionCode());

        if (actionCode.contains("beam")) {
            return Type::Beam;
        }
        if (actionCode.contains("grace") || actionCode.contains("acciaccatura") || actionCode.contains("appoggiatura")) {
            return Type::GraceNote;
        }
        if (actionCode.contains("frame") || actionCode.contains("box") || actionCode.contains("measure")) {
            return Type::Layout;
        }
        return Type::Custom;
    }
    default: {
        if (e->isSpanner()) {
            return Type::Line;
        }
        if (e->isTextBase()) {
            return Type::Text;
        }
    }
    }

    return Type::Custom;
}

std::function<void(PaletteCellPtr)> Palette::cellHandlerByPaletteType(const Palette::Type& type) const
{
    switch (type) {
    case Type::Bracket:
        return [](PaletteCellPtr cellPtr) {
            if (!cellPtr || !cellPtr->element || !cellPtr->element.get()->isBracket()) {
                return;
            }

            Bracket* bracket = toBracket(cellPtr->element.get());

            if (bracket->bracketType() == BracketType::BRACE) {
                bracket->setStaffSpan(0, 1);
                cellPtr->mag = 1.2;
            }
        };
    default:
        return nullptr;
    }
}
