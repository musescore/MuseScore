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
#include "dynamic.h"

#include "types/translatablestring.h"
#include "types/typesconv.h"
#include "types/symnames.h"

#include "anchors.h"
#include "dynamichairpingroup.h"
#include "expression.h"
#include "hairpin.h"
#include "measure.h"
#include "mscore.h"
#include "score.h"
#include "segment.h"
#include "system.h"
#include "tempo.h"
#include "undo.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//-----------------------------------------------------------------------------
//   Dyn
//    see: http://en.wikipedia.org/wiki/File:Dynamic's_Note_Velocity.svg
//-----------------------------------------------------------------------------

// variant with ligatures, works for both emmentaler and bravura:
constexpr DynamicType P = DynamicType::P;
constexpr DynamicType M = DynamicType::M;
constexpr DynamicType F = DynamicType::F;
constexpr DynamicType S = DynamicType::S;
constexpr DynamicType Z = DynamicType::Z;
constexpr DynamicType R = DynamicType::R;
constexpr DynamicType N = DynamicType::N;
const std::vector<Dyn> Dynamic::DYN_LIST = {
    { DynamicType::OTHER,  -1, 0,   true, {}, "", "" },
    { DynamicType::PPPPPP,  1, 0,   false, { P, P, P, P, P, P }, "pppppp",
      "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>" },
    { DynamicType::PPPPP,   5, 0,   false, { P, P, P, P, P }, "ppppp",
      "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>" },
    { DynamicType::PPPP,    10, 0,  false, { P, P, P, P }, "pppp",
      "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>" },
    { DynamicType::PPP,     16, 0,  false, { P, P, P }, "ppp",
      "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>" },
    { DynamicType::PP,      33, 0,  false, { P, P }, "pp",
      "<sym>dynamicPiano</sym><sym>dynamicPiano</sym>" },
    { DynamicType::P,       49, 0,  false, { P }, "p",
      "<sym>dynamicPiano</sym>" },

    { DynamicType::MP,      64, 0,   false, { M, P }, "mp",
      "<sym>dynamicMezzo</sym><sym>dynamicPiano</sym>" },
    { DynamicType::MF,      80, 0,   false, { M, F }, "mf",
      "<sym>dynamicMezzo</sym><sym>dynamicForte</sym>" },

    { DynamicType::F,       96, 0,   false, { F }, "f",
      "<sym>dynamicForte</sym>" },
    { DynamicType::FF,      112, 0,  false, { F, F }, "ff",
      "<sym>dynamicForte</sym><sym>dynamicForte</sym>" },
    { DynamicType::FFF,     126, 0,  false, { F, F, F }, "fff",
      "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>" },
    { DynamicType::FFFF,    127, 0,  false, { F, F, F, F }, "ffff",
      "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>" },
    { DynamicType::FFFFF,   127, 0,  false, { F, F, F, F, F }, "fffff",
      "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>" },
    { DynamicType::FFFFFF,  127, 0,  false, { F, F, F, F, F, F }, "ffffff",
      "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>" },

    { DynamicType::FP,      96, -47,  true, { F, P }, "fp",
      "<sym>dynamicForte</sym><sym>dynamicPiano</sym>" },
    { DynamicType::PF,      49, 47,   true, { P, F }, "pf",
      "<sym>dynamicPiano</sym><sym>dynamicForte</sym>" },

    { DynamicType::SF,      112, -18, true, { S, F }, "sf",
      "<sym>dynamicSforzando</sym><sym>dynamicForte</sym>" },
    { DynamicType::SFZ,     112, -18, true, { S, F, Z }, "sfz",
      "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>" },
    { DynamicType::SFF,     126, -18, true, { S, F, F }, "sff",
      "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>" },
    { DynamicType::SFFZ,    126, -18, true, { S, F, F, Z }, "sffz",
      "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>" },
    { DynamicType::SFFF,    127, -18, true, { S, F, F, F }, "sfff",
      "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>" },
    { DynamicType::SFFFZ,   127, -18, true, { S, F, F, F, Z }, "sfffz",
      "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>" },
    { DynamicType::SFP,     112, -47, true, { S, F, P }, "sfp",
      "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicPiano</sym>" },
    { DynamicType::SFPP,    112, -79, true, { S, F, P, P }, "sfpp",
      "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>" },

    { DynamicType::RFZ,     112, -18, true, { R, F, Z }, "rfz",
      "<sym>dynamicRinforzando</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>" },
    { DynamicType::RF,      112, -18, true, { R, F }, "rf",
      "<sym>dynamicRinforzando</sym><sym>dynamicForte</sym>" },
    { DynamicType::FZ,      112, -18, true, { F, Z }, "fz",
      "<sym>dynamicForte</sym><sym>dynamicZ</sym>" },

    { DynamicType::M,       96, -16,  true, { M }, "m",
      "<sym>dynamicMezzo</sym>" },
    { DynamicType::R,       112, -18, true, { R }, "r",
      "<sym>dynamicRinforzando</sym>" },
    { DynamicType::S,       112, -18, true, { S }, "s",
      "<sym>dynamicSforzando</sym>" },
    { DynamicType::Z,       80, 0,    true, { Z }, "z",
      "<sym>dynamicZ</sym>" },
    { DynamicType::N,       49, -48,  true, { N }, "n",
      "<sym>dynamicNiente</sym>" }
};

