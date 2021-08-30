#include "gp67domfixer.h"

#include <algorithm>

//#include "global/json_utils.hpp"
#include "global/log.h"

namespace  {
static constexpr int MIN_NOTES{ 3 };

struct Dynamic {
    std::string name;
    std::string shortName;
    std::vector<int> degrees;
};

static const std::vector<Dynamic> AvailableChords = {
    { /*name:*/ "major",  /*shortName:*/ " ", /*degrees:*/ { 0, 4, 7 } },
    { /*name:*/ "minor",  /*shortName:*/ "m", /*degrees:*/ { 0, 3, 7 } },
    { /*name:*/ "dim",    /*shortName:*/ "", /*degrees:*/ { 0, 3, 6 } },
    { /*name:*/ "aug",    /*shortName:*/ "", /*degrees:*/ { 0, 4, 8 } },
    { /*name:*/ "2",      /*shortName:*/ "", /*degrees:*/ { 0, 2, 4, 7 } },

    { /*name:*/ "7",      /*shortName:*/ "", /*degrees:*/ { 0, 4, 7, 10 } },
    { /*name:*/ "m7",     /*shortName:*/ "", /*degrees:*/ { 0, 3, 7, 10 } },
    { /*name:*/ "maj7",   /*shortName:*/ "", /*degrees:*/ { 0, 4, 7, 11 } },
    { /*name:*/ "dim7",   /*shortName:*/ "", /*degrees:*/ { 0, 3, 6, 9 } },
    { /*name:*/ "m/maj7", /*shortName:*/ "", /*degrees:*/ { 0, 3, 7, 11 } },

    { /*name:*/ "7+5",    /*shortName:*/ "", /*degrees:*/ { 0, 4, 8, 10 } },
    { /*name:*/ "7sus2",  /*shortName:*/ "", /*degrees:*/ { 0, 2, 7, 10 } },
    { /*name:*/ "7sus4",  /*shortName:*/ "", /*degrees:*/ { 0, 5, 7, 10 } },
    { /*name:*/ "6",      /*shortName:*/ "", /*degrees:*/ { 0, 4, 7, 9 } },
    { /*name:*/ "m6",     /*shortName:*/ "", /*degrees:*/ { 0, 3, 7, 9 } },

    { /*name:*/ "9",      /*shortName:*/ "", /*degrees:*/ { 0, 4, 7, 10, 2 } },
    { /*name:*/ "9",      /*shortName:*/ "", /*degrees:*/ { 0, 4, 10, 2 } },

    { /*name:*/ "-9",     /*shortName:*/ "", /*degrees:*/ { 0, 4, 7, 10, 1 } },
    { /*name:*/ "-9",     /*shortName:*/ "", /*degrees:*/ { 0, 4, 10, 1 } },

    { /*name:*/ "m9",     /*shortName:*/ "", /*degrees:*/ { 0, 3, 7, 10, 2 } },
    { /*name:*/ "m9",     /*shortName:*/ "", /*degrees:*/ { 0, 3, 10, 2 } },

    { /*name:*/ "m-9",    /*shortName:*/ "", /*degrees:*/ { 0, 3, 7, 10, 1 } },
    { /*name:*/ "m-9",    /*shortName:*/ "", /*degrees:*/ { 0, 3, 10, 1 } },

    { /*name:*/ "maj9",   /*shortName:*/ "", /*degrees:*/ { 0, 4, 7, 11, 2 } },
    { /*name:*/ "maj9",   /*shortName:*/ "", /*degrees:*/ { 0, 4, 11, 2 } },

    { /*name:*/ "sus2",   /*shortName:*/ "", /*degrees:*/ { 0, 2, 7 } },
    { /*name:*/ "sus4",   /*shortName:*/ "", /*degrees:*/ { 0, 5, 7 } },
    { /*name:*/ "add9",   /*shortName:*/ "", /*degrees:*/ { 0, 4, 7, 2 } },

    { /*name:*/ "5",      /*shortName:*/ "", /*degrees:*/ { 0, 7 } }
};

static const std::vector<Dynamic> AvailableChordsWithUnimportantOrder = {
    { /*name:*/ "dim7",   /*shortName:*/ "", /*degrees:*/ { 0, 3, 6, 9 } }
};

static const std::vector<std::string> NoteNames = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
}

using namespace Ms;

GP67DomFixer::GP67DomFixer()
{
}

void GP67DomFixer::fixGPDomModel(GPDomModel* gpDom)
{
    if (isLyricsOnBeats(gpDom)) {
        LOGD() << "lyrics on beats from gp";
    } else {
        LOGD() << "parse and self break lyrics on beats";
        breakLyricsOnBeats(gpDom);
    }

    if (isHasDiagrams(gpDom)) {
        LOGD() << "diagrams read from gp";
    } else {
        LOGD() << "parse and self create diagrams";
        createDiagrams(gpDom);
    }
}

