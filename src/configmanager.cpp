#include "configmanager.h"
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/replace.hpp>

namespace sawmill {

ConfigManager::ConfigManager()
{
}


ConfigManager::ConfigManager(const std::string &file)
{
	this->addConfigSource(file);
}


ConfigManager::ConfigManager(const std::vector<std::string> &files)
{
	for (std::vector<std::string>::const_iterator it = files.begin(); it != files.end(); it++) {
		this->addConfigSource(*it);
	}
}

void ConfigManager::addConfigSource(const std::string &str)
{
	boost::filesystem::path p(str);
	if (!boost::filesystem::exists(p)) {
		// Doesn't exist, check if this is a directory with wildcard...
		boost::filesystem::path dir = p.parent_path();
		if (!boost::filesystem::is_directory(dir)) {
			// Throw Exception
		}
		// 
	} else if (boost::filesystem::is_directory(p) ) {
	} else if (!(boost::filesystem::is_regular_file(p))) {
		// Throw exception
	}
}

unsigned int ConfigManager::getVersion() const
{
	return this->version;
}

// From http://stackoverflow.com/questions/3300419/file-name-matching-with-wildcard
bool ConfigManager::MatchTextWithWildcards(const std::string &text, std::string wildcardPattern, bool caseSensitive /*= true*/) const
{
	// Escape all regex special chars
	EscapeRegex(wildcardPattern);
	// Convert chars '*?' back to their regex equivalents
	boost::replace_all(wildcardPattern, "\\?", ".");
	boost::replace_all(wildcardPattern, "\\*", ".*");
	
	boost::regex pattern(wildcardPattern, caseSensitive ? boost::regex::normal : boost::regex::icase);
	
	return regex_match(text, pattern);
}
		
void ConfigManager::EscapeRegex(std::string &regex) const
{
		boost::replace_all(regex, "\\", "\\\\");
		boost::replace_all(regex, "^", "\\^");
		boost::replace_all(regex, ".", "\\.");
		boost::replace_all(regex, "$", "\\$");
		boost::replace_all(regex, "|", "\\|");
		boost::replace_all(regex, "(", "\\(");
		boost::replace_all(regex, ")", "\\)");
		boost::replace_all(regex, "[", "\\[");
		boost::replace_all(regex, "]", "\\]");
		boost::replace_all(regex, "*", "\\*");
		boost::replace_all(regex, "+", "\\+");
		boost::replace_all(regex, "?", "\\?");
		boost::replace_all(regex, "/", "\\/");
}

}
