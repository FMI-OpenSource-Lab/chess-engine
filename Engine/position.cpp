#define NOMINMAX

#include <iostream>
#include <sstream>
#include <memory>
#include <cstring>

#include "position.h"
#include "defs.h"

namespace ChessEngine
{
	char ascii_pieces[13] = "PNBRQKpnbrqk";

	constexpr Piece Pieces[]
	{
		WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
		BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING
	};

	std::ostream& operator<<(std::ostream& os, const Position& position)
	{
		os << "\n";

		// loop on ranks
		for (Rank rank = RANK_1; rank <= RANK_8; ++rank)
		{
			// loop on files
			for (File file = FILE_A; file <= FILE_H; ++file)
			{
				Piece p = position.get_piece_on(make_square(file, rank));

				if (p != NO_PIECE)
					os << " " << ascii_pieces[p];
				else os << " .";
			}

			os << "  " << (8 - rank) << " \n";
		}

		// print files
		os << "\n a b c d e f g h \n\n";

		os << "Side:		" << (position.side_to_move() == WHITE
			? "white\n"
			: "black");

		Square enpassant = position.ep_square();
		os << "\nEnpassant:	" << (enpassant != NONE
			? squareToCoordinates[enpassant]
			: "no") << "\n";

		CastlingRights castle = position.move_info->castling_rights;

		std::string castling;
		(castle & WK) ? castling += 'K' : castling += '-';
		(castle & WQ) ? castling += 'Q' : castling += '-';
		(castle & BK) ? castling += 'k' : castling += '-';
		(castle & BQ) ? castling += 'q' : castling += '-';

		os << "Castling:	" << castling << "\n";

		os << "FEN: " << position.get_fen() << "\n";

		return os;
	}

	Position& Position::set(const char* fen, MoveInfo* mi)
	{
		/*
		FEN describes a Chess position, It is one line ASCII string.

		A FEN containts 6 fields separated by space

		1. Piece placement (from white's perspective).
		Each rank is separated by every / (slash). Each rank has letters and/or numbers. Letters  or  represent the pieces,
		small caps are the Black's pieces (pnbrkq) and upper case letters (PNBRKQ) are the White's pieces.
		The numbers (from 1 to 8) show how many empty spaces need to be empty

		2. Active colour or side to move. Represented as 'w' or 'b'

		3. Castling ability. If neither side can castle it is '-'.
		Otherwise FEN string indicates the castling rights using the letter 'KQ' and their representive
		lowercase kq. Uppercase letters mean castling rights for White and Lowercase letters
		mean castling rights for Black. 'K' stands for kingside and 'Q' stands for queenside


		4. En passant target square. If there's no en passant target square then the notation for this
		is simplpy '-'. Otherwise the notation is the name of the square.
		If a pawn just made a 2 square move, this is the position "behind" the pawn.

		5. Halfmove clock. This is the number of halfmoves since the last pawn advance or capture.
		This is used to determine if a draw can be claimed under the 50-move rule.

		6. Fullmove number. The number of the full move.
		It starts at 1 and is incremented after Black's move.
		*/

		// convert the string into char *
		// const char* fenPtr = fenStr.c_str();

		// init a pointer to the fen string
		// allocate memroy
		char* fen_ptr = (char*)fen;

		Rank rank = RANK_1;
		File file = FILE_A;

		// reset boards and state variables
		memset(this, 0, sizeof(Position));
		memset(mi, 0, sizeof(MoveInfo));
		std::fill_n(piece_board, SQUARE_TOTAL, NO_PIECE);
		move_info = mi;

		side = WHITE;
		enpassant_square = NONE;
		castle = CASTLE_NB;

		size_t idx;

		// 1. Piece placement
		// loop over char elements
		for (char c = *fen_ptr; c != ' '; c = *++fen_ptr)
		{
			if (c == '/') // new line
			{
				// resent file
				// go to the next rank
				file = FILE_A;
				++rank;
			}
			else if (isdigit(c))
			{
				// leve this many empty spaces
				// c derefrences the pointer and subtracts '0' to convert it into int
				file += c - '0';
			}
			else // its a piece
			{
				Square sq = make_square(file, rank);
				idx = std::string(ascii_pieces).find(c);

				place_piece(Piece(idx), sq);

				++file;
			}
		}

		// 2. Side to move
		// inc pointer to castling rights and check the side to move
		side = (*++fen_ptr == 'w') ? WHITE : BLACK;

		// go to castling rights
		fen_ptr += 2;

		// 3. Castling rights
		while (*fen_ptr != ' ')
		{
			Square r_sq = NONE;
			Color c = islower(*fen_ptr) ? BLACK : WHITE;
			Piece rook = get_piece(c, ROOK);

			switch (*fen_ptr++)
			{
			case 'K': castle = castle | WK;
				for (r_sq = H1; get_piece_on(r_sq) != rook; --r_sq);
				break; // break from switch
			case 'Q': castle = castle | WQ;
				for (r_sq = A1; get_piece_on(r_sq) != rook; ++r_sq);
				break;// break from switch
			case 'k': castle = castle | BK;
				for (r_sq = H8; get_piece_on(r_sq) != rook; --r_sq);
				break; // break from switch
			case 'q': castle = castle | BQ;
				for (r_sq = A8; get_piece_on(r_sq) != rook; --r_sq);
				break; // break from switch
			case '-':
				break;
			}

			set_castling_rights(c, r_sq);
		}

		// 3. Enpassant square
		if (*++fen_ptr != '-')
		{
			// init enpassant suqare
			enpassant_square =
				make_square(
					File(fen_ptr[0] - 'a'),
					Rank(8 - (fen_ptr[1] - '0')));
		}
		else
			enpassant_square = NONE;

		std::istringstream ss(fen_ptr);
		// Halfmove clock and fullmove number
		ss >> std::skipws >> rule_fifty >> fullmove_number;

		// Convert from fullmove starting from one to gamePly starting from 0
		// handles also incorrect FEN's with fullmove = 0
		fullmove_number = std::max(2 * (fullmove_number - 1), 0) + (side == BLACK);

		move_info->castling_rights = castle;
		move_info->en_passant = enpassant_square;
		move_info->halfmove_clock = rule_fifty;

		return *this;
	}

