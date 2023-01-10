FROM debian:bullseye
MAINTAINER Nikolay Korotkiy <nikolay.korotkiy@wirenboard.com>

ENV CORE_PATH=/Z-Uno-G2-Core
ENV ARM_GCC_PATH=/gcc
ENV ARM_GCC_VERSION=7.2.1
ENV LIBCLANG_PATH=/libclang
ENV PATH="${PATH}:/zme_make"

RUN apt-get update && apt-get install -y make unzip wget && \
    wget -O - http://rus.z-wave.me/files/z-uno/g2/tc/arm-none-eabi-gcc-7_2_4-linux64.tar.gz | tar -xz -C / && \
    wget -O - http://rus.z-wave.me/files/z-uno/g2/tc/libclang_11_0_1-linux64.tar.gz | tar -xz -C / && \
    wget -O - https://github.com/Z-Wave-Me/Z-Uno-G2-Core/archive/refs/tags/ZUNO_CORE_BETA_03_00_09_B16.tar.gz | tar -xz -C / && \
    mv /Z-Uno-G2-Core-ZUNO_CORE_BETA_03_00_09_B16 /Z-Uno-G2-Core && \
    unzip /Z-Uno-G2-Core/toolchain/linux64/zme_make_linux64.zip -d / && \
    rm -fr /Z-Uno-G2-Core/toolchain && \
    chmod 755 /zme_make/zme_make
