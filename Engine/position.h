﻿#ifndef POSITION_H
#define POSITION_H

#include <vector>
#include <memory>
#include <deque>
#include <string>

#include "defs.h"
#include "move.h"
#include "movegen.h"

namespace ChessEngine
{
	// Empty fen string
	static const char* EMPTY_FEN = "8/8/8/8/8/8/8/8 b - - ";

	// Starting fen string
	static const char* START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

	// Some example fen string
	static const char* TEST_FEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ";
	static const char* TEST_ATTACKS_FEN = "8/8/8/3PN3/8/8/3p4/8 w - - ";

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

	extern char ascii_pieces[13];

	// char unicode_pieces[12] = { '♙','♘','♗','♖','♕','♔','♟','♞','♝','♜','♛','♚' };

	using BITBOARD = U64;

	enum class Movegen: uint16_t
	{
		ALL,
		QSEARCH,
		QUIET,
		CAPTURE
	};

	struct ScoredMove
	{
		Move move = NO_MOVE;
		int score = 0;
	};

	// Info structure stores information needed to restore a position
	// to its previous state when a move is taken back
	struct MoveInfo
	{
		// Copied when making a move
		CastlingRights castling_rights = CASTLE_NB;
		Square en_passant = NONE;
		PLY_TYPE fifty_move = 0;
		Piece captured_piece = NO_PIECE;
		BITBOARD threats = 0ULL;
		BITBOARD pinned_pieces = 0ULL;
	};

	// List to keep track of position states along the setup
	// std::deque is used because pointers to elements are not invalidated when list resizing
	using InfoListPtr = std::unique_ptr<std::deque<MoveInfo>>;

	// This class stores information to the board reperesentation as pieces,
	// side to move, castling info, etc.
	class Position
	{
	public:
		//static void init();

		Position() = default;
		// Delete the copy constructor
		Position(const Position&) = delete;
		// Delete the copy assignment operator
		Position& operator=(const Position&) = delete;

		// FEN i/o
		Position& set(const char* fen);
		std::string get_fen() const;

		// Squares
		Square ep_square() const { return enpassant_square; }
		template<PieceType pt>
		Square square(Color c) const { return getLS1B(get_pieces_bb(pt, c)); }
		Square castling_rook_square(CastlingRights cr) const { return rook_source_sq[cr]; }

		// Bitboards
		inline BITBOARD get_pieces_bb(PieceType pt = ALL_PIECES) const;
		inline BITBOARD get_pieces_bb(Piece p) const { return bitboards[p]; }
		inline BITBOARD get_pieces_bb(PieceType pt, Color c) const { return bitboards[get_piece(c, pt)]; }
		inline BITBOARD get_pieces_bb(Color c) const { return occupancies[c]; }

		inline BITBOARD get_our_pieces_bb() const { return occupancies[side]; }
		inline BITBOARD get_opponent_pieces_bb() const { return occupancies[~side]; }
		inline BITBOARD get_all_pieces_bb() const { return occupancies[BOTH]; }
		inline BITBOARD get_all_empty_squares_bb() const { return ~occupancies[BOTH]; }

		BITBOARD	get_attackers_to(Square s) const;
		BITBOARD	get_attackers_to(Square s, BITBOARD occ) const;

		template<PieceType pt>
		BITBOARD	get_attacks_by(Color c) const;
		BITBOARD	get_checked_squares(PieceType pt) const;
		BITBOARD	get_threats() const { return threats; }

		// Booleans
		bool is_empty(Square s) const { return get_piece_on(s) == NO_PIECE; }
		bool gives_check(Move m) const;
		bool can_castle(CastlingRights cr) const { return castle & cr;}

		// Pieces
		Piece get_piece_on(Square s) const;
		Piece moved_piece(Move m) const { return get_piece_on(m.source_square()); };
		Piece captured_piece() const;

		// PLY
		PLY_TYPE game_ply() const { return fullmove_number; };
		PLY_TYPE rule_fifty_count() const { return rule_fifty; };

