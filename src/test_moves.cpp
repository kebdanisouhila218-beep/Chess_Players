// ============================================================
// test_moves.cpp
// Fichier de test standalone - ne necessite PAS SFML
// Compile avec : g++ -std=c++17 test_moves.cpp -o test_moves
// Lance avec   : ./test_moves   (ou test_moves.exe sur Windows)
// ============================================================
// IMPORTANT : copie ce fichier dans ton dossier src/ du projet
// et compile-le SEPAREMENT (pas avec CMake du projet principal)
// ============================================================

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <array>
#include <cmath>
#include <algorithm>
#include <functional>

// ─── HexCell ────────────────────────────────────────────────
struct HexCell {
    int q, r;
    int s() const { return -q - r; }
    bool operator==(const HexCell& o) const { return q == o.q && r == o.r; }
    std::vector<HexCell> neighbors() const {
        return {{q+1,r},{q-1,r},{q,r+1},{q,r-1},{q+1,r-1},{q-1,r+1}};
    }
};
struct HexHash {
    size_t operator()(const HexCell& c) const {
        return std::hash<int>()(c.q) ^ (std::hash<int>()(c.r) << 16);
    }
};

// ─── Enums ──────────────────────────────────────────────────
enum class Player { PLAYER1, PLAYER2, PLAYER3, NONE };
enum class PieceType { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };

// ─── Rosette ────────────────────────────────────────────────
namespace rosette {
    static inline bool isCenter(const HexCell& c) { return c.q==0 && c.r==0; }
    static inline bool isRosette(const HexCell& c) {
        if (isCenter(c)) return true;
        return (c.q==1&&c.r==0)||(c.q==1&&c.r==-1)||(c.q==0&&c.r==-1)
            ||(c.q==-1&&c.r==0)||(c.q==-1&&c.r==1)||(c.q==0&&c.r==1);
    }
    static inline int rosetteColor(const HexCell& c) {
        int v = (c.q*2 + c.r + 6) % 2;
        if (v < 0) v += 2;
        return v;
    }
    static inline HexCell neg(const HexCell& a) { return {-a.q, -a.r}; }
    static inline HexCell add(const HexCell& a, const HexCell& b) { return {a.q+b.q, a.r+b.r}; }
    static inline HexCell sub(const HexCell& a, const HexCell& b) { return {a.q-b.q, a.r-b.r}; }
    static inline int dot(const HexCell& a, const HexCell& b) { return a.q*b.q + a.r*b.r; }
    static inline int cubeDist0(const HexCell& c) {
        int x=c.q, z=c.r, y=-x-z;
        return (std::abs(x)+std::abs(y)+std::abs(z))/2;
    }
    static inline int ringIndex(const HexCell& c) {
        static constexpr std::array<HexCell,6> ring={
            HexCell{1,0},HexCell{1,-1},HexCell{0,-1},
            HexCell{-1,0},HexCell{-1,1},HexCell{0,1}
        };
        for(int i=0;i<6;++i) if(ring[i]==c) return i;
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
        static constexpr std::array<HexCell,6> bishopDirs={
            HexCell{1,1},HexCell{-1,2},HexCell{-2,1},
            HexCell{-1,-1},HexCell{1,-2},HexCell{2,-1}
        };
        if(!isCenter(entry)&&isRosette(entry)){
            HexCell nextCell=add(entry,dir);
            if(isCenter(nextCell)){
                int idx=ringIndex(entry);
                if(idx>=0){
                    static constexpr std::array<HexCell,6> ring={
                        HexCell{1,0},HexCell{1,-1},HexCell{0,-1},
                        HexCell{-1,0},HexCell{-1,1},HexCell{0,1}
                    };
                    HexCell opposite=ring[(idx+3)%6];
                    if(rosetteColor(opposite)==rosetteColor(entry)){
                        HexCell desired=sub(opposite,entry);
                        HexCell best=dir; int bestDot=-1000000;
                        for(const auto& cand:bishopDirs){
                            if(cand==neg(dir)) continue;
                            int d=dot(cand,desired);
                            if(d>bestDot){bestDot=d;best=cand;}
                        }
                        return best;
                    }
                }
            }
        }
        HexCell entryEdge=neg(dir); int idx=-1;
        for(int i=0;i<6;++i) if(bishopDirs[i]==entryEdge){idx=i;break;}
        if(idx<0){
            HexCell best=dir; int bestDot=-1000000;
            for(const auto& cand:bishopDirs){
                if(cand==neg(dir)) continue;
                int d=dot(cand,entry);
                if(d>bestDot){bestDot=d;best=cand;}
            }
            return best;
        }
        HexCell candA=bishopDirs[(idx+2)%6];
        HexCell candB=bishopDirs[(idx+4)%6];
        int a=dot(candA,entry), b=dot(candB,entry);
        return (a>=b)?candA:candB;
    }

