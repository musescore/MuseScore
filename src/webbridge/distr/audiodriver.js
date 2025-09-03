

let AudioDriver = (function () {

    let audioContext = null;
    let processor = null;

    var api = {
        
        setup: async function(workerPort, audio_context) {

            if (audio_context) {
                audioContext = audio_context
            } else {
                audioContext = new AudioContext()
            }

            try {

                await audioContext.audioWorklet.addModule('./distr/audio_worklet_processor.js')
                processor = new AudioWorkletNode(audioContext, 'musedriver-processor');
                console.log("[processor-main] create AudioWorkletNode for audio_worklet_processor")

            } catch (error) {
                console.error(error)
            }

            processor.port.onmessage = function(event) {
                console.log("[processor]", event.data)
            }

            processor.port.postMessage({
                type: 'INITIALIZE_DRIVER',
                workerPort: workerPort,
                options: {}
            }, [workerPort]); 
        },

        open: function() {
            console.log("[driver]", "open")
            processor.connect(audioContext.destination)
            audioContext.resume()
        }
    }

    return api;

})();

export default AudioDriver;