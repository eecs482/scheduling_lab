#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>
#include <vector>

using std::cout;
using std::endl;
using std::ostream;
using std::vector;
using std::thread;

using Clock = std::chrono::high_resolution_clock;
using FloatS = std::chrono::duration<double, std::ratio<1>>;
using FloatMS = std::chrono::duration<double, std::milli>;
using TimePoint = std::chrono::time_point<Clock>;

TimePoint gettimeofday();
FloatMS ms_between(TimePoint t1, TimePoint t2);
ostream& operator<<(ostream&, FloatMS);

/**
 * Represents runtime statistics about a (parallel-executed) job.
 */
struct Summary {
  int jobs_completed;
  FloatMS average_response_time;
  double throughput;  ///< Jobs completed per second.

  template <typename ForwardIt>
  static Summary combined_from(ForwardIt begin, ForwardIt end);
};
ostream& operator<<(ostream&, const Summary&);

Summary lab_a();
Summary lab_b(int);
Summary lab_c(int, int);

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// JOB /////////////////////////////////////
/**
 * Do some "work."
 */
void job() {
  int sum = 0;
  for (int i = 0; i < 100000000; ++i) { sum += i; }
}

///////////////////////////////////// MAIN /////////////////////////////////////
int main() {
  cout << lab_a() << endl;
  cout << lab_b(32) << endl;
  cout << lab_c(32, 4) << endl;

  return 0;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
///////////////////////// LAB EXERCISE IMPLEMENTATIONS /////////////////////////
Summary lab_a() {
  cout << "Running part A." << endl;

  auto start = gettimeofday();
  job();
  auto end = gettimeofday();
  FloatMS runtime = ms_between(start, end);
  return Summary{1, runtime, 1 / FloatS(runtime).count()};
}

Summary lab_b(int num_jobs) {
  cout << "Running part B with " << num_jobs << " consecutive jobs." << endl;
  assert(num_jobs >= 0);

  auto start = gettimeofday();
  for (int i = 0; i < num_jobs; ++i) job();
  auto end = gettimeofday();

  FloatMS runtime = ms_between(start, end);
  return Summary{num_jobs, runtime / num_jobs,
                 num_jobs / FloatS(runtime).count()};
}

Summary lab_c(int num_jobs, int num_threads) {
  cout << "Running part C with " << num_jobs << " jobs and "
       << num_threads << "threads." << endl;
  assert(num_threads > 0);
  assert(num_jobs > 0);
  assert((num_jobs % num_threads) == 0
         and "num_jobs should be divisible by num_threads!");

  vector<Summary> results(num_threads);
  vector<thread> executors;
  for (int i = 0; i < num_threads; ++i) {
    executors.emplace_back(
        [&, i]() -> void {
          results[i] = lab_b(num_jobs / num_threads);
        });
  }

  // wait for executors to finish
  for (auto& t : executors) t.join();

  // compile runtime data
  return Summary::combined_from(results.begin(), results.end());
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// HELPERS ///////////////////////////////////
/**
 *  Return the an STL time point representing "right now."
 */
TimePoint gettimeofday() {
  return Clock::now();
}

/**
 * Return, as a floating point number, the absolute number of milliseconds
 * between t1 and t2. The order in which t1 and t2 are given does not matter;
 * the returned value will always be non-negative.
 */
FloatMS ms_between(TimePoint t1, TimePoint t2) {
  // t2 - t1 returns an std::chrono::duration (typically chrono::nanoseconds)
  //   with an integer representation
  // the FloatMS constructor converts this to a floating point value, and then
  //   to milliseconds
  FloatMS difference(t2 - t1);
  return FloatMS(std::abs(difference.count()));
}

/**
 * Print a FloatMS to an output stream.
 */
ostream& operator<<(ostream& os, FloatMS duration) {
  return os << duration.count() << "ms";
}

/**
 * Print a Summary to an output stream.
 */
ostream& operator<<(ostream& os, const Summary& s) {
  return os << "Average response time: " << s.average_response_time << '\n'
            << "Throughput:            " << s.throughput;
}

/**
 * Construct an aggregate Summary of several subtask summaries; subtasks
 * are assumed to have been run in parallel.
 *
 * - jobs_completed and throughput are combined additively.
 * - average_response_time is calculated by averaging the average response
 *   times of each subtask summary.
 */
template <typename ForwardIt>
Summary Summary::combined_from(ForwardIt begin, ForwardIt end) {
  // compile runtime data
  Summary aggregated{0, FloatMS(0), 0};
  int num_summaries = 0;
  for (ForwardIt it = begin; it != end; ++it) {
    const Summary& result = *it;
    num_summaries++;
    aggregated.jobs_completed += result.jobs_completed;
    aggregated.throughput += result.throughput;
    aggregated.average_response_time += result.average_response_time;
  }
  aggregated.average_response_time /= num_summaries;
  return aggregated;
}
