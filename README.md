# mppt (ESP32-C6)

Implémentation et outils de suivi du point de puissance maximale (MPPT) pour systèmes photovoltaïques, ciblant une plateforme embarquée ESP32-C6 (RISC-V, Wi‑Fi 6, BLE).

## Table des matières
- Présentation
- Matériel requis
- Prérequis (ESP-IDF)
- Installation
- Configuration du projet
- Build & Flash
- Utilisation
- Brochage (exemple)
- Tests
- Dépannage
- Roadmap
- Download prebuilt firmware
- Licence

## Présentation
Ce projet implémente des algorithmes MPPT (Perturb & Observe, Incremental Conductance, etc.) pour optimiser la puissance extraite de panneaux solaires. La cible principale est l’ESP32-C6 avec ESP-IDF.

## Matériel requis
- Carte ESP32-C6 (ex. ESP32-C6-DevKitC-1)
- Convertisseur DC-DC (buck/boost) pilotable (MOSFET + driver ou module de puissance)
- Capteurs :
  - Mesure tension (avec diviseur résistif)
  - Mesure courant (ex. shunt + amplificateur type INA219/INA226, ou ACS712)
- Alimentation de développement et panneau PV (ou source simulée)
- Câbles, breadboard/carte prototype

## Prérequis (ESP-IDF)
- ESP-IDF ≥ 5.1 (compatible ESP32-C6)
- Toolchain installée via `esp-idf` (Python, CMake, Ninja)
- `idf.py` accessible dans le shell

Installation ESP-IDF (résumé) :
```bash
# Linux/macOS
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
git checkout v5.1
./install.sh
. ./export.sh
```

Sous Windows, utiliser l’ESP-IDF Tools Installer.

## Installation
```bash
git clone https://github.com/atmani1-boop/mppt.git
cd mppt
# Exporter ESP-IDF si nécessaire
. $IDF_PATH/export.sh
```

## Configuration du projet
Configurer les paramètres via `menuconfig` :
```bash
idf.py menuconfig
```
Sections typiques :
- MPPT Configuration:
  - Algorithme: P&O / IncCond
  - Période d’échantillonnage (ms)
  - Pas de perturbation (duty step)
- Hardware Configuration:
  - Broches ADC tension/courant
  - Broche PWM pour commande du convertisseur
  - Fréquence PWM
- Logging:
  - Niveau de log (INFO/DEBUG)
  - Sortie série (baud)

Des fichiers de config peuvent être fournis dans `configs/` :
```ini
# configs/mppt.ini
algo = pno
sample_period_ms = 20
duty_step = 1
pwm_freq_hz = 50000
adc_v_pin = 1
adc_i_pin = 2
pwm_pin = 3
```

## Build & Flash
Brancher la carte (mode USB-JTAG/USB‑CDC si dispo sur C6).

```bash
idf.py set-target esp32c6
idf.py build
idf.py -p /dev/ttyACM0 flash monitor  # adapter le port (Windows: COM3)
```

Si le port n’est pas détecté :
```bash
idf.py -p /dev/ttyACM0 -b 115200 flash
idf.py -p /dev/ttyACM0 monitor
```

## Utilisation
Au démarrage, le firmware :
- Initialise ADC(s) pour mesurer V et I
- Configure le PWM pour piloter le convertisseur
- Exécute l’algorithme MPPT sélectionné
- Publie des logs sur le port série et (optionnel) via Wi‑Fi/BLE

Exemple de logs (série) :
```
[MPPT] algo=P&O, Ts=20ms, step=1, pwm=50kHz
[V=17.8V] [I=1.92A] [P=34.2W] duty=41%
[V=18.1V] [I=1.94A] [P=35.1W] duty=42%
...
```

Paramètres runtime (si implémentés) via `menuconfig` ou `Kconfig`:
- Changement d’algo
- Limites de tension/courant
- Filtres (moving average)

## Brochage (exemple ESP32-C6-DevKitC-1)
Note: Adapter selon ton schéma réel.
- PWM: GPIO3 (LEDC)
- ADC tension: ADC1 CH1 (GPIO1)
- ADC courant: ADC1 CH2 (GPIO2)
- UART TX/RX pour logs: par défaut USB‑CDC, sinon GPIO21/20
- Alimentation logique: 3.3V
- Masse commune avec capteurs et puissance

