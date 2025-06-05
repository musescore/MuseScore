/////////////////////////////////////////////////////////////////////////////
// Authors:     Laurent Pugin and Rodolfo Zitellini
// Created:     2014
// Copyright (c) Authors and others. All rights reserved.
//
// Code generated using a modified version of libmei
// by Andrew Hankinson, Alastair Porter, and Others
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// NOTE: this file was generated with the Verovio libmei version and
// should not be edited because changes will be lost.
/////////////////////////////////////////////////////////////////////////////

#ifndef __LIBMEI_STRINGTAB_H__
#define __LIBMEI_STRINGTAB_H__

#include "att.h"
#include "attdef.h"
#include "pugixml.hpp"

//----------------------------------------------------------------------------

#include "element.h"
#include "atts_externalsymbols.h"
#include "atts_shared.h"
#include "atts_stringtab.h"


namespace libmei {

/** A barre in a chord tablature grid. **/
class Barre : public Element, public AttLabelled, public AttTyped, public AttStartEndId, public AttStartId {
    public:
        Barre();
        virtual ~Barre();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** Describes the tuning of a course on a stringed instrument (e.g., guitar, lute). **/
class Course : public Element, public AttLabelled, public AttTyped, public AttAccidental, public AttPitch, public AttOctave {
    public:
        Course();
        virtual ~Course();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * Used to modify tuning information given by the course element.
 * Describes the tuning of an individual string within a course on a stringed
 * instrument (e.g., guitar, lute).
 **/
class String : public Element, public AttLabelled, public AttTyped, public AttAccidental, public AttPitch, public AttOctave {
    public:
        String();
        virtual ~String();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** A visual indication of the duration of a tabGrp. **/
class TabDurSym : public Element, public AttLabelled, public AttTyped, public AttStringtab, public AttLayerIdent, public AttColor, public AttExtSymAuth, public AttExtSymNames, public AttPlacementRelStaff, public AttTypography, public AttVisualOffsetHo, public AttVisualOffsetVo {
    public:
        TabDurSym();
        virtual ~TabDurSym();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * A group of simultaneous tab notes, comparable to a chord in CMN.
 * Rarely, may also contain rests, as in some "German" lute tablatures.
 **/
class TabGrp : public Element, public AttLabelled, public AttTyped, public AttAugmentDots, public AttDurationLog, public AttLayerIdent, public AttStaffIdent, public AttVisualOffsetHo {
    public:
        TabGrp();
        virtual ~TabGrp();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

} // namespace libmei

#endif // __LIBMEI_STRINGTAB_H__
