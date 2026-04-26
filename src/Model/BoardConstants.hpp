#pragma once
// Geometrie du roque sur le plateau Yalta 3 joueurs.
//
// Chaque roi ne partage une ligne droite qu'avec UNE seule tour (cote nord).
// Le roque cote dame (2e tour) n'est pas atteignable en ligne droite depuis
// le roi : la geometrie du plateau l'interdit.
//
//   PLAYER1 : Roi (4,3) -> (4,1)   Tour (4,0) -> (4,2)   [direction NORTH]
//   PLAYER2 : Roi (0,7) -> (0,5)   Tour (0,4) -> (0,6)   [direction NORTH]
//   PLAYER3 : Roi (8,11) -> (8,9)  Tour (8,8) -> (8,10)  [direction NORTH]
//
// Initialement bloque par le Cavalier et le Fou places entre roi et tour.
// King::getMoves() decouvre le roque par scan des 4 directions cardinales :
// aucune coordonnee n'est codee en dur dans King.cpp.
