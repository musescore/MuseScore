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
#ifndef MU_CONVERTER_CONVERTERCONTROLLER_H
#define MU_CONVERTER_CONVERTERCONTROLLER_H

#include <list>

#include "../iconvertercontroller.h"

#include "modularity/ioc.h"
#include "notation/inotationcreator.h"
#include "notation/inotationwritersregister.h"

#include "retval.h"

namespace mu::converter {
class ConverterController : public IConverterController
{
    INJECT(converter, notation::INotationCreator, notationCreator)
    INJECT(converter, notation::INotationWritersRegister, writers)

public:
    ConverterController() = default;

    Ret batchConvert(const io::path& batchJobFile) override;

private:

    struct Job {
        io::path in;
        io::path out;
    };

    using BatchJob = std::list<Job>;

    RetVal<BatchJob> parseBatchJob(const io::path& batchJobFile) const;

    Ret convert(const io::path& in, const io::path& out);
};
}

#endif // MU_CONVERTER_CONVERTERCONTROLLER_H
