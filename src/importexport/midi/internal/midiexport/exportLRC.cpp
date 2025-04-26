// CREATED BHY
#include "exportLRC.h"
#include "engraving/dom/chordrest.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/masterscore.h"
#include "engraving/compat/midi/compatmidirender.h"
#include <QFile>

using namespace mu::engraving;

/*
The purpose of this file is to add the feature of
exporting files with lyrics as LRC files.
*/
namespace mu::iex::LRC
{
    // PURPOSE: File writing the lyrics and timing.
    void ExportLRC::write(const QString &path, Score *score)
    {
        // Create file:
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly))
        {
            LOGE() << "Failed to create LRC file:" << path;
            return false;
        }

        // Get lyrics with timestamps:
        QMap<qreal, QString> lyrics = collectLyrics(score);

        // Write LRC file:
        writeMetadata(file, sscore);
        writeLyrics(file, lyrics);

        file.close();
        return true;
    }

    // PURPOSE: Writes metadata headers with the title, artist, and lyricist.
    void ExportLRC::writeMetadata(QFile &file, Score *score)
    {
        // Get metadata from score:
        QString title = score->metaTag("workTitle");
        QString artist = score->metaTag("composer");
        QString lyricist = score->metaTag("lyricist");

        // Write metadata:
        if (!title.isEmpty())
        {
            file.write("[ti:" + title.toUtf8() + "]\n");
        }
        if (!artist.isEmpty())
        {
            file.write("[ar:" + artist.toUtf8() + "]\n");
        }
        if (!lyricist.isEmpty())
        {
            file.write("[au:" + lyricist.toUtf8() + "]\n");
        }
    }

    // PURPOSE:Get lyrics with timestamps
    QMap<qreal, QString> ExportLRC::collectLyrics(Score *score)
    {
        QMap<qreal, QString> lyricsMap;
        const RepeatList &repeatList = score->repeatList();

        for (const RepeatSegment *rs : repeatList)
        {
            int tickOffset = rs->utick - rs->tick;

            for (MeasureBase *mb = rs->firstMeasure(); mb; mb = mb->next())
            {
                if (!mb->isMeasure())
                    continue;

                Measure *measure = toMeasure(mb);
                for (Segment *seg = measure->first(); seg; seg = seg->next())
                {
                    if (!seg->isChordRestType())
                        continue;

                    for (EngravingItem *e : seg->elist())
                    {
                        if (!e || !e->isChordRest())
                            continue;

                        ChordRest *cr = toChordRest(e);
                        for (Lyrics *lyric : cr->lyrics())
                        {
                            if (lyric->isEmpty())
                                continue;

                            // Calculate absolute tick position with repeat expansion
                            int utick = cr->tick().ticks() + tickOffset;
                            qreal ms = score->utick2utime(utick);
                            lyricsMap.insert(ms, lyric->plainText());
                        }
                    }
                }
            }
        }
        return lyricsMap;
    }

    // PURPOSE: Write lyrics with timestamps
    void ExportLRC::writeLyrics(QFile &file, const QMap<qreal, QString> &lyrics)
    {
        for (auto it = lyrics.constBegin(); it != lyrics.constEnd(); ++it)
        {
            QString line = QString("[%1]%2\n").arg(formatTimestamp(it.key())).arg(it.value());
            file.write(line.toUtf8());
        }
    }

    // PURPOSE: Convert milliseconds to [mm:ss.xx] format
    QString ExportLRC::formatTimestamp(qreal ms)
    {
        int totalSeconds = static_cast<int>(ms / 1000);
        int mm = totalSeconds / 60;
        int ss = totalSeconds % 60;
        int x = static_cast<int>(ms) % 1000 / 10;

        return QString("%1:%2.%3").arg(mm, 2, 10, QLatin1Char('0')).arg(ss, 2, 10, QLatin1Char('0')).arg(x, 2, 10, QLatin1Char('0'));
    }
} // namespace mu::iex::lrc
