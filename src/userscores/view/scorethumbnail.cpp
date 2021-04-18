#include "scorethumbnail.h"

#include <QVariant>

using namespace mu::userscores;

ScoreThumbnail::ScoreThumbnail(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
}

void ScoreThumbnail::setThumbnail(QVariant pixmap)
{
    if (pixmap.isNull()) {
        return;
    }

    m_thumbnail = pixmap.value<QPixmap>();
    update();
}

void ScoreThumbnail::paint(QPainter* painter)
{
    painter->drawPixmap(0, 0, width(), height(), m_thumbnail);
}
