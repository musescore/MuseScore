/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include <algorithm>
#include <array>
#include <memory>
#include <regex>
#include <string>
#include <string_view>
#include <unordered_set>

#include "engraving/dom/masterscore.h"
#include "engraving/dom/mscore.h"
#include "engraving/engravingerrors.h"
#include "framework/global/modularity/ioc.h"
#include "importexport/mnx/imnxconfiguration.h"
#include "log.h"

#include "engraving/tests/utils/scorerw.h"
#include "importexport/mnx/internal/notationmnxreader.h"
#include "importexport/mnx/internal/import/mnximporter.h"
#include "importexport/mnx/internal/export/mnxexporter.h"

#include "io/dir.h"
#include "io/buffer.h"
#include "io/file.h"
#include "io/fileinfo.h"
#include "io/path.h"

#include "engraving/compat/scoreaccess.h"
#include "engraving/infrastructure/localfileinfoprovider.h"
#include "types/bytearray.h"
#include "types/ret.h"
#include "engraving/rw/rwregister.h"

#include "mnxdom.h"

using namespace mu::engraving;
using namespace mu::iex::mnxio;
using namespace muse;

namespace {
bool exportBeamsEnabledForW3c(const std::string& baseName)
{
    static const std::unordered_set<std::string> testsWithoutBeams {
        "dotted-notes",
        "grace-note",
        "rest-positions",
        "tie-target-type"
    };
    return testsWithoutBeams.count(baseName) == 0;
}

bool exportRestPositionsEnabledForW3c(const std::string& baseName)
{
    return baseName == "rest-positions";
}

class ScopedExportBeamsSetting
{
public:
    explicit ScopedExportBeamsSetting(bool enabled)
    {
        m_configuration = muse::modularity::globalIoc()->resolve<IMnxConfiguration>("iex_mnx");
        if (m_configuration) {
            m_previous = m_configuration->mnxExportBeams();
            m_configuration->setMnxExportBeams(enabled);
        }
    }

    ~ScopedExportBeamsSetting()
    {
        if (m_configuration) {
            m_configuration->setMnxExportBeams(m_previous);
        }
    }

private:
    std::shared_ptr<IMnxConfiguration> m_configuration;
    bool m_previous = true;
};

class ScopedExportRestPositionsSetting
{
public:
    explicit ScopedExportRestPositionsSetting(bool enabled)
    {
        m_configuration = muse::modularity::globalIoc()->resolve<IMnxConfiguration>("iex_mnx");
        if (m_configuration) {
            m_previous = m_configuration->mnxExportRestPositions();
            m_configuration->setMnxExportRestPositions(enabled);
        }
    }

    ~ScopedExportRestPositionsSetting()
    {
        if (m_configuration) {
            m_configuration->setMnxExportRestPositions(m_previous);
        }
    }

private:
    std::shared_ptr<IMnxConfiguration> m_configuration;
    bool m_previous = false;
};
}

static const String MNX_DATA_DIR(u"data/project_examples/");
static const String MNX_REFERENCE_DIR(u"data/mnx_reference_examples/");
static const String MSCX_REFERENCE_DIR(u"data/mscx_reference_examples/");
static const String MSCX_PROJECT_REFERENCE_DIR(u"data/project_examples/");

static std::string normalizeMscxText(const std::string& text, bool normalizeBeamMode);

static const std::unordered_set<std::string> MNX_NO_ROUNDTRIP {
    /// @note File contains dynamics, which are not currently exported to MNX.
    "dynamics",
    /// @note clarinet38MissingTime omits a time signature in MNX, so roundtrip inserts one and mismatches.
    "clarinet38MissingTime",
    /// @note key77 includes an invalid transposed key; export falls back and mismatches expected output.
    "key77",
    /// @note multimeasure-rests has an explicit regular barline that is dropped on export, shifting eids.
    "multimeasure-rests",
    /// @note organ-layout is a W3C example missing clefs; we don't change the example, so skip roundtrip.
    "organ-layout",
    /// @note Files contains overlapping ottavas, which are not exported in the same octave due to MuseScore
    /// playback only playing one ottava at a time.
    "ottavas"
};

