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

#include "image.h"

#include "io/file.h"
#include "io/fileinfo.h"

#include "draw/types/pixmap.h"
#include "draw/types/transform.h"
#include "draw/svgrenderer.h"
#include "rw/xml.h"

#include "imageStore.h"
#include "masterscore.h"
#include "mscore.h"
#include "score.h"

#include "log.h"

using namespace mu;
using namespace mu::io;
using namespace mu::draw;
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
    : BSymbol(ElementType::IMAGE, parent, ElementFlag::MOVABLE)
{
    imageType        = ImageType::NONE;
    _size            = SizeF(0.0, 0.0);
    _storeItem       = 0;
    _dirty           = false;
    _lockAspectRatio = defaultLockAspectRatio;
    _autoScale       = defaultAutoScale;
    _sizeIsSpatium   = defaultSizeIsSpatium;
    _linkIsValid     = false;
}

Image::Image(const Image& img)
    : BSymbol(img)
{
    imageType        = img.imageType;
    buffer           = img.buffer;
    _size            = img._size;
    _lockAspectRatio = img._lockAspectRatio;
    _autoScale       = img._autoScale;
    _dirty           = img._dirty;
    _storeItem       = img._storeItem;
    _sizeIsSpatium   = img._sizeIsSpatium;
    if (_storeItem) {
        _storeItem->reference(this);
    }
    _linkPath        = img._linkPath;
    _linkIsValid     = img._linkIsValid;
    if (imageType == ImageType::RASTER) {
        rasterDoc = img.rasterDoc ? std::make_shared<Pixmap>(*img.rasterDoc) : nullptr;
    } else if (imageType == ImageType::SVG) {
        svgDoc = img.svgDoc ? new SvgRenderer(_storeItem->buffer()) : 0;
    }
    setZ(img.z());
}

//---------------------------------------------------------
//   Image
//---------------------------------------------------------

Image::~Image()
{
    if (_storeItem) {
        _storeItem->dereference(this);
    }
    if (imageType == ImageType::SVG) {
        delete svgDoc;
    }
}

//---------------------------------------------------------
//   setImageType
//---------------------------------------------------------

void Image::setImageType(ImageType t)
{
    imageType = t;
    if (imageType == ImageType::SVG) {
        svgDoc = 0;
    } else if (imageType == ImageType::RASTER) {
        rasterDoc.reset();
    } else {
        LOGD("illegal image type");
    }
}

//---------------------------------------------------------
//   imageSize
//---------------------------------------------------------

SizeF Image::imageSize() const
{
    if (!isValid()) {
        return SizeF();
    }

    if (imageType == ImageType::RASTER) {
        Size rasterSize = rasterDoc->size();
        return SizeF(rasterSize.width(), rasterSize.height());
    }

    return svgDoc->defaultSize();
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Image::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    bool emptyImage = false;
    if (imageType == ImageType::SVG) {
        if (!svgDoc) {
            emptyImage = true;
        } else {
            svgDoc->render(painter, bbox());
        }
    } else if (imageType == ImageType::RASTER) {
        if (rasterDoc == nullptr) {
            emptyImage = true;
        } else {
            painter->save();
            SizeF s;
            if (_sizeIsSpatium) {
                s = _size * spatium();
            } else {
                s = _size * DPMM;
            }
            if (score() && score()->printing() && !MScore::svgPrinting) {
                // use original image size for printing, but not for svg for reasonable file size.
                painter->scale(s.width() / rasterDoc->width(), s.height() / rasterDoc->height());
                painter->drawPixmap(PointF(0, 0), *rasterDoc);
            } else {
                Transform t = painter->worldTransform();
                Size ss = Size(s.width() * t.m11(), s.height() * t.m22());
                t.setMatrix(1.0, t.m12(), t.m13(), t.m21(), 1.0, t.m23(), t.m31(), t.m32(), t.m33());
                painter->setWorldTransform(t);
                if ((buffer.size() != ss || _dirty) && rasterDoc && !rasterDoc->isNull()) {
                    buffer = imageProvider()->scaled(*rasterDoc, ss);
                    _dirty = false;
                }
                if (buffer.isNull()) {
                    emptyImage = true;
                } else {
                    painter->drawPixmap(PointF(0.0, 0.0), buffer);
                }
            }
            painter->restore();
        }
    }

    if (emptyImage) {
        painter->setBrush(mu::draw::BrushStyle::NoBrush);
        painter->setPen(engravingConfiguration()->defaultColor());
        painter->drawRect(bbox());
        painter->drawLine(0.0, 0.0, bbox().width(), bbox().height());
        painter->drawLine(bbox().width(), 0.0, 0.0, bbox().height());
    }
    if (selected() && !(score() && score()->printing())) {
        painter->setBrush(mu::draw::BrushStyle::NoBrush);
        painter->setPen(engravingConfiguration()->selectionColor());
        painter->drawRect(bbox());
    }
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
    return _size.width() / _size.height();
}

