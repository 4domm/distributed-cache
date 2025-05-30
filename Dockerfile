FROM ubuntu:22.04 AS builder

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends apt-transport-https ca-certificates gnupg wget software-properties-common build-essential libpoco-dev && rm -rf /var/lib/apt/lists/*

RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc | apt-key add - && apt-add-repository 'deb https://apt.kitware.com/ubuntu/ jammy main' && apt-get update && apt-get install -y --no-install-recommends cmake && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY CMakeLists.txt .
COPY main.cpp network.cpp network.h storage.h cache.h lru_cache.h lfu_cache.h arc_cache.h twoq_cache.h bench.cpp random_cache.h ./

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --target distributed-cache -- -j$(nproc)

FROM ubuntu:22.04

RUN apt-get update && apt-get install -y --no-install-recommends libpoco-dev && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/distributed-cache /app/distributed-cache
COPY config.cfg /app/config.cfg

RUN chmod +x /app/distributed-cache

ENTRYPOINT ["/app/distributed-cache"]
CMD ["1", "/app/config.cfg"]