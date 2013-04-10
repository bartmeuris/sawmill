#ifndef __COMMENT_FILTER_H
# define __COMMENT_FILTER_H

#include <cstdio>
#include <string>
#include <sstream>
#include <iostream>
#include <boost/iostreams/filter/stdio.hpp>


namespace sawmill {

class JSONToken
{
public:
	enum Type {
		STRING,	 // String
		INT,     // Integer number
		FLOAT,   // Floating point number
		NAME,    // Non-stringified identifier
		ARR_S,   // [
		ARR_E,   // ]
		BLOCK_S, // {
		BLOCK_E, // }
		ROUND_S, // (
		ROUND_E, // )
		COMMA,   // ,
		COLON,   // :
		PLUS,    // +
		MIN,     // -
		MUL,     // *
		DIV,     // /
		MOD,     // %
		EQ,      // ==
		NEQ,     // !=
		NOT,     // !
		BIT_INV, // ~
		LOG_AND, // &&
		LOG_OR,  // ||
		BIT_AND, // &
		BIT_OR,  // |
		SHIFT_L, // <<
		SHIFT_R, // >>
		TNULL,   // null
		TRUE,    // true
		FALSE,   // false
		WHITE,   // Whitespace
		NEWLINE, // Newline
		COMMENT, // Comment
		END,     // END OF FILE
		EMPTY    // STUB
	};

	JSONToken(Type t) : val(), type(t) {}
	JSONToken(): val(), type(EMPTY) {}
	JSONToken(const std::string &v, Type t) : val(v), type(t) {}
	JSONToken(const std::string &v): val(v), type(EMPTY) {}

	void setVal(const std::string &newval)
	{
		this->val = newval;
	}
	void setType(Type newtype)
	{
		this->type = newtype;
	}
	void guessType()
	{
		if ((this->type != EMPTY) || (this->val.length() == 0)) {
			return;
		}
		if (this->val.length() == 1) {
			switch(this->val[0]) {
			case '~':
				setType(BIT_INV);
				break;
			case '&':
				setType(BIT_AND);
				break;
			case '|':
				setType(BIT_OR);
				break;
			case '!':
				setType(NOT);
				break;
			case '%':
				setType(MOD);
				break;
			case '/':
				setType(DIV);
				break;
			case '*':
				setType(MUL);
				break;
			case '-':
				setType(MIN);
				break;
			case '+':
				setType(PLUS);
				break;
			case ':':
				setType(COLON);
				break;
			case ',':
				setType(COMMA);
				break;
			case '[':
				setType(ARR_S);
				break;
			case ']':
				setType(ARR_E);
				break;
			case '{':
				setType(BLOCK_S);
				break;
			case '}':
				setType(BLOCK_E);
				break;
			case '(':
				setType(ROUND_S);
				break;
			case ')':
				setType(ROUND_E);
				break;
			case '\n':
				setType(NEWLINE);
				break;
			}
			if (this->type != EMPTY) return;
		} else if (this->val.length() == 2) {
			if (this->val == "==") {
				setType(EQ);
			} else if (this->val == "!=") {
				setType(NEQ);
			} else if (this->val == "&&") {
				setType(LOG_AND);
			} else if (this->val == "||") {
				setType(LOG_OR);
			} else if (this->val == "<<") {
				setType(SHIFT_L);
			} else if (this->val == ">>") {
				setType(SHIFT_R);
			}
			if (this->type != EMPTY) return;
		}
		if ((this->val[0] == ' ') || (this->val[0] == '\t')) {
			setType(WHITE);
			return;
		}
		if (this->val == "null") {
			setType(TNULL);
			return;
		} else if (this->val == "true") {
			setType(TRUE);
			return;
		} else if (this->val == "false") {
			setType(FALSE);
			return;
		}
		if ((this->val[0] == '"') && (this->val[this->val.length() - 1] == '"')) {
			setType(STRING);
			return;
		}
		std::string vstart = this->val.substr(0, 2);
		if ( (vstart == "//") || (vstart == "/*") ){
			setType(COMMENT);
			return;
		}
	}
	std::string val;
	Type type;
private:
	template<typename T> operator T () const;
};

class JSONFixer
{
public:
	explicit JSONFixer(bool strip_comments = true, bool strip_whitespace = true)
		: stripcomments(strip_comments),
		  stripwhitespace(strip_whitespace),
		  os(&std::cout),
		  is(&std::cin)
	{ }
	void set_ostream(std::ostream &new_ostream)
	{
		this->os = &new_ostream;
	}
	void set_istream(std::istream &new_istream)
	{
		this->is = &new_istream;
	}

	void fix()
	{
		this->set_ostream(std::cout);
		this->set_istream(std::cin);
		JSONToken tok;
		do {
			tok = getToken();
			if (tok.type == JSONToken::COMMENT) {
				if (!stripcomments) {
					this->out(tok.val);
				}
			} else if (tok.type == JSONToken::WHITE) {
				if (!stripwhitespace) {
					this->out(tok.val);
				}
			} else {
				this->out(tok.val);
			}
		} while (tok.type != JSONToken::END);
	}

private:
	bool stripcomments;
	bool stripwhitespace;
	std::ostream *os;
	std::istream *is;

	int peek()
	{
		return this->is->peek();
	}
	int get()
	{
		return this->is->get();
	}
	bool eof()
	{
		return this->is->eof();
	}
	void out(const std::string &str)
	{
		*(this->os) << str;
	}
	void out(int c)
	{
		this->os->put(c);
	}
	

