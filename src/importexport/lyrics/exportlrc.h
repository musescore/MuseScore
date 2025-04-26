#ifndef EXPORTLRC_H
#define EXPORTLRC_H

#include "project/inotationwriter.h"
#include <QIODevice>

namespace mu::engraving
{
    class Score;
}

namespace mu::iex::lrc
{
    class ExportLRC : public project::INotationWriter
    {
    public:
        // Interface implementation
        std::vector<UnitType> supportedUnitTypes() const override;
        bool supportsUnitType(UnitType) const override;
        muse::Ret write(notation::INotationPtr, muse::io::IODevice &, const Options &) override;
        void writeMetadata(QIODevice &device, const engraving::Score *score) const;
        muse::Ret writeList(const notation::INotationPtrList &, muse::io::IODevice &, const Options &) override;

    private:
        // Core lyric functionality
        QMap<qreal, QString> collectLyrics(const engraving::Score *) const;
        QString formatTimestamp(qreal ms) const;
    };
} // namespace mu::iex::lrc
#endif