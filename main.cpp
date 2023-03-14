#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <string>
#include <sys/time.h>

std::mutex m;
double total_job_time = 0;
constexpr int NUM_JOBS = 32;

double time_elapsed(const timeval& start, const timeval& end) {
    // returns elapsed time in seconds
	return end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec) / 1000000.0;
}

void job() {
	struct timeval start, end;
	gettimeofday(&start, nullptr);
	int total = 0;
	for(int i = 0; i < 1000000; ++i) {
		total += i;
	}
	gettimeofday(&end, nullptr);

	double my_duration = time_elapsed(start, end);

    std::scoped_lock lock(m);
	total_job_time += my_duration;
}

void batch(unsigned int count) {
	for (unsigned int i = 0; i < count; ++i) {
		job();
	}
}

int main(int argc, char** argv) {
	int num_threads = 1;
	if (argc >= 2) {
		num_threads = std::stoi(argv[1]);
    }

	if (num_threads > NUM_JOBS) {
		std::cerr << "Error! Too many threads - more than the number of jobs!" << std::endl;
		return 1;
	}
    else if (num_threads < 0) {
		std::cerr << "Error! Not enough threads." << std::endl;
		return 1;
    }

	struct timeval start, end;
	std::vector<std::thread> threads(num_threads);
	gettimeofday(&start, nullptr);
	for (int i = 0; i < num_threads; ++i) {
		threads[i] = std::thread(batch, NUM_JOBS / num_threads);
	}
	for (auto& t : threads) {
		t.join();
    }
	gettimeofday(&end, nullptr);

	double total_program_duration = time_elapsed(start, end);
    std::cout << "Results for " << num_threads << " threads:" << std::endl;
	std::cout << "throughput = " << NUM_JOBS / total_program_duration << std::endl;
	std::cout << "average response time = " << total_job_time / NUM_JOBS << std::endl << std::endl;
}
