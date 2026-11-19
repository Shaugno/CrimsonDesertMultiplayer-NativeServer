import sys, struct, collections, pefile
o = pefile.PE(sys.argv[1], fast_load=True); n = pefile.PE(sys.argv[2], fast_load=True)
a = o.get_data(0x6F14E10, 0x339A0); b = n.get_data(0x6F1AE10, 0x339A0)
base = 0x140000000
d = collections.Counter()
for k in range(0, len(a) - 7, 8):
    x, y = struct.unpack_from('<Q', a, k)[0], struct.unpack_from('<Q', b, k)[0]
    if base <= x < base + 0x20000000:
        d[y - x] += 1
for delta, c in d.most_common(12):
    print(f'{delta:+#x}  {c}')
print('distinct deltas', len(d), 'min', hex(min(d)), 'max', hex(max(d)))
