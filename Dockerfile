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
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DAMQP-CPP_BUILD_SHARED=OFF -DAMQP-CPP_LINUX_TCP=ON && \
    cmake --build build --config Release --target install -j "$(nproc)"

WORKDIR /usr/local/include
RUN git clone --depth=1 https://github.com/bfgroup/Lyra.git lyra

WORKDIR /app
COPY . .
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --config Release --parallel "$(nproc)"

FROM alpine:3.21.3

RUN apk add --no-cache \
    boost-system \
    boost-thread \
    openssl \
    libstdc++

RUN addgroup -S appgroup && adduser -S appuser -G appgroup
USER appuser

WORKDIR /app
COPY --from=builder /app/build/Server /app/Server

ENTRYPOINT ["/app/Server"]
CMD ["-t", "20"]
