FROM alpine:3.21.3 AS builder

RUN apk add --no-cache \
    boost-dev \
    cmake \
    make \
    openssl-dev \
    git \
    g++ \
    libstdc++ \
    spdlog-dev


RUN git clone --depth=1 https://github.com/ninja-build/ninja.git ninja
WORKDIR ninja
COPY CMakePresets.json .
RUN cmake -B build -S . -D CMAKE_BUILD_TYPE=Release -D BUILD_TESTING=OFF && \
    cmake --build build -j"$(nproc)" && \
    cp build/ninja /usr/local/bin/ninja

RUN git clone --depth=1 https://github.com/CopernicaMarketingSoftware/AMQP-CPP.git amqp-cpp
WORKDIR amqp-cpp
COPY CMakePresets.json .
RUN cmake --preset docker && \
    cmake --build build --config Release --target install -j "$(nproc)"

RUN git clone --depth=1 https://github.com/bfgroup/Lyra.git lyra
WORKDIR lyra
COPY CMakePresets.json .
RUN cmake --preset docker && \
    cmake --build build --config Release --target install  -j "$(nproc)"

RUN git clone --depth=1 --branch v1.15.0 https://github.com/gabime/spdlog.git spdlog
WORKDIR spdlog
COPY CMakePresets.json .
RUN cmake --preset docker  && \
    cmake --build build --target install  -j "$(nproc)"

WORKDIR /app
COPY . .
COPY CMakePresets.json .
RUN cmake --preset docker && \
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