//---------------------------------------------------------
//   dynamicsStyle
//---------------------------------------------------------

static const ElementStyle dynamicsStyle {
    { Sid::dynamicsMinDistance, Pid::MIN_DISTANCE },
    { Sid::avoidBarLines, Pid::AVOID_BARLINES },
    { Sid::dynamicsSize, Pid::DYNAMICS_SIZE },
    { Sid::centerOnNotehead, Pid::CENTER_ON_NOTEHEAD },
};

//---------------------------------------------------------
//   DynamicFragment
//---------------------------------------------------------

DynamicFragment::DynamicFragment(CharFormat charFormat, String dynamicText, bool displayAsExpression)
{
    format = charFormat;
    m_dynamicText = dynamicText;
    calculatePlainText(getDynamics(dynamicText));
    setDisplayAsExpressionText(displayAsExpression);
}

DynamicFragment::DynamicFragment(const DynamicFragment& dynamicFragment)
    : TextFragment(dynamicFragment)
{
    m_displayAsExpressionText = dynamicFragment.m_displayAsExpressionText;
    m_dynamicText = dynamicFragment.m_dynamicText;
    m_plainText = dynamicFragment.m_plainText;
}

DynamicFragment& DynamicFragment::operator =(const DynamicFragment& dynamicFragment)
{
    TextFragment::operator =(dynamicFragment);
    m_dynamicText = dynamicFragment.m_dynamicText;
    m_plainText = dynamicFragment.m_plainText;

    return *this;
}

void DynamicFragment::setText(String newText)
{
    std::vector<DynamicType> dynamics = getDynamics(newText);
    calculateDynamicText(dynamics);
    calculatePlainText(dynamics);
    setDisplayAsExpressionText(m_displayAsExpressionText);
}

std::vector<DynamicType> DynamicFragment::getDynamics(String text)
{
    static std::shared_ptr<IEngravingFontsProvider> provider = muse::modularity::globalIoc()->resolve<IEngravingFontsProvider>("engraving");

    std::vector<DynamicType> dynamics;
    for (char32_t i = 0; i < text.size(); i++) {
        SymId symId = provider->fallbackFont()->fromCode(text[i]);
        DynamicType dt = TConv::dynamicType(symId);
        if (dt == DynamicType::OTHER) {
            return dynamics;
        }
        dynamics.push_back(dt);
    }

    return dynamics;
}

void DynamicFragment::setDisplayAsExpressionText(bool asExpression)
{
    m_displayAsExpressionText = asExpression;
    if (asExpression) {
        TextFragment::setText(m_plainText);
    } else {
        format.setFontFamily(u"ScoreText");
        TextFragment::setText(m_dynamicText);
    }
}

void DynamicFragment::calculateDynamicText(std::vector<DynamicType> dynamics)
{
    static std::shared_ptr<IEngravingFontsProvider> provider = muse::modularity::globalIoc()->resolve<IEngravingFontsProvider>("engraving");

    m_dynamicText.clear();
    for (DynamicType dynamic : dynamics) {
        m_dynamicText += provider->fallbackFont()->symCode(TConv::symId(dynamic));
    }
}

void DynamicFragment::calculatePlainText(std::vector<DynamicType> dynamics)
{
    m_plainText.clear();
    for (DynamicType dynamic : dynamics) {
        for (const Dyn& dyn : Dynamic::dynamicList()) {
            if (dyn.type == dynamic) {
                m_plainText += *dyn.plainText;
                break;
            }
        }
    }
}

String DynamicFragment::toSymbolXml()
{
    static std::shared_ptr<IEngravingFontsProvider> provider = muse::modularity::globalIoc()->resolve<IEngravingFontsProvider>("engraving");

    String xmlText;
    for (size_t i = 0; i < m_dynamicText.size(); i++) {
        SymId symId = provider->fallbackFont()->fromCode(m_dynamicText.at(i).unicode());
        xmlText += u"<sym>" + String::fromAscii(SymNames::nameForSymId(symId).ascii()) + u"</sym>";
    }

    return xmlText;
}

