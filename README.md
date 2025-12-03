<!-- # At the very begainning of the README.MD, I gonna erratum statement
See more details in [Erratum_Statement.md](https://github.com/peihaoY/xSlice_Paper/Erratum_Statement.md) -->

# xSlice: Near-Real-Time Resource Slicing for QoS Optimization in 5G
xSlice is an online learning algorithm designed for the Near-Real-Time (Near-RT) RAN Intelligent Controller (RIC) in 5G O-RANs. Its primary goal is to dynamically adjust Medium Access Control (MAC)-layer resource allocation in response to changing network conditions such as wireless channel fluctuations, user mobility, traffic variations, and demand shifts.

To achieve this, xSlice formulates the Quality-of-Service (QoS) optimization as a regret minimization problem, where it accounts for throughput, latency, and reliability requirements of traffic sessions. It uses a Deep Reinforcement Learning (DRL) framework with an actor-critic model, combining value-based and policy-based learning methods. Additionally, a Graph Convolutional Network (GCN) is integrated within the DRL framework to handle dynamic traffic session numbers by embedding RAN data as graphs.

xSlice has been implemented on an O-RAN testbed with 10 smartphones. The results from extensive experiments show that xSlice reduces performance regret by 67% compared to existing state-of-the-art solutions, demonstrating its effectiveness in optimizing resource allocation and improving network performance in realistic scenarios.


## Getting Started
### Minimum hardware requirements:
- Laptop/Desktop/Server for OAI CN5G and OAI gNB
    - Operating System: Ubuntu 22.04 LTS
    - CPU: 12 cores x86_64 @ 3.5 GHz
    - RAM: 32 GB
- [USRP N300](https://www.ettus.com/all-products/USRP-N300/) or [USRP X300](https://www.ettus.com/all-products/x300-kit/) or [USRP B210](https://www.ettus.com/all-products/ub210-kit/)

### Software reference:
Our system consists of the configuration of RIC + xApp, 5G O-RAN, and 5G Core. xSlice has developed its own architecture and algorithms based on [Flexric](https://gitlab.eurecom.fr/mosaic5g/flexric), [OAI cn5g](https://gitlab.eurecom.fr/oai/cn5g), and [openairinterface5G](https://gitlab.eurecom.fr/oai/openairinterface5g).

## Dependencies and Code Clone

## 1. RIC + xApp Setup (https://github.com/peihaoY/xSlice_Paper)

### 1.1 python conv requirement
1. First, ensure that Python and pip are installed on the target computer. You can check this by running the following commands:
```bash
python --version  # or python3 --version
pip --version
```
If Python is not installed, please download and install it from the official Python website.

2. In the target directory, create and activate a new virtual environment and install dependencies. 
```bash
python -m venv myenv  # or python3 -m venv myenv
# Activate the virtual environment:
source myenv/bin/activate
```
Copy the requirements.txt file to your target computer's directory. Then, install all the dependencies using the following command:
```bash
pip install -r requirements.txt
#verify that all dependencies are correctly installed by running:
pip freeze
```


### 1.2 RIC prerequisites

Please find the CMAKE and SWIG dependencies on the [Flexric website](https://gitlab.eurecom.fr/mosaic5g/flexric).

<!-- - A *recent* CMake (at least v3.22). 

  On Ubuntu, you might want to use [this PPA](https://apt.kitware.com/) to install an up-to-date version.

- SWIG (at least  v.4.1). 

  We use SWIG as an interface generator to enable the multi-language feature (i.e., C/C++ and Python) for the xApps. Please, check your SWIG version (i.e, `swig
  -version`) and install it from scratch if necessary as described here: https://swig.org/svn.html or via the code below: 
  
  ```bash
  git clone https://github.com/swig/swig.git
  cd swig
  git checkout release-4.1
  ./autogen.sh
  ./configure --prefix=/usr/
  make -j8
  make install
  ```

- GCC (gcc-10, gcc-12, or gcc-13)

  gcc-11 is not currently supported.

- Other required dependencies. 

```bash
sudo apt install libsctp-dev python3.8 cmake-curses-gui libpcre2-dev python3-dev
``` -->

Note: - GCC (gcc-10)      *gcc-11 is not currently supported.

### 1.3 Clone the RIC +xApp code, build and install it. 

* Clone code and make install
```bash
# Get xSlice_xApp source code
git clone https://github.com/peihaoY/xSlice_Paper ~/xSlice_xApp  
# Build RIC
cd xSlice_xApp && mkdir build && cd build && cmake .. && make -j8 
# You can install the Service Models (SM) in your computer via:
sudo make install
```

## 2. O-RAN/gNB Setup  (https://github.com/peihaoY/xSlice_gNB)

### 2.1 5G Core Setup

Please install and configure OAI CN5G as described here: [OAI 5G NR CN tutorial](https://gitlab.eurecom.fr/oai/openairinterface5g/-/blob/develop/doc/NR_SA_Tutorial_OAI_CN5G.md)




### 2.2 Pre-requisites

Please find the UHD and other requirements on the [OAI gNB tutorial](https://gitlab.eurecom.fr/oai/openairinterface5g/-/blob/develop/doc/NR_SA_Tutorial_COTS_UE.md).

 <!-- Build UHD from source
```bash
sudo apt install -y autoconf automake build-essential ccache cmake cpufrequtils doxygen ethtool g++ git inetutils-tools libboost-all-dev libncurses-dev libusb-1.0-0 libusb-1.0-0-dev libusb-dev python3-dev python3-mako python3-numpy python3-requests python3-scipy python3-setuptools python3-ruamel.yaml

git clone https://github.com/EttusResearch/uhd.git ~/uhd
cd ~/uhd
git checkout v4.7.0.0
cd host
mkdir build
cd build
cmake ../
make -j $(nproc)
make test # This step is optional
sudo make install
sudo ldconfig
sudo uhd_images_downloader
``` -->
### 2.3 Install and Build xSlice_gNB

```bash
# Get xSlice_ORAN source code
git clone https://github.com/peihaoY/xSlice_ORAN.git ~/xSlice_ORAN
cd ~/xSlice_ORAN

# Install OAI dependencies and build ORAN
cd ~/xSlice_ORAN/cmake_targets
./build_oai -I
./build_oai -w USRP --ninja --build-e2 --gNB -C
```

- Smartphone set up
The COTS UE can now search for the network. You can find how to connect UE to gNB on [srsRAN website](https://docs.srsran.com/projects/project/en/latest/tutorials/source/cotsUE/source/index.html).

## Modified code structure
In both the RAN and RIC systems, there are numerous code files involved. Below, I have listed the files that I modified or added as part of implementing xSlice. The structure is as follows. For detailed comments and further information, please refer directly to the code.

### 1. xSlice_xApp

```bash
.
├── GNNRL                            # source code for DRL algrithom
│   ├── env.py              
│   ├── gcn.py                 
│   ├── multiarm.py                 # test code for multiarm algrithom 
│   └── ppo.py                 
├── examples                        # xApp for RAN Slicing and information monitor
│   ├── emulator            
│   ├── ric                         # RIC including E2 interface
│   └── xApp                        # source code for xApps
    │   ├── c                       # our xApps based on C code
        │   ├── ctrl            
│   ├── mac_ctrl.c                  # test code for mac layer control
            │   ├── xapp_peihao.c   # Source code for xSlice
    │   ├── python3                 # external python code examples for xApps
├── trandata                        # data storage
│   ├── KPM_UE.txt             
│   ├── slice_ctrl.bin         
│   ├── xapp_db_               
│   ├── kpm.py                      # show KPM data
│   ├── mac.py                      # show MAC-Layer data
│   └── rewards.csv            
```

### 2. xSlice_gNB

```bash
.
├── openair2                     
│   ├── E2AP                                    # source code for E2 interface
    │   ├── RAN_FUNCTION                   
        │   ├── CUSTOMIZED                      # monitor functions
            │   ├── ran_func_mac.c         
            │   └── ran_func_kpm.c         
        │   └── O-RAN                           # control service functions
            │   ├── rc_ctrl_service_style_2.c   # xSlice 
│   ├── LAYER2                                  # MAC layer funtions
    │   ├── NR_MAC_gNB                
        │   ├── slicing                         # slicing in nr mac scheduler
            │   ├── nr_slicingc.c              
        │   ├── gNB_scheduler_dlsch.c           # source code for downlink mac scheduler
```
### 3. Extend xSlice
If you wish to extend 'xSlice', please review the modification sections and comments in the code above. These will guide you through quickly getting started with implementing your own online learning algorithm in a new xApp.

## Run xSlice
## 1. On 5G Core server:
```bash
cd ~/oai-cn5g
docker compose up -d
```
## 2. On xSlice_gNB server:

### 2.1 Run xSlice_gNB 
If you are using the USRP N310, please directly install and run the following command. If you are using a different model of USRP, please modify your device address accordingly.
```bash
cd ~/xSlice_gNB
sudo cmake_targets/ran_build/build/nr-softmodem -O gnb.sa.band78.fr1.106PRB.2x2.usrpn300.conf --sa --usrp-tx-thread-config 1
```


### 2.2 Check UEs' successfully connected and generate demand:
You can use the following commands to check the 5G Core's AMF, UPF, and other components.
```bash
docker logs oai-amf -f
docker logs oai-upf -f
```

Check UEs' information:

<!-- Connection #0 to host 192.168.70.133 left intact
[2024-09-07 19:34:15.134] [amf_sbt] [info] Get response with HTTP code (200)
[2024-09-07 19:34:15.134] [amf_sbt] [info] Response body {"upCnxState": "DEACTIVATED"}
[2024-09-07 19:34:15.134] [amf_app] [debug] Parsing the message with the Simple Parser
[2024-09-07 19:34:15.134] [amf_sbt] [info] JSON part {"upCnxState": "DEACTIVATED"}
[2024-09-07 19:34:15.134] [amf_sbt] [debug] UP Deactivation
[2024-09-07 19:34:15.134] [amf_app] [debug] Trigger process response: Set promise with ID 37 to ready
[2024-09-07 19:34:15.134] [amf_server] [debug] Got result for PDU Session Id 5
[2024-09-07 19:34:15.134] [amf_n2] [debug] Removed UE NGAP context with amf_ue_ngap_id 30
[2024-09-07 19:34:15.134] [amf_n2] [debug] Removed UE NGAP context with ran_ue_ngap_id 6, gnb_id 57344
[2024-09-07 19:34:15.134] [ngap] [debug] Free NGAP Message PDU -->

```bash
[2024-09-07 19:34:15.902] [amf_app] [info]

|----------------------gNBs Information:--------------|
| Index | Status    | Global Id | gNB Name | PLMN     |
|-------|-----------|-----------|----------|----------|
| 1     | Connected | 0xE000    | gNB-OAI  | 001,01   |
|-----------------------------------------------------|


|-----------------------------------------------------------UEs Information:-------------------------------------------
| Index | 5GMM State       | IMSI            | GUTI              | RAN UE NGAP ID | AMF UE NGAP ID | PLMN   | Cell Id   |
|-------|------------------|-----------------|-------------------|----------------|----------------|--------|-----------|
| 1     | 5GMM-REGISTERED  | 001010000000001 | 00101010041000003 | 0x03           | 0x1F           | 001,01 | 0xE00000  |
| 2     | 5GMM-REGISTERED  | 001010000000002 | 00101010041000001 | 0x01           | 0x01           | 001,01 | 0xE00000  |
| 3     | 5GMM-REGISTERED  | 001010000000003 | 00101010041000006 | 0x06           | 0x06           | 001,01 | 0xE00000  |
| 4     | 5GMM-REGISTERED  | 001010000000004 | 00101010041000011 | 0x18           | 0x18           | 001,01 | 0xE00000  |
| 5     | 5GMM-REGISTERED  | 001010000000005 | 00101010041000012 | 0x14           | 0x14           | 001,01 | 0xE00000  |
| 6     | 5GMM-REGISTERED  | 001010000000006 | 00101010041000010 | 0x04           | 0x04           | 001,01 | 0xE00000  |
| 7     | 5GMM-REGISTERED  | 001010000000007 | 00101010041000004 | 0x0A           | 0x0A           | 001,01 | 0xE00000  |
| 8     | 5GMM-REGISTERED  | 001010000000008 | 00101010041000002 | 0x02           | 0x02           | 001,01 | 0xE00000  |
| 9     | 5GMM-REGISTERED  | 001010000000009 | 00101010041000009 | 0x0A           | 0x0A           | 001,01 | 0xE00000  |
| 10    | 5GMM-REGISTERED  | 001010000000010 | 00101010041000007 | 0x07           | 0x07           | 001,01 | 0xE00000  |
------------------------------------------------------------------------------------------------------
```
<!-- [2024-09-07 19:34:16.077] [sctp] [info] [Assoc_id 6, Socket 9] Received a message (length 147) from port 50209, on stream 1, PPID 60
[2024-09-07 19:34:16.077] [ngap] [debug] Handling SCTP payload from SCTP Server on assoc_id (6), stream_id (1), instreams (2), outstreams (2)
[2024-09-07 19:34:16.077] [ngap] [debug] Decoded NGAP message, procedure code 15, present 1 -->
You can generate traffic by accessing websites, streaming videos, downloading content, and more through the UE. Additionally, you can create traffic demands using iperf.

Run Iperf on UE to generate demand:
```bash
docker exec -it oai-ext-dn bash
iperf -u -t 86400 -i 1 -fk -B 192.168.70.135 -b 10M -c 10.0.0.2
```
## 3. On RIC+xApp server:
### 3.1 Start the nearRT-RIC
```bash
./xSlice_xApp/build/examples/ric/nearRT-RIC
```

### 3.2 Start xApps

Start the xSlicer xApp

```bash
./xSlice_xApp/build/examples/xApp/c/ctrl/xapp_peihao
```
### 3.3 Start testing our algrithom
```bash
python /xSlice_xApp/GNNRL/PPO.py    # or python3 /xSlice_xApp/GNNRL/PPO.py
```

* To evaluate xSlice in your host:
<!-- Warning: Ignoring XDG_SESSION_TYPE=wayland on Gnome. Use QT_QPA_PLATFORM=wayland to run on Wayland.
/home/phyan/flexric/GNNRL/rlnostate.py:76: UserWarning: Creating a tensor from a list of numpy.ndarrays is slow. Please consider converting the list to a single numpy.ndarray with numpy.array() before converting to a tensor. -->
```bash
phyan@inss-002:~/xSlice_Paper/GNNRL$ python3 ppo.py

# of episode :10, avg score : 774.6, optimization step: 0
Triggered internally at ../torch/csrc/utils/tensor_new.cpp:2.
  mini_batch = torch.tensor(s_batch, dtype=torch.float), torch.tensor(a_batch), torch.tensor(r_batch), torch.tensor(sp_batch), torch.tensor(done_batch)

# of episode :20, avg score : 953.6, optimization step: 100
# of episode :30, avg score : 1353.9, optimization step: 100
# of episode :40, avg score : 1574.1, optimization step: 200
# of episode :50, avg score : 2007.1, optimization step: 200
# of episode :60, avg score : 2076.8, optimization step: 300
# of episode :70, avg score : 2143.2, optimization step: 300
# of episode :80, avg score : 2181.3, optimization step: 400
```

* Results will be saved in floder [trandata]
    - trandata/ `KPM_UE.txt`, `slice_ctrl.bin`, `xapp_db_`  
    Please note that xapp_db_ is a compressed file. Be sure to extract it first before reviewing the contents.
    - trandata/ `rewards.csv` 

## Citation
If you use our code in your research, please cite our paper:
```bash
@ARTICLE{11244863,
  author={Yan, Peihao and Lu, Jie and Zeng, Huacheng and Thomas Hou, Y.},
  journal={IEEE Transactions on Networking}, 
  title={Near-Real-Time Resource Slicing for QoS Optimization in 5G O-RAN Using Deep Reinforcement Learning}, 
  year={2025},
  volume={},
  number={},
  pages={1-16},
  keywords={Open RAN;Quality of service;Resource management;5G mobile communication;Optimization;Dynamic scheduling;Throughput;Network slicing;Heuristic algorithms;Cellular networks;5G/6G;O-RAN;cellular networks;graph neural network (GNN);network slicing;deep reinforcement learning (DRL);QoS optimization},
  doi={10.1109/TON.2025.3628209}}
```

## Getting help

If you encounter a bug or have any questions regarding the paper, the code or the setup process, please feel free to contact us: phyan@msu.edu



