#pragma once
#include <string>
#include <vector>
#include <ostream>
#include "astnode.h"

namespace sqf
{
	namespace parse
	{

		class helper
		{
			std::ostream* merr;
			std::string(*mdbgsegmentcb)(const char*, size_t, size_t);
			bool(*mcontains_nular)(std::string);
			bool(*mcontains_unary)(std::string);
			bool(*mcontains_binary)(std::string);
			short(*mprecedence)(std::string);
		public:
			helper(
				std::ostream* err,
				std::string(*dbgsegment)(const char*, size_t, size_t),
				bool(*contains_nular)(std::string),
				bool(*contains_unary)(std::string),
				bool(*contains_binary)(std::string),
				short(*precedence)(std::string)
			) : merr(err), mdbgsegmentcb(dbgsegment), mcontains_nular(contains_nular), mcontains_unary(contains_unary), mcontains_binary(contains_binary), mprecedence(precedence) {}
			std::ostream& err(void) { return *merr; }
			std::string dbgsegment(const char* full, size_t off, size_t length) { return mdbgsegmentcb(full, off, length); }
			bool contains_nular(std::string s) { return mcontains_nular(s); }
			bool contains_unary(std::string s) { return mcontains_unary(s); }
			bool contains_binary(std::string s) { return mcontains_binary(s); }
			short precedence(std::string s) { return mprecedence(s); }
		};
	}
}