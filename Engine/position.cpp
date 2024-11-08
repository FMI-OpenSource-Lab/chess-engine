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

		CastlingRights castle = position.inf->castling;

		std::string castling;
		(castle & WK) ? castling += 'K' : castling += '-';
		(castle & WQ) ? castling += 'Q' : castling += '-';
		(castle & BK) ? castling += 'k' : castling += '-';
		(castle & BQ) ? castling += 'q' : castling += '-';

		os << "Castling:	" << castling << "\n";

		os << "FEN: " << position.get_fen() << "\n";

		return os;
	}

	Position& Position::set(const char* fen, Info* info)
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
		std::fill_n(piece_board, SQUARE_TOTAL, NO_PIECE);

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
			Square r_sq;
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

	bool Position::is_legal(Move m) const
	{
		// Base cases for legal move
		if (!m.is_move_ok() ||
			get_piece_color(moved_piece(m)) != side ||
			get_piece_on(square<KING>(side)) != get_piece(side, KING))
			return false;

		Color us = side;
		Square source = m.source_square();
		Square target = m.target_square();

		// En passant captures
		if (m.move_type() == MT_EN_PASSANT)
		{
			// if en pass exists
			if (target != ep_square()) return false;

			Square king_square = square<KING>(us);
			Square capture_square = ep_square() - pawn_push_direction(us);
			BITBOARD occ = pawn_attacks_bb(us, get_pieces_bb(PAWN, us)) & ep_square();

			// 1. check if moved piece is a pawn
			bool is_pawn = moved_piece(m) == get_piece(us, PAWN);
			// 2. check if the piece which is going to be en passant captured is a pawn
			bool is_enpassanted_pawn = get_piece_on(capture_square) == get_piece(~us, PAWN);
			// 3. check if there is no piece on the en passant (target) square
			bool is_square_empty = is_empty(target);

			// 4. check if en passant reveals a check (direct attack) -> this can happen only with the slider pieces (rook, bishop, queen)
			BITBOARD rook_revealed_check = attacks_bb_by<ROOK>(king_square, occ) & (get_pieces_bb(ROOK, ~us) | get_pieces_bb(QUEEN, ~us)); // intersect rook and queen attack from king square to rook square

			BITBOARD bishop_revealed_check = attacks_bb_by<BISHOP>(king_square, occ) & (get_pieces_bb(BISHOP, ~us) | get_pieces_bb(QUEEN, ~us)); // intersect with bishop attack to the bishop square

			return is_pawn &&
				is_enpassanted_pawn &&
				is_square_empty &&
				!rook_revealed_check &&
				!bishop_revealed_check;
		}

		// TODO: Castling legal moves
		if (m.move_type() == MT_CASTLING)
		{
			// if we are black our target square becomes G8 and C8
			// if we are white our target square becomes G1 and C1
			target = sq_relative_to_side(target > source ? G1 : C1, us);
			// if the target square is greater than the source square we castle to the kingside and vice versa
			Direction castle_dir = target > source ? RIGHT : LEFT;

			// Check for attackers on the castling squares
			for (Square s = target; s != source; s += castle_dir)
				if (get_attackers_to(s) & get_opponent_pieces_bb()) return false;

			// Check if the king squares on the rank are blocked
			Square l_rook_square = sq_relative_to_side(target > source ? G1 : B1, us);

			// Check for occupancy on the king rank
			// If we have Knight on B1 for example we check that if the intersection between all the pieces on the board and the square  
			for (Square s = l_rook_square; s != source; s += castle_dir)
				if (get_all_pieces_bb() & s) return false;

			// can castle
			return true;
		}

		// If king is moving check if target square is attacked by our opponent
		if (type_of_piece(get_piece_on(source)) == KING)
			return !(get_attackers_to(target, get_all_pieces_bb() ^ source)
				& get_opponent_pieces_bb());

		// Check if a piece is pinned
		// Only legal option for a pinned piece is to move along the slide towards or away from the attacker, but not elsewhere revealing a check

		// Checking if pieces are aligned and then if they are not pinned (blocking checks)
		return are_squares_aligned(source, target, square<KING>(us)) ||
			!(get_blocking_pieces(us) & source);
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
		// for debuging purposes
		assert(m.is_move_ok());

		Color us = side;
		Color opp = ~us;

		Square source = m.source_square();
		Square target = m.target_square();

		Piece piece_on_source = get_piece_on(source);
		Piece captured_piece =
			m.move_type() == MT_EN_PASSANT
			? get_piece(opp, PAWN)
			: get_piece_on(target);

		if (side == BLACK)
			fullmove_number++;

		if (m.move_type() == MT_CASTLING)
		{
			Square r_source, r_target;

			// Castle the king, defined below
			do_castle<true>(us, source, target, r_source, r_target);

			captured_piece = NO_PIECE;
		}

		if (captured_piece != NO_PIECE)
		{
			Square cap_sq = target;

			// If captured piece is a pawn
			if (type_of_piece(captured_piece) == PAWN)
			{
				// captured through an en passant attack
				if (m.move_type() == MT_EN_PASSANT)
				{
					cap_sq += pawn_push_direction(us);

					// Make assertions about the en passant capture
					// Check if the en passanted piece is a pawn
					assert(captured_piece == get_piece(us, PAWN));
					// Check if the target square is an en passant square
					assert(target == enpassant_square);
					// Check if the captured pawn is on 6th or 4th rank
					assert(rank_of(target) == us == WHITE ? RANK_4 : RANK_6);
					// Check whether theres a piece on the capture square
					assert(get_piece_on(target) == NO_PIECE);
					// Check if we are not capturing our pawn
					assert(get_piece_on(cap_sq) == get_piece(opp, PAWN));
				}

				// Update board and piece lists
				remove_piece(cap_sq);
			}

			// Reset rule 50 counter
			rule_fifty = 0;
		}

		// Update en passant
		if (enpassant_square != NONE)
			enpassant_square = NONE;

		// Update castling rights
		if (castle &&
			(CASTLING_RIGHTS_TABLE[source] |
				CASTLING_RIGHTS_TABLE[target]))
		{
			castle = castle &
				(CASTLING_RIGHTS_TABLE[source] |
					CASTLING_RIGHTS_TABLE[target]);
		}

		// Move the piece
		if (m.move_type() != MT_CASTLING)
		{
			move_piece(source, target);
		}

		// If the moving piece is a pawn
		if (type_of_piece(piece_on_source) == PAWN)
		{
			// Set en passant if the pawn is double pushed
			if ((int(target) ^ int(source)) == 16)
				// 8 if signe push, 16 for double
			{
				enpassant_square = Square(source + pawn_push_direction(us));
			}
			else if (m.move_type() == MT_PROMOTION)
			{
				Piece promoted = get_piece(us, m.promoted());
				PieceType promoted_type = m.promoted();

				// assert that we are promoting on the 8th or 1st rank
				assert(rank_of(target) == (us == WHITE) ? RANK_8 : RANK_1);
				// Make sure that we are not promoting ot pawn, king or no piece
				assert(type_of_piece(promoted) >= KNIGHT && type_of_piece(promoted) <= QUEEN);

				remove_piece(target);
				place_piece(promoted, target);
			}

			rule_fifty = 0;
		}

		side = ~side;

		new_info.captured_piece = captured_piece;
		new_info.en_passant = enpassant_square;
		new_info.halfmove_clock = rule_fifty;
		new_info.castling_rights = castle;

		repetition = 0;

		// Later will handle repetition logic
	}

	void Position::undo_move(Move m, MoveInfo& new_info)
	{
		// switch sides
		side = ~side;

		captured = new_info.captured_piece;
		enpassant_square = new_info.en_passant;
		rule_fifty = new_info.halfmove_clock;
		castle = new_info.castling_rights;

		Color us = side;
		Square source = m.source_square();
		Square target = m.target_square();
		Piece piece_on = get_piece_on(target);

		assert(is_empty(source) || m.move_type() == MT_CASTLING);
		assert(type_of_piece(captured) != KING);

		if (side == BLACK)
			fullmove_number--;

		if (m.move_type() == MT_PROMOTION)
		{
			// assert that we are promoting on the 8th or 1st rank
			assert(rank_of(target) == (us == WHITE) ? RANK_8 : RANK_1);
			// check if the piece is the correct promoted piece
			assert(type_of_piece(piece_on) == m.promoted());
			// check if we have promoted knight, bishop, rook, queen
			assert(type_of_piece(piece_on) >= KNIGHT && type_of_piece(piece_on) <= QUEEN);

			// Remove the piece
			remove_piece(target);
			// then set the new piece to be pawn
			// because only pawns can be promoted
			piece_on = get_piece(us, PAWN);
			place_piece(piece_on, target);
		}

		// Undo castling
		if (m.move_type() == MT_CASTLING)
		{
			Square r_source, r_target;
			do_castle<false>(us, source, target, r_source, r_target);
		}
		else
		{
			// this time move the piece from the target square to the source
			move_piece(target, source);

			if (captured != NO_PIECE)
			{
				Square csq = target;

				if (m.move_type() == MT_EN_PASSANT)
				{
					csq += pawn_push_direction(us);

					// Make assertions about the en passant capture
					// Check if we are capturing a pawn
					assert(type_of_piece(piece_on) == PAWN);
					// Check if the en passanted piece is a pawn
					assert(captured == get_piece(~us, PAWN));
					// Check if the target square is an en passant square
					assert(target == inf->previous->enpassant);
					// Check if the captured pawn is on 6th or 4th rank
					assert(rank_of(target) == us == WHITE ? RANK_4 : RANK_6);
					// Check whether theres a piece on the capture square
					assert(get_piece_on(csq) == NO_PIECE);
				}

				// Put back the captured piece
				place_piece(inf->captured_piece, csq);
			}
		}

		// Point our info pointer to its previous state
		inf = inf->previous;
		--fullmove_number;
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

	// Tests if the position is drawn by 50 rule move
	// or repetition. Not detecting stalemate
	bool Position::is_draw(PLY_TYPE ply) const
	{
		BITBOARD checkers =
			get_attackers_to(square<KING>(side)) & get_pieces_bb(~side);

		if (inf->rule_fifty > 99 && !checkers)
			return true;

		// Return draw if a position repeats once earlier but
		// strictly after the root, or repeats twice before or at the root
		return inf->repetition && inf->repetition < ply;
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

	// Helper function to set the castling rights to the info struct and to the
	// castling path array
	bool Position::is_castling_prevented(CastlingRights cr) const {
		assert(cr == WK || cr == WQ || cr == BK || cr == BQ);

		// Return intersection with pieces to see if its prevented
		return get_all_pieces_bb() & castling_path[cr];
	}
}