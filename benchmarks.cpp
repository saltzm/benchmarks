
#include <benchmark/benchmark.h>

#include <atomic>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

/*****************************************************************************
 * FUNCTION CALL OVERHEAD
 *****************************************************************************/

class Parent {
   public:
    virtual void increment() = 0;
    virtual int get() = 0;
};

class Child final : public Parent {
   public:
    void increment() override { ++i; }
    int get() override { return i; }

   private:
    int i{0};
};

class StandaloneNoInline {
   public:
    // TODO: Make portable across compilers.
    [[gnu::noinline]] void increment() { ++i; }
    [[gnu::noinline]] int get() { return i; }

   private:
    int i{0};
};

class StandaloneInline {
   public:
    // These will be inlined.
    void increment() { ++i; }
    int get() { return i; }

   private:
    int i{0};
};

void BM_virtualFunctionCallsThroughPointerToParent(benchmark::State& state) {
    std::unique_ptr<Parent> parent = std::make_unique<Child>();
    for (auto _ : state) {
        parent->increment();
        benchmark::DoNotOptimize(parent->get());
    }
}

void BM_virtualFunctionCallsThroughPointerToChild(benchmark::State& state) {
    std::unique_ptr<Child> child = std::make_unique<Child>();
    for (auto _ : state) {
        child->increment();
        benchmark::DoNotOptimize(child->get());
    }
}

void BM_virtualFunctionCallsThroughInstanceOfChild(benchmark::State& state) {
    Child child;
    for (auto _ : state) {
        child.increment();
        benchmark::DoNotOptimize(child.get());
    }
}

void BM_nonVirtualNonInlineFunctionCall(benchmark::State& state) {
    StandaloneNoInline obj;
    for (auto _ : state) {
        obj.increment();
        benchmark::DoNotOptimize(obj.get());
    }
}

void BM_inlineFunctionCall(benchmark::State& state) {
    StandaloneInline obj;
    for (auto _ : state) {
        obj.increment();
        benchmark::DoNotOptimize(obj.get());
    }
}

void BM_noFunctionCall(benchmark::State& state) {
    int i = 0;
    for (auto _ : state) {
        ++i;
        benchmark::DoNotOptimize(i);
    }
}

void BM_stdFunctionCall(benchmark::State& state) {
    int i = 0;
    std::function<void()> fn = [&i]() { ++i; };
    for (auto _ : state) {
        fn();
        benchmark::DoNotOptimize(i);
    }
}

void BM_lambdaFunctionCall(benchmark::State& state) {
    int i = 0;
    auto fn = [&i]() { ++i; };
    for (auto _ : state) {
        fn();
        benchmark::DoNotOptimize(i);
    }
}

template <typename Callable>
void functionThatCallsLambda(Callable&& callable) {
    callable();
}

void functionThatCallsFunction(std::function<void()>&& callable) { callable(); }

void BM_stdFunctionPassedAsParameterFunctionCall(benchmark::State& state) {
    int i = 0;
    auto fn = [&i]() { ++i; };
    for (auto _ : state) {
        functionThatCallsFunction([&i]() { ++i; });
        benchmark::DoNotOptimize(i);
    }
}

void BM_lambdaPassedAsParameterFunctionCall(benchmark::State& state) {
    int i = 0;
    auto fn = [&i]() { ++i; };
    for (auto _ : state) {
        functionThatCallsLambda([&i]() { ++i; });
        benchmark::DoNotOptimize(i);
    }
}

/*****************************************************************************
 * CACHE MISSES
 *
 * TODO: These assume L1 cache of 32K or smaller. See if we can make this more
 *       portable.
 *****************************************************************************/

static void BM_sequentialListAccess(benchmark::State& state) {
    constexpr int k = 1024;
    std::list<std::uint32_t> arr;

    for (auto i = 0; i < k; ++i) {
        arr.emplace_back(i);
    }

    for (auto _ : state) {
        std::uint32_t sum = 0;
        for (auto x : arr) {
            sum += x;
        }
        benchmark::DoNotOptimize(sum);
    }
}

static void BM_sequentialArrayAccess(benchmark::State& state) {
    constexpr int k = 1024;
    std::vector<std::uint32_t> arr;

    for (auto i = 0; i < k; ++i) {
        arr.emplace_back(i);
    }

    for (auto _ : state) {
        std::uint32_t sum = 0;
        for (auto x : arr) {
            sum += x;
        }
        benchmark::DoNotOptimize(sum);
    }
}

static void BM_sequentialArrayAccessSmallerThanL1(benchmark::State& state) {
    constexpr int k = 32;
    std::uint32_t arr[k][k];
    for (auto i = 0; i < k; ++i) {
        for (auto j = 0; j < k; ++j) {
            arr[j][i] = i * j;
        }
    }

    for (auto _ : state) {
        std::uint32_t sum = 0;
        for (auto i = 0; i < k; ++i) {
            for (auto j = 0; j < k; ++j) {
                // Row order traversal
                sum += arr[i][j];
            }
        }
        benchmark::DoNotOptimize(sum);
    }
}

