
\version "2.11.44"
% automatically converted from 12b-Tuplets-Finale.xml

\header {
    encodingsoftware = "Finale 2007 for Windows"
    tagline = "Finale 2007 for Windows"
    encodingdate = "2007-09-14"
    composer = "Reinhold Kainhofer"
    title = "Finale tuplet test"
    }

\layout {
    \context { \Score
        autoBeaming = ##f
        }
    }
PartPOneVoiceOne =  \relative c' {
    \clef "treble" \key c \major \time 30/4 | % 1
    \times 2/3  {
        c4 d4 e4 }
    \times 2/3  {
        f4 g4 a4 }
    \times 2/3  {
        b4 c4 d4 }
    \times 2/4  {
        e4 f4 g4 a4 }
    \times 1/4  {
        b4 c4 c4 b4 }
    \times 3/7  {
        a4 g4 f4 e4 d4 c4 b4 }
    \times 2/6  {
        a4 g4 f4 e4 d4 c4 }
    \bar "|." % 0
    }


% The score definition
\new Staff <<
    \context Staff << 
        \context Voice = "PartPOneVoiceOne" { \PartPOneVoiceOne }
        >>
    >>

