#include "score.h"

#include "defs.h"
#include "move.h"
#include "consts.h"
#include "position.h"
#include "bitboard.h"

#include <algorithm>
#include <memory>

namespace KhaosChess
{
	// King shield masks for kingside and queenside castling
	// Only define for the key squares we need
	BITBOARD king_shield_mask[BOTH][SQUARE_TOTAL];

	// Squares in front of a rank for each color (for passed pawn detection)
	BITBOARD in_front_bb[BOTH][RANK_NB];

	// Bonuses for connected and passed pawns
	Value connected_bonus[RANK_NB];

	// Bonuses for passed pawns
	Value passed_bonus[RANK_NB];

	namespace
	{
		// Generate the king shield mask for the color
		void generate_king_shiled_mask(const Color &c)
		{
			// Clear all the masks
			std::fill_n(king_shield_mask[c], 0ULL, SQUARE_TOTAL);

			Rank r = rank_relative_to_side(c, RANK_1);
			BITBOARD rank2 = c == WHITE ? Rank2_Bits : Rank7_Bits;

			for (File f : {FILE_C, FILE_E, FILE_G})
				// Queenside caslte, initial position and kingside castle
				king_shield_mask[c][make_square(f, r)] = rank2 & (file_bb(f - 1) | file_bb(f) | file_bb(f + 1));
		}

		// Generate the in front bitboard for the given colors
		void generate_in_front_mask(const Color &c)
		{
			BITBOARD rank = 0ULL;

			Rank begin = rank_relative_to_side(c, RANK_1);
			Rank end = rank_relative_to_side(c, RANK_7);

			while (begin != end)
			{
				// Build up the rank masks
				rank |= rank_bb(begin);

				// Then add the NOT into the mask array
				// This works because:
				// For rank 1 there are all the ranks that are infront of it
				// For rank 2 are all the pawns that are infront of it, but not 1... and so on
				in_front_bb[c][begin] = ~rank;

				// Finally, move the current rank up or down depending on the color
				(c == WHITE) ? ++begin : --begin;
			}

			// This ensures that there will be nothing infront of the edge ranks
			in_front_bb[c][c == WHITE ? RANK_8 : RANK_1] = 0ULL;
		}
	}

	void Eval::init()
	{
		// Initialize the king shield masks for both colors
		generate_king_shiled_mask(WHITE);
		generate_king_shiled_mask(BLACK);

		// Initialize the in front masks for both colors
		generate_in_front_mask(WHITE);
		generate_in_front_mask(BLACK);

		// Initialize connected pawn bonuses by rank
		// Higher ranks get higher bonuses as connected pawns become more
		// dangerous when advanced
		connected_bonus[RANK_1] = Value(0);	 // Impossible for pawns
		connected_bonus[RANK_2] = Value(5);	 // Small bonus on home rank
		connected_bonus[RANK_3] = Value(10); // Slightly more for developed pawns
		connected_bonus[RANK_4] = Value(15); // Center control
		connected_bonus[RANK_5] = Value(25); // Advanced into opponent territory
		connected_bonus[RANK_6] = Value(40); // Very advanced, threatening promotion
		connected_bonus[RANK_7] = Value(60); // One step from promotion
		connected_bonus[RANK_8] = Value(0);	 // Impossible for pawns

		// Initialize passed pawn bonuses by rank
		// Higher ranks get much higher bonuses as passed pawns closer to
		// promotion are exponentially more valuable
		passed_bonus[RANK_1] = Value(0);  // Impossible for passed pawns
		passed_bonus[RANK_2] = Value(5);  // Far from promotion
		passed_bonus[RANK_3] = Value(10); // Starting to be relevant
		passed_bonus[RANK_4] = Value(20); // Center ranks
		passed_bonus[RANK_5] = Value(40); // More than halfway to promotion
		passed_bonus[RANK_6] = Value(60); // Very advanced
		passed_bonus[RANK_7] = Value(90); // One step from promotion
		passed_bonus[RANK_8] = Value(0);  // Already promoted
	}