//---------------------------------------------------------
//   Dynamic
//---------------------------------------------------------

Dynamic::Dynamic(Segment* parent)
    : TextBase(ElementType::DYNAMIC, parent, TextStyleType::DYNAMICS, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    m_velocity    = -1;
    m_dynRange    = DynamicRange::PART;
    m_dynamicType = DynamicType::OTHER;
    m_changeInVelocity = 128;
    m_velChangeSpeed = DynamicSpeed::NORMAL;
    initElementStyle(&dynamicsStyle);
}

Dynamic::Dynamic(const Dynamic& d)
    : TextBase(d)
{
    m_dynamicType = d.m_dynamicType;
    m_velocity    = d.m_velocity;
    m_dynRange    = d.m_dynRange;
    m_changeInVelocity = d.m_changeInVelocity;
    m_velChangeSpeed = d.m_velChangeSpeed;
    _avoidBarLines = d._avoidBarLines;
    _dynamicsSize = d._dynamicsSize;
    _centerOnNotehead = d._centerOnNotehead;
    m_anchorToEndOfPrevious = d.m_anchorToEndOfPrevious;
}

//---------------------------------------------------------
//   velocity
//---------------------------------------------------------

int Dynamic::velocity() const
{
    return m_velocity <= 0 ? DYN_LIST[int(dynamicType())].velocity : m_velocity;
}

void Dynamic::setDynRange(DynamicRange range)
{
    m_dynRange = range;

    setVoiceAssignment(dynamicRangeToVoiceAssignment(range));
}

//---------------------------------------------------------
//   changeInVelocity
//---------------------------------------------------------

int Dynamic::changeInVelocity() const
{
    return m_changeInVelocity >= 128 ? DYN_LIST[int(dynamicType())].changeInVelocity : m_changeInVelocity;
}

//---------------------------------------------------------
//   setChangeInVelocity
//---------------------------------------------------------

void Dynamic::setChangeInVelocity(int val)
{
    if (DYN_LIST[int(dynamicType())].changeInVelocity == val) {
        m_changeInVelocity = 128;
    } else {
        m_changeInVelocity = val;
    }
}

//---------------------------------------------------------
//   velocityChangeLength
//    the time over which the velocity change occurs
//---------------------------------------------------------

Fraction Dynamic::velocityChangeLength() const
{
    if (changeInVelocity() == 0) {
        return Fraction::fromTicks(0);
    }

    double ratio = score()->tempomap()->tempo(segment()->tick().ticks()).val / Constants::DEFAULT_TEMPO.val;
    double speedMult;
    switch (velChangeSpeed()) {
    case DynamicSpeed::SLOW:
        speedMult = 1.3;
        break;
    case DynamicSpeed::FAST:
        speedMult = 0.5;
        break;
    case DynamicSpeed::NORMAL:
    default:
        speedMult = 0.8;
        break;
    }

    return Fraction::fromTicks(int(ratio * (speedMult * double(Constants::DIVISION))));
}

//---------------------------------------------------------
//   isVelocityChangeAvailable
//---------------------------------------------------------

bool Dynamic::isVelocityChangeAvailable() const
{
    switch (dynamicType()) {
    case DynamicType::FP:
    case DynamicType::SF:
    case DynamicType::SFZ:
    case DynamicType::SFF:
    case DynamicType::SFFZ:
    case DynamicType::SFFF:
    case DynamicType::SFFFZ:
    case DynamicType::SFP:
    case DynamicType::SFPP:
    case DynamicType::RFZ:
    case DynamicType::RF:
    case DynamicType::FZ:
    case DynamicType::M:
    case DynamicType::R:
    case DynamicType::S:
        return true;
    default:
        return false;
    }
}

double Dynamic::customTextOffset() const
{
    if (!_centerOnNotehead || m_dynamicType == DynamicType::OTHER) {
        return 0.0;
    }

    String referenceString = String::fromUtf8(DYN_LIST[int(m_dynamicType)].text);
    if (xmlText() == referenceString) {
        return 0.0;
    }

    Dynamic referenceDynamic(*this);
    referenceDynamic.setXmlText(referenceString);
    renderer()->layoutItem(toTextBase(&referenceDynamic));

    std::shared_ptr<TextFragment> referenceFragment;
    if (!referenceDynamic.ldata()->blocks.empty()) {
        TextBlock referenceBlock = referenceDynamic.ldata()->blocks.front();
        if (!referenceBlock.fragments().empty()) {
            referenceFragment = referenceDynamic.ldata()->blocks.front().fragments().front();
        }
    }

    const LayoutData* ldata = this->ldata();
    IF_ASSERT_FAILED(ldata) {
        return 0.0;
    }
    for (const TextBlock& block : ldata->blocks) {
        for (const std::shared_ptr<TextFragment>& fragment : block.fragments()) {
            if (fragment->text() == referenceFragment->text()) {
                return fragment->pos.x() - referenceFragment->pos.x();
            }
        }
    }

    return 0.0;
}

