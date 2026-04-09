import math
from itertools import product
import hashlib
import time

START_LENGTH = 4
END_LENGTH = 4

alphabet = list("abcdefghijklmnopqrstuvwxyz")
alphabet_upper = list("ABCDEFGHIJKLMNOPQRSTUVWXYZ")
numbers = list("1234567890")
whitespace = list("\n ")
punctuation = list(",.?!")

message_chars = (
    punctuation
    +alphabet
    +alphabet_upper
    +numbers
    +whitespace
)

total_times = []
total_found = []

for length in range(START_LENGTH, END_LENGTH + 1):
    checked_strings = 0
    valid = []
    start = time.time()
    print(f"Checking for length {length}")
    
    total = len(message_chars) ** length
    increment = max(1, total // 100)

    for i, combo in enumerate(product(message_chars, repeat=length), 1):
        checked_strings += 1
        if i % increment == 0:
            print(f"Checked {i}/{total} ({100*i/total:.0f}%). {math.ceil((1-(i/total))*((time.time()-start)/(i/total)))} seconds remaining")
        string = ''.join(combo)
        if hashlib.sha256(string.encode('utf-8')).hexdigest()[:8]==string[:4].encode('utf-8').hex():
            valid.append(string)
            print(f"Found: '{string}'")
    
    if valid:
        print("Found:")
        print(*valid, sep="\n")
    print(f"Took {math.ceil(time.time()-start)} seconds to check length {length}")
    total_found.extend(valid)
    total_times.append(round(time.time()-start,2))

print(f"\n\nOverall, found {len(total_found)} from length {START_LENGTH} to {END_LENGTH} across {math.ceil(sum(total_times))} seconds.")
print(*total_found, sep="\n")
print(f"Times for each split:")
print(*total_times, sep="\n")