import QtQuick 2.7

QtObject {

    id: root

    property var _resolver: null

    function initResolver() {

        var r = {}

        // AppShell
        r["musescore://home"] = function(d) { return {type: "dock" }};
        r["musescore://notation"] = function(d) { return {type: "dock" }};
        r["musescore://sequencer"] = function(d) { return {type: "dock" }};
        r["musescore://publish"] = function(d) { return {type: "dock" }};
        r["musescore://settings"] = function(d) { return {type: "dock" }};
        r["musescore://devtools"] = function(d) { return {type: "dock" }};

        // DevTools
        r["musescore://devtools/launcher/sample"] = function(d) { return {
                path: "DevTools/Launcher/SampleDialog.qml", params: {color: d.params.color}
            }};

        _resolver = r;
    }

    function resolvePage(data) {

        if (!_resolver) {
            initResolver();
        }

        var page;
        var builder = _resolver[data.uri]
        if (builder) {
            page = builder(data)
        }

        if (!page) {
            return null;
        }

        if (!page.type) {
            page.type = "popup"
        }

        if (!page.params) {
            page.params = {}
        }

        if (data.params.modal) {
            page.params["modal"] = data.params.modal
        }

        return page;
    }
}
