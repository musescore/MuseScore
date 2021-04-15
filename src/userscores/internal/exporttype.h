//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_USERSCORES_EXPORTTYPE_H
#define MU_USERSCORES_EXPORTTYPE_H

namespace mu::userscores {
struct ExportType;
class ExportTypeList : public QList<ExportType>
{
public:
    ExportTypeList();
    ExportTypeList(std::initializer_list<ExportType> args);

    bool contains(const QString& id) const;
    const ExportType& getById(const QString& id) const;

    QVariantList toVariantList() const;
};

struct ExportType
{
    QString id;
    QStringList suffixes;
    ExportTypeList subtypes;
    QString name;
    QString filterName;
    QString settingsPagePath;

    QVariantMap toMap() const;
    QString filter() const;
    bool hasSubtypes() const;

    static ExportType makeWithSuffixes(const QStringList& suffixes, const QString& name, const QString& filterName,
                                       const QString& settingsPagePath);
    static ExportType makeWithSubtypes(const ExportTypeList& subtypes, const QString& name);

    inline bool operator==(const ExportType& other) const { return id == other.id; }
    inline bool operator!=(const ExportType& other) const { return id != other.id; }
};
}

#endif // MU_USERSCORES_EXPORTTYPE_H
