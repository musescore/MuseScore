import QtQuick 2.0
import MuseScore 3.0

MuseScore {
      menuPath: "Plugins.propList"
      description: "Demonstrates use of get, set, readProperties and writeProperties"
      version: "1.0"
      onRun: {
            console.log("Proplist Started")
            var score = curScore;
            var sig=newElement(Ms.TIMESIG);
            console.log("new sig = "+sig);
            sig.set("color","red");
            var list = sig.readProperties;  // All readable properties.
            console.log("new list = "+list)
            for(var i=0; i<list.length; i++) {
              console.log(list[i]+"="+sig.get(list[i]));
            }
            list = sig.writeProperties; // All writeable properties. May be identical to readProperties
            console.log("Writable properties: "+list);
            Qt.quit()
            }
      }
