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
#include "noteinputcursor.h"

using namespace mu::notation;
using namespace mu::engraving;




//somethingstarted notes trying to fix #10664  -  dont delete these comments just yet.
// to-do:     if fret 1-9 one width
// to-do:       if fret 10+ different width
//to-do: reference  striungdata.cpp, because eventually i want it to have different widths for 1 or 2 digit frets. 


void NoteInputCursor::paint(mu::draw::Painter* painter)
{
    INotationNoteInputPtr noteInput = currentNoteInput();
    if (!noteInput || !noteInput->isNoteInputMode()) {
        return;
    }

    NoteInputState state = noteInput->state();
    RectF cursorRect  = noteInput->cursorRect();
    


            //will need these later. please dont delete. - somethingstarted
    //double widthMultiplierBefore = 1.0;  //change size of both (keep at 1 normally)
    //double oneDigitFretMult = 1.2;  //if it's  0-9,  frets width 
    //double twoDigitFretMult = 1.2;  //if it's 10+,  frets width

    double widthMultiplierFinal = 1.2; 
    constexpr int leftLineWidth = 3;    

    PointX oldCenter = cursorRect.center(); //store the old center
    double oldWidth = cursorRect.width();//store old width
    double newWidth = cursorRect.width() * widthMultiplierFinal;      //set new width
    double pixToSlideLeft = (((oldWidth - newWidth) / 2) - (leftLineWidth / 2)); //((find new left justification position) - (divide left line by 2 to find a middle ground between 2ch and 1ch frets.))

    cursorRect.setLeft(cursorRect.left() + pixToSlideLeft); 
    cursorRect.setWidth(newWidth); 
    
    
    

    Color fillColor = configuration()->selectionColor(state.currentVoiceIndex);
    Color cursorRectColor = fillColor;
    cursorRectColor.setAlpha(configuration()->cursorOpacity());
    painter->fillRect(cursorRect, cursorRectColor);

    

     
    RectF leftLine(cursorRect.topLeft().x(), cursorRect.topLeft().y(), leftLineWidth, cursorRect.height());
    Color lineColor = fillColor;
    painter->fillRect(leftLine, lineColor);

    if (state.staffGroup == StaffGroup::TAB) {
        const StaffType* staffType = state.staff ? state.staff->staffType() : nullptr;

        if (staffType) {
            staffType->drawInputStringMarks(painter, state.currentString, state.currentVoiceIndex, cursorRect);
        }
    }
}

INotationNoteInputPtr NoteInputCursor::currentNoteInput() const
{
    auto notation = globalContext()->currentNotation();
    if (!notation) {
        return nullptr;
    }

    auto interaction = notation->interaction();
    if (!interaction) {
        return nullptr;
    }

    return interaction->noteInput();
}
