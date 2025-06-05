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

#include "image.h"

#include "io/file.h"
#include "io/fileinfo.h"

#include "draw/types/pixmap.h"
#include "draw/svgrenderer.h"

#include "imageStore.h"
#include "masterscore.h"
#include "mscore.h"
#include "score.h"

#include "log.h"

using namespace mu;
using namespace muse::io;
using namespace muse::draw;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   propertyList
//---------------------------------------------------------

static bool defaultAutoScale        = false;
static bool defaultLockAspectRatio  = true;
static bool defaultSizeIsSpatium    = true;

//---------------------------------------------------------
//   Image
//---------------------------------------------------------

Image::Image(EngravingItem* parent)
    : BSymbol(ElementType::IMAGE, parent, ElementFlag::MOVABLE), muse::Injectable(BSymbol::iocContext())
{
    m_imageType        = ImageType::NONE;
    m_size            = SizeF(0.0, 0.0);
    m_storeItem       = 0;
    m_dirty           = false;
    m_lockAspectRatio = defaultLockAspectRatio;
    m_autoScale       = defaultAutoScale;
    m_sizeIsSpatium   = defaultSizeIsSpatium;
    m_linkIsValid     = false;
}

Image::Image(const Image& img)
    : BSymbol(img), muse::Injectable(img.muse::Injectable::iocContext())
{
    m_imageType        = img.m_imageType;
    m_buffer           = img.m_buffer;
    m_size            = img.m_size;
    m_lockAspectRatio = img.m_lockAspectRatio;
    m_autoScale       = img.m_autoScale;
    m_dirty           = img.m_dirty;
    m_storeItem       = img.m_storeItem;
    m_sizeIsSpatium   = img.m_sizeIsSpatium;
    if (m_storeItem) {
        m_storeItem->reference(this);
    }
    m_linkPath        = img.m_linkPath;
    m_linkIsValid     = img.m_linkIsValid;
    if (m_imageType == ImageType::RASTER) {
        m_rasterDoc = img.m_rasterDoc ? std::make_shared<Pixmap>(*img.m_rasterDoc) : nullptr;
    } else if (m_imageType == ImageType::SVG) {
        m_svgDoc = img.m_svgDoc ? new SvgRenderer(m_storeItem->buffer()) : 0;
    }
    setZ(img.z());
}

//---------------------------------------------------------
//   Image
//---------------------------------------------------------

Image::~Image()
{
    if (m_storeItem) {
        m_storeItem->dereference(this);
    }
    if (m_imageType == ImageType::SVG) {
        delete m_svgDoc;
    }
}

//---------------------------------------------------------
//   setImageType
//---------------------------------------------------------

void Image::setImageType(ImageType t)
{
    m_imageType = t;
    if (m_imageType == ImageType::SVG) {
        m_svgDoc = 0;
    } else if (m_imageType == ImageType::RASTER) {
        m_rasterDoc.reset();
    } else {
        LOGD("illegal image type");
    }
}

//---------------------------------------------------------
//   imageSize
//---------------------------------------------------------

muse::SizeF Image::imageSize() const
{
    if (!isValid()) {
        return muse::SizeF();
    }

    if (m_imageType == ImageType::RASTER) {
        muse::Size rasterSize = m_rasterDoc->size();
        return muse::SizeF(rasterSize.width(), rasterSize.height());
    }

    return m_svgDoc->defaultSize();
}

//---------------------------------------------------------
//   isImageFramed
//---------------------------------------------------------

bool Image::isImageFramed() const
{
    if (!explicitParent()) {
        return false;
    }

    return explicitParent()->isBox();
}

//---------------------------------------------------------
//   imageAspectRatio
//---------------------------------------------------------

double Image::imageAspectRatio() const
{
    return m_size.width() / m_size.height();
}

//---------------------------------------------------------
//   setImageHeight
//---------------------------------------------------------

void Image::updateImageHeight(const double& height)
{
    double aspectRatio = imageAspectRatio();

    m_size.setHeight(height);

    if (m_lockAspectRatio) {
        m_size.setWidth(height * aspectRatio);
    }
}

