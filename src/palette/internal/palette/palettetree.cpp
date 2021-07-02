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

#include <QBuffer>
#include <QAction>
#include <QMetaEnum>

#include "palette.h"
#include "palettetree.h"

#include "actions/actiontypes.h"

#include "libmscore/articulation.h"
#include "libmscore/fret.h"
#include "libmscore/icon.h"
#include "libmscore/image.h"
#include "libmscore/imageStore.h"
#include "libmscore/mscore.h"
#include "libmscore/score.h"
#include "libmscore/textbase.h"
#include "libmscore/element.h"
#include "libmscore/bracket.h"

#include "engraving/draw/qpainterprovider.h"

#include "thirdparty/qzip/qzipreader_p.h"
#include "thirdparty/qzip/qzipwriter_p.h"

#include "modularity/ioc.h"

#include "translation.h"

#include "../palette_config.h"
#include "draw/pen.h"

using namespace mu;
using namespace mu::draw;
using namespace mu::palette;
using namespace mu::framework;
using namespace mu::actions;

namespace Ms {
//---------------------------------------------------------
//   needsStaff
//    should a staff been drawn if e is used as icon in
//    a palette
//---------------------------------------------------------

static bool needsStaff(Element* e)
{
    if (!e) {
        return false;
    }
    switch (e->type()) {
    case ElementType::CHORD:
    case ElementType::BAR_LINE:
    case ElementType::CLEF:
    case ElementType::KEYSIG:
    case ElementType::TIMESIG:
    case ElementType::REST:
    case ElementType::BAGPIPE_EMBELLISHMENT:
        return true;
    default:
        return false;
    }
}

//---------------------------------------------------------
//   mimeData
//---------------------------------------------------------

template<class T>
static QByteArray mimeData(T* t)
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    XmlWriter xml(/* score */ nullptr, &buffer);
    xml.setClipboardmode(true);
    t->write(xml);
    buffer.close();
    return buffer.buffer();
}

//---------------------------------------------------------
//   readMimeData
//---------------------------------------------------------

