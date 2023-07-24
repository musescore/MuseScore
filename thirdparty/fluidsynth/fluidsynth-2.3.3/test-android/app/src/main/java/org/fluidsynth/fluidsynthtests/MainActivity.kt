package org.fluidsynth.fluidsynthtests

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import org.fluidsynth.fluidsynthtests.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        TestRunner.runTests()

        // Example of a call to a native method
        binding.sampleText.text = "If this shows up then there was no reported test failure!"
    }
}