	// Calcuates the material score of the position depending on the side to move
	template <Color Us>
	Value Eval::simple_evaluation(const Position &pos)
	{
		// Calculates only the material balance
		return PAWN_VALUE * (pos.count<PAWN>(Us) - pos.count<PAWN>(~Us)) + (pos.material_value(Us) - pos.material_value(~Us));
	}

	// Evaluates the mobility of the pieces
	template <Color Us>
	Value Eval::mobility(const Position &pos)
	{
		Value score = VALUE_ZERO;

		// Mobility bonus for each piece type (center-weighted)
		constexpr Value knight_mobility_bonus[9] = {-20, -10, 0, 10, 20, 30, 40, 45, 50};
		constexpr Value bishop_mobility_bonus[14] = {-20, -10, 0, 10, 20, 30, 40, 50, 55, 60, 62, 64, 66, 68};
		constexpr Value rook_mobility_bonus[15] = {-20, -10, 0, 10, 20, 30, 40, 50, 55, 60, 65, 70, 72, 74, 76};
		constexpr Value queen_mobility_bonus[28] = {-20, -10, 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 105, 110, 115, /*...*/};

		constexpr Color Them = ~Us;

		BITBOARD enemies = pos.get_pieces_bb(Them);
		BITBOARD ours = pos.get_pieces_bb(Us);

		// Skip mobility calculation if pieces are blocking a pin
		BITBOARD blockers = pos.get_king_blockers(Us);

		for (PieceType pt = KNIGHT; pt <= QUEEN; ++pt)
		{
			BITBOARD pieces = pos.get_pieces_bb(pt, Us) & ~blockers;

			while (pieces)
			{
				Square s = pop_ls1b(pieces);
				BITBOARD moves = 0ULL, all = pos.get_all_pieces_bb();

				// Calculate legal moves for this piece
				switch (pt)
				{
				case KNIGHT:
					moves = attacks_bb_by<KNIGHT>(s) & ~ours;
					break;
				case BISHOP:
					moves = attacks_bb_by<BISHOP>(s, all) & ~ours;
					break;
				case ROOK:
					moves = attacks_bb_by<ROOK>(s, all) & ~ours;
					break;
				case QUEEN:
					moves = attacks_bb_by<QUEEN>(s, all) & ~ours;
					break;
				default:
					moves = 0;
					break;
				}

				// Count mobility squares and add appropriate bonus
				int mobility_count = count_bits(moves);

				switch (pt)
				{
				case KNIGHT:
					score = knight_mobility_bonus[std::min(mobility_count, 8)];
					break;
				case BISHOP:
					score = bishop_mobility_bonus[std::min(mobility_count, 13)];
					break;
				case ROOK:
					score = rook_mobility_bonus[std::min(mobility_count, 14)];
					break;
				case QUEEN:
					score = queen_mobility_bonus[std::min(mobility_count, 27)];
					break;
				}
			}
		}

		return score;
	}