//---------------------------------------------------------
//   setImageHeight
//---------------------------------------------------------

void Image::updateImageHeight(const double& height)
{
    double aspectRatio = imageAspectRatio();

    _size.setHeight(height);

    if (_lockAspectRatio) {
        _size.setWidth(height * aspectRatio);
    }
}

//---------------------------------------------------------
//   setImageWidth
//---------------------------------------------------------

void Image::updateImageWidth(const double& width)
{
    double aspectRatio = imageAspectRatio();

    _size.setWidth(width);

    if (_lockAspectRatio) {
        _size.setHeight(width / aspectRatio);
    }
}

//---------------------------------------------------------
//   imageHeight
//---------------------------------------------------------

double Image::imageHeight() const
{
    return _size.height();
}

//---------------------------------------------------------
//   imageWidth
//---------------------------------------------------------

double Image::imageWidth() const
{
    return _size.width();
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Image::write(XmlWriter& xml) const
{
    // attempt to convert the _linkPath to a path relative to the score
    //
    // TODO : on Save As, score()->fileInfo() still contains the old path and fname
    //          if the Save As path is different, image relative path will be wrong!
    //
    String relativeFilePath;
    if (!_linkPath.isEmpty() && _linkIsValid) {
        FileInfo fi(_linkPath);
        // score()->fileInfo()->canonicalPath() would be better
        // but we are saving under a temp file name and the 'final' file
        // might not exist yet, so canonicalFilePath() may return only "/"
        // OTOH, the score 'final' file name is practically always canonical, at this point
        String scorePath = score()->masterScore()->fileInfo()->absoluteDirPath().toString();
        String imgFPath  = fi.canonicalFilePath();
        // if imgFPath is in (or below) the directory of scorePath
        if (imgFPath.startsWith(scorePath, mu::CaseSensitive)) {
            // relative img path is the part exceeding scorePath
            imgFPath.remove(0, scorePath.size());
            if (imgFPath.startsWith(u'/')) {
                imgFPath.remove(0, 1);
            }
            relativeFilePath = imgFPath;
        }
        // try 1 level up
        else {
            // reduce scorePath by one path level
            fi = FileInfo(scorePath);
            scorePath = fi.path();
            // if imgFPath is in (or below) the directory up the score directory
            if (imgFPath.startsWith(scorePath, mu::CaseSensitive)) {
                // relative img path is the part exceeding new scorePath plus "../"
                imgFPath.remove(0, scorePath.size());
                if (!imgFPath.startsWith(u'/')) {
                    imgFPath.prepend(u'/');
                }
                imgFPath.prepend(u"..");
                relativeFilePath = imgFPath;
            }
        }
    }
    // if no match, use full _linkPath
    if (relativeFilePath.isEmpty()) {
        relativeFilePath = _linkPath;
    }

    xml.startElement(this);
    BSymbol::writeProperties(xml);
    // keep old "path" tag, for backward compatibility and because it is used elsewhere
    // (for instance by Box:read(), Measure:read(), Note:read(), ...)
    xml.tag("path", _storeItem ? _storeItem->hashName() : relativeFilePath);
    xml.tag("linkPath", relativeFilePath);

    writeProperty(xml, Pid::AUTOSCALE);
    writeProperty(xml, Pid::SIZE);
    writeProperty(xml, Pid::LOCK_ASPECT_RATIO);
    writeProperty(xml, Pid::SIZE_IS_SPATIUM);

    xml.endElement();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Image::read(XmlReader& e)
{
    if (score()->mscVersion() <= 114) {
        _sizeIsSpatium = false;
    }

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "autoScale") {
            readProperty(e, Pid::AUTOSCALE);
        } else if (tag == "size") {
            readProperty(e, Pid::SIZE);
        } else if (tag == "lockAspectRatio") {
            readProperty(e, Pid::LOCK_ASPECT_RATIO);
        } else if (tag == "sizeIsSpatium") {
            // setting this using the property Pid::SIZE_IS_SPATIUM breaks, because the
            // property setter attempts to maintain a constant size. If we're reading, we
            // don't want to do that, because the stored size will be in:
            //    mm if size isn't spatium
            //    sp if size is spatium
            _sizeIsSpatium = e.readBool();
        } else if (tag == "path") {
            _storePath = e.readText();
        } else if (tag == "linkPath") {
            _linkPath = e.readText();
        } else if (tag == "subtype") {    // obsolete
            e.skipCurrentElement();
        } else if (!BSymbol::readProperties(e)) {
            e.unknown();
        }
    }

    // once all paths are read, load img or retrieve it from store
    // loading from file is tried first to update the stored image, if necessary

    io::path_t path;
    bool loaded = false;
    // if a store path is given, attempt to get the image from the store
    if (!_storePath.isEmpty()) {
        _storeItem = imageStore.getImage(_storePath);
        if (_storeItem) {
            _storeItem->reference(this);
            loaded = true;
        }
        // if no image in store, attempt to load from path (for backward compatibility)
        else {
            loaded = load(_storePath);
        }
        path = _storePath;
    }
    // if no success from store path, attempt loading from link path (for .mscx files)
    if (!loaded) {
        _linkIsValid = load(_linkPath);
        path = _linkPath;
    }

    if (path.withSuffix("svg")) {
        setImageType(ImageType::SVG);
    } else {
        setImageType(ImageType::RASTER);
    }
}

