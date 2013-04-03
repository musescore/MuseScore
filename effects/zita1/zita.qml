import QtQuick 1.1

Rectangle {
    id: screen
    width: 640
    height: 75
    border.width: 2
    border.color: "white"
    radius: 5
    smooth: true

    signal valueChanged(string name, real val)

    function updateValues() {
        delay.updateValues();
        xover.updateValues();
        rtlow.updateValues();
        rtmid.updateValues();
        fdamp.updateValues();
        eq1fr.updateValues();
        eq1gn.updateValues();
        eq2fr.updateValues();
        eq2gn.updateValues();
        opmix.updateValues();
        }

    Row {
          Image {
              source: "qrc:/zita1/revsect.png"
              fillMode: Image.PreserveAspectCrop
              Rotary { id: delay; pid: "delay"; x:  30; y: 32 }
              Rotary { id: xover; pid: "xover"; x:  92; y: 17 }
              Rotary { id: rtlow; pid: "rtlow"; x: 147; y: 17 }
              Rotary { id: rtmid; pid: "rtmid"; x: 207; y: 17 }
              Rotary { id: fdamp; pid: "fdamp"; x: 267; y: 17 }
              }
          Image {
              source: "qrc:/zita1/eq1sect.png"
              fillMode: Image.PreserveAspectCrop
              Rotary { id: eq1fr; pid: "eq1fr"; x: 19; y: 32 }
              Rotary { id: eq1gn; pid: "eq1gn"; x: 68; y: 17 }
              }
          Image {
              source: "qrc:/zita1/eq2sect.png"
              fillMode: Image.PreserveAspectCrop
              Rotary { id: eq2fr; pid: "eq2fr"; x: 19; y: 32 }
              Rotary { id: eq2gn; pid: "eq2gn"; x: 68; y: 17 }
              }
          Image {
              source: "qrc:/zita1/mixsect.png"
              fillMode: Image.PreserveAspectCrop
              Rotary { id: opmix; pid: "opmix"; x: 23; y: 32 }
              }
          Image {
              source: "qrc:/zita1/redzita.png"
              fillMode: Image.PreserveAspectCrop
              }
          }
    }