	template <Color Us>
	Value Eval::king_safety(const Position &pos)
	{
		constexpr Color Them = ~Us;
		Value score = VALUE_ZERO;

		// Safety bonus depends on number of pieces (less safety needed in endgame)
		int phase = pos.game_phase();
		if (phase < 8)
			return VALUE_ZERO; // No king safety in endgame

		Square king_square = pos.square<KING>(Us);

		// Count defended squares around the king
		BITBOARD king_ring = attacks_bb_by<KING>(king_square);
		BITBOARD defended = 0ULL;

		// Check which squares are defended by our pieces
		while (king_ring)
		{
			Square s = pop_ls1b(king_ring);
			if (pos.is_square_attacked(s, Us))
				defended |= s;
		}

		// Count defenders and attackers
		int defender_count = count_bits(defended);
		int attacker_count = 0;

		BITBOARD attackers = pos.get_attackers_to(king_square) & pos.get_pieces_bb(Them);

		while (attackers)
		{
			Square s = pop_ls1b(attackers);
			Piece p = pos.get_piece_on(s);
			PieceType pt = type_of_piece(p);

			// Count attackers based on their type
			attacker_count += piece_attack_weight[pt];
		}

		// Evaluate pawn shield in front of king (especially after castling)
		int pawn_shield = 0;
		BITBOARD shield_area = king_shield_mask[Us][king_square];

		// If we don't have a pre-calculated shield area for this king position
		// (king has moved from standard castling positions), generate one dynamically
		if (!shield_area)
		{
			Rank k_rank = rank_of(king_square);
			File k_file = file_of(king_square);

			// Get the rank of the king
			Rank shield_rank = (Us == WHITE) ? Rank(k_rank - 1) : Rank(k_rank + 1);

			// Only create shiled if we're not on an edge rank
			// For WHITE: king must not be on RANK 8
			// For BLACK: king must not be on RANK 1
			if ((Us == WHITE && k_rank > RANK_8) || (Us == BLACK && k_rank < RANK_1))
			{
				// Create shield for king and adjacent files
				shield_area = rank_bb(shield_rank);
				BITBOARD file_shield = file_bb(k_file);

				// Add adjacent files if not on the edge
				if (k_file > FILE_A)
					file_shield |= file_bb(File(k_file - 1));
				if (k_file < FILE_H)
					file_shield |= file_bb(File(k_file + 1));

				shield_area &= file_shield;
			}
		}

		if (!shield_area)
		{
			BITBOARD pawns = pos.get_pieces_bb(PAWN, Us) & shield_area;
			pawn_shield = count_bits(pawns) * 10; // 10 points per pawn in shield;
		}

		// King safety score combines defenders, attackers, and pawn shield
		Value safety = Value((defender_count * 10) + pawn_shield - (attacker_count * 20));

		// Adjust safety by game phase - more important in opening/middlegame
		safety = safety * phase / MAX_PHASE_SCORE;

		return safety;
	}

	template <Color Us>
	Value Eval::pawn_structure(const Position &pos)
	{
		constexpr Color Them = ~Us;
		Value score = VALUE_ZERO;

		// Penalties for pawn structure weaknesses
		constexpr Value isolated_penalty = Value(-20);
		constexpr Value doubled_penalty = Value(-15);
		constexpr Value backward_penalty = Value(-10);

		BITBOARD pawns = pos.get_pieces_bb(PAWN, Us);

		while (pawns)
		{
			Square s = pop_ls1b(pawns);
			File file = file_of(s);
			Rank rank = rank_of(s);

			// Get rank relative to side (white or black perspective)
			Rank relative_rank = rank_relative_to_side(Us, rank);

			// Check for isolated pawns (no friendly pawns on adjacent files)
			BITBOARD adjacent_files = 0ULL;

			if (file > FILE_A)
				adjacent_files |= file_bb(File(file - 1));
			if (file < FILE_H)
				adjacent_files |= file_bb(File(file + 1));

			if (!(pos.get_pieces_bb(PAWN, Us) & adjacent_files))
				score += isolated_penalty;

			// Check for doubled pawns (multiple pawns on same file)
			if (count_bits(pos.get_pieces_bb(PAWN, Us) & file_bb(file)) > 1)
				score += doubled_penalty;

			// Check for connected pawns (friendly pawns protecting this pawn)
			BITBOARD pawn_attacks = pawn_attacks_bb(Them, s);

			if (pawn_attacks & pos.get_pieces_bb(PAWN, Us))
				score += connected_bonus[relative_rank];

			// Check for passed pawns
			BITBOARD ahead = in_front_bb[Us][relative_rank];
			BITBOARD blocking_area = file_bb(file) | adjacent_files;

			if (!(ahead & blocking_area & pos.get_pieces_bb(PAWN, Them)))
				score += passed_bonus[relative_rank];

			// Check for backward pawns
			// A backward pawn cannot be advanced and is not protected by adjacent pawns
			// Square in front of pawn
			Square front_sq = s + pawn_push_direction(Us);

			// If the square in front is attacked by enemy pawns but not defended by ours
			if (pawn_attacks_bb(Them, front_sq) & pos.get_pieces_bb(PAWN, Them) &&
				!(pawn_attacks_bb(Us, front_sq) & pos.get_pieces_bb(PAWN, Us)))
			{
				// And if the pawn can't advance safely
				if (pos.get_piece_on(front_sq) != NO_PIECE || pos.is_square_attacked(front_sq, Them))
					score += backward_penalty;
			}
		}

		return score;
	}