ATTENTION:
- Respecter les limites ADC (3.3V max). Utiliser diviseurs résistifs et isolation si nécessaire.
- Filtrer les mesures (RC/numérique) pour stabilité MPPT.
- Protéger la puissance (diodes, fusibles, marge thermique).

## Tests
- Unitaires (si disponibles) via `unity`/`idf_unit_test`
- Intégration: valider stabilité MPPT sous variations irradiance/température.
- Mesure d’ondulation et rendement.

## Dépannage
- Port série indisponible: vérifier drivers, câbles, permissions (`dialout` sous Linux).
- Watchdog reset: augmenter délais, vérifier tâches lourdes.
- Mesures incohérentes: recalibrer ADC, vérifier diviseurs/offsets.
- Instabilité MPPT: réduire `duty_step`, ajouter filtrage, ajuster période d’échantillonnage.

## Roadmap
- [ ] Implémentation complète IncCond avec anti‑oscillation
- [ ] Mesure de puissance moyenne et tracking adaptatif
- [ ] Interface web BLE/Wi‑Fi pour monitoring
- [ ] Logger CSV sur SPIFFS/Flash
- [ ] Calibration automatique ADC

## Download prebuilt firmware

### Automatic Builds
Each push to the `main` branch triggers an automated build via GitHub Actions, producing ready-to-flash firmware binaries for ESP32-C6. You can download these prebuilt artifacts without needing to install ESP-IDF or compile the project yourself.

### Artifacts Available
The build workflow generates the following files:
- **mppt.bin**: Main application binary
- **bootloader.bin**: ESP32-C6 bootloader
- **partition-table.bin**: Partition table configuration
- **flasher_args.json**: Flash addresses and arguments for automated flashing
- **FLASH_INSTRUCTIONS.txt**: Detailed flashing instructions

### How to Download
1. Navigate to the [Actions tab](https://github.com/atmani1-boop/mppt/actions) of this repository
2. Click on the most recent **"Build ESP32-C6 Firmware"** workflow run (look for the green checkmark ✓)
3. Scroll down to the **Artifacts** section at the bottom of the page
4. Click on **esp32c6-firmware** to download a ZIP file containing all binaries and instructions
5. Extract the ZIP file to a folder on your computer

### Flashing the Firmware

#### Option 1: Using idf.py (requires ESP-IDF)
If you have ESP-IDF installed:
```bash
# Connect your ESP32-C6 board via USB
idf.py -p /dev/ttyACM0 flash

# On Windows
idf.py -p COM3 flash

# Monitor output (optional)
idf.py -p /dev/ttyACM0 monitor
```

#### Option 2: Using esptool.py (standalone - recommended for end users)
If you don't have ESP-IDF installed, you can use esptool.py directly:

1. Install esptool:
```bash
pip install esptool
```

2. Flash the firmware:
```bash
esptool.py -p /dev/ttyACM0 -b 460800 --before default_reset --after hard_reset --chip esp32c6 write_flash --flash_mode dio --flash_size detect --flash_freq 80m 0x0 bootloader.bin 0x8000 partition-table.bin 0x10000 mppt.bin
```

Replace `/dev/ttyACM0` with your serial port:
- **Linux**: `/dev/ttyUSB0` or `/dev/ttyACM0`
- **macOS**: `/dev/cu.usbserial-*`
- **Windows**: `COM3`, `COM4`, etc.

3. Monitor serial output (optional):
```bash
python -m serial.tools.miniterm /dev/ttyACM0 115200
```
Press `Ctrl+]` to exit the monitor.

### Troubleshooting Flash Issues
- **Board not detected**: Install USB drivers for your ESP32-C6 board
- **Permission denied (Linux)**: Add your user to the `dialout` group:
  ```bash
  sudo usermod -a -G dialout $USER
  ```
  Then log out and log back in.
- **Flash failures**: Try a lower baud rate (e.g., `-b 115200`)
- **Cannot enter download mode**: Hold the BOOT button while connecting USB
- **Device not responding**: Press the RESET button after flashing

For detailed instructions, see the **FLASH_INSTRUCTIONS.txt** file included in the downloaded artifacts.

## Licence
Choisir et ajouter un fichier `LICENSE` (MIT recommandé).

## Auteurs
- atmani1-boop