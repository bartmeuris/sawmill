#ifndef __CONFIGLOADER_H
# define __CONFIGLOADER_H

#include <string>
#include <vector>

namespace sawmill {

class ConfigManager
{
	public:
		ConfigManager();
		ConfigManager(const std::string &file);
		ConfigManager(const std::vector<std::string> &files);

		void addConfigSource(const std::string &str);

		/**
		 * Reload the configuration
		 */
		void reload();
		unsigned int getVersion() const;
		
		bool MatchTextWithWildcards(const std::string &text, std::string wildcardPattern, bool caseSensitive = true) const;
		void EscapeRegex(std::string &regex) const;
	protected:
	private:
		unsigned int version;
};

}
#endif // defined __CONFIGLOADER_H
