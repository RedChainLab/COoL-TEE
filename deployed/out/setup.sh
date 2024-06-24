UBUNTU_VERSION=focal
UBUNTU_NUM_VERSION=20.04
SDK_INSTALL_PATH_PREFIX=/opt/intel/
SDK_VERSION=2.24
SDK_SUBVERSION=100.3
sudo apt-get update && sudo apt-get install -y \
        build-essential \
        ocaml \
        ocamlbuild \
        automake \
        autoconf \
        libtool \
        wget \
        python-is-python3 \
        libssl-dev \
        git \
        cmake \
        perl \
    && \
    sudo apt-get install -y \
        libssl-dev \
        libcurl4-openssl-dev \
        protobuf-compiler \
        libprotobuf-dev \
        debhelper \
        cmake \
        reprepro \
        unzip \
        pkgconf \
        libboost-dev \
        libboost-system-dev \
        libboost-thread-dev \
        lsb-release \
        libsystemd0
### If you want to build it from sources
git clone https://github.com/intel/linux-sgx.git && cd linux-sgx && git checkout sgx_${SDK_VERSION}
make preparation
if [ ${UBUNTU_NUM_VERSION} = 20.04 ] 
then
	sudo cp external/toolset/ubuntu20.04/* /usr/local/bin && which ar as ld objcopy objdump ranlib
fi
make sdk_install_pkg
sudo ./linux/installer/bin/sgx_linux_x64_sdk_${SDK_VERSION}.${SDK_SUBVERSION}.bin --prefix ${SDK_INSTALL_PATH_PREFIX}

if [ ${UBUNTU_NUM_VERSION} = 20.04 ] 
then
echo deb [arch=amd64] https://download.01.org/intel-sgx/sgx_repo/ubuntu ${UBUNTU_VERSION} main | sudo tee /etc/apt/sources.list.d/intel-sgx.list \
&& wget -qO - https://download.01.org/intel-sgx/sgx_repo/ubuntu/intel-sgx-deb.key | sudo apt-key add
elif [ ${UBUNTU_NUM_VERSION} = 22.04 ] 
then
	echo 'deb [signed-by=/etc/apt/keyrings/intel-sgx-keyring.asc arch=amd64] https://download.01.org/intel-sgx/sgx_repo/ubuntu jammy main' | sudo tee /etc/apt/sources.list.d/intel-sgx.list
	sudo wget https://download.01.org/intel-sgx/sgx_repo/ubuntu/intel-sgx-deb.key 
	cat intel-sgx-deb.key | sudo tee /etc/apt/keyrings/intel-sgx-keyring.asc > /dev/null
fi
sudo apt-get update \
 && sudo apt-get install -y libsgx-epid libsgx-quote-ex libsgx-dcap-ql \
 && sudo apt-get install -y libsgx-urts-dbgsym libsgx-enclave-common-dbgsym libsgx-dcap-ql-dbgsym libsgx-dcap-default-qpl-dbgsym \
 && sudo apt-get install -y libsgx-*
make deb_psw_pkg DEBUG=1
sudo apt install ./linux/installer/deb/libsgx-urts/libsgx-urts_${SDK_VERSION}.${SDK_SUBVERSION}-${UBUNTU_VERSION}1_amd64.deb
source ${SDK_INSTALL_PATH_PREFIX}/sgxsdk/environment