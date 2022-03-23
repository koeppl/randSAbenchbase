#pragma once
#include <iostream>
#include <fstream>
#include "mmap.hpp"
#include "dcheck.hpp"
#include <functional>
#include <tudocomp/ds/uint_t.hpp>

#include <chrono>

namespace my {
  class Timer {
    using timepoint_t = std::chrono::system_clock::time_point;
    timepoint_t m_start, m_stop;

    public:
    Timer(): m_start(std::chrono::system_clock::now()) {}
    void stop() { 
      m_stop = std::chrono::system_clock::now();
    }
    void start() { 
      m_start = std::chrono::system_clock::now();
    }
    double restart() { 
      m_stop = std::chrono::system_clock::now();
      const double ret = report();
      m_start = m_stop;
      return ret;
    }
    double report() const {
      return std::chrono::duration<double>(m_stop - m_start).count();
    }
  };
}


static void assert_file_exists(const char* name) {
  std::ifstream f(name);
      if(!f.good()) {
        fprintf(stderr, "File %s does not exist!\n", name);
        exit(1);
      }
}

static void batch_query(const char* label, const std::string& basename, std::function<uint64_t(uint64_t)> access_sa, bool verify = true) {
  constexpr size_t MAX_QUERIES = 1e5; //-1ULL;
  constexpr size_t GROUP_SIZE = 100; //-1ULL;

  const std::string query_filename = basename + ".query";
  assert_file_exists(query_filename.c_str());
  
  const auto sa = [&] () -> std::pair<tdc::uint_t<40>*,size_t> {
    if(!verify) { return std::make_pair(nullptr,0UL); }
    const std::string sa_filename = basename + ".sa5";
    assert_file_exists(sa_filename.c_str());
    return map_file<tdc::uint_t<40>>(sa_filename.c_str());
  }();

  my::Timer timer;
  double accumulated_time = 0;
  double accumulated_group_time = 0;
  uint64_t query_index;
  std::ifstream queryfile(query_filename);
  size_t number_queries = 0;
  for(; number_queries < MAX_QUERIES; ++number_queries) {
    queryfile.read(reinterpret_cast<char*>(&query_index), 5); // read uint<40> bit integer
    if(!queryfile.good()) { break; }
    if(verify) {
      CHECK_LT(query_index, sa.second);
    }
    timer.start();
	const uint64_t answer = access_sa(query_index);
    timer.stop();
    accumulated_group_time += timer.restart();
    if(number_queries % GROUP_SIZE == 0) {
      accumulated_time += accumulated_group_time;
      fprintf(stderr, "\r%s: avg.time: %f group-time: %f, groupsize: %lu   %lu/%lu", label, accumulated_time/(1+number_queries), accumulated_group_time/GROUP_SIZE, GROUP_SIZE, number_queries, sa.second == 0 ? MAX_QUERIES : std::min(sa.second, MAX_QUERIES));
      accumulated_group_time = 0;
    }
    if(verify) {
      const uint64_t baseline_answer = sa.first[query_index];
      if(baseline_answer != answer) {
        fprintf(stderr, "\nSA[%lu] = %lu, but index returned %lu\n", query_index, baseline_answer, answer);
        // exit(1);
      }
    }
	// fwrite(&answer, sizeof(decltype(answer)), 1, stdout);
  }
  fprintf(stderr, "\nalgo=%s total_time=%f avg_time=%f queries=%lu\n", label, accumulated_time, accumulated_time/number_queries, number_queries);
  queryfile.close();
}


