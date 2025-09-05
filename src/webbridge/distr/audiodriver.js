

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

            // driver (processor) -> main
            processor.port.onmessage = function(event) {
                console.log("[processor]", event.data)
            }

            processor.port.postMessage({
                type: 'INITIALIZE_DRIVER',
                workerPort: workerPort,
                options: {}
            }, [workerPort]); 
        },

        outputSpec: function() {
            return {
                sampleRate: audioContext.sampleRate,
                samplesPerChannel: Math.round(audioContext.baseLatency * audioContext.sampleRate),
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