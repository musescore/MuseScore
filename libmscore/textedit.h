//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2014 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __TEXTEDIT_H__
#define __TEXTEDIT_H__

#include "element.h"
#include "text.h"
#include "undo.h"

namespace Ms {

//---------------------------------------------------------
//   TextEditData
//---------------------------------------------------------

struct TextEditData : public ElementEditData {
      QString oldXmlText;
      int startUndoIdx;

      TextCursor cursor;

      TextEditData(TextBase* t) : cursor(t)  {}
      ~TextEditData() {}
      };

//---------------------------------------------------------
//   ChangeText
//---------------------------------------------------------

class ChangeText : public UndoCommand {
      TextCursor c;
      QString s;

   protected:
      void insertText(EditData*);
      void removeText(EditData*);

   public:
      ChangeText(const TextCursor* tc, const QString& t) : c(*tc), s(t) {}
      virtual void undo(EditData*) override = 0;
      virtual void redo(EditData*) override = 0;
      const TextCursor& cursor() const { return c; }
      const QString& string() const    { return s; }
      };

//---------------------------------------------------------
//   InsertText
//---------------------------------------------------------

class InsertText : public ChangeText {

   public:
      InsertText(const TextCursor* tc, const QString& t) : ChangeText(tc, t) {}
      virtual void redo(EditData* ed) override { insertText(ed); }
      virtual void undo(EditData* ed) override { removeText(ed); }
      UNDO_NAME("InsertText")
      };

//---------------------------------------------------------
//   RemoveText
//---------------------------------------------------------

class RemoveText : public ChangeText {

   public:
      RemoveText(const TextCursor* tc, const QString& t) : ChangeText(tc, t) {}
      virtual void redo(EditData* ed) override { removeText(ed); }
      virtual void undo(EditData* ed) override { insertText(ed); }
      UNDO_NAME("InsertText")
      };

//---------------------------------------------------------
//   SplitJoinText
//---------------------------------------------------------

class SplitJoinText : public UndoCommand {
   protected:
      TextCursor c;
      virtual void split(EditData*);
      virtual void join(EditData*);

   public:
      SplitJoinText(const TextCursor* tc) : c(*tc) {}
      };

//---------------------------------------------------------
//   SplitText
//---------------------------------------------------------

class SplitText : public SplitJoinText {

      virtual void undo(EditData* data) override { join(data); }
      virtual void redo(EditData* data) override { split(data); }

   public:
      SplitText(const TextCursor* tc) : SplitJoinText(tc) {}
      UNDO_NAME("SplitText");
      };

//---------------------------------------------------------
//   JoinText
//---------------------------------------------------------

class JoinText : public SplitJoinText {

      virtual void undo(EditData* data) override { split(data); }
      virtual void redo(EditData* data) override { join(data);  }

   public:
      JoinText(const TextCursor* tc) : SplitJoinText(tc) {}
      UNDO_NAME("JoinText");
      };


}     // namespace Ms

#endif


