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

#ifndef AVS_TASKBARPROGRESS_H
#define AVS_TASKBARPROGRESS_H

#include <QObject>

#ifdef Q_OS_WIN
class QWinTaskbarProgress;
#endif

namespace Ms {
//! NOTE Probably somewhere there should be a common place for common UI controls
class TaskbarProgress : public QObject
{
    Q_OBJECT
public:
    TaskbarProgress(QObject* parent);
    ~TaskbarProgress();

    bool isAvalabled() const;

public slots:
    void show();
    void hide();
    void stop();

    void setRange(int min, int max);
    void setValue(int val);

private:
#ifdef Q_OS_WIN
    QWinTaskbarProgress* _winTaskbarProgress{ nullptr };
#endif
};
} // Ms

#endif // AVS_TASKBARPROGRESS_H
