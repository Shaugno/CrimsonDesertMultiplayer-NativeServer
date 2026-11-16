// Self-authored offline graph fixture. Never loads or accesses the game.
extern "C" __declspec(dllexport) __declspec(noinline) int fixture_orphan(int n) {
    if (n < 0) return n * 3;
    if (n & 1) return n + 19;
    return n ^ 0x53;
}
extern "C" __declspec(dllexport) __declspec(noinline) int fixture_branch(int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) sum += fixture_orphan(i);
    return sum;
}
int main(int argc, char**) { return fixture_branch(argc); }
