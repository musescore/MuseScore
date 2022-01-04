#include "qimageprovider.h"

#include <QBuffer>

#include "qimagepainterprovider.h"
#include "draw/pixmap.h"
#include "log.h"

using namespace mu::draw;

static const char FILE_FORMAT[] = "PNG";

std::shared_ptr<Pixmap> QImageProvider::createPixmap(const QByteArray& data) const
{
    QImage image;
    image.loadFromData(data);

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

// Temporary method, because now both our Pixmap and QImage can be used
std::shared_ptr<Pixmap> QImageProvider::pixmapFromQVariant(const QVariant& val)
{
    IF_ASSERT_FAILED(val.canConvert<Pixmap>() || val.canConvert<QImage>()) {
    }

    if (val.canConvert<Pixmap>()) {
        return std::make_shared<Pixmap>(val.value<Pixmap>());
    } else if (val.canConvert<QImage>()) {
        return std::make_shared<Pixmap>(Pixmap::fromQPixmap(QPixmap::fromImage(val.value<QImage>())));
    }
    return nullptr;
}

std::shared_ptr<IPaintProvider> QImageProvider::painterForImage(std::shared_ptr<Pixmap> pixmap)
{
    return QImagePainterProvider::make(pixmap);
}

void QImageProvider::saveAsPng(std::shared_ptr<Pixmap> px, QIODevice* device)
{
    Pixmap::toQPixmap(*px).save(device, FILE_FORMAT);
}
