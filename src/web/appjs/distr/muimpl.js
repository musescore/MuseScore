import config from "./config.js";
import qtLoad from "./qtloader.js";
import AudioDriver from "./audiodriver.js";

function setupInternalCallbacks(Module) {

    // Interactive
    Module.openFileDialog = function(callback) {
        console.log("[js] openFileDialog")
        const input = document.createElement('input');
        input.type = 'file';
        input.onchange = (e) => {
            const file = e.target.files[0];
            const fileName = file.name
            const reader = new FileReader();
            reader.onload = (e) => {
                const contents = e.target.result;
                const uint8View = new Uint8Array(contents);
                console.log("[js] openFileDialog fileName: ", fileName, ", contents: ", uint8View.length, ", [0]=", uint8View[0])
                callback(fileName, uint8View);
            };
            reader.readAsArrayBuffer(file); 
        };
        input.click();
    }
}

function setupRpc(Module)
{
    // Main <=> Worker 
    // port1 - main
    // port2 - worker
    Module.main_worker_rpcChannel = new MessageChannel();

    Module.main_worker_rpcSend = function(data) {
        Module.main_worker_rpcChannel.port1.postMessage(data)
    }

    Module.main_worker_rpcListen = function(data) {} // will be overridden

    Module.main_worker_rpcChannel.port1.onmessage = function(event) {
        Module.main_worker_rpcListen(event.data)
    };

    // Worker <=> Driver (processor)
    // port1 - driver
    // port2 - worker
    Module.driver_worker_rpcChannel = new MessageChannel();
}

async function setupDriver(Module) 
{
    Module.driver = AudioDriver;

    AudioDriver.onInited = function() {
        console.log("driver on inited add sound font")
        Module.ccall('addSoundFont', '', ['string'], [Module.soundFont]);
    }

    if (config.MUSE_MODULE_AUDIO_WORKER == "ON") {
        await AudioDriver.setup(Module.config, Module.driver_worker_rpcChannel.port1);
    } else {
        await AudioDriver.setup(Module.config, Module.main_worker_rpcChannel.port2);
    }

}

async function setupWorker(Module)
{
    // Initialize the worker.
    Module.worker = new Worker("distr/audioworker.js")

    var museAudioUrl = new URL("MuseAudio.js", window.location) + "";

    Module.worker.onmessage = function(event) {
        if (event.data.type == "WORKER_INITED") {
            Module.ccall('addSoundFont', '', ['string'], [Module.soundFont]);
        }
    }

    Module.worker.postMessage({
    type: 'INITIALIZE_WORKER',
    mainPort: Module.main_worker_rpcChannel.port2,
    driverPort: Module.driver_worker_rpcChannel.port2,
    options: {
        museAudioUrl: museAudioUrl
    }
    }, [Module.main_worker_rpcChannel.port2, Module.driver_worker_rpcChannel.port2]);
}

const MuImpl = {

    Module: {},

    loadModule: async function(opt) {

        this.Module = {
            config: config, // static configuration

            qt: {
                onLoaded: opt.onLoaded,
                onExit: opt.onExit,
                entryFunction: window.MuseScoreStudio_entry, // from MuseScoreStudio.js
                containerElements: [opt.screen],
            },

            soundFont: opt.soundFont
        }

        setupRpc(this.Module);
        setupInternalCallbacks(this.Module);

        this.Module = await qtLoad(this.Module);

        return this.Module;
    },

    loadScoreFile: async function(file) {
        if (!file) {
            return
        }

        const buffer = await file.arrayBuffer();
        this.loadScoreData(new Uint8Array(buffer)) 
    },

    loadScoreData: function(data) {
        const ptr = this.Module._malloc(data.length);
        this.Module.HEAPU8.set(data, ptr);
        this.Module._load(ptr, data.length);
        this.Module._free(ptr);
    },

    startAudioProcessing: async function() {
        await setupDriver(this.Module);

        if (config.MUSE_MODULE_AUDIO_WORKER == "ON") {
            await setupWorker(this.Module);
        }

        this.Module._startAudioProcessing()
    }
}

export default MuImpl;
