#include "qimageconverter.h"

#include <QPixmap>
#include <QBuffer>

using namespace mu::draw;

Pixmap QImageConverter::scaled(const Pixmap& origin, const Size& s) const
{
    QPixmap qtPixMap;
    qtPixMap.loadFromData(origin.data());
    qtPixMap = qtPixMap.scaled(s.width(), s.height());

    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    qtPixMap.save(&buffer, "PNG");

    Pixmap result({ qtPixMap.width(), qtPixMap.height() });
    result.setData(bytes);

    return result;
}
