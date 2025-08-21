# timkv

simple in-memory key–value storage

---


## Features

- In-memory key–value storage based on redis-like hashmap
- Configurable eviction: **LRU**, **LFU**
- Sharding 
- Simple HTTP API (`/get`, `/put`, `/delete`)

---


---

## Getting Started

### Clone the repo

```bash
git clone --recurse-submodules https://github.com/4domm/timkv.git
cd timkv
```

### Build with Makefile

```bash
make
```

### Or run with Makefile

```bash
make run <SHARD=0> <config.json>
```

