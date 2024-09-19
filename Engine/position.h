﻿#ifndef POSITION_H
#define POSITION_H

#include "defs.h"
#include "move.h"
#include "movegen.h"

#include <vector>
#include <memory>
#include <deque>
#include <string>

namespace ChessEngine
{
	// Empty fen string
	static const char* EMPTY_FEN = "8/8/8/8/8/8/8/8 b - - ";

	// Starting fen string
	static const char* START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

	// Some example fen string
	static const char* TEST_FEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ";
	static const char* TEST_ATTACKS_FEN = "8/8/8/3PN3/8/8/3p4/8 w - - ";

	// side to move
	extern Color side;

	// en passant square
	extern Square enpassant;

	/*
		binary representation of castling rights

		bin		dec		description

		0001	1		white king can castle to the king side
		0010	2		white king can castle to the queen gside
		0100	4		black king can castle to the king side
		1000	8		black king can castle to the queen side
	*/

	/*
		examples

		1111 both can castle both directions

		1001	white king => king side
				black king => queen side
	*/

	// castling bit
	extern CastlingRights castle;

	extern char ascii_pieces[13];

	// char unicode_pieces[12] = { '♙','♘','♗','♖','♕','♔','♟','♞','♝','♜','♛','♚' };

	// piece bitboards
	extern U64 bitboards[12];
	extern U64 occupancies[3];

	using BITBOARD = U64;

	// Info structure stores information needed to restore a position
	// to its previous state when a move is taken back
	struct Info
	{
		// copied when making a move
		CastlingRights _castling;
		Square _enpassant;
		PLY_TYPE _rule_fifty;
		PLY_TYPE _ply;

		// not copied
		Info* next;
		Info* previous;
		Piece captured_piece;
		int repetition;
		BITBOARD checks_bb;
		BITBOARD pins_bb;
		BITBOARD blocks_for_king_bb[BOTH];
		BITBOARD checks_squares[PIECE_TYPE_NB];
	};

	// List to keep track of position states along the setup
	// std::deque is used because pointers to elements are not invalidated when list resizing
	using InfoListPtr = std::unique_ptr<std::deque<Info>>;

	// This class stores information to the board reperesentation as pieces,
	// side to move, castling info, etc.
	class _Position
	{
	public:
		static void init();

		_Position() = default;

		// FEN i/o
		_Position& set(const char* fen, Info* info);
		std::string fen() const;
		void print_board();

		// Representation
		inline BITBOARD get_pieces(Piece p) const { return bitboards[p]; }
		inline BITBOARD get_pieces(PieceType pt, Color c) const { return bitboards[get_piece(c, pt)]; }
		inline BITBOARD get_pieces(Color c) const
		{
			return get_pieces(PAWN, c) |
				get_pieces(KNIGHT, c) |
				get_pieces(BISHOP, c) |
				get_pieces(ROOK, c) |
				get_pieces(QUEEN, c) |
				get_pieces(KING, c);
		}

		inline BITBOARD get_our_pieces() const { return get_pieces(side); }
		inline BITBOARD get_opponent_pieces() const { return get_pieces(~side); }
		inline BITBOARD get_all_pieces() const { return occupancies[BOTH]; }
		inline BITBOARD get_all_empty_squares() const { return ~occupancies[BOTH]; }

		// Checking
		BITBOARD checks() const;
		BITBOARD blocks_for_king(Color c) const;
		BITBOARD check_squares(PieceType pt) const;
		BITBOARD pins(Color c) const;

		// Attacks
		BITBOARD	get_attackers_to(Square s) const;
		BITBOARD	get_attackers_to(Square s, BITBOARD occ) const;
		template<PieceType pt>
		BITBOARD	get_attacks_by(Color c) const;
		Square		get_king_pos(Color c) const;
		BITBOARD	check_squares(Square s) const;
		BITBOARD	pinners(Color c) const;
		bool		is_square_attacked(Square square, Color side_to_move) const;

		// State info
		Info* info() const;

		// Castling
		bool			can_castle(CastlingRights cr) const;
		CastlingRights	castling_rights(Color c) const;
		Square			castling_rook_square(CastlingRights cr) const;

		// Move properties
		bool is_legal(Move m) const;
		bool is_pseudo_legal(Move m) const;
		bool is_capture(Move m) const;
		bool gives_check(Move m) const;
		Piece moved_piece(Move m) const;
		Piece captured_piece() const;

		// Position properties
		Square	 ep_square() const;
		Piece	 get_piece_on(Square s) const { return piece_board[s]; }
		Color	 side_to_move() const { return side; }
		bool	 is_empty(Square s) const { return get_piece_on(s) == NONE; }
		bool	 is_draw(PLY_TYPE ply) const;
		bool	 has_repeated() const;
		PLY_TYPE game_ply() const;
		PLY_TYPE rule_fifty_count() const;

		Value non_pawn_material(Color c) const;
		Value non_pawn_material() const;

		// Debugging position
		bool is_pos_ok() const;

		// Doing and undoing moves
		void do_move(Move m, Info& newInfo);
		void do_move(Move m, Info& newInfo, bool gives_check);
		void undo_move(Move m);

		void remove_piece(Square s);
		void place_piece(Piece p, Square s);

		// Static exchange eval
		bool see(Move m, int threshold = 0) const;

		friend std::ostream& operator<<(std::ostream& os, const _Position& position);

	private:
		void set_castling_right(Color c, Square r_source);
		template<bool will>
		void caslte(Color us, Square source, Square& target, Square& r_source, Square& r_target);
		
		void set_info();
		void set_check_info();

		void move_piece(Square source, Square target);

		// Data
		BITBOARD bitboards[12];
		BITBOARD occupancies[3];
		BITBOARD type[PIECE_TYPE_NB];

		int		 piece_count[PIECE_TYPE_NB];
		uint8_t	 castling_rights_mask[NONE];
		Square	 castling_r_square[CASTLING_RIGHT_NB];
		Piece	 piece_board[SQUARE_TOTAL];

		PLY_TYPE gamePly;
		Color	 side;
		
		// State info 
		Info* _info;
	};

	// check if square is attacked
	extern bool is_square_attacked(const Square& square, const Color color);
	extern void print_attacked_squares(Color c);

	namespace Position
	{
		void init(const char* fen);
	}

	// fen string input output
	extern void set(const char* fen);

	// Board representation
	extern void print_board();

	// Helper methods
	extern Piece get_piece(const char& symbol);
}
#endif // !POSITION_H
