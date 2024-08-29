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
#include "palettecell.h"
#include "palettecompat.h"

#include "mimedatautils.h"

#include "engraving/dom/actionicon.h"
#include "engraving/dom/engravingitem.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/fret.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/textbase.h"
#include "engraving/dom/tremolosinglechord.h"
#include "engraving/dom/tremolotwochord.h"

#include "engraving/rw/rwregister.h"
#include "engraving/rw/compat/tremolocompat.h"

#include "view/widgets/palettewidget.h"

#include "log.h"
#include "translation.h"

using namespace muse;
using namespace mu::palette;
using namespace mu::engraving;

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

PaletteCell::PaletteCell(ElementPtr e, const QString& _name, qreal _mag, const QPointF& _offset, const QString& _tag, QObject* parent)
    : QObject(parent), element(e), name(_name), mag(_mag), xoffset(_offset.x()), yoffset(_offset.y()), tag(_tag)
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
    // Keep in sync with PaletteCreator and all element types
    const ElementType type = element ? element->type() : ElementType::INVALID;
    switch (type) {
    case ElementType::ACTION_ICON:
        return "action";
    case ElementType::ARPEGGIO:
    case ElementType::CHORDLINE:
    case ElementType::GLISSANDO:
    case ElementType::HARMONY:
    case ElementType::JUMP:
    case ElementType::KEYSIG:
    case ElementType::MARKER:
        return "engraving";
    case ElementType::BAGPIPE_EMBELLISHMENT:
        return "engraving/bagpipeembellishment";
    case ElementType::CLEF:
        return "engraving/cleftype";
    case ElementType::DYNAMIC:
        return "engraving/dynamictype";
    case ElementType::HAIRPIN:
        if (name == u"Dynamic + hairpin") {
            return "palette";
        }
        return "engraving/hairpintype";
    case ElementType::LAYOUT_BREAK:
        return "engraving/layoutbreaktype";
    case ElementType::NOTEHEAD:
        return "engraving/noteheadgroup";
    case ElementType::OTTAVA:
        return "engraving/ottavatype";
    case ElementType::SPACER:
        return "engraving/spacertype";
    case ElementType::ACCIDENTAL:
    case ElementType::ARTICULATION:
    case ElementType::BAR_LINE:
    case ElementType::BREATH:
    case ElementType::FERMATA:
    case ElementType::MEASURE_REPEAT:
    case ElementType::ORNAMENT:
    case ElementType::SYMBOL:
        return "engraving/sym";
    case ElementType::TIMESIG:
        return "engraving/timesig";
    case ElementType::TREMOLO_SINGLECHORD:
    case ElementType::TREMOLO_TWOCHORD:
        return "engraving/tremolotype";
    case ElementType::TRILL:
        return "engraving/trilltype";
    case ElementType::TRIPLET_FEEL:
        return "engraving/tripletfeel";
    case ElementType::VIBRATO:
        return "engraving/vibratotype";
    default:
        break;
    }
    return "palette";
}

QString PaletteCell::translatedName() const
{
    const QString trName = muse::qtrc(translationContext(), name.toUtf8());

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
        target->setXmlText(muse::qtrc("palette", text.toUtf8().constData()));
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

bool PaletteCell::read(XmlReader& e, bool pasteMode)
{
    UNUSED(pasteMode);

    bool add = true;
    name = e.attribute("name");

    // using attributes instead of nested tags for
    // pre-3.3 version compatibility
    custom = e.hasAttribute("custom") ? e.intAttribute("custom") : false; // TODO: actually check master palette?
    visible = e.hasAttribute("visible") ? e.intAttribute("visible") : true;

    const bool translateElement = e.hasAttribute("trElement") ? e.intAttribute("trElement") : false;

    while (e.readNextStartElement()) {
        const AsciiStringView s(e.name());
        if (s == "staff") {
            drawStaff = e.readInt();
        } else if (s == "xoffset") {
            xoffset = e.readDouble();
        } else if (s == "yoffset") {
            yoffset = e.readDouble();
        } else if (s == "mag") {
            mag = e.readDouble();
        } else if (s == "tag") {
            tag = e.readText();
        }
        // added on palettes rework
        // TODO: remove or leave to switch from using attributes later?
        else if (s == "custom") {
            custom = e.readBool();
        } else if (s == "visible") {
            visible = e.readBool();
        } else if (s == "Tremolo") {
            compat::TremoloCompat tc;
            tc.parent = gpaletteScore->dummy()->chord();
            rw::RWRegister::reader()->readTremoloCompat(&tc, e);
            if (tc.single) {
                element.reset(tc.single);
            } else if (tc.two) {
                element.reset(tc.two);
            } else {
                UNREACHABLE;
            }

            if (element) {
                element->styleChanged();
            }
        } else {
            element.reset(Factory::createItemByName(s, gpaletteScore->dummy()));
            if (!element) {
                e.unknown();
            } else {
                rw::RWRegister::reader()->readItem(element.get(), e);
                PaletteCompat::migrateOldPaletteItemIfNeeded(element, gpaletteScore);
                element->styleChanged();

                if (element->type() == ElementType::ACTION_ICON) {
                    ActionIcon* icon = toActionIcon(element.get());
                    const muse::ui::UiAction& action = actionsRegister()->action(icon->actionCode());
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

void PaletteCell::write(XmlWriter& xml, bool pasteMode) const
{
    UNUSED(pasteMode);

    if (!element) {
        xml.tag("Cell");
        return;
    }

    // using attributes for `custom` and `visible` properties instead of nested tags
    // for pre-3.3 version compatibility
    XmlWriter::Attributes cellAttrs;
    if (!name.isEmpty()) {
        cellAttrs.push_back({ "name", name });
    }
    if (custom) {
        cellAttrs.push_back({ "custom", "1" });
    }
    if (!visible) {
        cellAttrs.push_back({ "visible", "0" });
    }

    if (untranslatedElement) {
        cellAttrs.push_back({ "trElement", "1" });
    }

    xml.startElement("Cell", cellAttrs);

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
    if (!RealIsEqual(mag, 1.0)) {
        xml.tag("mag", mag);
    }

    if (untranslatedElement) {
        rw::RWRegister::writer(untranslatedElement->iocContext())->writeItem(untranslatedElement.get(), xml);
    } else {
        rw::RWRegister::writer(element->iocContext())->writeItem(element.get(), xml);
    }
    xml.endElement();
}

PaletteCellPtr PaletteCell::fromMimeData(const QByteArray& data)
{
    return ::fromMimeData<PaletteCell>(data, "Cell");
}

PaletteCellPtr PaletteCell::fromElementMimeData(const QByteArray& data)
{
    PointF dragOffset;
    Fraction duration(1, 4);
    ElementPtr element(EngravingItem::readMimeData(gpaletteScore, ByteArray::fromQByteArrayNoCopy(data), &dragOffset, &duration));

    if (!element) {
        return nullptr;
    }

    if (!element->isSymbol()) { // not sure this check is necessary, it was so in the old palette
        element->setTrack(0);
    }

    if (element->isActionIcon()) {
        ActionIcon* icon = toActionIcon(element.get());
        const muse::ui::UiAction& action = actionsRegister()->action(icon->actionCode());
        if (action.isValid()) {
            icon->setAction(icon->actionCode(), static_cast<char16_t>(action.iconCode));
        }
    }

    const String name = (element->isFretDiagram()) ? toFretDiagram(element.get())->harmonyText() : element->translatedTypeUserName();

    return std::make_shared<PaletteCell>(element, name);
}

QByteArray PaletteCell::toMimeData() const
{
    return ::toMimeData(this);
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
