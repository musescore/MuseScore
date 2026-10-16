// SPDX-License-Identifier: GPL-3.0-only
#ifndef NO_QT_SUPPORT
#include "svgdrawing.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QCoreApplication>
#include <QSet>
#include <QHash>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QRegularExpression>
#include <QDataStream>
#include <QIODevice>
#include <QElapsedTimer>
#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <memory>
#include <limits>
#include <tuple>
#include <functional>
#include "score.h"
#include "masterscore.h"
#include "../infrastructure/eidregister.h"
#include "../devtools/iengravingelementsprovider.h"
#include "factory.h"
#include "select.h"
#include "note.h"
#include "chord.h"
#include "measure.h"
#include "segment.h"
#include "system.h"
#include "page.h"
#include "staff.h"
#include "imageStore.h"
#include "draw/svgrenderer.h"
#include "log.h"
#include "../editing/undo.h"

namespace mu::engraving::SvgDrawing {
struct GeometryCache {
    QJsonArray source;
    QByteArray signature;
    QJsonArray resolved;
    bool hasFreePoints = false;
};
struct ImageCache {
    QJsonObject metadata;
    QSet<QString> attachedStaffIds;
    QString rigidSvg;
    QByteArray layoutSignature;
    QString fragmentSvg;
    QRectF fragmentBox;
    QJsonArray mappedCommands;
    QJsonObject fragmentInfo;
    std::shared_ptr<GeometryCache> geometry;
    bool geometryShared = false;
    bool anchorRangeReady = false;
    bool severalAnchoredMeasures = false;
    QJsonObject firstAnchor,lastAnchor;
};
namespace {
QSet<const Score*> suspendedDrawingLayout;
QJsonObject metadata(const Image*);
void migrateStaffIds(const Image*,QJsonObject&);
void restoreSerializedStaffIds(const Image*,QJsonObject&);
class ChangeDrawingMetadata final : public UndoCommand {
public:
    ChangeDrawingMetadata(Image* image, std::string data) : m_image(image), m_data(std::move(data)) {}
    const char* name() const override { return "ChangeDrawingMetadata"; }
    std::vector<EngravingObject*> objectItems() const override { return {m_image}; }
protected:
    void flip(EditData*) override {
        auto previous = m_image->drawingData();
        const auto cache = m_image->drawingCache();
        m_image->setDrawingData(m_data);m_data = std::move(previous);
        // These commands change only viewing limits/fragment handles. Keep
        // the expensive musical mapping and text backing asset alive.
        metadata(m_image);
        if (cache && cache->metadata.value("geometry")==m_image->drawingCache()->metadata.value("geometry")) {
            m_image->drawingCache()->geometry=cache->geometry;
            m_image->drawingCache()->geometryShared=cache->geometryShared;
            if(cache->metadata.value("source")==m_image->drawingCache()->metadata.value("source"))m_image->drawingCache()->rigidSvg=cache->rigidSvg;
        }
        m_image->triggerLayout();
    }
private:
    Image* m_image;
    std::string m_data;
};
class ChangeDrawingPart final : public UndoCommand {
public:
    ChangeDrawingPart(Image* image,std::string part):m_image(image),m_part(std::move(part)){}
    const char* name() const override {return "ChangeDrawingPart";}
    std::vector<EngravingObject*> objectItems() const override {return {m_image};}
protected:
    void flip(EditData*) override {
        auto previous=m_image->drawingPart();m_image->setDrawingPart(m_part);m_part=std::move(previous);m_image->triggerLayout();
    }
private:
    Image* m_image;
    std::string m_part;
};
struct Part {
    Measure* measure = nullptr;
    QString key;
    Fraction start;
    Fraction end;
    double origin = 0;
    double left = 0;
    double width = 0;
    bool fullMeasure = false;
};
Fraction fraction(const QJsonObject& o, const char* key) {
    bool ok = false;
    const Fraction value = Fraction::fromString(muse::String::fromQString(o.value(key).toString()), &ok);
    return ok && value.isValid() && value.denominator() > 0 ? value : Fraction(0, 1);
}
QString measureKey(const QString& key) {
    bool ok=false;
    const auto value=Fraction::fromString(muse::String::fromQString(key),&ok);
    return ok&&value.isValid()&&value.denominator()>0?value.reduced().toString().toQString():key;
}
bool sameMeasureKey(const QString& a,const QString& b) {return a==b||measureKey(a)==measureKey(b);}
QString measureIdentity(const Measure* measure) {
    auto id=measure->eid();if(!id.isValid())id=measure->assignNewEID();
    return QString::fromStdString(id.toStdString());
}
Measure* referencedMeasure(Score* score,const QJsonObject& reference) {
    const auto text=reference.value("measureId").toString();
    if(!text.isEmpty()) {
        static const QRegularExpression validId("^[A-Za-z0-9+/]{1,11}_[A-Za-z0-9+/]{1,11}$");
        const auto id=validId.match(text).hasMatch()?EID::fromStdString(text.toStdString()):EID::invalid();
        if(id.isValid()) {
            const auto item=score->masterScore()->eidRegister()->findItemFromEID(id);
            // EIDRegister may retain an object held by undo or an old address.
            // Check the live-object registry before dereferencing that address.
            if(item&&score->elementsProvider()&&score->elementsProvider()->elements().count(item)&&item->isMeasure()&&item->eid()==id) {
                auto measure=toMeasure(item);
                if(measure->score()->tick2measure(measure->tick())!=measure)return nullptr;
                // Excerpts have their own measure objects, at the same time.
                if(measure->score()!=score)measure=score->tick2measure(measure->tick());
                if(measure)return measure;
            }
        }
    }
    bool ok=false;const auto tick=Fraction::fromString(muse::String::fromQString(reference.value("key").toString().section(':',-1)),&ok);
    if(!ok||!tick.isValid())return nullptr;
    auto measure=score->tick2measure(tick);
    return measure&&measure->tick()==tick?measure:nullptr;
}
QJsonObject withMeasureIdentities(Score* score,const QJsonObject& document) {
    if(document.value("measureIdentityVersion").toInt()>=1)return document;
    QHash<QString,QString> identities;
    for(const auto& value:document.value("parts").toArray()) {
        const auto part=value.toObject();
        if(const auto measure=referencedMeasure(score,part))identities.insert(measureKey(part.value("key").toString()),measureIdentity(measure));
    }
    std::function<QJsonValue(const QJsonValue&)> bind=[&](const QJsonValue& value)->QJsonValue {
        if(value.isArray()){QJsonArray result;for(const auto& item:value.toArray())result.append(bind(item));return result;}
        if(!value.isObject())return value;
        auto object=value.toObject();
        for(auto it=object.begin();it!=object.end();++it)if(it.value().isArray()||it.value().isObject()) {
            auto child=bind(it.value());
            if(it.key().contains('/')&&child.isObject()) {
                auto fragment=child.toObject();
                if(const auto measure=referencedMeasure(score,QJsonObject{{"key",it.key()}}))fragment["measureId"]=measureIdentity(measure);
                child=fragment;
            }
            it.value()=child;
        }
        const auto key=object.value("key").toString();
        if(!key.isEmpty()) {
            const auto canonical=measureKey(key.section(':',-1));auto id=identities.value(canonical);
            if(id.isEmpty())if(const auto measure=referencedMeasure(score,object)) {
                id=measureIdentity(measure);identities.insert(canonical,id);
            }
            if(!id.isEmpty())object["measureId"]=id;
        }
        return object;
    };
    auto result=bind(document).toObject();result["measureIdentityVersion"]=1;return result;
}
// Time keys remain aliases inside a live drawing. Export current keys only at
// editor/save boundaries; inserting time does not rewrite every following SVG.
QJsonObject documentWithCurrentKeys(const Image* image) {
    const auto document=metadata(image);auto score=image->score();
    QHash<QString,QString> keys;
    bool changed=false;
    auto remember=[&](const QJsonObject& reference) {
        if(const auto measure=referencedMeasure(score,reference)) {
            const auto saved=measureKey(reference.value("key").toString().section(':',-1)),current=measure->tick().reduced().toString().toQString();
            keys.insert(saved,current);changed|=saved!=current;
            if(reference.contains("measureId"))changed|=reference.value("measureId").toString()!=measureIdentity(measure);
        }
    };
    const auto parts=document.value("parts").toArray();const auto source=document.value("source").toObject();
    for(const auto& part:parts)remember(part.toObject());
    if(parts.isEmpty())for(const auto& link:source.value("anchors").toArray())remember(link.toObject());
    remember(document.value("rigidAnchor").toObject());
    for(const auto& value:source.value("shapes").toArray()) {
        const auto shape=value.toObject();remember(shape.value("textLink").toObject());
        auto layouts=shape.value("fragmentOverridesByLayout").toObject();layouts["legacy"]=shape.value("fragmentOverrides");
        for(const auto& layout:layouts)for(const auto& fragment:layout.toObject()) {
            const auto override=fragment.toObject();
            for(const char* side:{"start","end"})remember(override.value(side).toObject().value("link").toObject());
        }
    }
    if(!changed)return document;
    auto currentKey=[&](const QString& key) {
        const int colon=key.lastIndexOf(':');const auto current=keys.value(measureKey(key.mid(colon+1)));
        return current.isEmpty()?key:key.left(colon+1)+current;
    };
    std::function<QJsonValue(const QJsonValue&)> update=[&](const QJsonValue& value)->QJsonValue {
        if(value.isArray()){QJsonArray result;for(const auto& item:value.toArray())result.append(update(item));return result;}
        if(!value.isObject())return value;
        QJsonObject result;const auto object=value.toObject();
        for(auto it=object.begin();it!=object.end();++it) {
            auto key=it.key();
            if(key.contains('/')) {
                const auto fragment=it.value().toObject();
                if(fragment.contains("measureId")) {
                    if(const auto measure=referencedMeasure(score,fragment))key=measure->tick().reduced().toString().toQString();
                }else key=currentKey(key);
            }
            result[key]=it.key()=="key"&&it.value().isString()?QJsonValue(currentKey(it.value().toString())):update(it.value());
        }
        if(object.contains("measureId"))if(const auto measure=referencedMeasure(score,object)) {
            result["measureId"]=measureIdentity(measure);
            if(object.contains("key")) {
                const auto oldKey=object.value("key").toString();const int colon=oldKey.lastIndexOf(':');
                result["key"]=oldKey.left(colon+1)+measure->tick().reduced().toString().toQString();
            }
        }
        return result;
    };
    return update(document).toObject();
}
// Measure::tick2pos has legacy coordinate conventions. Interpolate segment positions explicitly.
double rhythmicX(const Measure* measure, const Fraction& time) {
    double x1 = 0;
    Fraction t1(0, 1);
    for (Segment* s = measure->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        if (s->rtick() == time) {
            return s->x();
        }
        if (s->rtick() > time) {
            const Fraction dt = s->rtick() - t1;
            return dt.isZero() ? s->x() : x1 + (s->x() - x1) * (time - t1).ticks() / dt.ticks();
        }
        x1 = s->x();
        t1 = s->rtick();
    }
    const Fraction dt = measure->ticks() - t1;
    return dt.isZero() ? measure->width() : x1 + (measure->width() - x1) * (time - t1).ticks() / dt.ticks();
}
Fraction timeAtX(const Measure* measure, double x) {
    double x1 = 0;
    Fraction t1(0, 1);
    for (Segment* s = measure->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        if (s->x() >= x && s->x() > x1) {
            const double ratio = std::clamp((x - x1) / (s->x() - x1), 0.0, 1.0);
            return t1 + Fraction::fromTicks(std::llround((s->rtick() - t1).ticks() * ratio));
        }
        x1 = s->x();
        t1 = s->rtick();
    }
    const double width = measure->width() - x1;
    const double ratio = width > 0 ? std::clamp((x - x1) / width, 0.0, 1.0) : 0;
    return t1 + Fraction::fromTicks(std::llround((measure->ticks() - t1).ticks() * ratio));
}
QJsonObject metadata(const Image* i) {
    auto& cache = i->drawingCache();
    if (!cache) {
        cache = std::make_shared<ImageCache>();
        cache->metadata = QJsonDocument::fromJson(QByteArray::fromStdString(i->drawingData())).object();
        restoreSerializedStaffIds(i,cache->metadata);
        migrateStaffIds(i,cache->metadata);
        for(const auto& value:cache->metadata.value("source").toObject().value("anchors").toArray()) {
            const auto id=value.toObject().value("staffId").toString();
            if(!id.isEmpty())cache->attachedStaffIds.insert(id);
        }
        for(const auto& value:cache->metadata.value("source").toObject().value("shapes").toArray()) {
            const auto shape=value.toObject();
            auto layouts=shape.value("fragmentOverridesByLayout").toObject();
            layouts[QStringLiteral("legacy")]=shape.value("fragmentOverrides").toObject();
            for(const auto& overrides:layouts)for(const auto& fragment:overrides.toObject())for(const char* side:{"start","end"}) {
                const auto id=fragment.toObject().value(side).toObject().value("link").toObject().value("staffId").toString();
                if(!id.isEmpty())cache->attachedStaffIds.insert(id);
            }
        }
    }
    auto result = cache->metadata;
    // Staff indices are local to a score. Linked copies in individual parts
    // keep stable staff IDs, while their parent image supplies the local staff.
    if (result.value("nativeOwner").toString() == "MuseDrawNative"
        && result.value("staff").toInt(-1) != int(i->staffIdx())) result["staff"] = int(i->staffIdx());
    return result;
}
QString staffId(const Score* score, staff_idx_t index) {
    return score && index < score->nstaves() ? QString::fromStdString(score->staff(index)->id().toStdString()) : QString();
}
QString layoutKey(const Score* score) {
    QStringList ids;
    for(staff_idx_t index=0;index<score->nstaves();++index)ids.append(staffId(score,index));
    return (score->isMaster()?QStringLiteral("master:"):QStringLiteral("part:"))+ids.join(',');
}
void restoreSerializedStaffIds(const Image* image,QJsonObject& doc) {
    if(doc.value("nativeOwner").toString()!="MuseDrawNative"||!image->score())return;
    const auto saved=doc.value("scoreStaffIds").toArray();
    const auto score=image->score();
    if(saved.isEmpty()||saved.size()!=int(score->nstaves()))return;
    QHash<QString,QString> ids;
    bool changed=false;
    for(int index=0;index<saved.size();++index) {
        const auto current=staffId(score,staff_idx_t(index));
        ids.insert(saved[index].toString(),current);
        changed|=saved[index].toString()!=current;
    }
    const auto previousLayout=doc.value("scoreLayoutKey").toString();
    const auto currentLayout=layoutKey(score);
    if(!changed&&previousLayout==currentLayout)return;
    auto remap=[&](const QString& id) {
        // An anchor belonging to an absent staff falls back to the owning
        // staff. It must not collide with the new local numeric staff IDs.
        return ids.contains(id)?ids.value(id):(id.startsWith("foreign:")?id:QStringLiteral("foreign:")+id);
    };
    std::function<QJsonValue(const QJsonValue&)> visit;
    visit=[&](const QJsonValue& value)->QJsonValue {
        if(value.isArray()) {
            QJsonArray result;for(const auto& child:value.toArray())result.append(visit(child));return result;
        }
        if(!value.isObject())return value;
        auto result=value.toObject();
        for(auto it=result.begin();it!=result.end();++it) {
            if(it.key()=="staffId"&&it.value().isString()&&changed)it.value()=remap(it.value().toString());
            else if(it.key()=="staffMap"&&changed) {
                auto map=it.value().toObject();for(auto entry=map.begin();entry!=map.end();++entry)entry.value()=remap(entry.value().toString());it.value()=map;
            }else if(it.key()=="fragmentOverridesByLayout") {
                auto layouts=visit(it.value()).toObject();
                if(!previousLayout.isEmpty()&&layouts.contains(previousLayout)&&previousLayout!=currentLayout) {
                    const auto overrides=layouts.take(previousLayout);layouts[currentLayout]=overrides;
                }
                it.value()=layouts;
            }else it.value()=visit(it.value());
        }
        return result;
    };
    doc=visit(doc).toObject();
    QJsonArray currentIds;for(staff_idx_t index=0;index<score->nstaves();++index)currentIds.append(staffId(score,index));
    doc["scoreStaffIds"]=currentIds;doc["scoreLayoutKey"]=currentLayout;
}
staff_idx_t localStaff(const Score* score, const QJsonObject& point, staff_idx_t fallback) {
    const QString id = point.value("staffId").toString();
    if (!id.isEmpty() && score) for (staff_idx_t index = 0; index < score->nstaves(); ++index) {
        if (staffId(score, index) == id) return index;
    }
    const QString key=point.value("key").toString();
    if(id.isEmpty() && score && key.contains(':')) {
        const int index=key.section(':',0,0).toInt();
        if(index>=0&&index<int(score->nstaves()))return staff_idx_t(index);
    }
    return fallback;
}
QJsonObject localLink(const Score* score, QJsonObject link, staff_idx_t fallback) {
    const QString key = link.value("key").toString();
    if (key.contains(':')) link["key"] = QStringLiteral("%1:%2").arg(localStaff(score,link,fallback)).arg(key.section(':',-1));
    return link;
}
QJsonObject linkWithStaffId(const Score* score,QJsonObject link) {
    const auto key=link.value("key").toString();
    const auto id=link.value("staffId").toString();
    const auto mapped=localStaff(score,link,muse::nidx);
    if(key.contains(':')&&(id.isEmpty()||mapped!=muse::nidx)) {
        const int index=key.section(':',0,0).toInt();
        if(index>=0&&index<int(score->nstaves()))link["staffId"]=staffId(score,index);
    }
    // Editor keys describe the new target; do not retain an ID from a point
    // that the user has moved to another measure.
    auto current=link;current.remove("measureId");
    if(const auto measure=referencedMeasure(const_cast<Score*>(score),current))link["measureId"]=measureIdentity(measure);
    return link;
}
QJsonObject canonicalOverrides(const Score* score,QJsonObject overrides) {
    for(auto it=overrides.begin();it!=overrides.end();++it) {
        auto fragment=it.value().toObject();
        if(const auto measure=referencedMeasure(const_cast<Score*>(score),QJsonObject{{"key",it.key()}}))fragment["measureId"]=measureIdentity(measure);
        for(const char* side:{"start","end"}) {
            auto delta=fragment.value(side).toObject();
            if(!delta.value("link").toObject().isEmpty()) {delta["link"]=linkWithStaffId(score,delta.value("link").toObject());fragment[side]=delta;}
        }
        it.value()=fragment;
    }
    return overrides;
}
QJsonObject currentOverrides(const Score* score,const QJsonObject& shape) {
    const auto layouts=shape.value("fragmentOverridesByLayout").toObject();
    if(!layouts.isEmpty())return layouts.value(layoutKey(score)).toObject();
    return shape.value("fragmentOverrides").toObject();
}
QJsonObject withCurrentOverrides(const Score* score,QJsonObject shape,const QJsonObject& overrides) {
    auto layouts=shape.value("fragmentOverridesByLayout").toObject();
    layouts[layoutKey(score)]=canonicalOverrides(score,overrides);
    shape["fragmentOverridesByLayout"]=layouts;
    shape.remove("fragmentOverrides");
    return shape;
}
QJsonObject sourceWithStaffIds(Score* score, QJsonObject source) {
    QJsonArray anchors;
    QJsonObject staffMap;
    for (staff_idx_t index=0; index<score->nstaves(); ++index) staffMap[QString::number(index)] = staffId(score,index);
    for (const auto& value:source.value("anchors").toArray()) anchors.append(linkWithStaffId(score,value.toObject()));
    source["anchors"]=anchors;source["staffMap"]=staffMap;
    auto shapes=source.value("shapes").toArray();
    for (int index=0; index<shapes.size(); ++index) {
        auto shape=shapes[index].toObject();
        const auto link=shape.value("textLink").toObject();
        if (link.value("kind")=="rhythm") shape["textLink"]=linkWithStaffId(score,link);
        if(shape.contains("fragmentOverrides"))shape=withCurrentOverrides(score,shape,shape.value("fragmentOverrides").toObject());
        shapes[index]=shape;
    }
    source["shapes"]=shapes;return source;
}
void migrateStaffIds(const Image* image,QJsonObject& doc) {
    if(doc.value("nativeOwner").toString()!="MuseDrawNative"||doc.value("staffIdentityVersion").toInt()>=1||!image->score())return;
    // Older builds kept the staff only in the editable source anchor, then
    // stripped it from musical geometry. Recover it once per metadata load.
    const Score* origin=image->score();
    for(auto linked:image->linkList())if(linked->isImage()) {
        const auto linkedImage=toImage(linked);
        if(linkedImage->score()&&linkedImage->score()->isMaster()) {origin=linkedImage->score();break;}
    }
    auto source=doc.value("source").toObject();
    auto map=source.value("staffMap").toObject();
    const auto ownerId=staffId(image->score(),image->staffIdx());
    auto identify=[&](QJsonObject link) {
        if(!link.value("staffId").toString().isEmpty())return link;
        const auto key=link.value("key").toString();
        if(!key.contains(':'))return link;
        auto id=map.value(key.section(':',0,0)).toString();
        if(id.isEmpty())id=staffId(origin,key.section(':',0,0).toUInt());
        if(id.isEmpty())id=ownerId;
        link["staffId"]=id;return link;
    };
    QJsonArray anchors;
    std::map<std::tuple<int,int,int>,QString> anchorIds;
    for(const auto& value:source.value("anchors").toArray()) {
        const auto link=identify(value.toObject());anchors.append(link);
        anchorIds.emplace(std::make_tuple(link.value("shape").toInt(),link.value("command").toInt(),link.value("field").toInt()),link.value("staffId").toString());
    }
    source["anchors"]=anchors;
    if(map.isEmpty())for(staff_idx_t index=0;index<origin->nstaves();++index)map[QString::number(index)]=staffId(origin,index);
    source["staffMap"]=map;
    auto shapes=source.value("shapes").toArray();
    for(int index=0;index<shapes.size();++index) {
        auto shape=shapes[index].toObject();
        const auto link=shape.value("textLink").toObject();
        if(link.value("kind")=="rhythm")shape["textLink"]=identify(link);
        auto identifyOverrides=[&](QJsonObject overrides) {
            for(auto it=overrides.begin();it!=overrides.end();++it) {
                auto fragment=it.value().toObject();
                for(const char* side:{"start","end"}) {
                    auto delta=fragment.value(side).toObject();
                    if(!delta.value("link").toObject().isEmpty()) {delta["link"]=identify(delta.value("link").toObject());fragment[side]=delta;}
                }
                it.value()=fragment;
            }
            return overrides;
        };
        auto layouts=shape.value("fragmentOverridesByLayout").toObject();
        for(auto it=layouts.begin();it!=layouts.end();++it)it.value()=identifyOverrides(it.value().toObject());
        if(shape.contains("fragmentOverrides"))layouts[layoutKey(origin)]=identifyOverrides(shape.value("fragmentOverrides").toObject());
        if(!layouts.isEmpty())shape["fragmentOverridesByLayout"]=layouts;
        shape.remove("fragmentOverrides");
        shapes[index]=shape;
    }
    source["shapes"]=shapes;doc["source"]=source;
    auto fallbackId=doc.value("staffId").toString();
    if(fallbackId.isEmpty())fallbackId=map.value(QString::number(doc.value("staff").toInt(int(image->staffIdx())))).toString();
    if(fallbackId.isEmpty())fallbackId=ownerId;
    auto geometry=doc.value("geometry").toArray();
    for(int si=0;si<geometry.size();++si) {
        auto shape=geometry[si].toObject();auto commands=shape.value("commands").toArray();
        for(int ci=0;ci<commands.size();++ci) {
            auto command=commands[ci].toArray();
            for(int pi=1;pi<command.size();++pi) {
                auto point=command[pi].toObject();
                if(!point.value("staffId").toString().isEmpty())continue;
                const auto found=anchorIds.find({si,ci,(pi-1)*2+1});
                point["staffId"]=found==anchorIds.end()?fallbackId:found->second;
                command[pi]=point;
            }
            commands[ci]=command;
        }
        shape["commands"]=commands;auto frame=shape.value("svgFrame").toArray();
        for(int index=0;index<frame.size();++index) {
            auto point=frame[index].toObject();if(point.value("staffId").toString().isEmpty())point["staffId"]=fallbackId;frame[index]=point;
        }
        if(!frame.isEmpty())shape["svgFrame"]=frame;
        geometry[si]=shape;
    }
    doc["geometry"]=geometry;doc["staffIdentityVersion"]=1;
}
QSet<QString> drawingStaffIds(const Image* image) {
    QSet<QString> result;
    const auto doc=metadata(image),source=doc.value("source").toObject();
    const auto map=source.value("staffMap").toObject();
    result.insert(staffId(image->score(),image->staffIdx()));
    for(const auto& value:source.value("anchors").toArray()) {
        const auto link=value.toObject();
        auto id=link.value("staffId").toString();
        const auto key=link.value("key").toString();
        if(id.isEmpty())id=map.value(key.section(':',0,0)).toString();
        if(id.isEmpty()&&key.contains(':'))id=staffId(image->score(),key.section(':',0,0).toUInt());
        if(!id.isEmpty())result.insert(id);
    }
    return result;
}
QJsonObject localShapeLinks(const Score* score,QJsonObject shape,staff_idx_t fallback) {
    const auto text=shape.value("textLink").toObject();
    if(text.value("kind")=="rhythm")shape["textLink"]=localLink(score,text,fallback);
    auto overrides=currentOverrides(score,shape);
    for(auto it=overrides.begin();it!=overrides.end();++it) {
        auto fragment=it.value().toObject();
        for(const char* side:{"start","end"}) {
            auto delta=fragment.value(side).toObject();
            if(!delta.value("link").toObject().isEmpty()) {delta["link"]=localLink(score,delta.value("link").toObject(),fallback);fragment[side]=delta;}
        }
        it.value()=fragment;
    }
    // The editor sees only this score's handles; all saved layouts remain on
    // the shape so editing a part cannot erase the master score's overrides.
    shape["fragmentOverrides"]=overrides;
    return shape;
}
void changeLinkedMetadata(Score* score,Image* image,const QJsonObject& doc) {
    // Custom drawing metadata is not a built-in MuseScore property; update
    // the linked copies in the same undo transaction as the edited drawing.
    for(auto linked:image->linkList())if(linked->isImage()) {
        auto target=toImage(linked);
        auto targetDoc=doc;
        if(target->score()!=score&&target->score()->isMaster())targetDoc.remove("detachedPartCopy");
        const auto data=QJsonDocument(targetDoc).toJson(QJsonDocument::Compact).toStdString();
        if(target->drawingData()!=data)score->undo(new ChangeDrawingMetadata(target,data));
    }
}
std::vector<Image*> groupImages(Score* score, const QString& group) {
    std::vector<Image*> result;
    if (group.isEmpty()) {
        return result;
    }
    for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
        for (Segment* s = m->first(); s; s = s->next()) {
            if (s->measure() != m) {
                break;
            }
            for (EngravingItem* e : s->annotations()) {
                if (e->isImage() && metadata(toImage(e)).value("group").toString() == group) {
                    result.push_back(toImage(e));
                }
            }
        }
    }
    return result;
}
std::vector<Part> selectionParts(Score* score, const Image* selectedImage = nullptr) {
    std::vector<Part> parts;
    const Selection& sel = score->selection();
    if (selectedImage || (sel.element() && sel.element()->isImage())) {
        const auto image=selectedImage ? selectedImage : toImage(sel.element());
        const QJsonObject d = metadata(image);
        if (d.value("nativeOwner").toString() == "MuseDrawNative") {
            // Native fragments carry the complete measure membership. Looking
            // it up must not scan every annotation for every image/layout pass.
            std::vector<std::pair<Measure*,QJsonObject>> saved;
            for (const auto& value : d.value("parts").toArray()) {
                const auto p = value.toObject();
                if(auto measure=referencedMeasure(score,p))saved.emplace_back(measure,p);
            }
            std::stable_sort(saved.begin(),saved.end(),[](const auto& a,const auto& b){return a.first->tick()<b.first->tick();});
            for (const auto& member:saved) {
                const auto m=member.first;
                if (!m->system() || !m->system()->page() || !m->system()->staff(image->staffIdx())) return {};
                const auto& p = member.second;
                // Keep the authored spelling in Part so geometry, drawingPart
                // and fragment overrides stay coherent with existing files.
                parts.push_back({m,p.value("key").toString(),std::clamp(fraction(p,"start"),Fraction(0,1),m->ticks()),
                    std::clamp(fraction(p,"end"),Fraction(0,1),m->ticks()),0,0,0,p.value("fullMeasure").toBool()});
            }
        } else {
        const auto members = groupImages(score, d.value("group").toString());
        for (Image* i : members) {
            Segment* seg = i->explicitParent() && i->explicitParent()->isSegment() ? toSegment(i->explicitParent()) : nullptr;
            if (!seg) {
                continue;
            }
            QJsonArray saved = d.value("parts").toArray();
            for (const auto& v : saved) {
                QJsonObject o = v.toObject();
                if (o.value("key").toString().toStdString() == i->drawingPart()) {
                    parts.push_back({seg->measure(), o.value("key").toString(), fraction(o, "start"), fraction(o, "end"), 0, 0, 0, o.value("fullMeasure").toBool()});
                    break;
                }
            }
        }
        }
    } else {
        Measure* first = sel.isRange() && sel.startSegment() ? sel.startSegment()->measure() : sel.findMeasure();
        if (!first) {
            return parts;
        }
        Fraction end = sel.isRange() ? sel.tickEnd() : first->endTick();
        for (Measure* m = first; m && m->tick() < end; m = m->nextMeasure()) {
            Fraction start = sel.isRange() ? std::max(Fraction(0, 1), sel.tickStart() - m->tick()) : Fraction(0, 1);
            Fraction last = std::min(m->ticks(), end - m->tick());
            if (last <= start) {
                continue;
            }
            parts.push_back({m, QString::fromStdString(m->tick().toString().toStdString()), start, last});
        }
    }
    std::sort(parts.begin(), parts.end(), [](const Part& a, const Part& b) {return a.measure->tick() < b.measure->tick();});
    double origin = 0;
    for (Part& p : parts) {
        p.left = p.fullMeasure ? 0 : rhythmicX(p.measure, p.start);
        p.width = std::max(0.01, (p.fullMeasure ? p.measure->width() : rhythmicX(p.measure, p.end)) - p.left) / p.measure->spatium();
        p.origin = origin;
        origin += p.width;
    }
    return parts;
}
QJsonObject pointBinding(const std::vector<Part>& parts, double virtualX, double y) {
    const Part* chosen = &parts.front();
    for (const Part& p : parts) {
        chosen = &p;
        if (virtualX <= p.origin + p.width) {
            break;
        }
    }
    const double local = (virtualX - chosen->origin) * chosen->measure->spatium() + chosen->left;
    const Fraction time = timeAtX(chosen->measure, local);
    const double residual = (local - rhythmicX(chosen->measure, time)) / chosen->measure->spatium();
    return {{"key", chosen->key}, {"measureId",measureIdentity(chosen->measure)}, {"time", QString::fromStdString(time.toString().toStdString())},
            {"dx", residual}, {"y", y}};
}
std::vector<Image*> nativeImages(Score* score) {
    std::vector<Image*> result;
    if (!score) return result;
    for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
        for (Segment* s = m->first(); s && s->measure() == m; s = s->next()) {
            for (EngravingItem* e : s->annotations()) {
                if (e->isImage() && metadata(toImage(e)).value("nativeOwner").toString() == "MuseDrawNative") result.push_back(toImage(e));
            }
        }
    }
    return result;
}
std::vector<Part> nativeParts(Score* score, const QJsonObject& source, const QRectF& box, track_idx_t* track) {
    std::vector<Part> result;
    Measure* first = nullptr;
    Measure* last = nullptr;
    QSet<QString> keys;
    int staff = -1;
    for (const auto& v : source.value("anchors").toArray()) {
        const auto a = v.toObject();
        const QString key = a.value("key").toString();
        if (key.contains(':')) {
            if (staff < 0) staff = key.section(':', 0, 0).toInt();
            keys.insert(measureKey(key.section(':', -1)));
        }
    }
    staff = staff >= 0 && staff < int(score->nstaves()) ? staff : -1;
    const auto shapes = source.value("shapes").toArray();
    if (staff >= 0 && !shapes.isEmpty() && shapes[0].toObject().value("singleAnchor").toBool()) {
        // Rigid text and characters belong to their attachment measure, even
        // when the frame extends into a margin or over another system.
        for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
            const QString key = QString::fromStdString(m->tick().toString().toStdString());
            if (!keys.contains(measureKey(key)) || !m->system() || !m->system()->page() || !m->system()->staff(staff)) continue;
            *track = staff * VOICES;
            result.push_back({m, key, Fraction(0,1), m->ticks(), 0, 0, std::max(0.01, m->width()) / m->spatium(), true});
            return result;
        }
    }
    System* row = nullptr;
    double nearest = std::numeric_limits<double>::infinity();
    for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
        if (!m->system() || !m->system()->page()) continue;
        for (staff_idx_t st = 0; st < score->nstaves(); ++st) {
            if (staff >= 0 && int(st) != staff) continue;
            if (!m->system()->staff(st) || !score->staff(st)->show()) continue;
            const double dx = std::max({m->canvasPos().x() - box.center().x(), box.center().x() - m->canvasPos().x() - m->width(), 0.0});
            const double dy = m->system()->staffCanvasYpage(st) + 2 * m->spatium() - box.center().y();
            const double distance = dx * dx + dy * dy;
            if (distance < nearest) {nearest = distance;row = m->system();*track = st * VOICES;}
        }
    }
    if (!row) return result;
    for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
        const QString key = QString::fromStdString(m->tick().toString().toStdString());
        const bool linked = keys.contains(measureKey(key));
        const bool overlap = m->system() == row && m->canvasPos().x() <= box.right() && m->canvasPos().x() + m->width() >= box.left();
        if (linked || overlap) {if (!first) first = m;last = m;}
    }
    if (!first) {
        // A drawing entirely in a margin still belongs to the nearest measure.
        double best=std::numeric_limits<double>::infinity();
        for(Measure* m=score->firstMeasure();m;m=m->nextMeasure()) {
            if(m->system()!=row)continue;
            const double dx=std::abs(m->canvasPos().x()+m->width()/2-box.center().x());
            if(dx<best){best=dx;first=m;last=m;}
        }
    }
    double origin = 0;
    for (Measure* m = first; m; m = m->nextMeasure()) {
        if (!m->system() || !m->system()->page() || !m->system()->staff(*track / VOICES)) return {};
        const double width = std::max(0.01, m->width()) / m->spatium();
        result.push_back({m, QString::fromStdString(m->tick().toString().toStdString()), Fraction(0,1), m->ticks(), origin, 0, width, true});
        origin += width;
        if (m == last) break;
    }
    return result;
}
QJsonObject canvasBinding(const std::vector<Part>& parts, double x, double y, staff_idx_t staff) {
    const Part* nearest = &parts.front();
    double distance = std::numeric_limits<double>::infinity();
    for (const Part& p : parts) {
        if (!p.measure->system()) continue;
        const double dx = std::max({p.measure->canvasPos().x() - x, x - p.measure->canvasPos().x() - p.measure->width(), 0.0});
        const double dy = p.measure->system()->staffCanvasYpage(staff) - y;
        const double d = dx * dx + dy * dy;
        if (d < distance) {distance = d;nearest = &p;}
    }
    const double sp = nearest->measure->spatium();
    const double local = x - nearest->measure->canvasPos().x();
    const Fraction t = timeAtX(nearest->measure, local);
    return {{"key",nearest->key},{"measureId",measureIdentity(nearest->measure)},{"staffId",staffId(nearest->measure->score(),staff)},{"time",QString::fromStdString(t.toString().toStdString())},
            {"dx",(local-rhythmicX(nearest->measure,t))/sp},
            {"y",(y-nearest->measure->system()->staffCanvasYpage(staff))/sp}};
}
QRectF anchoredRigidBox(Score* score, const QJsonObject& source, staff_idx_t fallback) {
    const auto shapes = source.value("shapes").toArray();
    const auto links = source.value("anchors").toArray();
    if (!score || shapes.isEmpty() || links.isEmpty()) return {};
    const auto shape = shapes[0].toObject();
    if (!shape.value("singleAnchor").toBool() || !shape.value("textLink").toObject().isEmpty()) return {};
    const auto link = links[0].toObject(), box = source.value("bounds").toObject();
    const auto commands = shape.value("commands").toArray();
    const int ci = link.value("command").toInt(-1), field = link.value("field").toInt(-1);
    const double sourceScale = link.value("scale").toDouble();
    if (ci < 0 || ci >= commands.size() || field < 1 || field + 1 >= commands[ci].toArray().size()
        || sourceScale <= 0 || link.value("denominator").toInt() <= 0
        || !link.contains("offsetX") || !link.contains("offsetY")) return {};
    const QString key = link.value("key").toString();
    if (!key.contains(':')) return {};
    const auto staff = localStaff(score,link,fallback);
    if (staff >= score->nstaves()) return {};
    if (const auto measure=referencedMeasure(score,link)) {
        if (!measure->system() || !measure->system()->page() || !measure->system()->staff(staff))return {};
        const double sp = measure->spatium(), scale = sp / sourceScale;
        const auto command = commands[ci].toArray();
        const Fraction time(link.value("numerator").toInt(), link.value("denominator").toInt());
        // Resolve the same centre and offsets as the editor, without relying on
        // an image layout that may still belong to the preceding edit.
        return QRectF(measure->canvasPos().x() + rhythmicX(measure, time) + link.value("offsetX").toDouble() * sp
                          + (box.value("x").toDouble() - command[field].toDouble()) * scale,
                      measure->system()->staffCanvasYpage(staff) + link.value("offsetY").toDouble() * sp
                          + (box.value("y").toDouble() - command[field + 1].toDouble()) * scale,
                      box.value("width").toDouble() * scale, box.value("height").toDouble() * scale);
    }
    return {};
}
QString number(double v) {return QString::number(v, 'g', 12);}
QString assetSource(const QJsonObject& shape) {
    if (shape.value("kind").toString() != "text") return shape.value("svgSource").toString();
    // Rebuild from semantic text so old tspan-based backing images recover too.
    const double size = std::max(1.0, shape.value("textSize").toDouble(24));
    const double width = shape.value("textFrameWidth").toDouble();
    const double height = shape.value("textFrameHeight").toDouble(size * 1.5);
    if (width <= 0 || height <= 0) return shape.value("svgSource").toString();
    const auto layout = shape.value("textLayout").toObject();
    const double left = layout.value("left").toDouble(size * 0.15);
    const double baseline = layout.value("baseline").toDouble(size * 1.15);
    const double leading = layout.value("leading").toDouble(size * 1.35);
    const auto lines = shape.value("textContent").toString().split('\n');
    QString svg;
    QXmlStreamWriter writer(&svg);
    writer.writeStartElement("svg");
    writer.writeDefaultNamespace("http://www.w3.org/2000/svg");
    writer.writeAttribute("width", "1");writer.writeAttribute("height", "1");
    writer.writeAttribute("viewBox", "0 0 " + number(width) + " " + number(height));
    writer.writeStartElement("g");
    writer.writeAttribute("font-family", shape.value("textFont").toString("Arial"));
    writer.writeAttribute("font-size", number(std::round(size)));
    const int fontStyle=shape.value("textFontStyle").toInt();
    writer.writeAttribute("font-weight",fontStyle&1?"bold":"normal");
    writer.writeAttribute("font-style",fontStyle&2?"italic":"normal");
    QString decoration;
    if(fontStyle&4)decoration+="underline ";
    if(fontStyle&8)decoration+="line-through";
    if(!decoration.isEmpty())writer.writeAttribute("text-decoration",decoration.trimmed());
    const int alignment=shape.value("textAlignment").toInt(1);
    const double textX=alignment==4?(width+left-size*0.15)/2:alignment==2?width-size*0.15:left;
    writer.writeAttribute("fill", shape.value("style").toObject().value("stroke").toString("#161616"));
    writer.writeAttribute("xml:space", "preserve");
    for (int index = 0; index < lines.size(); ++index) {
        writer.writeStartElement("text");
        writer.writeAttribute("x", number(textX));
        writer.writeAttribute("text-anchor",alignment==4?"middle":alignment==2?"end":"start");
        writer.writeAttribute("y", number(baseline + index * leading));
        writer.writeCharacters(lines[index]);writer.writeEndElement();
    }
    writer.writeEndElement();writer.writeEndElement();
    return svg;
}
QString flattenedAsset(const QString& svg) {
    // Qt's SVG Tiny renderer skips nested <svg> viewports. Keep the glyphs,
    // text and paths, expressing each viewport as an ordinary group transform.
    QXmlStreamReader reader(svg);
    QString body;
    QXmlStreamWriter writer(&body);
    int depth=0;
    while(!reader.atEnd()) {
        reader.readNext();
        if(reader.isStartElement()) {
            const bool viewport=reader.name()==u"svg";
            const auto attributes=reader.attributes();
            writer.writeStartElement(viewport?QStringLiteral("g"):reader.name().toString());
            if(depth==0)writer.writeNamespace("http://www.w3.org/1999/xlink","xlink");
            if(viewport) {
                const auto view=attributes.value("viewBox").toString().split(QRegularExpression("[\\s,]+"),Qt::SkipEmptyParts);
                if(view.size()==4) {
                    const double vw=view[2].toDouble(),vh=view[3].toDouble();
                    auto dimension=[&](const char* key,double fallback) {
                        bool ok=false;const double value=attributes.value(QLatin1String(key)).toDouble(&ok);
                        return ok&&value>0?value:fallback;
                    };
                    const double width=depth==0?1:dimension("width",vw),height=depth==0?1:dimension("height",vh);
                    if(vw>0&&vh>0)writer.writeAttribute("transform",attributes.value("transform").toString()+
                        " translate("+number(attributes.value("x").toDouble())+" "+number(attributes.value("y").toDouble())+
                        ") scale("+number(width/vw)+" "+number(height/vh)+") translate("+number(-view[0].toDouble())+" "+number(-view[1].toDouble())+")");
                }
            }
            for(const auto& attribute:attributes) {
                const QString name=attribute.name().toString();
                if(viewport&&(name=="viewBox"||name=="width"||name=="height"||name=="x"||name=="y"||name=="preserveAspectRatio"||name=="transform"))continue;
                writer.writeAttribute(attribute.qualifiedName().toString(),attribute.value().toString());
            }
            ++depth;
        }else if(reader.isEndElement()) {writer.writeEndElement();--depth;}
        else if(reader.isCharacters())writer.writeCharacters(reader.text().toString());
    }
    return reader.hasError()?QString():body;
}
QString assetBody(const QString& svg, const QJsonArray& matrix, double opacity) {
    if (svg.isEmpty() || matrix.size() != 6) {
        return {};
    }
    QString values;
    for (const auto& value : matrix) {
        values += number(value.toDouble()) + " ";
    }
    return "<g opacity=\"" + number(opacity) + "\" transform=\"matrix(" + values + ")\">" + flattenedAsset(svg) + "</g>";
}
QRectF paintBounds(const QJsonArray& commands) {
    double left=1e30,top=1e30,right=-1e30,bottom=-1e30;
    for(const auto& value:commands){const auto c=value.toArray();for(int i=1;i+1<c.size();i+=2){left=std::min(left,c[i].toDouble());top=std::min(top,c[i+1].toDouble());right=std::max(right,c[i].toDouble());bottom=std::max(bottom,c[i+1].toDouble());}}
    return right>=left?QRectF(left,top,std::max(1.0,right-left),std::max(1.0,bottom-top)):QRectF(0,0,1,1);
}
QString gradientDefinition(const QJsonObject& gradient,const QString& id,const QRectF& frame) {
    const auto from=gradient.value("from").toObject(),to=gradient.value("to").toObject();
    const auto start=gradient.value("fromPoint").toArray(),end=gradient.value("toPoint").toArray();
    const double x1=start.size()==2?start[0].toDouble():frame.x()+from.value("x").toDouble()*frame.width();
    const double y1=start.size()==2?start[1].toDouble():frame.y()+from.value("y").toDouble()*frame.height();
    const double x2=end.size()==2?end[0].toDouble():frame.x()+to.value("x").toDouble()*frame.width();
    const double y2=end.size()==2?end[1].toDouble():frame.y()+to.value("y").toDouble()*frame.height();
    QString result="<linearGradient id=\""+id+"\" gradientUnits=\"userSpaceOnUse\" spreadMethod=\"pad\" x1=\""+number(x1)+"\" y1=\""+number(y1)+"\" x2=\""+number(x2)+"\" y2=\""+number(y2)+"\">";
    static const QRegularExpression colour("^#[0-9a-fA-F]{6}([0-9a-fA-F]{2})?$");
    for(const auto& value:gradient.value("stops").toArray()) {
        const auto stop=value.toObject();const auto color=stop.value("color").toString();
        if(!colour.match(color).hasMatch())continue;
        result+="<stop offset=\""+number(std::clamp(stop.value("offset").toDouble(),0.0,1.0))+"\" stop-color=\""+color+"\"/>";
    }
    return result+"</linearGradient>";
}
QString styledPath(const QString& path,const QJsonObject& style,const QString& fillRule=QStringLiteral("nonzero"),const QRectF& frame=QRectF(0,0,1,1)) {
    QString definitions,stroke=style.value("stroke").toString(),fill=style.value("fill").toString();
    const auto id=QStringLiteral("musedraw-gradient-")+QString::number(qHash(path+QString::fromUtf8(QJsonDocument(style).toJson(QJsonDocument::Compact))),16);
    for(const char* channel:{"stroke","fill"}) {
        const auto gradient=style.value(QString::fromLatin1(channel)+"Gradient").toObject();if(gradient.isEmpty())continue;
        const auto name=id+QLatin1String(channel);definitions+=gradientDefinition(gradient,name,frame);
        (QLatin1String(channel)==QLatin1String("stroke")?stroke:fill)="url(#"+name+")";
    }
    return (definitions.isEmpty()?QString():"<defs>"+definitions+"</defs>")+"<path d=\""+path+"\" stroke=\""+stroke+"\" fill=\""+fill+
        "\" stroke-width=\""+number(style.value("width").toDouble())+"\" opacity=\""+number(style.value("opacity").toDouble())+
        "\" fill-rule=\""+(fillRule=="evenodd"?QStringLiteral("evenodd"):QStringLiteral("nonzero"))+"\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>";
}
QString styledRanges(const QJsonArray& ranges,const QStringList& commands,const QRectF& frame=QRectF(0,0,1,1)) {
    QString body;
    for(const auto& value:ranges) {
        const auto range=value.toObject();
        const int start=range.value("start").toInt(-1),end=range.value("end").toInt(-1);
        if(start<0||end<=start||end>commands.size())continue;
        QString path;for(int i=start;i<end;++i)path+=commands[i];
        body+=styledPath(path,range.value("style").toObject(),range.value("fillRule").toString(),frame);
    }
    return body;
}
QString rigidSvg(const QJsonObject& source) {
    const auto box=source.value("bounds").toObject();
    const double width=box.value("width").toDouble(),height=box.value("height").toDouble();
    if(width<=0||height<=0)return {};
    QString body;
    for(const auto& v:source.value("shapes").toArray()) {
        const auto shape=v.toObject(),style=shape.value("style").toObject();
        if(!shape.value("svgSource").toString().isEmpty()) {
            body+=assetBody(assetSource(shape),shape.value("svgMatrix").toArray(),style.value("opacity").toDouble());continue;
        }
        QString path;QStringList pieces;
        const auto ranges=shape.value("paintRanges").toArray();
        for(const auto& cv:shape.value("commands").toArray()) {
            const int start=path.size();const auto c=cv.toArray();path+=c[0].toString();
            for(int i=1;i<c.size();++i)path+=number(c[i].toDouble())+" ";
            if(!ranges.isEmpty())pieces.append(path.mid(start));
        }
        const auto frame=paintBounds(shape.value("commands").toArray());
        body+=ranges.isEmpty()?styledPath(path,style,shape.value("fillRule").toString(),frame):styledRanges(ranges,pieces,frame);
    }
    return "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\""+number(box.value("x").toDouble())+" "+number(box.value("y").toDouble())+" "+
        number(width)+" "+number(height)+"\" width=\""+number(width)+"\" height=\""+number(height)+"\">"+body+"</svg>";
}
// Resolve the musical points once, then let each clipped fragment reuse
// them. Page-relative unlinked points require the current fragment's origin.
bool rigidArrowHeadPoint(const QJsonObject& head, const QJsonArray& commands, QPointF* result,
                         const std::function<QPointF(double,double)>& transform) {
    auto coordinate=[&](int index,QPointF* point) {
        if(index<0||index>=commands.size())return false;
        const auto command=commands[index].toArray();
        if(command.size()<2)return false;
        const auto xy=command[1].toArray();if(xy.size()!=2)return false;
        *point=transform(xy[0].toDouble(),xy[1].toDouble());return true;
    };
    QPointF tip,direction;
    if(head.isEmpty()||!coordinate(head.value("tip").toInt(-1),&tip)
        ||!coordinate(head.value("direction").toInt(-1),&direction))return false;
    if(head.contains("direction2")) {
        QPointF other;if(!coordinate(head.value("direction2").toInt(-1),&other))return false;
        direction=(direction+other)/2;
    }
    const double dx=tip.x()-direction.x(),dy=tip.y()-direction.y(),length=std::hypot(dx,dy);
    if(length<0.000001)return false;
    const double ux=dx/length,uy=dy/length,along=head.value("along").toDouble(),across=head.value("across").toDouble();
    *result=QPointF(tip.x()+ux*along-uy*across,tip.y()+uy*along+ux*across);
    return std::isfinite(result->x())&&std::isfinite(result->y());
}
QJsonArray resolveGeometry(const QJsonObject& doc, const std::vector<Part>& parts, const Part& current) {
    std::map<QString,const Part*> targets;
    for(const auto& p:parts)targets.emplace(p.key,&p);
    QHash<QString,staff_idx_t> staffIndexes;
    const auto score=current.measure->score();
    for(staff_idx_t index=0;index<score->nstaves();++index)staffIndexes.insert(staffId(score,index),index);
    const double sp=current.measure->spatium();
    auto pointAt=[&](const QJsonObject& point) {
        const auto found=targets.find(point.value("key").toString());
        const auto& p=found==targets.end()?parts.front():*found->second;
        double x=p.origin+(rhythmicX(p.measure,fraction(point,"time"))-p.left)/p.measure->spatium()+point.value("dx").toDouble();
        double y=point.value("y").toDouble();
        const auto originStaff=staff_idx_t(doc.value("staff").toInt());
        const auto pointStaff=staffIndexes.value(point.value("staffId").toString(),originStaff);
        if (pointStaff!=originStaff && p.measure->system()->staff(pointStaff) && p.measure->system()->staff(originStaff)) {
            y+=(p.measure->system()->staffCanvasYpage(pointStaff)-p.measure->system()->staffCanvasYpage(originStaff))/p.measure->spatium();
        }
        const auto free=point.value("free").toObject();
        if(!free.isEmpty())for(const auto page:p.measure->score()->pages()) {
            if(page->pageNumber()!=free.value("page").toInt())continue;
            x=current.origin+(page->canvasPos().x()+free.value("x").toDouble()-current.measure->canvasPos().x()-current.left)/sp;
            y=(page->canvasPos().y()+free.value("y").toDouble()-current.measure->system()->staffCanvasYpage(doc.value("staff").toInt()))/sp;break;
        }
        return QJsonArray{x,y};
    };
    QJsonArray result;
    for(const auto& value:doc.value("geometry").toArray()) {
        auto shape=value.toObject();QJsonArray commands;QJsonObject rigidHeads;int ci=0;
        for(const auto& cv:shape.value("commands").toArray()) {
            const auto c=cv.toArray();QJsonArray mapped{c.at(0)};
            for(int n=1;n<c.size();++n)mapped.append(pointAt(c.at(n).toObject()));
            if(c.size()>1) {
                const auto head=c[1].toObject().value("arrowHead").toObject();
                if(!head.isEmpty())rigidHeads[QString::number(ci)]=head;
            }
            commands.append(mapped);
            ++ci;
        }
        const auto anchored=commands;
        for(auto it=rigidHeads.begin();it!=rigidHeads.end();++it) {
            QPointF position;
            if(!rigidArrowHeadPoint(it.value().toObject(),anchored,&position,[](double x,double y){return QPointF(x,y);}))continue;
            auto command=commands[it.key().toInt()].toArray();command[1]=QJsonArray{position.x(),position.y()};commands[it.key().toInt()]=command;
        }
        shape["commands"]=commands;shape["resolvedArrowHeads"]=rigidHeads;
        auto resolvePaint=[&](QJsonObject style) {
            for(const char* channel:{"strokeGradient","fillGradient"}) {
                auto gradient=style.value(channel).toObject();if(gradient.isEmpty())continue;
                for(const char* side:{"from","to"}) {
                    const auto binding=gradient.value(QString::fromLatin1(side)+"Binding").toObject();
                    if(!binding.isEmpty())gradient[QString::fromLatin1(side)+"Point"]=pointAt(binding);
                }
                style[channel]=gradient;
            }return style;
        };
        shape["style"]=resolvePaint(shape.value("style").toObject());
        auto ranges=shape.value("paintRanges").toArray();
        for(int ri=0;ri<ranges.size();++ri){auto range=ranges[ri].toObject();range["style"]=resolvePaint(range.value("style").toObject());ranges[ri]=range;}
        if(!ranges.isEmpty())shape["paintRanges"]=ranges;
        QJsonArray frame;
        for(const auto& point:shape.value("svgFrame").toArray())frame.append(pointAt(point.toObject()));
        shape["svgFrame"]=frame;result.append(shape);
    }
    return result;
}
QString paintSvg(const QJsonObject& doc, const std::vector<Part>& parts, const QString& partKey, QRectF* resultBox,
                 QJsonArray* mappedCommands = nullptr, QJsonObject* fragmentInfo = nullptr,
                 const QJsonArray* resolved = nullptr) {
    std::map<QString, Part> targets;
    for (const Part& p : parts) {
        targets[p.key] = p;
    }
    if (!targets.count(partKey)) {
        return {};
    }
    const Part& current = targets.at(partKey);
    const auto at = std::find_if(parts.begin(), parts.end(), [&](const Part& p) {return p.key == partKey;});
    const bool firstPiece = at == parts.begin();
    const bool lastPiece = at + 1 == parts.end();
    const bool startsPiece = firstPiece || (at - 1)->measure->system() != current.measure->system();
    const bool endsPiece = lastPiece || (at + 1)->measure->system() != current.measure->system();
    const auto source = doc.value("source").toObject();
    const auto sources = source.value("shapes").toArray();
    QJsonObject override;
    if(!sources.isEmpty()) {
        const auto overrides=currentOverrides(current.measure->score(),sources[0].toObject());
        for(auto it=overrides.begin();it!=overrides.end();++it) {
            const auto fragment=it.value().toObject();
            if(fragment.contains("measureId")&&referencedMeasure(current.measure->score(),fragment)==current.measure){override=fragment;break;}
        }
        if(override.isEmpty())override=overrides.value(partKey).toObject();
    }
    const double sp = current.measure->spatium();
    const Page* page = current.measure->system()->page();
    double before = startsPiece && !firstPiece ? std::clamp(source.value("systemStartContinuation").toDouble(),0.0,1.0)
        * std::max(0.0,current.measure->pagePos().x()+current.left)/sp : 0;
    double after = endsPiece && !lastPiece && page ? std::clamp(source.value("systemContinuation").toDouble(),0.0,1.0)
        * std::max(0.0,page->width()-current.measure->pagePos().x()-current.left-current.width*sp)/sp : 0;
    if (override.value("detached").toBool()) {
        before = startsPiece&&!firstPiece?override.value("before").toDouble():0;
        after = endsPiece&&!lastPiece?override.value("after").toDouble():0;
    }
    const double left = current.origin-before, right = current.origin+current.width+after;
    auto endpointDelta=[&](const char* side,double pivot) {
        auto delta=override.value(side).toObject();
        const auto link=delta.value("link").toObject();
        auto found=targets.end();
        if(link.contains("measureId")) {
            const auto measure=referencedMeasure(current.measure->score(),link);
            found=std::find_if(targets.begin(),targets.end(),[&](const auto& target){return target.second.measure==measure;});
        }else found=targets.find(link.value("key").toString().section(':',-1));
        if(found!=targets.end()&&link.value("denominator").toInt()>0) {
            const Fraction time(link.value("numerator").toInt(),link.value("denominator").toInt());
            const auto& p=found->second;
            delta["dx"]=p.origin+(rhythmicX(p.measure,time)-p.left)/p.measure->spatium()+link.value("markerOffsetX").toDouble()/sp-pivot;
            const auto system=p.measure->system();
            const auto originStaff=staff_idx_t(doc.value("staff").toInt());
            const auto targetStaff=localStaff(p.measure->score(),link,originStaff);
            const double relativeY=system&&system->staff(targetStaff)&&system->staff(originStaff)
                ? (system->staffCanvasYpage(targetStaff)-system->staffCanvasYpage(originStaff))/p.measure->spatium() : 0;
            delta["dy"]=link.value("offsetY").toDouble()+relativeY-override.value(QString::fromLatin1(side)+"PivotY").toDouble();
        }
        return delta;
    };
    const auto startDelta=startsPiece?endpointDelta("start",override.value("startPivot").toDouble(left)):QJsonObject();
    const auto endDelta=endsPiece?endpointDelta("end",override.value("endPivot").toDouble(right)):QJsonObject();
    auto deform = [&](double x, double y) {
        const double pivotLeft=override.value("startPivot").toDouble(left),pivotRight=override.value("endPivot").toDouble(right);
        const double u = std::clamp((x-pivotLeft)/std::max(0.01,pivotRight-pivotLeft),0.0,1.0);
        const double a = std::max(0.0,1-2*u), b = std::max(0.0,2*u-1);
        return QPointF(x+a*startDelta.value("dx").toDouble()+b*endDelta.value("dx").toDouble(),
                       y+a*startDelta.value("dy").toDouble()+b*endDelta.value("dy").toDouble());
    };
    if (fragmentInfo) *fragmentInfo = QJsonObject{{"startsPiece",startsPiece},{"endsPiece",endsPiece},
        {"derived",current.measure->system()!=parts.front().measure->system()},{"before",before},{"after",after},{"spatium",sp},{"partKey",current.measure->tick().reduced().toString().toQString()},
        {"cutLeft",left+startDelta.value("dx").toDouble()},{"cutRight",right+endDelta.value("dx").toDouble()}};
    double minY = std::numeric_limits<double>::infinity(), maxY = -minY;
    double minX = minY, maxX = maxY;
    QString body;
    const QJsonArray geometry = resolved ? *resolved : resolveGeometry(doc,parts,current);
    for (const auto& shapeValue : geometry) {
        const QJsonObject shape = shapeValue.toObject();
        auto deformPaint=[&](QJsonObject style) {
            for(const char* channel:{"strokeGradient","fillGradient"}) {
                auto gradient=style.value(channel).toObject();if(gradient.isEmpty())continue;
                for(const char* side:{"fromPoint","toPoint"}) {
                    const auto point=gradient.value(side).toArray();if(point.size()!=2)continue;
                    const auto position=deform(point[0].toDouble(),point[1].toDouble());gradient[side]=QJsonArray{position.x(),position.y()};
                }
                style[channel]=gradient;
            }return style;
        };
        const QJsonObject style = deformPaint(shape.value("style").toObject());
        QString path;QStringList pieces;
        auto ranges=shape.value("paintRanges").toArray();
        for(int ri=0;ri<ranges.size();++ri){auto range=ranges[ri].toObject();range["style"]=deformPaint(range.value("style").toObject());ranges[ri]=range;}
        int commandIndex=0;
        QJsonArray drawn;
        const auto shapeCommands=shape.value("commands").toArray();
        for (const auto& commandValue : shapeCommands) {
            const auto head=shape.value("resolvedArrowHeads").toObject().value(QString::number(commandIndex++)).toObject();
            const int pathStart=path.size();
            const QJsonArray command = commandValue.toArray();
            QJsonArray mapped{command.at(0)};
            path += command.at(0).toString();
            for (int n = 1; n < command.size(); ++n) {
                const auto point=command.at(n).toArray();
                const double originalX=point.at(0).toDouble(),originalY=point.at(1).toDouble();
                auto position = deform(originalX,originalY);
                if(n==1&&!head.isEmpty())rigidArrowHeadPoint(head,shapeCommands,&position,deform);
                const double x = position.x(), y = position.y();
                path += number(x) + " " + number(y) + " ";
                mapped.append(x);mapped.append(y);
                const double pad = style.value("width").toDouble() / 2 + 0.1;
                minX = std::min(minX,x-pad);maxX = std::max(maxX,x+pad);
                minY = std::min(minY, y - pad);
                maxY = std::max(maxY, y + pad);
            }
            if (mappedCommands) mappedCommands->append(mapped);
            drawn.append(mapped);
            if(!ranges.isEmpty())pieces.append(path.mid(pathStart));
        }
        if (!shape.value("svgSource").toString().isEmpty()) {
            const QJsonArray frame = shape.value("svgFrame").toArray();
            std::vector<QPointF> points;
            for (const auto& value : frame) {
                const auto point=value.toArray();
                points.emplace_back(point.at(0).toDouble(),point.at(1).toDouble());
            }
            if (points.size() == 3) {
                body += assetBody(assetSource(shape), QJsonArray{
                    points[1].x() - points[0].x(), points[1].y() - points[0].y(),
                    points[2].x() - points[0].x(), points[2].y() - points[0].y(), points[0].x(), points[0].y()}, style.value("opacity").toDouble());
            }
            continue;
        }
        const auto frame=paintBounds(drawn);
        body+=ranges.isEmpty()?styledPath(path,style,shape.value("fillRule").toString(),frame):styledRanges(ranges,pieces,frame);
    }
    if (!std::isfinite(minY) || !std::isfinite(maxY)) {
        return {};
    }
    const double height = std::max(0.1, maxY - minY);
    // Only internal musical boundaries clip a drawing. The original outer
    // edges retain strokes drawn in the page margins, including their caps.
    const double fragmentOrigin = firstPiece ? std::min(left,minX) : left+startDelta.value("dx").toDouble();
    const double fragmentRight = lastPiece ? std::max(right,maxX) : right+endDelta.value("dx").toDouble();
    const double fragmentWidth = std::max(0.01,fragmentRight-fragmentOrigin);
    if(fragmentInfo) {
        if(firstPiece)(*fragmentInfo)["cutLeft"]=fragmentOrigin;
        if(lastPiece)(*fragmentInfo)["cutRight"]=fragmentRight;
    }
    *resultBox = QRectF(fragmentOrigin, minY, fragmentWidth, height);
    return "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" + number(fragmentWidth * 100)
           + "\" height=\"" + number(height * 100) + "\" viewBox=\""
           + number(fragmentOrigin) + " " + number(minY) + " " + number(fragmentWidth) + " " + number(height)
           + "\" preserveAspectRatio=\"none\">" + body + "</svg>";
}
QString cachedRigidSvg(const Image* image) {
    const auto doc = metadata(image);
    auto& cache = *image->drawingCache();
    if (cache.rigidSvg.isEmpty()) cache.rigidSvg = rigidSvg(doc.value("source").toObject());
    return cache.rigidSvg;
}