static void BM_randomArrayAccessSmallerThanL1(benchmark::State& state) {
    constexpr int k = 32;
    std::uint32_t arr[k][k];
    for (auto i = 0; i < k; ++i) {
        for (auto j = 0; j < k; ++j) {
            arr[j][i] = i * j;
        }
    }

    for (auto _ : state) {
        std::uint32_t sum = 0;
        for (auto i = 0; i < k; ++i) {
            for (auto j = 0; j < k; ++j) {
                // Column order traversal
                sum += arr[j][i];
            }
        }
        benchmark::DoNotOptimize(sum);
    }
}

static void BM_sequentialArrayAccessBiggerThanL1(benchmark::State& state) {
    constexpr int k = 1'024;
    std::uint32_t arr[k][k];
    for (auto i = 0; i < k; ++i) {
        for (auto j = 0; j < k; ++j) {
            arr[i][j] = i * j;
        }
    }

    for (auto _ : state) {
        std::uint32_t sum = 0;
        for (auto i = 0; i < k; ++i) {
            for (auto j = 0; j < k; ++j) {
                // Row order traversal
                sum += arr[i][j];
            }
        }
        benchmark::DoNotOptimize(sum);
    }
}

static void BM_randomArrayAccessBiggerThanL1(benchmark::State& state) {
    constexpr int k = 1'024;
    std::uint32_t arr[k][k];
    for (auto i = 0; i < k; ++i) {
        for (auto j = 0; j < k; ++j) {
            arr[i][j] = i * j;
        }
    }

    for (auto _ : state) {
        std::uint32_t sum = 0;
        for (auto i = 0; i < k; ++i) {
            for (auto j = 0; j < k; ++j) {
                // Column order traversal
                sum += arr[j][i];
            }
        }
        benchmark::DoNotOptimize(sum);
    }
}

/*****************************************************************************
 * FALSE SHARING
 *****************************************************************************/

/**
 * Simple barrier based on busy-waiting
 */
class Barrier {
   public:
    Barrier(int numTotalThreads) : _numTotalThreads(numTotalThreads) {}

    void arriveAndWait() {
        ++_numThreadsArrived;
        while (_numThreadsArrived < _numTotalThreads) {
        }
    }

   private:
    std::atomic_int32_t _numThreadsArrived{0};
    int _numTotalThreads;
};

const auto kNumIterationsFalseSharing = 1000000;

static void BM_falseSharing(benchmark::State& state) {
    // Both of these will end up on the same cache line.
    // TODO: This isn't guaranteed. Make this better.
    struct Counter {
        std::uint32_t val{0};
    } counterA, counterB;

    for (auto _ : state) {
        Barrier barrier(3);

        std::thread a([&] {
            barrier.arriveAndWait();
            for (auto i = 0; i < kNumIterationsFalseSharing; ++i)
                benchmark::DoNotOptimize(++counterA.val);
        });
        std::thread b([&] {
            barrier.arriveAndWait();
            for (auto i = 0; i < kNumIterationsFalseSharing; ++i)
                benchmark::DoNotOptimize(++counterB.val);
        });

        barrier.arriveAndWait();
        auto start = std::chrono::high_resolution_clock::now();
        a.join();
        b.join();
        auto end = std::chrono::high_resolution_clock::now();

        auto elapsed_seconds =
            std::chrono::duration_cast<std::chrono::duration<double>>(end -
                                                                      start);

        state.SetIterationTime(elapsed_seconds.count());
    }
}

static void BM_noFalseSharing(benchmark::State& state) {
    // Align the struct at cache line boundaries.
    struct alignas(128) Counter {
        std::uint32_t val{0};
    } counterA, counterB;

    for (auto _ : state) {
        Barrier barrier(3);
        std::thread a([&] {
            barrier.arriveAndWait();

            for (auto i = 0; i < kNumIterationsFalseSharing; ++i)
                benchmark::DoNotOptimize(++counterA.val);
        });
        std::thread b([&] {
            barrier.arriveAndWait();

            for (auto i = 0; i < kNumIterationsFalseSharing; ++i)
                benchmark::DoNotOptimize(++counterB.val);
        });
        barrier.arriveAndWait();
        auto start = std::chrono::high_resolution_clock::now();
        a.join();
        b.join();
        auto end = std::chrono::high_resolution_clock::now();

        auto elapsed_seconds =
            std::chrono::duration_cast<std::chrono::duration<double>>(end -
                                                                      start);

        state.SetIterationTime(elapsed_seconds.count());
    }
}

