//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
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
#ifndef MU_FRAMEWORK_FILTERVALUE_H
#define MU_FRAMEWORK_FILTERVALUE_H

#include <QObject>
#include <QMetaType>

namespace mu {
namespace framework {
class CompareType
{
    Q_GADGET
public:
    enum Type
    {
        Equal,
        Contains
    };
    Q_ENUM(Type)
};

class FilterValue : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString roleName READ roleName WRITE setRoleName NOTIFY dataChanged)
    Q_PROPERTY(QVariant roleValue READ roleValue WRITE setRoleValue NOTIFY dataChanged)
    Q_PROPERTY(QVariant compareType READ compareType WRITE setCompareType NOTIFY dataChanged)

public:
    explicit FilterValue(QObject* parent = nullptr);

    QString roleName() const;
    QVariant roleValue() const;
    QVariant compareType() const;

public slots:
    void setRoleName(QString roleName);
    void setRoleValue(QVariant roleValue);
    void setCompareType(QVariant compareType);

signals:
    void dataChanged();

private:
    QString m_roleName;
    QVariant m_roleValue;
    QVariant m_compareType;
};
}
}

#endif // MU_FRAMEWORK_FILTERVALUE_H
