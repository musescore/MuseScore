

// -j ./job.json --extension "musescore://extensions/dev/batch_example?action=processing"
function main() {
    api.log.info("called main of processing.js")

    var score = api.engraving.curScore

    score.addText("title", "New Title")
}