	template <Color Us>
	Value Eval::piece_coordination(const Position &pos)
	{
		constexpr Color Them = ~Us;
		Value score = VALUE_ZERO;

		// Bonus values for piece coordination
		constexpr Value bishop_pair_bonus = Value(30);
		constexpr Value knight_pair_bonus = Value(10);
		constexpr Value rook_pair_bonus = Value(10);
		constexpr Value rook_on_open_file_bonus = Value(20);
		constexpr Value rook_on_semi_open_file_bonus = Value(10);
		constexpr Value rook_on_7th_bonus = Value(30);
		constexpr Value queen_and_rook_battery_bonus = Value(20);
		constexpr Value minor_behind_pawn_bonus = Value(5);

		// Bishop pair bonus
		if (pos.count<BISHOP>(Us) >= 2)
			score += bishop_pair_bonus;

		// Knight pair bonus
		if (pos.count<KNIGHT>(Us) >= 2)
			score += knight_pair_bonus;

		// Rook coordination - bonus for two rooks on same rank/file
		BITBOARD rooks = pos.get_pieces_bb(ROOK, Us);

		if (pos.count<ROOK>(Us) >= 2)
		{
			Square s1 = pop_ls1b(rooks);
			Square s2 = pop_ls1b(rooks);

			// if rooks are on the same file or rank
			if (has_bit_after_pop(in_between_bb(s1, s2)))
				score += rook_pair_bonus;
		}

		// Rooks on open/semi-open files and 7th rank
		BITBOARD our_pawns = pos.get_pieces_bb(PAWN, Us);
		BITBOARD their_pawns = pos.get_pieces_bb(PAWN, Them);

		Rank seventh_rank = (Us == WHITE) ? RANK_7 : RANK_2;

		BITBOARD temp_rooks = rooks;

		while (temp_rooks)
		{
			Square s = pop_ls1b(temp_rooks);

			File f = file_of(s);
			Rank r = rank_of(s);

			// Rook on open file (no pawns)
			if (!(file_bb(f) & (our_pawns | their_pawns)))
				score += rook_on_open_file_bonus;
			// Rook on semi-open file (no friendly pawns)
			else if (!(file_bb(f) & our_pawns))
				score += rook_on_semi_open_file_bonus;

			// Rook on 7th rank
			if (r == seventh_rank)
				score += rook_on_7th_bonus;
		}

		// Queen and rook on same file (battery)
		BITBOARD queens = pos.get_pieces_bb(QUEEN, Us);

		while (queens)
		{
			Square s = pop_ls1b(queens);
			File f = file_of(s);

			if (file_bb(f) & rooks)
				score += queen_and_rook_battery_bonus;
		}

		// Minors (knights/bishops) behind pawns for safety
		BITBOARD minors = pos.get_pieces_bb(KNIGHT, Us) | pos.get_pieces_bb(BISHOP, Us);
		BITBOARD pawn_front;

		constexpr Direction dir = Us == WHITE ? UP : DOWN;
		pawn_front = move_to<dir>(our_pawns);

		if (minors & pawn_front)
			score += minor_behind_pawn_bonus * count_bits(minors & pawn_front);

		return score;
	}

