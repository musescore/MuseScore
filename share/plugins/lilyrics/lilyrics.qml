import QtQuick
import QtQuick.Controls

import MuseScore 3.0
import Muse.Ui
import Muse.UiComponents

// Inspired by roblyric, by Robbie Matthews

MuseScore {
    version: "1.2" // 21 - June - 2022
    description: qsTr("Apply lyrics in lilypond format.")
    title: "Lilypond Lyrics"
    categoryCode: "lyrics"
    thumbnailName: "lilyrics.png"
    pluginType: "dialog"

    requiresScore: true

    width: 800
    height: 400

    property var font:"\"FreeSerif\"";
    property var sinalefas:
        ["<font face=\"ScoreText\"/><font face="+font+"/>", // recommended in musescore docs
        "‿", // unicode undertie, the musicxml standard is <elision>‿</elision>
        "_"  // ASCII underscore
    ];
    property var sinalefa: sinalefas[0];
    property var linkDictionaries: []
    property var linkOffsets: []
    property var onScreenStaves: []
    property var onScreenVoice: null
    property var onScreenVerse: null

    onRun: {}

    // no funciona en 3.2
    onScoreStateChanged: {
        spinStaff.value=staffSelection();
        spinVoice.value=voiceSelection();
        spinVerse.value=verseSelection();
        setTextCursor();
    }

    Item {
        id:window

        anchors.fill: parent

        StyledTextLabel {
            id: textLabel
            wrapMode: Text.WordWrap
            text: qsTr("Example: Ky -- _ ri -- e~e -- lei -- son __ _ _")
            anchors.left: window.left
            anchors.top: window.top
            anchors.leftMargin: 10
            anchors.topMargin: 10
        }

        Rectangle {

            anchors.top: textLabel.bottom
            anchors.left: window.left
            anchors.right: buttonDump.left
            anchors.bottom: bottomBar.top
            anchors.topMargin: 10
            anchors.bottomMargin: 10
            anchors.leftMargin: 10
            anchors.rightMargin: 5

            color: ui.theme.textFieldColor
            border.color: ui.theme.strokeColor
            border.width: Math.max(ui.theme.borderWidth, 1)
            radius: 3

            ScrollView {
                id: view

                anchors.fill: parent
                //! Bad work for Mac
                //ScrollBar.vertical.policy: ScrollBar.AlwaysOn

                TextArea {
                    id:textLily
                    width: parent.width
                    height: Math.max(textLily.implicitHeight, view.height)
                    anchors.margins: 8
                    wrapMode: TextEdit.WrapAnywhere
                    textFormat: TextEdit.PlainText
                    selectByMouse: true
                    text: ""
                }
            }
        }

        /******************************************
        **************** Buttons ******************
        ******************************************/

        // PASTE
        FlatButton {
            id : buttonPaste
            text: qsTr("Paste")
            anchors.right: window.right
            anchors.top: window.top
            anchors.topMargin: 10
            anchors.bottomMargin: 5
            anchors.rightMargin: 10
            height: 50
            toolTipTitle: qsTr("Copy the text to the score");
            onClicked: {
                curScore.startCmd();
                pasteGlobal();
                curScore.endCmd();
            }
        }

        // DUMP
        FlatButton {
            id : buttonDump
            text: qsTr("Dump")
            anchors.right: window.right
            anchors.top: buttonPaste.bottom
            anchors.topMargin: 5
            anchors.bottomMargin: 5
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            toolTipTitle: qsTr("Copy the lyrics of the score to the text area");
            onClicked: {
                //curScore.startCmd();
                dumpLyrics();
                //curScore.endCmd();
            }
        }

        // MERGE
        FlatButton {
            id : buttonMerge
            text: qsTr("Merge")
            anchors.right: window.right
            anchors.top: buttonDump.bottom
            anchors.topMargin: 5
            anchors.bottomMargin: 5
            anchors.rightMargin: 10
            toolTipTitle: qsTr("Combines several lyrics sections into one");
            onClicked: {
                //curScore.startCmd();
                mergeLyrics();
                //curScore.endCmd();
            }
        }

        // CLEAN
        FlatButton {
            id : buttonClean
            text: qsTr("Clean")
            anchors.right: window.right
            anchors.top: buttonMerge.bottom
            anchors.topMargin: 5
            anchors.bottomMargin: 5
            anchors.rightMargin: 10
            toolTipTitle: qsTr("Delete comments and lilypond commands");
            onClicked: {
                //curScore.startCmd();
                cleanLyrics();
                //curScore.endCmd();
            }
        }

        // UNTIE
        FlatButton {
            id : buttonTies
            text: qsTr("Untie")
            anchors.right: window.right
            anchors.top: buttonClean.bottom
            anchors.topMargin: 20
            anchors.bottomMargin: 5
            anchors.rightMargin: 10
            toolTipTitle: qsTr("Transform tied notes into dotted notes");
            onClicked: {
                curScore.startCmd();
                var track=(spinStaff.value-1)*4+spinVoice.value-1;
                ties2dots(track);
                curScore.endCmd();
            }
        }

        // UNTIE ALL
        FlatButton {
            id : buttonTiesAll
            text: qsTr("Untie all")
            anchors.right: window.right
            anchors.top: buttonTies.bottom
            anchors.topMargin: 2
            anchors.bottomMargin: 5
            anchors.rightMargin: 10
            toolTipTitle: qsTr("Transform tied notes into dotted notes in all the voices");
            onClicked: {
                curScore.startCmd();
                console.log(curScore.ntracks);
                for (var i=0; i<curScore.ntracks; i++)
                    ties2dots(i);
                spinStaff.value=1;
                spinVoice.value=1;
                curScore.endCmd();
            }
        }

        // CONDENSE
        FlatButton {
            id : buttonCondense
            text: qsTr("Condense")
            anchors.right: window.right
            anchors.top: buttonTiesAll.bottom
            anchors.topMargin: 20
            anchors.bottomMargin: 0
            anchors.rightMargin: 10
            toolTipTitle: qsTr("Convert the text to a more readable format.\n(it is necessary to expand it again in order to paste it into the score)");
            onClicked: {
                //curScore.startCmd();
                var s=textLily.text;
                textLily.cursorPosition=0;
                s=condenseLyrics(s);
                textLily.text=s;
                //curScore.endCmd();
            }
        }

        // EXPAND
        FlatButton {
            id : buttonExpand
            text: qsTr("Expand")
            anchors.right: window.right
            anchors.top: buttonCondense.bottom
            anchors.topMargin: 2
            anchors.bottomMargin: 5
            anchors.rightMargin: 10
            toolTipTitle: qsTr("Convert the text back to lilypond format");
            onClicked: {
                //curScore.startCmd();
                var s=textLily.text;
                s=expandLyrics(s);
                textLily.text=s;
                //curScore.endCmd();
            }
        }


        // CANCEL
        FlatButton {
            id : buttonCancel
            text: qsTr("Cancel")
            anchors.bottom: window.bottom
            anchors.right: window.right
            anchors.topMargin: 10
            anchors.bottomMargin: 10
            anchors.rightMargin: 10
            onClicked: {
                quit()
            }
        }


        /******************************************
        *********** Bottom bar controls ***********
        ******************************************/

        Row {
            id: bottomBar

            spacing: 10

            anchors.left: window.left
            anchors.right: window.right
            anchors.bottom: window.bottom
            anchors.margins: 10

            height: childrenRect.height

            Row {
                height: childrenRect.height

                StyledTextLabel {
                    id: labelStaff

                    anchors.verticalCenter: spinStaff.verticalCenter

                    text: qsTr("Staff: ")
                }

                SpinBox {
                    id : spinStaff
                    width: 40
                    from: 1;
                    to: Boolean(curScore) ? curScore.ntracks/4 : 0
                    value: staffSelection();
                    //horizontalAlignment: Text.AlignHCenter
                }
            }

            Row {
                height: childrenRect.height

                StyledTextLabel {
                    id: labelVoice

                    anchors.verticalCenter: spinVoice.verticalCenter

                    text: qsTr("Voice: ")
                }

                SpinBox {
                    id : spinVoice
                    width: 40
                    from: 1
                    to: 4
                    value: voiceSelection();
                    //horizontalAlignment: Text.AlignHCenter
                }
            }

            Row {
                height: childrenRect.height

                StyledTextLabel {
                    id: labelVerse

                    anchors.verticalCenter: spinVerse.verticalCenter

                    text: qsTr("Verse: ")
                }

                SpinBox {
                    id : spinVerse
                    width: 40
                    from: 1
                    to: 8 // arbitrary
                    value: verseSelection();
                    //horizontalAlignment: Text.AlignHCenter
                }
            }

            ComboBox {
                id : comboPos

                anchors.verticalCenter: comboElision.verticalCenter

                model: [qsTr("Above"), qsTr("Below")]
                currentIndex: 1
                width: 80
            }

            CheckBox {
                id : checkTie

                anchors.verticalCenter: comboElision.verticalCenter

                checked: true
                text: qsTr("Skip ties")

                onClicked: checked = !checked
            }

            CheckBox {
                id : checkExtender

                anchors.verticalCenter: comboElision.verticalCenter

                checked: true
                text: qsTr("Extender")

                onClicked: checked = !checked
            }

            FlatButton {
                id : buttonElision

                anchors.verticalCenter: comboElision.verticalCenter

                text: qsTr("~")
                width: 20
                toolTipTitle: qsTr("Character to use in synalepha:\n1: MuseScore elision\n2: Unicode undertie\n3: ASCII underscore");

            }

            ComboBox {
                id : comboElision
                model: ["1", "2", "3"]
                currentIndex: 0
                width: 36
            }
        }
    }

    /*********************************************
    *************** FUNCTIONS ********************
    *********************************************/

    function findSpaces(s) {
        s=s.replace(/" "/g, "@@@"); // any string of length 3 without spaces
        // find the indices of all whitespace characters
        var spaces=[];
        for (var i=0; i<s.length; i++) {
            if (s[i].match(/\s/)) spaces.push(i);
        }
        spaces.push(s.length); // possibility of cursor at the end of the string

        // now delete consecutive indices
        var purgedSpaces;
        purgedSpaces=[spaces[0]];
        var refIndex=0;
        for (var i=1; i<spaces.length; i++)
            if (spaces[i]!=spaces[refIndex]+i-refIndex) {
                purgedSpaces.push(spaces[i]);
                refIndex=i;
            }
        if (purgedSpaces[0]==0) purgedSpaces=purgedSpaces.slice(1);
        return purgedSpaces;
    }

    /******************************************
    *********** state changed functions *******
    *******************************************/

    function staffSelection() {
        if (!Boolean(curScore)) {
            return 0
        }

        var elem=curScore.selection.elements;
        if (elem.length==0) return 1;
        var track=elem[0].track;
        var voice=track % 4;
        return (track-voice)/4 + 1;
    }

    function voiceSelection() {
        if (!Boolean(curScore)) {
            return 0
        }

        var elem=curScore.selection.elements;
        if (elem.length==0) return 1;
        var track=elem[0].track;
        return track % 4 + 1;
    }

    function verseSelection() {
        if (!Boolean(curScore)) {
            return 0
        }

        var elem=curScore.selection.elements;
        if (elem.length==0) return 1;
        var verse=elem[0].verse;
        if (verse==null) return 1;
        return verse+1;
    }

    function setTextCursor() {
        if (!Boolean(curScore)) {
            return
        }

        var elem=curScore.selection.elements;
        if (elem.length==0) return;
        var verse=elem[0].verse;
        if (verse==null) return;
        textLily.select(0, 0); // clicking a lyric deselects in the textArea
        if (verse!=onScreenVerse-1) return;
        if (linkDictionaries.length==0) return;
        var voice=elem[0].parent.voice;
        if (voice!=onScreenVoice-1) return;
        var staff=(elem[0].track-voice)/4 + 1;
        var which=onScreenStaves.indexOf(staff);
        if (which==-1) return;

        var offset=linkOffsets[which];
        var linkDictionary=linkDictionaries[which];
        var tick=elem[0].parent.parent.tick;
        var ind=linkDictionary[tick];
        if (!ind) return;
        ind=ind+offset;
        if (ind>textLily.text.length) return;
        textLily.cursorPosition=ind;
        var start=ind-1;
        while(true) {
            if (start==offset) break;
            if (textLily.text[start-1].match(/\s/)) break;
            start--;
        }
        textLily.select(start, ind);
        textLily.forceActiveFocus();
    }


    /***************************************
    ************* Big functions ************
    ****************************************/

    function cleanLyrics() {
        var s=textLily.text;
        textLily.cursorPosition=0;
        s=s.replace(/%[^\n]*/g,"");       // delete comments
        s=s.replace(/\\[^ \n\t{}]*/g,""); // delete lilypond commands starting with \
        s=s.replace(/#[^ \n\t{}]*/g,"");  // delete scheme code starting with #
        s=s.replace(/  /g," ");           // double space -> single space
        s=stanzas(s);                     // stanzas
        textLily.text=s;
    }

    function mergeLyrics() {
        var s=textLily.text;
        const re = /{([^}]*)}/g;
        var match;
        var ss=[];
        while ((match = re.exec(s)) !== null) {
            ss.push(match[1]);
        }
        var ntracks=curScore.ntracks/4;
        if (ss.length<=ntracks) return;
        for (var i=ntracks; i<ss.length; i++) {
            ss[i%ntracks]+="\n" + ss[i];
        }
        var new_s="";
        for (var i=0; i<ntracks; i++) {
            new_s+= "{" + ss[i] + "}\n\n";
        }
        textLily.remove(0,s.length);
        textLily.insert(0, new_s);
    }

    function stanzas(s) {
        // before calling this funcion, \set has to be removed
        var re=/stanza(\s)*=(\s)*"([^"]*)"(\s)*([^\s]*)/g;
        var match;
        while ((match=re.exec(s)) !== null) {
            var stanzaNumber=match[3].replace(/ /g,""); // delete extra spaces
            s=  s.substring(0,match.index)
                    + stanzaNumber
                    + "_"
                    + match[5]
                    + s.substring(match.index+match[0].length);
            //  s=s.substring(0,match.index)+match[5]+s.substring(match.index+match[0].length); // this is to just delete the stanza
        }
        return s;
    }

    function dumpLyrics() {
        var skipTies=checkTie.checked;
        var extender=checkExtender.checked;
        var track=(spinStaff.value-1)*4+spinVoice.value-1;
        var verse=spinVerse.value-1;
        var sinalefa=sinalefas[comboElision.currentIndex];
        var cursor=curScore.newCursor();
        cursor.track=track;
        cursor.rewind(Cursor.SCORE_START);
        var s="";
        var justEnd=false;
        var w;
        var syl;
        var linkDictionary={};
        while(cursor.segment) {
            if (cursor.element.type==Element.CHORD) {
                // only works in Musescore version >= 3.3
                var note=cursor.element.notes[0];
                if (note.tieBack != null && skipTies) {
                    cursor.next(); continue;
                }
                var lyrics=cursor.element.lyrics;
                for (var i=0; i<lyrics.length; i++) {
                    if (lyrics[i].verse==verse) {
                        syl=lyrics[i].syllabic;
                        w=lyrics[i].text;
                        linkDictionary[cursor.tick]=s.length + w.length;
                        if (w=="") {
                            w="_";
                            if (justEnd && extender) w="__ _";
                            justEnd=false;
                        }
                        else {
                            if (w==" ") w="\" \"";
                            else w=w.replace(/ /g, "_");
                            w=w.replace(sinalefa,"~");
                            if (syl==Lyrics.BEGIN || syl==Lyrics.MIDDLE) w+=" --";
                            justEnd=syl==(Lyrics.END||syl==Lyrics.SINGLE);
                        }
                        s+=" " + w;
                    }
                }
            }
            cursor.next();
        }
        s=s.trim();
        textLily.text=s;
        textLily.cursorPosition=0;
        linkDictionaries=[linkDictionary];
        linkOffsets=[0];
        onScreenStaves=[spinStaff.value];
        onScreenVoice=spinVoice.value;
        onScreenVerse=spinVerse.value;
    }


    function deleteLyrics() {
        var track=(spinStaff.value-1)*4+spinVoice.value-1;
        var verse=spinVerse.value-1;
        var cursor=curScore.newCursor();
        cursor.track=track;
        cursor.rewind(Cursor.SCORE_START);
        while(cursor.segment) {
            if (cursor.element.type==Element.CHORD) {
                var lyrics=cursor.element.lyrics;
                for (var i=0; i<lyrics.length; i++) {
                    if (lyrics[i].verse==verse) {
                        cursor.element.remove(lyrics[i]);
                    }
                }
            }
            /*
            this should not be neccessary,
            but importing some broken midi files can
            result in lyrics applied to rests
            */
            if (cursor.element.type==Element.REST) {
                var lyrics=cursor.element.lyrics;
                if (lyrics!=null) // to allow running in older musescore versions
                    for (var i=0; i<lyrics.length; i++) {
                        if (lyrics[i].verse==verse) {
                            //cursor.element.remove(lyrics[i]);
                            /*
                        I cannot use the funcion remove() in rests (why?)
                        so I have to overwrite the lyrics properties
                        */
                            lyrics[i].text="";
                            lyrics[i].syllabic=Lyrics.SINGLE;
                        }
                    }
            }
            cursor.next();
        }
    }


    function pasteLyrics(s) {
        var skipTies=checkTie.checked;
        var track=(spinStaff.value-1)*4+spinVoice.value-1;
        var verse=spinVerse.value-1;
        var placement=comboPos.currentIndex;
        var spaceString="--SPACE--";
        var sinalefa=sinalefas[comboElision.currentIndex];
        //var s = textLily.text;
        var spaces=findSpaces(s);
        var linkDictionary = {};
        s=s.trim();
        s=s.replace(/" "/g,spaceString);
        s=s.replace(/\n/g,' ');
        s=s.replace(/\s\s+/g,' ');
        var list=s.split(" ");
        var index=0;
        var cursor=curScore.newCursor();
        cursor.track=track;
        cursor.rewind(Cursor.SCORE_START);
        var insideWord=false;
        var insideMelisma=false;
        var melismaBegin=null;
        var tickBegin=0;
        while(cursor.segment) {
            if (cursor.element.type==Element.CHORD) {
                var chord=cursor.element;
                var lyrics=chord.lyrics;
                var note=chord.notes[0];
                var tick=cursor.tick;
                // update the dictionary of links
                linkDictionary[tick]=spaces[index];

                // only works in Musescore version >= 3.3
                if (note.tieBack != null && skipTies) {
                    cursor.next(); continue;
                }
                var newLyric=newElement(Element.LYRICS);
                var w="";
                var syl=Lyrics.SINGLE;
                if (index<list.length) w=list[index];
                if (w==spaceString) w=" ";
                if (index+1<list.length && list[index+1]=="--") {
                    insideMelisma=false;
                    if (insideWord) syl=Lyrics.MIDDLE;
                    else {syl=Lyrics.BEGIN; insideWord=true; }
                    index++;
                }
                else if (index+1<list.length && list[index+1]=="__") {
                    syl=Lyrics.END;
                    insideWord=false;
                    insideMelisma=true;
                    melismaBegin=newLyric;
                    tickBegin=tick;
                    index++;
                }
                else if (w=="_" && insideWord) {
                    syl=Lyrics.MIDDLE;
                }
                else if (w=="_" && insideMelisma && checkExtender.checked) {
                    var ticks=tick-tickBegin;
                    melismaBegin.lyricTicks=fraction(ticks,division * 4); // division is a variable that holds how many ticks there are in a quarter note
                }
                else if (insideWord) {
                    syl=Lyrics.END; insideWord=false;
                }
                else if (insideMelisma) {
                    insideMelisma=false;
                }

                if (w=="_") w="";
                w=w.replace(/~/g,sinalefa);
                w=w.replace(/_/g," ");

                newLyric.text=w;
                newLyric.syllabic=syl;
                newLyric.verse=verse;
                newLyric.placement=placement;
                chord.add(newLyric);

                index++;
            }
            cursor.next();
        }
        linkDictionaries.push(linkDictionary);
    }

    function pasteGlobal() {
        var s=textLily.text;
        const re = /{[^}]*}/g;
        var match;
        var ss=[];
        linkOffsets=[];
        while ((match = re.exec(s)) !== null) {
            ss.push(match[0]);
            linkOffsets.push(match.index+1);
        }
        if (ss.length==0) {
            ss=["{" + s + "}"];
            linkOffsets=[0];
        }
        var startStaff=spinStaff.value;
        var endStaff=startStaff+ss.length-1;
        if (endStaff>curScore.ntracks/4) endStaff=curScore.ntracks/4;
        linkDictionaries=[];
        onScreenStaves=[];
        onScreenVoice=spinVoice.value;
        onScreenVerse=spinVerse.value;
        for (var i=startStaff; i<=endStaff; i++) {
            spinStaff.value=i;
            deleteLyrics();
            s=ss[i-startStaff];
            s=s.substring(1,s.length-1);
            pasteLyrics(s);
            onScreenStaves.push(i);
        }
        spinStaff.value=startStaff;
    }

    function ties2dots(track) {
        var cursor=curScore.newCursor();
        cursor.track=track;
        cursor.rewind(0);
        while(cursor.segment) {
            if(cursor.element.type!=Element.CHORD) {
                cursor.next();
                continue;
            }
            var note=cursor.element.notes[0];
            if (note.tieForward) {
                var dur1=cursor.element.duration.ticks;
                cursor.next();
                var note2=cursor.element.notes[0];
                var dur2=cursor.element.duration.ticks;
                if (dur1==2*dur2) {
                    curScore.selection.select(note2);
                    cmd("delete");
                    curScore.selection.select(note);
                    // It seems that the "default" duration
                    // is the one of the note just deleted, not the current one,
                    // for that reason I have to do inc-duration-dotted 3 times
                    cmd("inc-duration-dotted");
                    cmd("inc-duration-dotted");
                    cmd("inc-duration-dotted");
                }
            }
            cursor.next();
        }
        curScore.selection.clear();
    }



    /********** Functions on condensed format **********/

    function condenseLyrics(s) {

        // if already not in lilypond format do not do anything
        if (!s.match(/ __ | -- | _ /)) return s;

        s=s+" ";
        s=s.replace(/\n/g," \n");   // add a space at end of each line
        s=s.replace(/__/g,"");      // delete the mark of melisma
        s=s.replace(/[ \t]+/g," "); // collapse whitespace
        var ss=condenseHyphens(s);
        while(s!=ss) {
            s=ss;
            ss=condenseHyphens(s);
        }
        s=condenseExtenders(s);
        while(s!=ss) {
            s=ss;
            ss=condenseExtenders(s);
        }
        return s;
    }

    function condenseHyphens(s) {
        // convert a single block of type "He -- _ _ llo"
        // in a block of type "He---llo"
        var matched=s.match(/ -- (_ )*/);
        if (matched==null) return s;
        var m=matched[0];
        var start=s.indexOf(m);
        var end=start+m.length;
        m=m.replace(/ -- /,"-");
        m=m.replace(/_ /g,"-");
        return s.substring(0,start) + m + s.substring(end);
    }

    function condenseExtenders(s) {
        // convert a single bloc of type "World _ _ "
        // into a block of type "World  "
        var matched=s.match(/ (_ )+/);
        if (matched==null) return s;
        var m=matched[0];
        var start=s.indexOf(m);
        var end=start+m.length;
        m=m.replace(/_ /g," ");
        return s.substring(0,start) + m + s.substring(end);
    }

    function expandLyrics(s) {

        // if already in lilypond format do not do anything
        if (s.match(/ __ | -- | _ /)) return s;

        var prefix;
        if (checkExtender.checked) prefix=" __ ";
        else prefix=" ";

        // "   "  =>  " _ _ " or " __ _ _ "
        s=s.replace(/ /g," _ ");
        s=s.replace(/  /g," ");
        s=s.replace(/ _ ((_ )*)/g,prefix+"$1");
        s=s+" ";
        s=s.replace(/ __ ([^_])/g," $1");

        // he--llo  =>  he -- _ llo
        s=s.replace(/(-+)/g," -- $1");
        s=s.replace(/-/g,"_ ");
        s=s.replace(/ _ _  _ /g," -- ");
        s=s.trim();
        return s;
    }

}

