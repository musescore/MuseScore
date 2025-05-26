const MuApi = {

    onclickTest1: function() {
        this.muwasm.ccall('onclickTest1', '', ['number'], [42])
    }

}

let createMuApi = (function(config, onInited) {

    MuApi.muwasm = config.muwasm
    return MuApi
})

export default createMuApi;