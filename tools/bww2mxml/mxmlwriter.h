/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#ifndef MXMLWRITER_H
#define MXMLWRITER_H

/**
 \file
 Definition of class MxmlWriter
 */

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QTextStream>

#include "writer.h"

class QIODevice;

namespace Bww {
/**
   The writer that generates MusicXML output.
   */

class MxmlWriter : public Writer
{
public:
    MxmlWriter();
    void beginMeasure(const Bww::MeasureBeginFlags mbf);
    void endMeasure(const Bww::MeasureEndFlags mef);
    void header(const QString title, const QString type, const QString composer, const QString footer, const unsigned int temp);
    void note(const QString pitch, const QVector<BeamType> beamList, const QString type, const int dots, bool tieStart = false,
              bool tieStop = false, StartStop triplet = StartStop::ST_NONE, bool grace = false);
    void setOutDevice(QIODevice* outDevice) { out.setDevice(outDevice); }
    void tsig(const int beats, const int beat);
    void trailer();
private:
    int wholeDur() const { return 3 * 64; }             ///< Whole note duration
    struct StepAlterOct {                               ///< MusicXML step/alter/oct values
        QChar s;
        int a;
        int o;
        StepAlterOct(QChar step = 'C', int alter = 0, int oct = 1)
            : s(step), a(alter), o(oct) {}
    };
    QTextStream out;                                    ///< The output text stream
    int beats;                                          ///< Number of beats
    int beat;                                           ///< Beat type
    QMap<QString, StepAlterOct> stepAlterOctMap;        ///< Map bww pitch to step/alter/oct
    QMap<QString, QString> typeMap;                     ///< Map bww note types to MusicXML
    unsigned int regularMeasureNumber;                  ///< Current regular measure number
    unsigned int irregularMeasureNumber;                ///< Current irregular measure number
    unsigned int tempo;                                 ///< Tempo (0 = not specified)
    unsigned int ending;                                ///< Current ending
};
} // namespace Bww

#endif // MXMLWRITER_H
