import qtLoad from "./qtloader.js";

const MuImpl = {

    loadModule: async function(config) {
        const instance = await qtLoad({
            qt: {
                onLoaded: config.onLoaded,
                onExit: config.onExit,
                entryFunction: window.MuseScoreStudio_entry,
                containerElements: [config.screen],
            }
        });

        return instance;
    }
}

export default MuImpl;
