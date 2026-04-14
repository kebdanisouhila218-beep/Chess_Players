#pragma once
#include <vector>
#include <functional>

// HexCell represente une case logique par deux coordonnees entieres q et r.
// Dans ce projet, ces coordonnees sont utilisees sur une grille logique 12x12.
// Le plateau contient 96 cases valides, reparties en 6 sextants rectangulaires 4x4.
// La validite d'une case n'est pas portee par HexCell lui-meme : elle est geree par Board.
struct HexCell {
    int q, r;

    int s() const { return -q - r; }

    bool operator==(const HexCell& o) const noexcept {
        return q == o.q && r == o.r;
    }

    bool operator!=(const HexCell& o) const noexcept {
        return !(*this == o);
    }

    HexCell translated(int dq, int dr) const noexcept {
        return {q + dq, r + dr};
    }

    int manhattanLikeDistance(const HexCell& o) const noexcept {
        const int dq = q - o.q;
        const int dr = r - o.r;
        return (dq < 0 ? -dq : dq) + (dr < 0 ? -dr : dr);
    }

    std::vector<HexCell> neighbors() const {
        return {
            translated(1, 0), translated(-1, 0),
            translated(0, 1), translated(0, -1),
            translated(1, -1), translated(-1, 1)
        };
    }
};

// Hash injectif sur le domaine logique du plateau 12x12 : q + 12 * r.
// Pour des coordonnees hors domaine, on conserve un melange simple et stable.
struct HexHash {
    size_t operator()(const HexCell& c) const noexcept {
        if (c.q >= 0 && c.q < 12 && c.r >= 0 && c.r < 12) {
            return static_cast<size_t>(c.q + 12 * c.r);
        }
        return (static_cast<size_t>(static_cast<unsigned int>(c.q)) << 32)
             ^ static_cast<size_t>(static_cast<unsigned int>(c.r));
    }
};