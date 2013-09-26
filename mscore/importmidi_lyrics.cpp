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

bool isLyricsTrack(const MidiTrack &lyricsTrack)
      {
      bool lyricsFlag = false;

      for (const auto &i: lyricsTrack.events()) {
            const auto& e = i.second;
            if (isLyricEvent(e)) {
                  const uchar* data = (uchar*)e.edata();
                              // no charset handling here
                  std::string text = MidiCharset::fromUchar(data);
                  if (isLyricText(text)) {
                        if (!lyricsFlag)
                              lyricsFlag = true;
                        }
                  }
            else if (e.type() == ME_NOTE)
                  return false;
            }

      return lyricsFlag;
      }

std::multimap<ReducedFraction, std::string>
extractLyricsFromTrack(const MidiTrack &lyricsTrack, int division)
      {
      std::multimap<ReducedFraction, std::string> lyrics;

      for (const auto &i: lyricsTrack.events()) {
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

// find track to insert lyrics
// it will be the track with max onTime match cases, except drum tracks

int findBestTrack(const QList<MTrack> &tracks,
                  const std::multimap<ReducedFraction, std::string> &lyricTrack,
                  const std::set<int> &usedTracks)
      {
      int bestTrack = -1;
      int maxMatches = 0;

      for (int i = 0; i != tracks.size(); ++i) {
            if (tracks[i].mtrack->drumTrack() || usedTracks.find(i) != usedTracks.end())
                  continue;

            int matches = 0;
            for (const auto &e: lyricTrack) {
                  const auto &chords = tracks[i].chords;
                  if (chords.find(e.first) != chords.end())
                        ++matches;
                  }
            if (matches > maxMatches) {
                  maxMatches = matches;
                  bestTrack = i;
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
                  text->setTextStyleType(TEXT_STYLE_TITLE);
            else if (*textCounter == 2)
                  text->setTextStyleType(TEXT_STYLE_COMPOSER);
            text->setText(string.right(string.size() - TEXT_PREFIX.size()));

            MeasureBase* measure = score->first();
            if (measure->type() != Element::VBOX) {
                  measure = new VBox(score);
                  measure->setTick(0);
                  measure->setNext(score->first());
                  score->add(measure);
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

void addLyricsToScore(const std::multimap<ReducedFraction, std::string> &lyricTrack,
                      const Staff *staffAddTo)
      {
      Score *score = staffAddTo->score();
      int textCounter = 0;

      for (const auto &e: lyricTrack) {
            const auto &tick = e.first;
            QString str = MidiCharset::convertToCharset(e.second);
            if (tick == ReducedFraction(0, 1)) {
                  addTitle(score, str, &textCounter);
                  }
            else {
                  QString text = removeSlashes(str);
                  score->addLyrics(tick.ticks(), staffAddTo->idx(), text);
                  }
            }
      }

void extractLyricsToMidiData(const MidiFile *mf)
      {
      for (const auto &t: mf->tracks()) {
            if (!isLyricsTrack(t))
                  continue;
            preferences.midiImportOperations.addTrackLyrics(
                                     extractLyricsFromTrack(t, mf->division()));
            }
      }

void setInitialIndexes(QList<MTrack> &tracks)
      {
      std::set<int> usedTracks;
      const auto *lyricTracks = preferences.midiImportOperations.getLyrics();
      if (!lyricTracks)
            return;

      for (int i = 0; i != lyricTracks->size(); ++i) {
            const int bestTrack = findBestTrack(tracks, (*lyricTracks)[i], usedTracks);
            if (bestTrack >= 0) {
                  usedTracks.insert(bestTrack);
                  tracks[bestTrack].initLyricTrackIndex = i;
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
            const int bestTrack = findBestTrack(tracks, lyricTrack, usedTracks);
            if (bestTrack >= 0) {
                  usedTracks.insert(bestTrack);
                  addLyricsToScore(lyricTrack, tracks[bestTrack].staff);
                  }
            }
      }

void setLyricsFromOperations(const QList<MTrack> &tracks)
      {
      const auto *lyricTracks = preferences.midiImportOperations.getLyrics();
      if (!lyricTracks)
            return;
      for (const auto &track: tracks) {
            const auto opers
                  = preferences.midiImportOperations.trackOperations(track.indexOfOperation);
            if (opers.lyricTrackIndex >= 0 && opers.lyricTrackIndex < lyricTracks->size())
                  addLyricsToScore((*lyricTracks)[opers.lyricTrackIndex], track.staff);
            }
      }

void setLyrics(const MidiFile *mf, const QList<MTrack> &tracks)
      {
      if (preferences.midiImportOperations.count() == 0) {
            extractLyricsToMidiData(mf);
            setInitialLyricsFromMidiData(tracks);
            }
      else {
            setLyricsFromOperations(tracks);
            }
      }

QList<std::string> makeLyricsList()
      {
      QList<std::string> list;
      const auto *lyrics = preferences.midiImportOperations.getLyrics();
      if (!lyrics)
            return list;
      const unsigned int symbolLimit = 20;

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
