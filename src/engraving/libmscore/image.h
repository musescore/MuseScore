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

#include "modularity/ioc.h"

#include "engraving/draw/iimageconverter.h"
#include "bsymbol.h"

namespace mu::draw {
class Pixmap;
class SvgRenderer;
}

namespace Ms {
class ImageStoreItem;

enum class ImageType : char {
    NONE, RASTER, SVG
};

//---------------------------------------------------------
//   @@ Image
//---------------------------------------------------------

class Image final : public BSymbol
{
    INJECT(engraving, mu::draw::IImageConverter, imageConverter)
    union {
        mu::draw::Pixmap* rasterDoc;
        mu::draw::SvgRenderer* svgDoc;
    };
    ImageType imageType;

    mu::SizeF pixel2size(const mu::SizeF& s) const;
    mu::SizeF size2pixel(const mu::SizeF& s) const;

protected:
    ImageStoreItem* _storeItem;
    QString _storePath;             // the path of the img in the ImageStore
    QString _linkPath;              // the path of an external linked img
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
    QVector<mu::LineF> gripAnchorLines(Grip) const override { return QVector<mu::LineF>(); }

public:
    Image(Score* = 0);
    Image(const Image&);
    ~Image();

    Image* clone() const override { return new Image(*this); }
    ElementType type() const override { return ElementType::IMAGE; }
    void write(XmlWriter& xml) const override;
    void read(XmlReader&) override;
    bool load(const QString& s);
    bool loadFromData(const QString&, const QByteArray&);
    void layout() override;
    void draw(mu::draw::Painter*) const override;

    bool isImageFramed() const;
    qreal imageAspectRatio() const;
    void setSize(const mu::SizeF& s) { _size = s; }
    mu::SizeF size() const { return _size; }
    void updateImageHeight(const qreal& height);
    void updateImageWidth(const qreal& width);
    qreal imageHeight() const;
    qreal imageWidth() const;
    bool lockAspectRatio() const { return _lockAspectRatio; }
    void setLockAspectRatio(bool v) { _lockAspectRatio = v; }
    bool autoScale() const { return _autoScale; }
    void setAutoScale(bool v) { _autoScale = v; }
    ImageStoreItem* storeItem() const { return _storeItem; }
    bool sizeIsSpatium() const { return _sizeIsSpatium; }
    void setSizeIsSpatium(bool val) { _sizeIsSpatium = val; }

    QVariant getProperty(Pid) const override;
    bool setProperty(Pid propertyId, const QVariant&) override;
    QVariant propertyDefault(Pid id) const override;

    mu::SizeF imageSize() const;

    void setImageType(ImageType);
    ImageType getImageType() const { return imageType; }
    bool isValid() const { return rasterDoc || svgDoc; }

    Element::EditBehavior normalModeEditBehavior() const override { return Element::EditBehavior::Edit; }
    int gripsCount() const override { return 2; }
    Grip initialEditModeGrip() const override { return Grip(1); }
    Grip defaultGrip() const override { return Grip(1); }
    std::vector<mu::PointF> gripsPositions(const EditData&) const override;
};
}     // namespace Ms
#endif