bool Dynamic::isEditAllowed(EditData& ed) const
{
    if (ed.editTextualProperties) {
        return TextBase::isEditAllowed(ed);
    }

    static const std::set<KeyboardKey> ARROW_KEYS {
        Key_Left,
        Key_Right,
        Key_Up,
        Key_Down
    };

    return muse::contains(ARROW_KEYS, static_cast<KeyboardKey>(ed.key));
}

//-------------------------------------------------------------------
//   doAutoplace
//
//    Move Dynamic up or down to avoid collisions with other elements.
//-------------------------------------------------------------------

//void Dynamic::doAutoplace()
//{
//    Segment* s = segment();
//    if (!(s && autoplace())) {
//        return;
//    }

//    double minDistance = style().styleS(Sid::dynamicsMinDistance).val() * spatium();
//    RectF r = bbox().translated(pos() + s->pos() + s->measure()->pos());
//    double yOff = offset().y() - propertyDefault(Pid::OFFSET).value<PointF>().y();
//    r.translate(0.0, -yOff);

//    Skyline& sl       = s->measure()->system()->staff(staffIdx())->skyline();
//    SkylineLine sk(!placeAbove());
//    sk.add(r);

//    if (placeAbove()) {
//        double d = sk.minDistance(sl.north());
//        if (d > -minDistance) {
//            movePosY(-(d + minDistance));
//        }
//    } else {
//        double d = sl.south().minDistance(sk);
//        if (d > -minDistance) {
//            movePosY(d + minDistance);
//        }
//    }
//}

//--------------------------------------------------------------------------
//   manageBarlineCollisions
//      If necessary, offset dynamic left/right to clear barline collisions
//--------------------------------------------------------------------------

void Dynamic::manageBarlineCollisions()
{
    if (!_avoidBarLines || score()->nstaves() <= 1 || anchorToEndOfPrevious() || !isStyled(Pid::OFFSET)) {
        return;
    }

    Segment* thisSegment = segment();
    if (!thisSegment) {
        return;
    }

    System* system = measure()->system();
    if (!system) {
        return;
    }

    staff_idx_t barLineStaff = muse::nidx;
    if (placeAbove()) {
        // need to find the barline from the staff above
        // taking into account there could be invisible staves
        if (staffIdx() == 0) {
            return;
        }
        for (int staffIndex = static_cast<int>(staffIdx()) - 1; staffIndex >= 0; --staffIndex) {
            if (system->staff(staffIndex)->show()) {
                barLineStaff = staffIndex;
                break;
            }
        }
    } else {
        barLineStaff = staffIdx();
    }

    if (barLineStaff == muse::nidx) {
        return;
    }

    if (score()->staff(barLineStaff)->barLineSpan() < 1) {
        return; // Barline doesn't extend through staves
    }

    const double minBarLineDistance = 0.25 * spatium();

    // Check barlines to the left
    Segment* leftBarLineSegment = nullptr;
    for (Segment* segment = thisSegment; segment && segment->measure()->system() == system; segment = segment->prev1enabled()) {
        if (segment->segmentType() & SegmentType::BarLineType) {
            leftBarLineSegment = segment;
            break;
        }
    }
    if (leftBarLineSegment) {
        EngravingItem* e = leftBarLineSegment->elementAt(barLineStaff * VOICES);
        if (e) {
            double leftMargin = ldata()->bbox().translated(pagePos() - offset()).left()
                                - e->ldata()->bbox().translated(e->pagePos()).right()
                                - minBarLineDistance;
            if (leftMargin < 0) {
                mutldata()->moveX(-leftMargin);
                return;
            }
        }
    }

    // Check barlines to the right
    Segment* rightBarLineSegment = nullptr;
    for (Segment* segment = thisSegment; segment && segment->measure()->system() == system; segment = segment->next1enabled()) {
        if (segment->segmentType() & SegmentType::BarLineType) {
            rightBarLineSegment = segment;
            break;
        }
    }

    if (rightBarLineSegment) {
        EngravingItem* e = rightBarLineSegment->elementAt(barLineStaff * VOICES);
        if (e) {
            double rightMargin = e->ldata()->bbox().translated(e->pagePos()).left()
                                 - ldata()->bbox().translated(pagePos() - offset()).right()
                                 - minBarLineDistance;
            if (rightMargin < 0) {
                mutldata()->moveX(rightMargin);
                return;
            }
        }
    }
}

//---------------------------------------------------------
//   setDynamicType
//---------------------------------------------------------

