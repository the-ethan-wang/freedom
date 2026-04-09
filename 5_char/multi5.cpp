// g++ -O3 -march=native -pthread test.cpp -lcrypto

#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <openssl/sha.h>

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
};

void worker(uint64_t start, uint64_t end, int length, Result &result) {
    size_t base = alphabet.size();

    vector<uint8_t> current(length);
    vector<uint8_t> buffer(length);
    unsigned char hash[SHA256_DIGEST_LENGTH];

    // Decode starting index
    uint64_t tmp = start;
    for (int i = length - 1; i >= 0; --i) {
        current[i] = tmp % base;
        tmp /= base;
    }

    uint64_t local_count = 0;

    for (uint64_t i = start; i < end; ++i) {
        // Build candidate
        for (int j = 0; j < length; ++j)
            buffer[j] = alphabet[current[j]];

        SHA256(buffer.data(), length, hash);

        if (hash[0] == buffer[0] &&
            hash[1] == buffer[1] &&
            hash[2] == buffer[2] &&
            hash[3] == buffer[3]) {

            result.found.push_back(buffer);
        }

        // Increment base-N counter
        for (int pos = length - 1; pos >= 0; --pos) {
            current[pos]++;
            if (current[pos] < base) break;
            current[pos] = 0;
        }

        local_count++;

        // Batch update (reduces atomic contention massively)
        if ((local_count & 0xFFF) == 0) { // every 4096 iterations
            global_counter += 4096;
        }
    }

    // flush remainder
    global_counter += (local_count & 0xFFF);
}

int main() {
    const int LENGTH = 5;

    size_t base = alphabet.size();

    uint64_t total = 1;
    for (int i = 0; i < LENGTH; ++i)
        total *= base;

    cout << "Total combinations: " << total << endl;

    int num_threads = thread::hardware_concurrency();
    cout << "Using " << num_threads << " threads\n";

    vector<thread> threads;
    vector<Result> results(num_threads);

    uint64_t chunk = total / num_threads;

    auto start_time = chrono::steady_clock::now();

    for (int t = 0; t < num_threads; ++t) {
        uint64_t start = t * chunk;
        uint64_t end = (t == num_threads - 1) ? total : start + chunk;

        threads.emplace_back(worker, start, end, LENGTH, ref(results[t]));
    }

    // Progress + ETA loop
    while (true) {
        uint64_t done = global_counter.load();

        auto now = chrono::steady_clock::now();
        double elapsed = chrono::duration<double>(now - start_time).count();

        double progress = (double)done / total;

        double speed = done / elapsed; // hashes per second

        double remaining = (speed > 0)
            ? (total - done) / speed
            : 0;

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
            cout << endl;
            total_found++;
        }
    }

    cout << "\nTotal found: " << total_found << endl;

    auto end_time = chrono::steady_clock::now();
    double total_time = chrono::duration<double>(end_time - start_time).count();

    cout << "Total time: " << total_time << " seconds\n";

    return 0;
}