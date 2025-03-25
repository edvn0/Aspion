FROM debian:testing AS builder

RUN apt-get update && apt-get install -y \
    autoconf \
    build-essential \
    ccache \
    cmake \
    git \
    libffi-dev \
    libssl-dev \
    ninja-build \
    python3 \
    python3-pip \
    python3-venv

ENV VIRTUAL_ENV=/opt/venv
RUN python3 -m venv "$VIRTUAL_ENV"
ENV PATH="$VIRTUAL_ENV/bin:$PATH"

RUN pip install --upgrade pip && pip install conan

COPY conan/profiles/linux-gcc-x86_64 /root/.conan2/profiles/linux-gcc-x86_64
COPY conanfile.py CMakeLists.txt /app/
WORKDIR /app

ENV CC="ccache gcc"
ENV CXX="ccache g++"
ENV CMAKE_BUILD_PARALLEL_LEVEL=6

RUN conan install . \
    --profile:host=linux-gcc-x86_64 \
    --profile:build=linux-gcc-x86_64 \
    --build=missing

COPY . .

RUN conan build . \
    --profile:host=linux-gcc-x86_64 \
    --profile:build=linux-gcc-x86_64 \
    --build=missing

FROM debian:testing AS runtime

RUN apt-get update && apt-get install -y \
    adduser \
    libssl3 \
    libstdc++6 \
    netcat-traditional && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

RUN addgroup --system aspion_group && adduser --system --ingroup aspion_group aspion_user
USER aspion_user

WORKDIR /app

COPY --from=builder /app/build/Release/Server /app/Server
COPY wait-for-it.sh /app/wait-for-it.sh

ENTRYPOINT ["/app/wait-for-it.sh", "rabbitmq:5672", "--", "/app/Server"]
