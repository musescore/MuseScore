//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "avsomrreader.h"

#include <QFile>
#include <QBuffer>
#include <QXmlStreamReader>
#include <QPainter>

#include "thirdparty/qzip/qzipreader_p.h"
#include "mscore/preferences.h"

#include "avslog.h"

using namespace Ms::Avs;

AvsOmrReader::AvsOmrReader()
{
}

//---------------------------------------------------------
//   glyphColor
//---------------------------------------------------------

QColor AvsOmrReader::glyphColor(AvsOmr::GlyphUsed used) const
{
    if (_glyphColors.isEmpty()) {
        _glyphColors[AvsOmr::GlyphUsed::Used] = preferences.getColor(PREF_UI_AVSOMR_RECOGNITION_COLOR);
        _glyphColors[AvsOmr::GlyphUsed::Free] = preferences.getColor(PREF_UI_AVSOMR_NOT_RECOGNITION_COLOR);
        _glyphColors[AvsOmr::GlyphUsed::Free_Covered] = preferences.getColor(PREF_UI_AVSOMR_NOT_RECOGNITION_COLOR);
    }

    return _glyphColors.value(used);
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

std::shared_ptr<AvsOmr> AvsOmrReader::read(const QString& ormFilePath) const
{
    QFile zipFile(ormFilePath);
    if (!zipFile.exists()) {
        LOGE() << "not exists orm file: " << zipFile.fileName();
        return nullptr;
    }

    if (!zipFile.open(QIODevice::ReadOnly)) {
        LOGE() << "failed open orm file: " << zipFile.fileName();
        return nullptr;
    }

    return read(&zipFile);
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

std::shared_ptr<AvsOmr> AvsOmrReader::read(QIODevice* ormFile) const
{
    MQZipReader zip(ormFile);

    QByteArray book = zip.fileData("book.xml");
    if (book.isEmpty()) {
        LOGE() << "not exists book.xml";
        return nullptr;
    }

    QBuffer bookBuf(&book);
    bookBuf.open(QIODevice::ReadOnly);

    std::shared_ptr<AvsOmr> omr = std::make_shared<AvsOmr>();

    bool ok = readBook(omr->_book, &bookBuf);
    if (!ok) {
        LOGE() << "failed read book.xml";
        return nullptr;
    }

    for (uint16_t n = 1; n <= omr->_book.sheets; ++n) {
        QString path = makeSheetPath(n);
        QByteArray sheet = zip.fileData(path);
        if (sheet.isEmpty()) {
            LOGE() << "not exists sheet: " << path;
            continue;
        }
        QBuffer sheetBuf(&sheet);
        sheetBuf.open(QIODevice::ReadOnly);

        AvsOmr::Sheet* sh = new AvsOmr::Sheet();
        ok = readSheet(*sh, &sheetBuf);
        if (!ok) {
            LOGE() << "failed read sheet: " << path;
            continue;
        }

        omr->_sheets.insert(sh->num, sh);
    }

    omr->resolve();

    AvsOmr::Info& info = omr->_info;
    info.usedColor = glyphColor(AvsOmr::GlyphUsed::Used);
    info.freeColor = glyphColor(AvsOmr::GlyphUsed::Free);

    for (const AvsOmr::Sheet* sh : omr->_sheets) {
        for (AvsOmr::Glyph* g : sh->glyphs) {
            if (AvsOmr::GlyphUsed::Used == g->used) {
                ++info.usedCount;
            } else if (AvsOmr::GlyphUsed::Free == g->used) {
                ++info.freeCount;
            }
        }
    }

    return omr;
}

//---------------------------------------------------------
//   makeSheetPath
//---------------------------------------------------------

QString AvsOmrReader::makeSheetPath(uint16_t n) const
{
    QString name = "sheet#" + QString::number(n);
    return name + "/" + name + ".xml";
}

//---------------------------------------------------------
//   readBook
//---------------------------------------------------------

bool AvsOmrReader::readBook(AvsOmr::Book& b, QIODevice* xmlData) const
{
    QXmlStreamReader xml(xmlData);
    if (xml.readNextStartElement()) {
        if (!(xml.name() == "book")) {
            return false;
        }

        while (xml.readNextStartElement()) {
            QStringRef tag = xml.name();
            if ("sheet" == tag) {
                ++b.sheets;
                xml.skipCurrentElement();
            } else {
                xml.skipCurrentElement();
            }
        }
    }

    bool ok = !xml.hasError();
    if (!ok) {
        LOGE() << "failed read xml, err: " << xml.errorString();
    }

    return ok;
}

//---------------------------------------------------------
//   readSheet
//---------------------------------------------------------

bool AvsOmrReader::readSheet(AvsOmr::Sheet& sh, QIODevice* xmlData) const
{
    QXmlStreamReader xml(xmlData);
    if (xml.readNextStartElement()) {
        if (xml.name() == "sheet") {
            sh.num = xml.attributes().value("number").toInt();

            while (xml.readNextStartElement()) {
                QStringRef tag = xml.name();
                if (tag == "page") {
                    readPage(sh.page, xml);
                } else if (tag == "glyph-index") {
                    readGlyphs(sh, xml);
                } else {
                    xml.skipCurrentElement();
                }
            }
        }
    }

    bool ok = !xml.hasError();
    if (!ok) {
        LOGE() << "failed read xml, err: " << xml.errorString();
    }

    return ok;
}

//---------------------------------------------------------
//   readPage
//---------------------------------------------------------

void AvsOmrReader::readPage(AvsOmr::Page& p, QXmlStreamReader& xml) const
{
    while (xml.readNextStartElement()) {
        if (xml.name() == "system") {
            AvsOmr::System sys;
            readSystem(sys, xml);
            p.systems.append(sys);
        } else {
            xml.skipCurrentElement();
        }
    }
}

//---------------------------------------------------------
//   readSystem
//---------------------------------------------------------

void AvsOmrReader::readSystem(AvsOmr::System& sys, QXmlStreamReader& xml) const
{
    while (xml.readNextStartElement()) {
        auto tag = xml.name();
        if (tag == "stack") {
            AvsOmr::MStack st;
            readStack(st, xml);
            sys.mstacks.append(st);
        } else if (tag == "part") {
            readPart(sys.part, xml);
        } else if (tag == "free-glyphs") {
            QString str = xml.readElementText();
            sys.freeglyphs = toIDSet(str);
        } else if (tag == "sig") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "inters") {
                    readInters(sys.inters, xml);
                } else {
                    xml.skipCurrentElement();
                }
            }
        } else {
            xml.skipCurrentElement();
        }
    }
}

