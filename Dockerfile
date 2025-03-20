FROM debian:testing AS builder

# Install basic build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    python3 \
    python3-pip \
    python3-venv \
    libssl-dev \
    libffi-dev


# Create virtualenv for Python and activate it
ENV VIRTUAL_ENV=/opt/venv
RUN python3 -m venv $VIRTUAL_ENV
ENV PATH="$VIRTUAL_ENV/bin:$PATH"

# Install Conan inside virtualenv
RUN pip install --upgrade pip && pip install conan

# Copy custom Conan profile
COPY conan/profiles/linux-gcc-x86_64 /root/.conan2/profiles/linux-gcc-x86_64

# Set workdir
WORKDIR /app

# Copy entire project including CMakePresets and conanfile
COPY . .

# Install dependencies with Conan
RUN conan install . \
    --profile:host=linux-gcc-x86_64 \
    --profile:build=linux-gcc-x86_64 \
    --build=missing

# Build the application with Ninja
RUN conan build .

# Final stage: runtime image
FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y \
    libstdc++6 \
    libssl3 && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

# Create non-root user
RUN addgroup --system aspion_group && adduser --system --ingroup aspion_group aspion_user
USER aspion_user

WORKDIR /app
COPY --from=builder /app/build/Server /app/Server

ENTRYPOINT ["/app/Server"]
