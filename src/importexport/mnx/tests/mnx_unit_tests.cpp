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
#include <gtest/gtest.h>

#include <optional>
#include <string>

#include "engraving/types/typesconv.h"

#include "importexport/mnx/internal/shared/mnxtypesconv.h"

#ifdef MNXDOM_SYSTEM
#include <mnxdom/mnxdom.h>
#else
#include "mnxdom.h"
#endif

using namespace mu::engraving;
using namespace mu::iex::mnxio;
using namespace muse;

//---------------------------------------------------------
//   Synthesized MNX tests
//   Tests whose input is built here rather than read from a fixture file. The file-driven
//   import/export comparisons live in mnx_tests.cpp; keep this translation unit for cases
//   that exercise one behavior directly.
//---------------------------------------------------------

class Mnx_UnitTests : public ::testing::Test
{
};

//---------------------------------------------------------
//   dynamic spelling tests
//   toMnxDynamicFromLetters is how a dynamic MuseScore renders without a music font, and
//   therefore cannot classify, still reaches MNX. No score fixture spells a dynamic that
//   way -- they all carry glyphs -- so exercise the grammar directly.
//---------------------------------------------------------

namespace {
using DynPrefix = mnx::DynamicPrefix;
using DynSuffix = mnx::DynamicSuffix;
using DynValue = mnx::DynamicValue;

MnxDynamicMapping plainDynamic(DynValue value)
{
    MnxDynamicMapping mapping;
    mapping.value = value;
    return mapping;
}

MnxDynamicMapping accentDynamic(DynPrefix prefix, DynValue value, DynSuffix suffix,
                                std::optional<DynValue> residual = std::nullopt)
{
    MnxDynamicMapping mapping;
    mapping.isAccent = true;
    mapping.accentPrefix = prefix;
    mapping.accentSuffix = suffix;
    mapping.value = value;
    mapping.residualValue = residual;
    return mapping;
}

void expectDynamicLetters(const char* letters, const MnxDynamicMapping& expected)
{
    SCOPED_TRACE(letters);
    const auto actual = toMnxDynamicFromLetters(letters);
    ASSERT_TRUE(actual.has_value());
    EXPECT_EQ(actual->value, expected.value);
    EXPECT_EQ(actual->residualValue, expected.residualValue);
    EXPECT_EQ(actual->isAccent, expected.isAccent);
    EXPECT_EQ(actual->accentPrefix, expected.accentPrefix);
    EXPECT_EQ(actual->accentSuffix, expected.accentSuffix);
}
} // namespace

TEST_F(Mnx_UnitTests, dynamicValueSpellingsCarryNoAffixLetters)
{
    // toMnxDynamicFromLetters reads the accent affixes one character at a time, which is only
    // unambiguous because no DynamicValue spells itself with s, r, or z. That is a property of
    // mnxdom, not of this code, so assert it here rather than let a schema change silently
    // turn "sf" into a value lookup that swallows the prefix.
    for (const auto& [value, spelling] : mnx::EnumStringMapping<mnx::DynamicValue>::enumToString()) {
        SCOPED_TRACE(spelling);
        EXPECT_FALSE(spelling.empty());
        for (const char ch : spelling) {
            EXPECT_TRUE(ch == 'p' || ch == 'm' || ch == 'f' || ch == 'n')
                << "DynamicValue \"" << spelling << "\" contains '" << ch
                << "', which the letters grammar treats as an accent affix.";
        }
    }
}

TEST_F(Mnx_UnitTests, dynamicLettersAccepted)
{
    expectDynamicLetters("p", plainDynamic(DynValue::p));
    expectDynamicLetters("mf", plainDynamic(DynValue::mf));
    expectDynamicLetters("n", plainDynamic(DynValue::n));
    // A value is taken as far as it goes, so "fff" is one dynamic rather than f followed by a
    // residual ff. A residualValue here would make this an accent, which it is not.
    expectDynamicLetters("fff", plainDynamic(DynValue::fff));
    expectDynamicLetters("ffffff", plainDynamic(DynValue::ffffff));

    expectDynamicLetters("sf", accentDynamic(DynPrefix::s, DynValue::f, DynSuffix::None));
    expectDynamicLetters("sfz", accentDynamic(DynPrefix::s, DynValue::f, DynSuffix::z));
    expectDynamicLetters("rfz", accentDynamic(DynPrefix::r, DynValue::f, DynSuffix::z));
    expectDynamicLetters("fz", accentDynamic(DynPrefix::None, DynValue::f, DynSuffix::z));
    expectDynamicLetters("sfpp", accentDynamic(DynPrefix::s, DynValue::f, DynSuffix::None, DynValue::pp));

    // fp and pf carry no accent letters at all, but residualValue is accent-only in MNX.
    expectDynamicLetters("fp", accentDynamic(DynPrefix::None, DynValue::f, DynSuffix::None, DynValue::p));
    expectDynamicLetters("pf", accentDynamic(DynPrefix::None, DynValue::p, DynSuffix::None, DynValue::f));

    // Spellings MuseScore has no DynamicType for. These are the reason the fallback exists:
    // MNX builds a dynamic from parts, so it can say what MuseScore can only draw.
    expectDynamicLetters("sfzp", accentDynamic(DynPrefix::s, DynValue::f, DynSuffix::z, DynValue::p));
    expectDynamicLetters("ffz", accentDynamic(DynPrefix::None, DynValue::ff, DynSuffix::z));
}

TEST_F(Mnx_UnitTests, dynamicLettersRejected)
{
    // Anything that is not wholly a dynamic spelling has to be refused rather than guessed at,
    // since the caller hands us whatever text the user typed. Note that a bare affix is
    // rejected too: MNX has nothing to hang an accent on without a value.
    for (const char* letters : { "", "s", "r", "z", "sz",
                                 "sempre f", "f subito", "poco", "f-p",
                                 "SFZ", "Sfz",                          // spelling is case sensitive
                                 "fx", "fzz", "ffzz" }) {
        SCOPED_TRACE(letters);
        EXPECT_FALSE(toMnxDynamicFromLetters(letters).has_value());
    }
}

TEST_F(Mnx_UnitTests, dynamicLettersMatchDynamicTypeTable)
{
    for (int index = 0; index < int(DynamicType::LAST); index++) {
        const DynamicType type = DynamicType(index);
        const MnxDynamicMapping expected = toMnxDynamicType(type);
        if (!expected.value) {
            continue; // MNX cannot express this one
        }
        SCOPED_TRACE(TConv::toXml(type).ascii());

        // Every dynamic MNX can express must parse back from the letters MuseScore spells it
        // with, or the grammar and toMnxDynamicType have drifted apart.
        expectDynamicLetters(TConv::toXml(type).ascii(), expected);

        // toMuseScoreDynamicType returns the first table entry that matches, which is only
        // correct because no two types share a mapping. Check that they still do not.
        EXPECT_EQ(toMuseScoreDynamicType(expected), std::make_optional(type));
    }
}
