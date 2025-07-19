/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <QDateTime>

#include "util.h"

#include "score.h"

#include "engraving/dom/measurebase.h"
#include "engraving/dom/page.h"
#include "engraving/dom/score.h"
#include "engraving/dom/system.h"

using namespace mu;

namespace mu::plugins::api {
//---------------------------------------------------------
//   ScoreView
//---------------------------------------------------------

ScoreView::ScoreView(QQuickItem* parent)
    : uicomponents::QuickPaintedView(parent)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    score = nullptr;
}

//---------------------------------------------------------
//   FileIO
//---------------------------------------------------------

FileIO::FileIO(QObject* parent)
    : QObject(parent)
{
}

QString FileIO::read()
{
    if (m_source.isEmpty()) {
        emit error("source is empty");
        return QString();
    }
    QUrl url(m_source);
    QString source(m_source);
    if (url.isValid() && url.isLocalFile()) {
        source = url.toLocalFile();
    }
    QFile file(source);
    QString fileContent;
    if (file.open(QIODevice::ReadOnly)) {
        QString line;
        QTextStream t(&file);
        do {
            line = t.readLine();
            fileContent += line + "\n";
        } while (!line.isNull());
        file.close();
    } else {
        emit error("Unable to open the file");
        return QString();
    }
    return fileContent;
}

bool FileIO::write(const QString& data)
{
    if (m_source.isEmpty()) {
        return false;
    }

    QUrl url(m_source);

    QString source = (url.isValid() && url.isLocalFile()) ? url.toLocalFile() : m_source;

    QFile file(source);
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        return false;
    }

    QTextStream out(&file);
    out << data;
    file.close();
    return true;
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

bool FileIO::remove()
{
    if (m_source.isEmpty()) {
        return false;
    }

    QFile file(m_source);
    return file.remove();
}

bool FileIO::exists()
{
    QFile file(m_source);
    return file.exists();
}

int FileIO::modifiedTime()
{
    if (m_source.isEmpty()) {
        emit error("source is empty");
        return 0;
    }
    QUrl url(m_source);
    QString source(m_source);
    if (url.isValid() && url.isLocalFile()) {
        source = url.toLocalFile();
    }
    QFileInfo fileInfo(source);
    return fileInfo.lastModified().toSecsSinceEpoch();
}

void MsProcess::start(const QString& command)
{
    QT_WARNING_PUSH;
    QT_WARNING_DISABLE_DEPRECATED;
    DEPRECATED_USE("startWithArgs(program, [arg1, arg2, ...])");
    QProcess::start(command);
    QT_WARNING_POP;
}

void MsProcess::startWithArgs(const QString& program, const QStringList& args)
{
    QProcess::start(program, args, ReadWrite);
}

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void ScoreView::setScore(mu::plugins::api::Score* s)
{
    mu::engraving::Score* newScore = s ? s->score() : nullptr;
    setScore(newScore);
}

void ScoreView::setScore(mu::engraving::Score* s)
{
    MuseScoreView::setScore(s);
    m_currentPage = 0;
    score = s;

    if (score) {
        score->doLayout();

        mu::engraving::Page* page = score->pages()[m_currentPage];
        RectF pr(page->pageBoundingRect());
        qreal m1 = width() / pr.width();
        qreal m2 = height() / pr.height();
        mag = qMax(m1, m2);

        m_boundingRect = QRectF(0.0, 0.0, pr.width() * mag, pr.height() * mag);

        setWidth(pr.width() * mag);
        setHeight(pr.height() * mag);
    }
    update();
}

//---------------------------------------------------------
//   paint
//---------------------------------------------------------

void ScoreView::paint(QPainter* qp)
{
    muse::draw::Painter p(qp, "plugins_scoreview");
    p.setAntialiasing(true);
    p.fillRect(RectF(0.0, 0.0, width(), height()), m_color);
    if (!score) {
        return;
    }
    p.scale(mag, mag);

    mu::engraving::Page* page = score->pages()[m_currentPage];
    QList<const mu::engraving::EngravingItem*> el;
    for (engraving::System* s : page->systems()) {
        for (engraving::MeasureBase* m : s->measures()) {
            m->scanElements(&el, mu::engraving::collectElements, false);
        }
    }
    page->scanElements(&el, mu::engraving::collectElements, false);

    foreach (const mu::engraving::EngravingItem* e, el) {
        PointF pos(e->pagePos());
        p.translate(pos);
        mu::engraving::EngravingItem::renderer()->drawItem(e, &p);
        p.translate(-pos);
    }
}

//---------------------------------------------------------
//   setCurrentPage
//---------------------------------------------------------

void ScoreView::setCurrentPage(int n)
{
    if (!score) {
        return;
    }
    if (n < 0) {
        n = 0;
    }
    int nn = static_cast<int>(score->pages().size());
    if (nn == 0) {
        return;
    }
    if (n >= nn) {
        n = nn - 1;
    }
    m_currentPage = n;
    update();
}

//---------------------------------------------------------
//   nextPage
//---------------------------------------------------------

void ScoreView::nextPage()
{
    setCurrentPage(m_currentPage + 1);
}

//---------------------------------------------------------
//   prevPage
//---------------------------------------------------------

void ScoreView::prevPage()
{
    setCurrentPage(m_currentPage - 1);
}
} // namespace mu::plugins::api
