
#ifndef EXPORTLRC_H
#define EXPORTLRC_H

#include <QMap>
#include <QString>

namespace mu::engraving
{
    class Score;
}
namespace mu::iex::LRC
{

    //---------------------------------------------------------
    //   ExportLRC
    //---------------------------------------------------------
    class ExportLRC
    {
    public:
        static bool write(const QString &path, mu::engraving::Score *score);

    private:
        static void writeMetadata(QFile &file, mu::engraving::Score *score);

        static QMap<qreal, QString> collectLyrics(mu::engraving::Score *score);
        static void writeLyrics(QFile &file, const QMap<qreal, QString> &lyrics);
        static QString formatTimestamp(qreal ms);
    };
}

#endif // EXPORTLRC_H
