import requests
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from threading import Lock
url = 'http://localhost:8080/put'
total = 10000
max_workers = 10
latencies_lock = Lock()
latencies = []

session = requests.Session()


def send_request(i):
    key = f'key{i}'
    value = f'value{i}'
    start = time.time()
    try:
        session.post(url, json={'key': key, 'value': value})
        elapsed = (time.time() - start) * 1000
        with latencies_lock:
            latencies.append(elapsed)
    except Exception as e:
        elapsed = (time.time() - start) * 1000
        with latencies_lock:
            latencies.append(elapsed)
        return f"Exception for {key}: {e}"
    return None

with ThreadPoolExecutor(max_workers=max_workers) as executor:
    futures = [executor.submit(send_request, i) for i in range(total)]
    for future in as_completed(futures):
        result = future.result()
        if result:
            print(result)

if latencies:
    avg_latency = sum(latencies) / len(latencies)
    print(f"\nAverage latency: {avg_latency:.2f} ms")