FROM ubuntu:18.04
LABEL maintainer="Krish <K.DeSouza@Outlook.com>"
WORKDIR /ckb-next
RUN apt-get update && apt-get install -y bash dpkg systemd git build-essential cmake libudev-dev qt5-default zlib1g-dev libappindicator-dev libpulse-dev libquazip5-dev
COPY . /ckb-next
ARG LC_ALL
ENV LC_ALL C
RUN mkdir -p /run/systemd/system
RUN rm -rf ./build && \
    cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DSAFE_INSTALL=OFF -DSAFE_UNINSTALL=OFF && \
    cmake --build build --target all -- -j 2 && \
    cd build && \
    cmake . && \
    make package
ENTRYPOINT ["sh",".linux/debian/DOCKER/entry_point.sh"]
