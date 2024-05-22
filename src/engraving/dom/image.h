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

#ifndef MU_ENGRAVING_IMAGE_H
#define MU_ENGRAVING_IMAGE_H

#include "bsymbol.h"

#include "modularity/ioc.h"
#include "draw/iimageprovider.h"

namespace muse::draw {
class Pixmap;
class SvgRenderer;
}

namespace mu::engraving {
class ImageStoreItem;

enum class ImageType : char {
    NONE, RASTER, SVG
};

//---------------------------------------------------------
//   @@ Image
//---------------------------------------------------------

class Image final : public BSymbol, public muse::Injectable
{
    OBJECT_ALLOCATOR(engraving, Image)
    DECLARE_CLASSOF(ElementType::IMAGE)

    muse::Inject<muse::draw::IImageProvider> imageProvider = { this };

public:
    Image(EngravingItem* parent);
    Image(const Image&);
    ~Image();

    Image* clone() const override { return new Image(*this); }

    bool load(); // after set paths
    bool load(const muse::io::path_t& s);
    bool loadFromData(const muse::io::path_t& name, const muse::ByteArray&);

    void init();

    bool isImageFramed() const;
    double imageAspectRatio() const;
    void setSize(const SizeF& s) { m_size = s; }
    const SizeF& size() const { return m_size; }
    void updateImageHeight(const double& height);
    void updateImageWidth(const double& width);
    double imageHeight() const;
    double imageWidth() const;
    bool lockAspectRatio() const { return m_lockAspectRatio; }
    void setLockAspectRatio(bool v) { m_lockAspectRatio = v; }
    bool autoScale() const { return m_autoScale; }
    void setAutoScale(bool v) { m_autoScale = v; }
    ImageStoreItem* storeItem() const { return m_storeItem; }
    bool sizeIsSpatium() const { return m_sizeIsSpatium; }
    void setSizeIsSpatium(bool val) { m_sizeIsSpatium = val; }

    String storePath() const { return m_storePath; }
    void setStorePath(const String& p) { m_storePath = p; }
    String linkPath() const { return m_linkPath; }
    void setLinkPath(const String& p) { m_linkPath = p; }
    bool linkIsValid() const { return m_linkIsValid; }

    PropertyValue getProperty(Pid) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid id) const override;

    SizeF imageSize() const;

    void setImageType(ImageType);
    ImageType imageType() const { return m_imageType; }
    bool isValid() const { return m_rasterDoc || m_svgDoc; }

    muse::draw::SvgRenderer* svgRenderer() const { return m_svgDoc; }
    const std::shared_ptr<muse::draw::Pixmap>& rasterImage() const { return m_rasterDoc; }

    bool needStartEditingAfterSelecting() const override { return true; }
    int gripsCount() const override { return 2; }
    Grip initialEditModeGrip() const override { return Grip(1); }
    Grip defaultGrip() const override { return Grip(1); }
    std::vector<PointF> gripsPositions(const EditData&) const override;

    SizeF pixel2size(const SizeF& s) const;
    SizeF size2pixel(const SizeF& s) const;

    const muse::draw::Pixmap& buffer() const { return m_buffer; }
    void setBuffer(const muse::draw::Pixmap& p) const { m_buffer = p; }
    bool dirty() const { return m_dirty; }
    void setDirty(bool val) const { m_dirty = val; }

private:

    bool isEditable() const override { return true; }
    void startEditDrag(EditData&) override;
    void editDrag(EditData& ed) override;
    std::vector<LineF> gripAnchorLines(Grip) const override { return std::vector<LineF>(); }

    ImageStoreItem* m_storeItem = nullptr;
    String m_storePath;                 // the path of the img in the ImageStore
    String m_linkPath;                  // the path of an external linked img
    bool m_linkIsValid = false;         // whether _linkPath file exists or not
    mutable muse::draw::Pixmap m_buffer;  // cached rendering
    SizeF m_size;                   // in mm or spatium units
    bool m_lockAspectRatio = false;
    bool m_autoScale = false;           // fill parent frame
    bool m_sizeIsSpatium = false;
    mutable bool m_dirty = false;

    std::shared_ptr<muse::draw::Pixmap> m_rasterDoc;
    muse::draw::SvgRenderer* m_svgDoc = nullptr;

    ImageType m_imageType = ImageType::NONE;
};
} // namespace mu::engraving
#endif
