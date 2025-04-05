#pragma once

#include <vector>
#include <memory>
#include <deque>
#include <string>

#include "defs.h"
#include "move.h"
#include "movegen.h"

namespace KhaosChess
{
	// Empty fen string
	extern const std::string EMPTY_FEN;

	// Starting fen string
	extern const std::string START_FEN;

	// Some example fen string
	extern const std::string TEST_FEN;
	extern const std::string TEST_ATTACKS_FEN;

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

	extern char ascii_pieces[PIECE_NB];

	// char unicode_pieces[12] = { '♙','♘','♗','♖','♕','♔','♟','♞','♝','♜','♛','♚' };

	using BITBOARD = U64;

	// Info structure stores information needed to restore a position
	// to its previous state when a move is taken back
	struct MoveInfo
	{
		// Copied when making a move
		CastlingRights	castling_rights;
		Square			en_passant;
		PLY_TYPE		fifty_move; // halfmove clock
		Value			material[BOTH]; // Material value
		Piece			captured_piece;

		// Not copied
		MoveInfo* next;
		MoveInfo* prev;
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
		Position& set(const std::string& fen, MoveInfo* mi);
		std::string get_fen() const;

		// Squares
		template<PieceType pt>
		Square square(Color c) const { return get_ls1b(get_pieces_bb(pt, c)); }
		Square ep_square() const { return move_info->en_passant; }
		Square castling_rook_square(CastlingRights cr) const;

		// Bitboards
		inline BITBOARD get_pieces_bb(PieceType pt) const;
		inline BITBOARD get_pieces_bb(PieceType pt, Color c) const { return (type[pt] & occupancies[c]); }
		inline BITBOARD get_pieces_bb(Color c) const { return occupancies[c]; }

		inline BITBOARD get_our_pieces_bb() const { return occupancies[side]; }
		inline BITBOARD get_opponent_pieces_bb() const { return occupancies[~side]; }
		inline BITBOARD get_all_pieces_bb() const { return occupancies[BOTH]; }
		inline BITBOARD get_all_empty_squares_bb() const { return ~occupancies[BOTH]; }

		inline BITBOARD	get_attackers_to(Square s) const;
		inline BITBOARD	get_attackers_to(Square s, BITBOARD occ) const;

		template<PieceType pt>
		inline BITBOARD	get_attacks_by(Color c) const;
		inline BITBOARD	get_checked_squares(PieceType pt) const;
		inline BITBOARD	get_threats(PieceType pt) const { return threats[pt]; }
		inline BITBOARD	get_king_blockers(Color c) const { return blocking_pieces[c]; }
		inline BITBOARD	get_pinners(Color c) const { return pinning_pieces[c]; }

		// Booleans
		bool is_empty(Square s) const { return get_piece_on(s) == NO_PIECE; }
		bool gives_check(Move m) const;
		bool can_castle(CastlingRights cr) const { return move_info->castling_rights & cr; }
		bool is_castling_interrupted(CastlingRights cr) const;
		bool is_legal(Move m) const;
		bool is_square_attacked(Square s) const;
		bool is_square_attacked(Square s, Color c) const;

		// Pieces
		Piece get_piece_on(Square s) const;
		Piece moved_piece(Move m) const;
		Piece captured_piece() const;

		// PLY
		PLY_TYPE game_ply() const { return fullmove_number; };
		PLY_TYPE fifty_move_count() const { return move_info->fifty_move; };

		// Value
		Value see(Move m) const;
		Value material_value(Color c) const { return move_info->material[c]; }
		Value material_value() const { return material_value(WHITE) + material_value(BLACK); }

		// Piece count
		template<PieceType pt>
		int count(Color c) const;

		// Doing and undoing moves
		void do_move(const Move& m, MoveInfo& new_info);
		void undo_move(const Move& m);

		void update_blocks_and_pins(Color c);
		void remove_piece(Square s);
		void place_piece(Piece p, Square s);
		void calculate_threats();
		void print_attacked_squares(Color c) const;

		// Caslte & side
		CastlingRights castling_rights(Color c) const { return c & CastlingRights(move_info->castling_rights); }

		Color side_to_move() const { return side; }

		// Overrides
		friend std::ostream& operator<<(std::ostream& os, const Position& position);
	private:
		template<bool Do>
		void do_castle(Color us, Square source, Square& target, Square& r_source, Square& r_target);
		void set_castling_rights(Color c, Square r_source);
		void move_piece(Square source, Square target);

		BITBOARD get_least_valuable_piece(BITBOARD attacks, Color by_side, PieceType& pt) const;

		// Data
		BITBOARD occupancies[BOTH + 1]{};				// All pieces of the side to move
		BITBOARD type[PIECE_TYPE_NB]{};					// All the piece types
		BITBOARD castling_path[CASTLING_RIGHT_NB]{};	// Castling path depending on the castling side
		BITBOARD threats[PIECE_TYPE_NB]{};				// Piece type threads
		BITBOARD pinning_pieces[BOTH]{};				// Pieces that are pinning
		BITBOARD blocking_pieces[BOTH]{};				// Pieces that are blocking

