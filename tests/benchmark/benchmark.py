import subprocess
import statistics
import sys 

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
    if len(sys.argv) != 4:
        print("Usage: benchmark.py <num_threads> <total_requests> <num_iterations>")
        sys.exit(1)

    num_threads = int(sys.argv[1])
    total_requests = int(sys.argv[2])
    num_iterations = int(sys.argv[3])

    times = run_test(num_threads, total_requests, num_iterations)
    print_stats(times)