bool uniformPartCopy(const Image* image,const QJsonObject& doc) {
    return image->score()&&(!image->score()->isMaster()||doc.value("detachedPartCopy").toBool())
        &&doc.value("nativeOwner")=="MuseDrawNative"&&!doc.value("rigid").toBool();
}
struct WholePartPlacement {
    QRectF box;
    QRectF sourceBox;
    Measure* measure=nullptr;
    QJsonObject anchor;
    double scale=1;
};
WholePartPlacement wholePartPlacement(const Image* image,const QJsonObject& doc) {
    WholePartPlacement result;
    const auto parts=doc.value("parts").toArray();
    if(parts.isEmpty()||parts.first().toObject().value("key").toString().toStdString()!=image->drawingPart())return result;
    const auto segment=image->explicitParent()&&image->explicitParent()->isSegment()?toSegment(image->explicitParent()):nullptr;
    const auto measure=segment?segment->measure():nullptr;
    if(!measure||!measure->system()||!measure->system()->page()||!measure->system()->staff(image->staffIdx()))return result;
    const auto source=doc.value("source").toObject();
    const auto bounds=source.value("bounds").toObject();
    result.sourceBox=QRectF(bounds.value("x").toDouble(),bounds.value("y").toDouble(),bounds.value("width").toDouble(),bounds.value("height").toDouble());
    if(result.sourceBox.width()<=0||result.sourceBox.height()<=0)return result;
    const auto shapes=source.value("shapes").toArray(),geometry=doc.value("geometry").toArray();
    if(shapes.isEmpty()||geometry.isEmpty())return result;
    const auto shape=shapes.first().toObject();
    const double storedWidth=geometry.first().toObject().value("style").toObject().value("width").toDouble();
    const double sourceWidth=shape.value("style").toObject().value("width").toDouble();
    const double referenceSpatium=storedWidth>0?sourceWidth/storedWidth:measure->spatium();
    if(referenceSpatium<=0||!std::isfinite(referenceSpatium))return result;
    const double sp=measure->spatium();
    const auto page=measure->system()->page();
    const auto lastMember=referencedMeasure(image->score(),parts.last().toObject());
    Measure* last=measure;
    while(last->nextMeasure()&&last->nextMeasure()->system()==measure->system()
          &&last!=lastMember)last=last->nextMeasure();
    const double left=measure->canvasPos().x()+rhythmicX(measure,Fraction(0,1));
    const double right=last->canvasPos().x()+last->width()-0.3*sp;
    const double staffY=measure->system()->staffCanvasYpage(image->staffIdx());
    const double top=std::max(page->canvasPos().y()+3*sp,staffY-3*sp);
    const double bottom=page->canvasPos().y()+page->height()-3*sp;
    const double scale=std::min({sp/referenceSpatium,std::max(0.01,right-left)/result.sourceBox.width(),
                                std::max(0.01,bottom-top)/result.sourceBox.height()});
    double x=left,y=staffY;
    const auto anchors=source.value("anchors").toArray();
    if(!anchors.isEmpty()) {
        const auto link=anchors.first().toObject();
        const int ci=link.value("command").toInt(-1),field=link.value("field").toInt(-1);
        const auto commands=shape.value("commands").toArray();
        if(ci>=0&&ci<commands.size()&&field>=1&&field+1<commands[ci].toArray().size()) {
            const auto c=commands[ci].toArray();
            x=left+(result.sourceBox.x()-c[field].toDouble()+link.value("offsetX").toDouble()*referenceSpatium)*scale;
            y=staffY+(result.sourceBox.y()-c[field+1].toDouble()+link.value("offsetY").toDouble()*referenceSpatium)*scale;
        }
        result.anchor=link;
    }
    const double width=result.sourceBox.width()*scale,height=result.sourceBox.height()*scale;
    result.box=QRectF(std::clamp(x,left,std::max(left,right-width)),std::clamp(y,top,std::max(top,bottom-height)),width,height);
    result.measure=measure;result.scale=scale;
    return result;
}

