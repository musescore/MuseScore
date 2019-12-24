//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
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

#include "palettetree.h"

#include "globals.h"
#include "musescore.h"
#include "preferences.h"
#include "shortcut.h"

#include "libmscore/articulation.h"
#include "libmscore/fret.h"
#include "libmscore/icon.h"
#include "libmscore/image.h"
#include "libmscore/imageStore.h"
#include "libmscore/mscore.h"
#include "libmscore/score.h"
#include "libmscore/textbase.h"

#include "thirdparty/qzip/qzipreader_p.h"
#include "thirdparty/qzip/qzipwriter_p.h"

namespace Ms {

//---------------------------------------------------------
//   needsStaff
//    should a staff been drawn if e is used as icon in
//    a palette
//---------------------------------------------------------

static bool needsStaff(Element* e)
      {
      if (!e)
            return false;
      switch(e->type()) {
            case ElementType::CHORD:
            case ElementType::BAR_LINE:
            case ElementType::CLEF:
            case ElementType::KEYSIG:
            case ElementType::TIMESIG:
            case ElementType::REST:
            case ElementType::BAGPIPE_EMBELLISHMENT:
                  return true;
            default:
                  return false;
            }
      }

//---------------------------------------------------------
//   mimeData
//---------------------------------------------------------

template<class T>
static QByteArray mimeData(T* t)
      {
      QBuffer buffer;
      buffer.open(QIODevice::WriteOnly);
      XmlWriter xml(/* score */ nullptr, &buffer);
      xml.setClipboardmode(true);
      t->write(xml);
      buffer.close();
      return buffer.buffer();
      }

//---------------------------------------------------------
//   readMimeData
//---------------------------------------------------------

template<class T>
static std::unique_ptr<T> readMimeData(const QByteArray& data, const QString& tagName)
      {
      XmlReader e(data);
      e.setPasteMode(true);
      while (e.readNextStartElement()) {
            const QStringRef tag(e.name());
            if (tag == tagName) {
                  std::unique_ptr<T> t(new T);
                  if (!t->read(e))
                        return nullptr;
                  return t;
                  }
            else {
                  return nullptr;
                  }
            }
      return nullptr;
      }

//---------------------------------------------------------
//   PaletteCell::PaletteCell
//---------------------------------------------------------

PaletteCell::PaletteCell(std::unique_ptr<Element> e, const QString& _name, QString _tag, qreal _mag)
   : element(std::move(e)), name(_name), tag(_tag), mag(_mag)
      {
      drawStaff = needsStaff(element.get());
      }

//---------------------------------------------------------
//   PaletteCell::translationContext
//---------------------------------------------------------

const char* PaletteCell::translationContext() const
      {
      const ElementType type = element ? element->type() : ElementType::INVALID;
      switch (type) {
            case ElementType::ACCIDENTAL:
            case ElementType::ARTICULATION:
            case ElementType::BREATH:
            case ElementType::FERMATA:
            case ElementType::SYMBOL:
                  return "symUserNames"; // libmscore/sym.cpp, Sym::symUserNames
            case ElementType::CLEF:
                  return "clefTable"; // libmscore/clef.cpp, ClefInfo::clefTable[]
            case ElementType::KEYSIG:
                  return "MuseScore"; // libmscore/keysig.cpp, keyNames[]
            case ElementType::MARKER:
                  return "markerType"; // libmscore/marker.cpp, markerTypeTable[]
            case ElementType::JUMP:
                  return "jumpType"; // libmscore/jump.cpp, jumpTypeTable[]
            case ElementType::TREMOLO:
                  return "Tremolo"; // libmscore/tremolo.cpp, tremoloName[]
            case ElementType::BAGPIPE_EMBELLISHMENT:
                  return "bagpipe"; // libmscore/bagpembell.cpp, BagpipeEmbellishment::BagpipeEmbellishmentList[]
            case ElementType::TRILL:
                  return "trillType"; // libmscore/trill.cpp, trillTable[]
            case ElementType::VIBRATO:
                  return "vibratoType"; // libmscore/vibrato.cpp, vibratoTable[]
            case ElementType::CHORDLINE:
                  return "Ms"; // libmscore/chordline.cpp, scorelineNames[]
            case ElementType::NOTEHEAD:
                  return "noteheadnames"; // libmscore/note.cpp, noteHeadGroupNames[]
            case ElementType::ICON:
                  return "action"; // mscore/shortcut.cpp, Shortcut::_sc[]
            default:
                  break;
            }
      return "Palette";
      }

//---------------------------------------------------------
//   PaletteCell::translatedName
//---------------------------------------------------------

QString PaletteCell::translatedName() const
      {
      const QString trName(qApp->translate(translationContext(), name.toUtf8()));

      if (element && element->isTextBase() && name.contains("%1"))
            return trName.arg(toTextBase(element.get())->plainText());
      return trName;
      }

//---------------------------------------------------------
//   PaletteCell::retranslate
///   Retranslates cell content, e.g. text if the element
///   is TextBase.
//---------------------------------------------------------

void PaletteCell::retranslate()
      {
      if (untranslatedElement && element->isTextBase()) {
            TextBase* target = toTextBase(element.get());
            TextBase* orig = toTextBase(untranslatedElement.get());
            const QString& text = orig->xmlText();
            target->setXmlText(qApp->translate("Palette", text.toUtf8().constData()));
            }
      }

//---------------------------------------------------------
//   PaletteCell::setElementTranslated
//---------------------------------------------------------

void PaletteCell::setElementTranslated(bool translate)
      {
      if (translate && element) {
            untranslatedElement = std::move(element);
            element.reset(untranslatedElement->clone());
            retranslate();
            }
      else
            untranslatedElement.reset();
      }

//---------------------------------------------------------
//   PaletteCell::write
//---------------------------------------------------------

void PaletteCell::write(XmlWriter& xml) const
      {
      if (!element) {
            xml.tagE("Cell");
            return;
            }

      // using attributes for `custom` and `visible`
      // properties instead of nested tags for pre-3.3
      // version compatibility
      xml.stag(QString("Cell")
         + (!name.isEmpty() ? " name=\"" + XmlWriter::xmlString(name) + "\"" : "")
         + (custom ? " custom=\"1\"" : "")
         + (!visible ? " visible=\"0\"" : "")
         + (untranslatedElement ? " trElement=\"1\"" : "")
         );

      if (drawStaff)
            xml.tag("staff", drawStaff);
      if (xoffset)
            xml.tag("xoffset", xoffset);
      if (yoffset)
            xml.tag("yoffset", yoffset);
      if (!tag.isEmpty())
            xml.tag("tag", tag);
      if (mag != 1.0)
            xml.tag("mag", mag);

      if (untranslatedElement)
            untranslatedElement->write(xml);
      else
            element->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   PaletteCell::read
//---------------------------------------------------------

bool PaletteCell::read(XmlReader& e)
      {
      bool add = true;
      name = e.attribute("name");

      // using attributes instead of nested tags for
      // pre-3.3 version compatibility
      custom = e.hasAttribute("custom") ? e.intAttribute("custom") : false; // TODO: actually check master palette?
      visible = e.hasAttribute("visible") ? e.intAttribute("visible") : true;

      const bool translateElement = e.hasAttribute("trElement") ? e.intAttribute("trElement") : false;

      while (e.readNextStartElement()) {
            const QStringRef& s(e.name());
            if (s == "staff")
                  drawStaff = e.readInt();
            else if (s == "xoffset")
                  xoffset = e.readDouble();
            else if (s == "yoffset")
                  yoffset = e.readDouble();
            else if (s == "mag")
                  mag = e.readDouble();
            else if (s == "tag")
                  tag = e.readElementText();

            // added on palettes rework
            // TODO: remove or leave to switch from using attributes later?
            else if (s == "custom")
                  custom = e.readBool();
            else if (s == "visible")
                  visible = e.readBool();

            else {
                  element.reset(Element::name2Element(s, gscore));
                  if (!element)
                        e.unknown();
                  else {
                        element->read(e);
                        element->styleChanged();
                        if (element->type() == ElementType::ICON) {
                              Icon* icon = static_cast<Icon*>(element.get());
                              QAction* ac = getAction(icon->action());
                              if (ac) {
                                    QIcon qicon(ac->icon());
                                    icon->setAction(icon->action(), qicon);
                                    }
                              else {
                                    add = false; // action is not valid, don't add it to the palette.
                                    }
                              }
                        }
                  }
            }

      setElementTranslated(translateElement);

      return add && element;
      }

//---------------------------------------------------------
//   PaletteCell::readMimeData
//---------------------------------------------------------

PaletteCellPtr PaletteCell::readMimeData(const QByteArray& data)
      {
      return Ms::readMimeData<PaletteCell>(data, "Cell");
      }

//---------------------------------------------------------
//   PaletteCell::readMimeData
//---------------------------------------------------------

PaletteCellPtr PaletteCell::readElementMimeData(const QByteArray& data)
      {
      QPointF dragOffset;
      Fraction duration(1, 4);
      std::unique_ptr<Element> e(Element::readMimeData(gscore, data, &dragOffset, &duration));

      if (!e)
            return nullptr;

      if (!e->isSymbol()) // not sure this check is necessary, it was so in the old palette
            e->setTrack(0);

      if (e->isIcon()) {
            Icon* i = toIcon(e.get());
            const QByteArray& action = i->action();
            if (!action.isEmpty()) {
                  const Shortcut* s = Shortcut::getShortcut(action);
                  if (s) {
                        QAction* a = s->action();
                        QIcon icon(a->icon());
                        i->setAction(action, icon);
                        }
                  }
            }

      const QString name = (e->isFretDiagram()) ? toFretDiagram(e.get())->harmonyText() : e->userName();

      return PaletteCellPtr(new PaletteCell(std::move(e), name));
      }

//---------------------------------------------------------
//   PaletteCell::mimeData
//---------------------------------------------------------

QByteArray PaletteCell::mimeData() const
      {
      return Ms::mimeData(this);
      }

//---------------------------------------------------------
//   PaletteCell::readMimeData
//---------------------------------------------------------

std::unique_ptr<PalettePanel> PalettePanel::readMimeData(const QByteArray& data)
      {
      return Ms::readMimeData<PalettePanel>(data, "Palette");
      }

//---------------------------------------------------------
//   PalettePanel::read
//---------------------------------------------------------

bool PalettePanel::read(XmlReader& e)
      {
      _name = e.attribute("name");
      _type = Type::Unknown;
      while (e.readNextStartElement()) {
            const QStringRef tag(e.name());
            if (tag == "gridWidth")
                  _gridSize.setWidth(e.readDouble());
            else if (tag == "gridHeight")
                  _gridSize.setHeight(e.readDouble());
            else if (tag == "mag")
                  _mag = e.readDouble();
            else if (tag == "grid")
                  _drawGrid = e.readInt();
            else if (tag == "moreElements")
                  setMoreElements(e.readInt());
            else if (tag == "yoffset")
                  _yOffset = e.readDouble();
            else if (tag == "drumPalette")      // obsolete
                  e.skipCurrentElement();
            else if (tag == "type") {
                  bool ok;
                  const int t = QMetaEnum::fromType<Type>().keyToValue(e.readElementText().toLatin1().constData(), &ok);
                  if (ok)
                        _type = Type(t);
                  }
            else if (tag == "visible")
                  _visible = e.readBool();
            else if (e.pasteMode() && tag == "expanded")
                  _expanded = e.readBool();
            else if (tag == "editable")
                  _editable = e.readBool();
            else if (tag == "Cell") {
                  PaletteCellPtr cell(new PaletteCell);
                  if (!cell->read(e))
                        continue;
                  cells.push_back(std::move(cell));
                  }
            else
                  e.unknown();
            }
      // (from old palette): make sure hgrid and vgrid are not 0, we divide by them later
      if (_gridSize.width() <= 0)
            _gridSize.setWidth(28);
      if (_gridSize.width() <= 0)
            _gridSize.setHeight(28);

      if (_type == Type::Unknown)
            _type = guessType();

      return true;
      }

//---------------------------------------------------------
//   PalettePanel::mimeData
//---------------------------------------------------------

QByteArray PalettePanel::mimeData() const
      {
      return Ms::mimeData(this);
      }

//---------------------------------------------------------
//   PalettePanel::write
//---------------------------------------------------------

void PalettePanel::write(XmlWriter& xml) const
      {
      xml.stag(QString("Palette name=\"%1\"").arg(XmlWriter::xmlString(_name)));
      xml.tag("type", QMetaEnum::fromType<Type>().valueToKey(int(_type)));
      xml.tag("gridWidth", _gridSize.width());
      xml.tag("gridHeight", _gridSize.height());
      xml.tag("mag", _mag);
      if (_drawGrid)
            xml.tag("grid", _drawGrid);

      xml.tag("moreElements", _moreElements);
      if (_yOffset != 0.0)
            xml.tag("yoffset", _yOffset);

      xml.tag("visible", _visible, true);
      xml.tag("editable", _editable, true);

      if (xml.clipboardmode())
            xml.tag("expanded", _expanded, false);

      for (auto& cell: cells) {
//             if (cells[i] && cells[i]->tag == "ShowMore")
//                   continue;
            if (!cell) { // from old palette, not sure if it is still needed
                  xml.tagE("Cell");
                  continue;
                  }
            cell->write(xml);
            }
      xml.etag();
      }

//---------------------------------------------------------
//   writePaletteFailed
//---------------------------------------------------------

static void writePaletteFailed(const QString& path)
      {
      QString s = qApp->translate("Palette", "Writing Palette File\n%1\nfailed: ").arg(path); // reason?
      QMessageBox::critical(mscore, qApp->translate("Palette", "Writing Palette File"), s);
      }

//---------------------------------------------------------
//   PalettePanel::writeToFile
///   write as compressed zip file and include
///   images as needed
//---------------------------------------------------------

bool PalettePanel::writeToFile(const QString& p) const
      {
      QSet<ImageStoreItem*> images;
      size_t n = cells.size();
      for (size_t i = 0; i < n; ++i) {
            if (cells[i] == 0 || cells[i]->element == 0 || cells[i]->element->type() != ElementType::IMAGE)
                  continue;
            images.insert(toImage(cells[i]->element.get())->storeItem());
            }

      QString path(p);
      if (!path.endsWith(".mpal"))
            path += ".mpal";

      MQZipWriter f(path);
      // f.setCompressionPolicy(QZipWriter::NeverCompress);
      f.setCreationPermissions(
         QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner
         | QFile::ReadUser | QFile::WriteUser | QFile::ExeUser
         | QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup
         | QFile::ReadOther | QFile::WriteOther | QFile::ExeOther);

      if (f.status() != MQZipWriter::NoError) {
            writePaletteFailed(path);
            return false;
            }
      QBuffer cbuf;
      cbuf.open(QIODevice::ReadWrite);
      XmlWriter xml(gscore, &cbuf);
      xml.header();
      xml.stag("container");
      xml.stag("rootfiles");
      xml.stag(QString("rootfile full-path=\"%1\"").arg(XmlWriter::xmlString("palette.xml")));
      xml.etag();
      foreach (ImageStoreItem* ip, images) {
            QString ipath = QString("Pictures/") + ip->hashName();
            xml.tag("file", ipath);
            }
      xml.etag();
      xml.etag();
      cbuf.seek(0);
      //f.addDirectory("META-INF");
      //f.addDirectory("Pictures");
      f.addFile("META-INF/container.xml", cbuf.data());

      // save images
      foreach(ImageStoreItem* ip, images) {
            QString ipath = QString("Pictures/") + ip->hashName();
            f.addFile(ipath, ip->buffer());
            }
      {
      QBuffer cbuf1;
      cbuf1.open(QIODevice::ReadWrite);
      XmlWriter xml1(gscore, &cbuf1);
      xml1.header();
      xml1.stag("museScore version=\"" MSC_VERSION "\"");
      write(xml1);
      xml1.etag();
      cbuf1.close();
      f.addFile("palette.xml", cbuf1.data());
      }
      f.close();
      if (f.status() != MQZipWriter::NoError) {
            writePaletteFailed(path);
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   PalettePanel::readFromFile
//---------------------------------------------------------

bool PalettePanel::readFromFile(const QString& p)
      {
      QString path(p);
      if (!path.endsWith(".mpal"))
            path += ".mpal";

      MQZipReader f(path);
      if (!f.exists()) {
            qDebug("palette <%s> not found", qPrintable(path));
            return false;
            }
      cells.clear();

      QByteArray ba = f.fileData("META-INF/container.xml");

      XmlReader e(ba);
      // extract first rootfile
      QString rootfile = "";
      QList<QString> images;
      while (e.readNextStartElement()) {
            if (e.name() != "container") {
                  e.unknown();
                  break;;
                  }
            while (e.readNextStartElement()) {
                  if (e.name() != "rootfiles") {
                        e.unknown();
                        break;
                        }
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());

                        if (tag == "rootfile") {
                              if (rootfile.isEmpty())
                                    rootfile = e.attribute("full-path");
                              e.readNext();
                              }
                        else if (tag == "file")
                              images.append(e.readElementText());
                        else
                              e.unknown();
                        }
                  }
            }
      //
      // load images
      //
      foreach(const QString& s, images)
            imageStore.add(s, f.fileData(s));

      if (rootfile.isEmpty()) {
            qDebug("can't find rootfile in: %s", qPrintable(path));
            return false;
            }

      ba = f.fileData(rootfile);
      e.clear();
      e.addData(ba);
      while (e.readNextStartElement()) {
            if (e.name() == "museScore") {
                  QString version = e.attribute("version");
                  QStringList sl = version.split('.');
                  int versionId = sl[0].toInt() * 100 + sl[1].toInt();
                  gscore->setMscVersion(versionId); // TODO: what is this?

                  while (e.readNextStartElement()) {
                        if (e.name() == "Palette")
                              read(e);
                        else
                              e.unknown();
                        }
                  }
            else
                  e.unknown();
            }
      return true;
      }

//---------------------------------------------------------
//   PalettePanel::insert
//---------------------------------------------------------

PaletteCell* PalettePanel::insert(int idx, Element* e, const QString& name, QString tag, qreal mag)
      {
      if (e)
            e->layout(); // layout may be important for comparing cells, e.g. filtering "More" popup content
      PaletteCell* cell = new PaletteCell(std::unique_ptr<Element>(e), name, tag, mag);
      cells.emplace(cells.begin() + idx, cell);
      return cell;
      }

//---------------------------------------------------------
//   PalettePanel::append
//---------------------------------------------------------

PaletteCell* PalettePanel::append(Element* e, const QString& name, QString tag, qreal mag)
      {
      if (e)
            e->layout(); // layout may be important for comparing cells, e.g. filtering "More" popup content
      PaletteCell* cell = new PaletteCell(std::unique_ptr<Element>(e), name, tag, mag);
      cells.emplace_back(cell);
      return cell;
      }

//---------------------------------------------------------
//   PalettePanel::takeCells
//---------------------------------------------------------

std::vector<PaletteCellPtr> PalettePanel::takeCells(int idx, int count)
      {
      std::vector<PaletteCellPtr> removedCells;
      removedCells.reserve(count);

      if (idx < 0 || idx + count > int(cells.size()))
            return removedCells;

      auto removeBegin = cells.begin() + idx;
      auto removeEnd = removeBegin + count;

      removedCells.insert(removedCells.end(), std::make_move_iterator(removeBegin), std::make_move_iterator(removeEnd));
      cells.erase(removeBegin, removeEnd);

      return removedCells;
      }

//---------------------------------------------------------
//   PalettePanel::insertCells
//---------------------------------------------------------

bool PalettePanel::insertCells(int idx, std::vector<PaletteCellPtr> insertedCells)
      {
      if (idx < 0 || idx > int(cells.size()))
            return false;

      cells.insert(cells.begin() + idx, std::make_move_iterator(insertedCells.begin()), std::make_move_iterator(insertedCells.end()));

      return true;
      }

//---------------------------------------------------------
//   PalettePanel::insertCell
//---------------------------------------------------------

bool PalettePanel::insertCell(int idx, PaletteCellPtr cell)
      {
      if (idx < 0 || idx > int(cells.size()))
            return false;

      cells.insert(cells.begin() + idx, std::move(cell));

      return true;
      }

//---------------------------------------------------------
//   isSame
///   Helper function to compare two Elements
// TODO: make it operator==?
//---------------------------------------------------------

static bool isSame(const Element& e1, const Element& e2)
      {
      return e1.type() == e2.type()
         && e1.subtype() == e2.subtype()
         && e1.mimeData(QPointF()) == e2.mimeData(QPointF());
      }

//---------------------------------------------------------
//   PalettePanel::findPaletteCell
//---------------------------------------------------------

int PalettePanel::findPaletteCell(const PaletteCell& cell, bool matchName) const
      {
      const Element* el = cell.element.get();
      if (!el)
            return -1;

      for (int i = 0; i < int(cells.size()); ++i) {
            const PaletteCell& localCell = *cells[i];
            if (matchName && localCell.name != cell.name)
                  continue;
            const Element* exElement = localCell.element.get();
            if (exElement && !isSame(*exElement, *el))
                  continue;
            if (localCell.tag != cell.tag
                  || localCell.drawStaff != cell.drawStaff
                  || localCell.xoffset != cell.xoffset
                  || localCell.yoffset != cell.yoffset
                  || localCell.mag != cell.mag
                  || localCell.readOnly != cell.readOnly
                  || localCell.visible != cell.visible
                  || localCell.custom != cell.custom
                  )
                  continue;
            return i;
            }
      return -1;
      }

//---------------------------------------------------------
//   PalettePanel::guessType
//---------------------------------------------------------

PalettePanel::Type PalettePanel::guessType() const
      {
      if (cells.empty())
            return Type::Custom;

      const Element* e = nullptr;
      for (const auto& c : cells) {
            if (c->element) {
                  e = c->element.get();
                  break;
                  }
            }

      if (!e)
            return Type::Custom;

      switch (e->type()) {
            case ElementType::CLEF:
                  return Type::Clef;
            case ElementType::KEYSIG:
                  return Type::KeySig;
            case ElementType::TIMESIG:
                  return Type::TimeSig;
            case ElementType::BRACKET:
            case ElementType::BRACKET_ITEM:
                  return Type::Bracket;
            case ElementType::ACCIDENTAL:
                  return Type::Accidental;
            case ElementType::ARTICULATION:
            case ElementType::BEND:
                  return toArticulation(e)->isOrnament() ? Type::Ornament : Type::Articulation;
            case ElementType::FERMATA:
                  return Type::Articulation;
            case ElementType::BREATH:
                  return Type::Breath;
            case ElementType::NOTEHEAD:
                  return Type::NoteHead;
            case ElementType::BAR_LINE:
                  return Type::BarLine;
            case ElementType::ARPEGGIO:
            case ElementType::GLISSANDO:
                  return Type::Arpeggio;
            case ElementType::TREMOLO:
                  return Type::Tremolo;
            case ElementType::TEMPO_TEXT:
                  return Type::Tempo;
            case ElementType::DYNAMIC:
                  return Type::Dynamic;
            case ElementType::FINGERING:
                  return Type::Fingering;
            case ElementType::MARKER:
            case ElementType::JUMP:
            case ElementType::REPEAT_MEASURE:
                  return Type::Repeat;
            case ElementType::FRET_DIAGRAM:
                  return Type::FretboardDiagram;
            case ElementType::BAGPIPE_EMBELLISHMENT:
                  return Type::BagpipeEmbellishment;
            case ElementType::LAYOUT_BREAK:
            case ElementType::SPACER:
                  return Type::Break;
            case ElementType::SYMBOL:
                  return Type::Accordion;
            case ElementType::ICON: {
                  const Icon* i = toIcon(e);
                  const QByteArray& action = i->action();
                  if (action.contains("beam"))
                        return Type::Beam;
                  if (action.contains("grace") || action.contains("acciaccatura") || action.contains("appoggiatura"))
                        return Type::GraceNote;
                  if (action.contains("frame") || action.contains("box") || action.contains("measure"))
                        return Type::Frame;
                  return Type::Custom;
                  }
            default: {
                  if (e->isSpanner())
                        return Type::Line;
                  if (e->isTextBase())
                        return Type::Text;
                  }
            };

      return Type::Custom;
      }

//---------------------------------------------------------
//   PalettePanel::contentType
///   Returns palette type if it is defined or deduces it
///   from the palette content for custom palettes.
//---------------------------------------------------------

PalettePanel::Type PalettePanel::contentType() const
      {
      Type t = type();
      if (t == Type::Unknown || t == Type::Custom)
            t = guessType();

      if (t == Type::Unknown || t == Type::Custom)
            return Type::Clef; // if no type can be deduced, use Clef type by default

      return t;
      }

//---------------------------------------------------------
//   PalettePanel::retranslate
//---------------------------------------------------------

void PalettePanel::retranslate()
      {
      for (auto& c : cells)
            c->retranslate();
      }

//---------------------------------------------------------
//   PaletteTree::insert
///   PaletteTree takes the ownership over the PalettePanel
//---------------------------------------------------------

void PaletteTree::insert(int idx, PalettePanel* palette)
      {
      palettes.emplace(palettes.begin() + idx, palette);
      }

//---------------------------------------------------------
//   PaletteTreeModel::append
///   PaletteTree takes the ownership over the PalettePanel
//---------------------------------------------------------

void PaletteTree::append(PalettePanel* palette)
      {
      palettes.emplace_back(palette);
      }

//---------------------------------------------------------
//   PaletteTree::write
//---------------------------------------------------------

void PaletteTree::write(XmlWriter& xml) const
      {
      xml.stag("PaletteBox"); // for compatibility with old palettes file format
      for (const auto& p : palettes)
            p->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   PaletteTree::read
//---------------------------------------------------------

bool PaletteTree::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef tag(e.name());
            if (tag == "Palette") {
                  std::unique_ptr<PalettePanel> p(new PalettePanel);
                  p->read(e);
                  palettes.push_back(std::move(p));
                  }
            else
                  e.unknown();
            }
      return true;
      }

//---------------------------------------------------------
//   PaletteTree::retranslate
//---------------------------------------------------------

void PaletteTree::retranslate()
      {
      for (auto& p : palettes)
            p->retranslate();
      }

//---------------------------------------------------------
//   paintPaletteElement
//---------------------------------------------------------

static void paintPaletteElement(void* data, Element* e)
      {
      QPainter* p = static_cast<QPainter*>(data);
      p->save();
      p->translate(e->pos());
      e->draw(p);
      p->restore();
      }

//---------------------------------------------------------
//   PaletteCellIconEngine::paint
//---------------------------------------------------------

void PaletteCellIconEngine::paint(QPainter* painter, const QRect& r, QIcon::Mode mode, QIcon::State state)
      {
      const bool selected = mode == QIcon::Selected;

      const qreal oldSpatium = gscore->spatium();

      qreal _spatium = gscore->spatium();
//      qreal mag      = PALETTE_SPATIUM * _extraMag * guiScaling / _spatium;
      qreal mag      = PALETTE_SPATIUM * _extraMag / _spatium;
      gscore->setSpatium(SPATIUM20);

      QPainter& p = *painter;

      const int hgrid = r.width();
      const int vgrid = r.height();

      qreal dy = lrint(2 * PALETTE_SPATIUM * _extraMag);

      //
      // draw symbols
      //

      // QPen pen(palette().color(QPalette::Normal, QPalette::Text));
      QPen pen(Qt::black);
      pen.setWidthF(MScore::defaultStyle().value(Sid::staffLineWidth).toDouble() * PALETTE_SPATIUM * _extraMag);

      const qreal _yOffset = 0.0; // TODO

      const int yoffset  = gscore->spatium() * _yOffset;
      const QRect rShift = r.translated(0, yoffset);
      p.setPen(pen);
      QColor c(MScore::selectColor[0]);
      if (selected) {
            c.setAlpha(100);
            p.fillRect(r, c);
            }
      else if (state == QIcon::On) {
            c.setAlpha(60);
            p.fillRect(r, c);
            }

      PaletteCellConstPtr cc = cell();

      if (!cc)
            return;

      QString tag = cc->tag;
      if (!tag.isEmpty()) {
            p.setPen(Qt::darkGray);
            QFont f(p.font());
            f.setPointSize(12);
            p.setFont(f);
            if (tag == "ShowMore")
                  p.drawText(r, Qt::AlignCenter, "???");
            else
                  p.drawText(rShift, Qt::AlignLeft | Qt::AlignTop, tag);
            }

      p.setPen(pen);

      Element* el = cc->element.get();
      if (!el)
            return;

      const bool drawStaff = cc->drawStaff;

      qreal cellMag = cc->mag * mag;
      if (el->isIcon()) {
            toIcon(el)->setExtent((hgrid < vgrid ? hgrid : vgrid) - 4);
            cellMag = 1.0;
            }
      el->layout();

      if (drawStaff) {
            qreal y = r.y() + vgrid * .5 - dy + _yOffset * _spatium * cellMag;
            qreal x = r.x() + 3;
            qreal w = hgrid - 6;
            for (int i = 0; i < 5; ++i) {
                  qreal yy = y + PALETTE_SPATIUM * i * _extraMag;
                  p.drawLine(QLineF(x, yy, x + w, yy));
                  }
            }
      p.save();
      p.setRenderHint(QPainter::Antialiasing, true); // TODO: needed?
      p.scale(cellMag, cellMag);

      double gw = hgrid / cellMag;
      double gh = vgrid / cellMag;
      const double gx = r.x() + cc->xoffset * _spatium;
      const double gy = r.y() + cc->yoffset * _spatium;

      double sw = el->width();
      double sh = el->height();
      double sy;

      if (drawStaff)
            sy = gy + gh * .5 - 2.0 * _spatium;
      else
            sy  = gy + (gh - sh) * .5 - el->bbox().y();
      double sx  = gx + (gw - sw) * .5 - el->bbox().x();

      sy += _yOffset * _spatium;

      p.translate(sx, sy);

      QColor color;
      if (selected)
            color = QApplication::palette(mscore).color(QPalette::Normal, QPalette::HighlightedText);
      else {
            // show voice colors for notes
            if (el->isChord())
                  color = el->curColor();
            else
                  color = QApplication::palette(mscore).color(QPalette::Normal, QPalette::Text);
            }

      p.setPen(QPen(color));
      el->scanElements(&p, paintPaletteElement);
      p.restore();

      gscore->setSpatium(oldSpatium);
      }

} // namespace Ms
