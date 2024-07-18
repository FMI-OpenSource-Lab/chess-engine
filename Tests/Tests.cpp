#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{
	TEST_CLASS(PieceAttacksTests)
	{
	private:
		Attacks attacks;
		std::string expected;
		std::string result;

	public:
		// Piece attacks
		TEST_METHOD(AssertKnightAttacks)
		{
			U64 asd = 1ULL;

			expected = "567348067172352";
			result = std::to_string(attacks.maskKnightAttacks(e4));

			Assert::AreEqual(expected, result);
		}
	};
}
