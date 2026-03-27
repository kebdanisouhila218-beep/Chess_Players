#pragma once
#include <vector>
#include <functional>

struct HexCell {
    int q, r;

    int s() const { return -q - r; }

    bool operator==(const HexCell& o) const {
        return q == o.q && r == o.r;
    }

    std::vector<HexCell> neighbors() const {
        return {
            {q+1, r  }, {q-1, r  },
            {q,   r+1}, {q,   r-1},
            {q+1, r-1}, {q-1, r+1}
        };
    }
};

// Hash pour utiliser HexCell dans unordered_map
struct HexHash {
    size_t operator()(const HexCell& c) const {
        return std::hash<int>()(c.q) ^ (std::hash<int>()(c.r) << 16);
    }
};