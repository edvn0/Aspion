import requests
import multiprocessing
import threading
import time
import sys

# Configuration
HOST = "localhost"
PORT = 8080
PATH = "/"
NUM_REQUESTS = 100  # Number of requests per thread
NUM_THREADS = 10     # Number of threads per process
NUM_PROCESSES = 4    # Number of processes

# Target URL
URL = f"http://{HOST}:{PORT}{PATH}"


def send_request(thread_id):
    """Send a single request to the server and return the response status."""
    try:
        response = requests.get(URL)
        return response.status_code
    except requests.exceptions.RequestException as e:
        print(f"Thread {thread_id} - Request failed: {e}")
        return None


def worker(thread_id, num_requests):
    """Thread worker that sends multiple requests."""
    success = 0
    failure = 0
    for _ in range(num_requests):
        status = send_request(thread_id)
        if status == 200:
            success += 1
        else:
            failure += 1
    return success, failure


def process_worker(num_threads, num_requests):
    """Process worker that spawns threads for sending requests."""
    threads = []
    results = []

    for i in range(num_threads):
        thread = threading.Thread(target=lambda res, idx: res.append(worker(idx, num_requests)), args=(results, i))
        threads.append(thread)
        thread.start()

    for thread in threads:
        thread.join()

    # Aggregate results
    total_success = sum(r[0] for r in results)
    total_failure = sum(r[1] for r in results)
    return total_success, total_failure


def main():
    """Main function to handle multiprocessing."""
    start_time = time.time()
    with multiprocessing.Pool(NUM_PROCESSES) as pool:
        results = pool.starmap(process_worker, [(NUM_THREADS, NUM_REQUESTS)] * NUM_PROCESSES)

    # Aggregate results from all processes
    total_success = sum(r[0] for r in results)
    total_failure = sum(r[1] for r in results)

    end_time = time.time()
    duration = end_time - start_time

    print(f"\nBenchmark Completed 🚀")
    print(f"Total Requests Sent: {NUM_PROCESSES * NUM_THREADS * NUM_REQUESTS}")
    print(f"Successful Responses: {total_success}")
    print(f"Failed Requests: {total_failure}")
    print(f"Total Execution Time: {duration:.2f} seconds")
    print(f"Requests per Second: {int((NUM_PROCESSES * NUM_THREADS * NUM_REQUESTS) / duration)} RPS")


if __name__ == "__main__":
    main()