void Dynamic::setDynamicType(const String& tag)
{
    std::string utf8Tag = tag.toStdString();
    size_t n = DYN_LIST.size();
    for (size_t i = 0; i < n; ++i) {
        if (TConv::toXml(DynamicType(i)).ascii() == utf8Tag || DYN_LIST[i].text == utf8Tag) {
            setDynamicType(DynamicType(i));
            setXmlText(String::fromUtf8(DYN_LIST[i].text));
            return;
        }
    }
    LOGD("setDynamicType: other <%s>", muPrintable(tag));
    setDynamicType(DynamicType::OTHER);
    setXmlText(tag);
}

bool Dynamic::isDynamicType(String s)
{
    static std::shared_ptr<IEngravingFontsProvider> provider = muse::modularity::globalIoc()->resolve<IEngravingFontsProvider>("engraving");

    for (char32_t i = 0; i < s.size(); i++) {
        SymId symId = provider->fallbackFont()->fromCode(s[i]);
        if (TConv::dynamicType(symId) == DynamicType::OTHER) {
            return false;
        }
    }

    return true;
}

String Dynamic::dynamicText(DynamicType t)
{
    return String::fromStdString(DYN_LIST[int(t)].text);
}

Expression* Dynamic::snappedExpression() const
{
    EngravingItem* item = ldata()->itemSnappedAfter();
    return item && item->isExpression() ? toExpression(item) : nullptr;
}

HairpinSegment* Dynamic::findSnapBeforeHairpinAcrossSystemBreak() const
{
    /* Normally it is the hairpin which looks for a snappable dynamic. Except if this dynamic
     * is on the first beat of next system, in which case it needs to look back for a hairpin. */
    Segment* seg = segment();
    Measure* measure = seg ? seg->measure() : nullptr;
    System* system = measure ? measure->system() : nullptr;
    bool isOnFirstBeatOfSystem = system && system->firstMeasure() == measure && seg->rtick().isZero();
    if (!isOnFirstBeatOfSystem) {
        return nullptr;
    }

    Measure* prevMeasure = measure->prevMeasure();
    System* prevSystem = prevMeasure ? prevMeasure->system() : nullptr;
    if (!prevSystem) {
        return nullptr;
    }

    for (SpannerSegment* spannerSeg : prevSystem->spannerSegments()) {
        if (!spannerSeg->isHairpinSegment() || spannerSeg->track() != track() || spannerSeg->spanner()->tick2() != tick()) {
            continue;
        }
        HairpinSegment* hairpinSeg = toHairpinSegment(spannerSeg);
        if (hairpinSeg->findElementToSnapAfter() == this) {
            return hairpinSeg;
        }
    }

    return nullptr;
}

bool Dynamic::acceptDrop(EditData& ed) const
{
    ElementType droppedType = ed.dropElement->type();
    return droppedType == ElementType::DYNAMIC || droppedType == ElementType::EXPRESSION || droppedType == ElementType::HAIRPIN;
}

EngravingItem* Dynamic::drop(EditData& ed)
{
    EngravingItem* item = ed.dropElement;

    if (item->isHairpin()) {
        score()->addHairpinToDynamic(toHairpin(item), this);
        return item;
    }

    if (item->isDynamic()) {
        Dynamic* dynamic = toDynamic(item);
        undoChangeProperty(Pid::DYNAMIC_TYPE, dynamic->dynamicType());
        undoChangeProperty(Pid::TEXT, dynamic->xmlText());
        delete dynamic;
        ed.dropElement = this;
        return this;
    }

    if (item->isExpression()) {
        item->setTrack(track());
        item->setParent(segment());
        toExpression(item)->setVoiceAssignment(voiceAssignment());
        score()->undoAddElement(item);
        return item;
    }

    return nullptr;
}

int Dynamic::dynamicVelocity(DynamicType t)
{
    return DYN_LIST[int(t)].velocity;
}

TranslatableString Dynamic::subtypeUserName() const
{
    if (dynamicType() == DynamicType::OTHER) {
        String s = plainText().simplified();
        if (s.size() > 20) {
            s.truncate(20);
            s += u"…";
        }
        return TranslatableString::untranslatable(s);
    } else {
        return TConv::userName(dynamicType());
    }
}

void Dynamic::startEdit(EditData& ed)
{
    if (ed.editTextualProperties) {
        if (useExpressionFontFace()) {
            for (TextBlock block : ldata()->blocks) {
                for (std::shared_ptr<TextFragment> fragment : block.fragments()) {
                    if (DynamicFragment* dynamicFragment = dynamic_cast<DynamicFragment*>(fragment.get())) {
                        dynamicFragment->setDisplayAsExpressionText(false);
                    }
                }
            }
        }

        TextBase::startEdit(ed);
    } else {
        startEditNonTextual(ed);
    }
}

