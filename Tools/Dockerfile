FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV QT_VERSION=6.11.0
ENV QT_ARCH=linux_gcc_64
ENV QT_INSTALL=/opt/qt

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake ninja-build python3 python3-pip \
    git wget curl ca-certificates file \
    libgl1-mesa-dev libegl1-mesa-dev libxkbcommon-dev libxkbcommon-x11-dev \
    libxcb1-dev libxcb-cursor-dev libxcb-glx0-dev libxcb-icccm4-dev \
    libxcb-image0-dev libxcb-keysyms1-dev libxcb-randr0-dev \
    libxcb-render-util0-dev libxcb-render0-dev libxcb-shape0-dev \
    libxcb-shm0-dev libxcb-sync-dev libxcb-util-dev libxcb-xfixes0-dev \
    libxcb-xkb-dev libx11-xcb-dev libfontconfig1-dev libfreetype6-dev \
    libdbus-1-dev libdbus-1-3 \
    libxkbcommon-x11-0 libxcb-cursor0 libxcb-icccm4 libxcb-image0 \
    libxcb-keysyms1 libxcb-randr0 libxcb-render-util0 libxcb-shape0 \
    libxcb-sync1 libxcb-xfixes0 libxcb-xkb1 libfuse2 imagemagick \
    && rm -rf /var/lib/apt/lists/*

RUN pip3 install --no-cache-dir aqtinstall && \
    aqt install-qt linux desktop ${QT_VERSION} ${QT_ARCH} \
        -m qtcharts qtserialport \
        -O ${QT_INSTALL}

RUN cd /opt && \
    wget -q https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage && \
    wget -q https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage && \
    chmod +x linuxdeploy*.AppImage

ENV LD_LIBRARY_PATH=""
ENV PATH="${QT_INSTALL}/${QT_VERSION}/gcc_64/bin:/opt:${PATH}"
ENV LD_LIBRARY_PATH="${QT_INSTALL}/${QT_VERSION}/gcc_64/lib:${LD_LIBRARY_PATH}"
ENV APPIMAGE_EXTRACT_AND_RUN=1

WORKDIR /work

CMD ["sh", "-c", "sed -i 's/\\r$//' /work/Tools/build-in-container.sh && sh /work/Tools/build-in-container.sh"]
