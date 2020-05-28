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

#ifndef AVS_INFOPOPUP_H
#define AVS_INFOPOPUP_H

#include <QFrame>

#include "avsomr/avsomr.h"

class QCheckBox;

namespace Ms {
class ScoreView;

namespace Avs {
class InfoPopup : public QFrame
{
    Q_OBJECT

public:
    InfoPopup();

    void setRecognizedChecked(bool arg);
    void setNotRecognizedChecked(bool arg);

    void showOnView(ScoreView* view, const AvsOmr::Info& info);

signals:
    void recognizedCheckedChanged(bool checked);
    void notrecognizedCheckedChanged(bool checked);

private slots:
    void onViewChanged();

private:

    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

    void updatePopup(ScoreView* view);

    QCheckBox* _recognizedCheck{ nullptr };
    QCheckBox* _notrecognizedCheck{ nullptr };
};
} // Avs
} // Ms

#endif // AVS_INFOPOPUP_H