bool Dynamic::edit(EditData& ed)
{
    if (ed.editTextualProperties) {
        return TextBase::edit(ed);
    } else {
        return editNonTextual(ed);
    }
}

bool Dynamic::editNonTextual(EditData& ed)
{
    if (ed.key == Key_Shift) {
        if (ed.isKeyRelease) {
            score()->hideAnchors();
        } else {
            EditTimeTickAnchors::updateAnchors(this, track());
        }
        triggerLayout();
        return true;
    }

    if (!isEditAllowed(ed)) {
        return false;
    }

    bool leftRightKey = ed.key == Key_Left || ed.key == Key_Right;
    bool altMod = ed.modifiers & AltModifier;
    bool shiftMod = ed.modifiers & ShiftModifier;

    bool changeAnchorType = shiftMod && altMod && leftRightKey;
    if (changeAnchorType) {
        undoChangeProperty(Pid::ANCHOR_TO_END_OF_PREVIOUS, !anchorToEndOfPrevious(), propertyFlags(Pid::ANCHOR_TO_END_OF_PREVIOUS));
    }
    bool doesntNeedMoveSeg = changeAnchorType && ((ed.key == Key_Left && anchorToEndOfPrevious())
                                                  || (ed.key == Key_Right && !anchorToEndOfPrevious()));
    if (doesntNeedMoveSeg) {
        checkMeasureBoundariesAndMoveIfNeed();
        return true;
    }

    bool moveSeg = shiftMod && (ed.key == Key_Left || ed.key == Key_Right);
    if (moveSeg) {
        bool moved = moveSegment(ed);
        EditTimeTickAnchors::updateAnchors(this, track());
        checkMeasureBoundariesAndMoveIfNeed();
        return moved;
    }

    if (shiftMod) {
        return false;
    }

    if (!nudge(ed)) {
        return false;
    }

    triggerLayout();
    return true;
}

void Dynamic::checkMeasureBoundariesAndMoveIfNeed()
{
    /* Dynamics are always assigned to a ChordRest segment if available at this tick,
     * EXCEPT if we are at a measure boundary. In this case, if anchorToEndOfPrevious()
     * we must assign to a TimeTick segment at the end of previous measure, otherwise to a
     * ChordRest segment at the start of the next measure. */

    Segment* curSeg = segment();
    Fraction curTick = curSeg->tick();
    Measure* curMeasure = curSeg->measure();
    Measure* prevMeasure = curMeasure->prevMeasure();
    bool needMoveToNext = curTick == curMeasure->endTick() && !anchorToEndOfPrevious();
    bool needMoveToPrevious = curSeg->rtick().isZero() && anchorToEndOfPrevious() && prevMeasure;

    if (!needMoveToPrevious && !needMoveToNext) {
        return;
    }

    Segment* newSeg = nullptr;
    if (needMoveToPrevious) {
        newSeg = prevMeasure->findSegment(SegmentType::TimeTick, curSeg->tick());
        if (!newSeg) {
            TimeTickAnchor* anchor = EditTimeTickAnchors::createTimeTickAnchor(prevMeasure, curTick - prevMeasure->tick(), staffIdx());
            EditTimeTickAnchors::updateLayout(prevMeasure);
            newSeg = anchor->segment();
        }
    } else {
        newSeg = curSeg->next1(SegmentType::ChordRest);
    }

    if (newSeg && newSeg->tick() == curTick) {
        score()->undoChangeParent(this, newSeg, staffIdx());
        if (snappedExpression()) {
            score()->undoChangeParent(snappedExpression(), newSeg, staffIdx());
        }
    }
}

bool Dynamic::moveSegment(const EditData& ed)
{
    bool forward = ed.key == Key_Right;
    if (!(ed.modifiers & AltModifier)) {
        if (anchorToEndOfPrevious()) {
            undoResetProperty(Pid::ANCHOR_TO_END_OF_PREVIOUS);
            if (forward) {
                return true;
            }
        }
    }

    Segment* curSeg = segment();
    IF_ASSERT_FAILED(curSeg) {
        return false;
    }

    Segment* newSeg = forward ? curSeg->next1ChordRestOrTimeTick() : curSeg->prev1ChordRestOrTimeTick();
    if (!newSeg) {
        return false;
    }

    undoMoveSegment(newSeg, newSeg->tick() - curSeg->tick());

    return true;
}

void Dynamic::undoMoveSegment(Segment* newSeg, Fraction tickDiff)
{
    score()->undoChangeParent(this, newSeg, staffIdx());
    moveSnappedItems(newSeg, tickDiff);
}