	// Get FEN function. Exists for debugging purposes only
	std::string Position::get_fen() const
	{
		int empty_sq_count;
		std::ostringstream ss;

		for (Rank rank = RANK_1; rank <= RANK_8; ++rank)
		{
			for (File file = FILE_A; file <= FILE_H; ++file)
			{
				for (empty_sq_count = 0;
					file <= FILE_H && is_empty(make_square(file, rank));
					++file) ++empty_sq_count;

				// means print empty square count
				if (empty_sq_count) ss << empty_sq_count;

				// print piece on the square
				if (file <= FILE_H) ss << ascii_pieces[
					get_piece_on(make_square(file, rank))
				];
			}

			if (rank < RANK_8) ss << '/';
		}

		// Side to move
		ss << (side == WHITE ? " w " : " b ");

		// Castling rights
		if (can_castle(WK)) ss << 'K';
		if (can_castle(WQ)) ss << 'Q';
		if (can_castle(BK)) ss << 'k';
		if (can_castle(BQ)) ss << 'q';

		if (!can_castle(ANY)) ss << '-';

		// En passant square

		ss << " " << (ep_square() == NONE
			? "- "
			: squareToCoordinates[ep_square()])
			<< " " << rule_fifty << " "
			<< 1 + (fullmove_number - (side == BLACK)) / 2;

		return ss.str();
	}

	template<PieceType pt>
	inline BITBOARD Position::get_attacks_by(Color c) const
	{
		if (pt == PAWN)
			return c == WHITE
			? pawn_attacks_bb<WHITE>(get_pieces_bb(PAWN, WHITE))
			: pawn_attacks_bb<BLACK>(get_pieces_bb(PAWN, BLACK));

		BITBOARD attacks = 0;
		BITBOARD bb = get_pieces_bb(pt, c);

		while (bb)
		{
			attacks |= attacks_bb_by<pt>(getLS1B_square(bb), get_all_pieces_bb());
			resetLSB(bb);
		}

		return attacks;
	}