	template <Color Us>
	Value Eval::threats(const Position &pos)
	{
		constexpr Color Them = ~Us;
		Value score = VALUE_ZERO;

		// Threat bonuses and penalties
		constexpr Value hanging_piece_penalty = Value(-25);
		constexpr Value pawn_attack_minor_bonus = Value(20);
		constexpr Value pawn_attack_rook_bonus = Value(40);
		constexpr Value pawn_attack_queen_bonus = Value(80);
		constexpr Value multiple_attack_bonus = Value(10);
		constexpr Value safe_pawn_attack_bonus = Value(15);

		BITBOARD their_pieces = pos.get_pieces_bb(Them);
		BITBOARD our_pawns = pos.get_pieces_bb(PAWN, Us);
		BITBOARD our_knights = pos.get_pieces_bb(KNIGHT, Us);
		BITBOARD our_bishops = pos.get_pieces_bb(BISHOP, Us);
		BITBOARD our_rooks = pos.get_pieces_bb(ROOK, Us);
		BITBOARD our_queens = pos.get_pieces_bb(QUEEN, Us);

		// Detect hanging (undefended) enemy pieces
		while (their_pieces)
		{
			Square s = pop_ls1b(their_pieces);

			// Skip king, it can't be hanging
			if (type_of_piece(pos.get_piece_on(s)) == KING)
				continue;

			if (pos.is_square_attacked(s, Us) && !pos.is_square_attacked(s, Them))
			{
				PieceType pt = type_of_piece(pos.get_piece_on(s));
				score += hanging_piece_penalty * piece_attack_weight[pt];
			}
		}

		// Pawn attacks on enemy pieces
		BITBOARD pawn_attacks = pawn_attacks_bb<Us>(our_pawns);
		BITBOARD attacked_by_pawns = pawn_attacks & pos.get_pieces_bb(Them);

		while (attacked_by_pawns)
		{
			Square s = pop_ls1b(attacked_by_pawns);
			PieceType pt = type_of_piece(pos.get_piece_on(s));

			// Different bonuses based on attacked piece type
			switch (pt)
			{
			case KNIGHT:
			case BISHOP:
				score += pawn_attack_minor_bonus;
				break;
			case ROOK:
				score += pawn_attack_rook_bonus;
				break;
			case QUEEN:
				score += pawn_attack_queen_bonus;
				break;
			default:
				break;
			}
		}

		// Multiple attacks on undefended squares
		BITBOARD all_attacks = 0ULL;
		BITBOARD multiple_attacks = 0ULL;

		// Add pawn attacks
		all_attacks |= pawn_attacks;

		// Add knight attacks
		while (our_knights)
		{
			Square s = pop_ls1b(our_knights);

			BITBOARD attacks = attacks_bb_by<KNIGHT>(s);

			multiple_attacks |= all_attacks & attacks;
			all_attacks |= attacks;
		}

		// Add bishop attacks
		BITBOARD all_pieces = pos.get_all_pieces_bb();
		while (our_bishops)
		{
			Square s = pop_ls1b(our_bishops);
			BITBOARD attacks = attacks_bb_by<BISHOP>(s, all_pieces);

			multiple_attacks |= all_attacks & attacks;
			all_attacks |= attacks;
		}

		// Add rook attacks
		while (our_rooks)
		{
			Square s = pop_ls1b(our_rooks);
			BITBOARD attacks = attacks_bb_by<ROOK>(s, all_pieces);

			multiple_attacks |= all_attacks & attacks;
			all_attacks |= attacks;
		}

		// Add queen attacks
		while (our_queens)
		{
			Square s = pop_ls1b(our_queens);
			BITBOARD attacks = attacks_bb_by<QUEEN>(s, all_pieces);

			multiple_attacks |= all_attacks & attacks;
			all_attacks |= attacks;
		}

		// Count multiple attacks on enemy pieces
		BITBOARD multiply_attacked = multiple_attacks & their_pieces;
		score += multiple_attack_bonus * count_bits(multiply_attacked);

		// Safe pawn attacks (squares where our pawns attack but aren't attacked by enemy pawns)
		BITBOARD enemy_pawn_attacks;

		enemy_pawn_attacks = pawn_attacks_bb(Them, pos.get_pieces_bb(PAWN, Them));

		BITBOARD safe_pawn_attack_squares = pawn_attacks & ~enemy_pawn_attacks & ~pos.get_pieces_bb(Us);
		score += safe_pawn_attack_bonus * count_bits(safe_pawn_attack_squares);

		return score;
	}