// Only musical geometry participates in this key. Panning, selection and
// repeated layout passes must not regenerate every command of every fragment.
QByteArray fragmentLayoutSignature(const Image* image, const std::vector<Part>& parts, bool includePart = true, bool includePlacement = true) {
    QByteArray key;
    QDataStream out(&key, QIODevice::WriteOnly);
    out << (includePart ? QString::fromStdString(image->drawingPart()) : QString()) << quint32(parts.size());
    for (const Part& p : parts) {
        const auto m = p.measure;
        const auto system = m->system();
        const auto page = system ? system->page() : nullptr;
        out << p.key << p.origin << p.left << p.width << m->spatium()
            << QString::fromStdString(m->ticks().toString().toStdString());
        if(includePlacement)out << m->canvasPos().x() << m->canvasPos().y() << m->pagePos().x()
            << quint64(reinterpret_cast<quintptr>(system))
            << (page ? page->width() : 0.0)
            << (system && system->staff(image->staffIdx()) ? system->staffCanvasYpage(image->staffIdx()) : 0.0);
        // A shared drawing may have points attached to several staves. Their
        // relative spacing can change independently in a part or full score.
        if(system && system->staff(image->staffIdx()))for(const auto& id:image->drawingCache()->attachedStaffIds) {
            const auto staff=localStaff(image->score(),QJsonObject{{"staffId",id}},image->staffIdx());
            if(staff!=image->staffIdx())out << id << (system->staff(staff) ? system->staffCanvasYpage(staff)-system->staffCanvasYpage(image->staffIdx()) : 0.0);
        }
        for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            out << QString::fromStdString(s->rtick().toString().toStdString()) << s->x();
        }
        out << QString();
    }
    // Unlinked points can refer to another page. Include those page positions
    // as well, so page reflow never reuses a stale clipped fragment.
    if(includePlacement)for (const auto page : image->score()->pages()) {
        out << qint32(page->pageNumber()) << page->canvasPos().x() << page->canvasPos().y();
    }
    return key;
}