class Mnx_Tests : public ::testing::Test
{
public:
    MasterScore* readMnxScore(const String& fileName, bool isAbsolutePath = false);
    std::string exportMnxJson(Score* score);
    MasterScore* importMnxFromJson(const std::string& json, const String& virtualPath);
    MasterScore* roundTripMnxScore(const String& sourceFile, const String& exportedFile);

    bool compareWithMscxReference(Score* score, const String& referencePath, const char* testName = nullptr);
    bool importReferenceExample(const String& baseName);
    void runProjectFileTest(const char* name);
    void runW3cExampleTest(const char* name);
};

//---------------------------------------------------------
//   fixupScore -- do required fixups after reading/importing score
//---------------------------------------------------------

static void fixupScore(MasterScore* score)
{
    score->connectTies();
    score->masterScore()->rebuildMidiMapping();
    score->setSaved(false);
}

MasterScore* Mnx_Tests::readMnxScore(const String& fileName, bool isAbsolutePath)
{
    auto importFunc = [](MasterScore* score, const muse::io::path_t& path) -> Err {
        NotationMnxReader reader;
        Ret ret = reader.read(score, path);
        if (ret.success()) {
            return Err::NoError;
        }

        const int code = ret.code();
        if (code >= static_cast<int>(Ret::Code::EngravingFirst)
            && code <= static_cast<int>(Ret::Code::EngravingLast)) {
            return static_cast<Err>(code);
        }

        if (code == static_cast<int>(Ret::Code::NotSupported)
            || code == static_cast<int>(Ret::Code::BadData)) {
            return Err::FileBadFormat;
        }

        return Err::UnknownError;
    };

    MasterScore* score = ScoreRW::readScore(fileName, isAbsolutePath, importFunc);
    return score;
}

std::string Mnx_Tests::exportMnxJson(Score* score)
{
    auto mnxConfiguration = muse::modularity::globalIoc()->resolve<mu::iex::mnxio::IMnxConfiguration>("iex_mnx");
    const bool exportBeams = mnxConfiguration ? mnxConfiguration->mnxExportBeams() : true;
    const bool exportRestPositions = mnxConfiguration ? mnxConfiguration->mnxExportRestPositions() : false;
    LOGI() << "MNX export initiated; exportBeams=" << (exportBeams ? "true" : "false")
           << " exportRestPositions=" << (exportRestPositions ? "true" : "false");
    MnxExporter exporter(score, exportBeams, exportRestPositions);
    Ret ret = exporter.exportMnx();
    if (!ret.success()) {
        return {};
    }

    return exporter.mnxDocument().root()->dump(2);
}

MasterScore* Mnx_Tests::importMnxFromJson(const std::string& json, const String& virtualPath)
{
    auto score = std::unique_ptr<MasterScore>(
        compat::ScoreAccess::createMasterScoreWithBaseStyle(nullptr));
    score->setFileInfoProvider(std::make_shared<LocalFileInfoProvider>(muse::io::path_t(virtualPath)));

    try {
        auto doc = mnx::Document::create(json.data(), json.size());
        if (!mnx::validation::schemaValidate(doc)) {
            ADD_FAILURE() << "Roundtrip MNX is not valid: " << virtualPath.toStdString();
            return nullptr;
        }
        if (doc.global().measures().empty()) {
            ADD_FAILURE() << "Roundtrip MNX contains no measures: " << virtualPath.toStdString();
            return nullptr;
        }
        MnxImporter importer(score.get(), std::move(doc));
        importer.importMnx();
    } catch (const std::exception& ex) {
        ADD_FAILURE() << "Roundtrip MNX failed to parse: " << ex.what();
        return nullptr;
    }

    return score.release();
}

