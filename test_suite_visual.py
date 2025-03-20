import requests
import multiprocessing
import threading
import time
import numpy as np
import matplotlib.pyplot as plt

# Configuration
HOST = "localhost"
PORT = 8080
PATH = "/"
NUM_REQUESTS = 100  # Number of requests per thread
NUM_THREADS = 10     # Number of threads per process
NUM_PROCESSES = 20    # Number of processes

# Target URL
URL = f"http://{HOST}:{PORT}{PATH}"


def send_request(thread_id, latencies):
    """Send a single request to the server and record the response time."""
    try:
        start = time.perf_counter()
        response = requests.get(URL)
        end = time.perf_counter()

        latency = (end - start) * 1000  # Convert to milliseconds
        latencies.append(latency)

        return response.status_code
    except requests.exceptions.RequestException as e:
        print(f"Thread {thread_id} - Request failed: {e}")
        return None


def worker(thread_id, num_requests, latencies):
    """Thread worker that sends multiple requests."""
    success = 0
    failure = 0
    for _ in range(num_requests):
        status = send_request(thread_id, latencies)
        if status == 200:
            success += 1
        else:
            failure += 1
    return success, failure


def process_worker(num_threads, num_requests):
    """Process worker that spawns threads for sending requests."""
    threads = []
    results = []
    thread_latencies = []  # Collect latencies per process

    for i in range(num_threads):
        thread = threading.Thread(target=lambda res, idx: res.append(worker(idx, num_requests, thread_latencies)),
                                  args=(results, i))
        threads.append(thread)
        thread.start()

    for thread in threads:
        thread.join()

    # Aggregate results
    total_success = sum(r[0] for r in results)
    total_failure = sum(r[1] for r in results)

    return total_success, total_failure, thread_latencies


def main():
    """Main function to handle multiprocessing."""
    manager = multiprocessing.Manager()
    latencies = manager.list()  # Shared list for response times

    start_time = time.time()
    with multiprocessing.Pool(NUM_PROCESSES) as pool:
        results = pool.starmap(process_worker, [(NUM_THREADS, NUM_REQUESTS)] * NUM_PROCESSES)

    # Aggregate results from all processes
    total_success = sum(r[0] for r in results)
    total_failure = sum(r[1] for r in results)

    # Collect all latencies from processes
    all_latencies = []
    for r in results:
        all_latencies.extend(r[2])

    end_time = time.time()
    duration = end_time - start_time

    # Convert latencies to NumPy array for analysis
    latencies = np.array(all_latencies)

    # Descriptive statistics
    mean_latency = np.mean(latencies)
    median_latency = np.median(latencies)
    percentile_95 = np.percentile(latencies, 95)
    percentile_99 = np.percentile(latencies, 99)
    min_latency = np.min(latencies)
    max_latency = np.max(latencies)

    total_requests = NUM_PROCESSES * NUM_THREADS * NUM_REQUESTS
    rps = total_requests / duration

    # Print summary
    print(f"\nBenchmark Completed 🚀")
    print(f"Total Requests Sent: {total_requests}")
    print(f"Successful Responses: {total_success}")
    print(f"Failed Requests: {total_failure}")
    print(f"Total Execution Time: {duration:.2f} seconds")
    print(f"Requests per Second (RPS): {int(rps)} RPS")
    print(f"\n📊 **Latency Statistics (in ms)**")
    print(f"Mean Latency: {mean_latency:.2f} ms")
    print(f"Median Latency: {median_latency:.2f} ms")
    print(f"95th Percentile: {percentile_95:.2f} ms")
    print(f"99th Percentile: {percentile_99:.2f} ms")
    print(f"Min Latency: {min_latency:.2f} ms")
    print(f"Max Latency: {max_latency:.2f} ms")

    # Generate visualizations
    visualize_results(latencies, rps, total_success, total_failure)


def visualize_results(latencies, rps, total_success, total_failure):
    """Generate matplotlib graphs for latency distribution and RPS."""
    fig, axes = plt.subplots(1, 2, figsize=(12, 5))

    # Latency Distribution
    axes[0].hist(latencies, bins=30, color='blue', alpha=0.7)
    axes[0].set_title("Latency Distribution")
    axes[0].set_xlabel("Response Time (ms)")
    axes[0].set_ylabel("Frequency")

    # Requests Per Second
    labels = ["Total Requests", "Successful", "Failed"]
    values = [NUM_PROCESSES * NUM_THREADS * NUM_REQUESTS, total_success, total_failure]
    axes[1].bar(labels, values, color=["blue", "green", "red"], alpha=0.7)
    axes[1].set_title("Request Success vs Failure")

    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    main()

