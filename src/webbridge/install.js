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

// Copy src files
copyFile(HERE+"/viewer/viewer.html", OUTPUT_DIR+"/MuseScoreStudio.html"); // replace origin MuseScoreStudio.html
copyFile(HERE+"/viewer/index.html", OUTPUT_DIR+"/index.html");
copyFile(HERE+"/viewer/run_server.sh", OUTPUT_DIR+"/run_server.sh");
copyFile(HERE+"/distr/muapi.js", OUTPUT_DIR+"/muapi.js");
