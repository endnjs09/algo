FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    g++ \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /sandbox

COPY TraceLogger.h .
COPY TraceLogger.cpp .
COPY shadow_include ./shadow_include

CMD ["bash"]