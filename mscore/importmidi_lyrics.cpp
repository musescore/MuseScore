#include "importmidi_lyrics.h"
#include "importmidi_inner.h"
#include "importmidi_fraction.h"
#include "importmidi_chord.h"
#include "libmscore/box.h"
#include "libmscore/element.h"
#include "libmscore/measurebase.h"
#include "libmscore/score.h"
#include "libmscore/staff.h"
#include "libmscore/text.h"
#include "midi/midifile.h"
#include "preferences.h"

#include <set>


namespace Ms {

extern Preferences preferences;

namespace MidiLyrics {

const std::string META_PREFIX = "@";
const std::string TEXT_PREFIX = "@T";


bool isMetaText(const std::string &text)
      {
      return text.substr(0, META_PREFIX.size()) == META_PREFIX;
      }

bool isLyricText(const std::string &text)
      {
      return !isMetaText(text) || text.substr(0, TEXT_PREFIX.size()) == TEXT_PREFIX;
      }

bool isLyricEvent(const MidiEvent &e)
      {
      return e.type() == ME_META && (e.metaType() == META_TEXT
                                     || e.metaType() == META_LYRIC);
      }

std::multimap<ReducedFraction, std::string>
extractLyricsFromTrack(const MidiTrack &track, int division)
      {
      std::multimap<ReducedFraction, std::string> lyrics;

      for (const auto &i: track.events()) {
            const auto& e = i.second;
            if (isLyricEvent(e)) {
                  const uchar* data = (uchar*)e.edata();
                  std::string text = MidiCharset::fromUchar(data);
                  if (isLyricText(text)) {
                        const auto tick = toMuseScoreTicks(i.first, division);
                                    // no charset handling here
                        lyrics.insert({tick, text});
                        }
                  }
            }
      return lyrics;
      }

struct BestTrack
      {
      int index = -1;
                  // <orig time, current time - after quantization>
      std::vector<std::pair<ReducedFraction, ReducedFraction>> matchedLyricTimes;
      };

// find track to insert lyrics
// it will be the track with max onTime match cases, except drum tracks

BestTrack findBestTrack(
            const QList<MTrack> &tracks,
            const std::multimap<ReducedFraction, std::string> &lyricTrack,
            const std::set<int> &usedTracks)
      {
      BestTrack bestTrack;
      int maxMatches = 0;

      for (int i = 0; i != tracks.size(); ++i) {
            if (tracks[i].mtrack->drumTrack() || usedTracks.find(i) != usedTracks.end())
                  continue;

            int matches = 0;
            for (const auto &chord: tracks[i].chords) {
                  for (const auto &note: chord.second.notes) {
                        if (lyricTrack.find(note.origOnTime) != lyricTrack.end()) {
                              ++matches;
                              bestTrack.matchedLyricTimes.push_back({chord.first,
                                                                     note.origOnTime});
                              break;
                              }
                        }
                  }
            if (matches > maxMatches) {
                  maxMatches = matches;
                  bestTrack.index = i;
                  }
            }

      return bestTrack;
      }

void addTitle(Score *score, const QString &string, int *textCounter)
      {
      if (string.left(TEXT_PREFIX.size()) == QString::fromStdString(TEXT_PREFIX)) {
            ++*textCounter;
            Text* text = new Text(score);
            if (*textCounter == 1)
                  text->setTextStyleType(TextStyleType::TITLE);
            else if (*textCounter == 2)
                  text->setTextStyleType(TextStyleType::COMPOSER);
            text->setText(string.right(string.size() - TEXT_PREFIX.size()));

            MeasureBase* measure = score->first();
            if (measure->type() != Element::Type::VBOX) {
                  measure = new VBox(score);
                  measure->setTick(0);
                  measure->setNext(score->first());
                  score->measures()->add(measure);
                  }
            measure->add(text);
            }
      }

// remove slashes in kar format

QString removeSlashes(const QString &text)
      {
      QString newText = text;
      newText = newText.replace("/", "");
      newText = newText.replace("\\", "");
      return newText;
      }

std::string removeSlashes(const std::string &text)
      {
      std::string str = text;
      char chars[] = "/\\";
      for (unsigned int i = 0; i != strlen(chars); ++i)
            str.erase(std::remove(str.begin(), str.end(), chars[i]), str.end());
      return str;
      }

void addLyricsToScore(
            const std::multimap<ReducedFraction, std::string> &lyricTrack,
            const std::vector<std::pair<ReducedFraction, ReducedFraction>> &matchedLyricTimes,
            const Staff *staffAddTo)
      {
      Score *score = staffAddTo->score();
      int textCounter = 0;

      for (const auto &timePair: matchedLyricTimes) {
            const auto lyricTime = timePair.first;
            const auto it = lyricTrack.find(timePair.second);

            Q_ASSERT_X(it != lyricTrack.end(),
                       "MidiLyrics::addLyricsToScore", "Lyric time not found");

            QString text = MidiCharset::convertToCharset(it->second);
            if (lyricTime == ReducedFraction(0, 1))
                  addTitle(score, text, &textCounter);
            else
                  score->addLyrics(lyricTime.ticks(), staffAddTo->idx(), removeSlashes(text));
            }
      }

void extractLyricsToMidiData(const MidiFile *mf)
      {
      for (const auto &t: mf->tracks()) {
            const auto lyrics = extractLyricsFromTrack(t, mf->division());
            if (!lyrics.empty())
                  preferences.midiImportOperations.addTrackLyrics(lyrics);
            }
      }

void assignLyricsToTracks(QList<MTrack> &tracks)
      {
      std::set<int> usedTracks;
      const auto *lyricTracks = preferences.midiImportOperations.getLyrics();
      if (!lyricTracks)
            return;

      for (int i = 0; i != lyricTracks->size(); ++i) {
            const BestTrack bestTrack = findBestTrack(tracks, (*lyricTracks)[i], usedTracks);
            if (bestTrack.index >= 0) {
                  usedTracks.insert(bestTrack.index);
                  tracks[bestTrack.index].initLyricTrackIndex = i;
                  }
            }
      }

void setInitialLyricsFromMidiData(const QList<MTrack> &tracks)
      {
      std::set<int> usedTracks;
      const auto *lyricTracks = preferences.midiImportOperations.getLyrics();
      if (!lyricTracks)
            return;

      for (const auto &lyricTrack: *lyricTracks) {
            const BestTrack bestTrack = findBestTrack(tracks, lyricTrack, usedTracks);
            if (bestTrack.index >= 0) {
                  usedTracks.insert(bestTrack.index);
                  addLyricsToScore(lyricTrack,
                                   bestTrack.matchedLyricTimes,
                                   tracks[bestTrack.index].staff);
                  }
            }
      }

std::vector<std::pair<ReducedFraction, ReducedFraction> > findMatchedLyricTimes(
            const std::multimap<ReducedFraction, MidiChord> &chords,
            const std::multimap<ReducedFraction, std::string> &lyricTrack)
      {
      std::vector<std::pair<ReducedFraction, ReducedFraction> > matchedLyricTimes;

      for (const auto &chord: chords) {
            for (const auto &note: chord.second.notes) {
                  if (lyricTrack.find(note.origOnTime) != lyricTrack.end()) {
                        matchedLyricTimes.push_back({chord.first, note.origOnTime});
                        break;
                        }
                  }
            }
      return matchedLyricTimes;
      }

void setLyricsFromOperations(const QList<MTrack> &tracks)
      {
      const auto *lyricTracks = preferences.midiImportOperations.getLyrics();
      if (!lyricTracks)
            return;
      for (const auto &track: tracks) {
            const auto opers = preferences.midiImportOperations.trackOperations(
                                                                  track.indexOfOperation);
            if (opers.lyricTrackIndex >= 0 && opers.lyricTrackIndex < lyricTracks->size()) {
                  const auto &lyricTrack = (*lyricTracks)[opers.lyricTrackIndex];
                  const auto matchedLyricTimes = findMatchedLyricTimes(track.chords, lyricTrack);

                  addLyricsToScore(lyricTrack, matchedLyricTimes, track.staff);
                  }
            }
      }

void setLyricsToScore(const QList<MTrack> &tracks)
      {
      if (preferences.midiImportOperations.count() == 0)
            setInitialLyricsFromMidiData(tracks);
      else
            setLyricsFromOperations(tracks);
      }

QList<std::string> makeLyricsListForUI()
      {
      QList<std::string> list;
      const auto *lyrics = preferences.midiImportOperations.getLyrics();
      if (!lyrics)
            return list;
      const unsigned int symbolLimit = 16;

      for (const auto &trackLyric: *lyrics) {
            std::string lyricText;

            for (const auto &lyric: trackLyric) {
                  const auto &text = removeSlashes(lyric.second);
                  if (isMetaText(text))
                        continue;
                  if (!lyricText.empty())
                        lyricText += " ";       // visual text delimeter
                  if (lyricText.size() + text.size() > symbolLimit)
                        lyricText += text.substr(0, symbolLimit - lyricText.size());
                  else
                        lyricText += text;
                  if (lyricText.size() >= symbolLimit - 1) {
                        lyricText += "...";
                        break;
                        }
                  }
            list.push_back(lyricText);
            }
      return list;
      }

} // namespace MidiLyrics
} // namespace Ms
