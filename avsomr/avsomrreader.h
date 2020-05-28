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

#ifndef AVS_AVSOMRREADER_H
#define AVS_AVSOMRREADER_H

#include <QString>
#include <QMap>
#include <QList>
#include <QSet>
#include <QColor>

#include "avsomr.h"

class QXmlStreamReader;

namespace Ms {
namespace Avs {
class AvsOmrReader
{
public:
    AvsOmrReader();

    std::shared_ptr<AvsOmr> read(const QString& ormFilePath) const;
    std::shared_ptr<AvsOmr> read(QIODevice* ormFile) const;

private:
    QString makeSheetPath(uint16_t n) const;

    bool readBook(AvsOmr::Book& b, QIODevice* xmlData) const;
    bool readSheet(AvsOmr::Sheet& sh, QIODevice* xmlData) const;
    bool readPicture(QImage& img, QIODevice* xmlData) const;

    void readPage(AvsOmr::Page& p, QXmlStreamReader& xml) const;
    void readSystem(AvsOmr::System& sys, QXmlStreamReader& xml) const;
    void readStack(AvsOmr::MStack& st, QXmlStreamReader& xml) const;
    void readPart(AvsOmr::Part& pt, QXmlStreamReader& xml) const;
    void readStaff(AvsOmr::Staff& sf, QXmlStreamReader& xml) const;
    void readInters(AvsOmr::Inters& its, QXmlStreamReader& xml) const;
    void readBarline(AvsOmr::Barline& bl, QXmlStreamReader& xml) const;
    void readInterGlyphID(QSet<AvsOmr::ID>& gls, QXmlStreamReader& xml) const;

    void readGlyphs(AvsOmr::Sheet& sh, QXmlStreamReader& xml) const;
    void readImage(QImage& img, QXmlStreamReader& xml, const QColor& color) const;

    QList<AvsOmr::ID> toIDList(const QString& str) const;
    QSet<AvsOmr::ID> toIDSet(const QString& str) const;

    QColor glyphColor(AvsOmr::GlyphUsed used) const;
    mutable QMap<AvsOmr::GlyphUsed, QColor> _glyphColors;
};
} // Avs
} // Ms

#endif // AVS_AVSOMRREADER_H
