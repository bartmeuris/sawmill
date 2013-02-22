#ifndef __CONFIGLOADER_H
# define __CONFIGLOADER_H

#include <string>
#include <vector>
#include <openssl/md5.h>

namespace sawmill {

class ConfigManager
{
	public:
		ConfigManager();

		void addConfigSource(const std::string &source);
		void addConfigSource(const std::vector<std::string> &sources);
		
		/**
		 * (re)load the configuration
		 */
		void load();

		unsigned int getVersion() const;
		
		void escapeRegex(std::string &regex) const;
	protected:
	private:
		void addMD5(const std::string &fn);
		void findConfigFiles();
		void findConfigFiles(const std::string &source);
		
		std::vector<std::string> configsources;
		std::vector<std::string> configfiles;
		unsigned int version;
		MD5_CTX md5context;
		std::string current_md5;
};

}
#endif // defined __CONFIGLOADER_H
