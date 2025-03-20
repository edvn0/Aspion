# Use Alpine as base for minimal image
FROM alpine:3.21.3 AS builder

# Install required dependencies including musl and g++
RUN apk add --no-cache \
    boost-dev \
    build-base \
    cmake \
    openssl-dev \
    git \
    g++ \
    libstdc++

# Clone and build external dependencies
WORKDIR /usr/local/include
RUN git clone --depth=1 https://github.com/bfgroup/Lyra.git lyra
RUN git clone --depth=1 https://github.com/CopernicaMarketingSoftware/AMQP-CPP.git amqp-cpp && \
    cmake -B amqp-cpp/build -S amqp-cpp -DCMAKE_BUILD_TYPE=Release -DAMQP-CPP_BUILD_SHARED=OFF -DAMQP-CPP_LINUX_TCP=ON && \
    cmake --build amqp-cpp/build --config Release --target install -j "$(nproc)"

# Build application with musl-gcc
WORKDIR /app
COPY . .
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DIS_DOCKER=ON && \
    cmake --build build --config Release --parallel "$(nproc)"

# Use a minimal runtime image
FROM alpine:3.21.3

# Install runtime dependencies
RUN apk add --no-cache \
    boost-system \
    boost-thread \
    openssl \
    libstdc++

# Set non-root user
RUN addgroup -S appgroup && adduser -S appuser -G appgroup
USER appuser

# Copy built application from builder stage
WORKDIR /app
COPY --from=builder /app/build/Server /app/Server

# Set entrypoint
ENTRYPOINT ["/app/Server"]
CMD ["-t", "20"]