void Dynamic::moveSnappedItems(Segment* newSeg, Fraction tickDiff) const
{
    if (EngravingItem* itemSnappedBefore = ldata()->itemSnappedBefore()) {
        if (itemSnappedBefore->isHairpinSegment()) {
            Hairpin* hairpinBefore = toHairpinSegment(itemSnappedBefore)->hairpin();
            hairpinBefore->undoMoveEnd(tickDiff);
        }
    }

    if (EngravingItem* itemSnappedAfter = ldata()->itemSnappedAfter()) {
        Hairpin* hairpinAfter = itemSnappedAfter->isHairpinSegment() ? toHairpinSegment(itemSnappedAfter)->hairpin() : nullptr;
        if (itemSnappedAfter->isExpression()) {
            Expression* expressionAfter = toExpression(itemSnappedAfter);
            Segment* curExpressionSegment = expressionAfter->segment();
            if (curExpressionSegment != newSeg) {
                score()->undoChangeParent(expressionAfter, newSeg, expressionAfter->staffIdx());
            }
            EngravingItem* possibleHairpinAfterExpr = expressionAfter->ldata()->itemSnappedAfter();
            if (!hairpinAfter && possibleHairpinAfterExpr && possibleHairpinAfterExpr->isHairpinSegment()) {
                hairpinAfter = toHairpinSegment(possibleHairpinAfterExpr)->hairpin();
            }
        }
        if (hairpinAfter) {
            hairpinAfter->undoMoveStart(tickDiff);
        }
    }
}

bool Dynamic::nudge(const EditData& ed)
{
    bool ctrlMod = ed.modifiers & ControlModifier;
    double step = spatium() * (ctrlMod ? MScore::nudgeStep10 : MScore::nudgeStep);
    PointF addOffset = PointF();
    switch (ed.key) {
    case Key_Up:
        addOffset = PointF(0.0, -step);
        break;
    case Key_Down:
        addOffset = PointF(0.0, step);
        break;
    case Key_Left:
        addOffset = PointF(-step, 0.0);
        break;
    case Key_Right:
        addOffset = PointF(step, 0.0);
        break;
    default:
        return false;
    }
    undoChangeProperty(Pid::OFFSET, offset() + addOffset, PropertyFlags::UNSTYLED);
    return true;
}

void Dynamic::editDrag(EditData& ed)
{
    ElementEditDataPtr eed = ed.getData(this);
    if (!eed) {
        return;
    }

    EditTimeTickAnchors::updateAnchors(this, track());

    KeyboardModifiers km = ed.modifiers;
    if (km != (ShiftModifier | ControlModifier)) {
        staff_idx_t si = staffIdx();
        Segment* seg = nullptr; // don't prefer any segment while dragging, just snap to the closest
        static constexpr double spacingFactor = 0.5;
        score()->dragPosition(canvasPos(), &si, &seg, spacingFactor, allowTimeAnchor());
        if ((seg && seg != segment()) || staffIdx() != si) {
            const PointF oldOffset = offset();
            PointF pos1(canvasPos());
            score()->undoChangeParent(this, seg, staffIdx());
            Expression* snappedExpr = snappedExpression();
            if (snappedExpr) {
                score()->undoChangeParent(snappedExpr, seg, staffIdx());
            }
            setOffset(PointF());

            renderer()->layoutItem(this);

            PointF pos2(canvasPos());
            const PointF newOffset = pos1 - pos2;
            setOffset(newOffset);
            setOffsetChanged(true);
            eed->initOffset += newOffset - oldOffset;
        }
    }

    EngravingItem::editDrag(ed);
}