const QJsonArray& resolvedGeometryFor(const Image* image, const QJsonObject& doc, const std::vector<Part>& parts) {
    metadata(image);
    auto& imageCache=*image->drawingCache();
    const auto source=doc.value("geometry").toArray();
    if(!imageCache.geometry||imageCache.geometry->source!=source) {
        imageCache.geometry=std::make_shared<GeometryCache>();imageCache.geometry->source=source;imageCache.geometryShared=false;
        for(const auto& value:source)for(const auto& command:value.toObject().value("commands").toArray()) {
            const auto c=command.toArray();
            for(int n=1;n<c.size();++n)if(!c[n].toObject().value("free").toObject().isEmpty())imageCache.geometry->hasFreePoints=true;
        }
    }
    if(!imageCache.geometryShared&&!imageCache.geometry->hasFreePoints) {
        const auto members=groupImages(image->score(),doc.value("group").toString());
        for(Image* member:members) {
            metadata(member);
            auto& other=*member->drawingCache();
            if(other.geometry&&other.geometry->source==source&&other.geometryShared){imageCache.geometry=other.geometry;break;}
        }
        for(Image* member:members) {
            if(metadata(member).value("geometry").toArray()!=source)continue;
            auto& other=*member->drawingCache();
            other.geometry=imageCache.geometry;other.geometryShared=true;
        }
    }
    auto& cache=*imageCache.geometry;
    const auto key=fragmentLayoutSignature(image,parts,cache.hasFreePoints,cache.hasFreePoints);
    if(cache.signature!=key) {
        const auto current=std::find_if(parts.begin(),parts.end(),[&](const Part& p){return p.key.toStdString()==image->drawingPart();});
        if(current!=parts.end())cache.resolved=resolveGeometry(doc,parts,*current);
        else cache.resolved={};
        cache.signature=key;
    }
    return cache.resolved;
}

QString cachedPaintSvg(const Image* image, const QJsonObject& doc, const std::vector<Part>& parts,
                       QRectF* box, QJsonArray* mapped = nullptr, QJsonObject* info = nullptr) {
    metadata(image);
    auto& cache = *image->drawingCache();
    const auto signature = fragmentLayoutSignature(image, parts);
    if (cache.layoutSignature != signature) {
        cache.mappedCommands = {};cache.fragmentInfo = {};
        cache.fragmentSvg = paintSvg(doc, parts, QString::fromStdString(image->drawingPart()),
                                     &cache.fragmentBox, &cache.mappedCommands, &cache.fragmentInfo, &resolvedGeometryFor(image,doc,parts));
        cache.layoutSignature = signature;
    }
    *box = cache.fragmentBox;
    if (mapped) *mapped = cache.mappedCommands;
    if (info) *info = cache.fragmentInfo;
    return cache.fragmentSvg;
}

