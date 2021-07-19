# Building image
FROM ubuntu:20.04 as builder
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends -y \
  build-essential cmake git ca-certificates wget \
  python3 libpython3-dev python3-pip \
  libusb-1.0-0-dev libudev-dev \
  gcc-arm-linux-gnueabihf libc6-dev-armhf-cross && \
  rm -rf /var/cache/apt /var/lib/apt/lists/* /usr/share/doc

# Build speculos
RUN cd /opt/ && git clone --depth=1 https://github.com/LedgerHQ/speculos && \
  rm -rf speculos/.git && \
  cd speculos && mkdir build && cmake -Bbuild -H. && make -C build -j$(nproc)

# Install Python test requirements
COPY tests/requirements.txt /tmp
RUN pip3 install --user -r /tmp/requirements.txt

# Clone the Nano S SDK
RUN cd /opt && git clone https://github.com/LedgerHQ/nanos-secure-sdk --depth=1 --branch=2.0.0-1 && rm -rf /opt/nanos-secure-sdk/.git

# Clone the Nano X sdk. Also patch it because the clang version verification check is broken because ¯\_(ツ)_/¯...
COPY nanox_sdk.patch /tmp
RUN cd /opt && git clone https://github.com/LedgerHQ/nanox-secure-sdk --depth=1 && rm -rf /opt/nanox-secure-sdk/.git && \
    (cd nanox-secure-sdk && patch -p1 </tmp/nanox_sdk.patch) && rm /tmp/nanox_sdk.patch

# Download GCC ARM toolchain
RUN cd /opt && \
  wget 'https://developer.arm.com/-/media/Files/downloads/gnu-rm/10-2020q4/gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2?revision=ca0cbf9c-9de2-491c-ac48-898b5bbc0443&la=en&hash=68760A8AE66026BCF99F05AC017A6A50C6FD832A' -q -O gcc.tar.bz2 && \
  tar xf gcc.tar.bz2 && rm gcc.tar.bz2 && \
  ln -s /opt/gcc-arm-* /opt/gcc-arm-none-eabi

# Download Clang 7 to compile Nano X applications
RUN cd /opt && \
  wget 'https://releases.llvm.org/7.0.0/clang+llvm-7.0.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz' -q -O clang.tar.xz && \
  tar xf clang.tar.xz && rm clang.tar.xz && \
  ln -s /opt/clang+llvm-* /opt/clang

# Final image
FROM ubuntu:20.04

# Install packages
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends -y \
  build-essential g++ cmake gcc-multilib \
  qemu-user-static \
  python3-construct python3-jsonschema python3-mnemonic python3-pyelftools python3-flask-restful \
  libsodium-dev libhidapi-dev && \
  rm -rf /var/cache/apt /var/lib/apt/lists/* /usr/share/doc

# Copy speculos
COPY --from=builder /opt/speculos/ /opt/speculos

# Copy the Nano S SDK
COPY --from=builder /opt/nanos-secure-sdk/ /opt/nanos-secure-sdk

# Copy the Nano X SDK
COPY --from=builder /opt/nanox-secure-sdk/ /opt/nanox-secure-sdk

# Copy Python dependencies
COPY --from=builder  /root/.local /root/.local
ENV PATH=/root/.local/bin:$PATH

# Copy GCC ARM toolchain
COPY --from=builder /opt/gcc-arm-none-eabi /opt/gcc-arm-none-eabi

# Copy Clang 7
COPY --from=builder /opt/clang /opt/clang

ENV NANOS_SDK=/opt/nanos-secure-sdk
ENV NANOX_SDK=/opt/nanox-secure-sdk
ENV CLANGPATH=/opt/clang/bin/
ENV GCCPATH=/opt/gcc-arm-none-eabi/bin/
