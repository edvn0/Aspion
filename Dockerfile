FROM alpine:3.21.3 AS builder

RUN apk add --no-cache \
    boost-dev \
    build-base \
    cmake \
    openssl-dev \
    git \
    g++ \
    libstdc++

RUN git clone --depth=1 https://github.com/CopernicaMarketingSoftware/AMQP-CPP.git amqp-cpp
WORKDIR amqp-cpp
RUN cmake -B build -S . -DCMAKE_INSTALL_PREFIX=/usr/local/ -DCMAKE_BUILD_TYPE=Release -D AMQP-CPP_BUILD_SHARED=OFF -D AMQP-CPP_LINUX_TCP=ON && \
    cmake --build build --config Release --target install -j "$(nproc)"

RUN git clone --depth=1 https://github.com/bfgroup/Lyra.git lyra
WORKDIR lyra
RUN cmake -B build -S . -DCMAKE_INSTALL_PREFIX=/usr/local/ -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --config Release --target install

WORKDIR /app
COPY . .
RUN cmake -B build -S . -D CMAKE_BUILD_TYPE=Release && \
    cmake --build build --config Release --parallel "$(nproc)"

FROM alpine:3.21.3

RUN apk add --no-cache \
    boost-system \
    boost-thread \
    openssl \
    libstdc++

RUN addgroup -S aspion_group && adduser -S aspion_user -G aspion_group
USER aspion_user

WORKDIR /app
COPY --from=builder /app/build/Server /app/Server

ENTRYPOINT ["/app/Server"]
CMD ["-t", "20"]
