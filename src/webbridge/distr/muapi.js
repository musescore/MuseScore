const MuApi = {

    onclickTest1: function() {
        this.Module.ccall('onclickTest1', '', ['number'], [42])
    },

    load: function(data) {
        console.log("[js] load len: ", data.length)
        const ptr = this.Module._malloc(data.length);
        this.Module.HEAPU8.set(data, ptr);
        this.Module._load(ptr, data.length);
        this.Module._free(ptr);
    }

}

let createMuApi = (function(config, onInited) {

    MuApi.Module = config.muwasm
    return MuApi
})

export default createMuApi;