template<class T>
static std::shared_ptr<T> readMimeData(const QByteArray& data, const QString& tagName)
{
    XmlReader e(data);
    e.setPasteMode(true);
    while (e.readNextStartElement()) {
        const QStringRef tag(e.name());
        if (tag == tagName) {
            std::shared_ptr<T> t(new T);
            if (!t->read(e)) {
                return nullptr;
            }
            return t;
        } else {
            return nullptr;
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   PaletteCell::PaletteCell
//---------------------------------------------------------
PaletteCell::PaletteCell(ElementPtr e, const QString& _name, qreal _mag)
    : element(e), name(_name), mag(_mag)
{
    id = makeId();

    drawStaff = needsStaff(element.get());
}

QString PaletteCell::makeId()
{
    static int id = 0;
    return QString::number(++id);
}

//---------------------------------------------------------
//   PaletteCell::translationContext
//---------------------------------------------------------

const char* PaletteCell::translationContext() const
{
    const ElementType type = element ? element->type() : ElementType::INVALID;
    switch (type) {
    case ElementType::ACCIDENTAL:
    case ElementType::ARTICULATION:
    case ElementType::BAR_LINE:
    case ElementType::BREATH:
    case ElementType::FERMATA:
    case ElementType::SYMBOL:
        return "symUserNames";           // libmscore/sym.cpp, Sym::symUserNames
    case ElementType::CLEF:
        return "clefTable";           // libmscore/clef.cpp, ClefInfo::clefTable[]
    case ElementType::KEYSIG:
        return "MuseScore";           // libmscore/keysig.cpp, keyNames[]
    case ElementType::MARKER:
        return "markerType";           // libmscore/marker.cpp, markerTypeTable[]
    case ElementType::JUMP:
        return "jumpType";           // libmscore/jump.cpp, jumpTypeTable[]
    case ElementType::TREMOLO:
        return "Tremolo";           // libmscore/tremolo.cpp, tremoloName[]
    case ElementType::BAGPIPE_EMBELLISHMENT:
        return "bagpipe";           // libmscore/bagpembell.cpp, BagpipeEmbellishment::BagpipeEmbellishmentList[]
    case ElementType::TRILL:
        return "trillType";           // libmscore/trill.cpp, trillTable[]
    case ElementType::VIBRATO:
        return "vibratoType";           // libmscore/vibrato.cpp, vibratoTable[]
    case ElementType::CHORDLINE:
        return "Ms";           // libmscore/chordline.cpp, scorelineNames[]
    case ElementType::NOTEHEAD:
        return "noteheadnames";           // libmscore/note.cpp, noteHeadGroupNames[]
    case ElementType::ICON:
        return "action";           // mscore/shortcut.cpp, Shortcut::_sc[]
    default:
        break;
    }
    return "palette";
}

//---------------------------------------------------------
//   PaletteCell::translatedName
//---------------------------------------------------------

QString PaletteCell::translatedName() const
{
    const QString trName = mu::qtrc(translationContext(), name.toUtf8());

    if (element && element->isTextBase() && name.contains("%1")) {
        return trName.arg(toTextBase(element.get())->plainText());
    }
    return trName;
}

//---------------------------------------------------------
//   PaletteCell::retranslate
///   Retranslates cell content, e.g. text if the element
///   is TextBase.
//---------------------------------------------------------

void PaletteCell::retranslate()
{
    if (untranslatedElement && element->isTextBase()) {
        TextBase* target = toTextBase(element.get());
        TextBase* orig = toTextBase(untranslatedElement.get());
        const QString& text = orig->xmlText();
        target->setXmlText(mu::qtrc("palette", text.toUtf8().constData()));
    }
}

//---------------------------------------------------------
//   PaletteCell::setElementTranslated
//---------------------------------------------------------

void PaletteCell::setElementTranslated(bool translate)
{
    if (translate && element) {
        untranslatedElement = element;
        element.reset(untranslatedElement->clone());
        retranslate();
    } else {
        untranslatedElement.reset();
    }
}

//---------------------------------------------------------
//   PaletteCell::write
//---------------------------------------------------------

void PaletteCell::write(XmlWriter& xml) const
{
    if (!element) {
        xml.tagE("Cell");
        return;
    }

    // using attributes for `custom` and `visible`
    // properties instead of nested tags for pre-3.3
    // version compatibility
    xml.stag(QString("Cell")
             + (!name.isEmpty() ? " name=\"" + XmlWriter::xmlString(name) + "\"" : "")
             + (custom ? " custom=\"1\"" : "")
             + (!visible ? " visible=\"0\"" : "")
             + (untranslatedElement ? " trElement=\"1\"" : "")
             );

    if (drawStaff) {
        xml.tag("staff", drawStaff);
    }
    if (xoffset) {
        xml.tag("xoffset", xoffset);
    }
    if (yoffset) {
        xml.tag("yoffset", yoffset);
    }
    if (!tag.isEmpty()) {
        xml.tag("tag", tag);
    }
    if (mag != 1.0) {
        xml.tag("mag", mag);
    }

    if (untranslatedElement) {
        untranslatedElement->write(xml);
    } else {
        element->write(xml);
    }
    xml.etag();
}

//---------------------------------------------------------
//   PaletteCell::read
//---------------------------------------------------------

bool PaletteCell::read(XmlReader& e)
{
    bool add = true;
    name = e.attribute("name");

    // using attributes instead of nested tags for
    // pre-3.3 version compatibility
    custom = e.hasAttribute("custom") ? e.intAttribute("custom") : false;   // TODO: actually check master palette?
    visible = e.hasAttribute("visible") ? e.intAttribute("visible") : true;

    const bool translateElement = e.hasAttribute("trElement") ? e.intAttribute("trElement") : false;

    while (e.readNextStartElement()) {
        const QStringRef& s(e.name());
        if (s == "staff") {
            drawStaff = e.readInt();
        } else if (s == "xoffset") {
            xoffset = e.readDouble();
        } else if (s == "yoffset") {
            yoffset = e.readDouble();
        } else if (s == "mag") {
            mag = e.readDouble();
        } else if (s == "tag") {
            tag = e.readElementText();
        }
        // added on palettes rework
        // TODO: remove or leave to switch from using attributes later?
        else if (s == "custom") {
            custom = e.readBool();
        } else if (s == "visible") {
            visible = e.readBool();
        } else {
            element.reset(Element::name2Element(s, gscore));
            if (!element) {
                e.unknown();
            } else {
                element->read(e);
                element->styleChanged();

                if (element->type() == ElementType::ICON) {
                    Icon* icon = static_cast<Icon*>(element.get());
                    const mu::ui::UiAction& action = adapter()->getAction(icon->actionCode());
                    if (action.isValid()) {
                        icon->setAction(icon->actionCode(), static_cast<char16_t>(action.iconCode));
                    } else {
                        add = false;
                    }
                }
            }
        }
    }

    setElementTranslated(translateElement);

    return add && element;
}

//---------------------------------------------------------
//   PaletteCell::readMimeData
//---------------------------------------------------------

PaletteCellPtr PaletteCell::readMimeData(const QByteArray& data)
{
    return Ms::readMimeData<PaletteCell>(data, "Cell");
}

//---------------------------------------------------------
//   PaletteCell::readMimeData
//---------------------------------------------------------

PaletteCellPtr PaletteCell::readElementMimeData(const QByteArray& data)
{
    PointF dragOffset;
    Fraction duration(1, 4);
    ElementPtr element(Element::readMimeData(gscore, data, &dragOffset, &duration));

    if (!element) {
        return nullptr;
    }

    if (!element->isSymbol()) { // not sure this check is necessary, it was so in the old palette
        element->setTrack(0);
    }

    if (element->isIcon()) {
        Icon* icon = toIcon(element.get());
        const mu::ui::UiAction& action = adapter()->getAction(icon->actionCode());
        if (action.isValid()) {
            icon->setAction(icon->actionCode(), static_cast<char16_t>(action.iconCode));
        }
    }

    const QString name = (element->isFretDiagram()) ? toFretDiagram(element.get())->harmonyText() : element->userName();

    return std::make_shared<PaletteCell>(element, name);
}

//---------------------------------------------------------
//   PaletteCell::mimeData
//---------------------------------------------------------

QByteArray PaletteCell::mimeData() const
{
    return Ms::mimeData(this);
}

//---------------------------------------------------------
//   PaletteCell::readMimeData
//---------------------------------------------------------

PalettePanelPtr PalettePanel::readMimeData(const QByteArray& data)
{
    return Ms::readMimeData<PalettePanel>(data, "Palette");
}

PalettePanel::PalettePanel(Type t)
    : _type(t)
{
    static int id = 0;
    _id = QString::number(++id);
}

QString PalettePanel::id() const
{
    return _id;
}

QString PalettePanel::translatedName() const
{
    return mu::qtrc("palette", _name.toUtf8());
}

//---------------------------------------------------------
//   PalettePanel::read
//---------------------------------------------------------

bool PalettePanel::read(XmlReader& e)
{
    _name = e.attribute("name");
    _type = Type::Unknown;
    while (e.readNextStartElement()) {
        const QStringRef tag(e.name());
        if (tag == "gridWidth") {
            _gridSize.setWidth(e.readDouble());
        } else if (tag == "gridHeight") {
            _gridSize.setHeight(e.readDouble());
        } else if (tag == "mag") {
            _mag = e.readDouble();
        } else if (tag == "grid") {
            _drawGrid = e.readInt();
        } else if (tag == "moreElements") {
            setMoreElements(e.readInt());
        } else if (tag == "yoffset") {
            _yOffset = e.readDouble();
        } else if (tag == "drumPalette") {      // obsolete
            e.skipCurrentElement();
        } else if (tag == "type") {
            bool ok = true;
            const int t = QMetaEnum::fromType<Type>().keyToValue(e.readElementText().toLatin1().constData(), &ok);
            if (ok) {
                _type = Type(t);
            }
        } else if (tag == "visible") {
            _visible = e.readBool();
        } else if (e.pasteMode() && tag == "expanded") {
            _expanded = e.readBool();
        } else if (tag == "editable") {
            _editable = e.readBool();
        } else if (tag == "Cell") {
            PaletteCellPtr cell = std::make_shared<PaletteCell>();
            cell->id = PaletteCell::makeId();
            if (!cell->read(e)) {
                continue;
            }

            auto cellHandler = cellHandlerByPaletteType(_type);
            if (cellHandler) {
                cellHandler(cell);
            }

            cells.push_back(cell);
        } else {
            e.unknown();
        }
    }
    // (from old palette): make sure hgrid and vgrid are not 0, we divide by them later
    if (_gridSize.width() <= 0) {
        _gridSize.setWidth(28);
    }
    if (_gridSize.width() <= 0) {
        _gridSize.setHeight(28);
    }

    if (_type == Type::Unknown) {
        _type = guessType();
    }

    return true;
}

//---------------------------------------------------------
//   PalettePanel::mimeData
//---------------------------------------------------------

QByteArray PalettePanel::mimeData() const
{
    return Ms::mimeData(this);
}

//---------------------------------------------------------
//   PalettePanel::write
//---------------------------------------------------------

void PalettePanel::write(XmlWriter& xml) const
{
    xml.stag(QString("Palette name=\"%1\"").arg(XmlWriter::xmlString(_name)));
    xml.tag("type", QMetaEnum::fromType<Type>().valueToKey(int(_type)));
    xml.tag("gridWidth", _gridSize.width());
    xml.tag("gridHeight", _gridSize.height());
    xml.tag("mag", _mag);
    if (_drawGrid) {
        xml.tag("grid", _drawGrid);
    }

    xml.tag("moreElements", _moreElements);
    if (_yOffset != 0.0) {
        xml.tag("yoffset", _yOffset);
    }

    xml.tag("visible", _visible, true);
    xml.tag("editable", _editable, true);

    if (xml.clipboardmode()) {
        xml.tag("expanded", _expanded, false);
    }

    for (auto& cell: cells) {
//             if (cells[i] && cells[i]->tag == "ShowMore")
//                   continue;
        if (!cell) {     // from old palette, not sure if it is still needed
            xml.tagE("Cell");
            continue;
        }
        cell->write(xml);
    }
    xml.etag();
}

//---------------------------------------------------------
//   writePaletteFailed
//---------------------------------------------------------

void PalettePanel::showWritingPaletteError(const QString& path) const
{
    std::string title = mu::trc("palette", "Writing Palette File");
    std::string message = mu::qtrc("palette", "Writing Palette File\n%1\nfailed: ").arg(path).toStdString();
    interactive()->error(title, message);
}

//---------------------------------------------------------
//   PalettePanel::writeToFile
///   write as compressed zip file and include
///   images as needed
//---------------------------------------------------------

bool PalettePanel::writeToFile(const QString& p) const
{
    QSet<ImageStoreItem*> images;
    size_t n = cells.size();
    for (size_t i = 0; i < n; ++i) {
        if (cells[i] == 0 || cells[i]->element == 0 || cells[i]->element->type() != ElementType::IMAGE) {
            continue;
        }
        images.insert(toImage(cells[i]->element.get())->storeItem());
    }

    QString path(p);
    if (!path.endsWith(".mpal")) {
        path += ".mpal";
    }

    MQZipWriter f(path);
    // f.setCompressionPolicy(QZipWriter::NeverCompress);
    f.setCreationPermissions(
        QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner
        | QFile::ReadUser | QFile::WriteUser | QFile::ExeUser
        | QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup
        | QFile::ReadOther | QFile::WriteOther | QFile::ExeOther);

    if (f.status() != MQZipWriter::NoError) {
        showWritingPaletteError(path);
        return false;
    }
    QBuffer cbuf;
    cbuf.open(QIODevice::ReadWrite);
    XmlWriter xml(gscore, &cbuf);
    xml.header();
    xml.stag("container");
    xml.stag("rootfiles");
    xml.stag(QString("rootfile full-path=\"%1\"").arg(XmlWriter::xmlString("palette.xml")));
    xml.etag();
    foreach (ImageStoreItem* ip, images) {
        QString ipath = QString("Pictures/") + ip->hashName();
        xml.tag("file", ipath);
    }
    xml.etag();
    xml.etag();
    cbuf.seek(0);
    //f.addDirectory("META-INF");
    //f.addDirectory("Pictures");
    f.addFile("META-INF/container.xml", cbuf.data());

    // save images
    foreach (ImageStoreItem* ip, images) {
        QString ipath = QString("Pictures/") + ip->hashName();
        f.addFile(ipath, ip->buffer());
    }
    {
        QBuffer cbuf1;
        cbuf1.open(QIODevice::ReadWrite);
        XmlWriter xml1(gscore, &cbuf1);
        xml1.header();
        xml1.stag("museScore version=\"" MSC_VERSION "\"");
        write(xml1);
        xml1.etag();
        cbuf1.close();
        f.addFile("palette.xml", cbuf1.data());
    }
    f.close();
    if (f.status() != MQZipWriter::NoError) {
        showWritingPaletteError(path);
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   PalettePanel::readFromFile
//---------------------------------------------------------

bool PalettePanel::readFromFile(const QString& p)
{
    QString path(p);
    if (!path.endsWith(".mpal")) {
        path += ".mpal";
    }

    MQZipReader f(path);
    if (!f.exists()) {
        qDebug("palette <%s> not found", qPrintable(path));
        return false;
    }
    cells.clear();

    QByteArray ba = f.fileData("META-INF/container.xml");

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
                const QStringRef& tag(e.name());

                if (tag == "rootfile") {
                    if (rootfile.isEmpty()) {
                        rootfile = e.attribute("full-path");
                    }
                    e.readNext();
                } else if (tag == "file") {
                    images.append(e.readElementText());
                } else {
                    e.unknown();
                }
            }
        }
    }
    //
    // load images
    //
    foreach (const QString& s, images) {
        imageStore.add(s, f.fileData(s));
    }

    if (rootfile.isEmpty()) {
        qDebug("can't find rootfile in: %s", qPrintable(path));
        return false;
    }

    ba = f.fileData(rootfile);
    e.clear();
    e.addData(ba);
    while (e.readNextStartElement()) {
        if (e.name() == "museScore") {
            QString version = e.attribute("version");
            QStringList sl = version.split('.');
            int versionId = sl[0].toInt() * 100 + sl[1].toInt();
            gscore->setMscVersion(versionId);       // TODO: what is this?

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

//---------------------------------------------------------
//   PalettePanel::insert
//---------------------------------------------------------

PaletteCellPtr PalettePanel::insert(int idx, ElementPtr element, const QString& name, qreal mag)
{
    if (element) {
        element->layout(); // layout may be important for comparing cells, e.g. filtering "More" popup content
    }

    PaletteCellPtr cell = std::make_shared<PaletteCell>(element, name, mag);

    auto cellHandler = cellHandlerByPaletteType(_type);
    if (cellHandler) {
        cellHandler(cell);
    }

    cells.emplace(cells.begin() + idx, cell);
    return cell;
}

//---------------------------------------------------------
//   PalettePanel::append
//---------------------------------------------------------

PaletteCellPtr PalettePanel::append(ElementPtr element, const QString& name, qreal mag)
{
    if (element) {
        element->layout(); // layout may be important for comparing cells, e.g. filtering "More" popup content
    }

    PaletteCellPtr cell = std::make_shared<PaletteCell>(element, name, mag);

    auto cellHandler = cellHandlerByPaletteType(_type);
    if (cellHandler) {
        cellHandler(cell);
    }

    cells.emplace_back(cell);
    return cell;
}

//---------------------------------------------------------
//   PalettePanel::takeCells
//---------------------------------------------------------

std::vector<PaletteCellPtr> PalettePanel::takeCells(int idx, int count)
{
    std::vector<PaletteCellPtr> removedCells;
    removedCells.reserve(count);

    if (idx < 0 || idx + count > int(cells.size())) {
        return removedCells;
    }

    auto removeBegin = cells.begin() + idx;
    auto removeEnd = removeBegin + count;

    removedCells.insert(removedCells.end(), std::make_move_iterator(removeBegin), std::make_move_iterator(removeEnd));
    cells.erase(removeBegin, removeEnd);

    return removedCells;
}

//---------------------------------------------------------
//   PalettePanel::insertCells
//---------------------------------------------------------

bool PalettePanel::insertCells(int idx, std::vector<PaletteCellPtr> insertedCells)
{
    if (idx < 0 || idx > int(cells.size())) {
        return false;
    }

    cells.insert(cells.begin() + idx, std::make_move_iterator(insertedCells.begin()),
                 std::make_move_iterator(insertedCells.end()));

    return true;
}

//---------------------------------------------------------
//   PalettePanel::insertCell
//---------------------------------------------------------

bool PalettePanel::insertCell(int idx, PaletteCellPtr cell)
{
    if (idx < 0 || idx > int(cells.size())) {
        return false;
    }

    cells.insert(cells.begin() + idx, cell);

    return true;
}

//---------------------------------------------------------
//   PalettePanel::scaledGridSize
//---------------------------------------------------------

QSize PalettePanel::scaledGridSize() const
{
    return gridSize() * Palette::paletteScaling();
}

//---------------------------------------------------------
//   isSame
///   Helper function to compare two Elements
// TODO: make it operator==?
//---------------------------------------------------------

static bool isSame(const Element& e1, const Element& e2)
{
    return e1.type() == e2.type()
           && e1.subtype() == e2.subtype()
           && e1.mimeData(PointF()) == e2.mimeData(PointF());
}

//---------------------------------------------------------
//   PalettePanel::findPaletteCell
//---------------------------------------------------------

int PalettePanel::findPaletteCell(const PaletteCell& cell, bool matchName) const
{
    const Element* el = cell.element.get();
    if (!el) {
        return -1;
    }

    for (int i = 0; i < int(cells.size()); ++i) {
        const PaletteCell& localCell = *cells[i];
        if (matchName && localCell.name != cell.name) {
            continue;
        }
        const Element* exElement = localCell.element.get();
        if (exElement && !isSame(*exElement, *el)) {
            continue;
        }
        if (localCell.tag != cell.tag
            || localCell.drawStaff != cell.drawStaff
            || localCell.xoffset != cell.xoffset
            || localCell.yoffset != cell.yoffset
            || localCell.mag != cell.mag
            || localCell.readOnly != cell.readOnly
            || localCell.visible != cell.visible
            || localCell.custom != cell.custom
            ) {
            continue;
        }
        return i;
    }
    return -1;
}

//---------------------------------------------------------
//   PalettePanel::guessType
//---------------------------------------------------------

PalettePanel::Type PalettePanel::guessType() const
{
    if (cells.empty()) {
        return Type::Custom;
    }

    const Element* e = nullptr;
    for (const auto& c : cells) {
        if (c->element) {
            e = c->element.get();
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
    case ElementType::ICON: {
        const Icon* icon = toIcon(e);
        QString action = QString::fromStdString(icon->actionCode());

        if (action.contains("beam")) {
            return Type::Beam;
        }
        if (action.contains("grace") || action.contains("acciaccatura") || action.contains("appoggiatura")) {
            return Type::GraceNote;
        }
        if (action.contains("frame") || action.contains("box") || action.contains("measure")) {
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

std::function<void(PaletteCellPtr)> PalettePanel::cellHandlerByPaletteType(const PalettePanel::Type& type) const
{
    switch (type) {
    case Type::Bracket: return [](PaletteCellPtr cellPtr) {
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

//---------------------------------------------------------
//   PalettePanel::contentType
///   Returns palette type if it is defined or deduces it
///   from the palette content for custom palettes.
//---------------------------------------------------------

PalettePanel::Type PalettePanel::contentType() const
{
    Type t = type();
    if (t == Type::Unknown || t == Type::Custom) {
        t = guessType();
    }

    if (t == Type::Unknown || t == Type::Custom) {
        return Type::Clef;     // if no type can be deduced, use Clef type by default
    }
    return t;
}

//---------------------------------------------------------
//   PalettePanel::retranslate
//---------------------------------------------------------

void PalettePanel::retranslate()
{
    for (auto& c : cells) {
        c->retranslate();
    }
}

//---------------------------------------------------------
//   PaletteTree::insert
///   PaletteTree takes the ownership over the PalettePanel
//---------------------------------------------------------

void PaletteTree::insert(int idx, PalettePanel* palette)
{
    palettes.emplace(palettes.begin() + idx, palette);
}

//---------------------------------------------------------
//   PaletteTreeModel::append
///   PaletteTree takes the ownership over the PalettePanel
//---------------------------------------------------------

void PaletteTree::append(PalettePanel* palette)
{
    palettes.emplace_back(palette);
}

//---------------------------------------------------------
//   PaletteTree::write
//---------------------------------------------------------

void PaletteTree::write(XmlWriter& xml) const
{
    xml.stag("PaletteBox");   // for compatibility with old palettes file format

    for (const auto& p : palettes) {
        p->write(xml);
    }
    xml.etag();
}

//---------------------------------------------------------
//   PaletteTree::read
//---------------------------------------------------------

bool PaletteTree::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const QStringRef tag(e.name());
        if (tag == "Palette") {
            PalettePanelPtr p = std::make_shared<PalettePanel>();
            p->read(e);
            palettes.push_back(p);
        } else {
            e.unknown();
        }
    }
    return true;
}

//---------------------------------------------------------
//   PaletteTree::retranslate
//---------------------------------------------------------

void PaletteTree::retranslate()
{
    for (auto& p : palettes) {
        p->retranslate();
    }
}

//---------------------------------------------------------
//   paintIconElement
/// Paint an icon element so that it fills a QRect, preserving
/// aspect ratio, and leaving a small margin around the edges.
//---------------------------------------------------------

static void paintIconElement(mu::draw::Painter& painter, const RectF& rect, Element* e)
{
    Q_ASSERT(e && e->isIcon());
    painter.save();   // so we can restore it after we are done using it

    constexpr int margin = 4.0;
    qreal extent = qMin(rect.height(), rect.width()) - margin;

    Icon* icon = toIcon(e);
    icon->setExtent(extent);

    extent /= 2.0;
    PointF iconCenter(extent, extent);

    painter.translate(rect.center() - iconCenter);   // change coordinates
    icon->draw(&painter);
    painter.restore();   // restore coordinates
}

//---------------------------------------------------------
//   paintScoreElement
/// Paint a non-icon element centered at the origin of the
/// painter's coordinate system. If alignToStaff is true then
/// the element is only centered horizontally; i.e. vertical
/// alignment is unchanged from the default so that item will
/// appear at the correct height on the staff.
//---------------------------------------------------------

void PaletteCellIconEngine::paintScoreElement(mu::draw::Painter& painter, Element* element, qreal spatium, bool alignToStaff) const
{
    Q_ASSERT(element && !element->isIcon());
    painter.save(); // so we can restore painter after we are done using it

    const qreal sizeRatio = spatium / gscore->spatium();
    painter.scale(sizeRatio, sizeRatio);   // scale coordinates so element is drawn at correct size

    element->layout(); // calculate bbox
    PointF origin = element->bbox().center();

    if (alignToStaff) {
        // y = 0 is position of the element's parent.
        // If the parent is the staff (or a segment on the staff) then
        // y = 0 corresponds to the position of the top staff line.
        origin.setY(0.0);
    }

    painter.translate(-1.0 * origin); // shift coordinates so element is drawn at correct position

    element->scanElements(&painter, Palette::paintPaletteElement);
    painter.restore(); // return painter to saved initial state
}

//---------------------------------------------------------
//   paintStaff
/// Paint a 5 line staff centered within a QRect and return the
/// distance from the top of the QRect to the uppermost staff line.
//---------------------------------------------------------

qreal PaletteCellIconEngine::paintStaff(Painter& painter, const RectF& rect, qreal spatium)
{
    painter.save(); // so we can restore painter after we are done using it
    Pen pen(configuration()->elementsColor());
    pen.setWidthF(MScore::defaultStyle().value(Sid::staffLineWidth).toDouble() * spatium);
    painter.setPen(pen);

    constexpr int numStaffLines = 5;
    const qreal staffHeight = spatium * (numStaffLines - 1);
    const qreal topLineDist = rect.center().y() - (staffHeight / 2.0);

    // lines bounded horizontally by edge of target (with small margin)
    constexpr qreal margin = 3.0;
    const qreal x1 = rect.left() + margin;
    const qreal x2 = rect.right() - margin;

    // draw staff lines with middle line centered vertically on target
    qreal y = topLineDist;
    for (int i = 0; i < numStaffLines; ++i) {
        painter.drawLine(LineF(x1, y, x2, y));
        y += spatium;
    }

    painter.restore(); // return painter to saved initial state
    return topLineDist;
}

//---------------------------------------------------------
//   paintBackground
//---------------------------------------------------------

void PaletteCellIconEngine::paintBackground(Painter& painter, const RectF& rect, bool selected, bool current)
{
    QColor c(configuration()->accentColor());
    if (current || selected) {
        c.setAlpha(selected ? 100 : 60);
        painter.fillRect(rect, c);
    }
}

//---------------------------------------------------------
//   paintTag
//---------------------------------------------------------

void PaletteCellIconEngine::paintTag(Painter& painter, const RectF& rect, QString tag)
{
    if (tag.isEmpty()) {
        return;
    }

    painter.save();   // so we can restore it after we are done using it
    painter.setPen(configuration()->elementsColor());
    Font f(painter.font());
    f.setPointSizeF(12.0);
    painter.setFont(f);

    if (tag == "ShowMore") {
        painter.drawText(rect, Qt::AlignCenter, "???");
    } else {
        painter.drawText(rect, Qt::AlignLeft | Qt::AlignTop, tag);
    }

    painter.restore();   // return to saved initial state (undo pen and font changes, etc.)
}

//---------------------------------------------------------
//   PaletteCellIconEngine::paintCell
//---------------------------------------------------------

void PaletteCellIconEngine::paintCell(mu::draw::Painter& painter, const RectF& rect, bool selected, bool current) const
{
    const qreal _yOffset = 0.0;   // TODO

    paintBackground(painter, rect, selected, current);

    if (!_cell) {
        return;
    }

    paintTag(painter, rect, _cell->tag);

    Element* el = _cell->element.get();
    if (!el) {
        return;
    }

    if (el->isIcon()) {
        paintIconElement(painter, rect, el);
        return;     // never draw staff for icon elements
    }

    const bool drawStaff = _cell->drawStaff;
    const qreal spatium = PALETTE_SPATIUM * _extraMag * _cell->mag;

    PointF origin = rect.center();   // draw element at center of cell by default
    painter.translate(0, _yOffset * spatium);   // offset both element and staff

    if (drawStaff) {
        const qreal topLinePos = paintStaff(painter, rect, spatium);     // draw dummy staff lines onto rect.
        origin.setY(topLinePos);     // vertical position relative to staff instead of cell center.
    }

    painter.translate(origin);
    painter.translate(_cell->xoffset * spatium, _cell->yoffset * spatium);   // additional offset for element only
    painter.setPen(Pen(configuration()->elementsColor()));

    paintScoreElement(painter, el, spatium, drawStaff);
}

//---------------------------------------------------------
//   PaletteCellIconEngine::paint
//---------------------------------------------------------

void PaletteCellIconEngine::paint(QPainter* qp, const QRect& rect, QIcon::Mode mode, QIcon::State state)
{
    mu::draw::Painter p(qp, "palettecell");
    p.save();   // so we can restore it later
    p.setAntialiasing(true);
    paintCell(p, RectF::fromQRectF(rect), mode == QIcon::Selected, state == QIcon::On);
    p.restore();   // return painter to saved initial state (undo any changes to pen, coordinates, font, etc.)
}
} // namespace Ms
