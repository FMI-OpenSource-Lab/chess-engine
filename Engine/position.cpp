#define NOMINMAX

#include <iostream>
#include <sstream>
#include <memory>
#include <cstring>
#include <stddef.h>

#include "position.h"
#include "bitboard.h"
#include "defs.h"

namespace ChessEngine
{
	char ascii_pieces[PIECE_NB] = " PNBRQKpnbrqk";

	constexpr Piece Pieces[]
	{
		NO_PIECE, WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
		NO_PIECE, BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING
	};

	std::ostream& operator<<(std::ostream& os, const Position& position)
	{
		os << "\n";

		// loop on ranks
		for (Rank rank = RANK_8; rank <= RANK_1; ++rank)
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

		Color side = position.side_to_move();

		os << "\nSide:		" << (side == WHITE
			? "white"
			: "black");

		Square enpassant = position.ep_square();
		os << "\nEnpassant:	" << (enpassant != NONE
			? squareToCoordinates[enpassant]
			: "no") << "\n";

		CastlingRights castle = position.get_castling_rights();

		std::string castling;
		(castle & WK) ? castling += 'K' : castling += '-';
		(castle & WQ) ? castling += 'Q' : castling += '-';
		(castle & BK) ? castling += 'k' : castling += '-';
		(castle & BQ) ? castling += 'q' : castling += '-';

		os << "Castling:	" << castling << "\n";

		os << "\nFEN: " << position.get_fen() << "\n";

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

		Rank rank = RANK_8;
		File file = FILE_A;

		// reset boards and state variables
		memset(this, 0, sizeof(Position));
		memset(mi, 0, sizeof(MoveInfo));
		move_info = mi;

		// std::fill_n(piece_board, SQUARE_TOTAL, NO_PIECE);

		side = WHITE;
		CastlingRights* castle = &move_info->castling_rights;

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
			CastlingRights cr = *castle;

			switch (*fen_ptr++)
			{
			case 'K': *castle = cr | WK;
				for (r_sq = H1; get_piece_on(r_sq) != rook; --r_sq);
				break; // break from switch
			case 'Q': *castle = cr | WQ;
				for (r_sq = A1; get_piece_on(r_sq) != rook; ++r_sq);
				break;// break from switch
			case 'k': *castle = cr | BK;
				for (r_sq = H8; get_piece_on(r_sq) != rook; --r_sq);
				break; // break from switch
			case 'q': *castle = cr | BQ;
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
			move_info->en_passant = make_square(
				File(fen_ptr[0] - 'a'),
				Rank(8 - (fen_ptr[1] - '0')));
		}
		else
			move_info->en_passant = NONE;

		std::istringstream ss(fen_ptr);
		// Halfmove clock and fullmove number
		ss >> std::skipws >> move_info->fifty_move >> fullmove_number;

		// Convert from fullmove starting from one to gamePly starting from 0
		// handles also incorrect FEN's with fullmove = 0
		fullmove_number = std::max(2 * (fullmove_number - 1), 0) + (side == BLACK);

		checkers = get_attackers_to(square<KING>(side)) & get_opponent_pieces_bb();
		calculate_threats();

		return *this;
	}

	// Get FEN function. Exists for debugging purposes only
	std::string Position::get_fen() const
	{
		int empty_sq_count;
		std::ostringstream ss;

		for (Rank rank = RANK_8; rank <= RANK_1; ++rank)
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

			if (rank < RANK_1) ss << '/';
		}

		// Side to move
		ss << (side == WHITE ? " w " : " b ");

		// Castling rights
		if (can_castle(WK)) ss << 'K';
		if (can_castle(WQ)) ss << 'Q';
		if (can_castle(BK)) ss << 'k';
		if (can_castle(BQ)) ss << 'q';

		if (!can_castle(ANY)) ss << '-';