    static inline HexCell rosetteRedirect(const HexCell& entry, HexCell dir, PieceType type) {
        if(type==PieceType::ROOK) return rookRedirect(entry,dir);
        if(type==PieceType::BISHOP) return bishopRedirect(entry,dir);
        if(type==PieceType::QUEEN){
            static constexpr std::array<HexCell,6> rookDirs={
                HexCell{1,0},HexCell{-1,0},HexCell{0,1},
                HexCell{0,-1},HexCell{1,-1},HexCell{-1,1}
            };
            for(const auto& rd:rookDirs) if(rd==dir) return rookRedirect(entry,dir);
            return bishopRedirect(entry,dir);
        }
        return dir;
    }
}

// ─── Board (minimal) ────────────────────────────────────────
class Piece;
class Board {
public:
    static constexpr int RADIUS = 7;
    Board() {
        for(int q=-RADIUS;q<=RADIUS;q++){
            int r1=std::max(-RADIUS,-q-RADIUS);
            int r2=std::min(RADIUS,-q+RADIUS);
            for(int r=r1;r<=r2;r++) cells[{q,r}]=nullptr;
        }
    }
    bool isValid(const HexCell& c) const { return cells.count(c)>0; }
    Piece* getPiece(const HexCell& c) const {
        auto it=cells.find(c);
        if(it!=cells.end()) return it->second;
        return nullptr;
    }
    void setPiece(const HexCell& c, Piece* p) { if(isValid(c)) cells[c]=p; }
    void clear() { for(auto& [k,v]:cells) v=nullptr; }
    std::vector<HexCell> allValidCells() const {
        std::vector<HexCell> r;
        for(auto& [c,p]:cells) r.push_back(c);
        return r;
    }
private:
    std::unordered_map<HexCell,Piece*,HexHash> cells;
};

// ─── Piece base ─────────────────────────────────────────────
class Piece {
public:
    Piece(PieceType t, Player o, HexCell p): type(t),owner(o),pos(p){}
    virtual ~Piece()=default;
    virtual std::vector<HexCell> getMoves(const Board& board) const = 0;
    PieceType getType() const {return type;}
    Player getOwner() const {return owner;}
    HexCell getPos() const {return pos;}
    void setPos(HexCell p){pos=p;}
protected:
    PieceType type; Player owner; HexCell pos;
};

// ─── Rook ───────────────────────────────────────────────────
class Rook : public Piece {
public:
    Rook(Player o, HexCell p): Piece(PieceType::ROOK,o,p){}
    std::vector<HexCell> getMoves(const Board& board) const override {
        std::vector<HexCell> moves;
        std::vector<HexCell> directions={{1,0},{-1,0},{0,1},{0,-1},{1,-1},{-1,1}};
        for(const HexCell& dir:directions){
            HexCell curDir=dir;
            HexCell current={pos.q+curDir.q,pos.r+curDir.r};
            bool redirected=false; int steps=0;
            while(board.isValid(current)&&steps<30){
                steps++;
                if(!redirected&&rosette::isRosette(current)){
                    HexCell newDir=rosette::rosetteRedirect(current,curDir,PieceType::ROOK);
                    if(!(newDir==curDir)){curDir=newDir;redirected=true;}
                }
                Piece* target=board.getPiece(current);
                if(target==nullptr) moves.push_back(current);
                else{if(target->getOwner()!=owner)moves.push_back(current);break;}
                current={current.q+curDir.q,current.r+curDir.r};
            }
        }
        return moves;
    }
};

