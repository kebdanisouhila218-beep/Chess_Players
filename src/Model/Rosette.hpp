#pragma once

#include "HexCell.hpp"
#include "Piece.hpp"

#include <array>
#include <cmath>

namespace rosette {

static inline bool isCenter(const HexCell& c) {
    return c.q == 0 && c.r == 0;
}

static inline bool isRosette(const HexCell& c) {
    if (isCenter(c)) return true;
    return (c.q == 1 && c.r == 0)
        || (c.q == 1 && c.r == -1)
        || (c.q == 0 && c.r == -1)
        || (c.q == -1 && c.r == 0)
        || (c.q == -1 && c.r == 1)
        || (c.q == 0 && c.r == 1);
}

static inline int rosetteColor(const HexCell& c) {
    int v = (c.q * 2 + c.r + 6) % 2;
    if (v < 0) v += 2;
    return v;
}

static inline int cubeDist0(const HexCell& c) {
    int x = c.q, z = c.r, y = -x - z;
    return (std::abs(x) + std::abs(y) + std::abs(z)) / 2;
}

static inline HexCell add(const HexCell& a, const HexCell& b) { return {a.q+b.q, a.r+b.r}; }
static inline HexCell sub(const HexCell& a, const HexCell& b) { return {a.q-b.q, a.r-b.r}; }
static inline HexCell neg(const HexCell& a) { return {-a.q, -a.r}; }
static inline int dot(const HexCell& a, const HexCell& b) { return a.q*b.q + a.r*b.r; }

static inline int ringIndex(const HexCell& c) {
    static constexpr std::array<HexCell, 6> ring = {
        HexCell{1,0}, HexCell{1,-1}, HexCell{0,-1},
        HexCell{-1,0}, HexCell{-1,1}, HexCell{0,1}
    };
    for (int i = 0; i < 6; ++i)
        if (ring[i] == c) return i;
    return -1;
}

    
  static inline HexCell rookRedirect(const HexCell& entry, HexCell dir) {
    // Regle exacte :
    // - Centre (0,0) : jamais de redirect, la tour continue tout droit
    // - Ring cell : redirect SEULEMENT si dir pointe vers le centre (0,0)
    //   car sinon la tour peut sortir normalement de la rosette

    if (isCenter(entry)) return dir;

    // Sur un ring cell, la direction exacte vers (0,0) = neg(entry)
    // car tous les ring cells sont des voisins directs de (0,0)
    HexCell toCenter = neg(entry);

    if (!(dir == toCenter)) {
        // La tour ne va pas vers le centre -> pas de redirect
        return dir;
    }

    // La tour veut traverser le centre -> redirect lateral
    static constexpr std::array<HexCell, 6> rookDirs = {
        HexCell{1,0}, HexCell{0,1}, HexCell{-1,1},
        HexCell{-1,0}, HexCell{0,-1}, HexCell{1,-1}
    };

    // entryEdge = direction depuis laquelle on est arrive = neg(dir)
    HexCell entryEdge = neg(dir); // = entry (car toCenter = neg(entry))
    int idx = -1;
    for (int i = 0; i < 6; ++i) {
        if (rookDirs[i] == entryEdge) { idx = i; break; }
    }
    if (idx < 0) return dir;

    // Les 2 sorties laterales sont a +2 et -2 dans le cycle (= 120 degres)
    HexCell candA = rookDirs[(idx + 2) % 6];
    HexCell candB = rookDirs[(idx + 4) % 6];

    // Choisit celle qui pointe le plus vers l'exterieur
    int a = dot(candA, entry);
    int b = dot(candB, entry);
    return (a >= b) ? candA : candB;
}
static inline HexCell bishopRedirect(const HexCell& entry, HexCell dir) {
    static constexpr std::array<HexCell, 6> bishopDirs = {
        HexCell{1,1}, HexCell{-1,2}, HexCell{-2,1},
        HexCell{-1,-1}, HexCell{1,-2}, HexCell{2,-1}
    };

    // Cas special : diagonale qui va vers le centre depuis un ring cell
    // -> sortie par l'oppose meme couleur
    if (!isCenter(entry) && isRosette(entry)) {
        HexCell nextCell = add(entry, dir);
        if (isCenter(nextCell)) {
            int idx = ringIndex(entry);
            if (idx >= 0) {
                static constexpr std::array<HexCell, 6> ring = {
                    HexCell{1,0}, HexCell{1,-1}, HexCell{0,-1},
                    HexCell{-1,0}, HexCell{-1,1}, HexCell{0,1}
                };
                HexCell opposite = ring[(idx + 3) % 6];
                if (rosetteColor(opposite) == rosetteColor(entry)) {
                    // Direction vers l'oppose
                    HexCell desired = sub(opposite, entry);
                    HexCell best = dir;
                    int bestDot = -1000000;
                    for (const auto& cand : bishopDirs) {
                        if (cand == neg(dir)) continue;
                        int d = dot(cand, desired);
                        if (d > bestDot) { bestDot = d; best = cand; }
                    }
                    return best;
                }
                // Couleur differente -> deflexion laterale (ci-dessous)
            }
        }
    }

    // Deflexion laterale : meme logique que rook mais sur le cycle des directions bishop
    HexCell entryEdge = neg(dir);
    int idx = -1;
    for (int i = 0; i < 6; ++i) {
        if (bishopDirs[i] == entryEdge) { idx = i; break; }
    }
    if (idx < 0) {
        // dir n'est pas une direction bishop standard -> dot fallback
        HexCell best = dir;
        int bestDot = -1000000;
        for (const auto& cand : bishopDirs) {
            if (cand == neg(dir)) continue;
            int d = dot(cand, entry);
            if (d > bestDot) { bestDot = d; best = cand; }
        }
        return best;
    }

    HexCell candA = bishopDirs[(idx + 1) % 6];
    HexCell candB = bishopDirs[(idx + 5) % 6];

    int a = dot(candA, entry);
    int b = dot(candB, entry);
    return (a >= b) ? candA : candB;
}

static inline HexCell rosetteRedirect(const HexCell& entry, HexCell dir, PieceType type) {
    if (type == PieceType::ROOK)
        return rookRedirect(entry, dir);
    if (type == PieceType::BISHOP)
        return bishopRedirect(entry, dir);
    if (type == PieceType::QUEEN) {
        static constexpr std::array<HexCell, 6> rookDirs = {
            HexCell{1,0}, HexCell{-1,0}, HexCell{0,1},
            HexCell{0,-1}, HexCell{1,-1}, HexCell{-1,1}
        };
        for (const auto& rd : rookDirs)
            if (rd == dir) return rookRedirect(entry, dir);
        return bishopRedirect(entry, dir);
    }
    return dir;
}

} // namespace rosette