void Dynamic::endEdit(EditData& ed)
{
    if (ed.editTextualProperties) {
        if (useExpressionFontFace()) {
            for (TextBlock block : ldata()->blocks) {
                for (std::shared_ptr<TextFragment> fragment : block.fragments()) {
                    if (DynamicFragment* dynamicFragment = dynamic_cast<DynamicFragment*>(fragment.get())) {
                        dynamicFragment->setDisplayAsExpressionText(true);
                    }
                }
            }
        }
        TextBase::endEdit(ed);
        if (!xmlText().contains(String::fromStdString(DYN_LIST[int(m_dynamicType)].text))) {
            m_dynamicType = DynamicType::OTHER;
        }
    } else {
        endEditNonTextual(ed);
    }
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Dynamic::reset()
{
    undoResetProperty(Pid::DIRECTION);
    undoResetProperty(Pid::CENTER_BETWEEN_STAVES);
    TextBase::reset();
    Expression* snappedExp = snappedExpression();
    if (snappedExp && snappedExp->getProperty(Pid::OFFSET) != snappedExp->propertyDefault(Pid::OFFSET)) {
        snappedExp->reset();
    }
}

//---------------------------------------------------------
//   getDragGroup
//---------------------------------------------------------

std::unique_ptr<ElementGroup> Dynamic::getDragGroup(std::function<bool(const EngravingItem*)> isDragged)
{
    if (auto g = HairpinWithDynamicsDragGroup::detectFor(this, isDragged)) {
        return g;
    }
    if (auto g = DynamicNearHairpinsDragGroup::detectFor(this, isDragged)) {
        return g;
    }
    if (auto g = DynamicExpressionDragGroup::detectFor(this, isDragged)) {
        return g;
    }
    return TextBase::getDragGroup(isDragged);
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Dynamic::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::DYNAMIC_TYPE:
        return m_dynamicType;
    case Pid::VELOCITY:
        return velocity();
    case Pid::SUBTYPE:
        return int(m_dynamicType);
    case Pid::VELO_CHANGE:
        if (isVelocityChangeAvailable()) {
            return changeInVelocity();
        } else {
            return PropertyValue();
        }
    case Pid::VELO_CHANGE_SPEED:
        return m_velChangeSpeed;
    case Pid::AVOID_BARLINES:
        return avoidBarLines();
    case Pid::DYNAMICS_SIZE:
        return _dynamicsSize;
    case Pid::CENTER_ON_NOTEHEAD:
        return _centerOnNotehead;
    case Pid::PLAY:
        return playDynamic();
    case Pid::ANCHOR_TO_END_OF_PREVIOUS:
        return anchorToEndOfPrevious();
    default:
        return TextBase::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Dynamic::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::DYNAMIC_TYPE:
        m_dynamicType = v.value<DynamicType>();
        break;
    case Pid::VELOCITY:
        m_velocity = v.toInt();
        break;
    case Pid::SUBTYPE:
        m_dynamicType = v.value<DynamicType>();
        break;
    case Pid::VELO_CHANGE:
        if (isVelocityChangeAvailable()) {
            setChangeInVelocity(v.toInt());
        }
        break;
    case Pid::VELO_CHANGE_SPEED:
        m_velChangeSpeed = v.value<DynamicSpeed>();
        break;
    case Pid::AVOID_BARLINES:
        setAvoidBarLines(v.toBool());
        break;
    case Pid::DYNAMICS_SIZE:
        _dynamicsSize = v.toDouble();
        break;
    case Pid::CENTER_ON_NOTEHEAD:
        _centerOnNotehead = v.toBool();
        break;
    case Pid::PLAY:
        setPlayDynamic(v.toBool());
        break;
    case Pid::ANCHOR_TO_END_OF_PREVIOUS:
        setAnchorToEndOfPrevious(v.toBool());
        break;
    default:
        if (!TextBase::setProperty(propertyId, v)) {
            return false;
        }
        break;
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Dynamic::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TEXT_STYLE:
        return TextStyleType::DYNAMICS;
    case Pid::VELOCITY:
        return -1;
    case Pid::VELO_CHANGE:
        if (isVelocityChangeAvailable()) {
            return DYN_LIST[int(dynamicType())].changeInVelocity;
        } else {
            return PropertyValue();
        }
    case Pid::VELO_CHANGE_SPEED:
        return DynamicSpeed::NORMAL;
    case Pid::PLAY:
        return true;
    case Pid::ANCHOR_TO_END_OF_PREVIOUS:
        return false;
    default:
        return TextBase::propertyDefault(id);
    }
}

Sid Dynamic::getPropertyStyle(Pid pid) const
{
    switch (pid) {
    case Pid::PLACEMENT:
        return Sid::dynamicsPlacement;
    default:
        return TextBase::getPropertyStyle(pid);
    }
}

void Dynamic::styleChanged()
{
    for (const TextBlock& block : ldata()->blocks) {
        for (const std::shared_ptr<TextFragment>& fragment : block.fragments()) {
            DynamicFragment* dynamicFragment = dynamic_cast<DynamicFragment*>(fragment.get());
            if (dynamicFragment) {
                dynamicFragment->setDisplayAsExpressionText(useExpressionFontFace());
            }
        }
    }

    TextBase::styleChanged();
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Dynamic::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), translatedSubtypeUserName());
}

//---------------------------------------------------------
//   screenReaderInfo
//---------------------------------------------------------

String Dynamic::screenReaderInfo() const
{
    String s;

    if (dynamicType() == DynamicType::OTHER) {
        s = plainText().simplified();
    } else {
        s = TConv::translatedUserName(dynamicType());
    }
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), s);
}

bool Dynamic::useExpressionFontFace() const
{
    return style().styleB(Sid::dynamicsOverrideFont) && style().styleB(Sid::dynamicsUseExpressionFont);
}
}