MasterScore* Mnx_Tests::roundTripMnxScore(const String& sourceFile, const String& exportedFile)
{
    std::unique_ptr<MasterScore> score(readMnxScore(sourceFile));
    if (!score) {
        return nullptr;
    }

    fixupScore(score.get());
    score->doLayout();

    const std::string json = exportMnxJson(score.get());
    if (json.empty()) {
        return nullptr;
    }

    MasterScore* roundTrip = importMnxFromJson(json, exportedFile);
    if (roundTrip) {
        fixupScore(roundTrip);
        roundTrip->doLayout();
    }

    return roundTrip;
}

bool Mnx_Tests::compareWithMscxReference(Score* score, const String& referencePath, const char* testName)
{
#if MUE_MNX_WRITE_REFS
    const String referenceAbsPath = ScoreRW::rootPath() + u"/" + referencePath;
    const io::path_t referenceDir = io::dirpath(referenceAbsPath);
    io::Dir dir(referenceDir);
    if (!dir.exists()) {
        io::Dir::mkpath(referenceDir);
    }
    return ScoreRW::saveScore(score, referenceAbsPath);
#else
    io::Buffer buffer;
    if (!buffer.open(io::IODevice::WriteOnly)) {
        ADD_FAILURE() << "Failed to open buffer for MSCX output.";
        return false;
    }

    if (!rw::RWRegister::writer(score->iocContext())->writeScore(score, &buffer)) {
        ADD_FAILURE() << "Failed to serialize score to MSCX.";
        return false;
    }

    // Records needing to avoid BeamMode comparisons must go here.
    constexpr std::array<std::string_view, 1> normalizationTests{
        /// @note The exporter exports layout beams, which omits one of the beams-over-barline in the input
        "project_beamsOverBarlines"
    };

    bool normalizeBeamMode = false;
    if (testName) {
        for (const std::string_view name : normalizationTests) {
            if (name == testName) {
                normalizeBeamMode = true;
                break;
            }
        }
    }

    LOGI() << "BeamMode normalization " << (normalizeBeamMode ? "enabled" : "disabled") << " for "
           << (testName ? testName : "unnamed test");

    const std::string outputText = normalizeMscxText(
        std::string(reinterpret_cast<const char*>(buffer.data().constData()), buffer.data().size()), normalizeBeamMode);

    ByteArray referenceData;
    const String referenceAbsPath = ScoreRW::rootPath() + u"/" + referencePath;
    Ret readRet = io::File::readFile(referenceAbsPath, referenceData);
    if (!readRet.success()) {
        ADD_FAILURE() << "Failed to read MSCX reference file: " << referenceAbsPath.toStdString()
                      << " (code " << readRet.code() << ")";
        return false;
    }

    const std::string referenceText = normalizeMscxText(
        std::string(referenceData.constChar(), referenceData.size()), normalizeBeamMode);

    if (referenceText == outputText) {
        return true;
    }

    const size_t maxLen = std::min(referenceText.size(), outputText.size());
    size_t mismatch = 0;
    while (mismatch < maxLen && referenceText[mismatch] == outputText[mismatch]) {
        ++mismatch;
    }

    const size_t context = 80;
    const size_t start = mismatch > context ? mismatch - context : 0;
    const size_t end = std::min(mismatch + context, maxLen);

    ADD_FAILURE() << "MSCX mismatch at index " << mismatch
                  << " (ref len " << referenceText.size()
                  << ", out len " << outputText.size() << ")";
    ADD_FAILURE() << "Reference snippet: " << referenceText.substr(start, end - start);
    ADD_FAILURE() << "Output snippet:    " << outputText.substr(start, end - start);

    const bool hasBeamMode = referenceText.find("BeamMode") != std::string::npos
                             || outputText.find("BeamMode") != std::string::npos;
    const bool hasStemDirection = referenceText.find("StemDirection") != std::string::npos
                                  || outputText.find("StemDirection") != std::string::npos;
    if (hasBeamMode || hasStemDirection) {
        ADD_FAILURE() << "Contains BeamMode=" << (hasBeamMode ? "yes" : "no")
                      << ", StemDirection=" << (hasStemDirection ? "yes" : "no");
    }

    return false;
#endif
}

