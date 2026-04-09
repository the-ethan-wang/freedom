// g++ -O3 -march=native -funroll-loops -flto -fomit-frame-pointer -pthread test.cpp -lcrypto

#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <openssl/sha.h>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

const string chars =
    ",.?!"
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "1234567890"
    "\n ";

vector<uint8_t> alphabet(chars.begin(), chars.end());

atomic<uint64_t> global_counter(0);

struct Result {
    vector<vector<uint8_t>> found;
    char padding[64]; // prevent false sharing
};

void worker(uint64_t start, uint64_t end, int length, Result &result, int thread_id) {
#ifdef _WIN32
    // Pin thread to core
    SetThreadAffinityMask(GetCurrentThread(), 1ULL << thread_id);
#endif

    const uint8_t* alpha = alphabet.data();
    const size_t base = alphabet.size();

    uint8_t current[32];
    uint8_t buffer[32];
    unsigned char hash[SHA256_DIGEST_LENGTH];

    SHA256_CTX ctx;

    // Decode starting index
    uint64_t tmp = start;
    for (int i = length - 1; i >= 0; --i) {
        current[i] = tmp % base;
        buffer[i] = alpha[current[i]];
        tmp /= base;
    }

    uint64_t local_count = 0;

    for (uint64_t i = start; i < end; ++i) {
        // Hash
        SHA256_Init(&ctx);
        SHA256_Update(&ctx, buffer, length);
        SHA256_Final(hash, &ctx);

        // Fast 4-byte compare
        if (*(uint32_t*)hash == *(uint32_t*)buffer) {
            result.found.emplace_back(buffer, buffer + length);
        }

        // Increment base-N counter (incremental update)
        for (int pos = length - 1; pos >= 0; --pos) {
            current[pos]++;
            if (current[pos] < base) {
                buffer[pos] = alpha[current[pos]];
                break;
            }
            current[pos] = 0;
            buffer[pos] = alpha[0];
        }

        local_count++;

        // Batch atomic update (very infrequent)
        if ((local_count & 0xFFFF) == 0) {
            global_counter += 0x10000;
        }
    }

    // Flush remaining
    global_counter += (local_count & 0xFFFF);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    const int LENGTH = 5;

    const size_t base = alphabet.size();

    uint64_t total = 1;
    for (int i = 0; i < LENGTH; ++i)
        total *= base;

    cout << "Total combinations: " << total << "\n";

    int num_threads = thread::hardware_concurrency();
    cout << "Using " << num_threads << " threads\n";

    vector<thread> threads;
    vector<Result> results(num_threads);

    uint64_t chunk = total / num_threads;

    auto start_time = chrono::steady_clock::now();

    for (int t = 0; t < num_threads; ++t) {
        uint64_t start = t * chunk;
        uint64_t end = (t == num_threads - 1) ? total : start + chunk;

        threads.emplace_back(worker, start, end, LENGTH, ref(results[t]), t);
    }

    // Progress monitor
    while (true) {
        uint64_t done = global_counter.load(memory_order_relaxed);

        auto now = chrono::steady_clock::now();
        double elapsed = chrono::duration<double>(now - start_time).count();

        double speed = done / elapsed;
        double progress = (double)done / total;
        double remaining = (speed > 0) ? (total - done) / speed : 0;

        cout << "\rProgress: " << (progress * 100.0) << "% "
             << "| Speed: " << (uint64_t)speed << " H/s "
             << "| ETA: " << (uint64_t)remaining << "s   ";
        cout.flush();

        if (done >= total) break;

        this_thread::sleep_for(chrono::milliseconds(500));
    }

    for (auto &t : threads)
        t.join();

    cout << "\n\nResults:\n";

    size_t total_found = 0;

    for (auto &r : results) {
        for (auto &v : r.found) {
            for (auto b : v) cout << (char)b;
            cout << '\n';
            total_found++;
        }
    }

    cout << "\nTotal found: " << total_found << "\n";

    auto end_time = chrono::steady_clock::now();
    double total_time = chrono::duration<double>(end_time - start_time).count();

    cout << "Total time: " << total_time << " seconds\n";

    return 0;
}