		// Value
		Value see(Move m) const;

		// Doing and undoing moves
		void do_move(Move m, MoveInfo& new_info);
		void do_move(Move m, MoveInfo& new_info, bool gives_check);
		void undo_move(Move m, MoveInfo& new_info);

		void remove_piece(Square s);
		void place_piece(Piece p, Square s);
		void calculate_threats();
		void clear_mi_stack(); // clear the move info stack
		void set_mi_stack(MoveInfo& mi, PLY_TYPE fifty_move); // set the move info stack

		// Caslte & side
		CastlingRights castling_rights(Color c) const {
			return c & CastlingRights(castle);
		}

		Color side_to_move() const { return side; }

		// Overrides
		friend std::ostream& operator<<(std::ostream& os, const Position& position);

	private:
		void set_castling_rights(Color c, Square r_source);
		template<bool Do>
		void do_castle(Color us, Square source, Square& target, Square& r_source, Square& r_target);

		void move_piece(Square source, Square target);

		BITBOARD get_least_valuable_piece(BITBOARD attacks, Color by_side, PieceType& pt) const;

		// Data
		BITBOARD bitboards[NO_PIECE]{};
		BITBOARD occupancies[BOTH + 1]{};
		BITBOARD type[PIECE_TYPE_NB]{};
		BITBOARD castling_path[CASTLING_RIGHT_NB]{};
		BITBOARD threats{};
		BITBOARD pinned_pieces{};
		BITBOARD blocking_pieces{};

		Piece	 piece_board[SQUARE_TOTAL]{};
		Piece	 captured{};

		uint8_t	 castling_rights_mask[NONE]{};

		Square rook_source_sq[CASTLING_RIGHT_NB]{};
		Square enpassant_square{};
		
		CastlingRights castle{};
		
		Color	 side{};

		PLY_TYPE fullmove_number{};
		PLY_TYPE rule_fifty{};
		PLY_TYPE repetition{};

		// State info 
		//std::array<MoveInfo, TOTAL_MAX_DEPTH> move_info;
		MoveInfo move_info[TOTAL_MAX_DEPTH];
	};

	inline Piece Position::get_piece_on(Square s) const
	{
		assert(is_square_ok(s));
		return piece_board[s];
	}

	inline BITBOARD Position::get_pieces_bb(PieceType pt) const { return type[pt]; }

	inline BITBOARD Position::get_attackers_to(Square s) const
	{
		return get_attackers_to(s, get_all_pieces_bb());
	}

	inline Piece Position::captured_piece() const { return captured; }

	inline void Position::place_piece(Piece p, Square s)
	{
		piece_board[s] = p;

		set_bit(bitboards[p], s);
		set_bit(occupancies[get_piece_color(p)], s);

		occupancies[BOTH] |= occupancies[WHITE] | occupancies[BLACK];
		type[ALL_PIECES] |= type[type_of_piece(p)] |= s;

		set_bit(occupancies[get_piece_color(p)], s);
	}

	inline void Position::remove_piece(Square s)
	{
		Piece p = piece_board[s];

		rm_bit(type[ALL_PIECES], s);
		rm_bit(type[type_of_piece(p)], s);
		rm_bit(occupancies[get_piece_color(p)], s);
		rm_bit(occupancies[BOTH], s);

		piece_board[s] = NO_PIECE;
	}

	inline void Position::move_piece(Square source, Square target)
	{
		Piece s_p = piece_board[source];
		BITBOARD dest = square_to_BB(source) | square_to_BB(target);

		type[ALL_PIECES] ^= dest;
		type[type_of_piece(s_p)] ^= dest;
		occupancies[get_piece_color(s_p)] ^= dest;

		piece_board[source] = NO_PIECE;
		piece_board[target] = s_p;
	}

	inline void Position::do_move(Move m, MoveInfo& new_info)
	{
		do_move(m, new_info, gives_check(m));
	}
}
#endif // !POSITION_H
