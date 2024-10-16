#!/bin/bash

# Save current path
CURRENT_PATH=$(pwd)

# Install software dependencies
sudo cp -R ../opencv/lib/* /usr/lib/.
sudo cp -R ../opencv/include/* /usr/local/include/.
sudo cp -R ../opencv/share/* /usr/local/share/.
sudo ldconfig

sudo apt update
sudo apt -y install apt-utils build-essential software-properties-common wget unzip curl git cmake
sudo apt -y install googletest libgtest-dev libboost-all-dev libusb-1.0-0-dev
sudo apt -y install libglew-dev libglfw3-dev libcanberra-gtk-module ffmpeg

# Install Python pip
sudo apt -y install python3-pip python3-distutils
python3 -m pip install pip --upgrade

# Install pybind
cd ~/Downloads
rm -Rf v2.6.0*
rm -Rf pybind11-2.6.0*
wget https://github.com/pybind/pybind11/archive/v2.6.0.zip
unzip v2.6.0.zip
cd ~/Downloads/pybind11-2.6.0
mkdir build && cd build
cmake .. -DPYBIND11_TEST=OFF
cmake --build .
sudo cmake --build . --target install

# Compile Metavision OpenEB
cd "$CURRENT_PATH"
cmake .. -DBUILD_TESTING=OFF
cmake --build . --config Release -- -j $(nproc)

# Update environment variables
echo "source $CURRENT_PATH/utils/scripts/setup_env.sh" >> ~/.bashrc
echo "export MV_HAL_PLUGIN_PATH=$CURRENT_PATH/lib/metavision/hal/plugins/framos_imx636" >> ~/.bashrc
