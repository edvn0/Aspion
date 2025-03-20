FROM alpine:3.21.3

RUN apk add --no-cache \
    build-base \
    cmake \
    boost-dev \
    git

WORKDIR /usr/local/include
RUN git clone --depth=1 https://github.com/bfgroup/Lyra.git lyra

WORKDIR /app

COPY . .

RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DIS_DOCKER=ON && \
    cmake --build build --config Release --parallel 20 

ENTRYPOINT ["./build/Server", "-t", "20"]
