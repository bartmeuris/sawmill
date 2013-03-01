#include "version.h"
#include "version_gen.h"
#include "config.h"
#include <string.h>
#include <string>
#include <iostream>


// Do some checking on RELEASE builds with uncommitted changes
#if !defined(DEBUG) && defined(GIT_MODIFIED) && ( GIT_MODIFIED > 0 )
# error RELEASE BUILD WITH UNCOMMITTED GIT CHANGES!!
#endif
#define TXTWIDTH 20

std::ostream &show_info(std::ostream &out, const char *name)
{
	out << std::string(strlen(APP_NAME) + 1, ' ') << name << std::string(TXTWIDTH - strlen(name), ' ') << ": ";
	return out;
}

/* UGLY FUNCTION WITH LOADS OF DEFINES */
void get_version(std::ostream &out, bool extended)
{
	// App info
	out << APP_NAME;
#ifdef DEBUG
	out << " DEBUG";
#endif // DEBUG
	out << " version " << VER_MAJOR << "." << VER_MINOR;
#ifdef VER_BUILD
	out << "." << VER_BUILD;
#endif // VER_BUILD
	
	// Build info
	out << " (";
#ifdef GIT_HASH
	out << "git:" << GIT_HASH;
# ifdef GIT_MODIFIED
	if (GIT_MODIFIED != 0) {
		out << "*";
	}
# endif // GIT_MODIFIED
	out << " ";
#endif // GIT_HASH

#ifdef BUILD_DATETIME
	out << BUILD_DATETIME;
#endif // BUILD_DATETIME
	out << ")";
	out << std::endl;
	
	////////////////////////////////////////////////
	// EXTENDED information
	if (extended) {
#ifdef GIT_HASH_LONG
		show_info(out, "git hash") << GIT_HASH_LONG;
# ifdef GIT_MODIFIED
		if (GIT_MODIFIED != 0) {
			out << " -- UNCOMMITTED CHANGES: " << GIT_MODIFIED;
		}
# endif // GIT_MODIFIED
		out << std::endl;
#endif // GIT_HASH_LONG

#ifdef GIT_USER
		show_info(out, "Built by") << GIT_USER;
# ifdef GIT_EMAIL
		out << " <" << GIT_EMAIL << ">";
# endif // GIT_EMAIL
		out << std::endl;
#endif // GIT_USER

#ifdef BUILD_DATETIME
		show_info(out, "Build date/time") << BUILD_DATETIME << std::endl;
#endif
#ifdef BUILD_MACHINE_NAME
		show_info(out, "Build machine") << BUILD_MACHINE_NAME << std::endl;
#endif // BUILD_MACHINE_NAME
#ifdef  OS_NAME
		show_info(out, "OS") << OS_NAME << std::endl;
#endif // OS_NAME
	}
}