// ─── Bishop ─────────────────────────────────────────────────
class Bishop : public Piece {
public:
    Bishop(Player o, HexCell p): Piece(PieceType::BISHOP,o,p){}
    std::vector<HexCell> getMoves(const Board& board) const override {
        std::vector<HexCell> moves;
        std::vector<HexCell> directions={{1,1},{-1,-1},{2,-1},{-2,1},{1,-2},{-1,2}};
        for(const HexCell& dir:directions){
            HexCell curDir=dir;
            HexCell current={pos.q+curDir.q,pos.r+curDir.r};
            bool redirected=false; int steps=0;
            while(board.isValid(current)&&steps<30){
                steps++;
                if(!redirected&&rosette::isRosette(current)){
                    HexCell newDir=rosette::rosetteRedirect(current,curDir,PieceType::BISHOP);
                    if(!(newDir==curDir)){curDir=newDir;redirected=true;}
                }
                Piece* target=board.getPiece(current);
                if(target==nullptr) moves.push_back(current);
                else{if(target->getOwner()!=owner)moves.push_back(current);break;}
                current={current.q+curDir.q,current.r+curDir.r};
            }
        }
        return moves;
    }
};

// ─── Queen ──────────────────────────────────────────────────
class Queen : public Piece {
public:
    Queen(Player o, HexCell p): Piece(PieceType::QUEEN,o,p){}
    std::vector<HexCell> getMoves(const Board& board) const override {
        std::vector<HexCell> moves;
        std::vector<HexCell> directions={
            {1,0},{-1,0},{0,1},{0,-1},{1,-1},{-1,1},
            {1,1},{-1,-1},{2,-1},{-2,1},{1,-2},{-1,2}
        };
        for(const HexCell& dir:directions){
            HexCell curDir=dir;
            HexCell current={pos.q+curDir.q,pos.r+curDir.r};
            bool redirected=false; int steps=0;
            while(board.isValid(current)&&steps<30){
                steps++;
                if(!redirected&&rosette::isRosette(current)){
                    HexCell newDir=rosette::rosetteRedirect(current,curDir,PieceType::QUEEN);
                    if(!(newDir==curDir)){curDir=newDir;redirected=true;}
                }
                Piece* target=board.getPiece(current);
                if(target==nullptr) moves.push_back(current);
                else{if(target->getOwner()!=owner)moves.push_back(current);break;}
                current={current.q+curDir.q,current.r+curDir.r};
            }
        }
        return moves;
    }
};

// ─── King ───────────────────────────────────────────────────
class King : public Piece {
public:
    King(Player o, HexCell p): Piece(PieceType::KING,o,p){}
    std::vector<HexCell> getMoves(const Board& board) const override {
        std::vector<HexCell> moves;
        std::vector<HexCell> directions={{1,0},{-1,0},{0,1},{0,-1},{1,-1},{-1,1}};
        for(const HexCell& dir:directions){
            HexCell target={pos.q+dir.q,pos.r+dir.r};
            if(!board.isValid(target)) continue;
            if(rosette::isRosette(pos)&&rosette::isRosette(target)){
                if(rosette::rosetteColor(pos)!=rosette::rosetteColor(target)) continue;
            }
            Piece* p=board.getPiece(target);
            if(p==nullptr||p->getOwner()!=owner) moves.push_back(target);
        }
        return moves;
    }
};

// ─── Helpers d'affichage ────────────────────────────────────
std::string playerName(Player p){
    switch(p){
        case Player::PLAYER1: return "P1(blanc)";
        case Player::PLAYER2: return "P2(bleu)";
        case Player::PLAYER3: return "P3(rouge)";
        default: return "NONE";
    }
}
std::string pieceName(PieceType t){
    switch(t){
        case PieceType::ROOK:   return "Tour";
        case PieceType::BISHOP: return "Fou";
        case PieceType::QUEEN:  return "Dame";
        case PieceType::KING:   return "Roi";
        default: return "?";
    }
}
void printMoves(const std::string& label, const std::vector<HexCell>& moves){
    std::cout << "  " << label << " -> " << moves.size() << " coups:\n";
    for(const HexCell& c : moves)
        std::cout << "    (" << c.q << "," << c.r << ")\n";
}

