import subprocess
import statistics

def run_test(num_threads, total_requests, num_iterations):
    times = []

    for _ in range(num_iterations):
        result = subprocess.run(['./build/main', str(num_threads), str(total_requests)], capture_output=True, text=True)
        output = result.stdout.splitlines()[-1]  # Extract the time taken from the last line
        time_taken = int(output.split()[-2])  # Extract the time value
        times.append(time_taken)

    return times

def print_stats(times):
    print(f'Times: {times}')
    print(f'Average: {statistics.mean(times)} ms')
    print(f'Median: {statistics.median(times)} ms')

if __name__ == "__main__":
    num_threads = 2  # Set the desired number of threads
    total_requests = 10000  # Set the desired total number of requests
    num_iterations = 10  # Set the desired number of iterations

    times = run_test(num_threads, total_requests, num_iterations)
    print_stats(times)

