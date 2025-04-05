#define NOMINMAX

#include <iostream>
#include <sstream>
#include <memory>
#include <cstring>
#include <stddef.h>

#include "position.h"
#include "bitboard.h"
#include "defs.h"

namespace KhaosChess
{
	// Empty fen string
	const std::string EMPTY_FEN = "8/8/8/8/8/8/8/8 b - - ";

	// Starting fen string
	const std::string START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

	// Some example fen string
	const std::string TEST_FEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ";
	const std::string TEST_ATTACKS_FEN = "8/8/8/3PN3/8/8/3p4/8 w - - ";

	char ascii_pieces[PIECE_NB] = " PNBRQKpnbrqk";

	constexpr Piece Pieces[]{
		NO_PIECE, WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
		NO_PIECE, BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING};

	std::ostream &operator<<(std::ostream &os, const Position &position)
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
				else
					os << " .";
			}

			os << "  " << (8 - rank) << " \n";
		}

		// print files
		os << "\n a b c d e f g h \n\n";

		Color side = position.side_to_move();

		os << "\nSide:		" << (side == WHITE ? "white" : "black");

		Square enpassant = position.ep_square();
		os << "\nEnpassant:	" << (enpassant != NONE ? squareToCoordinates[enpassant] : "no") << "\n";

		CastlingRights castle = position.move_info->castling_rights;

		std::string castling;
		(castle & WK) ? castling += 'K' : castling += '-';
		(castle & WQ) ? castling += 'Q' : castling += '-';
		(castle & BK) ? castling += 'k' : castling += '-';
		(castle & BQ) ? castling += 'q' : castling += '-';

		os << "Castling:	" << castling << "\n";

		os << "\nFEN: " << position.get_fen() << "\n";

		return os;
	}

	Position &Position::set(const std::string &fen, MoveInfo *mi)
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

		Rank rank = RANK_8;
		File file = FILE_A;

		// reset boards and state variables
		memset(this, 0, sizeof(Position));
		memset(mi, 0, sizeof(MoveInfo));
		move_info = mi;

		// std::fill_n(piece_board, SQUARE_TOTAL, NO_PIECE);

		side = WHITE;
		CastlingRights castle = move_info->castling_rights;

		size_t idx = 0;

		// 1. Piece placement
		// loop over char elements
		while (idx < fen.length() && fen[idx] != ' ')
		{
			char c = fen[idx++];

			if (c == '/') // new line
			{
				file = FILE_A;
				++rank;
			}
			else if (isdigit(c))
			{
				// leave this many empty spaces
				file += c - '0';
			}
			else // it's a piece
			{
				Square sq = make_square(file, rank);
				size_t piece_idx = std::string(ascii_pieces).find(c);

				place_piece(Piece(piece_idx), sq);
				++file;
			}
		}

		idx++;

		// 2. Side to move
		// inc pointer to castling rights and check the side to move
		side = (idx < fen.length() && fen[idx] == 'w') ? WHITE : BLACK;

		// go to castling rights
		idx += 2;

		// 3. Castling rights
		while (idx < fen.length() && fen[idx] != ' ')
		{
			Square r_sq = NONE;
			Color c = islower(fen[idx]) ? BLACK : WHITE;
			Piece rook = get_piece(c, ROOK);

			unsigned char token = char(toupper(fen[idx++]));

			if (token == 'K')
				for (r_sq = sq_relative_to_side(H1, c); get_piece_on(r_sq) != rook; --r_sq)
					;
			else if (token == 'Q')
				for (r_sq = sq_relative_to_side(A1, c); get_piece_on(r_sq) != rook; ++r_sq)
					;
			else if (token >= 'A' && token <= 'H')
				r_sq = make_square(File(token - 'A'), rank_relative_to_side(c, RANK_1));
			else
				continue; // invalid token

			set_castling_rights(c, r_sq);
		}

		idx++;

		// 3. Enpassant square
		if (idx < fen.length() && fen[idx] != '-')
		{
			// init enpassant suqare
			move_info->en_passant = make_square(
				File(fen[idx] - 'a'),
				Rank(8 - (fen[idx] - '0')));

			idx += 2; // skip the en passant square and space
		}
		else
			move_info->en_passant = NONE, idx++;

		// Skip to halfmove and fullmove
		idx++;

		// 5 & 6. Halfmove clock and fullmove number
		// Get the rest of the string for parsing numbers
		std::string remainder = fen.substr(idx);
		std::istringstream ss(remainder);

		// Read halfmove and fullmove
		ss >> move_info->fifty_move >> fullmove_number;

		// Convert from fullmove starting from one to gamePly starting from 0
		// handles also incorrect FEN's with fullmove = 0
		fullmove_number = std::max(2 * (fullmove_number - 1), 0) + (side == BLACK);

		// Calculate threats and material
		calculate_threats();
		move_info->material[WHITE] = move_info->material[BLACK] = VALUE_ZERO;

		for (BITBOARD b = get_all_pieces_bb(); b;)
		{
			Piece p = get_piece_on(pop_ls1b(b));
			PieceType pt = type_of_piece(p);

			if (pt != KING && pt != PAWN)
				move_info->material[get_piece_color(p)] += PieceValue[p];
		}

		return *this;
	}

	// Get FEN function. Exists for debugging purposes only
	std::string Position::get_fen() const
	{
		int empty_sq_count;
		std::ostringstream ss;

		// 1. Piece placement section
		for (Rank rank = RANK_8; rank <= RANK_1; ++rank)
		{
			for (File file = FILE_A; file <= FILE_H; ++file)
			{
				for (empty_sq_count = 0;
					 file <= FILE_H && is_empty(make_square(file, rank));
					 ++file)
					++empty_sq_count;

				// means print empty square count
				if (empty_sq_count)
					ss << empty_sq_count;

				// print piece on the square
				if (file <= FILE_H)
					ss << ascii_pieces[get_piece_on(make_square(file, rank))];
			}

			if (rank < RANK_1)
				ss << '/';
		}

		// Side to move
		ss << (side == WHITE ? " w " : " b ");

		// Castling rights
		if (can_castle(WK))
			ss << 'K';
		if (can_castle(WQ))
			ss << 'Q';
		if (can_castle(BK))
			ss << 'k';
		if (can_castle(BQ))
			ss << 'q';

		if (!can_castle(ANY))
			ss << '-';

		ss << " " << (ep_square() == NONE // En passant square
						  ? "- "
						  : squareToCoordinates[ep_square()])
		   << " " << move_info->fifty_move << " "		   // Rule 50
		   << 1 + (fullmove_number - (side == BLACK)) / 2; // Current move count

		return ss.str();
	}

	template <PieceType pt>
	inline BITBOARD Position::get_attacks_by(Color c) const
	{
		if (pt == PAWN)
			return c == WHITE
					   ? pawn_attacks_bb<WHITE>(get_pieces_bb(PAWN, WHITE))
					   : pawn_attacks_bb<BLACK>(get_pieces_bb(PAWN, BLACK));

		BITBOARD attacks = 0ULL;
		BITBOARD attackers = get_pieces_bb(pt, c);

		while (attackers)
			attacks |= attacks_bb_by<pt>(pop_ls1b(attackers), get_all_pieces_bb());

		return attacks;
	}

	void Position::print_attacked_squares(Color c) const
	{
		std::cout << "\nAttacked squares:\n\n";

		for (Rank rank = RANK_8; rank <= RANK_1; ++rank)
		{
			for (File file = FILE_A; file <= FILE_H; ++file)
			{
				Square sq = make_square(file, rank);
				std::string s = is_square_attacked(sq, c) ? " 1" : " 0";

				std::cout << s;
			}

			std::cout << "  " << (8 - rank) << " \n";
		}

		// print files
		std::cout << "\n a b c d e f g h \n\n";
	}

	// Shows a bitboard of the possible pieces that can give check to the opposite king in a given position
	BITBOARD Position::get_checked_squares(PieceType pt) const
	{
		Square ksq = square<KING>(~side);
		BITBOARD all = get_all_pieces_bb();

		switch (pt)
		{
		case PAWN:
			return pawn_attacks_bb(~side, ksq);
		case KNIGHT:
			return attacks_bb_by<KNIGHT>(ksq);
		case BISHOP:
			return attacks_bb_by<BISHOP>(ksq, all);
		case ROOK:
			return attacks_bb_by<ROOK>(ksq, all);
		case QUEEN:
			return attacks_bb_by<QUEEN>(ksq, all);

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
		if (get_king_blockers(~us) & source)
			return !are_squares_aligned(source, target, opp_king_square) || m.move_type() == MT_CASTLING;

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
	void Position::do_move(const Move &m, MoveInfo &new_info)
	{
		assert(m.is_move_ok());
		assert(&new_info != move_info);

		// Copy the old struct into the new one up to the captured_piece field
		// prev and next are not copied
		std::memcpy(&new_info, move_info, offsetof(struct MoveInfo, captured_piece));

		// Much like a linked list
		// Assign then previous block to be equal to the old struct
		new_info.prev = move_info;

		// then the next on the old to be the current new
		move_info->next = &new_info;

		// and the one we are at to be the current new
		move_info = &new_info;

		fullmove_number++;		 // increment on every move, displayed correctly on get_fen()
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
							 : on_target;			 // can be NO_PIECE or every other piece (without KING)

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
			else
				move_info->material[them] -= PieceValue[captured]; // remove the captured piece value

			remove_piece(capture_sq);
			move_info->fifty_move = 0;
		}

		// Reset en passant square
		if (move_info->en_passant != NONE)
			move_info->en_passant = NONE;

		// Update castling rights if needed
		move_info->castling_rights = move_info->castling_rights & CASTLING_RIGHTS_TABLE[source];
		move_info->castling_rights = move_info->castling_rights & CASTLING_RIGHTS_TABLE[target];

		if (m_type != MT_CASTLING)
			move_piece(source, target);

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

				move_info->material[us] += PieceValue[promoted];
			}

			move_info->fifty_move = 0;
		}

		move_info->captured_piece = captured;

		side = ~side;

		calculate_threats();

		// Repetition calculation needs to happen
		// will do after hashing the moves
	}

	void Position::undo_move(const Move &m)
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
		BITBOARD opp_queen = get_pieces_bb(QUEEN, ~us);

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

			BITBOARD occ = get_all_pieces_bb() ^ source ^ cap_sq; // remove our pawn and cap square

			BITBOARD r_att = attacks_bb_by<ROOK>(ksq, occ) & (get_pieces_bb(ROOK, ~us) | opp_queen);
			BITBOARD b_att = attacks_bb_by<BISHOP>(ksq, occ) & (get_pieces_bb(BISHOP, ~us) | opp_queen);

			// if such attack after removing the pieces does not exist
			// then the move is legal, hence return true
			return !(r_att | b_att);
		}
		// Not castling into check
		// And castling path is clear
		case MT_CASTLING:
		{
			target = sq_relative_to_side(target > source ? G1 : C1, us);
			Direction step = target > source ? LEFT : RIGHT;

			for (Square s = target; s != source; s += step)
				if (is_square_attacked(s, ~us))
					return false;

			return true;
		}
		}

		if (type_of_piece(get_piece_on(source)) == KING) // moving the king
														 // Check if the target square is attacked by the enemy
		{
			if (is_square_attacked(target, ~side))
				return false;

			// Check if moving the king exposes it to attacks from sliding pieces
			BITBOARD occ = get_all_pieces_bb() ^ source; // remove the king
			// Get the rook and bishop attacks
			BITBOARD b1 = attacks_bb_by<ROOK>(target, occ) & (get_pieces_bb(ROOK, ~us) | opp_queen);
			BITBOARD b2 = attacks_bb_by<BISHOP>(target, occ) & (get_pieces_bb(BISHOP, ~us) | opp_queen);

			return !(b1 | b2);
		}

		// Since king exposing to checks is handled
		// Other cases are:

		// Capture of checking piece. The captured piece is NOT absoliutely pinned
		// Moving along the direction, towards or away from the king
		return !(get_king_blockers(us) & source) || are_squares_aligned(source, target, ksq);
	}

	// Helper for do/undo castling move
	template <bool Do>
	void Position::do_castle(Color us, Square source, Square &target, Square &r_source, Square &r_target)
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

	BITBOARD Position::get_least_valuable_piece(BITBOARD attacks, Color by_side, PieceType &pt) const
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

		BITBOARD source_set = square_to_BB(source);
		BITBOARD attacks = get_attackers_to(target);

		Piece attacked_piece = get_piece_on(target);
		PieceType attacked_type = type_of_piece(attacked_piece);

		gain[depth] = PieceValue[get_piece_on(target)];

		do
		{
			BITBOARD occ = get_all_pieces_bb();

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

		move_info->castling_rights |= cr;

		rook_source_sq[cr] = r_source;

		Square r_target = sq_relative_to_side(cr & KINGSIDE ? F1 : D1, c);
		Square k_target = sq_relative_to_side(cr & KINGSIDE ? G1 : C1, c);

		BITBOARD k_and_r = square_to_BB(k_source) | r_source;

		castling_path[cr] = (in_between_bb(r_source, r_target) |
							 in_between_bb(k_source, k_target)) &
							~k_and_r;
	}

	void Position::calculate_threats()
	{
		update_blocks_and_pins(WHITE);
		update_blocks_and_pins(BLACK);

		Square ksq = square<KING>(~side);

		BITBOARD all = get_all_pieces_bb();

		threats[PAWN] = pawn_attacks_bb(~side, ksq);
		threats[KNIGHT] = attacks_bb_by<KNIGHT>(ksq);
		threats[BISHOP] = attacks_bb_by<BISHOP>(ksq, all);
		threats[ROOK] = attacks_bb_by<ROOK>(ksq, all);
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
		BITBOARD snipers = ((attacks_bb_by<ROOK>(ksq) & (rooks | queens)) |
							(attacks_bb_by<BISHOP>(ksq) & (bishops | queens))) &
						   get_pieces_bb(~c);

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