		ss << " " << (ep_square() == NONE // En passant square
			? "- "
			: squareToCoordinates[ep_square()])
			<< " " << move_info->fifty_move << " " // Rule 50
			<< 1 + (fullmove_number - (side == BLACK)) / 2; // Current move count

		return ss.str();
	}

	template<PieceType pt>
	inline BITBOARD Position::get_attacks_by(Color c) const
	{
		if (pt == PAWN) return c == WHITE
			? pawn_attacks_bb<WHITE>(get_pieces_bb(PAWN, WHITE))
			: pawn_attacks_bb<BLACK>(get_pieces_bb(PAWN, BLACK));

		BITBOARD attacks = 0ULL;
		BITBOARD bb = get_pieces_bb(pt, c);
		BITBOARD all = get_all_pieces_bb();

		while (bb)
			attacks |= attacks_bb_by(QUEEN, pop_ls1b(bb), all);

		return attacks;
	}

	inline BITBOARD Position::get_attackers_to(Square s, BITBOARD occ) const
	{
		BITBOARD pawn_att =
			(pawn_attacks_bb<WHITE>(square_to_BB(s)) & get_pieces_bb(PAWN, BLACK))
			| (pawn_attacks_bb<BLACK>(square_to_BB(s)) & get_pieces_bb(PAWN, WHITE));
		BITBOARD knight_att = attacks_bb_by<KNIGHT>(s) & get_pieces_bb(KNIGHT);
		BITBOARD horizontal = attacks_bb_by<ROOK>(s, occ) & (get_pieces_bb(ROOK) | get_pieces_bb(QUEEN));
		BITBOARD diagonal = attacks_bb_by<BISHOP>(s, occ) & (get_pieces_bb(BISHOP) | get_pieces_bb(QUEEN));
		BITBOARD king_att = attacks_bb_by<KING>(s) & get_pieces_bb(KING);

		return
			pawn_att | knight_att | horizontal | diagonal | king_att;
	}

	// Shows a bitboard of the possible pieces that can give check to the opposite king in a given position
	BITBOARD Position::get_checked_squares(PieceType pt) const
	{
		Square ksq = square<KING>(~side);
		BITBOARD all = get_all_pieces_bb();

		switch (pt)
		{
		case PAWN: return pawn_attacks_bb(~side, ksq);
		case KNIGHT: return attacks_bb_by<KNIGHT>(ksq);
		case BISHOP: return attacks_bb_by<BISHOP>(ksq, all);
		case ROOK: return attacks_bb_by<ROOK>(ksq, all);
		case QUEEN: return attacks_bb_by<QUEEN>(ksq, all);

		default: // KING
			return 0ULL;
		}
	}

	// This function helps the move generation to determine 
	// if move in the current position gives a check
	// Tests if pseudo-legal move gives check
	bool Position::gives_check(Move m) const
	{
		assert(m.is_move_ok());
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
		// if its king, result will be greater than 0, hence a check
		// else result will be 0, hence not a check
		if (get_threats(type_of_piece(get_piece_on(source))) & target)
			return true;

		// Discovered check
		//  Get the pieces that block cheks (that are pinned)
		//  then return true if they are not aligned or if we are castling

		//	Already checked if a possible true result 
		//  is not caused by direct check of sliding capture
		if (blocking_pieces[~side] & source) return
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
			return attacks_bb_by(m.promoted(), target, get_all_pieces_bb() ^ source) & opp_king_square;

			// Handle the en passant capture with check
			// Direct and discovered checks are already handled above
			// so the only thing left to do is 
			// handling discovered check through captured pawn
		case MT_EN_PASSANT:
		{
			Square csq = make_square(file_of(target), rank_of(source));
			BITBOARD bb = (get_all_pieces_bb() ^ source ^ csq) | target;

			BITBOARD rook_from_king_bb = attacks_bb_by<ROOK>(opp_king_square, bb) & (get_pieces_bb(QUEEN, us) | get_pieces_bb(ROOK, us));
			BITBOARD bishop_from_king_bb = attacks_bb_by<BISHOP>(opp_king_square, bb) & (get_pieces_bb(QUEEN, us) | get_pieces_bb(BISHOP, us));

			// Return true if sliders are attacking king after the capture
			return rook_from_king_bb | bishop_from_king_bb;
		}
		default: // CASTLING
		{
			// Castling is encoded as king captures the rook
			Square rook_target = sq_relative_to_side(target > source ? F1 : D1, us);

			return get_threats(ROOK) & rook_target;
		}
		}
	}

	// Makes a move and saves the information in the Info
	// Move is assumed to be legal
	void Position::do_move(Move m, MoveInfo& new_info, bool gives_check)
	{
		assert(m.is_move_ok());
		assert(&new_info != move_info);

		// Copy the old struct into the new one up to the captured_piece field
		// prev and next are not copied
		std::memcpy(&new_info, move_info,
			offsetof(struct MoveInfo, captured_piece));

		// Much like a linked list
		// Assign then previous block to be equal to the old struct
		new_info.prev = move_info;

		// then the next on the old to be the current new
		move_info->next = &new_info;

		// and the one we are at to be the current new
		move_info = &new_info;

		fullmove_number++; // increment on every move, displayed correctly on get_fen()
		++move_info->fifty_move; // will be reset ot 0 in case of capture or pawn move

		Color us = side;
		Color them = ~us;

		Square source = m.source_square();
		Square target = m.target_square();

		MoveType m_type = m.move_type();

		Piece on_source = get_piece_on(source);
		Piece on_target = get_piece_on(target);
		Piece captured = m_type == MT_EN_PASSANT
			? get_piece(them, PAWN) // captured piece is opponents pawn
			: on_target; // can be NO_PIECE or every other piece (without KING)

		assert(get_piece_color(on_source) == us); // moving our piece instead of enemy piece
		assert(captured == NO_PIECE || get_piece_color(captured) == (m_type != MT_CASTLING ? them : us));
		assert(type_of_piece(captured) != KING); // make sure king is not captured

		// Castling
		if (m_type == MT_CASTLING)
		{
			assert(on_source == get_piece(us, KING)); // source piece is our king
			// since castling is encoded as "King captures rook"
			assert(captured == get_piece(us, ROOK)); // captured piece is rook

			Square r_source, r_target;
			do_castle<true>(us, source, target, r_source, r_target);
			captured = NO_PIECE;
		}

		// Captures
		if (captured)
		{
			Square capture_sq = target;

			if (type_of_piece(captured) == PAWN && m_type == MT_EN_PASSANT)
			{
				capture_sq -= pawn_push_direction(us);

				assert(on_source == get_piece(us, PAWN));
				assert(target == move_info->en_passant);
				assert(rank_relative_to_side(us, rank_of(target)) == RANK_6);
				assert(on_target == NO_PIECE);
				assert(get_piece_on(capture_sq) == get_piece(them, PAWN));
			}

			remove_piece(capture_sq);
			move_info->fifty_move = 0;
		}

		// Reset en passant square
		if (move_info->en_passant != NONE) move_info->en_passant = NONE;
		
		// Update castling rights if needed
		if (move_info->castling_rights && (castling_rights_mask[source] | castling_rights_mask[target])) move_info->castling_rights 
			= move_info->castling_rights & ~(castling_rights_mask[source] | castling_rights_mask[target]);

		if (m_type != MT_CASTLING)
		{
			move_piece(source, target);
		}

		if (type_of_piece(on_source) == PAWN)
		{
			// Set an en passant square if the moved pawn can be captured
			if ((int(target) ^ int(source)) == 16 // double push
				&& (pawn_attacks_bb(us, target - pawn_push_direction(us)) & get_pieces_bb(PAWN, them)))
			{
				move_info->en_passant = target - pawn_push_direction(us);
			}
			else if (m_type == MT_PROMOTION)
			{
				Piece promoted = get_piece(us, m.promoted());
				PieceType promoted_type = type_of_piece(promoted);

				assert(rank_relative_to_side(us, rank_of(target)) == RANK_8); // promoting on the correct rank
				assert(promoted_type >= KNIGHT && promoted_type <= QUEEN);

				// erase the old piece and put the new one
				remove_piece(target);
				place_piece(promoted, target);

			}

			move_info->fifty_move = 0;
		}

		checkers = gives_check
			? get_attackers_to(square<KING>(them)) & get_our_pieces_bb()
			: 0ULL;

		move_info->captured_piece = captured;

		side = ~side;

		calculate_threats();

		// Repetition calculation needs to happen
		// will do after hashing the moves
	}

	void Position::undo_move(Move m)
	{
		assert(m.is_move_ok());

		side = ~side; // flip side

		Color us = side;

		Square source = m.source_square();
		Square target = m.target_square();

		Piece on_target = get_piece_on(target);
		PieceType target_piece = type_of_piece(on_target);

		MoveType mt = m.move_type();

		assert(is_empty(source) || mt == MT_CASTLING);
		assert(type_of_piece(move_info->captured_piece) != KING);

		if (mt == MT_PROMOTION)
		{
			assert(rank_relative_to_side(us, rank_of(target)) == RANK_8);
			assert(target_piece == m.promoted());
			assert(target_piece >= KNIGHT && target_piece <= QUEEN);

			remove_piece(target);
			on_target = get_piece(us, PAWN);
			place_piece(on_target, target); // place pawn on the 8th rank, will be moved later
		}

		if (mt == MT_CASTLING)
		{
			Square r_source, r_target;
			do_castle<false>(us, source, target, r_source, r_target);
		}
		else
		{
			move_piece(target, source);

			if (move_info->captured_piece)
			{
				Square cap_sq = target;

				if (mt == MT_EN_PASSANT)
				{
					cap_sq -= pawn_push_direction(us);

					assert(target_piece == PAWN);
					assert(target == move_info->prev->en_passant);
					assert(rank_relative_to_side(us, rank_of(target)) == RANK_6);
					assert(get_piece_on(cap_sq) == NO_PIECE);
					assert(move_info->captured_piece == get_piece(~us, PAWN));
				}

				assert(get_piece_color(move_info->captured_piece) == ~us);
				place_piece(move_info->captured_piece, cap_sq);
			}
		}

		// return to the previous state
		move_info = move_info->prev;
		fullmove_number--;
	}

	bool Position::is_legal(Move m) const
	{
		assert(m.is_move_ok());

		Color us = side;
		Square source = m.source_square();
		Square target = m.target_square();
		Square ksq = square<KING>(us);

		MoveType mt = m.move_type();

		BITBOARD opp = get_opponent_pieces_bb();

		assert(get_piece_color(moved_piece(m)) == us);
		assert(get_piece_on(square<KING>(us)) == get_piece(us, KING));

		switch (mt)
		{
			// This case is tested after removing the enemy pawn and moving our pawn
			// Checking for discovered check
		case MT_EN_PASSANT:
		{
			assert(target == move_info->en_passant);
			assert(moved_piece(m) == get_piece(us, PAWN));

			Square cap_sq = target - pawn_push_direction(us);

			assert(get_piece_on(cap_sq) == get_piece(~us, PAWN));
			assert(get_piece_on(target) == NO_PIECE);

			BITBOARD occ = get_all_pieces_bb() ^ source ^ cap_sq;
			BITBOARD opp_queen = get_pieces_bb(QUEEN, ~us);

			BITBOARD r_att = attacks_bb_by<ROOK>(ksq, occ) & (get_pieces_bb(ROOK, ~us) | opp_queen);
			BITBOARD b_att = attacks_bb_by<BISHOP>(ksq, occ) & (get_pieces_bb(BISHOP, ~us) | opp_queen);

			return !(r_att | b_att);
		}
		// Not castling into check
		// Not checking if the castling path is clear
		case MT_CASTLING:
		{
			target = sq_relative_to_side(target > source ? G1 : C1, us);
			Direction step = target > source ? LEFT : RIGHT;

			for (Square s = target; s != source; s += step)
				if (get_attackers_to(s) & opp) return false;

			return true;
		}
		}

		if (type_of_piece(get_piece_on(source)) == KING) // moving the king
			// Check if the target square is attacked by the enemy
			return !(get_attackers_to(target, get_all_pieces_bb() ^ source) & opp);

		// Since king exposing to checks is handled
		// Other cases are:

		// Capture of checking piece. The captured piece is NOT absoliutely pinned
		// Moving along the direction, towards or away from the king
		return !(get_king_blockers(us) & source)
			|| are_squares_aligned(source, target, ksq);
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

		Value gain[32]{}, depth = 0;
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
				attacks |= pinning_pieces[side]; // PLACEHOLDER
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

		move_info->castling_rights = move_info->castling_rights | cr;

		castling_rights_mask[k_source] |= cr;
		castling_rights_mask[r_source] |= cr;
		rook_source_sq[cr] = r_source;

		Square r_target = sq_relative_to_side(cr & KINGSIDE ? F1 : D1, c);
		Square k_target = sq_relative_to_side(cr & KINGSIDE ? G1 : C1, c);

		BITBOARD k_and_r = square_to_BB(k_source) | r_source;

		castling_path[cr] = (
			in_between_bb(r_source, r_target) |
			in_between_bb(k_source, k_target)
			) & ~k_and_r;
	}

	void Position::calculate_threats()
	{
		update_blocks_and_pins(WHITE);
		update_blocks_and_pins(BLACK);

		Square ksq = square<KING>(~side);

		BITBOARD our = get_all_pieces_bb();

		threats[PAWN] = pawn_attacks_bb(~side, ksq);
		threats[KNIGHT] = attacks_bb_by<KNIGHT>(ksq);
		threats[BISHOP] = attacks_bb_by<BISHOP>(ksq, our);
		threats[ROOK] = attacks_bb_by<ROOK>(ksq, our);
		threats[QUEEN] = threats[BISHOP] | threats[ROOK];
		threats[KING] = 0; // Can't have threats by king
	}

	void Position::update_blocks_and_pins(Color c)
	{
		blocking_pieces[c] = 0ULL;
		pinning_pieces[~c] = 0ULL;

		Square ksq = square<KING>(c);

		BITBOARD rooks = get_pieces_bb(ROOK);
		BITBOARD bishops = get_pieces_bb(BISHOP);
		BITBOARD queens = get_pieces_bb(QUEEN);
		BITBOARD color_pieces = get_pieces_bb(c);

		// snipers are calculated such that there are no pieces on the board
		// to get the line between the king and the slider
		BITBOARD snipers = (
			(attacks_bb_by<ROOK>(ksq) & (rooks | queens)) |
			(attacks_bb_by<BISHOP>(ksq) & (bishops | queens))
			) & get_pieces_bb(~c);

		// All pieces without the snipers
		BITBOARD occ = get_all_pieces_bb() ^ snipers;

		// looping through all the snipers
		while (snipers)
		{
			// getting the sniper's square and popping its bit
			Square sniper_sq = pop_ls1b(snipers);
			BITBOARD btw = in_between_bb(ksq, sniper_sq) & occ;

			// if there is space between the king square and the slider
			// and is still left space after removing a bit
			// then there is greater than or equal to one blocker between them
			if (btw && !has_bit_after_pop(btw))
			{
				blocking_pieces[c] |= btw;

				// If the blocking piece is of the opposite colour
				// the blocker is pinned by the pinner (the sniper)
				if (btw & color_pieces)
					pinning_pieces[~c] |= sniper_sq;
			}
		}
	}
}