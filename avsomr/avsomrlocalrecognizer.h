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

#ifndef AVS_AVSOMRLOCALRECOGNIZER_H
#define AVS_AVSOMRLOCALRECOGNIZER_H

#include "iavsomrrecognizer.h"

namespace Ms {
namespace Avs {
class AvsOmrLocal;
class AvsOmrLocalRecognizer : public IAvsOmrRecognizer
{
public:
    AvsOmrLocalRecognizer();
    ~AvsOmrLocalRecognizer() override;

    QString type() const override;
    bool isAvailable() const override;
    bool recognize(const QString& filePath, QByteArray* avsFileData, const OnStep& onStep) override;

private:

    AvsOmrLocal* avsOmrLocal() const;

    QString makeBuildPath() const;
    Ret cleanDir(const QString& dirPath);
    Ret readFile(QByteArray* avsData, const QString& avsPath);
};
} // Avs
} // Ms

#endif // AVS_AVSOMRLOCALRECOGNIZER_H
