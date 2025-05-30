# Distributed Cache

A simple key–value storage implemented in cpp(this cpp code is bad).

---

## Table of Contents

- [Features](#features)
- [Getting Started](#getting-started)
    - [Clone the repo](#clone-the-repo)
    - [Build with Docker](#build-with-docker)
    - [Run a Single Shard](#run-a-single-shard)
    - [Run Multiple Shards (Docker Compose)](#run-multiple-shards-docker-compose)

---

## Features

- In-memory key–value storage
- Configurable eviction: **LRU**, **LFU**, **2Q**, **ARC**, **Random**
- Sharding 
- Simple HTTP API (`/get`, `/put`, `/delete`, `/stat`)

---


---

## Getting Started

### Clone the repo

```bash
git clone https://github.com/4domm/distributed-cache.git
cd distributed-cache
```
## Build-with-docker
```bash
docker build -t distributed-cache:latest .
```
## Run-a-single-shard
```bash
docker run -it --rm -p 8080:8080 distributed-cache:latest  0 config.cfg
```
## Run-multiple-shards-docker-compose
```bash
docker-compose up --build
```
