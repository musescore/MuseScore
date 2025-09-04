
class MuseDriverProcessor extends AudioWorkletProcessor {

    constructor(options) {
        super(options);

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
        this.port.postMessage("pong from processor: " + JSON.stringify(event.data))
    }

}

registerProcessor('musedriver-processor', MuseDriverProcessor);