#include "pch.h"
#include "CppUnitTest.h"

#include <string>

#include "../Engine/defs.h"
#include "../Engine/consts.h"
#include "../Engine/main.cpp"
#include "../Engine/attacks.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{
	TEST_CLASS(Tests)
	{
	public:
		// TODO: Fix tests and errors
		TEST_METHOD(AssertBitboardKnighAttack_e4)
		{
			std::string expectedBitboardValue = "567348067172352";

			attacks att;

			Assert::AreEqual(
				expectedBitboardValue,
				std::to_string(att.maskKnightAttacks(e4)));
		}
	};
}