const fs = require('fs');

console.log("install js api step")

var args = process.argv.slice(2);
console.log("args:", args)
console.log("__dirname:", __dirname);

const HERE=__dirname
const OUTPUT_DIR = args.length > 0 ? args[0] : "./out"

function copyFile(src, dst) {
    try {
      fs.copyFileSync(src, dst);
      console.info("success: coped " + src + " => " + dst)
    } catch (err) {
      console.error("error: failed coped " + src + " => " + dst)
    }
}

// Remove Unnecessary Qt files
fs.rmSync(OUTPUT_DIR+"/MuseScoreStudio.html", {force: true})
fs.rmSync(OUTPUT_DIR+"/qtloader.js", {force: true})
fs.rmSync(OUTPUT_DIR+"/qtlogo.svg", {force: true})

// Copy api 
fs.mkdirSync(OUTPUT_DIR+"/distr", { recursive: true });
copyFile(HERE+"/distr/muapi.js", OUTPUT_DIR+"/distr/muapi.js");
copyFile(HERE+"/distr/muimpl.js", OUTPUT_DIR+"/distr/muimpl.js");
copyFile(HERE+"/distr/qtloader.js", OUTPUT_DIR+"/distr/qtloader.js");
copyFile(HERE+"/distr/audioworker.js", OUTPUT_DIR+"/distr/audioworker.js");
copyFile(HERE+"/distr/audiodriver.js", OUTPUT_DIR+"/distr/audiodriver.js"); 
copyFile(HERE+"/distr/audio_worklet_processor.js", OUTPUT_DIR+"/distr/audio_worklet_processor.js");

// Copy viewer
copyFile(HERE+"/viewer/viewer.html", OUTPUT_DIR+"/viewer.html");
copyFile(HERE+"/viewer/index.html", OUTPUT_DIR+"/index.html");
copyFile(HERE+"/viewer/index.html", OUTPUT_DIR+"/MuseScoreStudio.html"); 

// Copy tools
copyFile(HERE+"/viewer/run_server.sh", OUTPUT_DIR+"/run_server.sh");

