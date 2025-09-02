

console.log("===================Run muworker================")

let MuseAudio = {
    preRun: [],
    postRun: [],
    locateFile: function(path, prefix) { 
        var file = prefix + "../" + path;
        return file
    },
    setStatus: (function () {
        return function (text) {
            if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
            console.info(text);
        };
    })(),
    onLogMessage: function (type, error) {
      console.log("[worker]", type, error)
        // postMessage({
        //     type: "log",
        //     messageType: type,
        //     message: error
        // });
    },
  onRuntimeInitialized: function () {
    console.info("[worker] onRuntimeInitialized")
    try {
      MuseAudio.ccall('Init', '', ['number'], [42]);
    } catch (error) {
      MuseAudio.onLogMessage("ERROR", error)
    }
  }
}

function initialize(data)
{
  MuseAudio.mainPort = data.port;
  MuseAudio.mainPort.onmessage = function(event) {
    console.log("From Main:", event.data);
  }

  try {
    console.log("[worker]", "try importScripts:", data.options.museAudioUrl)
    importScripts(data.options.museAudioUrl);
  } catch (error) {
    console.error("ERROR:", error)
  }

    try {
    console.log("[worker]", "call entry: MuseAudio_entry")
    MuseAudio_entry(MuseAudio);
  } catch (error) {
    console.error("ERROR:", error)
  }
}

// from main
onmessage = function(event) {
  console.log("[worker]", JSON.stringify(event.data))
  if (event.data.type === "INITIALIZE_WORKER") {
    initialize(event.data)    
  }
};