/*****************************************************************************
 * LOCKING VS. ATOMICS
 *****************************************************************************/

const auto kNumIterationsMutex = 1000000;

static void BM_useMutex(benchmark::State& state) {
    std::mutex mtx;
    std::uint32_t counter{0};
    for (auto _ : state) {
        Barrier barrier(3);
        std::thread a([&] {
            barrier.arriveAndWait();
            for (auto i = 0; i < kNumIterationsMutex; ++i) {
                std::lock_guard lk(mtx);
                benchmark::DoNotOptimize(++counter);
            }
        });
        std::thread b([&] {
            barrier.arriveAndWait();
            for (auto i = 0; i < kNumIterationsMutex; ++i) {
                std::lock_guard lk(mtx);
                benchmark::DoNotOptimize(++counter);
            }
        });

        barrier.arriveAndWait();
        auto start = std::chrono::high_resolution_clock::now();
        a.join();
        b.join();
        auto end = std::chrono::high_resolution_clock::now();

        auto elapsed_seconds =
            std::chrono::duration_cast<std::chrono::duration<double>>(end -
                                                                      start);

        state.SetIterationTime(elapsed_seconds.count());
    }
}

// TODO: This benchmark is suspect and doesn't really compare to the previous
// one. Figure out something better.
static void BM_useMutexNoContention(benchmark::State& state) {
    for (auto _ : state) {
        Barrier barrier(2);
        std::thread a([&] {
            barrier.arriveAndWait();
            std::mutex mtx;
            std::uint32_t counter{0};
            for (auto i = 0; i < kNumIterationsMutex; ++i) {
                std::lock_guard lk(mtx);
                benchmark::DoNotOptimize(++counter);
            }
        });
        std::thread b([&] {
            barrier.arriveAndWait();
            std::mutex mtx;
            std::uint32_t counter{0};
            for (auto i = 0; i < kNumIterationsMutex; ++i) {
                std::lock_guard lk(mtx);
                benchmark::DoNotOptimize(++counter);
            }
        });
        barrier.arriveAndWait();
        auto start = std::chrono::high_resolution_clock::now();
        a.join();
        b.join();
        auto end = std::chrono::high_resolution_clock::now();

        auto elapsed_seconds =
            std::chrono::duration_cast<std::chrono::duration<double>>(end -
                                                                      start);

        state.SetIterationTime(elapsed_seconds.count());
    }
}

static void BM_useAtomic(benchmark::State& state) {
    for (auto _ : state) {
        std::atomic_int32_t counter{0};

        Barrier barrier(2);
        std::thread a([&] {
            barrier.arriveAndWait();
            for (auto i = 0; i < kNumIterationsMutex; ++i) {
                benchmark::DoNotOptimize(++counter);
            }
        });
        std::thread b([&] {
            barrier.arriveAndWait();
            for (auto i = 0; i < kNumIterationsMutex; ++i)
                benchmark::DoNotOptimize(++counter);
        });
        barrier.arriveAndWait();
        auto start = std::chrono::high_resolution_clock::now();
        a.join();
        b.join();
        auto end = std::chrono::high_resolution_clock::now();

        auto elapsed_seconds =
            std::chrono::duration_cast<std::chrono::duration<double>>(end -
                                                                      start);

        state.SetIterationTime(elapsed_seconds.count());
    }
}

BENCHMARK(BM_virtualFunctionCallsThroughPointerToParent);
BENCHMARK(BM_virtualFunctionCallsThroughPointerToChild);
BENCHMARK(BM_virtualFunctionCallsThroughInstanceOfChild);
BENCHMARK(BM_nonVirtualNonInlineFunctionCall);
BENCHMARK(BM_inlineFunctionCall);
BENCHMARK(BM_noFunctionCall);
BENCHMARK(BM_stdFunctionCall);
BENCHMARK(BM_lambdaFunctionCall);

BENCHMARK(BM_stdFunctionPassedAsParameterFunctionCall);
BENCHMARK(BM_lambdaPassedAsParameterFunctionCall);

BENCHMARK(BM_sequentialListAccess);
BENCHMARK(BM_sequentialArrayAccess);

BENCHMARK(BM_sequentialArrayAccessSmallerThanL1);
BENCHMARK(BM_randomArrayAccessSmallerThanL1);

BENCHMARK(BM_sequentialArrayAccessBiggerThanL1);
BENCHMARK(BM_randomArrayAccessBiggerThanL1);

BENCHMARK(BM_falseSharing)->UseManualTime();
BENCHMARK(BM_noFalseSharing)->UseManualTime();

BENCHMARK(BM_useMutex)->UseManualTime();
BENCHMARK(BM_useMutexNoContention)->UseManualTime();
BENCHMARK(BM_useAtomic)->UseManualTime();

BENCHMARK_MAIN();
