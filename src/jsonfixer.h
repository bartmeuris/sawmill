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
			case ' ':
			case '\t':
				// White space
				break;
			case '\n':
			case '\r':
				if (outlen == 0) {
					out.put(std::cin.get());
					outlen++;
				}
				go = false;
				break;
			case '"':
				if (outlen != 0) {
					go = false;
					break;
				}
				// Fetch until a new " is encountered
				while ( ( c = std::cin.get()) != EOF ) {
					if (c == '"') {
						
					}
				}
				break;
			case ',':
				if (outlen == 0) {
					out.put(std::cin.get());
					outlen++;
				}
				go = false;
				break;
			case '/':
				c = std::cin.get();
				if (std::cin.peek() == '/') {
					if (outlen != 0) {
						std::cin.unget();
						go = false;
						break;
					}
					if (!stripcomments) {
						out.put(c);
					}
					while (!std::cin.eof()) {
						c = std::cin.get();
						if ((c == '\n') || (c == '\r')) {
							std::cin.unget();
							if (!stripcomments) {
								go = false;
							}
							break;
						} else if (!stripcomments) {
							out.put(c);
						}
					}
				} else if (c == '*') { 

				} else if (c == EOF) {
					out.put(c);
					outlen++;
					break;
				} else {
					out.put(prev);
					outlen++;
				}
				break;
			default:
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
