#include "importmidi_lyrics.h"
#include "importmidi_inner.h"
#include "libmscore/staff.h"
#include "libmscore/score.h"
#include "midi/midifile.h"
#include "importmidi_fraction.h"
#include "importmidi_chord.h"
#include "libmscore/text.h"
#include "libmscore/measurebase.h"
#include "libmscore/element.h"
#include "libmscore/box.h"


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
            if (tracks[i].mtrack->drumTrack())
                  continue;

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

void addTitle(Score *score, const MidiEvent &e, int *textCounter)
      {
      const uchar* data = (uchar*)e.edata();
      QString string((char*)data);
      const QString textSign = "@T";

      if (string.left(textSign.size()) == textSign) {
            ++*textCounter;
            Text* text = new Text(score);
            if (*textCounter == 1)
                  text->setTextStyleType(TEXT_STYLE_TITLE);
            else if (*textCounter == 2)
                  text->setTextStyleType(TEXT_STYLE_COMPOSER);
            text->setText(string.right(string.size() - textSign.size()));

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

void addLyrics(const MidiFile *mf,
               const MidiTrack &lyricsTrack,
               const Staff *staffAddTo)
      {
      Score *score = staffAddTo->score();
      int textCounter = 0;

      for (const auto &origEvt: lyricsTrack.events()) {
            const auto &e = origEvt.second;
            if ((e.type() == ME_META) && (e.metaType() == META_TEXT
                                          || e.metaType() == META_LYRIC)) {
                  const auto tick = toMuseScoreTicks(origEvt.first, mf->division());
                  if (tick == ReducedFraction(0, 1)) {
                        addTitle(score, e, &textCounter);
                        }
                  else {
                        const uchar* data = (uchar*)e.edata();
                        QString text((char*)data);
                                    // remove slashes in kar format
                        text = text.replace("/", "");
                        text = text.replace("\\", "");
                        score->addLyrics(tick.ticks(), staffAddTo->idx(), text);
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
                  addLyrics(mf, t, tracks[bestTrack].staff);
            }
      }

} // namespace MidiLyrics
} // namespace Ms
