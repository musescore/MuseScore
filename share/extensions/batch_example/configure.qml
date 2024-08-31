import QtQuick 2.15

import MuseApi.Controls 1.0

Rectangle {

    id: root

    implicitHeight: 400
    implicitWidth: 400

    color: api.theme.backgroundPrimaryColor

    property string inputDir: "./input"
    property string outputDir: "./output"
    property string processingUri: "musescore://extensions/dev/batch_example?action=processing"

    Column {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        StyledTextLabel {
            text: "Batch example"
        }

        FlatButton {
            text: "Select input dir: " + root.inputDir
            onClicked: {
                root.inputDir = api.converter.selectDir("Input")
            }
        }

        FlatButton {
            text: "Select output dir: " + root.outputDir
            onClicked: {
                root.outputDir = api.converter.selectDir("Output")
            }
        }

        FlatButton {
            id: run
            text: "Run"
            onClicked: {
                var inputFiles = api.converter.scanDir(root.inputDir)

                var job = []
                for (var i in inputFiles) {
                    var file = inputFiles[i]
                    var task = {
                        "in": file,
                        "out": root.outputDir + "/" + api.converter.basename(file) + ".mscz"
                    }
                    job.push(task)
                }

                status.text = "processing " + inputFiles.length + " files..."

                Qt.callLater(run.processing, job)
            }

            function processing(job) {
                var json = JSON.stringify(job);
                api.log.info(json)
                var ok = api.converter.batch(root.outputDir, json, root.processingUri, run.progress)
                status.text = ok ? "success" : "failed"
            }

            function progress(current, total, info) {
                status.text = current + " / " + total + " " + info;
            }
        }

        StyledTextLabel {
            id: status
            text: ""

            onTextChanged: {
                api.log.info(status.text)
            }
        }
    }
}
