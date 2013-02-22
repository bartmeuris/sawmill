#ifndef __SAWMILL_H
# define __SAWMILL_H

#include <string>
#include <vector>
#include "configmanager.h"

namespace sawmill {

class SawMill
{
public:
	SawMill();

	void showVersion(std::ostream &out);
	ConfigManager &config() { return configset; }

	void run();
	bool ready();
private:
	bool initialized;
	ConfigManager configset;
};

} // namespace sawmill

#endif // ifndef __SAWMILL_H