		Piece	 piece_board[SQUARE_TOTAL]{};			// Board of pieces
		Piece	 captured{};							// Captured piece

		Square	 rook_source_sq[CASTLING_RIGHT_NB]{};

		Color	 side{};

		PLY_TYPE fullmove_number{};
		PLY_TYPE repetition{};

		int		 piece_count[PIECE_NB]{};				// Piece count

		// State info 
		MoveInfo* move_info{};
	};

	template<PieceType pt>
	inline int Position::count(Color c) const { return piece_count[get_piece(c, pt)]; }

	inline bool Position::is_square_attacked(Square s, Color us) const
	{
		BITBOARD all = get_all_pieces_bb();

		bool is_pawn_attack = us == WHITE
			? pawn_attacks_bb(BLACK, s) & get_pieces_bb(PAWN, WHITE)
			: pawn_attacks_bb(WHITE, s) & get_pieces_bb(PAWN, BLACK);

		bool is_knight_attack = attacks_bb_by<KNIGHT>(s) & get_pieces_bb(KNIGHT, us);
		bool is_bishop_attack = attacks_bb_by<BISHOP>(s, all) & get_pieces_bb(BISHOP, us);
		bool is_rook_attack = attacks_bb_by<ROOK>(s, all) & get_pieces_bb(ROOK, us);
		bool is_queen_attack = attacks_bb_by<QUEEN>(s, all) & get_pieces_bb(QUEEN, us);

		return is_pawn_attack ||
			is_knight_attack ||
			is_bishop_attack ||
			is_rook_attack ||
			is_queen_attack;
	}

	inline bool Position::is_square_attacked(Square s) const
	{
		return is_square_attacked(s, side);
	};

	inline Piece Position::get_piece_on(Square s) const
	{
		assert(is_square_ok(s));
		return piece_board[s];
	}

	inline Piece Position::moved_piece(Move m) const
	{
		assert(m.is_move_ok());
		return get_piece_on(m.source_square());
	}

	inline BITBOARD Position::get_pieces_bb(PieceType pt) const { return type[pt]; }

	inline BITBOARD Position::get_attackers_to(Square s, BITBOARD occ) const
	{
		BITBOARD w_pawn_att = pawn_attacks_bb(WHITE, s) & get_pieces_bb(PAWN, BLACK);
		BITBOARD b_pawn_att = pawn_attacks_bb(BLACK, s) & get_pieces_bb(PAWN, WHITE);
		BITBOARD knight_att = attacks_bb_by<KNIGHT>(s) & get_pieces_bb(KNIGHT);
		BITBOARD horizontal = attacks_bb_by<ROOK>(s, occ) & (get_pieces_bb(ROOK) | get_pieces_bb(QUEEN));
		BITBOARD diagonal = attacks_bb_by<BISHOP>(s, occ) & (get_pieces_bb(BISHOP) | get_pieces_bb(QUEEN));
		BITBOARD king_att = attacks_bb_by<KING>(s) & get_pieces_bb(KING);

		return w_pawn_att | b_pawn_att | knight_att | horizontal | diagonal | king_att;
	}

	inline BITBOARD Position::get_attackers_to(Square s) const
	{
		return get_attackers_to(s, get_all_pieces_bb());
	}

	inline Piece Position::captured_piece() const { return captured; }

	inline void Position::place_piece(Piece p, Square s)
	{
		piece_board[s] = p;

		set_bit(type[type_of_piece(p)], s);
		set_bit(occupancies[get_piece_color(p)], s);

		occupancies[BOTH] |= occupancies[WHITE] | occupancies[BLACK];

		piece_count[p]++;
		piece_count[get_piece(get_piece_color(p), ALL_PIECES)]++;
	}

	inline void Position::remove_piece(Square s)
	{
		Piece p = piece_board[s];

		rm_bit(type[type_of_piece(p)], s);
		rm_bit(occupancies[get_piece_color(p)], s);
		rm_bit(occupancies[BOTH], s);

		piece_board[s] = NO_PIECE;

		piece_count[p]--;
		piece_count[get_piece(get_piece_color(p), ALL_PIECES)]--;
	}

	inline void Position::move_piece(Square source, Square target)
	{
		Piece p = piece_board[source];
		BITBOARD dest = square_to_BB(source) | square_to_BB(target);

		// remove source and target
		type[type_of_piece(p)] ^= dest;
		// remove occupanices of this colour
		occupancies[get_piece_color(p)] ^= dest;
		// remove the source and add the target 
		occupancies[BOTH] ^= dest;

		piece_board[source] = NO_PIECE;
		piece_board[target] = p;
	}

	inline bool Position::is_castling_interrupted(CastlingRights cr) const
	{
		assert(cr == WK || cr == WQ || cr == BK || cr == BQ);
		return get_all_pieces_bb() & castling_path[cr];
	}

	inline Square Position::castling_rook_square(CastlingRights cr) const
	{
		assert(cr == WK || cr == WQ || cr == BK || cr == BQ);
		return rook_source_sq[cr];
	}
}
