/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "bsymbol.h"

#include "modularity/ioc.h"
#include "draw/iimageprovider.h"

namespace mu::draw {
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

class Image final : public BSymbol
{
    OBJECT_ALLOCATOR(engraving, Image)
    DECLARE_CLASSOF(ElementType::IMAGE)

    INJECT(mu::draw::IImageProvider, imageProvider)

public:
    Image(EngravingItem* parent = 0);
    Image(const Image&);
    ~Image();

    Image* clone() const override { return new Image(*this); }

    bool load(); // after set paths
    bool load(const io::path_t& s);
    bool loadFromData(const io::path_t& name, const mu::ByteArray&);

    void draw(mu::draw::Painter*) const override;

    void init();

    bool isImageFramed() const;
    double imageAspectRatio() const;
    void setSize(const mu::SizeF& s) { _size = s; }
    mu::SizeF size() const { return _size; }
    void updateImageHeight(const double& height);
    void updateImageWidth(const double& width);
    double imageHeight() const;
    double imageWidth() const;
    bool lockAspectRatio() const { return _lockAspectRatio; }
    void setLockAspectRatio(bool v) { _lockAspectRatio = v; }
    bool autoScale() const { return _autoScale; }
    void setAutoScale(bool v) { _autoScale = v; }
    ImageStoreItem* storeItem() const { return _storeItem; }
    bool sizeIsSpatium() const { return _sizeIsSpatium; }
    void setSizeIsSpatium(bool val) { _sizeIsSpatium = val; }

    String storePath() const { return _storePath; }
    void setStorePath(const String& p) { _storePath = p; }
    String linkPath() const { return _linkPath; }
    void setLinkPath(const String& p) { _linkPath = p; }
    bool linkIsValid() const { return _linkIsValid; }

    PropertyValue getProperty(Pid) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid id) const override;

    mu::SizeF imageSize() const;

    void setImageType(ImageType);
    ImageType getImageType() const { return imageType; }
    bool isValid() const { return rasterDoc || svgDoc; }

    bool needStartEditingAfterSelecting() const override { return true; }
    int gripsCount() const override { return 2; }
    Grip initialEditModeGrip() const override { return Grip(1); }
    Grip defaultGrip() const override { return Grip(1); }
    std::vector<mu::PointF> gripsPositions(const EditData&) const override;

protected:

    friend class layout::v0::TLayout;

    ImageStoreItem* _storeItem;
    String _storePath;             // the path of the img in the ImageStore
    String _linkPath;              // the path of an external linked img
    bool _linkIsValid;              // whether _linkPath file exists or not
    mutable mu::draw::Pixmap buffer;         ///< cached rendering
    mu::SizeF _size;                   // in mm or spatium units
    bool _lockAspectRatio;
    bool _autoScale;                ///< fill parent frame
    bool _sizeIsSpatium;
    mutable bool _dirty;

    bool isEditable() const override { return true; }
    void startEditDrag(EditData&) override;
    void editDrag(EditData& ed) override;
    std::vector<mu::LineF> gripAnchorLines(Grip) const override { return std::vector<mu::LineF>(); }

private:
    mu::SizeF pixel2size(const mu::SizeF& s) const;
    mu::SizeF size2pixel(const mu::SizeF& s) const;

    std::shared_ptr<mu::draw::Pixmap> rasterDoc;
    mu::draw::SvgRenderer* svgDoc = nullptr;

    ImageType imageType = ImageType::NONE;
};
} // namespace mu::engraving
#endif
