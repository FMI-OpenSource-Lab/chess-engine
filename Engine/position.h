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
		BITBOARD _bitboards[NO_PIECE];
		BITBOARD _occupancies[BOTH + 1]; // +1 becase we include the 2 sides
		CastlingRights _castling;
		Color _side;
		Square _enpassant;
		PLY_TYPE _rule_fifty;
		PLY_TYPE _ply;

		// not copied
		Info* next;
		Info* previous;
		BITBOARD blocking_pieces[BOTH];
		BITBOARD pinning_pieces[BOTH];
		Piece captured_piece;
		int repetition;
	};

	// List to keep track of position states along the setup
	// std::deque is used because pointers to elements are not invalidated when list resizing
	using InfoListPtr = std::unique_ptr<std::deque<Info>>;

	// This class stores information to the board reperesentation as pieces,
	// side to move, castling info, etc.
	class _Position
	{
	public:
		//static void init();

		_Position() = default;
		_Position(const _Position&) = delete;
		_Position& operator=(const _Position&) = delete;

		// FEN i/o
		_Position& _set(const char* fen, Info* info);

		// Squares
		Square ep_square() const { return inf->_enpassant; }
		template<PieceType pt>
		Square square(Color c) const { return getLS1B_square(get_pieces_bb(pt, c)); }

		// Bitboards
		inline BITBOARD get_pieces_bb(PieceType pt = ALL_PIECES) const;
		inline BITBOARD get_pieces_bb(Piece p) const { return bitboards[p]; }
		inline BITBOARD get_pieces_bb(PieceType pt, Color c) const { return bitboards[get_piece(c, pt)]; }
		inline BITBOARD get_pieces_bb(Color c) const
		{
			return get_pieces_bb(PAWN, c) |
				get_pieces_bb(KNIGHT, c) |
				get_pieces_bb(BISHOP, c) |
				get_pieces_bb(ROOK, c) |
				get_pieces_bb(QUEEN, c) |
				get_pieces_bb(KING, c);
		}

		inline BITBOARD get_our_pieces_bb() const { return get_pieces_bb(side); }
		inline BITBOARD get_opponent_pieces_bb() const { return get_pieces_bb(~side); }
		inline BITBOARD get_all_pieces_bb() const { return occupancies[BOTH]; }
		inline BITBOARD get_all_empty_squares_bb() const { return ~occupancies[BOTH]; }

		BITBOARD	get_attackers_to(Square s) const;
		BITBOARD	get_attackers_to(Square s, BITBOARD occ) const;
		template<PieceType pt>
		BITBOARD	get_attacks_by(Color c) const;
		BITBOARD	get_blocking_pieces(Color c) const { return inf->blocking_pieces[c]; }
		BITBOARD	get_pinned_pieces(Color c) const { return inf->pinning_pieces[c]; }

		// Booleans
		// bool is_square_attacked(Square square, Color side_to_move) const;

		bool can_castle(CastlingRights cr) const;

		bool is_legal(Move m) const;
		bool is_capture(Move m) const;

		bool is_empty(Square s) const { return get_piece_on(s) == NO_PIECE; }
		bool is_draw(PLY_TYPE ply) const;
		bool has_repeated() const;
		bool gives_check(Move m) const;

		bool is_pos_ok() const;

		bool static_evaluation(Move m, int threshold = 0) const; // Static exchange evaluation

		// Pieces
		Piece moved_piece(Move m) const { return get_piece_on(m.source_square()); };
		Piece captured_piece() const;
		Piece get_piece_on(Square s) const { return piece_board[s]; }

		// PLY
		PLY_TYPE game_ply() const { return gamePly; };
		PLY_TYPE rule_fifty_count() const { return inf->_rule_fifty; };

		// Value
		Value non_pawn_material(Color c) const;
		Value non_pawn_material() const;

		// Doing and undoing moves
		void do_move(Move m, Info& newInfo);
		void do_move(Move m, Info& newInfo, bool gives_check);
		void undo_move(Move m);

		void remove_piece(Square s);
		void place_piece(Piece p, Square s);

		void update_blocks_and_pins(Color c) const;

		// Caslte & side
		CastlingRights	castling_rights(Color c) const
		{
			return c & CastlingRights(inf->_castling);
		}

		Color	 side_to_move() const { return side; }

		// State info
		Info* info() const;

		// Overrides
		friend std::ostream& operator<<(std::ostream& os, const _Position& position);

	private:
		template<bool>
		void caslte(Color us, Square source, Square& target, Square& r_source, Square& r_target);

		void set_info();
		void set_check_info();

		void move_piece(Square source, Square target);

		// Data
		BITBOARD bitboards[NO_PIECE];
		BITBOARD occupancies[BOTH + 1];
		BITBOARD type[PIECE_TYPE_NB];

		Piece	 piece_board[SQUARE_TOTAL];

		uint8_t	 castling_rights_mask[NONE];

		PLY_TYPE gamePly;
		Color	 side;

		// State info 
		Info* inf;
	};

	inline BITBOARD _Position::get_pieces_bb(PieceType pt) const { return type[pt]; }

	inline bool _Position::can_castle(CastlingRights cr) const
	{
		return inf->_castling & cr;
	}

	inline BITBOARD _Position::get_attackers_to(Square s) const
	{
		return get_attackers_to(s, get_all_pieces_bb());
	}

	inline bool _Position::is_capture(Move m) const
	{
		return (!is_empty(m.target_square()) && m.move_type() != MT_CASTLING) || m.move_type() == MT_EN_PASSANT;
	}

	inline Piece _Position::captured_piece() const { return inf->captured_piece; }

	inline void _Position::place_piece(Piece p, Square s)
	{
		piece_board[s] = p;

		set_bit(bitboards[p], s);
		set_bit(occupancies[get_piece_color(p)], s);

		occupancies[BOTH] |= occupancies[WHITE] | occupancies[BLACK];
		type[ALL_PIECES] |= type[type_of_piece(p)] |= s;

		occupancies[get_piece_color(p)] |= s;
	}

	inline void _Position::remove_piece(Square s)
	{
		Piece p = piece_board[s];

		type[ALL_PIECES] ^= s;

		type[type_of_piece(p)] ^= s;
		occupancies[get_piece_color(p)] ^= s;
		occupancies[BOTH] ^= s;
		piece_board[s] = NO_PIECE;
	}

	inline void _Position::move_piece(Square source, Square target)
	{
		Piece s_p = piece_board[source];
		BITBOARD dest = square_to_BB(source) | square_to_BB(target);

		type[ALL_PIECES] ^= dest;
		type[type_of_piece(s_p)] ^= dest;
		occupancies[get_piece_color(s_p)] ^= dest;

		piece_board[source] = NO_PIECE;
		piece_board[target] = s_p;
	}

	inline void _Position::do_move(Move m, Info& newInfo)
	{
		do_move(m, newInfo, gives_check(m));
	}

	inline Info* _Position::info() const { return inf; }

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
