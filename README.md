# CAN application protocol (CANopen) over EtherCAT (CoE) debugger

## Build
Dependencies (Ubuntu 18.04): `libcap-dev libc-ares-dev libgcrypt20-dev libkrb5-dev flex bison qtbase5-dev qtmultimedia5-dev libqt5svg5-dev TODO`

```
mkdir ${HOME}/coe_debugging
cd ${HOME}/coe_debugging
git clone https://code.wireshark.org/review/wireshark.git
cd wireshark
git checkout master-2.4
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=$(pwd)/../install ..
make -j`nproc`
make install
cd ${HOME}/coe_debugging/wireshark
mkdir -p install/include/wireshark
find . -name "*.h" ! -path "*build*" ! -path "*install/include*" -exec cp --parents {} "install/include/wireshark" \;
cp build/config.h install/include/wireshark

cd ${HOME}/coe_debugging
git clone git@github.com:epl-viz/liblibwireshark.git  # TODO fork?
cd liblibwireshark
mkdir build
cd build
cmake -DWireshark_DIR=${HOME}/coe_debugging/wireshark/install/lib/wireshark -DCMAKE_INSTALL_PREFIX=$(pwd)/../install ..
make -j`nproc`
make install

cd ${HOME}/coe_debugging/liblibwireshark/install
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${HOME}/coe_debugging/wireshark/install/lib:${HOME}/coe_debugging/liblibwireshark/install/lib"
# Test if liblibwireshark examples are working
bin/simple_print ../samples/http.pcapng
bin/load_plugin -p ${HOME}/coe_debugging/wireshark/install/lib/wireshark/plugins/2.4.17 ../samples/

cd ${HOME}/coe_debugging
git clone THIS-REPOSITORY
cd coedbg
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=$(pwd)/../install -DLIBLIBWIRESHARK_DIR=${HOME}/coe_debugging/liblibwireshark/install -DWireshark_DIR=${HOME}/coe_debugging/wireshark/install/lib/wireshark ..
make -j`nproc`
make install

cd ${HOME}/coe_debugging/coedbg/build
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${HOME}/coe_debugging/wireshark/install/lib:${HOME}/coe_debugging/liblibwireshark/install/lib"
./coedbg --help
```
