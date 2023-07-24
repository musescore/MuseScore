package org.fluidsynth.fluidsynthtests

class TestRunner {
    companion object {
        external fun runTests()
        init {
            System.loadLibrary("native-lib")
        }
    }
}