//---------------------------------------------------------
//   setImageWidth
//---------------------------------------------------------

void Image::updateImageWidth(const double& width)
{
    double aspectRatio = imageAspectRatio();

    m_size.setWidth(width);

    if (m_lockAspectRatio) {
        m_size.setHeight(width / aspectRatio);
    }
}

//---------------------------------------------------------
//   imageHeight
//---------------------------------------------------------

double Image::imageHeight() const
{
    return m_size.height();
}

//---------------------------------------------------------
//   imageWidth
//---------------------------------------------------------

double Image::imageWidth() const
{
    return m_size.width();
}

bool Image::load()
{
    // once all paths are read, load img or retrieve it from store
    // loading from file is tried first to update the stored image, if necessary

    muse::io::path_t path;
    bool loaded = false;
    // if a store path is given, attempt to get the image from the store
    if (!m_storePath.isEmpty()) {
        m_storeItem = imageStore.getImage(m_storePath);
        if (m_storeItem) {
            m_storeItem->reference(this);
            loaded = true;
        }
        // if no image in store, attempt to load from path (for backward compatibility)
        else {
            loaded = load(m_storePath);
        }
        path = m_storePath;
    }
    // if no success from store path, attempt loading from link path (for .mscx files)
    if (!loaded) {
        loaded = load(m_linkPath);
        m_linkIsValid = loaded;
        path = m_linkPath;
    }

    if (path.withSuffix("svg") || path.withSuffix("svgz")) {
        setImageType(ImageType::SVG);
    } else {
        setImageType(ImageType::RASTER);
    }

    return loaded;
}

//---------------------------------------------------------
//   load
//    load image from file and put into ImageStore
//    return true on success
//---------------------------------------------------------

bool Image::load(const muse::io::path_t& ss)
{
    if (ss.empty()) {
        return false;
    }

    muse::io::path_t path(ss);
    // if file path is relative, prepend score path
    FileInfo fi(path);
    if (fi.isRelative()) {
        path = masterScore()->fileInfo()->absoluteDirPath() + '/' + path;
        fi = FileInfo(path);
    }

    m_linkIsValid = false;                       // assume link fname is invalid
    File f(path);
    if (!f.open(IODevice::ReadOnly)) {
        LOGD() << "failed load file: " << path;
        return false;
    }
    muse::ByteArray ba = f.readAll();
    f.close();

    m_linkIsValid = true;
    m_linkPath = fi.canonicalFilePath();
    m_storeItem = imageStore.add(m_linkPath, ba);
    m_storeItem->reference(this);
    if (path.withSuffix("svg") || path.withSuffix("svgz")) {
        setImageType(ImageType::SVG);
    } else {
        setImageType(ImageType::RASTER);
    }
    return true;
}

//---------------------------------------------------------
//   loadFromData
//    load image from data and put into ImageStore
//    return true on success
//---------------------------------------------------------

bool Image::loadFromData(const path_t& name, const muse::ByteArray& ba)
{
    m_linkIsValid = false;
    m_linkPath = u"";
    m_storeItem = imageStore.add(name, ba);
    m_storeItem->reference(this);
    if (name.withSuffix("svg") || name.withSuffix("svgz")) {
        setImageType(ImageType::SVG);
    } else {
        setImageType(ImageType::RASTER);
    }
    return true;
}

//---------------------------------------------------------
//   startDrag
//---------------------------------------------------------

