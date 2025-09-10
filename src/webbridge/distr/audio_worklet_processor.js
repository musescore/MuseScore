
class MuseDriverProcessor extends AudioWorkletProcessor {

    constructor(options) {
        super(options);

        this.port.onmessage = this.onMessageFromMain.bind(this);

        this.buffer = [];

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
        //this.debugLog("received data: " + msg.data.length)
        try {
        this.buffer.push(...msg.data);
          } catch (e) {
       this.debugLog("process error: " + e.toString())
   }
    }

    process(inputs, outputs, parameters) {

        const output = outputs[0];

       // this.debugLog("process")

      //  try {

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

       // if (this.buffer.length < output[0].length * 2) {
            this.requestAudio()
        //}

   // } catch (e) {
   //     this.debugLog("process error: " + e.toString())
   // }

        return true; 
    }
}

registerProcessor('musedriver-processor', MuseDriverProcessor);