static String mnxBaseNameFromMacro(const char* name)
{
    std::string baseName = name;
    std::replace(baseName.begin(), baseName.end(), '_', '-');
    return String::fromUtf8(baseName.c_str());
}

static String mscxRefName(const String& baseName)
{
    return baseName + u"_ref.mscx";
}

static String projectRefPath(const String& baseName)
{
    return MSCX_PROJECT_REFERENCE_DIR + mscxRefName(baseName);
}

static String w3cRefPath(const String& baseName)
{
    return MSCX_REFERENCE_DIR + mscxRefName(baseName);
}

static String tempRoundTripPath(const String& baseName)
{
    return u"<roundtrip>/" + baseName + u".mnx";
}

static std::string normalizeMscxText(const std::string& text, bool normalizeBeamMode)
{
    static const std::regex crlfRe("\r\n");
    static const std::regex tagWhitespaceRe(">\\s+<");
    // Part names get changed in round trip
    static const std::regex trackNameRe("<trackName>[\\s\\S]*?</trackName>");
    // Beam modes are different on round trip if inbound file doesn't set useBeams
    static const std::regex beamModeRe("<BeamMode>[\\s\\S]*?</BeamMode>");
    // Explicit normal barlines are redundant and not preserved on export
    static const std::regex normalBarlineRe("<BarLine>(?:(?!<subtype>)[\\s\\S])*?</BarLine>");
    std::string out = std::regex_replace(text, crlfRe, "\n");
    out = std::regex_replace(out, trackNameRe, "");
    if (normalizeBeamMode) {
        out = std::regex_replace(out, beamModeRe, "");
    }
    out = std::regex_replace(out, normalBarlineRe, "");
    return std::regex_replace(out, tagWhitespaceRe, "><");
}

bool Mnx_Tests::importReferenceExample(const String& baseName)
{
    const String referencePath = w3cRefPath(baseName);
    const String referenceAbsPath = ScoreRW::rootPath() + u"/" + referencePath;
#if !MUE_MNX_WRITE_REFS
    if (!io::FileInfo::exists(referenceAbsPath)) {
        ADD_FAILURE() << "Missing MSCX reference file: " << referencePath.toStdString();
        return false;
    }
#endif

    SCOPED_TRACE(baseName.toStdString());
    const String sourcePath = MNX_REFERENCE_DIR + baseName + u".json";

    std::unique_ptr<MasterScore> score(readMnxScore(sourcePath));
    if (!score) {
        ADD_FAILURE() << "Failed to import MNX reference file: " << sourcePath.toStdString();
        return false;
    }

    fixupScore(score.get());
    score->doLayout();

    return compareWithMscxReference(score.get(), referencePath);
}

void Mnx_Tests::runProjectFileTest(const char* name)
{
    const String baseName = String::fromUtf8(name);
    const String sourcePath = MNX_DATA_DIR + baseName + u".mnx";

    std::unique_ptr<MasterScore> score(readMnxScore(sourcePath));
    ASSERT_TRUE(score);

    fixupScore(score.get());
    score->doLayout();

    const String referencePath = projectRefPath(baseName);
    const std::string testName = std::string("project_") + name;
    EXPECT_TRUE(compareWithMscxReference(score.get(), referencePath, testName.c_str()));

    if (MUE_MNX_WRITE_REFS) {
        return;
    }
    if (MNX_NO_ROUNDTRIP.count(baseName.toStdString()) > 0) {
        return;
    }

    const String exportName = tempRoundTripPath(baseName);
    std::unique_ptr<MasterScore> roundTrip(roundTripMnxScore(sourcePath, exportName));
    ASSERT_TRUE(roundTrip);

    EXPECT_TRUE(compareWithMscxReference(roundTrip.get(), referencePath, testName.c_str()));
}