bool GP67DomFixer::isLyricsOnBeats(const GPDomModel* gpDom)
{
    const auto& masterBars = gpDom->masterBars();
    for (const auto& [trackNum, track] : gpDom->tracks()) {
        if (track->lyrics().empty()) {
            continue;
        }

        for (size_t mi = 0; mi < masterBars.size(); ++mi) {
            const auto& mb = masterBars[mi];

            IF_ASSERT_FAILED(mb->bars().size() > trackNum) {
                continue;
            }

            const auto& trackBar = mb->bars().at(trackNum);
            if (trackBar->_voices.empty()) {
                continue;
            }

            auto& voice = trackBar->_voices[0];
            for (auto& beat : voice->_beats) {
                if (!beat->lyrics().empty()) {
                    return true;
                }
            }
        }
    }

    return false;
}

//! NOTE За основу взят код старого плеера от сюда
//! https://git.wsmgroup.ru/web/haxe-flash_ultimateguitar/blob/master/TabReader/src/songModel/Song.hx#L992

void GP67DomFixer::breakLyricsOnBeats(GPDomModel* gpDom)
{
//    std::vector<std::unique_ptr<GPMasterBar>>& masterBars = gpDom->_masterBars;
//
//    for ( const auto& [trackNum, track] : gpDom->tracks()) {
//
//        if (track->lyrics().empty()) {
//            continue;
//        }
//
//        std::string originTrackLyrics = track->lyrics();
//        size_t trackLyricsOffset = track->lyricsOffset();
//
//        //! NOTE Подготовка и парсинг лирики
//        std::vector<std::string> beatLyrics;
//        {
//
//            std::string trackLyrics;
//            trackLyrics.reserve(originTrackLyrics.size());
//
//            bool skip = false;
//            for (auto c : originTrackLyrics) {
//                if (c == '[') {
//                    skip = true;
//                } else if (c == ']') {
//                    skip = false;
//                } else if (!skip) {
//                    trackLyrics.push_back(c);
//                }
//            }
//
//            xtz::replaceAll(trackLyrics, " -", "- ");
//            xtz::replaceAll(trackLyrics, "- ", "-");;
//
//            // слова через дефис
//            xtz::replaceAll(trackLyrics, "--", "&&&&&-");
//            xtz::replaceAll(trackLyrics, "-", "- ");
//            xtz::replaceAll(trackLyrics, "\r", "_____");
//            xtz::replaceAll(trackLyrics, "\n", "_____");
//
//            std::regex rx("_{5,}");
//            xtz::replaceAll(trackLyrics, rx, " ");
//
//            xtz::split(trackLyrics, beatLyrics, " ");
//            for (auto &bl : beatLyrics) {
//                xtz::replaceAll(bl, "&&&&&", "-");
//            }
//        }
//
//        if (beatLyrics.empty()) {
//            return;
//        }
//
//        //! NOTE Установка лирики по битам
//        {
//            int numberOfBeatWithoutLyrics = 0;
//            int beatNumber = 0;
//
//            for (size_t mi = 0; mi < masterBars.size(); ++mi) {
//
//                auto& measure = masterBars[mi];
//
//                IF_ASSERT_FAILED(measure->_bars.size() > trackNum) {
//                    continue;
//                }
//
//                auto& trackBar = measure->_bars[trackNum];
//                if (trackBar->_voices.empty()) {
//                    continue;
//                }
//
//                auto& voice = trackBar->_voices[0];
//                for ( auto& beat : voice->_beats) {
//
//                    bool hasLyrics = false;
//                    if (mi < trackLyricsOffset) {
//                        numberOfBeatWithoutLyrics++;
//                        beatNumber++;
//                    }
//                    else {
//                        if (!beat->isRest()) {
//                            hasLyrics = true;
//                            beatNumber++;
//                        }
//                    }
//
//                    if (hasLyrics && mi >= trackLyricsOffset) {
//                        size_t index = beatNumber - numberOfBeatWithoutLyrics - 1;
//                        if (index < beatLyrics.size()) {
//
//                            std::string beatLyric = beatLyrics[index];
//                            xtz::replace(beatLyric, "+", " ");
//
//                            beat->setLyrics(trackNum, (uint32_t)mi, beatLyric);
//                        }
//                    }
//                }
//            }
//        }
//    }
}

bool GP67DomFixer::isHasDiagrams(const GPDomModel* gpDom)
{
    for (const auto& [trackNum, track] : gpDom->tracks()) {
        if (!track->diagram().empty()) {
            return true;
        }
    }
    return false;
}

//! NOTE За основу взят код старого плеера от сюда
//! https://git.wsmgroup.ru/web/haxe-flash_ultimateguitar/blob/master/TabReader/src/songModel/filter/SongFilterTracksRenameByInstruments.hx