	inline BITBOARD Position::get_attackers_to(Square s, BITBOARD occ) const
	{
		BITBOARD pawn_att =
			(pawn_attacks_bb<WHITE>(s) & get_pieces_bb(PAWN, BLACK))
			| (pawn_attacks_bb<BLACK>(s) & get_pieces_bb(PAWN, WHITE));

		BITBOARD knight_att = attacks_bb_by<KNIGHT>(s) & get_pieces_bb(KNIGHT);
		BITBOARD rook_att = attacks_bb_by<ROOK>(s, occ) & get_pieces_bb(ROOK);
		BITBOARD bishop_att = attacks_bb_by<BISHOP>(s, occ) & get_pieces_bb(BISHOP);
		BITBOARD queen_att = attacks_bb_by<QUEEN>(s, occ) & get_pieces_bb(QUEEN);
		BITBOARD king_att = attacks_bb_by<KING>(s) & get_pieces_bb(KING);

		return
			(pawn_attacks_bb<BLACK>(s) & get_pieces_bb(PAWN, WHITE))
			| (pawn_attacks_bb<WHITE>(s) & get_pieces_bb(PAWN, BLACK))
			| (attacks_bb_by<KNIGHT>(s) & get_pieces_bb(KNIGHT))
			| (attacks_bb_by<ROOK>(s, occ) & (get_pieces_bb(ROOK) | get_pieces_bb(QUEEN)))
			| (attacks_bb_by<BISHOP>(s, occ) & (get_pieces_bb(BISHOP) | get_pieces_bb(QUEEN)))
			| (attacks_bb_by<KING>(s) & get_pieces_bb(KING));
	}

	BITBOARD Position::get_checked_squares(PieceType pt) const
	{
		Square ksq = square<KING>(~side);

		switch (pt)
		{
		case PAWN: return pawn_attacks_bb(~side, ksq);
		case KNIGHT: return attacks_bb_by<KNIGHT>(ksq);
		case BISHOP: return attacks_bb_by<BISHOP>(ksq, get_all_pieces_bb());
		case ROOK: return attacks_bb_by<ROOK>(ksq, get_all_pieces_bb());
		case QUEEN: return attacks_bb_by<QUEEN>(ksq, get_all_pieces_bb());

		default: // KING
			return 0;
		}
	}

	// This function helps the move generation to determine 
	// if move in the current position gives a check
	bool Position::gives_check(Move m) const
	{
		assert(get_piece_color(moved_piece(m)) == side);

		Color us = side;

		Square source = m.source_square();
		Square target = m.target_square();

		Square king_square = square<KING>(us);
		Square opp_king_square = square<KING>(~us);

		// Cases are calculated after making a move and test whether a 
		// pseudo-legal move gives a check

		// Direct check
		// Get the attacks from the piece that is on the source square 
		// then intersect with the target
		// if its king result will be greater than 0, hence a check
		// else result will be 0, hence not a check
		if (get_checked_squares(type_of_piece(get_piece_on(source))) & target)
			return true;

		// Discovered check
		//  Get the pieces that block cheks (that are pinned)
		//  then return true if they are not aligned or if we are castling

		//	Already checked if a possible true result 
		//  is not caused by direct check of sliding capture
		if (get_blocking_pieces(~us) & source) return
			!are_squares_aligned(source, target, opp_king_square)
			|| m.move_type() == MT_CASTLING;

		// On move types
		// In case of NORMAL move (a check cannot be given)
		// In case of PROMOTION
		// In case of EN_PASSANT 
		//		could be en_passant capture with check 
		//		(may fall into the discovered checks category)
		//		could be a discovered check with a captutured pawn
		// In case of CASLTLE

		switch (m.move_type())
		{
			// If the move is any move that does not result in attack
			// that means the move is not giving check
		case MT_NORMAL:
			return false;

			// Handle the case where the promoted piece checks the king
		case MT_PROMOTION:
			return attacks_bb_by(m.promoted(), target, get_all_pieces_bb() ^ source);

			// Handle the en passant capture with check
			// Direct and discovered checks are already handled above
			// so the only thing left to do is 
			// handling discovered check through captured pawn
		case MT_EN_PASSANT:
		{
			Square csq = make_square(file_of(target), rank_of(source));
			BITBOARD bb = (get_all_pieces_bb() ^ source ^ csq) | target;

			BITBOARD rook_from_king_bb =
				attacks_bb_by<ROOK>(square<KING>(~us), bb) &
				(get_pieces_bb(QUEEN, us) | get_pieces_bb(ROOK, us));
			BITBOARD bishop_from_king_bb =
				attacks_bb_by<BISHOP>(square<KING>(~us), bb) &
				(get_pieces_bb(QUEEN, us) | get_pieces_bb(BISHOP, us));

			// Return true if sliders are attacking king after the capture
			return rook_from_king_bb | bishop_from_king_bb;
		}
		default: // CASTLING
		{
			// Castling is encoded as king captures the rook
			Square rook_target = sq_relative_to_side(target > source ? F1 : D1, us);

			return get_checked_squares(ROOK) & rook_target;
		}
		}
	}