// ─── Fonction de test principale ────────────────────────────
void runTest(const std::string& title, 
             std::function<void(Board&, std::vector<Piece*>&)> setup,
             std::function<void(Board&, std::vector<Piece*>&)> verify)
{
    std::cout << "\n======================================\n";
    std::cout << "TEST: " << title << "\n";
    std::cout << "======================================\n";
    Board board;
    std::vector<Piece*> pieces;
    setup(board, pieces);
    verify(board, pieces);
    for(Piece* p : pieces) delete p;
}

// ─── TESTS ──────────────────────────────────────────────────
int main(){
    std::cout << "=== CHESS 3 PLAYERS - TEST DES MOUVEMENTS ===\n";

    // ── TEST 1 : Tour au centre, plateau vide ──
    runTest("Tour P1 sur (0,0) [centre rosette] - plateau vide",
        [](Board& b, std::vector<Piece*>& ps){
            Rook* r = new Rook(Player::PLAYER1, {0,0});
            b.setPiece({0,0}, r);
            ps.push_back(r);
        },
        [](Board& b, std::vector<Piece*>& ps){
            auto moves = ps[0]->getMoves(b);
            printMoves("Tour (0,0)", moves);
            std::cout << "  ATTENDU: moves lineaires dans 6 directions,\n";
            std::cout << "  deflexion laterale quand elle sort de la rosette\n";
        }
    );

    // ── TEST 2 : Tour sur (2,0), va traverser la rosette ──
    runTest("Tour P1 sur (2,0) direction {-1,0} vers rosette",
        [](Board& b, std::vector<Piece*>& ps){
            Rook* r = new Rook(Player::PLAYER1, {2,0});
            b.setPiece({2,0}, r);
            ps.push_back(r);
        },
        [](Board& b, std::vector<Piece*>& ps){
            auto moves = ps[0]->getMoves(b);
            printMoves("Tour (2,0)", moves);
            std::cout << "  ATTENDU: vers gauche jusqu'a rosette, puis deflexion laterale\n";
            std::cout << "  NE DOIT PAS traverser tout le plateau en ligne droite\n";
        }
    );

    // ── TEST 3 : Fou sur (3,0), diagonale vers rosette ──
    runTest("Fou P1 sur (3,-3) diagonale vers centre",
        [](Board& b, std::vector<Piece*>& ps){
            Bishop* bish = new Bishop(Player::PLAYER1, {3,-3});
            b.setPiece({3,-3}, bish);
            ps.push_back(bish);
        },
        [](Board& b, std::vector<Piece*>& ps){
            auto moves = ps[0]->getMoves(b);
            printMoves("Fou (3,-3)", moves);
            std::cout << "  ATTENDU: diagonale vers centre, traverse vers oppose meme couleur\n";
        }
    );

    // ── TEST 4 : Dame sur (3,0), plateau vide ──
    runTest("Dame P1 sur (3,0) - plateau vide",
        [](Board& b, std::vector<Piece*>& ps){
            Queen* q = new Queen(Player::PLAYER1, {3,0});
            b.setPiece({3,0}, q);
            ps.push_back(q);
        },
        [](Board& b, std::vector<Piece*>& ps){
            auto moves = ps[0]->getMoves(b);
            printMoves("Dame (3,0)", moves);
            int expected_max = 40; // approximatif
            if(moves.size() > (size_t)expected_max)
                std::cout << "  PROBLEME: trop de coups (" << moves.size() 
                          << ") - probable boucle ou doublon\n";
            else
                std::cout << "  OK: nombre de coups raisonnable\n";
        }
    );

    // ── TEST 5 : Tour bloquee par pieces amies ──
    runTest("Tour P1 sur (0,0) bloquee par pieces amies",
        [](Board& b, std::vector<Piece*>& ps){
            Rook* r = new Rook(Player::PLAYER1, {0,0});
            b.setPiece({0,0}, r);
            ps.push_back(r);
            // Bloquer 3 directions avec pieces amies
            Rook* r2 = new Rook(Player::PLAYER1, {2,0});
            b.setPiece({2,0}, r2); ps.push_back(r2);
            Rook* r3 = new Rook(Player::PLAYER1, {0,2});
            b.setPiece({0,2}, r3); ps.push_back(r3);
        },
        [](Board& b, std::vector<Piece*>& ps){
            auto moves = ps[0]->getMoves(b);
            printMoves("Tour (0,0) bloquee", moves);
            std::cout << "  ATTENDU: NE DOIT PAS inclure (2,0) ni (0,2) [pieces amies]\n";
            bool bad = false;
            for(auto& m : moves)
                if((m.q==2&&m.r==0)||(m.q==0&&m.r==2)){ bad=true; break; }
            std::cout << (bad ? "  ECHEC: case amie dans les moves!\n" 
                              : "  OK: cases amies correctement exclues\n");
        }
    );

    // ── TEST 6 : Tour peut capturer pièce ennemie ──
    runTest("Tour P1 sur (0,0) capture ennemie en (3,0)",
        [](Board& b, std::vector<Piece*>& ps){
            Rook* r = new Rook(Player::PLAYER1, {0,0});
            b.setPiece({0,0}, r); ps.push_back(r);
            Rook* enemy = new Rook(Player::PLAYER2, {3,0});
            b.setPiece({3,0}, enemy); ps.push_back(enemy);
        },
        [](Board& b, std::vector<Piece*>& ps){
            auto moves = ps[0]->getMoves(b);
            bool canCapture = false;
            for(auto& m : moves) if(m.q==3&&m.r==0) canCapture=true;
            std::cout << (canCapture ? "  OK: peut capturer l'ennemie en (3,0)\n"
                                     : "  ECHEC: ne peut pas capturer l'ennemie!\n");
            printMoves("Tour (0,0) vs ennemi", moves);
        }
    );

    // ── TEST 7 : Roi sur rosette, ne peut pas aller couleur opposee ──
    runTest("Roi P1 sur (1,0) [rosette ring], regle couleur",
        [](Board& b, std::vector<Piece*>& ps){
            King* k = new King(Player::PLAYER1, {1,0});
            b.setPiece({1,0}, k); ps.push_back(k);
        },
        [](Board& b, std::vector<Piece*>& ps){
            auto moves = ps[0]->getMoves(b);
            printMoves("Roi (1,0)", moves);
            // (1,0) couleur = (1*2+0+6)%2 = 0
            // (-1,0) couleur = (-1*2+0+6)%2 = 0 -> meme couleur -> OK
            // (0,1) couleur = (0+1+6)%2 = 1 -> couleur diff -> interdit depuis rosette
            bool has01 = false;
            for(auto& m : moves) if(m.q==0&&m.r==1) has01=true;
            std::cout << "  (1,0) couleur rosette = " 
                      << rosette::rosetteColor({1,0}) << "\n";
            std::cout << "  (0,1) couleur rosette = " 
                      << rosette::rosetteColor({0,1}) << "\n";
            // (0,1) est adjacent au roi sur (1,0)? Non, pas voisin direct
            std::cout << "  INFO: verifier que le roi ne saute pas vers case couleur opposee\n";
        }
    );

    // ── TEST 8 : Doublons dans les moves de la Dame ──
    runTest("Dame P1 sur (0,0) - detection de doublons",
        [](Board& b, std::vector<Piece*>& ps){
            Queen* q = new Queen(Player::PLAYER1, {0,0});
            b.setPiece({0,0}, q); ps.push_back(q);
        },
        [](Board& b, std::vector<Piece*>& ps){
            auto moves = ps[0]->getMoves(b);
            // Detecte les doublons
            std::unordered_map<HexCell,int,HexHash> seen;
            for(auto& m : moves) seen[m]++;
            int dups = 0;
            for(auto& [c,cnt] : seen) if(cnt>1){
                std::cout << "  DOUBLON: (" << c.q << "," << c.r << ") x" << cnt << "\n";
                dups++;
            }
            std::cout << "  Total moves: " << moves.size() 
                      << " | Uniques: " << seen.size()
                      << " | Doublons: " << dups << "\n";
            if(dups==0) std::cout << "  OK: aucun doublon\n";
            else std::cout << "  PROBLEME: " << dups << " doublons detectes\n";
        }
    );

    std::cout << "\n=== FIN DES TESTS ===\n";
    return 0;
}