bool validPaint(const QJsonObject& style) {
    static const QRegularExpression colour("^#[0-9a-fA-F]{6}([0-9a-fA-F]{2})?$");
    for(const char* key:{"strokeGradient","fillGradient"}) {
        if(!style.contains(key))continue;
        if(!style.value(key).isObject())return false;
        const auto gradient=style.value(key).toObject();
        for(const char* side:{"from","to"}) {
            const auto point=gradient.value(side).toObject();
            for(const char* axis:{"x","y"})if(!point.value(axis).isDouble()||!std::isfinite(point.value(axis).toDouble())||std::abs(point.value(axis).toDouble())>100000)return false;
        }
        const auto stops=gradient.value("stops").toArray();if(stops.size()<2||stops.size()>128)return false;
        for(const auto& value:stops) {
            const auto stop=value.toObject();const double offset=stop.value("offset").toDouble(-1);
            if(!std::isfinite(offset)||offset<0||offset>1||!colour.match(stop.value("color").toString()).hasMatch())return false;
        }
    }return true;
}
bool validSource(const QJsonObject& source) {
    if (source.value("format") != "musescore-drawing" || source.value("version").toInt() != 1) {
        return false;
    }
    const QJsonArray shapes = source.value("shapes").toArray();
    if (shapes.isEmpty() || shapes.size() > 1000) {
        return false;
    }
    for (const auto& v : shapes) {
        const QJsonObject shape = v.toObject();
        const QJsonObject style = shape.value("style").toObject();
        const auto validColor = [](QString c) {
            if (c.size() != 7 || c.at(0) != '#') {
                return false;
            }
            for (int i = 1; i < c.size(); ++i) {
                if (!QString("0123456789abcdefABCDEF").contains(c.at(i))) {
                    return false;
                }
            }
            return true;
        };
        if (!validPaint(style)||(style.value("stroke") != "none" && !validColor(style.value("stroke").toString()))
            || (style.value("fill") != "none" && !validColor(style.value("fill").toString()))
            || !(style.value("width").toDouble() > 0) || style.value("width").toDouble() > 1000000
            || style.value("opacity").toDouble() < 0 || style.value("opacity").toDouble() > 1) {
            return false;
        }
        const QJsonArray commands = shape.value("commands").toArray();
        if (commands.isEmpty() || commands.size() > 100000) {
            return false;
        }
        for (const auto& cv : commands) {
            const QJsonArray c = cv.toArray();
            if (c.isEmpty()) {
                return false;
            }
            const QString op = c.at(0).toString();
            const int count = op == "M" || op == "L" ? 3 : op == "C" ? 7 : op == "Z" ? 1 : 0;
            if (count == 0 || c.size() != count) {
                return false;
            }
            for (int n = 1; n < c.size(); ++n) {
                if (!c.at(n).isDouble() || !std::isfinite(c.at(n).toDouble()) || std::abs(c.at(n).toDouble()) > 1e7) {
                    return false;
                }
            }
        }
        if(shape.contains("paintRanges")) {
            if(!shape.value("paintRanges").isArray())return false;
            const auto ranges=shape.value("paintRanges").toArray();
            if(ranges.size()>10000)return false;
            for(const auto& value:ranges) {
                const auto range=value.toObject(),rangeStyle=range.value("style").toObject();
                const int start=range.value("start").toInt(-1),end=range.value("end").toInt(-1);
                if(!validPaint(rangeStyle)||start<0||end<=start||end>commands.size()
                    ||(rangeStyle.value("stroke")!="none"&&!validColor(rangeStyle.value("stroke").toString()))
                    ||(rangeStyle.value("fill")!="none"&&!validColor(rangeStyle.value("fill").toString()))
                    ||!(rangeStyle.value("width").toDouble()>0)||rangeStyle.value("width").toDouble()>1000000
                    ||!std::isfinite(rangeStyle.value("opacity").toDouble())||rangeStyle.value("opacity").toDouble()<0
                    ||rangeStyle.value("opacity").toDouble()>1)return false;
            }
        }
    }
    return true;
}
}
std::string serializedData(const Image* image) {
    if(metadata(image).value("nativeOwner").toString()!="MuseDrawNative"||!image->score())return image->drawingData();
    auto doc=documentWithCurrentKeys(image);
    QJsonArray ids;
    for(staff_idx_t index=0;index<image->score()->nstaves();++index)ids.append(staffId(image->score(),index));
    doc["scoreStaffIds"]=ids;doc["scoreLayoutKey"]=layoutKey(image->score());
    if(!image->score()->isMaster())doc["detachedPartCopy"]=true;
    return QJsonDocument(doc).toJson(QJsonDocument::Compact).toStdString();
}
std::string serializedPart(const Image* image) {
    const auto doc=metadata(image);
    if(doc.value("nativeOwner")!="MuseDrawNative")return image->drawingPart();
    for(const auto& value:doc.value("parts").toArray()) {
        const auto part=value.toObject();if(part.value("key").toString().toStdString()!=image->drawingPart())continue;
        if(const auto measure=referencedMeasure(image->score(),part))return measure->tick().reduced().toString().toStdString();
    }
    return image->drawingPart();
}
std::vector<Staff*> linkedStaves(const Image* image) {
    if(!image||!image->staff())return {};
    auto result=image->staff()->staffList();
    const auto ids=drawingStaffIds(image);
    for(Score* target:image->score()->scoreList()) {
        if(std::any_of(result.begin(),result.end(),[&](const Staff* staff){return staff->score()==target;}))continue;
        for(Staff* staff:target->staves())if(ids.contains(staffId(target,staff->idx()))) {result.push_back(staff);break;}
    }
    return result;
}
staff_idx_t excerptSourceStaff(const Image* image,const std::vector<staff_idx_t>& sourceStaves) {
    if(!image||metadata(image).value("nativeOwner").toString()!="MuseDrawNative")return muse::nidx;
    if(std::find(sourceStaves.begin(),sourceStaves.end(),image->staffIdx())!=sourceStaves.end())return image->staffIdx();
    const auto ids=drawingStaffIds(image);
    for(const auto staff:sourceStaves)if(ids.contains(staffId(image->score(),staff)))return staff;
    return muse::nidx;
}
QString selectedDocument(Score* score) {
    if (!score || !score->selection().element() || !score->selection().element()->isImage()) {
        return {};
    }
    const auto image=toImage(score->selection().element());
    const auto doc=metadata(image).value("nativeOwner")=="MuseDrawNative"?documentWithCurrentKeys(image):metadata(image);
    return QString::fromUtf8(QJsonDocument(doc.value("source").toObject())
                             .toJson(QJsonDocument::Compact));
}
QVariantList editingPoints(Score* score, const Image* suppliedImage) {
    QVariantList result;
    const auto e = suppliedImage ? suppliedImage : score ? score->selection().element() : nullptr;
    if (!e || !e->isImage()) {
        return result;
    }
    const auto image = toImage(e);
    const auto doc = metadata(image);
    if(uniformPartCopy(image,doc)) {
        const auto placement=wholePartPlacement(image,doc);
        if(placement.box.isEmpty())return result;
        const auto source=doc.value("source").toObject();
        const auto shapes=source.value("shapes").toArray();
        for(int si=0;si<shapes.size();++si) {
            const auto commands=shapes[si].toObject().value("commands").toArray();
            for(int ci=0;ci<commands.size();++ci) {
                const auto c=commands[ci].toArray();
                for(int field=1;field+1<c.size();field+=2) {
                    result.append(QVariantMap{{"shape",si},{"command",ci},{"field",field},
                        {"x",placement.box.x()+(c[field].toDouble()-placement.sourceBox.x())*placement.scale},
                        {"y",placement.box.y()+(c[field+1].toDouble()-placement.sourceBox.y())*placement.scale}});
                }
            }
        }
        return result;
    }
    const auto parts = doc.value("rigid").toBool() ? std::vector<Part>() : selectionParts(score, image);
    if(!doc.value("rigid").toBool()&&doc.value("nativeOwner").toString()=="MuseDrawNative"&&parts.empty())return result;
    if (parts.empty()) {
        const auto source = doc.value("source").toObject();
        const auto box = source.value("bounds").toObject();
        const double width = box.value("width").toDouble(), height = box.value("height").toDouble();
        if (width <= 0 || height <= 0) {
            return result;
        }
        const auto attachedBox = anchoredRigidBox(score, source,image->staffIdx());
        const RectF current = attachedBox.isEmpty() ? image->canvasBoundingRect()
            : RectF(attachedBox.x(), attachedBox.y(), attachedBox.width(), attachedBox.height());
        const auto shapes = source.value("shapes").toArray();
        for (int si = 0; si < shapes.size(); ++si) {
            const auto commands = shapes.at(si).toObject().value("commands").toArray();
            for (int ci = 0; ci < commands.size(); ++ci) {
                const auto command = commands.at(ci).toArray();
                for (int field = 1; field + 1 < command.size(); field += 2) {
                    result.append(QVariantMap{{"shape", si}, {"command", ci}, {"field", field},
                        {"x", current.x() + (command.at(field).toDouble() - box.value("x").toDouble()) / width * current.width()},
                        {"y", current.y() + (command.at(field + 1).toDouble() - box.value("y").toDouble()) / height * current.height()}});
                }
            }
        }
        return result;
    }
    const auto geometry = doc.value("geometry").toArray();
    const auto& resolved = resolvedGeometryFor(image,doc,parts);
    QHash<QString,staff_idx_t> staffIndexes;
    for(staff_idx_t index=0;index<score->nstaves();++index)staffIndexes.insert(staffId(score,index),index);
    for (int si = 0; si < geometry.size(); ++si) {
        const auto commands = geometry.at(si).toObject().value("commands").toArray();
        for (int ci = 0; ci < commands.size(); ++ci) {
            const auto command = commands.at(ci).toArray();
            for (int pi = 1; pi < command.size(); ++pi) {
                auto point = command.at(pi).toObject();
                const auto head=point.value("arrowHead").toObject();
                const bool rigidHead=!head.isEmpty();
                if(rigidHead) {
                    const int tip=head.value("tip").toInt(-1);
                    if(tip<0||tip>=commands.size())continue;
                    const auto tipCommand=commands[tip].toArray();if(tipCommand.size()<2)continue;
                    point=tipCommand[1].toObject();
                }
                const auto target = std::find_if(parts.begin(), parts.end(), [&](const Part& p) {
                    return p.key == point.value("key").toString();
                });
                if (target == parts.end() || !target->measure->system()) {
                    continue;
                }
                const Fraction time = fraction(point, "time");
                const double sp = target->measure->spatium();
                double canvasX=target->measure->canvasPos().x()+rhythmicX(target->measure,time)+point.value("dx").toDouble()*sp;
                const auto targetStaff=staffIndexes.value(point.value("staffId").toString(),image->staffIdx());
                if(targetStaff>=score->nstaves()||!target->measure->system()->staff(targetStaff))continue;
                double canvasY=target->measure->system()->staffCanvasYpage(targetStaff)+point.value("y").toDouble()*sp;
                const auto free=point.value("free").toObject();
                if(!free.isEmpty())for(const auto page:score->pages()) {
                    if(page->pageNumber()!=free.value("page").toInt())continue;
                    canvasX=page->canvasPos().x()+free.value("x").toDouble();canvasY=page->canvasPos().y()+free.value("y").toDouble();break;
                }
                if(rigidHead&&si<resolved.size()) {
                    const auto mapped=resolved[si].toObject().value("commands").toArray();
                    const int tip=head.value("tip").toInt(-1);
                    if(ci>=mapped.size()||tip<0||tip>=mapped.size())continue;
                    const auto here=mapped[ci].toArray(),there=mapped[tip].toArray();
                    if(here.size()<2||there.size()<2)continue;
                    const auto p=here[1].toArray(),t=there[1].toArray();if(p.size()!=2||t.size()!=2)continue;
                    canvasX+=(p[0].toDouble()-t[0].toDouble())*sp;
                    canvasY+=(p[1].toDouble()-t[1].toDouble())*sp;
                }
                result.append(QVariantMap{{"shape", si}, {"command", ci}, {"field", (pi - 1) * 2 + 1},
                                          {"key", rigidHead?QString():target->measure->tick().reduced().toString().toQString()}, {"staff",int(targetStaff)},{"staffId",point.value("staffId")},{"numerator", time.numerator()},
                                          {"denominator", time.denominator()}, {"spatium", sp},
                                          {"offsetX", point.value("dx").toDouble()}, {"offsetY", point.value("y").toDouble()},
                                          {"x",canvasX},{"y",canvasY}});
            }
        }
    }
    return result;
}

