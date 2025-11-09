/**
 * ESP32 Multi-Board Template Flasher - Application Logic
 */

// Board configuration
const BOARDS = {
  esp32_dev: {
    name: 'ESP32 DevKit V1',
    icon: '📟',
    description: 'Standard ESP32 development board'
  },
  esp32s3_dev: {
    name: 'ESP32-S3 DevKit',
    icon: '⚡',
    description: 'ESP32-S3 with USB OTG and more RAM'
  }
};

// Initialize when DOM is ready
document.addEventListener('DOMContentLoaded', () => {
  checkBrowserSupport();
  generateDeviceSelection();
});

// Check if browser supports Web Serial API
function checkBrowserSupport() {
  if (!navigator.serial) {
    const warning = document.getElementById('no-support-warning');
    if (warning) {
      warning.classList.remove('hidden');
    }
    console.warn('Web Serial API not supported');
  }
}

// Generate device selection UI
function generateDeviceSelection() {
  const container = document.getElementById('device-selection');
  const installButton = document.querySelector('esp-web-install-button');
  
  if (!container || !installButton) return;

  for (const [boardId, boardInfo] of Object.entries(BOARDS)) {
    const label = document.createElement('label');
    label.className = 'device-option';

    const input = document.createElement('input');
    input.type = 'radio';
    input.name = 'board';
    input.value = boardId;
    input.addEventListener('change', (e) => handleBoardSelection(e, installButton));

    const deviceLabel = document.createElement('span');
    deviceLabel.className = 'device-label';

    const icon = document.createElement('span');
    icon.className = 'device-icon';
    icon.textContent = boardInfo.icon;

    const nameSpan = document.createElement('span');
    nameSpan.className = 'device-name';
    nameSpan.textContent = boardInfo.name;

    const descSpan = document.createElement('span');
    descSpan.className = 'device-desc';
    descSpan.textContent = boardInfo.description;

    deviceLabel.appendChild(icon);
    deviceLabel.appendChild(nameSpan);
    if (boardInfo.description) {
      deviceLabel.appendChild(descSpan);
    }

    label.appendChild(input);
    label.appendChild(deviceLabel);
    container.appendChild(label);
  }

  // Select first board by default
  const firstInput = container.querySelector('input[type="radio"]');
  if (firstInput) {
    firstInput.checked = true;
    handleBoardSelection({ target: firstInput }, installButton);
  }
}

// Handle board selection
function handleBoardSelection(event, installButton) {
  const boardId = event.target.value;
  const manifestPath = `manifest_${boardId}.json`;

  if (installButton) {
    installButton.setAttribute('manifest', manifestPath);
    installButton.classList.remove('hidden');
  }

  console.log(`Selected board: ${boardId}, manifest: ${manifestPath}`);
}
