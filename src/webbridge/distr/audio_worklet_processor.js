
class MuseDriverProcessor extends AudioWorkletProcessor {

    constructor(options) {
        super(options);

        this.port.onmessage = this.onMessageFromMain.bind(this);

        this.buffer = [];
        this.channel_inited = false;
        this.audio_requested = false;

        this.debugLog("end of constructor MuseDriverProcessor")
    }

    init(data) {
        this.workerPort = data.workerPort
        this.workerPort.onmessage = this.onMessageFromWorker.bind(this);
    }

    sendToMain(data) {
        this.port.postMessage(data)
    }

    onMessageFromMain(event) {
        this.debugLog("pong from processor: " + JSON.stringify(event.data))

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
        //this.debugLog("[processor] from worker " + event.data.type)

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
        this.buffer.push(...msg.data);
        this.audio_requested = false;
    }

    process(inputs, outputs, parameters) {

        const output = outputs[0];

        let totalWriten = 0;
        for (let ci = 0; ci < output.length; ++ci) {
            let channel = output[ci];
            const samplesToWrite = Math.min(this.buffer.length, channel.length);
            for (let i = 0; i < samplesToWrite; i++) {
                channel[i] = this.buffer[i * 2 + ci];
            }

            totalWriten += samplesToWrite;
        }

        this.buffer = this.buffer.slice(totalWriten);

        if (this.buffer.length <= (totalWriten * 10)) {
            this.requestAudio()
        }

        return true; 
    }
}

registerProcessor('musedriver-processor', MuseDriverProcessor);