//---------------------------------------------------------
//   load
//    load image from file and put into ImageStore
//    return true on success
//---------------------------------------------------------

bool Image::load(const io::path_t& ss)
{
    if (ss.empty()) {
        return false;
    }

    io::path_t path(ss);
    // if file path is relative, prepend score path
    FileInfo fi(path);
    if (fi.isRelative()) {
        path = masterScore()->fileInfo()->absoluteDirPath() + '/' + path;
        fi = FileInfo(path);
    }

    _linkIsValid = false;                       // assume link fname is invalid
    File f(path);
    if (!f.open(IODevice::ReadOnly)) {
        LOGD() << "failed load file: " << path;
        return false;
    }
    ByteArray ba = f.readAll();
    f.close();

    _linkIsValid = true;
    _linkPath = fi.canonicalFilePath();
    _storeItem = imageStore.add(_linkPath, ba);
    _storeItem->reference(this);
    if (path.withSuffix("svg")) {
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

bool Image::loadFromData(const path_t& name, const ByteArray& ba)
{
    _linkIsValid = false;
    _linkPath = u"";
    _storeItem = imageStore.add(name, ba);
    _storeItem->reference(this);
    if (name.withSuffix("svg")) {
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
    double ratio = _size.width() / _size.height();
    double dx = ed.delta.x();
    double dy = ed.delta.y();
    if (_sizeIsSpatium) {
        double _spatium = spatium();
        dx /= _spatium;
        dy /= _spatium;
    } else {
        dx /= DPMM;
        dy /= DPMM;
    }
    if (ed.curGrip == Grip::START) {
        _size.setWidth(_size.width() + dx);
        if (_lockAspectRatio) {
            _size.setHeight(_size.width() / ratio);
        }
    } else {
        _size.setHeight(_size.height() + dy);
        if (_lockAspectRatio) {
            _size.setWidth(_size.height() * ratio);
        }
    }
    layout();
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<mu::PointF> Image::gripsPositions(const EditData&) const
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
    return s / (_sizeIsSpatium ? spatium() : DPMM);
}

//---------------------------------------------------------
//   size2pixel
//---------------------------------------------------------

SizeF Image::size2pixel(const SizeF& s) const
{
    return s * (_sizeIsSpatium ? spatium() : DPMM);
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Image::layout()
{
    setPos(0.0, 0.0);
    if (imageType == ImageType::SVG && !svgDoc) {
        if (_storeItem) {
            svgDoc = new SvgRenderer(_storeItem->buffer());
        }
    } else if (imageType == ImageType::RASTER && !rasterDoc) {
        if (_storeItem) {
            rasterDoc = imageProvider()->createPixmap(_storeItem->buffer());
            if (!rasterDoc->isNull()) {
                _dirty = true;
            }
        }
    }
    if (_size.isNull()) {
        _size = pixel2size(imageSize());
    }

    // if autoscale && inside a box, scale to box relevant size
    if (autoScale() && explicitParent() && ((explicitParent()->isHBox() || explicitParent()->isVBox()))) {
        if (_lockAspectRatio) {
            double f = _sizeIsSpatium ? spatium() : DPMM;
            SizeF size(imageSize());
            double ratio = size.width() / size.height();
            double w = parentItem()->width();
            double h = parentItem()->height();
            if ((w / h) < ratio) {
                _size.setWidth(w / f);
                _size.setHeight((w / ratio) / f);
            } else {
                _size.setHeight(h / f);
                _size.setWidth(h * ratio / f);
            }
        } else {
            _size = pixel2size(parentItem()->bbox().size());
        }
    }

    // in any case, adjust position relative to parent
    setbbox(RectF(PointF(), size2pixel(_size)));
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
        SizeF s = size2pixel(_size);
        setSizeIsSpatium(v.toBool());
        _size = pixel2size(s);
    }
    break;
    default:
        rv = EngravingItem::setProperty(propertyId, v);
        break;
    }
    setGenerated(false);
    _dirty = true;
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
