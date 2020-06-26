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

            var page = root.resolvePage(data.data())
            console.log("try open uri: " + data.value("uri") + ", page: " + JSON.stringify(page))
            if (!(page && (page.type === "dock" || page.type === "popup"))) {
                data.setValue("ret", {errcode: 101 }) // ResolveFailed
                return;
            }

            if (page.type === "dock") {
                root.requestedDockPage(data.value("uri"))
                root.provider.onOpen(page.type)
                data.setValue("ret", {errcode: 0 })
                return;
            }

            if (page.type === "popup") {

                var comp = Qt.createComponent("../../" + page.path);
                if (comp.status !== Component.Ready) {
                    console.log("[qml] failed create component: " + page.path + ", err: " + comp.errorString())
                    data.setValue("ret", {errcode: 102 }) // CreateFailed
                    return;
                }

                var obj = comp.createObject(root.topParent, page.params);
                obj.objectID = root.provider.objectID(obj)

                var ret = (obj.ret && obj.ret.errcode) ? obj.ret : {errcode: 0}
                data.setValue("ret", (obj.ret && obj.ret.errcode) ? obj.ret : {errcode: 0})
                data.setValue("objectID", obj.objectID)

                if (ret.errcode > 0) {
                    return;
                }

                obj.closed.connect(function() {
                    root.provider.onPopupClose(obj.objectID, obj.ret ? obj.ret : {errcode: 0})
                    obj.destroy()
                })

                root.provider.onOpen(page.type)

                if (data.value("sync")) {
                    obj.exec()
                } else {
                    obj.show()
                }
            }
        }
    }

    function resolvePage(data) {
        var page = resolver.resolvePage(data)
        return page;
    }
}
