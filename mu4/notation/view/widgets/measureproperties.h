//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __MEASUREPROPERTIES_H__
#define __MEASUREPROPERTIES_H__

#include <QDialog>

#include "ui_measureproperties.h"
#include "libmscore/sig.h"
#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "notation/inotation.h"

namespace Ms {
class Measure;
class Fraction;
}

namespace mu {
namespace notation {
class MeasurePropertiesDialog : public QDialog, private Ui::MeasurePropertiesBase
{
    Q_OBJECT

    INJECT(notation, mu::context::IGlobalContext, context)

    Q_PROPERTY(int index READ index WRITE setIndex NOTIFY indexChanged)

public:
    MeasurePropertiesDialog(QWidget* parent = nullptr);
    MeasurePropertiesDialog(const MeasurePropertiesDialog& dialog);

    int index() const;

public slots:
    void setIndex(int index);

signals:
    void indexChanged(int index);

private slots:
    void bboxClicked(QAbstractButton* button);
    void gotoNextMeasure();
    void gotoPreviousMeasure();

private:
    void apply();
    Ms::Fraction len() const;
    bool isIrregular() const;
    int repeatCount() const;
    bool visible(int staffIdx);
    bool stemless(int staffIdx);
    void setMeasure(Ms::Measure* measure);

    Ms::Measure* m_measure = nullptr;
    int m_measureIndex = -1;

    std::shared_ptr<INotation> m_notation;
};
}
}

Q_DECLARE_METATYPE(mu::notation::MeasurePropertiesDialog)
#endif
