

#include "exportlrc.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/repeatlist.h"
#include "engraving/dom/lyrics.h"
#include <QBuffer>

using namespace mu::engraving;

namespace mu::iex::lrc
{
    // Interface implementation
    std::vector<project::INotationWriter::UnitType> ExportLRC::supportedUnitTypes() const
    {
        return {UnitType::PER_PART};
    }
    bool ExportLRC::supportsUnitType(UnitType ut) const { return ut == UnitType::PER_PART; }

    muse::Ret ExportLRC::write(notation::INotationPtr notation, muse::io::IODevice &device, const Options &)
    {
        Score *score = notation->elements()->msScore();
        QByteArray data;
        QBuffer buffer(&data);

        buffer.open(QIODevice::WriteOnly);

        writeMetadata(buffer, score);

        const auto lyrics = collectLyrics(score);

        // Write lyrics
        for (auto it = lyrics.constBegin(); it != lyrics.constEnd(); ++it)
        {
            buffer.write(QString("[%1]%2\n").arg(formatTimestamp(it.key()), it.value()).toUtf8());
        }

        device.write(data);
        return muse::Ret(muse::Ret::Code::Ok);
    }

    muse::Ret ExportLRC::writeList(const notation::INotationPtrList &, muse::io::IODevice &, const Options &)
    {
        return muse::Ret(muse::Ret::Code::NotSupported);
    }

    void ExportLRC::writeMetadata(QIODevice &device, const engraving::Score *score) const
    {
        QString metadata;

        // Title
        QString title = QString::fromStdString(score->metaTag(muse::String("workTitle")).toStdString());
        if (!title.isEmpty())
        {
            metadata += QString("[ti:%1]\n").arg(title);
        }

        // Composer/Artist
        QString artist = QString::fromStdString(score->metaTag(muse::String("composer")).toStdString());
        if (!artist.isEmpty())
        {
            metadata += QString("[ar:%1]\n").arg(artist);
        }

        if (!metadata.isEmpty())
        {
            device.write(metadata.toUtf8());
        }
    }

    // Core lyric collection (simplified)
    QMap<qreal, QString> ExportLRC::collectLyrics(const Score *score) const
    {
        QMap<qreal, QString> lyrics;
        const RepeatList &repeats = score->repeatList();

        for (const RepeatSegment *rs : repeats)
        {
            const int tickOffset = rs->utick - rs->tick;

            for (const MeasureBase *mb = rs->firstMeasure(); mb; mb = mb->next())
            {
                if (!mb->isMeasure())
                    continue;

                for (Segment *seg = toMeasure(mb)->first(); seg; seg = seg->next())
                {
                    if (!seg->isChordRestType())
                        continue;

                    for (EngravingItem *e : seg->elist())
                    {
                        if (!e || !e->isChordRest())
                            continue;

                        for (Lyrics *l : toChordRest(e)->lyrics())
                        {
                            // if (l->text().empty())
                            if (l->plainText().isEmpty())
                                continue;

                            const qreal time = score->utick2utime(l->tick().ticks() + tickOffset) * 1000;
                            lyrics.insert(time, l->plainText());
                        }
                    }
                }
            }
        }
        return lyrics;
    }

    QString ExportLRC::formatTimestamp(qreal ms) const
    {
        const int totalSec = static_cast<int>(ms / 1000);
        return QString("%1:%2.%3")
            .arg(totalSec / 60, 2, 10, QLatin1Char('0'))
            .arg(totalSec % 60, 2, 10, QLatin1Char('0'))
            .arg(static_cast<int>(ms) % 1000 / 10, 2, 10, QLatin1Char('0'));
    }
} // namespace mu::iex::lrc