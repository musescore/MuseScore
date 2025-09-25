import MuseAudio_entry from '../MuseAudio.js';

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
      debugLog(type + ": " + error)
    },
    inited: false,
    onRuntimeInitialized: function () {
        debugLog("onRuntimeInitialized MuseAudio: " + MuseAudio)
        try {
        //MuseAudio.ccall('Init', '', ['number'], [42]);
        MuseAudio._Init(42)
        MuseAudio.inited = true;
        globalThis.port.postMessage({type: "DRIVER_INITED" });
        } catch (error) {
        debugLog("init ERROR: " + error)
        }
    }
}

globalThis.self = {
    location: {
        href: ""
    }
}

class MuseDriverProcessor extends AudioWorkletProcessor {

    constructor(options) {
        super(options)

        this.port.onmessage = this.onMessageFromMain.bind(this);

        this.debugLog("end of constructor MuseDriverProcessor")

        globalThis.port = this.port;
        globalThis.debugLog = function(data) {
            globalThis.port.postMessage(data)
        }
    }

    init(data) {
        this.config = data.config;
        this.IS_USED_WORKER = this.config.MUSE_MODULE_AUDIO_WORKER == "ON";


        if (this.IS_USED_WORKER) {

            this.workerPort = data.rpcPort
            this.workerPort.onmessage = this.onMessageFromWorker.bind(this);

            this.workerBuffer = [];
            this.channel_inited = false;
            this.audio_requested = false;

        } else {

            // Rpc
            // main <=> worker
            MuseAudio.mainPort = data.rpcPort;
            MuseAudio.main_worker_rpcSend = function(data) {
                MuseAudio.mainPort.postMessage(data)
            }

            MuseAudio.main_worker_rpcListen = function(data) {} // will be overridden
            MuseAudio.mainPort.onmessage = function(event) {
                MuseAudio.main_worker_rpcListen(event.data)
            }

            // Init wasm
            try {
                MuseAudio_entry(MuseAudio);
            } catch (error) {
                this.debugLog("MuseAudio_entry ERROR:" + error)
            }

            // Buffer
            this.wasmBuffer = {
                size: 0,
                ptr: null
            }

        }
    }

    sendToMain(data) {
        this.port.postMessage(data)
    }

    onMessageFromMain(event) {
        if (event.data.type == "INITIALIZE_DRIVER") {
            this.init(event.data)
        }
    }

    debugLog(msg) {
        this.sendToMain({type: "debug", msg: msg})
    }

    sendToWorker(data) {
        this.workerPort.postMessage(data)
    }

    onMessageFromWorker(event) {
        if (event.data.type == "response_audio") {
            this.onResponseAudio(event.data)
        } else if (event.data.type == "channel_inited") {
            this.channel_inited = true;
        }
    }

    requestAudio() {
        if (!this.channel_inited) {
            return;
        }

        if (this.audio_requested) {
            return;
        }

        this.sendToWorker({type: "request_audio", samplesPerChannel: 1024})
        this.audio_requested = true;
    }

    onResponseAudio(msg) {
        this.workerBuffer.push(...msg.data);
        this.audio_requested = false;
    }

    process(inputs, outputs, parameters) {

        // Direct wasm, not worker
        if (!this.IS_USED_WORKER) {
            if (!MuseAudio.inited) {
                // Wait for the WASM module to be loaded.
                return true;
            }

            const output = outputs[0];
            const chCount = output.length;
            const samplesPerCh = output[0].length;
            const totalSamples = chCount * samplesPerCh;

            const bufferSize = totalSamples * 4 /*float*/
            if (this.wasmBuffer.size < bufferSize) {
                MuseAudio._free(this.wasmBuffer.ptr);
                this.wasmBuffer.ptr = MuseAudio._malloc(bufferSize);
                this.wasmBuffer.size = bufferSize;
            }

            MuseAudio._process(this.wasmBuffer.ptr, samplesPerCh);

            const view = new Float32Array(MuseAudio.HEAPU8.buffer, this.wasmBuffer.ptr, totalSamples);

            for (let ci = 0; ci < output.length; ++ci) {
                let channel = output[ci];
                for (let i = 0; i < channel.length; i++) {
                    let index = i * 2 + ci;
                    if (index < view.length) {
                        channel[i] = view[i * 2 + ci];
                    } else {
                        channel[i] = 0;
                    }
                }
            }
        } 
        // use worker
        else {
            const output = outputs[0];

            let totalWriten = 0;
            for (let ci = 0; ci < output.length; ++ci) {
                let channel = output[ci];
                for (let i = 0; i < channel.length; i++) {
                    let index = i * 2 + ci;
                    if (index < this.workerBuffer.length) {
                        channel[i] = this.workerBuffer[i * 2 + ci];
                    } else {
                        channel[i] = 0;
                    }
                }

                totalWriten += Math.min(this.workerBuffer.length, channel.length);
            }

            this.workerBuffer = this.workerBuffer.slice(totalWriten);

            if (this.workerBuffer.length <= (totalWriten * 10)) {
                this.requestAudio()
            }
        }

        return true; 
    }
}

registerProcessor('musedriver-processor', MuseDriverProcessor);