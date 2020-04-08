#include "elementrepositoryservice.h"

#include "chord.h"
#include "stem.h"
#include "hook.h"
#include "beam.h"
#include "glissando.h"
#include "hairpin.h"

ElementRepositoryService::ElementRepositoryService(QObject* parent) : QObject(parent)
{

}

QObject* ElementRepositoryService::getQObject()
{
    return this;
}

void ElementRepositoryService::updateElementList(const QList<Ms::Element*>& newRawElementList)
{
    m_elementList = exposeRawElements(newRawElementList);

    emit elementsUpdated();
}

QList<Ms::Element*> ElementRepositoryService::findElementsByType(const Ms::ElementType elementType) const
{
    switch (elementType) {
    case Ms::ElementType::CHORD: return findChords();
    case Ms::ElementType::NOTE: return findNotes();
    case Ms::ElementType::STEM: return findStems();
    case Ms::ElementType::HOOK: return findHooks();
    case Ms::ElementType::BEAM: return findBeams();
    case Ms::ElementType::GLISSANDO_SEGMENT: return findGlissandos();
    case Ms::ElementType::HAIRPIN_SEGMENT: return findHairpins();
    default:
        QList<Ms::Element*> resultList;

        for (Ms::Element* element : m_elementList) {
            if (element->type() == elementType) {
                resultList << element;
            }
        }

        return resultList;
    }
}

QList<Ms::Element*> ElementRepositoryService::takeAllElements() const
{
    return m_elementList;
}

QList<Ms::Element*> ElementRepositoryService::exposeRawElements(const QList<Ms::Element*>& rawElementList) const
{
    QList<Ms::Element*> resultList;

    for (const Ms::Element* element : rawElementList) {

        if (!resultList.contains(element->elementBase())) {
            resultList << element->elementBase();
        }

        if (element->type() == Ms::ElementType::BEAM) {
            const Ms::Beam* beam = Ms::toBeam(element);

            for (Ms::ChordRest* chordRest : beam->elements()) {
                resultList << chordRest;
            }
        }
    }

    return resultList;
}

QList<Ms::Element*> ElementRepositoryService::findChords() const
{
    QList<Ms::Element*> resultList;

    for (const Ms::Element* element : m_elementList) {

        if (element->type() == Ms::ElementType::CHORD) {
            resultList << Ms::toChord(const_cast<Ms::Element*>(element));
        }
    }

    return resultList;
}

QList<Ms::Element*> ElementRepositoryService::findNotes() const
{
    QList<Ms::Element*> resultList;

    for (const Ms::Element* element : findChords()) {

        const Ms::Chord* chord = Ms::toChord(element);

        if (!chord)
            continue;

        for (Ms::Element* note : chord->notes()) {
            resultList << note;
        }
    }

    return resultList;
}

QList<Ms::Element *> ElementRepositoryService::findStems() const
{
    QList<Ms::Element*> resultList;

    for (const Ms::Element* element : findChords()) {

        const Ms::Chord* chord = Ms::toChord(element);

        if (chord && chord->stem()) {
            resultList << chord->stem();
        }
    }

    return resultList;
}

QList<Ms::Element*> ElementRepositoryService::findHooks() const
{
    QList<Ms::Element*> resultList;

    for (const Ms::Element* element : findChords()) {

        const Ms::Chord* chord = Ms::toChord(element);

        if (chord && chord->hook()) {
            resultList << chord->hook();
        }
    }

    return resultList;
}

QList<Ms::Element*> ElementRepositoryService::findBeams() const
{
    QList<Ms::Element*> resultList;

    for (const Ms::Element* element : findChords()) {

        Ms::Element* beam = nullptr;

        if (element->isChord()) {
            const Ms::Chord* chord = Ms::toChord(element);

            if (!chord)
                continue;

            beam = chord->beam();

        } else if (element->isBeam()) {
            beam = const_cast<Ms::Element*>(element);
        }

        if (!beam || resultList.contains(beam))
            continue;

        resultList << beam;
    }

    return resultList;
}

QList<Ms::Element*> ElementRepositoryService::findGlissandos() const
{
    QList<Ms::Element*> resultList;

    for (Ms::Element* element : m_elementList) {

        if (element->type() == Ms::ElementType::GLISSANDO_SEGMENT) {

            const Ms::GlissandoSegment* glissandoSegment = Ms::toGlissandoSegment(element);

            if (!glissandoSegment)
                continue;

            resultList << glissandoSegment->glissando();

        } else if (element->type() == Ms::ElementType::GLISSANDO) {
            resultList << element;
        }
    }

    return resultList;
}

QList<Ms::Element*> ElementRepositoryService::findHairpins() const
{
    QList<Ms::Element*> resultList;

    for (Ms::Element* element : m_elementList) {

        if (element->type() == Ms::ElementType::HAIRPIN_SEGMENT) {

            const Ms::HairpinSegment* hairpinSegment = Ms::toHairpinSegment(element);

            if (!hairpinSegment)
                continue;

            resultList << hairpinSegment->hairpin();

        } else if (element->type() == Ms::ElementType::HAIRPIN) {
            resultList << element;
        }
    }

    return resultList;
}