static QVariantList anchorsFiltered(Score* score, int subdivision, const std::set<std::pair<int,staff_idx_t>>* targets) {
    QVariantList result;
    if (!score || (subdivision != 0 && subdivision != 4 && subdivision != 8 && subdivision != 16 && subdivision != 32)) {
        return result;
    }
    // Follow the displayed measures, including consolidated multi-measure rests.
    // The underlying hidden measures do not have a usable system layout.
    for (Measure* measure = score->firstMeasureMM(); measure; measure = measure->nextMeasureMM()) {
        const auto system = measure->system();
        if (!system || !system->page()) {
            continue;
        }
        const auto page = system->page();
        if(targets&&std::none_of(targets->begin(),targets->end(),[&](const auto& target){return target.first==int(page->pageNumber());}))continue;
        const auto pageBox = page->canvasBoundingRect();
        for (staff_idx_t staff = 0; staff < score->nstaves(); ++staff) {
            if(targets&&!targets->count({int(page->pageNumber()),staff}))continue;
            if (!system->staff(staff) || !score->staff(staff)->show()) {
                continue;
            }
            const double y = system->staffCanvasYpage(staff), sp = measure->spatium();
            const auto row = QStringLiteral("%1:%2:%3").arg(system->page()->pageNumber())
                             .arg(system->canvasPos().y(), 0, 'g', 12).arg(staff);
            const auto key = QStringLiteral("%1:%2").arg(staff)
                             .arg(QString::fromStdString(measure->tick().toString().toStdString()));
            if(subdivision==0) {
                for(Segment* segment=measure->first(SegmentType::ChordRest);segment;segment=segment->next(SegmentType::ChordRest)) {
                    const EngravingItem* symbol=nullptr;
                    for(int voice=0;voice<VOICES;++voice) {
                        const auto element=segment->element(staff*VOICES+voice);
                        if(element&&element->isChord()){symbol=toChord(element)->upNote();break;}
                        if(element&&element->isRest()&&!symbol)symbol=element;
                    }
                    if(!symbol)continue;
                    const Fraction time=segment->rtick();
                    const auto box=symbol->canvasBoundingRect();
                    const double bindingX=measure->canvasPos().x()+rhythmicX(measure,time);
                    result.append(QVariantMap{{"x",box.center().x()},{"bindingX",bindingX},{"y",y},{"railY",box.top()-0.75*sp},
                        {"row",row},{"staff",int(staff)},{"key",key},{"numerator",time.numerator()},{"denominator",time.denominator()},{"spatium",sp},
                        {"page",int(page->pageNumber())},{"pageX",pageBox.x()},{"pageY",pageBox.y()},
                        {"pageWidth",pageBox.width()},{"pageHeight",pageBox.height()}});
                }
                continue;
            }
            for (Fraction t(0, 1); t <= measure->ticks(); t += Fraction(1, subdivision)) {
                const double x = measure->canvasPos().x() + rhythmicX(measure, t);
                result.append(QVariantMap{{"x", x}, {"y", y}, {"railY", y - 2.5 * sp},
                    {"row", row}, {"staff", int(staff)}, {"key", key},
                    {"numerator", t.numerator()}, {"denominator", t.denominator()}, {"spatium", sp},
                    {"page",int(page->pageNumber())},{"pageX",pageBox.x()},{"pageY",pageBox.y()},
                    {"pageWidth",pageBox.width()},{"pageHeight",pageBox.height()}});
            }
        }
    }
    return result;
}
QVariantList anchors(Score* score,int subdivision) {return anchorsFiltered(score,subdivision,nullptr);}
QString insert(Score* score, const QString& sourceDocument, const QString& mode,
               const QRectF& sourceBounds, const RectF& canvasBounds, bool onScore, bool ownTransaction, bool reuseFragments) {
    if (!score) {
        return QStringLiteral("Abra uma partitura.");
    }
    if (sourceDocument.size() > 16 * 1024 * 1024) {
        return QStringLiteral("O desenho excede o tamanho máximo de 16 MB.");
    }
    const QJsonObject source = sourceWithStaffIds(score,QJsonDocument::fromJson(sourceDocument.toUtf8()).object());
    if (!validSource(source) || sourceBounds.width() <= 0 || sourceBounds.height() <= 0) {
        return QStringLiteral("O desenho contém dados inválidos.");
    }
    const auto oldElement = score->selection().element();
    const Image* oldImage = oldElement && oldElement->isImage() ? toImage(oldElement) : nullptr;
    const QJsonObject old = oldImage ? metadata(oldImage) : QJsonObject();
    const bool native = source.value("nativeOwner").toString() == "MuseDrawNative";
    QString group = native ? QStringLiteral("native-") + source.value("shapes").toArray().at(0).toObject().value("drawId").toString()
                           : old.value("group").toString();
    if (group.isEmpty()) {
        group = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    QJsonObject doc{{"version", 1}, {"group", group}, {"source", source}};
    if (native) {
        doc.insert("nativeOwner", "MuseDrawNative");
        doc.insert("measureIdentityVersion",1);
        doc.insert("nativeIndex", source.value("nativeIndex"));
        doc.insert("staffIdentityVersion",1);
    }
    std::vector<std::unique_ptr<Image>> pending;
    std::vector<std::pair<Image*,std::string>> reused;
    std::string reusedData;
    std::shared_ptr<GeometryCache> sharedGeometry;
    const auto reusableMembers=native&&reuseFragments?groupImages(score,group):std::vector<Image*>();
    if (mode == "note" && native) {
        track_idx_t track = 0;
        auto parts = nativeParts(score, source, sourceBounds, &track);
        if (parts.empty()) return QCoreApplication::translate("drawing-tools","The score layout is not available yet.");
        const auto& part = parts.front();
        auto segment = part.measure->first(SegmentType::ChordRest);
        if (!segment || !part.measure->system()) return QCoreApplication::translate("drawing-tools","The score layout is not available yet.");
        auto renderingSource=source;
        renderingSource["bounds"]=QJsonObject{{"x",sourceBounds.x()},{"y",sourceBounds.y()},{"width",sourceBounds.width()},{"height",sourceBounds.height()}};
        const QString svg=rigidSvg(renderingSource);
        const double sp = part.measure->spatium();
        doc["rigid"] = true;
        doc["fixedX"] = (sourceBounds.x()-segment->canvasPos().x())/sp;
        doc["fixedY"] = (sourceBounds.y()-part.measure->system()->staffCanvasYpage(track/VOICES))/sp;
        doc["fixedWidth"] = sourceBounds.width()/sp;doc["fixedHeight"] = sourceBounds.height()/sp;
        const auto links = source.value("anchors").toArray();
        const auto sourceShapes=source.value("shapes").toArray();
        if(!sourceShapes.isEmpty()&&sourceShapes[0].toObject().value("suppressAutoAnchors").toBool()&&links.isEmpty()) {
            const auto page=part.measure->system()->page();
            doc["rigidFree"]=QJsonObject{{"page",int(page->pageNumber())},{"x",sourceBounds.x()-page->canvasPos().x()},{"y",sourceBounds.y()-page->canvasPos().y()}};
        }
        if (!links.isEmpty()) {
            const auto link = links[0].toObject();
            const QString key = link.value("key").toString().section(':',-1);
            const auto target = std::find_if(parts.begin(),parts.end(),[&](const Part& p){return sameMeasureKey(p.key,key);});
            if (target!=parts.end()&&link.value("denominator").toInt()>0) {
                const Fraction time(link.value("numerator").toInt(),link.value("denominator").toInt());
                const double anchorX=target->measure->canvasPos().x()+rhythmicX(target->measure,time);
                doc["rigidAnchor"]=QJsonObject{{"key",key},{"measureId",measureIdentity(target->measure)},{"time",QString::fromStdString(time.toString().toStdString())}};
                doc["fixedX"]=(sourceBounds.x()-anchorX)/sp;
            }
        }
        auto image = std::unique_ptr<Image>(Factory::createImage(segment));
        image->setTrack(track);image->setAutoplace(false);image->setSizeIsSpatium(true);
        image->setSize(SizeF(sourceBounds.width()/sp,sourceBounds.height()/sp));
        image->setDrawingPart("rigid");image->setDrawingData(QJsonDocument(doc).toJson(QJsonDocument::Compact).toStdString());
        image->loadFromData("drawing.svg",muse::ByteArray::fromQByteArray(svg.toUtf8()));image->init();
        if (!image->isValid()) return QCoreApplication::translate("drawing-tools","Could not load the SVG.");
        pending.push_back(std::move(image));
    } else if (mode == "note" && !native) {
        auto notes = score->selection().noteList();
        Note* note = !notes.empty() ? notes.front()
                     : oldImage && oldImage->explicitParent() && oldImage->explicitParent()->isNote()
                       ? toNote(oldImage->explicitParent()) : nullptr;
        if (!note) {
            return QStringLiteral("Selecione a nota que receberá o desenho.");
        }
        const double sp = note->spatium();
        const double scale = onScore ? canvasBounds.width() / sourceBounds.width() / sp : 0.04;
        const QJsonArray shapes = source.value("shapes").toArray();
        QString body;
        for (const auto& v : shapes) {
            const auto sh = v.toObject(), st = sh.value("style").toObject();
            if (!sh.value("svgSource").toString().isEmpty()) {
                body += assetBody(assetSource(sh), sh.value("svgMatrix").toArray(), st.value("opacity").toDouble());
                continue;
            }
            QString path;
            for (const auto& cv : sh.value("commands").toArray()) {
                const auto c = cv.toArray();
                path += c.at(0).toString();
                for (int n = 1; n < c.size(); ++n) {
                    path += number(c.at(n).toDouble()) + " ";
                }
            }
            body += "<path d=\"" + path + "\" stroke=\"" + st.value("stroke").toString()
                    + "\" fill=\"" + st.value("fill").toString() + "\" stroke-width=\"" + number(st.value("width").toDouble())
                    + "\" opacity=\"" + number(st.value("opacity").toDouble())
                    + "\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>";
        }
        const QString svg = "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"" + number(sourceBounds.x()) + " "
                            + number(sourceBounds.y()) + " " + number(sourceBounds.width()) + " "
                            + number(sourceBounds.height()) + "\">" + body + "</svg>";
        auto image = std::unique_ptr<Image>(Factory::createImage(note));
        image->setTrack(note->track());
        image->setSizeIsSpatium(true);
        image->setSize(SizeF(sourceBounds.width() * scale, sourceBounds.height() * scale));
        image->setOffset(onScore ? canvasBounds.topLeft() - note->canvasPos() : PointF(0, -5 * sp));
        image->setAutoplace(false);
        image->setDrawingData(QJsonDocument(doc).toJson(QJsonDocument::Compact).toStdString());
        image->loadFromData("drawing.svg", muse::ByteArray::fromQByteArray(svg.toUtf8()));
        image->init();
        if (!image->isValid()) {
            return QStringLiteral("Não foi possível carregar o SVG.");
        }
        pending.push_back(std::move(image));
    } else if (mode == "rhythm" || (mode == "note" && native)) {
        track_idx_t nativeTrack = 0;
        auto parts = native ? nativeParts(score, source, sourceBounds, &nativeTrack) : selectionParts(score);
        if (parts.empty() || parts.size() > 512) {
            return QStringLiteral("Selecione um compasso ou intervalo de até 512 compassos.");
        }
        const auto& sel = score->selection();
        track_idx_t track = native ? nativeTrack : sel.isRange() ? sel.staffStart() * VOICES
                            : oldElement ? oldElement->track() : 0;
        if (track >= score->nstaves() * VOICES) {
            track = 0;
        }
        if (!parts.front().measure->system()) {
            return QStringLiteral("O layout da partitura ainda não está disponível.");
        }
        for (const Part& part : parts) {
            if (!part.measure || !part.measure->system() || !part.measure->system()->staff(track / VOICES)) {
                return QCoreApplication::translate("drawing-tools", "The score layout is not available yet.");
            }
        }
        const double sp = parts.front().measure->spatium();
        doc["staff"]=int(track/VOICES);
        doc["staffId"]=staffId(score,track/VOICES);
        const double total = parts.back().origin + parts.back().width;
        const double yScale = onScore ? canvasBounds.height() / sourceBounds.height() / sp : 0.04;
        const double baseline = parts.front().measure->system()->staffCanvasYpage(track / VOICES);
        const double yStart = onScore ? (canvasBounds.y() - baseline) / sp : -5;
        QJsonArray geometry, savedParts;
        for (const Part& p : parts) {
            savedParts.append(QJsonObject{{"key", p.key}, {"measureId",measureIdentity(p.measure)}, {"start", QString::fromStdString(p.start.toString().toStdString())},
                                          {"end", QString::fromStdString(p.end.toString().toStdString())}, {"fullMeasure", p.fullMeasure}});
        }
        const QJsonArray manualAnchors = source.value("anchors").toArray();
        std::map<std::tuple<int,int,int>,QJsonObject> anchorsByPoint;
        for(const auto& value:manualAnchors) {
            const auto a=value.toObject();
            anchorsByPoint.emplace(std::make_tuple(a.value("shape").toInt(-1),a.value("command").toInt(-1),a.value("field").toInt(-1)),a);
        }
        int shapeIndex = 0;
        for (const auto& v : source.value("shapes").toArray()) {
            auto shape = v.toObject();
            QJsonArray boundCommands;
            const auto commands = shape.value("commands").toArray();
            const auto rigidHeads=shape.value("rigidArrowHeads").toObject();
            for (int ci = 0; ci < commands.size(); ++ci) {
                const auto c = commands.at(ci).toArray();
                QJsonArray bound{c.at(0)};
                for (int n = 1; n < c.size(); n += 2) {
                    const double x = (c.at(n).toDouble() - sourceBounds.x()) / sourceBounds.width() * total;
                    const double y = yStart + (c.at(n + 1).toDouble() - sourceBounds.y()) * yScale;
                    auto point = native ? canvasBinding(parts, c.at(n).toDouble(), c.at(n + 1).toDouble(), track / VOICES) : pointBinding(parts, x, y);
                    const auto binding=anchorsByPoint.find(std::make_tuple(shapeIndex,ci,n));
                    if(binding!=anchorsByPoint.end()) {
                        const auto& a=binding->second;
                            const QString key = a.value("key").toString().section(':', -1);
                            const auto exists = std::find_if(parts.begin(), parts.end(), [&](const Part& p) {return sameMeasureKey(p.key,key);});
                            if (exists != parts.end() && a.value("denominator").toInt() > 0) {
                                Fraction t(a.value("numerator").toInt(), a.value("denominator").toInt());
                                t = std::clamp(t, exists->start, exists->end);
                                point = QJsonObject{{"key", exists->key}, {"measureId",measureIdentity(exists->measure)}, {"time", QString::fromStdString(t.toString().toStdString())},
                                                    {"staffId",a.value("staffId")},{"dx", a.value("offsetX").toDouble()}, {"y", a.value("offsetY").toDouble()}};
                            }
                    }
                    // A head follows its anchored tip with rigid local offsets.
                    // Explicit links from the attachment tool override this rule.
                    auto head=rigidHeads.value(QString::number(ci)).toObject();
                    const bool manualHead=binding!=anchorsByPoint.end()&&binding->second.value("headManual").toBool();
                    if(native&&n==1&&!head.isEmpty()&&!manualHead) {
                        auto coordinate=[&](int index,QPointF* value) {
                            if(index<0||index>=commands.size())return false;
                            const auto command=commands[index].toArray();if(command.size()<3)return false;
                            *value=QPointF(command[1].toDouble(),command[2].toDouble());return true;
                        };
                        QPointF tip,direction;
                        bool valid=coordinate(head.value("tip").toInt(-1),&tip)&&coordinate(head.value("direction").toInt(-1),&direction);
                        if(valid&&head.contains("direction2")) {
                            QPointF other;valid=coordinate(head.value("direction2").toInt(-1),&other);direction=(direction+other)/2;
                        }
                        const double dx=tip.x()-direction.x(),dy=tip.y()-direction.y(),length=std::hypot(dx,dy);
                        if(valid&&length>0.000001) {
                            const double ux=dx/length,uy=dy/length,px=c[n].toDouble()-tip.x(),py=c[n+1].toDouble()-tip.y();
                            head["along"]=(px*ux+py*uy)*yScale;head["across"]=(-px*uy+py*ux)*yScale;
                            point["arrowHead"]=head;
                        }
                    }
                    if(native&&shape.value("unlinkedPoints").toObject().value(QString::number(ci)+":"+QString::number(n)).toBool()) {
                        const auto key=point.value("key").toString();
                        const auto target=std::find_if(parts.begin(),parts.end(),[&](const Part& p){return sameMeasureKey(p.key,key);});
                        if(target!=parts.end()&&target->measure->system()->page()) {
                            const auto page=target->measure->system()->page();
                            point["free"]=QJsonObject{{"page",int(page->pageNumber())},{"x",c.at(n).toDouble()-page->canvasPos().x()},{"y",c.at(n+1).toDouble()-page->canvasPos().y()}};
                        }
                    }
                    bound.append(point);
                }
                boundCommands.append(bound);
            }
            auto style = shape.value("style").toObject();
            style.insert("width", style.value("width").toDouble() * yScale);
            const auto paintFrame=paintBounds(commands);
            auto bindPaint=[&](QJsonObject paint) {
                if(!native)return paint;
                for(const char* channel:{"strokeGradient","fillGradient"}) {
                    auto gradient=paint.value(channel).toObject();if(gradient.isEmpty())continue;
                    for(const char* side:{"from","to"}) {
                        const auto coordinate=gradient.value(side).toObject();
                        gradient[QString::fromLatin1(side)+"Binding"]=canvasBinding(parts,
                            paintFrame.x()+coordinate.value("x").toDouble()*paintFrame.width(),
                            paintFrame.y()+coordinate.value("y").toDouble()*paintFrame.height(),track/VOICES);
                    }
                    paint[channel]=gradient;
                }return paint;
            };
            style=bindPaint(style);
            QJsonObject boundShape{{"commands", boundCommands}, {"style", style}};
            if(shape.contains("fillRule"))boundShape["fillRule"]=shape.value("fillRule");
            if(shape.contains("paintRanges")) {
                auto ranges=shape.value("paintRanges").toArray();
                for(int ri=0;ri<ranges.size();++ri) {
                    auto range=ranges[ri].toObject(),rangeStyle=range.value("style").toObject();
                    rangeStyle["width"]=rangeStyle.value("width").toDouble()*yScale;range["style"]=bindPaint(rangeStyle);ranges[ri]=range;
                }
                boundShape["paintRanges"]=ranges;
            }
            if (!shape.value("svgSource").toString().isEmpty()) {
                const QJsonArray matrix = shape.value("svgMatrix").toArray();
                if (matrix.size() != 6) {
                    return QStringLiteral("Transformação SVG inválida.");
                }
                QJsonArray frame;
                for (int corner = 0; corner < 3; ++corner) {
                    const double px = matrix[4].toDouble() + (corner == 1 ? matrix[0].toDouble() : corner == 2 ? matrix[2].toDouble() : 0);
                    const double py = matrix[5].toDouble() + (corner == 1 ? matrix[1].toDouble() : corner == 2 ? matrix[3].toDouble() : 0);
                    frame.append(native ? canvasBinding(parts, px, py, track / VOICES) : pointBinding(parts, (px - sourceBounds.x()) / sourceBounds.width() * total, yStart + (py - sourceBounds.y()) * yScale));
                }
                boundShape.insert("svgSource", shape.value("svgSource"));
                boundShape.insert("svgFrame", frame);
            }
            geometry.append(boundShape);
            ++shapeIndex;
        }
        doc.insert("parts", savedParts);
        doc.insert("geometry", geometry);
        const std::string text = QJsonDocument(doc).toJson(QJsonDocument::Compact).toStdString();
        // Pending fragments are not in the score yet. Share their mapping
        // explicitly instead of discovering the same group for every fragment.
        sharedGeometry=std::make_shared<GeometryCache>();sharedGeometry->source=geometry;
        for(const auto& value:geometry)for(const auto& command:value.toObject().value("commands").toArray()) {
            const auto c=command.toArray();
            for(int n=1;n<c.size();++n)if(!c[n].toObject().value("free").toObject().isEmpty())sharedGeometry->hasFreePoints=true;
        }
        if(sharedGeometry->hasFreePoints)sharedGeometry.reset();
        reusedData=text;
        for (const Part& p : parts) {
            Segment* segment = p.measure->first(SegmentType::ChordRest);
            if (!segment) {
                return QStringLiteral("Há um compasso sem segmento musical disponível.");
            }
            const auto existing=std::find_if(reusableMembers.begin(),reusableMembers.end(),[&](Image* image){
                return image->track()==track&&image->explicitParent()==segment;
            });
            if(existing!=reusableMembers.end()) {
                reused.emplace_back(*existing,p.key.toStdString());continue;
            }
            auto image = std::unique_ptr<Image>(Factory::createImage(segment));
            image->setTrack(track);
            image->setAutoplace(false);
            image->setSizeIsSpatium(true);
            image->setDrawingData(text);
            image->setDrawingPart(p.key.toStdString());
            if(sharedGeometry) {
                metadata(image.get());image->drawingCache()->geometry=sharedGeometry;image->drawingCache()->geometryShared=true;
            }
            QRectF box;
            const QString svg = cachedPaintSvg(image.get(), doc, parts, &box);
            image->setSize(SizeF(box.width(), box.height()));
            image->loadFromData("drawing.svg", muse::ByteArray::fromQByteArray(svg.toUtf8()));
            image->init();
            if (!image->isValid()) {
                return QStringLiteral("Não foi possível gerar um trecho SVG.");
            }
            pending.push_back(std::move(image));
        }
    } else {
        return QStringLiteral("Tipo de vínculo desconhecido.");
    }
    // A constructor parent only supplies ownership/context. The factory also
    // registers explicitParent(), which undoAddElement and SVG layout require.
    // Validate before touching the old images or transferring undo ownership.
    for (const auto& image : pending) {
        if (!image->explicitParent() || image->score() != score) {
            return QCoreApplication::translate("drawing-tools", "The score layout is not available yet.");
        }
    }
    LOGI() << "[MuseDraw native] explicit parent validated; responsive-render-v1; musical-fragment-update-v2; parts-flow-checkup-v1";
    auto oldMembers = groupImages(score, native ? group : old.value("group").toString());
    if (!native && oldImage && oldMembers.empty() && !old.value("group").toString().isEmpty()) {
        oldMembers.push_back(const_cast<Image*>(oldImage));
    }
    if (ownTransaction) score->startCmd(muse::TranslatableString("drawing-tools", "Insert vector drawing"));
    for (Image* i : oldMembers) {
        if(std::none_of(reused.begin(),reused.end(),[&](const auto& entry){return entry.first==i;}))score->undoRemoveElement(i);
    }
    for(const auto& entry:reused) {
        score->undo(new ChangeDrawingMetadata(entry.first,reusedData));
        if(entry.first->drawingPart()!=entry.second)score->undo(new ChangeDrawingPart(entry.first,entry.second));
        if(sharedGeometry) {
            metadata(entry.first);entry.first->drawingCache()->geometry=sharedGeometry;entry.first->drawingCache()->geometryShared=true;
        }
    }
    for (auto& i : pending) {
        score->undoAddElement(i.release());
    }
    if (ownTransaction) score->endCmd();
    return QStringLiteral("Desenho inserido. Salve a partitura para guardar o SVG e seus vínculos.");
}
void setEditing(Score* score, bool active) {
    for (Image* i : nativeImages(score)) {
        i->setHiddenByDrawingEditor(active);
        i->setSelectable(false);
    }
}

static QString nativeDocumentFiltered(Score* score, const QSet<QString>* drawingIds) {
    QJsonObject result{{"format","musescore-drawing"},{"version",1},{"coordinateSpace","score"},{"anchorMode","rhythm"}};
    QJsonArray shapes, links;
    auto images = nativeImages(score);
    std::stable_sort(images.begin(), images.end(), [](Image* a, Image* b) {
        return metadata(a).value("nativeIndex").toInt() < metadata(b).value("nativeIndex").toInt();
    });
    QSet<QString> seen;
    for (Image* image : images) {
        auto doc = metadata(image);
        const QString group = doc.value("group").toString();
        if (seen.contains(group)) continue;
        seen.insert(group);
        auto source = doc.value("source").toObject();
        auto array = source.value("shapes").toArray();
        if (array.isEmpty()) continue;
        auto shape = array[0].toObject();
        if(drawingIds&&!drawingIds->contains(shape.value("drawId").toString()))continue;
        doc=documentWithCurrentKeys(image);source=doc.value("source").toObject();array=source.value("shapes").toArray();shape=array[0].toObject();
        auto commands = shape.value("commands").toArray();
        const int index = shapes.size();
        const bool whole=uniformPartCopy(image,doc);
        // A score group has one whole copy per part. Pick its stable first
        // fragment even when another fragment is encountered first in staff order.
        if(whole) {
            const auto keys=doc.value("parts").toArray();
            const auto first=keys.isEmpty()?QString():keys.first().toObject().value("key").toString();
            for(auto candidate:images)if(serializedPart(candidate)==first.toStdString()
                &&metadata(candidate).value("group")==group){image=candidate;break;}
        }
        const auto placement=whole?wholePartPlacement(image,metadata(image)):WholePartPlacement();
        const auto points = editingPoints(score, image);
        const auto originalLinks = source.value("anchors").toArray();
        std::map<std::pair<int,int>,QJsonObject> linksByPoint;
        for (const auto& value : originalLinks) {
            const auto link=value.toObject();linksByPoint.emplace(std::make_pair(link.value("command").toInt(),link.value("field").toInt()),link);
        }
        for (const auto& value : points) {
            auto point = value.toMap();
            const int ci = point.value("command").toInt(), field = point.value("field").toInt();
            if (ci < 0 || ci >= commands.size()) continue;
            auto command = commands[ci].toArray();
            if (field < 1 || field + 1 >= command.size()) continue;
            command[field] = point.value("x").toDouble();command[field+1] = point.value("y").toDouble();commands[ci] = command;
            if (point.value("key").toString().isEmpty()) {
                // Rigid groups and text expose only their one saved centre
                // attachment; frame corners are transformation controls.
                const auto saved=linksByPoint.find({ci,field});
                if(saved!=linksByPoint.end()&&!whole) {
                    auto restored=saved->second;
                    restored["shape"]=index;restored["x"]=point.value("x").toDouble();restored["y"]=point.value("y").toDouble();
                    links.append(localLink(score,restored,image->staffIdx()));
                }
                continue;
            }
            bool editable = false;
            const auto saved = linksByPoint.find({ci,field});
            if (saved != linksByPoint.end()) {
                    const auto& link = saved->second;
                    editable = link.value("editable").toBool();
                    if(link.value("headManual").toBool())point["headManual"]=true;
                    if(link.contains("markerOffsetX"))point["markerOffsetX"]=link.value("markerOffsetX").toDouble();
            }
            point["shape"] = index;point["scale"] = point.value("spatium");point["editable"] = editable;
            point["key"] = QStringLiteral("%1:%2").arg(point.value("staff",int(image->staffIdx())).toInt()).arg(point.value("key").toString());
            links.append(QJsonObject::fromVariantMap(point));
        }
        if(!whole&&shape.value("svgSource").toString().isEmpty()) {
            const auto geometry=doc.value("geometry").toArray();
            const auto bound=geometry.isEmpty()?QJsonObject():geometry[0].toObject();
            const auto frame=paintBounds(commands);
            auto restorePaint=[&](QJsonObject style,const QJsonObject& boundStyle) {
                for(const char* channel:{"strokeGradient","fillGradient"}) {
                    auto gradient=style.value(channel).toObject();if(gradient.isEmpty())continue;
                    const auto saved=boundStyle.value(channel).toObject();
                    for(const char* side:{"from","to"}) {
                        const auto reference=saved.value(QString::fromLatin1(side)+"Binding").toObject();if(reference.isEmpty())continue;
                        const auto measure=referencedMeasure(score,reference);const auto staff=localStaff(score,reference,image->staffIdx());
                        if(!measure||!measure->system()||!measure->system()->staff(staff))continue;
                        const double sp=measure->spatium();
                        const double x=measure->canvasPos().x()+rhythmicX(measure,fraction(reference,"time"))+reference.value("dx").toDouble()*sp;
                        const double y=measure->system()->staffCanvasYpage(staff)+reference.value("y").toDouble()*sp;
                        gradient[side]=QJsonObject{{"x",(x-frame.x())/frame.width()},{"y",(y-frame.y())/frame.height()}};
                    }
                    style[channel]=gradient;
                }return style;
            };
            shape["style"]=restorePaint(shape.value("style").toObject(),bound.value("style").toObject());
            auto ranges=shape.value("paintRanges").toArray();const auto boundRanges=bound.value("paintRanges").toArray();
            for(int ri=0;ri<ranges.size()&&ri<boundRanges.size();++ri){auto range=ranges[ri].toObject();range["style"]=restorePaint(range.value("style").toObject(),boundRanges[ri].toObject().value("style").toObject());ranges[ri]=range;}
            if(!ranges.isEmpty())shape["paintRanges"]=ranges;
        }
        if(whole&&!placement.box.isEmpty()) {
            shape["singleAnchor"]=true;shape["anchorMode"]="note";
            auto style=shape.value("style").toObject();style["width"]=style.value("width").toDouble()*placement.scale;shape["style"]=style;
            const int ci=placement.anchor.value("command").toInt(0),field=placement.anchor.value("field").toInt(1);
            if(ci>=0&&ci<commands.size()&&field>=1&&field+1<commands[ci].toArray().size()) {
                const auto c=commands[ci].toArray();
                const double sp=placement.measure->spatium();
                const double anchorX=placement.measure->canvasPos().x()+rhythmicX(placement.measure,Fraction(0,1));
                const double anchorY=placement.measure->system()->staffCanvasYpage(image->staffIdx());
                links.append(QJsonObject{{"shape",index},{"command",ci},{"field",field},{"editable",true},
                    {"key",QStringLiteral("%1:%2").arg(image->staffIdx()).arg(QString::fromStdString(placement.measure->tick().toString().toStdString()))},
                    {"staffId",staffId(score,image->staffIdx())},{"numerator",0},{"denominator",1},{"scale",sp},
                    {"offsetX",(c[field].toDouble()-anchorX)/sp},{"offsetY",(c[field+1].toDouble()-anchorY)/sp}});
            }
        }
        shape["commands"] = commands;
        shape=localShapeLinks(score,shape,image->staffIdx());
        // A stored asset frame consists of three musical points defining its affine transform.
        if (whole&&!shape.value("svgSource").toString().isEmpty()&&!placement.box.isEmpty()) {
            auto matrix=shape.value("svgMatrix").toArray();
            if(matrix.size()==6) {
                for(int i=0;i<4;++i)matrix[i]=matrix[i].toDouble()*placement.scale;
                matrix[4]=placement.box.x()+(matrix[4].toDouble()-placement.sourceBox.x())*placement.scale;
                matrix[5]=placement.box.y()+(matrix[5].toDouble()-placement.sourceBox.y())*placement.scale;
                shape["svgMatrix"]=matrix;
            }
        } else if (!shape.value("svgSource").toString().isEmpty()) {
            const auto parts = doc.value("rigid").toBool() ? std::vector<Part>() : selectionParts(score, image);
            const auto geometry = doc.value("geometry").toArray();
            if (!parts.empty() && !geometry.isEmpty()) {
                const auto frame = geometry[0].toObject().value("svgFrame").toArray();
                std::vector<QPointF> mapped;
                for (const auto& v : frame) {
                    const auto point = v.toObject();
                    const auto member=referencedMeasure(score,point);
                    const auto target = std::find_if(parts.begin(),parts.end(),[&](const Part& part){return part.measure==member;});
                    if (target != parts.end() && target->measure->system()) {
                        const double sp = target->measure->spatium();
                        mapped.emplace_back(target->measure->canvasPos().x()+rhythmicX(target->measure,fraction(point,"time"))+point.value("dx").toDouble()*sp,
                                            target->measure->system()->staffCanvasYpage(localStaff(score,point,image->staffIdx()))+point.value("y").toDouble()*sp);
                    }
                }
                if (mapped.size() == 3) shape["svgMatrix"] = QJsonArray{mapped[1].x()-mapped[0].x(),mapped[1].y()-mapped[0].y(),mapped[2].x()-mapped[0].x(),mapped[2].y()-mapped[0].y(),mapped[0].x(),mapped[0].y()};
            }
        }
        if (doc.value("rigid").toBool() && !shape.value("svgSource").toString().isEmpty()) {
            auto matrix = shape.value("svgMatrix").toArray();
            const auto savedBox = source.value("bounds").toObject();
            const auto attachedBox = anchoredRigidBox(score, source,image->staffIdx());
            const auto box = attachedBox.isEmpty() ? image->canvasBoundingRect()
                : RectF(attachedBox.x(), attachedBox.y(), attachedBox.width(), attachedBox.height());
            const double sx = box.width()/savedBox.value("width").toDouble(1), sy = box.height()/savedBox.value("height").toDouble(1);
            if (matrix.size()==6) {
                matrix[0]=matrix[0].toDouble()*sx;matrix[2]=matrix[2].toDouble()*sx;
                matrix[1]=matrix[1].toDouble()*sy;matrix[3]=matrix[3].toDouble()*sy;
                matrix[4]=box.x()+(matrix[4].toDouble()-savedBox.value("x").toDouble())*sx;
                matrix[5]=box.y()+(matrix[5].toDouble()-savedBox.value("y").toDouble())*sy;shape["svgMatrix"]=matrix;
            }
        }
        if (shape.value("kind").toString() == "text" && shape.value("svgSource").toString().contains("<tspan")) {
            shape["svgSource"] = assetSource(shape);
        }
        shapes.append(shape);
        for (const char* setting : {"subdivision","systemContinuation","systemStartContinuation","pointerMode","strokeWidth","strokeColor","secondaryColor","secondaryActive","smoothing","pressureEnabled","strokeOpacity","filled","pointCurvature","dragResistance","anchorMode","anchorGridMode"}) {
            if (source.contains(setting)) result[setting] = source.value(setting);
        }
    }
    result["shapes"] = shapes;result["anchors"] = links;
    return QString::fromUtf8(QJsonDocument(result).toJson(QJsonDocument::Compact));
}

QString nativeDocument(Score* score) {return nativeDocumentFiltered(score,nullptr);}

QVariantList nativeFragments(Score* score, const QString& previewDocument) {
    QVariantList result;
    const auto preview = QJsonDocument::fromJson(previewDocument.toUtf8()).object();
    std::map<QString, QJsonValue> previewShapes;
    for (const auto& candidate : preview.value("shapes").toArray()) {
        previewShapes.emplace(candidate.toObject().value("drawId").toString(), candidate);
    }
    std::map<QString, std::vector<Part>> groupParts;
    for (Image* image : nativeImages(score)) {
        auto doc = metadata(image);
        bool changedPreview=false;
        if (!preview.isEmpty()) {
            auto source = doc.value("source").toObject();
            const auto original = source.value("shapes").toArray();
            if (!original.isEmpty()) {
                const auto found = previewShapes.find(original[0].toObject().value("drawId").toString());
                if (found != previewShapes.end()) {
                    auto shape=found->second.toObject();
                    if(shape.contains("fragmentOverrides"))shape=withCurrentOverrides(score,shape,shape.value("fragmentOverrides").toObject());
                    source["shapes"] = QJsonArray{shape};
                }
                if(preview.value("nativeOperation")=="fragment"&&original[0].toObject().value("drawId")==preview.value("drawId")) {
                    source["shapes"]=QJsonArray{withCurrentOverrides(score,original[0].toObject(),preview.value("fragmentOverrides").toObject())};
                }
            }
            for(const char* key:{"systemContinuation","systemStartContinuation"})if(preview.contains(key))source[key]=preview.value(key);
            changedPreview=source!=doc.value("source").toObject();
            doc["source"] = source;
        }
        const auto shapes = doc.value("source").toObject().value("shapes").toArray();
        if (shapes.isEmpty()) continue;
        QString svg;
        RectF box;
        QJsonArray mapped;
        QJsonObject info;
        if(uniformPartCopy(image,doc)) {
            const auto placement=wholePartPlacement(image,doc);
            if(placement.box.isEmpty())continue;
            svg=cachedRigidSvg(image);
            box=RectF(placement.box.x(),placement.box.y(),placement.box.width(),placement.box.height());
        } else if (doc.value("rigid").toBool()) {
            svg=cachedRigidSvg(image);
            const auto attachedBox = anchoredRigidBox(score, doc.value("source").toObject(),image->staffIdx());
            box = attachedBox.isEmpty() ? image->canvasBoundingRect()
                : RectF(attachedBox.x(), attachedBox.y(), attachedBox.width(), attachedBox.height());
        } else {
            const auto group = doc.value("group").toString();
            auto foundParts = groupParts.find(group);
            if (foundParts == groupParts.end()) foundParts = groupParts.emplace(group, selectionParts(score,image)).first;
            const auto& parts = foundParts->second;
            if (parts.empty()) continue;
            QRectF clip;
            svg = !changedPreview ? cachedPaintSvg(image,doc,parts,&clip,&mapped,&info)
                : paintSvg(doc,parts,QString::fromStdString(image->drawingPart()),&clip,&mapped,&info,&resolvedGeometryFor(image,doc,parts));
            const auto target = std::find_if(parts.begin(),parts.end(),[&](const Part& p){return p.key.toStdString()==image->drawingPart();});
            if (target == parts.end() || !target->measure->system()) continue;
            const double sp=image->spatium();
            box=RectF(target->measure->canvasPos().x()+target->left+(clip.x()-target->origin)*sp,
                      target->measure->system()->staffCanvasYpage(image->staffIdx())+clip.y()*sp,clip.width()*sp,clip.height()*sp);
            const double offsetX=target->measure->canvasPos().x()+target->left-target->origin*sp;
            const double offsetY=target->measure->system()->staffCanvasYpage(image->staffIdx());
            info["unitOriginX"]=offsetX;
            info["unitOriginY"]=offsetY;
            for (int i=0;i<mapped.size();++i) {
                auto command=mapped[i].toArray();
                for (int n=1;n+1<command.size();n+=2) {command[n]=command[n].toDouble()*sp+offsetX;command[n+1]=command[n+1].toDouble()*sp+offsetY;}
                mapped[i]=command;
            }
            info["cutLeft"]=info.value("cutLeft").toDouble()*sp+offsetX;
            info["cutRight"]=info.value("cutRight").toDouble()*sp+offsetX;
        }
        if (!svg.isEmpty()) {
            auto fragment=info.toVariantMap();
            fragment.insert(QVariantMap{{"id",shapes[0].toObject().value("drawId").toString()},
                {"x",box.x()},{"y",box.y()},{"width",box.width()},{"height",box.height()},{"svg",svg},
                {"commands",mapped.toVariantList()}});
            result.append(fragment);
        }
    }
    return result;
}

QString syncNative(Score* score, const QString& document) {
    const QJsonObject source = QJsonDocument::fromJson(document.toUtf8()).object();
    if (source.value("nativeOperation").toString() == "continuation" || source.value("nativeOperation").toString() == "fragment") {
        const bool fragment=source.value("nativeOperation")=="fragment";
        if (!score || source.value("format")!="musescore-drawing" || source.value("version").toInt()!=1) return QStringLiteral("Invalid drawing update.");
        if(fragment&&(source.value("drawId").toString().isEmpty()||!source.value("fragmentOverrides").isObject()))return QStringLiteral("Invalid drawing fragment.");
        const auto incomingOverrides=fragment?canonicalOverrides(score,source.value("fragmentOverrides").toObject()):QJsonObject();
        if(!fragment)for (const char* key : {"systemContinuation","systemStartContinuation"}) {
            if (!source.value(key).isDouble() || !std::isfinite(source.value(key).toDouble())
                || source.value(key).toDouble()<0 || source.value(key).toDouble()>1) return QStringLiteral("Invalid continuation.");
        }
        bool started=false;
        for (Image* image : nativeImages(score)) {
            auto doc=metadata(image),saved=doc.value("source").toObject();
            if(fragment) {
                auto shapes=saved.value("shapes").toArray();
                if(shapes.isEmpty()||shapes[0].toObject().value("drawId")!=source.value("drawId"))continue;
                auto shape=shapes[0].toObject();
                if(currentOverrides(score,shape)==incomingOverrides)continue;
                shape=withCurrentOverrides(score,shape,incomingOverrides);shapes[0]=shape;saved["shapes"]=shapes;
            }else{
                if (saved.value("systemContinuation")==source.value("systemContinuation")
                    && saved.value("systemStartContinuation")==source.value("systemStartContinuation")) continue;
                saved["systemContinuation"]=source.value("systemContinuation");
                saved["systemStartContinuation"]=source.value("systemStartContinuation");
            }
            if(!started){score->startCmd(muse::TranslatableString("drawing-tools","Edit vector drawing"));started=true;}
            doc["source"]=saved;
            changeLinkedMetadata(score,image,doc);
        }
        if(started)score->endCmd();
        return {};
    }
    const auto shapes = source.value("shapes").toArray();
    if (!score || document.size() > 16 * 1024 * 1024 || source.value("format") != "musescore-drawing"
        || (!shapes.isEmpty() && !validSource(source))) {
        return QCoreApplication::translate("drawing-tools","The drawing contains invalid data.");
    }
    const auto existing = nativeImages(score);
    const auto current = QJsonDocument::fromJson(nativeDocument(score).toUtf8()).object();
    auto normalizedLinks = [](const QJsonArray& array, int shapeIndex) {
        QJsonArray result;
        for (const auto& value : array) {
            const auto link = value.toObject();
            if (link.value("shape").toInt() != shapeIndex) continue;
            QJsonObject normalized;
            for (const char* key : {"command","field","key","staffId","numerator","denominator","offsetX","offsetY","editable","markerOffsetX","headManual"}) {
                if (link.contains(key)) normalized[key] = link.value(key);
            }
            result.append(normalized);
        }
        return result;
    };
    QSet<QString> wanted;
    bool modified = false;
    for (const auto& v : shapes) {
        const auto sh = v.toObject();
        const QString id = sh.value("drawId").toString();
        if (id.isEmpty() || wanted.contains(id)) return QCoreApplication::translate("drawing-tools","The drawing contains invalid data.");
        wanted.insert(id);
    }
    if (current.value("shapes") == source.value("shapes") && current.value("systemContinuation") == source.value("systemContinuation")
        && current.value("systemStartContinuation") == source.value("systemStartContinuation") && current.value("anchors") == source.value("anchors")
        && current.value("anchorGridMode")==source.value("anchorGridMode")&&current.value("subdivision")==source.value("subdivision")) return {};
    score->startCmd(muse::TranslatableString("drawing-tools","Edit vector drawing"));
    for (Image* i : existing) {
        const auto d = metadata(i);
        const auto savedShapes = d.value("source").toObject().value("shapes").toArray();
        if (!savedShapes.isEmpty() && !wanted.contains(savedShapes[0].toObject().value("drawId").toString())) {score->undoRemoveElement(i);modified=true;}
    }
    for (int index = 0; index < shapes.size(); ++index) {
        const auto shape = shapes[index].toObject();
        bool unchanged = false;
        bool metadataOnly = false;
        const auto currentShapes = current.value("shapes").toArray();
        for (int previous = 0; previous < currentShapes.size(); ++previous) {
            const auto sh = currentShapes[previous].toObject();
            if (sh.value("drawId") != shape.value("drawId")) continue;
            auto beforeShape=sh,afterShape=shape;
            beforeShape.remove("fragmentOverrides");afterShape.remove("fragmentOverrides");
            beforeShape.remove("selectionGroup");afterShape.remove("selectionGroup");
            metadataOnly=beforeShape==afterShape
                && normalizedLinks(current.value("anchors").toArray(),previous)==normalizedLinks(source.value("anchors").toArray(),index);
            unchanged = sh == shape
                        && normalizedLinks(current.value("anchors").toArray(),previous) == normalizedLinks(source.value("anchors").toArray(),index)
                        && current.value("systemContinuation") == source.value("systemContinuation")
                        && current.value("systemStartContinuation") == source.value("systemStartContinuation")
                        && current.value("anchorGridMode")==source.value("anchorGridMode")&&current.value("subdivision")==source.value("subdivision");
            break;
        }
        if (unchanged) continue;
        if (metadataOnly) {
            for (Image* image : existing) {
                auto doc=metadata(image),saved=doc.value("source").toObject();
                auto savedShapes=saved.value("shapes").toArray();
                if(savedShapes.isEmpty()||savedShapes[0].toObject().value("drawId")!=shape.value("drawId"))continue;
                auto savedShape=savedShapes[0].toObject();
                if(shape.contains("selectionGroup"))savedShape["selectionGroup"]=shape.value("selectionGroup");else savedShape.remove("selectionGroup");
                savedShape=withCurrentOverrides(score,savedShape,shape.value("fragmentOverrides").toObject());
                savedShapes[0]=savedShape;saved["shapes"]=savedShapes;
                for(const char* key:{"subdivision","systemContinuation","systemStartContinuation","pointerMode","strokeWidth","strokeColor","secondaryColor","secondaryActive","smoothing","pressureEnabled","strokeOpacity","filled","pointCurvature","dragResistance","anchorMode","anchorGridMode"}) {
                    if(source.contains(key))saved[key]=source.value(key);
                }
                doc["source"]=saved;doc["nativeIndex"]=index;
                if(doc==metadata(image))continue;
                changeLinkedMetadata(score,image,doc);modified=true;
            }
            continue;
        }
        QJsonObject single = source;
        single["nativeOwner"] = "MuseDrawNative";single["nativeIndex"] = index;single["shapes"] = QJsonArray{shape};
        QJsonArray bindings;
        for (const auto& v : source.value("anchors").toArray()) {
            auto b = v.toObject();
            if (b.value("shape").toInt() == index) {b["shape"] = 0;bindings.append(b);}
        }
        single["anchors"] = bindings;
        double left=1e20,top=1e20,right=-1e20,bottom=-1e20;
        for (const auto& v : shape.value("commands").toArray()) {
            const auto c = v.toArray();
            for (int n=1;n+1<c.size();n+=2) {left=std::min(left,c[n].toDouble());right=std::max(right,c[n].toDouble());top=std::min(top,c[n+1].toDouble());bottom=std::max(bottom,c[n+1].toDouble());}
        }
        const double pad=shape.value("style").toObject().value("width").toDouble()/2+0.01;
        QRectF box(left-pad,top-pad,std::max(0.01,right-left+2*pad),std::max(0.01,bottom-top+2*pad));
        single["bounds"] = QJsonObject{{"x",box.x()},{"y",box.y()},{"width",box.width()},{"height",box.height()}};
        const QString error = insert(score,QString::fromUtf8(QJsonDocument(single).toJson(QJsonDocument::Compact)),
                                     shape.value("anchorMode").toString("rhythm"),box,RectF(box.x(),box.y(),box.width(),box.height()),true,false);
        if (!error.startsWith("Desenho inserido")) {score->endCmd(true);return error;}
        modified = true;
    }
    score->endCmd(!modified);
    return {};
}

// Inserting time changes absolute measure keys. Keep that remap and the
// crossing drawing's reanchoring in the existing insertion undo transaction.
class InsertionAnchorIndex {
    struct Marker {double x;QVariantMap data;};
    std::vector<std::pair<int,QRectF>> pages;
    std::map<std::pair<int,staff_idx_t>,std::map<double,std::vector<Marker>>> rows;
public:
    explicit InsertionAnchorIndex(Score* score) {
        for(const auto page:score->pages()) {
            const auto box=page->canvasBoundingRect();
            pages.emplace_back(int(page->pageNumber()),QRectF(box.x(),box.y(),box.width(),box.height()));
        }
    }
    int pageAt(const QPointF& point) const {
        double best=std::numeric_limits<double>::max();int result=-1;
        for(const auto& page:pages) {
            const auto& box=page.second;
            const double dx=std::max({box.left()-point.x(),0.0,point.x()-box.right()});
            const double dy=std::max({box.top()-point.y(),0.0,point.y()-box.bottom()});
            const double distance=dx*dx+dy*dy;
            if(distance<best){best=distance;result=page.first;if(distance==0)break;}
        }
        return result;
    }
    void populate(Score* score,int subdivision,const std::set<std::pair<int,staff_idx_t>>& targets) {
        for(const auto& value:anchorsFiltered(score,subdivision,&targets)) {
            auto marker=value.toMap();
            rows[{marker.value("page").toInt(),marker.value("staff").toUInt()}][marker.value("y").toDouble()]
                .push_back({marker.value("x").toDouble(),std::move(marker)});
        }
        for(auto& page:rows)for(auto& row:page.second)std::stable_sort(row.second.begin(),row.second.end(),[](const auto& a,const auto& b){return a.x<b.x;});
    }
    QVariantMap nearest(const QPointF& point,staff_idx_t staff) const {
        const auto page=rows.find({pageAt(point),staff});if(page==rows.end())return {};
        const Marker* target=nullptr;double best=std::numeric_limits<double>::max();
        for(const auto& row:page->second) {
            const double dy=row.first-point.y(),vertical=dy*dy;if(vertical>best)continue;
            const auto& markers=row.second;
            const auto next=std::lower_bound(markers.begin(),markers.end(),point.x(),[](const Marker& marker,double x){return marker.x<x;});
            auto consider=[&](const Marker& marker) {
                const double dx=marker.x-point.x(),distance=dx*dx+vertical;
                if(distance<best){best=distance;target=&marker;}
            };
            if(next!=markers.end())consider(*next);
            if(next!=markers.begin())consider(*std::prev(next));
        }
        return target?target->data:QVariantMap();
    }
};

class MeasureInsertionState {
public:
    struct Crossing {
        QString id;
        int nativeIndex=0;
        Measure* first=nullptr;
        staff_idx_t staff=0;
        QPointF origin;
    };
    struct Saved {
        Score* score=nullptr;
        QJsonObject document;
        std::vector<std::pair<Image*,QJsonObject>> images;
        std::vector<Crossing> crossing;
    };
    Fraction tick;
    Fraction length;
    qint64 captureMilliseconds=0;
    std::vector<Saved> scores;
    ~MeasureInsertionState() {
        QElapsedTimer adaptation;adaptation.start();qint64 layoutMilliseconds=0;
        // Legacy drawings acquire identities once. Following drawings that
        // already use EIDs need no metadata/fragment rewrite on insertion.
        for(auto& saved:scores) {
            QHash<QString,std::string> groups;
            for(const auto& entry:saved.images) {
                if(metadata(entry.first).value("measureIdentityVersion").toInt()>=1)continue;
                auto image=entry.first;const auto group=entry.second.value("group").toString();
                auto cached=groups.find(group);
                if(cached==groups.end())cached=groups.insert(group,QJsonDocument(entry.second).toJson(QJsonDocument::Compact).toStdString());
                saved.score->undo(new ChangeDrawingMetadata(image,*cached));
            }
        }
        for(auto& saved:scores) {
            if(saved.crossing.empty()){suspendedDrawingLayout.remove(saved.score);continue;}
            // Resolve the new score grid while the affected old drawing
            // fragments are frozen; no partially rebuilt system is queried.
            QElapsedTimer layoutTimer;layoutTimer.start();
            saved.score->doLayout();
            layoutMilliseconds+=layoutTimer.elapsed();
            suspendedDrawingLayout.remove(saved.score);
            const int subdivision=saved.document.value("anchorGridMode")=="note"?0:saved.document.value("subdivision").toInt(8);
            const auto shapes=saved.document.value("shapes").toArray();
            const auto originalLinks=saved.document.value("anchors").toArray();
            InsertionAnchorIndex grid(saved.score);
            std::map<QString,std::pair<QPointF,staff_idx_t>> displacements;
            for(const auto& crossing:saved.crossing) {
                if(!crossing.first||!crossing.first->system()||!crossing.first->system()->staff(crossing.staff))continue;
                const QPointF origin(crossing.first->canvasPos().x(),crossing.first->system()->staffCanvasYpage(crossing.staff));
                displacements.emplace(crossing.id,std::make_pair(origin-crossing.origin,crossing.staff));
            }
            std::set<std::pair<int,staff_idx_t>> targets;
            for(const auto& value:originalLinks) {
                const auto link=value.toObject();const int index=link.value("shape").toInt(-1);
                if(index<0||index>=shapes.size())continue;
                const auto shape=shapes[index].toObject();const auto displacement=displacements.find(shape.value("drawId").toString());
                if(displacement==displacements.end())continue;
                const auto commands=shape.value("commands").toArray();
                const int ci=link.value("command").toInt(-1),field=link.value("field").toInt(-1);
                if(ci<0||ci>=commands.size())continue;
                const auto command=commands[ci].toArray();if(field<1||field+1>=command.size())continue;
                const QPointF point=QPointF(command[field].toDouble(),command[field+1].toDouble())+displacement->second.first;
                targets.emplace(grid.pageAt(point),localStaff(saved.score,link,displacement->second.second));
            }
            grid.populate(saved.score,subdivision,targets);
            for(const auto& crossing:saved.crossing) {
                int index=-1;
                for(int i=0;i<shapes.size();++i)if(shapes[i].toObject().value("drawId")==crossing.id){index=i;break;}
                if(index<0||!crossing.first||!crossing.first->system()||!crossing.first->system()->staff(crossing.staff))continue;
                const QPointF origin(crossing.first->canvasPos().x(),crossing.first->system()->staffCanvasYpage(crossing.staff));
                const auto delta=origin-crossing.origin;
                auto shape=shapes[index].toObject();auto commands=shape.value("commands").toArray();
                double left=1e20,top=1e20,right=-1e20,bottom=-1e20;
                for(int ci=0;ci<commands.size();++ci) {
                    auto command=commands[ci].toArray();
                    for(int field=1;field+1<command.size();field+=2) {
                        const double x=command[field].toDouble()+delta.x(),y=command[field+1].toDouble()+delta.y();
                        command[field]=x;command[field+1]=y;left=std::min(left,x);right=std::max(right,x);top=std::min(top,y);bottom=std::max(bottom,y);
                    }
                    commands[ci]=command;
                }
                shape["commands"]=commands;
                if(shape.contains("svgMatrix")) {
                    auto matrix=shape.value("svgMatrix").toArray();
                    if(matrix.size()==6){matrix[4]=matrix[4].toDouble()+delta.x();matrix[5]=matrix[5].toDouble()+delta.y();shape["svgMatrix"]=matrix;}
                }
                shape=withCurrentOverrides(saved.score,shape,{});
                QJsonArray links;
                for(const auto& value:originalLinks) {
                    auto link=value.toObject();if(link.value("shape").toInt()!=index)continue;
                    const int ci=link.value("command").toInt(-1),field=link.value("field").toInt(-1);
                    if(ci<0||ci>=commands.size())continue;
                    const auto command=commands[ci].toArray();if(field<1||field+1>=command.size())continue;
                    const QPointF point(command[field].toDouble(),command[field+1].toDouble());
                    const auto staff=localStaff(saved.score,link,crossing.staff);
                    const auto target=grid.nearest(point,staff);
                    if(target.isEmpty())continue;
                    const double scale=target.value("spatium").toDouble();if(scale<=0)continue;
                    const double x=target.value("x").toDouble(),bindingX=target.value("bindingX",x).toDouble();
                    link["shape"]=0;link["key"]=target.value("key").toString();link["staffId"]=staffId(saved.score,staff);
                    link["numerator"]=target.value("numerator").toInt();link["denominator"]=target.value("denominator").toInt();
                    link["scale"]=scale;link["offsetX"]=(point.x()-bindingX)/scale;link["offsetY"]=(point.y()-target.value("y").toDouble())/scale;
                    link["markerOffsetX"]=x-bindingX;links.append(link);
                }
                const double pad=shape.value("style").toObject().value("width").toDouble()/2+0.01;
                const QRectF box(left-pad,top-pad,std::max(0.01,right-left+2*pad),std::max(0.01,bottom-top+2*pad));
                auto document=saved.document;document["shapes"]=QJsonArray{shape};document["anchors"]=links;
                document["nativeOwner"]="MuseDrawNative";document["nativeIndex"]=crossing.nativeIndex;
                document["bounds"]=QJsonObject{{"x",box.x()},{"y",box.y()},{"width",box.width()},{"height",box.height()}};
                const auto error=insert(saved.score,QString::fromUtf8(QJsonDocument(document).toJson(QJsonDocument::Compact)),
                    shape.value("anchorMode").toString("rhythm"),box,RectF(box.x(),box.y(),box.width(),box.height()),true,false,true);
                if(!error.startsWith("Desenho inserido"))LOGE()<<"[MuseDraw measure insertion] "<<error;
            }
        }
        if(!scores.empty())LOGI()<<"[MuseDraw measure insertion] capture="<<captureMilliseconds
            <<"ms adapt="<<adaptation.elapsed()<<"ms score-layout="<<layoutMilliseconds<<"ms scores="<<scores.size();
    }
};

static QHash<const MasterScore*,std::weak_ptr<MeasureInsertionBatch>> insertionBatches;
class MeasureInsertionBatch {
public:
    const MasterScore* master=nullptr;
    std::shared_ptr<MeasureInsertionState> state;
    ~MeasureInsertionBatch() {insertionBatches.remove(master);state.reset();}
};

std::shared_ptr<MeasureInsertionBatch> batchMeasureInsertion(Score* score) {
    const auto master=score->masterScore();
    if(const auto active=insertionBatches.value(master).lock())return active;
    auto batch=std::make_shared<MeasureInsertionBatch>();batch->master=master;
    insertionBatches.insert(master,batch);return batch;
}

std::shared_ptr<MeasureInsertionState> prepareMeasureInsertion(Score* score,const Fraction& tick,const Fraction& length) {
    const auto batch=insertionBatches.value(score->masterScore()).lock();
    if(batch&&batch->state){batch->state->length+=length;return batch->state;}
    QElapsedTimer capture;capture.start();
    auto state=std::make_shared<MeasureInsertionState>();state->tick=tick;state->length=length;
    for(auto local:score->scoreList()) {
        MeasureInsertionState::Saved saved;saved.score=local;
        QSet<QString> seen,affectedGroups;bool affected=false;
        for(auto image:nativeImages(local)) {
            const auto doc=metadata(image);saved.images.emplace_back(image,doc);
            const auto group=doc.value("group").toString();if(seen.contains(group))continue;seen.insert(group);
            auto& range=*image->drawingCache();
            if(!range.anchorRangeReady) {
                Fraction first,last;QSet<QString> ids;
                for(const auto& value:doc.value("source").toObject().value("anchors").toArray()) {
                    auto link=value.toObject();const auto member=referencedMeasure(local,link);if(!member)continue;
                    const auto id=measureIdentity(member);link["measureId"]=id;ids.insert(id);
                    const auto time=member->tick()+Fraction(link.value("numerator").toInt(),std::max(1,link.value("denominator").toInt()));
                    if(range.firstAnchor.isEmpty()||time<first){first=time;range.firstAnchor=link;}
                    if(range.lastAnchor.isEmpty()||time>last){last=time;range.lastAnchor=link;}
                }
                range.severalAnchoredMeasures=ids.size()>1;range.anchorRangeReady=true;
            }
            const auto firstLink=range.firstAnchor;
            const auto firstMember=referencedMeasure(local,firstLink),lastMember=referencedMeasure(local,range.lastAnchor);
            const bool before=firstMember&&firstMember->tick()+Fraction(firstLink.value("numerator").toInt(),std::max(1,firstLink.value("denominator").toInt()))<tick;
            const bool after=lastMember&&lastMember->tick()+Fraction(range.lastAnchor.value("numerator").toInt(),std::max(1,range.lastAnchor.value("denominator").toInt()))>=tick;
            const auto parts=doc.value("parts").toArray();
            if(!parts.isEmpty()) {
                const auto member=referencedMeasure(local,parts.last().toObject());
                if(member&&member->tick()>=tick){affected=true;affectedGroups.insert(group);}
            }
            if(after){affected=true;affectedGroups.insert(group);}
            const auto sourceShapes=doc.value("source").toObject().value("shapes").toArray();
            if(before&&after&&range.severalAnchoredMeasures&&!sourceShapes.isEmpty()&&!sourceShapes[0].toObject().value("singleAnchor").toBool()) {
                MeasureInsertionState::Crossing crossing;crossing.id=sourceShapes[0].toObject().value("drawId").toString();
                crossing.nativeIndex=doc.value("nativeIndex").toInt();
                crossing.staff=localStaff(local,firstLink,image->staffIdx());
                if(auto measure=referencedMeasure(local,firstLink)) {
                    if(measure->system()&&measure->system()->staff(crossing.staff)) {
                        crossing.first=measure;crossing.origin=QPointF(measure->canvasPos().x(),measure->system()->staffCanvasYpage(crossing.staff));
                    }
                }
                if(crossing.first){saved.crossing.push_back(crossing);affected=true;affectedGroups.insert(group);}
            }
        }
        if(!affected)continue;
        saved.images.erase(std::remove_if(saved.images.begin(),saved.images.end(),[&](const auto& entry){
            return !affectedGroups.contains(entry.second.value("group").toString());
        }),saved.images.end());
        QHash<QString,QJsonObject> identified;
        for(auto& entry:saved.images) {
            const auto group=entry.second.value("group").toString();
            auto found=identified.find(group);
            if(found==identified.end())found=identified.insert(group,withMeasureIdentities(local,entry.second));
            entry.second=*found;
        }
        if(!saved.crossing.empty()) {
            QSet<QString> ids;for(const auto& crossing:saved.crossing)ids.insert(crossing.id);
            saved.document=QJsonDocument::fromJson(nativeDocumentFiltered(local,&ids).toUtf8()).object();
        }
        suspendedDrawingLayout.insert(local);state->scores.push_back(std::move(saved));
    }
    state->captureMilliseconds=capture.elapsed();
    if(batch)batch->state=state;
    return state;
}

bool layout(const Image* image, Image::LayoutData* data) {
    if (image->drawingPart().empty() || image->drawingData().empty()
        || !image->explicitParent() || !image->explicitParent()->isSegment()) {
        return false;
    }
    const QJsonObject doc = metadata(image);
    if (doc.value("version").toInt() != 1) {
        return false;
    }
    if(doc.value("nativeOwner")=="MuseDrawNative"&&suspendedDrawingLayout.contains(image->score()))return true;
    if(doc.value("nativeOwner").toString()=="MuseDrawNative")const_cast<Image*>(image)->setSelectable(false);
    if(uniformPartCopy(image,doc)) {
        auto writable=const_cast<Image*>(image);
        const auto placement=wholePartPlacement(image,doc);
        if(placement.box.isEmpty()) {
            // Keep logical fragment membership for undo, routing and stable
            // multi-rest layout; the extra instances carry no visible ink.
            writable->setDrawingSvg("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"1\" height=\"1\" viewBox=\"0 0 1 1\"/>");
            writable->setSize(SizeF(0.01,0.01));writable->setOffset(PointF(0,0));
            data->setPos(0,0);data->setBbox(RectF(0,0,0,0));return true;
        }
        const auto segment=toSegment(image->explicitParent());
        const double sp=image->spatium();
        const double x=placement.box.x()-segment->canvasPos().x();
        const double y=placement.box.y()-placement.measure->system()->staffCanvasYpage(image->staffIdx());
        writable->setDrawingSvg(cachedRigidSvg(image).toStdString());
        writable->setSize(SizeF(placement.box.width()/sp,placement.box.height()/sp));
        writable->setOffset(PointF(x,y));
        data->setPos(x,y);data->setBbox(RectF(0,0,placement.box.width(),placement.box.height()));
        return true;
    }
    if (doc.value("rigid").toBool()) {
        const auto parentSegment=toSegment(image->explicitParent());
        if(!parentSegment->measure()||!parentSegment->measure()->system()||!parentSegment->measure()->system()->staff(image->staffIdx()))return false;
        const double sp = image->spatium();
        const_cast<Image*>(image)->setDrawingSvg(cachedRigidSvg(image).toStdString());
        double x=doc.value("fixedX").toDouble()*sp;
        const auto anchor=doc.value("rigidAnchor").toObject();
        if(!anchor.isEmpty())if(const auto m=referencedMeasure(image->score(),anchor)) {
            if(m->system())x+=m->canvasPos().x()+rhythmicX(m,fraction(anchor,"time"))-parentSegment->canvasPos().x();
        }
        double y=doc.value("fixedY").toDouble()*sp;
        const auto free=doc.value("rigidFree").toObject();
        if(!free.isEmpty())for(const auto page:image->score()->pages()) {
            if(page->pageNumber()!=free.value("page").toInt())continue;
            const auto segment=toSegment(image->explicitParent());
            x=page->canvasPos().x()+free.value("x").toDouble()-segment->canvasPos().x();
            y=page->canvasPos().y()+free.value("y").toDouble()-segment->measure()->system()->staffCanvasYpage(image->staffIdx());break;
        }
        const auto shapes=doc.value("source").toObject().value("shapes").toArray();
        const auto link=shapes.isEmpty()?QJsonObject():shapes[0].toObject().value("textLink").toObject();
        QPointF target;
        bool attached=false;
        if(link.value("kind").toString()=="shape") {
            for(Image* parentImage:nativeImages(image->score())) {
                if(parentImage==image)continue;
                const auto parentShapes=metadata(parentImage).value("source").toObject().value("shapes").toArray();
                if(parentShapes.isEmpty()||parentShapes[0].toObject().value("drawId")!=link.value("drawId"))continue;
                for(const auto& v:editingPoints(image->score(),parentImage)) {
                    const auto p=v.toMap();
                    if(p.value("command").toInt()==link.value("command").toInt()&&p.value("field").toInt()==link.value("field").toInt()) {
                        target=QPointF(p.value("x").toDouble(),p.value("y").toDouble());attached=true;break;
                    }
                }
                break;
            }
        } else if(link.value("kind").toString()=="rhythm"&&link.value("denominator").toInt()>0) {
            const auto staff=localStaff(image->score(),link,image->staffIdx());
            const Fraction time(link.value("numerator").toInt(),link.value("denominator").toInt());
            if(const auto m=referencedMeasure(image->score(),link)) {
                if(m->system()&&m->system()->staff(staff)) {
                    target=QPointF(m->canvasPos().x()+rhythmicX(m,time)+link.value("markerOffsetX").toDouble(),m->system()->staffCanvasYpage(staff));attached=true;
                }
            }
        }
        if(attached) {
            const auto segment=toSegment(image->explicitParent());
            x=target.x()+link.value("dx").toDouble()-doc.value("fixedWidth").toDouble()*sp/2-segment->canvasPos().x();
            y=target.y()+link.value("dy").toDouble()-doc.value("fixedHeight").toDouble()*sp/2-segment->measure()->system()->staffCanvasYpage(image->staffIdx());
        }
        const auto attachedBox = anchoredRigidBox(image->score(), doc.value("source").toObject(),image->staffIdx());
        if (!attachedBox.isEmpty()) {
            data->setPos(attachedBox.x() - parentSegment->canvasPos().x(),
                         attachedBox.y() - parentSegment->measure()->system()->staffCanvasYpage(image->staffIdx()));
            data->setBbox(RectF(0,0,attachedBox.width(),attachedBox.height()));
        } else {
            data->setPos(x,y);
            data->setBbox(RectF(0,0,doc.value("fixedWidth").toDouble()*sp,doc.value("fixedHeight").toDouble()*sp));
        }
        return true;
    }
    const auto parts = selectionParts(image->score(),image);
    if (parts.empty()) {
        return false;
    }
    QRectF box;
    const QString svg = cachedPaintSvg(image, doc, parts, &box);
    if (svg.isEmpty()) {
        return false;
    }
    const auto segment = toSegment(image->explicitParent());
    const auto current = std::find_if(parts.begin(), parts.end(), [&](const Part& p) {return p.measure == segment->measure();});
    if (current == parts.end()) {
        return false;
    }
    auto writable = const_cast<Image*>(image);
    writable->setDrawingSvg(svg.toStdString());
    writable->setSize(SizeF(box.width(), box.height()));
    data->setPos(current->left - segment->x() + (box.x() - current->origin) * image->spatium(), box.y() * image->spatium());
    data->setBbox(RectF(0, 0, box.width() * image->spatium(), box.height() * image->spatium()));
    return true;
}
}
#endif