	// Tokenizer 
	JSONToken getToken()
	{
		JSONToken ret;

		int prev, c;
		std::stringstream out;
		int outlen = 0;
		bool go = true;
		prev = 0;
		if (eof()) {
			ret.setType(JSONToken::END);
			return ret;
		}
		while ( go ) {
			c = peek();
			switch (c) {
			case EOF:
				ret.setType(JSONToken::END);
				go = false;
				break;
			case '=':
				prev = get();
				c = peek();
				if ( c == '>' ) {
					out.put(':'); outlen++;
					go = false;
				} else if (c == '=') {
					out.put(prev); outlen++;
					out.put(get()); outlen++;
					go = false;
				} else {
					out.put(prev); outlen++;
				}
				break;
			case '-':
				if (outlen != 0) {
					go = false;
					break;
				}
				prev = get();
				out.put(prev); outlen++;
				c = peek();
				if (( c < '0' ) || ( c > '9' )) {
					// not numeric
					go = false;
				}
				break;
		// Generic single character separators
			case '(':
			case ')':
			case '[':
			case ']':
			case '{':
			case '}':
			case ':':
			case ',':
			case '~':
			case '+':
			case '*':
			case '%':
				if (outlen == 0) {
					out.put(get()); outlen++;
				}
				go = false;
				break;
		// White space
			case ' ':
			case '\t':
				// White space
				// 1. if input empty -> eat all whitespace, then break
				// 2. if input not empty -> break
				if (outlen != 0) {
					go = false;
					break;
				}
				do {
					prev = get();
					out.put(prev); outlen++;
					c = peek();
				} while ((c == ' ') || (c == '\t'));
				go = false;
				ret.setType(JSONToken::WHITE);
				break;
		// New lines
			case '\n':
			case '\r':
				if (outlen == 0) {
					// Send "newline" token
					do {
						get();
						c = peek();
					} while ((c == '\n') || (c == '\r'));
					out.put('\n'); outlen++;
				}
				// Always end of token.
				ret.setType(JSONToken::NEWLINE);
				go = false;
				break;
		// Strings
			case '"':
				if (outlen != 0) {
					go = false;
					break;
				}
				// Fetch until a new " is encountered
				do {
					prev = get();
					out.put(prev); outlen++;
					c = peek();
				} while ( ( c != EOF ) && ( ( c != '"' ) || ( ( c == '"' ) && (prev == '\\') ) ) );
				get();
				out.put('"'); outlen++;
				ret.setType(JSONToken::STRING);
				go = false;
				break;
		// Comments
			case '/':
				if (outlen != 0) {
					go = false;
					break;
				}
				prev = get();
				c = peek();
				if (c == '/') {
					// Line comment
					if (!stripcomments) {
						out.put(prev); outlen++;
					}
					while (!eof()) {
						c = get();
						if ((c == '\n') || (c == '\r')) {
							go = false;
							ret.setType(JSONToken::COMMENT);
							break;
						} else if (!stripcomments) {
							out.put(c); outlen++;
						}
					}
				} else if (c == '*') { 
					// Block comment
					out.put(prev); outlen++;
					out.put(c); outlen++;
					prev = get(); // Flush '*'
					c = get();
					if (c == EOF) {
						go = false;
						ret.setType(JSONToken::COMMENT);
						break;
					}
					do {
						out.put(c); outlen++;
						prev = c;
						c = get();
					} while ( ( c != EOF ) && ( prev != '*' ) && ( c != '/' ) );
					if (c == '/') {
						out.put(c); outlen++;
					}
					ret.setType(JSONToken::COMMENT);
					go = false;
				} else if (c == EOF) {
					go = false;
				} else {
					out.put(prev); outlen++;
				}
				break;
		// Default output ?
			default:
				out.put(get()); outlen++;
				break;
			}
		}
		ret.setVal(out.str());
		if (ret.type == JSONToken::EMPTY) {
			ret.guessType();
		}
		return ret;
	}

};

class JSONFixerFilter
	: public boost::iostreams::stdio_filter
{
public:
	explicit JSONFixerFilter(bool strip_comments = true, bool strip_whitespace = true)
		: stripcomments(strip_comments),
		  stripwhitespace(strip_whitespace)
	{}

private:
	bool stripcomments;
	bool stripwhitespace;

	void do_filter()
	{
		JSONFixer fixer(stripcomments, stripwhitespace);
		fixer.fix();
	}
};

class JSONFixerFilter_old
	: public boost::iostreams::stdio_filter
{
public:
	void do_filter()
	{
		int lastchar = 0, c;
		bool comment_block = false;
		bool skip_eol = false;
		bool in_string = false;

		while ( (c = std::cin.get()) != EOF ) {
			if (comment_block) {
				if ( (lastchar == '*') && (c == '/') ) {
					comment_block = false;
					lastchar = 0;
					continue;
				}
			} else if (skip_eol) {
				if ( ( c == '\r' ) || ( c == '\n' ) ) {
					skip_eol = false;
					lastchar = 0;
					std::cout.put(c);
					continue;
				}
			} else if (in_string) {
				if (( c == '"' ) && ( lastchar != '\\' )) {
					in_string = false;
				}
				std::cout.put(c);
			} else {
				if ( lastchar == '/' ) {
					if (c == '*') {
						comment_block = true;
					} else if (c == '/') {
						skip_eol = true;
					} else {
						std::cout.put('/');
						std::cout.put(c);
					}
				} else if ( c != '/') {
					if ( c == '"') {
						in_string = true;
					}
					std::cout.put(c);
				}
			}
			lastchar = c;
		}
	}
};


} // namespace sawmill

#endif
