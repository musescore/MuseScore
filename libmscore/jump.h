//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __JUMP_H__
#define __JUMP_H__

#include "text.h"

namespace Ms {
//---------------------------------------------------------
//   @@ Jump
///    Jump label
//
//   @P continueAt  string
//   @P jumpTo      string
// not used?
//      jumpType    enum (Jump.DC, .DC_AL_FINE, .DC_AL_CODA, .DS_AL_CODA, .DS_AL_FINE, .DS, USER) (read only)
//   @P playUntil   string
//---------------------------------------------------------

class Jump final : public TextBase
{
    QString _jumpTo;
    QString _playUntil;
    QString _continueAt;
    bool _playRepeats;

public:
    enum class Type : char {
        DC,
        DC_AL_FINE,
        DC_AL_CODA,
        DS_AL_CODA,
        DS_AL_FINE,
        DS,
        USER
    };

    Jump(Score*);

    void setJumpType(Type t);
    Type jumpType() const;
    QString jumpTypeUserName() const;

    Jump* clone() const override { return new Jump(*this); }
    ElementType type() const override { return ElementType::JUMP; }
    int subtype() const override { return int(jumpType()); }

    Measure* measure() const { return toMeasure(parent()); }

    void read(XmlReader&) override;
    void write(XmlWriter& xml) const override;

    void layout() override;

    QString jumpTo() const { return _jumpTo; }
    QString playUntil() const { return _playUntil; }
    QString continueAt() const { return _continueAt; }
    void setJumpTo(const QString& s) { _jumpTo = s; }
    void setPlayUntil(const QString& s) { _playUntil = s; }
    void setContinueAt(const QString& s) { _continueAt = s; }
    void undoSetJumpTo(const QString& s);
    void undoSetPlayUntil(const QString& s);
    void undoSetContinueAt(const QString& s);
    bool playRepeats() const { return _playRepeats; }
    void setPlayRepeats(bool val) { _playRepeats = val; }

    QVariant getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const QVariant&) override;
    QVariant propertyDefault(Pid) const override;

    Element* nextSegmentElement() override;
    Element* prevSegmentElement() override;
    QString accessibleInfo() const override;
};

//---------------------------------------------------------
//   JumpTypeTable
//---------------------------------------------------------

struct JumpTypeTable {
    Jump::Type type;
    const char* text;
    const char* jumpTo;
    const char* playUntil;
    const char* continueAt;
    QString userText;
};

extern const JumpTypeTable jumpTypeTable[];
int jumpTypeTableSize();
}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Jump::Type);

#endif
