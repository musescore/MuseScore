// Get references to the voice checkboxes
const voice1Checkbox = document.getElementById('voice1');
const voice2Checkbox = document.getElementById('voice2');
const voice3Checkbox = document.getElementById('voice3');
const voice4Checkbox = document.getElementById('voice4');

// Function to toggle the visibility of a voice
function toggleVoiceVisibility(voiceCheckbox) {
  voiceCheckbox.checked = !voiceCheckbox.checked;
  renderScore();
}

// Function to cycle through visible voices
function cycleVoices(direction) {
  const visibleVoices = [voice1Checkbox, voice2Checkbox, voice3Checkbox, voice4Checkbox].filter(checkbox => checkbox.checked);
  const currentIndex = visibleVoices.indexOf(visibleVoices.find(checkbox => checkbox.checked));
  const nextIndex = (currentIndex + direction + visibleVoices.length) % visibleVoices.length;

  visibleVoices.forEach(checkbox => checkbox.checked = false);
  visibleVoices[nextIndex].checked = true;

  renderScore();
}

// Event listeners for keyboard shortcuts
document.addEventListener('keydown', (event) => {
  if (event.ctrlKey && event.shiftKey) {
    switch (event.key) {
      case '1':
        toggleVoiceVisibility(voice1Checkbox);
        break;
      case '2':
        toggleVoiceVisibility(voice2Checkbox);
        break;
      case '3':
        toggleVoiceVisibility(voice3Checkbox);
        break;
      case '4':
        toggleVoiceVisibility(voice4Checkbox);
        break;
      case 'A':
        // Show or hide all voices
        voice1Checkbox.checked = !voice1Checkbox.checked;
        voice2Checkbox.checked = !voice2Checkbox.checked;
        voice3Checkbox.checked = !voice3Checkbox.checked;
        voice4Checkbox.checked = !voice4Checkbox.checked;
        renderScore();
        break;
      case 'S':
        // Show only the selected voice
        const selectedVoice = [voice1Checkbox, voice2Checkbox, voice3Checkbox, voice4Checkbox].find(checkbox => checkbox.checked);
        voice1Checkbox.checked = false;
        voice2Checkbox.checked = false;
        voice3Checkbox.checked = false;
        voice4Checkbox.checked = false;
        if (selectedVoice) selectedVoice.checked = true;
        renderScore();
        break;
      case 'Tab':
        // Cycle through visible voices
        if (event.shiftKey) {
          cycleVoices(-1); // Cycle backward
        } else {
          cycleVoices(1); // Cycle forward
        }
        break;
      // Add more cases for additional keyboard shortcuts
    }
  }
});