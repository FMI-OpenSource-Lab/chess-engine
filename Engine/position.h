#ifndef POSITION_H
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

	// Info structure stores information needed to restore a position
	// to its previous state when a move is taken back
	struct Info
	{
		// copied when making a move
		CastlingRights castling;
		Square enpassant;
		PLY_TYPE rule_fifty;
		PLY_TYPE ply;

		// not copied
		Info* next;
		Info* previous;
		BITBOARD blockers_for_king_checks[BOTH];
		BITBOARD pinner_pieces[BOTH];
		Piece captured_piece;
		int repetition;
	};

	// List to keep track of position states along the setup
	// std::deque is used because pointers to elements are not invalidated when list resizing
	using InfoListPtr = std::unique_ptr<std::deque<Info>>;

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
		Position& set(const char* fen, Info* info);
		std::string get_fen() const;

		// Squares
		Square ep_square() const { return inf->enpassant; }
		template<PieceType pt>
		Square square(Color c) const { return getLS1B_square(get_pieces_bb(pt, c)); }
		Square castling_rook_square(CastlingRights cr) const { return rook_source_sq[cr]; }

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
		BITBOARD	get_blocking_pieces(Color c) const { return inf->blockers_for_king_checks[c]; };
		BITBOARD	get_pinned_pieces(Color c) const { return inf->pinner_pieces[c]; };
		BITBOARD	get_checked_squares(PieceType pt) const;

		// Booleans
		bool can_castle(CastlingRights cr) const;

		bool is_legal(Move m) const;
		bool is_capture(Move m) const;

		bool is_empty(Square s) const { return get_piece_on(s) == NO_PIECE; }
		bool is_draw(PLY_TYPE ply) const;
		bool has_repeated() const;
		bool gives_check(Move m) const;
		bool is_castling_prevented(CastlingRights cr) const;

		// Pieces
		Piece get_piece_on(Square s) const;
		Piece moved_piece(Move m) const { return get_piece_on(m.source_square()); };
		Piece captured_piece() const;

		// PLY
		PLY_TYPE game_ply() const { return gamePly; };
		PLY_TYPE rule_fifty_count() const { return inf->rule_fifty; };

		// Value
		Value non_pawn_material(Color c) const;
		Value non_pawn_material() const;
		Value see(Move m) const;

		// Doing and undoing moves
		void do_move(Move m, Info& newInfo);
		void do_move(Move m, Info& newInfo, bool gives_check);
		void undo_move(Move m);

		void remove_piece(Square s);
		void place_piece(Piece p, Square s);

		void update_blocks_and_pins(Color c) const;

		// Caslte & side
		CastlingRights castling_rights(Color c) const {
			return c & CastlingRights(inf->castling);
		}

		Color side_to_move() const { return side; }

		// State info
		Info* info() const;

		// Overrides
		friend std::ostream& operator<<(std::ostream& os, const Position& position);

	private:
		void set_castling_rights(Color c, Square r_source);
		template<bool Do>
		void do_castle(Color us, Square source, Square& target, Square& r_source, Square& r_target);

		void set_check_info() const;

		void move_piece(Square source, Square target);

		BITBOARD get_least_valuable_piece(BITBOARD attacks, Color by_side, PieceType& pt) const;

		// Data
		BITBOARD bitboards[NO_PIECE];
		BITBOARD occupancies[BOTH + 1];
		BITBOARD type[PIECE_TYPE_NB];
		BITBOARD castling_path[CASTLING_RIGHT_NB];

		Piece	 piece_board[SQUARE_TOTAL];

		uint8_t	 castling_rights_mask[NONE];

		Square rook_source_sq[CASTLING_RIGHT_NB];
		PLY_TYPE gamePly;
		Color	 side;

		// State info 
		Info* inf;
	};

	inline Piece Position::get_piece_on(Square s) const
	{
		assert(is_square_ok(s));
		return piece_board[s];
	}

	inline BITBOARD Position::get_pieces_bb(PieceType pt) const { return type[pt]; }

	inline bool Position::can_castle(CastlingRights cr) const
	{
		return inf->castling & cr;
	}

	inline BITBOARD Position::get_attackers_to(Square s) const
	{
		return get_attackers_to(s, get_all_pieces_bb());
	}

	inline bool Position::is_capture(Move m) const
	{
		return (!is_empty(m.target_square()) && m.move_type() != MT_CASTLING) || m.move_type() == MT_EN_PASSANT;
	}

	inline Piece Position::captured_piece() const { return inf->captured_piece; }

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

	inline void Position::do_move(Move m, Info& newInfo)
	{
		do_move(m, newInfo, gives_check(m));
	}

	inline Info* Position::info() const { return inf; }
}
#endif // !POSITION_H
