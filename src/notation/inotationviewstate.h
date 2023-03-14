/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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
#ifndef MU_NOTATION_INOTATIONVIEWSTATE_H
#define MU_NOTATION_INOTATIONVIEWSTATE_H

#include <memory>

#include "async/channel.h"
#include "types/retval.h"

#include "engraving/infrastructure/mscreader.h"
#include "engraving/infrastructure/mscwriter.h"

#include "draw/types/transform.h"

#include "notationtypes.h"

namespace mu::notation {
class NotationPaintView;
class INotationViewState
{
public:
    virtual ~INotationViewState() = default;

    virtual Ret read(const engraving::MscReader& reader, const io::path_t& pathPrefix = "") = 0;
    virtual Ret write(engraving::MscWriter& writer, const io::path_t& pathPrefix = "") = 0;

    virtual bool isMatrixInited() const = 0;
    virtual void setMatrixInited(bool inited) = 0;

    virtual draw::Transform matrix() const = 0;
    virtual async::Channel<draw::Transform /*newMatrix*/, NotationPaintView* /*sender*/> matrixChanged() const = 0;
    virtual void setMatrix(const draw::Transform& matrix, NotationPaintView* sender) = 0;

    virtual ValCh<int> zoomPercentage() const = 0;

    virtual ValCh<ZoomType> zoomType() const = 0;
    virtual void setZoomType(ZoomType type) = 0;

    virtual ViewMode viewMode() const = 0;
    virtual void setViewMode(const ViewMode& mode) = 0;

    virtual bool needSave() const = 0;
    virtual async::Notification needSaveChanged() const = 0;
    virtual void markAsSaved() = 0;

    virtual void makeDefault() = 0;
};

using INotationViewStatePtr = std::shared_ptr<INotationViewState>;
}

#endif // MU_NOTATION_INOTATIONVIEWSTATE_H
