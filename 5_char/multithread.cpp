// g++ -O3 -march=native -pthread main.cpp -lcrypto

#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <openssl/sha.h>
#include <mutex>

using namespace std;

const string chars =
    ",.?!"
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "1234567890"
    "\n ";

vector<uint8_t> alphabet(chars.begin(), chars.end());

atomic<uint64_t> global_counter(0);
vector<vector<uint8_t>> global_found;
mutex found_mutex;

void worker(uint64_t start, uint64_t end, int length) {
    size_t base = alphabet.size();

    vector<uint8_t> current(length);
    vector<uint8_t> buffer(length);
    unsigned char hash[SHA256_DIGEST_LENGTH];

    // Decode starting index into base-N representation
    uint64_t tmp = start;
    for (int i = length - 1; i >= 0; --i) {
        current[i] = tmp % base;
        tmp /= base;
    }

    for (uint64_t i = start; i < end; ++i) {
        // Build string
        for (int j = 0; j < length; ++j)
            buffer[j] = alphabet[current[j]];

        // Hash
        SHA256(buffer.data(), length, hash);

        // Check first 4 bytes
        if (hash[0] == buffer[0] &&
            hash[1] == buffer[1] &&
            hash[2] == buffer[2] &&
            hash[3] == buffer[3]) {

            lock_guard<mutex> lock(found_mutex);
            global_found.push_back(buffer);

            cout << "Found: ";
            for (auto b : buffer) cout << (char)b;
            cout << endl;
        }

        // Increment base-N counter
        for (int pos = length - 1; pos >= 0; --pos) {
            current[pos]++;
            if (current[pos] < base) break;
            current[pos] = 0;
        }

        global_counter++;
    }
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
    uint64_t chunk = total / num_threads;

    for (int t = 0; t < num_threads; ++t) {
        uint64_t start = t * chunk;
        uint64_t end = (t == num_threads - 1) ? total : start + chunk;

        threads.emplace_back(worker, start, end, LENGTH);
    }

    // Progress monitor
    while (global_counter < total) {
        uint64_t done = global_counter.load();
        double progress = (double)done / total;

        cout << "\rProgress: " << (progress * 100.0) << "%";
        cout.flush();

        this_thread::sleep_for(chrono::seconds(1));
    }

    for (auto &t : threads)
        t.join();

    cout << "\nDone.\n";
    cout << "Found: " << global_found.size() << endl;

    return 0;
}