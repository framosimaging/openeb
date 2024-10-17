&nbsp;
# About
Metavision OpenEB software installation instructions for EVS FRAMOS Sensor Module on the Nvidia Jetson platform.
After reading this document, the customer will be able to install FRAMOS Sensor Module Ecosystem Metavision OpenEB Software Package and its features. Furthermore, customer will be able to modify and rebuilt example applications, and use additional software in order to stream from an image sensor.

## Prerequisites

- Target System with installed FRAMOS Sensor Driver

- Target System with Internet access, required for automatic installation of the dependencies


## Clone, Install, Compile and Run

1. Clone the Framos git repository to the Home directory on the Jetson platform:
  ```
  cd ~
  
  git clone https://github.com/framosimaging/openeb.git
  ```
2. An automated installation script covers the installation of all required dependencies, compilation of Metavision OpenEB software and updating environment variables:
  ```
  cd ~/openeb/build

  ./install_build.sh
  ```
3. Run one of the examples:
   ```
   cd ~/openeb/build/bin
   
   ./metavision_player
   ./metavision_viewer
   ./metavision_hal_viewer
   ```

## Supported features

Plugin supported features are:

- Bias handling through v4l2 controls
- ROI handling through v4l2 controls
- Serial number discovery on EEPROM through v4l2 control
- File discovery
- Analytics information
    - Displays number of acquired, processed and dropped frames

## Known issues

1. Applications that require more processing power introduce latency in image stream. 
    - Number of events generated from an image sensor should be decreased by adjusting the biases correctly and setting ROI if possible. 
    - Application can be configured to optimize processing time.

