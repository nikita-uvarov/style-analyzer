#ifndef STYLE_ANALYZER_INDENTATION_CONTEXT_H
#define STYLE_ANALYZER_INDENTATION_CONTEXT_H

#include <vector>
#include <iostream>
#include <memory>
#include <cstdint>
#include <map>

#include <clang-c/Index.h>

#include "Streams.h"

namespace sa
{
	using std::string;
	using std::vector;
	using std::map;
	using std::unique_ptr;
	using std::ifstream;
	using std::ofstream;

	typedef uint32_t InvisibleModifierId;
	typedef uint32_t IndentationVariableId;
	typedef uint32_t TokenClassId;

	struct TokenInterval
	{
		bool isAfterNewline;
		uint32_t nSpaces;
	};

	class IndentationContext
	{
	public :

		void appendToken (uint32_t fileBufferOffset, string tokenValue, TokenInterval previousShift);

		void annotateToken (unsigned tokenIndex, CXToken clangToken, CXCursor clangCursor);

		uint32_t getNumTokens() const;

		CXToken getClangToken (unsigned tokenIndex) const;

		CXCursor getClangCursor (unsigned tokenIndex) const;

		TokenInterval getIntervalBefore (unsigned tokenIndex) const;

		const vector <InvisibleModifierId>& getInvisibleModifiersBefore (unsigned tokenIndex) const;

		void setTokenClass (unsigned tokenIndex, TokenClassId tokenClass);

		void addInvisibleModifier (unsigned beforeToken, InvisibleModifierId id);

		// Modifier removal functions if needed only

		TokenClassId getTokenClassId (string className);
		InvisibleModifierId getInvisibleModifierId (string iModifierName);

		string getInvisibleModifierName (InvisibleModifierId id) const;
		string getTokenClassName (TokenClassId id) const;

		void save (IOutputStream* stream);
		static unique_ptr <IndentationContext> load (IInputStream* stream);
		static unique_ptr <IndentationContext> create (CXTranslationUnit unit);

	private :

		IndentationContext (const IndentationContext&) = delete;
		IndentationContext& operator= (const IndentationContext&) = delete;

		struct AnnotatedTokenInterval
		{
			TokenInterval interval;
			vector <InvisibleModifierId> iModifiers;
		};

		struct Token
		{
			uint32_t fileBufferOffset;
			string tokenValue;

			TokenClassId tokenClass;
			AnnotatedTokenInterval afterTokenInterval;

			CXToken clangToken;
			CXCursor clangCursor;
		};

		AnnotatedTokenInterval beforeFirstTokenInterval;
		vector <Token> tokenStream;

		map <string, TokenClassId> tokenClassNameToId;
		map <string, InvisibleModifierId> invisibleModifierNameToId;
	};
}

#endif // STYLE_ANALYZER_INDENTATION_CONTEXT_H
