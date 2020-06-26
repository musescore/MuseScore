import QtQuick 2.7
import MuseScore.Ui 1.0

Item {

    id: root

    property var topParent: null
    property var resolver: null
    property var provider: ui._launchProvider

    signal requestedDockPage(var uri)

    anchors.fill: parent

    Rectangle {
        width: 100
        height: 100
        color: "#440000"
    }

    Connections {
        target: root.provider

        onFireOpen: {
            console.log("onFireOpen: " + JSON.stringify(data.data()))

            var ret = {}
            var page = root.resolvePage(data.data())
            if (page.type === "dock") {

                root.requestedDockPage(data.value("uri"))
                ret = {errcode: 0}

            } else if (page.type === "popup") {

                var comp = Qt.createComponent("../../" + page.path);
                if (comp.status === Component.Ready) {

                    var obj = comp.createObject(root.topParent, page.params);
                    obj.objectID = root.provider.objectID(obj)
                    ret = (obj.ret && obj.ret.errcode) ? obj.ret : {errcode: 0}

                    obj.closed.connect(function() {
                        console.log("[qml] closed: " + obj.objectID + ", ret: " + JSON.stringify(obj.ret))
                        root.provider.onClose(obj.objectID, obj.ret ? obj.ret : {errcode: 0})
                        obj.destroy()
                    })

                    if (data.value("sync")) {
                        obj.exec()
                    } else {
                        obj.show()
                    }

                } else {
                    ret = {errcode: 1, text: comp.errorString()}
                }
            } else {
                ret = {errcode: 1, text: "not supported page type: " + page.type}
            }

            console.log("[qml] open ret: " + JSON.stringify(ret) + ", type: " + page.type)
            data.setValue("ret", ret)
            data.setValue("page_type", page.type)
        }
    }

    function resolvePage(data) {

        var page = resolver.resolvePage(data)
        return page;
    }
}