void GP67DomFixer::createDiagrams(GPDomModel* gpDom)
{
//    std::vector<std::unique_ptr<GPMasterBar>>& masterBars = gpDom->_masterBars;
//
//    auto trackDiagramId = [](GPTrack* t, const std::string& chordName) {
//        for (const auto& [id, d] : t->_diagrams) {
//            if (d.name == chordName) {
//                return id;
//            }
//        }
//        return -1;
//    };
//
//    auto lastDiagramId = [](GPTrack* t) {
//        int lastId = -1;
//        for (auto it = t->_diagrams.cbegin(); it != t->_diagrams.cend(); ++it) {
//            if (it->first > lastId) {
//                lastId = it->first;
//            }
//        }
//        return lastId;
//    };
//
//
//    for (auto& [trackNum, track] : gpDom->_tracks) {
//
//        if (!track->isGuitar()) {
//            continue;
//        }
//
//        std::vector<GPTrack::String> strings = track->strings();
//        for (size_t mi = 0; mi < masterBars.size(); ++mi) {
//
//            auto& measure = masterBars[mi];
//
//            IF_ASSERT(measure->_bars.size() > trackNum) {
//                continue;
//            }
//
//            auto& trackBar = measure->_bars[trackNum];
//            if (trackBar->_voices.empty()) {
//                continue;
//            }
//
//            auto& voice = trackBar->_voices[0];
//            for ( auto& beat : voice->_beats) {
//
//                std::string chordName  = chordNameForBeatWithStrings(beat.get(), strings);
//                if (chordName.empty()) {
//                    beat->setDiagramIdx(trackNum, (int32_t)mi, -1);
//                    continue;
//                }
//
//                int diaId = trackDiagramId(track.get(), chordName);
//                if (diaId == -1) {
//                    diaId = lastDiagramId(track.get()) + 1;
//
//                    //! TODO Нужно добавить др свойства Diagram
//                    //! если нужно будет рисовать сами диаграмы,
//                    //! сейчас диаграмы используются только, чтобы показывать аккорды
//                    GPTrack::Diagram dia;
//                    dia.id = diaId;
//                    dia.name = chordName;
//
//                    track->addDiagram({dia.id, dia});
//                }
//
//                beat->setDiagramIdx(trackNum, (int32_t)mi, diaId);
//            }
//        }
//    }
}

std::string GP67DomFixer::chordNameForBeatWithStrings(GPBeat* beat, const std::vector<GPTrack::String>& strings)
{
//    std::string chordName;
//    const std::vector<std::shared_ptr<GPNote>>& notes = beat->notes();
//    std::vector<int> chordMidiIndexes;
//
//    if (notes.size() < MIN_NOTES) {
//        return std::string();
//    }
//
//    for (const auto& note : notes) {
//
//        if(!(note->string() >= 0 && note->string() < strings.size())) {
//            return std::string();
//        }
//
//        IF_ASSERT(note->fret() >= 0) {
//            return std::string();
//        }
//
//        int noteMidiIndex = strings[note->string()].tunning + note->fret();
//        chordMidiIndexes.push_back(noteMidiIndex);
//    }
//
//    std::sort(chordMidiIndexes.begin(), chordMidiIndexes.end());
//
//    int lastEqualDegreeBaseNoteMidiIndex = -1;
//    int k = 0;
//    for (int baseNoteMidiIndex : chordMidiIndexes)
//    {
//        std::vector<int> chordDegrees;
//        int baseNoteDegree = baseNoteMidiIndex % 12;
//        for (int outherNoteMidiIndex : chordMidiIndexes)
//        {
//            int outherNoteDegree = outherNoteMidiIndex % 12;
//            int chordDegree = (12 - (baseNoteDegree - outherNoteDegree)) % 12;
//            chordDegrees.push_back(chordDegree);
//        }
//
//        std::sort(chordDegrees.begin(), chordDegrees.end());
//        auto last = std::unique(chordDegrees.begin(), chordDegrees.end());
//        chordDegrees.erase(last, chordDegrees.end());
//
//        const std::vector<Dynamic>* availableChords = nullptr;
//        if (k > 0)
//        {
//            availableChords = &AvailableChordsWithUnimportantOrder;
//        }
//        else
//        {
//            availableChords = &AvailableChords;
//        }
//
//        for (const Dynamic& availableChord : *availableChords)
//        {
//            std::vector<int> availableChordDegrees = availableChord.degrees;
//            std::sort(availableChordDegrees.begin(), availableChordDegrees.end());
//
//            if (chordDegrees == availableChordDegrees) {
//                if (lastEqualDegreeBaseNoteMidiIndex == -1 || baseNoteMidiIndex < lastEqualDegreeBaseNoteMidiIndex) {
//                    lastEqualDegreeBaseNoteMidiIndex = baseNoteMidiIndex;
//                    IF_ASSERT(baseNoteDegree >= 0 && baseNoteDegree < NoteNames.size()) {
//                        return std::string();
//                    }
//                    chordName = NoteNames[baseNoteDegree] + (availableChord.shortName.empty() ? availableChord.name : availableChord.shortName);
//                }
//            }
//        }
//
//        k++;
//    }
//
//    return chordName;

    return "";
}
