import hashlib

t = ",bsO?"
bytes_str = t.encode('utf-8')
a = hashlib.sha256(bytes_str).hexdigest()[:8]
b = bytes_str.hex()[:8]
print(a)
print(b)