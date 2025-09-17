import MuImpl from "./muimpl.js"

const DEFAULT_SOUNDFONT = "sound/MS%20Basic.sf3"

const MuApi = {

    // Load score
    loadScoreFile: MuImpl.loadScoreFile,
    loadScoreData: MuImpl.loadScoreData,

    // Start audio
    startAudioProcessing: MuImpl.startAudioProcessing.bind(MuImpl),
}

async function createMuApi(config) {

    if (!config.soundFont) {
        config.soundFont = window.location.origin + "/" + DEFAULT_SOUNDFONT
    }

    MuApi.Module = await MuImpl.loadModule(config)

    MuApi.Module.onProjectSaved = function(data) {
        console.log("[js muapi internal] onProjectSaved len: ", data.length)
        if (config.onProjectSaved) {
            config.onProjectSaved(data)
        }
    }

    return MuApi
}

export default createMuApi;

