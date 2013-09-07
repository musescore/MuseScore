#include "importmidi_lyrics.h"
#include "importmidi_inner.h"
#include "libmscore/staff.h"
#include "libmscore/score.h"
#include "midi/midifile.h"
#include "importmidi_fraction.h"
#include "importmidi_chord.h"


namespace Ms {
namespace MidiLyrics {

bool isLyricsTrack(const MidiTrack &lyricsTrack)
      {
      bool lyricsFlag = false;
      bool noteFlag = false;

      for (const auto &i: lyricsTrack.events()) {
            const auto& e = i.second;
            if ((e.type() == ME_META) && (e.metaType() == META_TEXT
                                          || e.metaType() == META_LYRIC)) {
                  if (!lyricsFlag)
                        lyricsFlag = true;
                  }
            else if (e.type() == ME_NOTE) {
                  if (!noteFlag)
                        noteFlag = true;
                  break;
                  }
            }
      return !noteFlag && lyricsFlag;
      }

int findBestTrack(const QList<MTrack> &tracks,
                  const MidiFile *mf,
                  const MidiTrack &lyricsTrack)
      {
      int bestTrack = -1;
      int maxMatches = 0;

      for (int i = 0; i != tracks.size(); ++i) {
            int matches = 0;
            for (const auto &origEvt: lyricsTrack.events()) {
                  const auto &e = origEvt.second;
                  if ((e.type() == ME_META) && (e.metaType() == META_TEXT
                                                || e.metaType() == META_LYRIC)) {
                        const auto tick = toMuseScoreTicks(origEvt.first, mf->division());
                        const auto &chords = tracks[i].chords;
                        if (chords.find(tick) != chords.end())
                              ++matches;
                        }
                  }
            if (matches > maxMatches) {
                  maxMatches = matches;
                  bestTrack = i;
                  }
            }

      return bestTrack;
      }

void addLyrics(const MidiFile *mf,
               const MidiTrack &lyricsTrack,
               const MTrack &trackToInsertLyrics)
      {
      for (const auto &origEvt: lyricsTrack.events()) {
            const auto &e = origEvt.second;
            if ((e.type() == ME_META) && (e.metaType() == META_TEXT
                                          || e.metaType() == META_LYRIC)) {
                  const auto tick = toMuseScoreTicks(origEvt.first, mf->division());
                  const auto &chords = trackToInsertLyrics.chords;
                  if (chords.find(tick) != chords.end()) {
                        const uchar* data = (uchar*)e.edata();
                        const QString text((char*)data);
                        Staff *staff = trackToInsertLyrics.staff;
                        Score *score = staff->score();
                        score->addLyrics(tick.ticks(), staff->idx(), text);
                        }
                  }
            }
      }

void extractLyrics(const QList<MTrack> &tracks,
                   const MidiFile *mf)
      {
      for (const auto &t: mf->tracks()) {
            if (!isLyricsTrack(t))
                  continue;
                        // we have found track with lyrics and no note events
                        // now let's find track to insert that lyrics
                        // it will be the track with max onTime match cases
            int bestTrack = findBestTrack(tracks, mf, t);
            if (bestTrack >= 0)
                  addLyrics(mf, t, tracks[bestTrack]);
            }
      }

} // namespace MidiLyrics
} // namespace Ms
