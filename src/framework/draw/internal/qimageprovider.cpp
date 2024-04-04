#include "qimageprovider.h"

#include <QBuffer>

#include "qimagepainterprovider.h"
#include "types/pixmap.h"

#include "log.h"

using namespace muse::draw;

static const char FILE_FORMAT[] = "PNG";

std::shared_ptr<Pixmap> QImageProvider::createPixmap(const ByteArray& data) const
{
    QImage image;
    image.loadFromData(data.toQByteArrayNoCopy());

    return std::make_shared<Pixmap>(Pixmap::fromQPixmap(QPixmap::fromImage(image)));
}

std::shared_ptr<Pixmap> QImageProvider::createPixmap(int w, int h, int dpm, const Color& color) const
{
    QImage image(w, h, QImage::Format_ARGB32_Premultiplied);
    image.setDotsPerMeterX(dpm);
    image.setDotsPerMeterY(dpm);
    image.fill(color.toQColor());

    return std::make_shared<Pixmap>(Pixmap::fromQPixmap(QPixmap::fromImage(image)));
}

Pixmap QImageProvider::scaled(const Pixmap& origin, const Size& s) const
{
    QPixmap qtPixmap = Pixmap::toQPixmap(origin);
    qtPixmap = qtPixmap.scaled(s.width(), s.height());

    return Pixmap::fromQPixmap(qtPixmap);
}

std::shared_ptr<IPaintProvider> QImageProvider::painterForImage(std::shared_ptr<Pixmap> pixmap)
{
    return QImagePainterProvider::make(pixmap);
}

void QImageProvider::saveAsPng(std::shared_ptr<Pixmap> px, io::IODevice* device)
{
    QBuffer buf;
    buf.open(QIODevice::WriteOnly);
    Pixmap::toQPixmap(*px).save(&buf, FILE_FORMAT);
    device->write(buf.data());
}
