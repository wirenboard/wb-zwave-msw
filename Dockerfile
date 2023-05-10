FROM debian:bullseye
MAINTAINER Nikolay Korotkiy <nikolay.korotkiy@wirenboard.com>

ENV CORE_PATH=/z-uno2_core_03_00_12_beta05
ENV ARM_GCC_PATH=/gcc
ENV ARM_GCC_VERSION=10.3.1
ENV LIBCLANG_PATH=/libclang
ENV PATH="${PATH}:/zme_make"

RUN apt-get update && apt-get install -y make unzip wget && \
    wget -qO - http://z-uno.z-wave.me/files/z-uno2/cores/z-uno2_core_03_00_12_beta05.zip -O tmp.zip && \
    unzip tmp.zip -d / && rm tmp.zip && \
    wget -qO - http://z-uno.z-wave.me/files/z-uno2/tc/arm-none-eabi-gcc-10_3_1-linux64.tar.gz | tar -xz -C / && \
    wget -qO - http://z-uno.z-wave.me/files/z-uno/g2/tc/libclang_11_0_1-linux64.tar.gz | tar -xz -C / && \
    wget -qO - http://z-uno.z-wave.me/files/z-uno2/tc/zme_make_00_04_02_beta03-linux64.tar.gz | tar -xz -C / && \
    chmod 755 /zme_make/zme_make
