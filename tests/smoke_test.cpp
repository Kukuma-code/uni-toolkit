// scaffold smoke: ツールチェーン（C++17）と CTest 配線の検証のみ。
#include <cassert>

int main() {
    static_assert(__cplusplus >= 201703L, "C++17 required");
    assert(true);
    return 0;
}
