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

#ifndef __LIBMEI_LYRICS_H__
#define __LIBMEI_LYRICS_H__

#include "att.h"
#include "attdef.h"
#include "pugixml.hpp"

//----------------------------------------------------------------------------

#include "element.h"
#include "atts_shared.h"


namespace libmei {

/**
 * Recurring lyrics, especially at the end of each verse or stanza of a poem or
 * song lyrics; a chorus.
 **/
class Refrain : public Element, public AttLabelled, public AttTyped, public AttLang, public AttPlacementRelStaff, public AttTypography, public AttVisualOffsetVo, public AttVoltaGroupingSym {
    public:
        Refrain();
        virtual ~Refrain();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * Division of a poem or song lyrics, sometimes having a fixed length, meter or
 * rhyme scheme; a stanza.
 **/
class Verse : public Element, public AttLabelled, public AttTyped, public AttPlacementRelStaff, public AttTypography, public AttVisualOffsetVo, public AttVoltaGroupingSym, public AttNNumberLike {
    public:
        Verse();
        virtual ~Verse();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** Sung text for a specific iteration of a repeated section of music. **/
class Volta : public Element, public AttLabelled, public AttTyped, public AttLang, public AttTypography, public AttVisualOffsetVo {
    public:
        Volta();
        virtual ~Volta();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

} // namespace libmei

#endif // __LIBMEI_LYRICS_H__