void Mnx_Tests::runW3cExampleTest(const char* name)
{
    const String baseName = mnxBaseNameFromMacro(name);
    const std::string baseNameUtf8 = baseName.toStdString();
    ScopedExportBeamsSetting exportBeamsGuard(exportBeamsEnabledForW3c(baseNameUtf8));
    ScopedExportRestPositionsSetting exportRestPositionsGuard(exportRestPositionsEnabledForW3c(baseNameUtf8));

    if (!importReferenceExample(baseName)) {
        return;
    }

    if (MUE_MNX_WRITE_REFS) {
        return;
    }
    if (MNX_NO_ROUNDTRIP.count(baseName.toStdString()) > 0) {
        return;
    }

    const std::string testName = std::string("w3c_") + name;
    const String referencePath = w3cRefPath(baseName);
    const String referenceAbsPath = ScoreRW::rootPath() + u"/" + referencePath;
    if (!io::FileInfo::exists(referenceAbsPath)) {
        return;
    }

    const String exportName = tempRoundTripPath(baseName);
    std::unique_ptr<MasterScore> roundTrip(roundTripMnxScore(MNX_REFERENCE_DIR + baseName + u".json", exportName));
    ASSERT_TRUE(roundTrip);

    EXPECT_TRUE(compareWithMscxReference(roundTrip.get(), referencePath, testName.c_str()));
}

