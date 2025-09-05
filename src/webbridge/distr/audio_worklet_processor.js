
class MuseDriverProcessor extends AudioWorkletProcessor {

    constructor(options) {
        super(options);

        this.port.onmessage = this.onMessageFromMain.bind(this);

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
        }
    }

    requestAudio() {
        this.sendToWorker({type: "request_audio", samplesPerChannel: 1024})
    }

    onResponseAudio(msg) {
        let data = msg.data;
        this.debugLog("received data: " + data.length)
    }

    process(inputs, outputs, parameters) {

        this.requestAudio()

        const input = inputs[0];
        const output = outputs[0];

        for (let channel = 0; channel < input.length; ++channel) {
            for (let i = 0; i < input[channel].length; ++i) {
                output[channel][i] = input[channel][i] * 0.8; 
            }
        }
        return true; 
    }



}

registerProcessor('musedriver-processor', MuseDriverProcessor);