//---------------------------------------------------------
//   readStack
//---------------------------------------------------------

void AvsOmrReader::readStack(AvsOmr::MStack& st, QXmlStreamReader& xml) const
{
    auto attrs = xml.attributes();
    st.id = attrs.value("id").toInt();
    st.left = attrs.value("left").toInt();
    st.right = attrs.value("right").toInt();

    xml.skipCurrentElement();
}

//---------------------------------------------------------
//   readPart
//---------------------------------------------------------

void AvsOmrReader::readPart(AvsOmr::Part& pt, QXmlStreamReader& xml) const
{
    while (xml.readNextStartElement()) {
        auto tag = xml.name();
        if (tag == "staff") {
            AvsOmr::Staff sf;
            readStaff(sf, xml);
            pt.staffs.append(sf);
        } else {
            xml.skipCurrentElement();
        }
    }
}

//---------------------------------------------------------
//   readStaff
//---------------------------------------------------------

void AvsOmrReader::readStaff(AvsOmr::Staff& sf, QXmlStreamReader& xml) const
{
    auto attrs = xml.attributes();
    sf.id = attrs.value("id").toInt();

    while (xml.readNextStartElement()) {
        auto tag = xml.name();
        if (tag == "header") {
            sf.header.start = xml.attributes().value("start").toInt();
            sf.header.stop = xml.attributes().value("stop").toInt();
            while (xml.readNextStartElement()) {
                if (xml.name() == "clef") {
                    sf.header.clefID = xml.readElementText().toInt();
                } else if (xml.name() == "key") {
                    sf.header.keyID = xml.readElementText().toInt();
                } else if (xml.name() == "time") {
                    sf.header.timeID = xml.readElementText().toInt();
                } else {
                    xml.skipCurrentElement();
                }
            }
        } else if (tag == "barlines") {
            QString str = xml.readElementText();
            sf.barlines = toIDList(str);
        } else {
            xml.skipCurrentElement();
        }
    }
}

//---------------------------------------------------------
//   readInters
//---------------------------------------------------------

void AvsOmrReader::readInters(AvsOmr::Inters& its, QXmlStreamReader& xml) const
{
    while (xml.readNextStartElement()) {
        auto tag = xml.name();
        if ("barline" == tag) {
            AvsOmr::Barline bl;
            readBarline(bl, xml);
            its.barlines.insert(bl.id, bl);
        } else if ("brace" == tag) {
            readInterGlyphID(its.usedglyphs, xml);
        } else if ("clef" == tag) {
            readInterGlyphID(its.usedglyphs, xml);
        } else if ("time-whole" == tag) {
            readInterGlyphID(its.usedglyphs, xml);
        } else if ("beam" == tag) {
            readInterGlyphID(its.usedglyphs, xml);
        } else if ("ledger" == tag) {
            readInterGlyphID(its.usedglyphs, xml);
        } else if ("head" == tag) {
            readInterGlyphID(its.usedglyphs, xml);
        } else if ("stem" == tag) {
            readInterGlyphID(its.usedglyphs, xml);
        } else if ("dynamics" == tag) {
            readInterGlyphID(its.usedglyphs, xml);
        } else if ("alter" == tag) {
            readInterGlyphID(its.usedglyphs, xml);
        } else {
            xml.skipCurrentElement();
        }
    }
}

