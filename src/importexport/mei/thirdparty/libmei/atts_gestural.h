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

#ifndef __LIBMEI_ATTS_GESTURAL_H__
#define __LIBMEI_ATTS_GESTURAL_H__

#include "att.h"
#include "attdef.h"
#include "pugixml.hpp"

//----------------------------------------------------------------------------

#include <string>

namespace libmei {

//----------------------------------------------------------------------------
// AttAccidentalGes
//----------------------------------------------------------------------------

class AttAccidentalGes : public Att {
protected:
    AttAccidentalGes();
    ~AttAccidentalGes() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetAccidentalGes();

    /** Read the values for the attribute class **/
    bool ReadAccidentalGes(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteAccidentalGes(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetAccidGes(data_ACCIDENTAL_GESTURAL accidGes_) { m_accidGes = accidGes_; }
    data_ACCIDENTAL_GESTURAL GetAccidGes() const { return m_accidGes; }
    bool HasAccidGes() const;
    ///@}

private:
    /** Records the performed pitch inflection. **/
    data_ACCIDENTAL_GESTURAL m_accidGes;
};

//----------------------------------------------------------------------------
// InstAccidentalGes
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttAccidentalGes
 */

class InstAccidentalGes : public AttAccidentalGes {
public:
    InstAccidentalGes() = default;
    virtual ~InstAccidentalGes() = default;
};

//----------------------------------------------------------------------------
// AttAttacking
//----------------------------------------------------------------------------

class AttAttacking : public Att {
protected:
    AttAttacking();
    ~AttAttacking() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetAttacking();

    /** Read the values for the attribute class **/
    bool ReadAttacking(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteAttacking(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetAttacca(data_BOOLEAN attacca_) { m_attacca = attacca_; }
    data_BOOLEAN GetAttacca() const { return m_attacca; }
    bool HasAttacca() const;
    ///@}

private:
    /**
     * Indicates that the performance of the next musical division should begin
     * immediately following this one.
     **/
    data_BOOLEAN m_attacca;
};

//----------------------------------------------------------------------------
// InstAttacking
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttAttacking
 */

class InstAttacking : public AttAttacking {
public:
    InstAttacking() = default;
    virtual ~InstAttacking() = default;
};

//----------------------------------------------------------------------------
// AttNoteGes
//----------------------------------------------------------------------------

class AttNoteGes : public Att {
protected:
    AttNoteGes();
    ~AttNoteGes() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetNoteGes();

    /** Read the values for the attribute class **/
    bool ReadNoteGes(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteNoteGes(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetOctGes(data_OCTAVE octGes_) { m_octGes = octGes_; }
    data_OCTAVE GetOctGes() const { return m_octGes; }
    bool HasOctGes() const;
    //
    void SetPnameGes(data_PITCHNAME pnameGes_) { m_pnameGes = pnameGes_; }
    data_PITCHNAME GetPnameGes() const { return m_pnameGes; }
    bool HasPnameGes() const;
    ///@}

private:
    /** Records performed octave information that differs from the written value. **/
    data_OCTAVE m_octGes;
    /** Contains a performed pitch name that differs from the written value. **/
    data_PITCHNAME m_pnameGes;
};

//----------------------------------------------------------------------------
// InstNoteGes
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttNoteGes
 */

class InstNoteGes : public AttNoteGes {
public:
    InstNoteGes() = default;
    virtual ~InstNoteGes() = default;
};

//----------------------------------------------------------------------------
// AttOrnamentAccidGes
//----------------------------------------------------------------------------

class AttOrnamentAccidGes : public Att {
protected:
    AttOrnamentAccidGes();
    ~AttOrnamentAccidGes() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetOrnamentAccidGes();

    /** Read the values for the attribute class **/
    bool ReadOrnamentAccidGes(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteOrnamentAccidGes(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetAccidupperGes(data_ACCIDENTAL_GESTURAL accidupperGes_) { m_accidupperGes = accidupperGes_; }
    data_ACCIDENTAL_GESTURAL GetAccidupperGes() const { return m_accidupperGes; }
    bool HasAccidupperGes() const;
    //
    void SetAccidlowerGes(data_ACCIDENTAL_GESTURAL accidlowerGes_) { m_accidlowerGes = accidlowerGes_; }
    data_ACCIDENTAL_GESTURAL GetAccidlowerGes() const { return m_accidlowerGes; }
    bool HasAccidlowerGes() const;
    ///@}

private:
    /** Records the sounding accidental associated with an upper neighboring note. **/
    data_ACCIDENTAL_GESTURAL m_accidupperGes;
    /** Records the sounding accidental associated with a lower neighboring note. **/
    data_ACCIDENTAL_GESTURAL m_accidlowerGes;
};

//----------------------------------------------------------------------------
// InstOrnamentAccidGes
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttOrnamentAccidGes
 */

class InstOrnamentAccidGes : public AttOrnamentAccidGes {
public:
    InstOrnamentAccidGes() = default;
    virtual ~InstOrnamentAccidGes() = default;
};

//----------------------------------------------------------------------------
// AttSoundLocation
//----------------------------------------------------------------------------

class AttSoundLocation : public Att {
protected:
    AttSoundLocation();
    ~AttSoundLocation() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetSoundLocation();

    /** Read the values for the attribute class **/
    bool ReadSoundLocation(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteSoundLocation(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetAzimuth(data_DEGREES azimuth_) { m_azimuth = azimuth_; }
    data_DEGREES GetAzimuth() const { return m_azimuth; }
    bool HasAzimuth() const;
    //
    void SetElevation(data_DEGREES elevation_) { m_elevation = elevation_; }
    data_DEGREES GetElevation() const { return m_elevation; }
    bool HasElevation() const;
    ///@}

private:
    /** The lateral or left-to-right plane. **/
    data_DEGREES m_azimuth;
    /** The above-to-below axis. **/
    data_DEGREES m_elevation;
};

//----------------------------------------------------------------------------
// InstSoundLocation
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttSoundLocation
 */

class InstSoundLocation : public AttSoundLocation {
public:
    InstSoundLocation() = default;
    virtual ~InstSoundLocation() = default;
};

} // namespace libmei

#endif // __LIBMEI_ATTS_GESTURAL_H__
