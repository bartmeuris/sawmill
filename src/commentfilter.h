#ifndef __COMMENT_FILTER_H
# define __COMMENT_FILTER_H

#include <cstdio>
#include <iostream>
#include <boost/iostreams/filter/stdio.hpp>


namespace sawmill{

class CommentFilter
	: public boost::iostreams::stdio_filter
{
public:
	explicit CommentFilter() { }
private:
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