void Image::startEditDrag(EditData& data)
{
    BSymbol::startEditDrag(data);
    ElementEditDataPtr eed = data.getData(this);

    eed->pushProperty(Pid::SIZE);
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Image::editDrag(EditData& ed)
{
    if (ed.curGrip == Grip::MIDDLE) {
        setOffset(offset() + ed.evtDelta);
        setOffsetChanged(true);
        triggerLayout();
        return;
    }

    double ratio = m_size.width() / m_size.height();
    double dx = ed.delta.x();
    double dy = ed.delta.y();
    if (m_sizeIsSpatium) {
        double _spatium = spatium();
        dx /= _spatium;
        dy /= _spatium;
    } else {
        dx /= DPMM;
        dy /= DPMM;
    }
    if (ed.curGrip == Grip::START) {
        m_size.setWidth(m_size.width() + dx);
        if (m_lockAspectRatio) {
            m_size.setHeight(m_size.width() / ratio);
        }
    } else if (ed.curGrip == Grip::END) {
        m_size.setHeight(m_size.height() + dy);
        if (m_lockAspectRatio) {
            m_size.setWidth(m_size.height() * ratio);
        }
    }

    triggerLayout();
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<PointF> Image::gripsPositions(const EditData&) const
{
    RectF r(pageBoundingRect());
    return {
        PointF(r.x() + r.width(), r.y() + r.height() * .5),
        PointF(r.x() + r.width() * .5, r.y() + r.height())
    };
}

//---------------------------------------------------------
//   pixel2Size
//---------------------------------------------------------

SizeF Image::pixel2size(const SizeF& s) const
{
    return s / (m_sizeIsSpatium ? spatium() : DPMM);
}

//---------------------------------------------------------
//   size2pixel
//---------------------------------------------------------

SizeF Image::size2pixel(const SizeF& s) const
{
    return s * (m_sizeIsSpatium ? spatium() : DPMM);
}

void Image::init()
{
    if (m_imageType == ImageType::SVG && !m_svgDoc) {
        if (m_storeItem) {
            m_svgDoc = new SvgRenderer(m_storeItem->buffer());
        }
    } else if (m_imageType == ImageType::RASTER && !m_rasterDoc) {
        if (m_storeItem) {
            m_rasterDoc = imageProvider()->createPixmap(m_storeItem->buffer());
            if (!m_rasterDoc->isNull()) {
                m_dirty = true;
            }
        }
    }
    if (m_size.isNull()) {
        m_size = pixel2size(imageSize());
    }
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Image::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::AUTOSCALE:
        return autoScale();
    case Pid::SIZE:
        return PropertyValue::fromValue(size());
    case Pid::IMAGE_HEIGHT:
        return imageHeight();
    case Pid::IMAGE_WIDTH:
        return imageWidth();
    case Pid::IMAGE_FRAMED:
        return isImageFramed();
    case Pid::LOCK_ASPECT_RATIO:
        return lockAspectRatio();
    case Pid::SIZE_IS_SPATIUM:
        return sizeIsSpatium();
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Image::setProperty(Pid propertyId, const PropertyValue& v)
{
    bool rv = true;
    score()->addRefresh(canvasBoundingRect());
    switch (propertyId) {
    case Pid::AUTOSCALE:
        setAutoScale(v.toBool());
        break;
    case Pid::SIZE:
        setSize(v.value<SizeF>());
        break;
    case Pid::IMAGE_HEIGHT:
        updateImageHeight(v.toDouble());
        break;
    case Pid::IMAGE_WIDTH:
        updateImageWidth(v.toDouble());
        break;
    case Pid::IMAGE_FRAMED:
        break;
    case Pid::LOCK_ASPECT_RATIO:
        setLockAspectRatio(v.toBool());
        break;
    case Pid::SIZE_IS_SPATIUM: {
        SizeF s = size2pixel(m_size);
        setSizeIsSpatium(v.toBool());
        m_size = pixel2size(s);
    }
    break;
    default:
        rv = EngravingItem::setProperty(propertyId, v);
        break;
    }
    setGenerated(false);
    m_dirty = true;
    triggerLayout();
    return rv;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Image::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::AUTOSCALE:
        return defaultAutoScale;
    case Pid::SIZE:
        return PropertyValue::fromValue(pixel2size(imageSize()));
    case Pid::IMAGE_HEIGHT:
        return pixel2size(imageSize()).height();
    case Pid::IMAGE_WIDTH:
        return pixel2size(imageSize()).width();
    case Pid::IMAGE_FRAMED:
        return isImageFramed();
    case Pid::LOCK_ASPECT_RATIO:
        return defaultLockAspectRatio;
    case Pid::SIZE_IS_SPATIUM:
        return defaultSizeIsSpatium;
    default:
        return EngravingItem::propertyDefault(id);
    }
}
}
