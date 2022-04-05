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
#include "palettecell.h"

#include "mimedatautils.h"

#include "engraving/rw/xml.h"
#include "engraving/libmscore/actionicon.h"
#include "engraving/libmscore/engravingitem.h"
#include "engraving/libmscore/fret.h"
#include "engraving/libmscore/masterscore.h"
#include "engraving/libmscore/textbase.h"
#include "engraving/libmscore/factory.h"

#include "engraving/accessibility/accessibleitem.h"

#include "view/widgets/palettewidget.h"

#include "log.h"
#include "translation.h"

using namespace mu::palette;
using namespace mu::engraving;
using namespace Ms;

static bool needsStaff(ElementPtr e)
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

PaletteCell::PaletteCell(QObject* parent)
    : QObject(parent)
{
    id = makeId();
}

PaletteCell::PaletteCell(ElementPtr e, const QString& _name, qreal _mag, const QString& _tag, QObject* parent)
    : QObject(parent), element(e), name(_name), mag(_mag), tag(_tag)
{
    id = makeId();
    drawStaff = needsStaff(element);
}

QAccessibleInterface* PaletteCell::accessibleInterface(QObject* object)
{
    PaletteCell* cell = qobject_cast<PaletteCell*>(object);
    IF_ASSERT_FAILED(cell) {
        return nullptr;
    }

    return static_cast<QAccessibleInterface*>(new AccessiblePaletteCellInterface(cell));
}

QString PaletteCell::makeId()
{
    static unsigned int id = 0;
    return QString::number(++id);
}

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
        // libmscore/sym.cpp, Sym::symUserNames
        return "symUserNames";
    case ElementType::CLEF:
        // libmscore/clef.cpp, ClefInfo::clefTable[]
        return "clefTable";
    case ElementType::KEYSIG:
        // libmscore/keysig.cpp, keyNames[]
        return "MuseScore";
    case ElementType::MARKER:
        // libmscore/marker.cpp, markerTypeTable[]
        return "markerType";
    case ElementType::JUMP:
        // libmscore/jump.cpp, jumpTypeTable[]
        return "jumpType";
    case ElementType::TREMOLO:
        // libmscore/tremolo.cpp, tremoloName[]
        return "Tremolo";
    case ElementType::BAGPIPE_EMBELLISHMENT:
    // libmscore/bagpembell.cpp, BagpipeEmbellishment::BagpipeEmbellishmentList[]
    case ElementType::TRILL:
        // libmscore/trill.cpp, trillTable[]
        return "trillType";
    case ElementType::VIBRATO:
        // libmscore/vibrato.cpp, vibratoTable[]
        return "vibratoType";
    case ElementType::CHORDLINE:
        // libmscore/chordline.cpp, scorelineNames[]
        return "Ms";
    case ElementType::NOTEHEAD:
        // libmscore/note.cpp, noteHeadGroupNames[]
        return "noteheadnames";
    case ElementType::ACTION_ICON:
        // mscore/shortcut.cpp, Shortcut::_sc[]
        return "action";
    default:
        break;
    }
    return "palette";
}

QString PaletteCell::translatedName() const
{
    const QString trName = mu::qtrc(translationContext(), name.toUtf8());

    if (element && element->isTextBase() && name.contains("%1")) {
        return trName.arg(toTextBase(element.get())->plainText());
    }
    return trName;
}

/// Retranslates cell content, e.g. text if the element is TextBase.
void PaletteCell::retranslate()
{
    if (untranslatedElement && element->isTextBase()) {
        TextBase* target = toTextBase(element.get());
        TextBase* orig = toTextBase(untranslatedElement.get());
        const QString& text = orig->xmlText();
        target->setXmlText(mu::qtrc("palette", text.toUtf8().constData()));
    }
}

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