	// Makes a move and saves the information in the Info
	// Move is assumed to be legal or pseudo-legal
	void Position::do_move(Move m, MoveInfo& new_info, bool gives_check)
	{
		// TODO
	}

	void Position::undo_move(Move m, MoveInfo& new_info)
	{
		// TODO
	}

	// Helper for do/undo castling move
	template<bool Do>
	void Position::do_castle(Color us, Square source, Square& target, Square& r_source, Square& r_target)
	{
		bool king_side = target > source;
		r_source = target;
		r_target = sq_relative_to_side(king_side ? F1 : D1, us);
		target = sq_relative_to_side(king_side ? G1 : C1, us);

		// Remove both pieces
		remove_piece(Do ? source : target);
		remove_piece(Do ? r_source : r_target);

		// Remove piece does not do this
		piece_board[Do ? source : target] = NO_PIECE;
		piece_board[Do ? r_source : r_target] = NO_PIECE;

		place_piece(get_piece(us, KING), Do ? target : source);
		place_piece(get_piece(us, ROOK), Do ? r_target : r_source);
	}

	BITBOARD Position::get_least_valuable_piece(BITBOARD attacks, Color by_side, PieceType& pt) const
	{
		for (pt = PAWN; pt <= KING; ++pt)
		{
			// subset contains the attacks of the pieces
			// intersected with the current piece
			BITBOARD subset =
				attacks & get_pieces_bb(pt, by_side);

			// Order of the pieces in the enumerator are 
			// from least to most valuable piece
			if (subset)
				// return this piece's bitboard
				return subset & -subset; // single bit
		}

		// the set is empty
		return 0;
	}

	Value Position::see(Move m) const
	{
		Color stm = side; // Side to move

		Square source = m.source_square();
		Square target = m.target_square();

		Value gain[32], depth = 0;
		BITBOARD may_Xray =
			get_pieces_bb(PAWN) |
			get_pieces_bb(BISHOP) |
			get_pieces_bb(ROOK) |
			get_pieces_bb(QUEEN);

		BITBOARD occ = get_all_pieces_bb();
		BITBOARD source_set = square_to_BB(source);
		BITBOARD attacks = get_attackers_to(target, occ);

		Piece attacked_piece = get_piece_on(target);
		PieceType attacked_type = type_of_piece(attacked_piece);

		gain[depth] = PieceValue[get_piece_on(target)];

		do
		{
			// next depth and side
			depth++;
			// current gain is calculated at getting the current attacking
			// piece and substracting the value of the attacked piece

			gain[depth] = PieceValue[attacked_piece] - gain[depth - 1];
			attacks ^= source_set; // reset bit in set to traverse

			// reset bit in temporary occupancy (for X-Rays)
			occ ^= source_set;
			if (source_set & may_Xray)
				attacks |= get_pinned_pieces(~stm);
			source_set =
				get_least_valuable_piece(attacks, Color(depth & 1), attacked_type);

		} while (source_set);

		while (--depth)
			gain[depth - 1] = -std::max(-gain[depth - 1], gain[depth]);

		return gain[0];
	}

	void Position::set_castling_rights(Color c, Square r_source)
	{
		Square k_source = square<KING>(c);

		CastlingRights cr = c & (k_source < r_source ? KINGSIDE : QUEENSIDE);

		castling_rights_mask[k_source] |= cr;
		castling_rights_mask[r_source] |= cr;
		rook_source_sq[cr] = r_source;

		Square r_target = sq_relative_to_side(cr & KINGSIDE ? G1 : C1, c);
		Square k_target = sq_relative_to_side(cr & QUEENSIDE ? F1 : D1, c);

		Square k_and_r = k_source | int(r_source);

		castling_path[cr] = (
			in_between_bb(r_source, r_target) |
			in_between_bb(k_source, k_target)
			) & ~k_and_r;
	}
	BITBOARD Position::get_pinned_pieces(Color s) const
	{
		BITBOARD pins = 0ULL;

		// get the opp king position 
		Square ksq = square<KING>(s);

		// TODO
		// and check if he can be attacked directly from his position

		return pins;
	}

	BITBOARD Position::get_blocking_pieces(Color s) const
	{
		BITBOARD blocks = 0ULL;

		return blocks;
	}
}