// g++ -O3 -march=native main.cpp -lcrypto

#include <iostream>
#include <vector>
#include <cmath>
#include <ctime>
#include <openssl/sha.h>

using namespace std;

int main() {
    const int START_LENGTH = 6;
    const int END_LENGTH = 6;
 
    vector<uint8_t> alphabet;

    string chars =
        ",.?!" 
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "1234567890"
        "\n ";

    for (char c : chars)
        alphabet.push_back(static_cast<uint8_t>(c));

    vector<double> total_times;
    vector<vector<uint8_t>> total_found;

    for (int length = START_LENGTH; length <= END_LENGTH; ++length) {
        clock_t start = clock();
        cout << "Checking length " << length << endl;

        size_t base = alphabet.size();
        size_t total = pow(base, length);
        size_t increment = max((size_t)1, total / 100);

        vector<uint8_t> current(length, 0);
        vector<uint8_t> buffer(length);
        unsigned char hash[SHA256_DIGEST_LENGTH];

        vector<vector<uint8_t>> valid;

        for (size_t i = 0; i < total; ++i) {
            if (i % increment == 0) {
                double elapsed = double(clock() - start) / CLOCKS_PER_SEC;
                double progress = double(i) / total;
                double remaining = (progress > 0)
                    ? ceil((1 - progress) * (elapsed / progress))
                    : 0;

                cout << "Checked " << i << "/" << total
                     << " (" << int(progress * 100) << "%)"
                     << " ETA: " << remaining << "s\n";
            }

            for (int j = 0; j < length; ++j)
                buffer[j] = alphabet[current[j]];

            SHA256(buffer.data(), length, hash);

            bool match = true;
            for (int j = 0; j < 4; ++j) {
                if (hash[j] != buffer[j]) {
                    match = false;
                    break;
                }
            }

            if (match) {
                valid.push_back(buffer);
                cout << "Found: ";
                for (auto b : buffer) cout << (char)b;
                cout << endl;
            }

            for (int pos = length - 1; pos >= 0; --pos) {
                current[pos]++;
                if (current[pos] < base) break;
                current[pos] = 0;
            }
        }

        double elapsed = double(clock() - start) / CLOCKS_PER_SEC;
        cout << "Took " << ceil(elapsed) << " seconds\n";

        total_found.insert(total_found.end(), valid.begin(), valid.end());
        total_times.push_back(elapsed);
    }

    cout << "\nOverall found: " << total_found.size() << endl;

    for (auto &v : total_found) {
        for (auto b : v) cout << (char)b;
        cout << endl;
    }

    cout << "Times:\n";
    for (auto t : total_times)
        cout << t << endl;

    return 0;
}