//---------------------------------------------------------
//   readBarline
//---------------------------------------------------------

void AvsOmrReader::readBarline(AvsOmr::Barline& bl, QXmlStreamReader& xml) const
{
    auto attrs = xml.attributes();
    bl.id = attrs.value("id").toInt();
    bl.glyphID = attrs.value("glyph").toInt();

    xml.skipCurrentElement();
}

//---------------------------------------------------------
//   readInterGlyphID
//---------------------------------------------------------

void AvsOmrReader::readInterGlyphID(QSet<AvsOmr::ID>& gls, QXmlStreamReader& xml) const
{
    auto attrs = xml.attributes();
    AvsOmr::ID glyphID = attrs.value("glyph").toInt();
    gls.insert(glyphID);

    xml.skipCurrentElement();
}

//---------------------------------------------------------
//   toIDList
//---------------------------------------------------------

QList<AvsOmr::ID> AvsOmrReader::toIDList(const QString& str) const
{
    if (str.isEmpty()) {
        return QList<AvsOmr::ID>();
    }

    QList<AvsOmr::ID> ids;
    QStringList strList = str.split(' ');
    for (const QString& s : strList) {
        ids.append(s.toInt());
    }

    return ids;
}

//---------------------------------------------------------
//   toIDSet
//---------------------------------------------------------

QSet<AvsOmr::ID> AvsOmrReader::toIDSet(const QString& str) const
{
    if (str.isEmpty()) {
        return QSet<AvsOmr::ID>();
    }

    QSet<AvsOmr::ID> ids;
    QStringList strList = str.split(' ');
    for (const QString& s : strList) {
        ids.insert(s.toInt());
    }

    return ids;
}

//---------------------------------------------------------
//   readGlyphs
//---------------------------------------------------------

void AvsOmrReader::readGlyphs(AvsOmr::Sheet& sh, QXmlStreamReader& xml) const
{
    while (xml.readNextStartElement()) {
        if (xml.name() == "glyph") {
            auto attrs = xml.attributes();
            AvsOmr::Glyph* g = new AvsOmr::Glyph();
            g->bbox.setLeft(attrs.value("left").toInt());
            g->bbox.setTop(attrs.value("top").toInt());
            g->id = attrs.value("id").toInt();

            if (sh.isGlyphUsed(g->id)) {
                g->used = AvsOmr::GlyphUsed::Used;
            } else if (sh.isGlyphFree(g->id)) {
                g->used = AvsOmr::GlyphUsed::Free;
            }

            while (xml.readNextStartElement()) {
                if (xml.name() == "run-table") {
                    readImage(g->img, xml, glyphColor(g->used));
                } else {
                    xml.skipCurrentElement();
                }
            }

            g->bbox.setRight(g->bbox.left() + g->img.width());
            g->bbox.setBottom(g->bbox.top() + g->img.height());

            sh.glyphs.insert(g->id, g);
        } else {
            xml.skipCurrentElement();
        }
    }
}

//---------------------------------------------------------
//   readImage
//---------------------------------------------------------

void AvsOmrReader::readImage(QImage& img, QXmlStreamReader& xml, const QColor& color) const
{
    IF_ASSERT(xml.isStartElement() && xml.name() == "run-table") {
        return;
    }

    auto attrs = xml.attributes();
    int width = attrs.value("width").toInt();
    int height = attrs.value("height").toInt();
    bool isVertical = attrs.value("orientation") == "VERTICAL";

    img = QImage(width, height, QImage::Format_ARGB32);
    img.fill(Qt::transparent);

    QPainter p(&img);
    p.setPen(color);

    auto drawLine = [&p, isVertical](int i, const QString& line) {
                        if (line.isEmpty()) {
                            return;
                        }

                        QStringList segs = line.split(' ');
                        bool isFill{ true };
                        int j{ 0 };
                        for (const QString& seg : segs) {
                            int length = seg.toInt();
                            if (length > 0 && isFill) {
                                if (isVertical) {
                                    p.drawLine(i, j, i, j + length);
                                } else {
                                    p.drawLine(j, i, j + length, i);
                                }
                            }
                            j += length;
                            isFill = !isFill;
                        }
                    };

    int i = 0;

    while (xml.readNextStartElement()) {
        if (xml.name() == "runs") {
            QString line = xml.readElementText();
            drawLine(i, line);
            ++i;
        } else {
            xml.skipCurrentElement();
        }
    }
}

//---------------------------------------------------------
//   readPicture
//---------------------------------------------------------

bool AvsOmrReader::readPicture(QImage& img, QIODevice* xmlData) const
{
    QXmlStreamReader xml(xmlData);
    while (xml.readNextStartElement()) {
        if (xml.name() == "run-table") {
            readImage(img, xml, QColor(Qt::darkBlue));
        } else {
            xml.skipCurrentElement();
        }
    }

    return true;
}
