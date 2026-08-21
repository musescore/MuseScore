/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

// Map Encore title/subtitle/author/copyright blocks to a MuseScore title frame.

#include "mappers.h"

#include <QRegularExpression>

#include "engraving/style/style.h"
#include "engraving/dom/box.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/jump.h"
#include "engraving/dom/marker.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/text.h"

using namespace mu::engraving;

namespace mu::iex::enc {
// Translate Encore page tokens (#P, #D, #T) to MuseScore macros ($P, $D, $m).
static String translateHeaderFooterTokens(const String& s)
{
    String out = s;
    out.replace(u"#P", u"$P");
    out.replace(u"#D", u"$D");
    out.replace(u"#T", u"$m");
    return out;
}

void addTitleFrame(MasterScore* score, const EncTitle& titleBlock)
{
    // TITL stores multi-line content as separate slots (subtitle1..2, author1..4, etc.).
    // Join non-empty slots with newline, same as Encore's MusicXML exporter does.
    auto joinSlots = [](const std::vector<QString>& items) -> QString {
        QStringList nonEmpty;
        for (const QString& s : items) {
            if (!s.isEmpty()) {
                nonEmpty.append(s);
            }
        }
        return nonEmpty.join(QChar('\n'));
    };
    // Promote the first non-empty subtitle to title when the title slot is blank (TITL fixed-line layout quirk).
    QString effectiveTitle = titleBlock.title;
    std::vector<QString> subtitleSlots = titleBlock.subtitle;
    if (effectiveTitle.isEmpty()) {
        for (QString& slot : subtitleSlots) {
            if (!slot.isEmpty()) {
                effectiveTitle = slot;
                slot = QString();
                break;
            }
        }
    }

    const QString joinedSubtitle    = joinSlots(subtitleSlots);
    const QString joinedInstruction = joinSlots(titleBlock.instruction);
    const QString joinedAuthor      = joinSlots(titleBlock.author);
    const QString joinedCopyright   = joinSlots(titleBlock.copyright);

    const bool hasSubtitle    = !joinedSubtitle.isEmpty();
    const bool hasInstruction = !joinedInstruction.isEmpty();
    const bool hasAuthor      = !joinedAuthor.isEmpty();
    const bool hasCopyright   = !joinedCopyright.isEmpty();

    if (!effectiveTitle.isEmpty()) {
        score->setMetaTag(u"workTitle", String(effectiveTitle));
    }
    if (hasSubtitle) {
        score->setMetaTag(u"subtitle", String(joinedSubtitle));
    }
    if (hasInstruction) {
        score->setMetaTag(u"lyricist", String(joinedInstruction));
    }
    if (hasAuthor) {
        score->setMetaTag(u"composer", String(joinedAuthor));
    }
    if (hasCopyright) {
        score->setMetaTag(u"copyright", String(joinedCopyright));
    }

    if (effectiveTitle.isEmpty() && !hasSubtitle && !hasAuthor && !hasInstruction) {
        return;
    }

    VBox* vbox = Factory::createTitleVBox(score->dummy()->system());
    vbox->setNext(score->first());
    score->measures()->add(vbox);

    if (!effectiveTitle.isEmpty()) {
        Text* t = Factory::createText(vbox, TextStyleType::TITLE);
        t->setPlainText(String(effectiveTitle));
        vbox->add(t);
    }
    if (hasSubtitle) {
        Text* t = Factory::createText(vbox, TextStyleType::SUBTITLE);
        t->setPlainText(String(joinedSubtitle));
        vbox->add(t);
    }
    if (hasInstruction) {
        Text* t = Factory::createText(vbox, TextStyleType::LYRICIST);
        t->setPlainText(String(joinedInstruction));
        vbox->add(t);
    }
    if (hasAuthor) {
        Text* t = Factory::createText(vbox, TextStyleType::COMPOSER);
        t->setPlainText(String(joinedAuthor));
        vbox->add(t);
    }

    // Header/footer: map alignment to odd/even Sid slots. Same-alignment slots joined with newline.
    auto applyHFGroup = [score](const std::vector<EncHeaderFooter>& items,
                                mu::engraving::Sid sidL,
                                mu::engraving::Sid sidC,
                                mu::engraving::Sid sidR,
                                mu::engraving::Sid sidEvenL,
                                mu::engraving::Sid sidEvenC,
                                mu::engraving::Sid sidEvenR) {
        std::map<EncTextAlign, QStringList> grouped;
        for (const EncHeaderFooter& hf : items) {
            if (hf.text.isEmpty()) {
                continue;
            }
            grouped[hf.align].append(hf.text);
        }
        for (const auto& [align, lines] : grouped) {
            mu::engraving::Sid sid     = sidL;
            mu::engraving::Sid sidEven = sidEvenL;
            if (align == EncTextAlign::CENTER) {
                sid     = sidC;
                sidEven = sidEvenC;
            } else if (align == EncTextAlign::RIGHT) {
                sid     = sidR;
                sidEven = sidEvenR;
            }
            const String text = translateHeaderFooterTokens(
                String(lines.join(QChar('\n'))));
            score->style().set(sid, text);
            score->style().set(sidEven, text);
        }
    };
    applyHFGroup(titleBlock.header,
                 mu::engraving::Sid::oddHeaderL, mu::engraving::Sid::oddHeaderC, mu::engraving::Sid::oddHeaderR,
                 mu::engraving::Sid::evenHeaderL, mu::engraving::Sid::evenHeaderC, mu::engraving::Sid::evenHeaderR);
    applyHFGroup(titleBlock.footer,
                 mu::engraving::Sid::oddFooterL, mu::engraving::Sid::oddFooterC, mu::engraving::Sid::oddFooterR,
                 mu::engraving::Sid::evenFooterL, mu::engraving::Sid::evenFooterC, mu::engraving::Sid::evenFooterR);
}

void addRepeatMark(Score* /*score*/, Measure* measure, EncRepeatType rt)
{
    switch (rt) {
    case EncRepeatType::SEGNO: {
        Marker* m = Factory::createMarker(measure);
        m->setMarkerType(MarkerType::SEGNO);
        m->setTrack(0);
        measure->add(m);
        break;
    }
    case EncRepeatType::CODA1: {
        // CODA1=0x85 is "To Coda" (jump source); CODA2=0x89 is the Coda destination.
        Marker* m = Factory::createMarker(measure);
        m->setMarkerType(MarkerType::TOCODA);
        m->setTrack(0);
        measure->add(m);
        break;
    }
    case EncRepeatType::CODA2: {
        Marker* m = Factory::createMarker(measure);
        m->setMarkerType(MarkerType::CODA);
        m->setTrack(0);
        measure->add(m);
        break;
    }
    case EncRepeatType::FINE: {
        Marker* m = Factory::createMarker(measure);
        m->setMarkerType(MarkerType::FINE);
        m->setTrack(0);
        measure->add(m);
        break;
    }
    case EncRepeatType::DC: {
        Jump* j = Factory::createJump(measure);
        j->setJumpType(JumpType::DC);
        j->setPlayRepeats(true);
        j->setTrack(0);
        measure->add(j);
        break;
    }
    case EncRepeatType::DS: {
        Jump* j = Factory::createJump(measure);
        j->setJumpType(JumpType::DS);
        j->setPlayRepeats(true);
        j->setTrack(0);
        measure->add(j);
        break;
    }
    case EncRepeatType::DCALFINE: {
        Jump* j = Factory::createJump(measure);
        j->setJumpType(JumpType::DC_AL_FINE);
        j->setPlayRepeats(true);
        j->setTrack(0);
        measure->add(j);
        break;
    }
    case EncRepeatType::DSALFINE: {
        Jump* j = Factory::createJump(measure);
        j->setJumpType(JumpType::DS_AL_FINE);
        j->setPlayRepeats(true);
        j->setTrack(0);
        measure->add(j);
        break;
    }
    case EncRepeatType::DCALCODA: {
        Jump* j = Factory::createJump(measure);
        j->setJumpType(JumpType::DC_AL_CODA);
        j->setPlayRepeats(true);
        j->setTrack(0);
        measure->add(j);
        break;
    }
    case EncRepeatType::DSALCODA: {
        Jump* j = Factory::createJump(measure);
        j->setJumpType(JumpType::DS_AL_CODA);
        j->setPlayRepeats(true);
        j->setTrack(0);
        measure->add(j);
        break;
    }
    default:
        break;
    }
}
} // namespace mu::iex::enc
