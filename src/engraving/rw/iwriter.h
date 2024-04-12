/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_ENGRAVING_IWRITER_H
#define MU_ENGRAVING_IWRITER_H

#include <memory>
#include <variant>

#include "global/io/iodevice.h"
#include "xmlwriter.h"

namespace mu::engraving {
class Score;
class EngravingItem;
class Segment;
class SelectionFilter;
}

namespace mu::engraving::rw {
struct WriteInOutData;
class IWriter
{
public:
    virtual ~IWriter() = default;

    virtual bool writeScore(Score* score, muse::io::IODevice* device, bool onlySelection, WriteInOutData* out = nullptr) = 0;

    using Supported = std::variant<std::monostate
                                   >;

    template<typename T>
    static void check_supported_static(T item)
    {
        if constexpr (std::is_same<T, const EngravingItem*>::value || std::is_same<T, EngravingItem*>::value) {
            // supported
        } else {
            Supported check(item);
            (void)check;
        }
    }

    template<typename T>
    void writeItem(const T item, XmlWriter& xml)
    {
#ifndef NDEBUG
        check_supported_static(item);
#endif
        doWriteItem(static_cast<const EngravingItem*>(item), xml);
    }

    virtual void writeSegments(XmlWriter& xml, SelectionFilter* filter, track_idx_t st, track_idx_t et, Segment* sseg, Segment* eseg, bool,
                               bool, Fraction& curTick) = 0;

private:
    virtual void doWriteItem(const EngravingItem* item, XmlWriter& xml) = 0;
};

using IWriterPtr = std::shared_ptr<IWriter>;
}

#endif // MU_ENGRAVING_IWRITER_H