bool PaletteCell::read(XmlReader& e)
{
    bool add = true;
    name = e.attribute("name");

    // using attributes instead of nested tags for
    // pre-3.3 version compatibility
    custom = e.hasAttribute("custom") ? e.intAttribute("custom") : false; // TODO: actually check master palette?
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
            element.reset(Factory::createItemByName(s, gpaletteScore->dummy()));
            if (!element) {
                e.unknown();
            } else {
                element->read(e);
                element->styleChanged();

                if (element->type() == ElementType::ACTION_ICON) {
                    ActionIcon* icon = toActionIcon(element.get());
                    const mu::ui::UiAction& action = actionsRegister()->action(icon->actionCode());
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

void PaletteCell::write(XmlWriter& xml) const
{
    if (!element) {
        xml.tagE("Cell");
        return;
    }

    // using attributes for `custom` and `visible` properties instead of nested tags
    // for pre-3.3 version compatibility
    xml.startObject(QString("Cell")
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
    xml.endObject();
}

PaletteCellPtr PaletteCell::fromMimeData(const QByteArray& data)
{
    return Ms::fromMimeData<PaletteCell>(data, "Cell");
}

PaletteCellPtr PaletteCell::fromElementMimeData(const QByteArray& data)
{
    PointF dragOffset;
    Fraction duration(1, 4);
    ElementPtr element(EngravingItem::readMimeData(gpaletteScore, data, &dragOffset, &duration));

    if (!element) {
        return nullptr;
    }

    if (!element->isSymbol()) { // not sure this check is necessary, it was so in the old palette
        element->setTrack(0);
    }

    if (element->isActionIcon()) {
        ActionIcon* icon = toActionIcon(element.get());
        const mu::ui::UiAction& action = actionsRegister()->action(icon->actionCode());
        if (action.isValid()) {
            icon->setAction(icon->actionCode(), static_cast<char16_t>(action.iconCode));
        }
    }

    const QString name = (element->isFretDiagram()) ? toFretDiagram(element.get())->harmonyText() : element->typeUserName();

    return std::make_shared<PaletteCell>(element, name);
}

QByteArray PaletteCell::toMimeData() const
{
    return Ms::toMimeData(this);
}

AccessiblePaletteCellInterface::AccessiblePaletteCellInterface(PaletteCell* cell)
{
    m_cell = cell;
}

bool AccessiblePaletteCellInterface::isValid() const
{
    return m_cell != nullptr;
}

QObject* AccessiblePaletteCellInterface::object() const
{
    return m_cell;
}

QAccessibleInterface* AccessiblePaletteCellInterface::childAt(int, int) const
{
    return nullptr;
}

QAccessibleInterface* AccessiblePaletteCellInterface::parent() const
{
    auto paletteWidget = parentWidget();
    if (!paletteWidget) {
        return nullptr;
    }

    return QAccessible::queryAccessibleInterface(paletteWidget);
}

QAccessibleInterface* AccessiblePaletteCellInterface::child(int) const
{
    return nullptr;
}

int AccessiblePaletteCellInterface::childCount() const
{
    return 0;
}

int AccessiblePaletteCellInterface::indexOfChild(const QAccessibleInterface*) const
{
    return -1;
}

QString AccessiblePaletteCellInterface::text(QAccessible::Text t) const
{
    switch (t) {
    case QAccessible::Text::Name:
        return m_cell->element->accessibleInfo();
    default:
        break;
    }

    return QString();
}

void AccessiblePaletteCellInterface::setText(QAccessible::Text, const QString&)
{
}

QRect AccessiblePaletteCellInterface::rect() const
{
    return QRect();
}

QAccessible::Role AccessiblePaletteCellInterface::role() const
{
    return QAccessible::ListItem;
}

QAccessible::State AccessiblePaletteCellInterface::state() const
{
    QAccessible::State state;
    state.invisible = false;
    state.invalid = false;
    state.disabled = false;

    state.focusable = true;
    state.focused = m_cell->focused;

    return state;
}

QObject* AccessiblePaletteCellInterface::parentWidget() const
{
    auto palette = m_cell->parent();
    if (!palette) {
        return nullptr;
    }

    return palette->parent();
}
