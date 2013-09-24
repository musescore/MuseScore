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

const QString META_PREFIX = "@";
const QString TEXT_PREFIX = "@T";


bool isMetaText(const QString &text)
      {
      return text.left(META_PREFIX.size()) == META_PREFIX;
      }

bool isLyricText(const QString &text)
      {
      return !isMetaText(text) || text.left(TEXT_PREFIX.size()) == TEXT_PREFIX;
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
                  QString text((char*)data);
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

std::multimap<ReducedFraction, QString>
extractLyricsFromTrack(const MidiTrack &lyricsTrack, int division)
      {
      std::multimap<ReducedFraction, QString> lyrics;

      for (const auto &i: lyricsTrack.events()) {
            const auto& e = i.second;
            if (isLyricEvent(e)) {
                  const uchar* data = (uchar*)e.edata();
                  QString text((char*)data);
                  if (isLyricText(text)) {
                        const auto tick = toMuseScoreTicks(i.first, division);
                        lyrics.insert({tick, text});
                        }
                  }
            }
      return lyrics;
      }

// find track to insert lyrics
// it will be the track with max onTime match cases, except drum tracks

int findBestTrack(const QList<MTrack> &tracks,
                  const std::multimap<ReducedFraction, QString> &lyricTrack,
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
      if (string.left(TEXT_PREFIX.size()) == TEXT_PREFIX) {
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

QString removeSlashes(const QString &text)
      {
      QString newText = text;
                  // remove slashes in kar format
      newText = newText.replace("/", "");
      newText = newText.replace("\\", "");
      return newText;
      }

void addLyricsToScore(const std::multimap<ReducedFraction, QString> &lyricTrack,
                      const Staff *staffAddTo)
      {
      Score *score = staffAddTo->score();
      int textCounter = 0;

      for (const auto &e: lyricTrack) {
            const auto &tick = e.first;
            if (tick == ReducedFraction(0, 1)) {
                  addTitle(score, e.second, &textCounter);
                  }
            else {
                  QString text = removeSlashes(e.second);
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
            MidiLyrics::extractLyricsToMidiData(mf);
            setInitialLyricsFromMidiData(tracks);
            }
      else {
            setLyricsFromOperations(tracks);
            }
      }

QStringList makeLyricsList()
      {
      QStringList list;
      const auto *lyrics = preferences.midiImportOperations.getLyrics();
      if (!lyrics)
            return list;
      const int symbolLimit = 20;

      for (const auto &trackLyric: *lyrics) {
            QString lyricText;

            for (const auto &lyric: trackLyric) {
                  const QString &text = removeSlashes(lyric.second);
                  if (isMetaText(text))
                        continue;
                  if (!lyricText.isEmpty())
                        lyricText += " ";       // visual text delimeter
                  if (lyricText.size() + text.size() > symbolLimit)
                        lyricText += text.left(symbolLimit - lyricText.size());
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
