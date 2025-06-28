import qtLoad from "./qtloader.js";

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

const MuImpl = {

    loadModule: async function(config) {
        const instance = await qtLoad({
            qt: {
                onLoaded: config.onLoaded,
                onExit: config.onExit,
                entryFunction: window.MuseScoreStudio_entry, // from MuseScoreStudio.js
                containerElements: [config.screen],
            }
        });

        setupInternalCallbacks(instance)

        return instance;
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
}

export default MuImpl;
