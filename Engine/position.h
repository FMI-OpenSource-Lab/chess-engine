#ifndef POSITION_H
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
		BITBOARD _occupancies[BOTH + 1]; // +1 becase 
		CastlingRights _castling;
		Color _side;
		Square _enpassant;
		PLY_TYPE _rule_fifty;
		PLY_TYPE _ply;

		// not copied
		Info* next;
		Info* previous;
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
		static void init();

		_Position() = default;

		// FEN i/o
		_Position& set(const char* fen, Info* info);
		std::string fen() const;
		void print_board();

		// Squares
		Square ep_square() const { return inf->_enpassant; }
		Square castling_rook_square(CastlingRights cr) const;

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

		// Booleans
		bool is_square_attacked(Square square, Color side_to_move) const;

		bool can_castle(CastlingRights cr) const;

		bool is_legal(Move m) const;
		bool is_pseudo_legal(Move m) const;
		bool is_capture(Move m) const;

		bool gives_check(Move m) const;
		bool is_empty(Square s) const { return get_piece_on(s) == NO_PIECE; }
		bool is_draw(PLY_TYPE ply) const;
		bool has_repeated() const;

		bool is_pos_ok() const;

		bool see(Move m, int threshold = 0) const; // Static exchange evaluation

		// Pieces
		Piece moved_piece(Move m) const { return get_piece_on(m.source_square()); };
		Piece captured_piece() const;
		Piece get_piece_on(Square s) const { return piece_board[s]; }

		// PLY
		PLY_TYPE game_ply() const;
		PLY_TYPE rule_fifty_count() const;

		// Value
		Value non_pawn_material(Color c) const;
		Value non_pawn_material() const;

		// Doing and undoing moves
		void do_move(Move m, Info& newInfo);
		void do_move(Move m, Info& newInfo, bool gives_check);
		void undo_move(Move m);

		void remove_piece(Square s);
		void place_piece(Piece p, Square s);

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
		void set_castling_right(Color c, Square r_source);
		template<bool will>
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

	inline bool _Position::can_castle(CastlingRights cr) const {
		return inf->_castling & cr;
	}

	inline BITBOARD _Position::get_attackers_to(Square s, BITBOARD occ) const
	{
		return
			(pawn_attacks_bb<BLACK>(s) & get_pieces_bb(PAWN, WHITE))
			| (pawn_attacks_bb<WHITE>(s) & get_pieces_bb(PAWN, BLACK))
			| (attacks_bb_by<KNIGHT>(s) & get_pieces_bb(KNIGHT))
			| (attacks_bb_by<ROOK>(s, occ) & (get_pieces_bb(ROOK) | get_pieces_bb(QUEEN)))
			| (attacks_bb_by<BISHOP>(s, occ) & (get_pieces_bb(BISHOP) | get_pieces_bb(QUEEN)))
			| (attacks_bb_by<KING>(s) & get_pieces_bb(KING));
	}

	inline BITBOARD _Position::get_attackers_to(Square s) const
	{
		return get_attackers_to(s, get_all_pieces_bb());
	}

	template<PieceType pt>
	inline BITBOARD _Position::get_attacks_by(Color c) const
	{
		if (pt == PAWN)
			return c == WHITE
			? pawn_attacks_bb<WHITE>(get_pieces_bb(PAWN, WHITE))
			: pawn_attacks_bb<BLACK>(get_pieces_bb(PAWN, BLACK));

		BITBOARD attacks = 0;
		BITBOARD bb = get_pieces_bb(pt, c);

		while (bb)
		{
			attacks |= attacks_bb_by<pt>(getLS1B_square(bb), get_all_pieces(side));
			resetLSB(bb);
		}

		return attacks;
	}

	inline Piece _Position::captured_piece() const { return inf->captured_piece; }

	//inline void _Position::place_piece(Piece p, Square s) 
	//{
	//	piece_board[s] = p;
	//	type[ALL_PIECES] |= type[type_of_piece(p, side)] |= s;
	//	occupancies[p >> 3] |= s;
	//}

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
