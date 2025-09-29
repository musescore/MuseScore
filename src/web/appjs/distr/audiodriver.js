

async function URLFromFiles(files) {
    const promises = files.map(file =>
        fetch(file).then(response => response.text())
    );

    const texts = await Promise.all(promises);
    const text = texts.join("");
    const blob = new Blob([text], { type: "application/javascript" });
    return URL.createObjectURL(blob);
}

let AudioDriver = (function () {

    let audioContext = null;
    let processor = null;

    var api = {

        inited: false, 
        onInited: null,
        
        setup: async function(config, rpcPort, audio_context) {

            if (audio_context) {
                audioContext = audio_context
            } else {
                audioContext = new AudioContext()
            }

            try {

                const code = await URLFromFiles([
                    './MuseAudio.js',
                    './distr/audio_worklet_processor.js'
                ]);

                await audioContext.audioWorklet.addModule(code)
                processor = new AudioWorkletNode(audioContext, 'musedriver-processor', {
                                numberOfInputs: 0,
                                numberOfOutputs: 1,
                                outputChannelCount: [2],
                                });
                console.log("[processor-main] create AudioWorkletNode for audio_worklet_processor")

            } catch (error) {
                console.error(error)
            }

            // driver (processor) -> main
            processor.port.onmessage = function(event) {
                console.log("[processor]", event.data)

                if (event.data.type == "DRIVER_INITED") {
                    api.inited = true;
                    if (api.onInited) {
                        api.onInited();
                    }
                }
            }

            processor.port.postMessage({
                type: 'INITIALIZE_DRIVER',
                config: config,
                rpcPort: rpcPort,
                options: {}
            }, [rpcPort]); 
        },

        outputSpec: function() {
            return {
                sampleRate: audioContext.sampleRate,
                samplesPerChannel: Math.max(Math.round(audioContext.baseLatency * audioContext.sampleRate), 128),
                audioChannelCount: audioContext.destination.channelCount
            }
        },

        open: function() {
            console.log("[driver]", "open")
            processor.connect(audioContext.destination)
            audioContext.resume()
        },

        resume: function() {
            console.log("[driver]", "resume")
            audioContext.resume()
        },

        suspend: function() {
            console.log("[driver]", "suspend")
            audioContext.suspend();
        },

        close: function() {
            console.log("[driver]", "close")
            processor.disconnect()
        }
    }

    return api;

})();

export default AudioDriver;