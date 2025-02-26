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

#include "atts_stringtab.h"

//----------------------------------------------------------------------------

#include <cassert>

//----------------------------------------------------------------------------

namespace libmei {

//----------------------------------------------------------------------------
// AttStringtab
//----------------------------------------------------------------------------

AttStringtab::AttStringtab() : Att()
{
    ResetStringtab();
}

void AttStringtab::ResetStringtab()
{
    m_tabFing = "";
    m_tabFret = "";
    m_tabLine = 0;
    m_tabString = "";
    m_tabCourse = MEI_UNSET;
}

bool AttStringtab::ReadStringtab(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("tab.fing")) {
        this->SetTabFing(StrToStr(element.attribute("tab.fing").value()));
        if (removeAttr) element.remove_attribute("tab.fing");
        hasAttribute = true;
    }
    if (element.attribute("tab.fret")) {
        this->SetTabFret(StrToStr(element.attribute("tab.fret").value()));
        if (removeAttr) element.remove_attribute("tab.fret");
        hasAttribute = true;
    }
    if (element.attribute("tab.line")) {
        this->SetTabLine(StrToInt(element.attribute("tab.line").value()));
        if (removeAttr) element.remove_attribute("tab.line");
        hasAttribute = true;
    }
    if (element.attribute("tab.string")) {
        this->SetTabString(StrToStr(element.attribute("tab.string").value()));
        if (removeAttr) element.remove_attribute("tab.string");
        hasAttribute = true;
    }
    if (element.attribute("tab.course")) {
        this->SetTabCourse(StrToInt(element.attribute("tab.course").value()));
        if (removeAttr) element.remove_attribute("tab.course");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttStringtab::WriteStringtab(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasTabFing()) {
        element.append_attribute("tab.fing") = StrToStr(this->GetTabFing()).c_str();
        wroteAttribute = true;
    }
    if (this->HasTabFret()) {
        element.append_attribute("tab.fret") = StrToStr(this->GetTabFret()).c_str();
        wroteAttribute = true;
    }
    if (this->HasTabLine()) {
        element.append_attribute("tab.line") = IntToStr(this->GetTabLine()).c_str();
        wroteAttribute = true;
    }
    if (this->HasTabString()) {
        element.append_attribute("tab.string") = StrToStr(this->GetTabString()).c_str();
        wroteAttribute = true;
    }
    if (this->HasTabCourse()) {
        element.append_attribute("tab.course") = IntToStr(this->GetTabCourse()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttStringtab::HasTabFing() const
{
    return (m_tabFing != "");
}

bool AttStringtab::HasTabFret() const
{
    return (m_tabFret != "");
}

bool AttStringtab::HasTabLine() const
{
    return (m_tabLine != 0);
}

bool AttStringtab::HasTabString() const
{
    return (m_tabString != "");
}

bool AttStringtab::HasTabCourse() const
{
    return (m_tabCourse != MEI_UNSET);
}

//----------------------------------------------------------------------------
// AttStringtabPosition
//----------------------------------------------------------------------------

AttStringtabPosition::AttStringtabPosition() : Att()
{
    ResetStringtabPosition();
}

void AttStringtabPosition::ResetStringtabPosition()
{
    m_tabPos = MEI_UNSET;
}

bool AttStringtabPosition::ReadStringtabPosition(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("tab.pos")) {
        this->SetTabPos(StrToInt(element.attribute("tab.pos").value()));
        if (removeAttr) element.remove_attribute("tab.pos");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttStringtabPosition::WriteStringtabPosition(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasTabPos()) {
        element.append_attribute("tab.pos") = IntToStr(this->GetTabPos()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttStringtabPosition::HasTabPos() const
{
    return (m_tabPos != MEI_UNSET);
}

//----------------------------------------------------------------------------
// AttStringtabTuning
//----------------------------------------------------------------------------

AttStringtabTuning::AttStringtabTuning() : Att()
{
    ResetStringtabTuning();
}

void AttStringtabTuning::ResetStringtabTuning()
{
    m_tabStrings = "";
    m_tabCourses = "";
}

bool AttStringtabTuning::ReadStringtabTuning(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("tab.strings")) {
        this->SetTabStrings(StrToStr(element.attribute("tab.strings").value()));
        if (removeAttr) element.remove_attribute("tab.strings");
        hasAttribute = true;
    }
    if (element.attribute("tab.courses")) {
        this->SetTabCourses(StrToStr(element.attribute("tab.courses").value()));
        if (removeAttr) element.remove_attribute("tab.courses");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttStringtabTuning::WriteStringtabTuning(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasTabStrings()) {
        element.append_attribute("tab.strings") = StrToStr(this->GetTabStrings()).c_str();
        wroteAttribute = true;
    }
    if (this->HasTabCourses()) {
        element.append_attribute("tab.courses") = StrToStr(this->GetTabCourses()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttStringtabTuning::HasTabStrings() const
{
    return (m_tabStrings != "");
}

bool AttStringtabTuning::HasTabCourses() const
{
    return (m_tabCourses != "");
}

} // namespace libmei