#define MNX_PROJECT_FILE_TEST(name) \
    TEST_F(Mnx_Tests, project_##name) { runProjectFileTest(#name); }

#define MNX_PROJECT_FILE_TEST_DISABLED(name) \
    TEST_F(Mnx_Tests, DISABLED_project_##name) { runProjectFileTest(#name); }

#define MNX_W3C_EXAMPLE_TEST(name) \
    TEST_F(Mnx_Tests, w3c_##name) { runW3cExampleTest(#name); }

#define MNX_W3C_EXAMPLE_TEST_DISABLED(name) \
    TEST_F(Mnx_Tests, DISABLED_w3c_##name) { runW3cExampleTest(#name); }

MNX_PROJECT_FILE_TEST(altoFluteTrem)
MNX_PROJECT_FILE_TEST(altoFluteTremMissingKey)
MNX_PROJECT_FILE_TEST_DISABLED(barlineTypesOriginal) // the original file is just for creating the edited file.
MNX_PROJECT_FILE_TEST(barlineTypesWithShort)
MNX_PROJECT_FILE_TEST(bcl)
MNX_PROJECT_FILE_TEST(beamsOverBarlines)
MNX_PROJECT_FILE_TEST(clarinet38)
MNX_PROJECT_FILE_TEST(clarinet38MissingTime)
MNX_PROJECT_FILE_TEST(enharmonicPart)
MNX_PROJECT_FILE_TEST(graceBeamed)
MNX_PROJECT_FILE_TEST(key56Wrapped56Edited)
MNX_PROJECT_FILE_TEST_DISABLED(key56Wrapped56Unedited) // the unedited file is just for creating the edited file.
MNX_PROJECT_FILE_TEST(key77)
MNX_PROJECT_FILE_TEST(key77Wrapped)
MNX_PROJECT_FILE_TEST(layoutBrackets)
MNX_PROJECT_FILE_TEST(measnumSequences)
MNX_PROJECT_FILE_TEST(multinoteTremolos)
MNX_PROJECT_FILE_TEST(multinoteTremolosAdv)
MNX_PROJECT_FILE_TEST(ottavas)
MNX_PROJECT_FILE_TEST(percussionKit)
MNX_PROJECT_FILE_TEST(restPosition)
MNX_PROJECT_FILE_TEST(tupletHiddenRest)
MNX_PROJECT_FILE_TEST(tupletNested)
MNX_PROJECT_FILE_TEST(tupletSimple)

MNX_W3C_EXAMPLE_TEST(accidentals)
MNX_W3C_EXAMPLE_TEST(articulations)
MNX_W3C_EXAMPLE_TEST(beam_hooks)
MNX_W3C_EXAMPLE_TEST(beams_across_barlines)
MNX_W3C_EXAMPLE_TEST(beams_inner_grace_notes)
MNX_W3C_EXAMPLE_TEST(beams_secondary_beam_breaks_implied)
MNX_W3C_EXAMPLE_TEST(beams_secondary_beam_breaks)
MNX_W3C_EXAMPLE_TEST(beams)
MNX_W3C_EXAMPLE_TEST(clef_changes)
MNX_W3C_EXAMPLE_TEST(dotted_notes)
MNX_W3C_EXAMPLE_TEST(dynamics)
MNX_W3C_EXAMPLE_TEST(grace_note)
MNX_W3C_EXAMPLE_TEST(grace_notes_beamed)
MNX_W3C_EXAMPLE_TEST(grand_staff)
MNX_W3C_EXAMPLE_TEST(hello_world)
MNX_W3C_EXAMPLE_TEST(jumps_dal_segno)
MNX_W3C_EXAMPLE_TEST(jumps_ds_al_fine)
MNX_W3C_EXAMPLE_TEST(key_signatures)
MNX_W3C_EXAMPLE_TEST(lyric_line_metadata)
MNX_W3C_EXAMPLE_TEST(lyrics_basic)
MNX_W3C_EXAMPLE_TEST(lyrics_multi_line)
MNX_W3C_EXAMPLE_TEST(multi_note_tremolos)
MNX_W3C_EXAMPLE_TEST(multimeasure_rests)
MNX_W3C_EXAMPLE_TEST(multiple_layouts)
MNX_W3C_EXAMPLE_TEST(multiple_voices)
MNX_W3C_EXAMPLE_TEST_DISABLED(orchestral_layout)
MNX_W3C_EXAMPLE_TEST(organ_layout)
MNX_W3C_EXAMPLE_TEST(ottavas_8va)
MNX_W3C_EXAMPLE_TEST(parts)
MNX_W3C_EXAMPLE_TEST(repeats_alternate_endings_advanced)
MNX_W3C_EXAMPLE_TEST(repeats_alternate_endings_simple)
MNX_W3C_EXAMPLE_TEST(repeats_implied_start_repeat)
MNX_W3C_EXAMPLE_TEST(repeats_more_once_repeated)
MNX_W3C_EXAMPLE_TEST(repeats)
MNX_W3C_EXAMPLE_TEST(rest_positions)
MNX_W3C_EXAMPLE_TEST(single_note_tremolos)
MNX_W3C_EXAMPLE_TEST(slurs_chords)
MNX_W3C_EXAMPLE_TEST(slurs_targeting_specific_notes)
MNX_W3C_EXAMPLE_TEST(slurs)
MNX_W3C_EXAMPLE_TEST_DISABLED(system_layouts)
MNX_W3C_EXAMPLE_TEST(tempo_markings)
MNX_W3C_EXAMPLE_TEST(three_note_chord_and_half_rest)
MNX_W3C_EXAMPLE_TEST(tie_target_type)
MNX_W3C_EXAMPLE_TEST(ties)
MNX_W3C_EXAMPLE_TEST(time_signature_glyphs)
MNX_W3C_EXAMPLE_TEST(time_signatures)
MNX_W3C_EXAMPLE_TEST(tuplets)
MNX_W3C_EXAMPLE_TEST(two_bar_c_major_scale)

#undef MNX_W3C_EXAMPLE_TEST_DISABLED
#undef MNX_W3C_EXAMPLE_TEST
#undef MNX_PROJECT_FILE_TEST_DISABLED
#undef MNX_PROJECT_FILE_TEST