	// Explicit template instantiations
	template Value Eval::king_safety<WHITE>(const Position &);
	template Value Eval::king_safety<BLACK>(const Position &);

	template Value Eval::pawn_structure<WHITE>(const Position &);
	template Value Eval::pawn_structure<BLACK>(const Position &);

	template Value Eval::piece_coordination<WHITE>(const Position &);
	template Value Eval::piece_coordination<BLACK>(const Position &);

	template Value Eval::threats<WHITE>(const Position &);
	template Value Eval::threats<BLACK>(const Position &);

	// Evaluates the position using the material score and positional score
	Value Eval::evaluate(const Position &pos)
	{
		Color us = pos.side_to_move();

		// Material and positional (PST) score
		Value material_score = simple_evaluation<WHITE>(pos);
		Value positional_score = pos.pst_value(us);

		// Advanced evaluation components
		Value mobility_score, king_safety_score, pawn_structure_score,
			piece_coord_score, threats_score;

		if (us == WHITE)
		{
			mobility_score = mobility<WHITE>(pos);
			king_safety_score = king_safety<WHITE>(pos);
			pawn_structure_score = pawn_structure<WHITE>(pos);
			piece_coord_score = piece_coordination<WHITE>(pos);
			threats_score = threats<WHITE>(pos);
		}
		else
		{
			mobility_score = -mobility<BLACK>(pos);
			king_safety_score = -king_safety<BLACK>(pos);
			pawn_structure_score = -pawn_structure<BLACK>(pos);
			piece_coord_score = -piece_coordination<BLACK>(pos);
			threats_score = -threats<BLACK>(pos);
		}

		// Total evaluation
		Value total_score = material_score + positional_score +
							mobility_score + king_safety_score +
							pawn_structure_score + piece_coord_score +
							threats_score;

		return total_score;
	}

	/*
	inline int score_move(int move)
	{
		// score capture moves
		if (get_move_capture(move))
		{
			// target piece
			Piece target = WHITE_PAWN;

			Piece start = !side ? BLACK_PAWN : WHITE_PAWN;
			Piece end = !side ? BLACK_KING : WHITE_KING;

			for (Piece p = start; p <= end; ++p)
			{
				if (get_bit(bitboards[p], get_move_target(move)))
				{
					target = p;
					break;
				}
			}

			// score move by MVV LVA lookup [source][target]
			return MVV_LVA[get_move_piece(move)][target];
		}
		else // quiet move
		{

		}

		return 0;
	}

	int sort_move(moves* move_list)
	{
		// move scores
		std::unique_ptr<int[]> move_scores = std::make_unique<int[]>(move_list->count);

		std::cout << "\n\n";
		for (int c = 0; c < move_list->count; c++)
			move_scores[c] = score_move(move_list->moves[c]);

		for (int curr_mv = 0; curr_mv < move_list->count; curr_mv++)
			for (int next_mv = curr_mv + 1; next_mv < move_list->count; next_mv++)
			{
				// cmp current and next move scores
				if (move_scores[curr_mv] < move_scores[next_mv])
				{
					// swap the scores
					std::swap(move_scores[curr_mv], move_scores[next_mv]);
					// swap the moves
					std::swap(move_list->moves[curr_mv], move_list->moves[next_mv]);
				}
			}

		return 0;
	}

	void print_move_scores(moves* move_list)
	{
		printf("     Move scores:\n\n");

		// loop over moves within a move list
		for (int count = 0; count < move_list->count; count++)
		{
			printf("     move: ");
			print_move(move_list->moves[count]);
			printf(" score: %d\n", score_move(move_list->moves[count]));
		}
	}
	*/
}