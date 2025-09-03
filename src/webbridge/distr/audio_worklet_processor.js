
class MuseDriverProcessor extends AudioWorkletProcessor {

    constructor(options) {
        super(options);

        console.log("[processor] constructor MuseDriverProcessor")

        this.port.onmessage = this.onMessageFromMain.bind(this);

        this.port.postMessage("end of constructor MuseDriverProcessor")

    }

    process(inputs, outputs, parameters) {

        const input = inputs[0];
        const output = outputs[0];

        for (let channel = 0; channel < input.length; ++channel) {
            for (let i = 0; i < input[channel].length; ++i) {
                output[channel][i] = input[channel][i] * 0.8; 
            }
        }
        return true; 
    }

    onMessageFromMain(event) {
        console.log("[processor]", "onMessageFromMain:", event.data);
        this.port.postMessage("pong from processor: " + JSON.stringify(event.data))
    }

}

registerProcessor('musedriver-processor', MuseDriverProcessor);
console.log("[processor] registerProcessor musedriver-processor")