#ifndef __COMMENT_FILTER_H
# define __COMMENT_FILTER_H

#include <cstdio>
#include <string>
#include <sstream>
#include <iostream>
#include <boost/iostreams/filter/stdio.hpp>


namespace sawmill{

class JSONToken
{
public:
	JSONToken()
	{
	}
};

class JSONFixer
	: public boost::iostreams::stdio_filter
{
public:
	explicit JSONFixer(bool strip_comments = true, bool strip_whitespace = true)
		: stripcomments(strip_comments), stripwhitespace(strip_whitespace)
	{ }
private:
	bool stripcomments;
	bool stripwhitespace;

	std::string getToken()
	{
		int prev, c;
		std::stringstream out;
		int outlen = 0;
		bool go = true;
		prev = 0;
		if (std::cin.eof()) {
			return "";
		}
		while ( go ) {
			c = std::cin.peek();
			switch (c) {
			case EOF:
				go = false;
				break;
		// Generic single character separators
			case '[':
			case ']':
			case '{':
			case '}':
			case ':':
			case ',':
				if (outlen == 0) {
					out.put(std::cin.get());
					outlen++;
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
					prev = std::cin.get();
					if (!stripwhitespace) {
						out.put(prev);
						outlen++;
					}
					c = std::cin.peek();
				} while ((c == ' ') || (c == '\t'));
				if (!stripwhitespace) {
					go = false;
				}
				break;
		// New lines
			case '\n':
			case '\r':
				if (outlen == 0) {
					// Send "newline" token
					do {
						std::cin.get();
						c = std::cin.peek();
					} while ((c == '\n') || (c == '\r'));
					out.put('\n');
					outlen++;
				}
				// Always end of token.
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
					prev = std::cin.get();
					out.put(prev);
					outlen++;
					c = std::cin.peek();
				} while ( ( c != EOF ) && ( ( c != '"' ) || ( ( c == '"' ) && (prev == '\\') ) ) );
				std::cin.get();
				out.put('"');
				outlen++;
				go = false;
				break;
		// Comments
			case '/':
				if (outlen != 0) {
					go = false;
					break;
				}
				prev = std::cin.get();
				c = std::cin.peek();
				if (c == '/') {
					// Line comment
					if (!stripcomments) {
						out.put(prev);
						outlen++;
					}
					while (!std::cin.eof()) {
						c = std::cin.get();
						if ((c == '\n') || (c == '\r')) {
							if (!stripcomments) {
								// Don't eat newlines if comments are not stripped
								std::cin.unget();
								go = false;
							}
							break;
						} else if (!stripcomments) {
							out.put(c);
							outlen++;
						}
					}
				} else if (c == '*') { 
					// Block comment
					if (!stripcomments) {
						out.put(prev);
						outlen++;
						out.put(c);
						outlen++;
					}
					prev = std::cin.get(); // Flush '*'
					c = std::cin.get();
					if (c == EOF) {
						go = false;
						break;
					}
					do {
						if (!stripcomments) {
							out.put(c);
							outlen++;
						}
						prev = c;
						c = std::cin.get();
					} while ( ( c != EOF ) && ( prev != '*' ) && ( c != '/' ) );
					if (c == '/') {
						if (!stripcomments) {
							out.put(c);
							outlen++;
						}
					}
					if (!stripcomments) {
						go = false;
					}
				} else if (c == EOF) {
					go = false;
				} else {
					out.put(prev);
					outlen++;
				}
				break;
		// Default output ?
			default:
				out.put(std::cin.get());
				break;
			}
		}
		return out.str();
	}

	void out(const std::string &str)
	{
		std::cout << str;
	}
	void out(int c)
	{
		std::cout.put(c);
	}

	void do_filter()
	{
		std::string ret;
		do {
			ret = getToken();
			/*
			if (ret == "\n") {
				std::cout << "Token: --\\n--\n";
			} else {
				std::cout << "Token: --" << ret << "--\n";
			}
			*/
			std::cout << ret;
		} while (ret != "");
	}

	void do_filter_comments()
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
