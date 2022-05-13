/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_GLOBAL_XMLSTREAMREADER_H
#define MU_GLOBAL_XMLSTREAMREADER_H

#include <vector>

#include <QIODevice>
#include <QByteArray>

class QXmlStreamReader;

namespace mu {
class XmlStreamReader
{
public:

    enum TokenType {
        NoToken = 0,
        Invalid,
        StartDocument,
        EndDocument,
        StartElement,
        EndElement,
        Characters,
        Comment,
        DTD,
        EntityReference,
        ProcessingInstruction
    };

    enum Error {
        NoError,
        UnexpectedElementError,
        CustomError,
        NotWellFormedError,
        PrematureEndOfDocumentError
    };

    struct Attribute
    {
        QString name;
        QString value;
    };

    explicit XmlStreamReader(QIODevice* device);
    explicit XmlStreamReader(const QByteArray& data);
    virtual ~XmlStreamReader();

    void setData(const QByteArray& data);

    bool readNextStartElement();
    bool atEnd() const;
    void skipCurrentElement();
    TokenType readNext();
    TokenType tokenType() const;
    QString tokenString() const;
    bool isWhitespace() const;

    QStringRef name() const;

    QString attribute(const char* s) const;
    bool hasAttribute(const char* s) const;
    std::vector<Attribute> attributes() const;

    QString readElementText();
    QStringRef text() const;

    int64_t lineNumber() const;
    int64_t columnNumber() const;
    Error error() const;
    QString errorString() const;
    void raiseError(const QString& message = QString());

private:
    //! NOTE Temporary implementation
    QXmlStreamReader* m_reader = nullptr;
};
}

#endif // MU_GLOBAL_XMLSTREAMREADER_H
