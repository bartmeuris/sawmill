/****************************************************************************
 * Project: SawMill
 * Authors:
 *          Bart Meuris <bart . meuris @ gmail . com>
 *
 * License: (to be discussed LGPL version 2.0?)
 ****************************************************************************
 * Module description:
 *     Class that manages the configuration, centralizes the reloading,
 *     parsing, processing, ...
 *
 ***************************************************************************/

#include "configmanager.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

namespace bfs = boost::filesystem;
namespace bio = boost::iostreams;

namespace sawmill {

ConfigManager::ConfigManager()
	:configsources(), configfiles(), version(-1), current_md5()
{
}

void ConfigManager::addConfigSource(const std::vector<std::string> &sources)
{
	for (std::vector<std::string>::const_iterator it = sources.begin(); it != sources.end(); it++) {
		addConfigSource(*it);
	}
}

void ConfigManager::addConfigSource(const std::string &source)
{
	configsources.push_back(source);
}

void ConfigManager::load()
{
	// Locate all configfiles from the config sources - files could have been added
	this->findConfigFiles();

	// Load and parse them -- Keep old parsed config if present and only replace when no errors occured.
	// Also calculate hashes so we update our config version if it differs
	unsigned char configmd5[MD5_DIGEST_LENGTH];
	MD5_Init(&md5context);
	for (std::vector<std::string>::const_iterator it = configfiles.begin(); it != configfiles.end(); it++) {
		std::cout << "Found configfile: " << *it << std::endl;
		// Todo: Load this configuration file
		this->addMD5(*it);
	}
	MD5_Final(configmd5, &md5context);
	
	// Compare and store the MD5
	std::ostringstream md5hash;
	md5hash << std::hex << std::uppercase << std::setfill( '0' );
	for (size_t i = 0; i < sizeof(configmd5); i++) {
		md5hash << std::setw( 2 ) << (configmd5[i] & 0xFF);
	}
	if (this->current_md5 != md5hash.str()) {
		this->version++;
		this->current_md5 = md5hash.str();
		std::cout << "Loaded new configuration: v" << this->version << " (hash: " << this->current_md5 << ")" <<  std::endl;
	} else {
		std::cout << "Configuration not changed: v" << this->version << " (hash: " << this->current_md5 << ")" <<  std::endl;
	}
}

void ConfigManager::addMD5(const std::string &fn)
{
	// Use memory mapped files for calculating MD5sum, should be faster :)
	bfs::path filepath(fn);
	uintmax_t filesize = bfs::file_size( filepath );
	bio::mapped_file_source file;
	file.open(filepath.string(), filesize);

	if(file.is_open()) {
		MD5_Update(&md5context, file.data(), filesize);
		file.close();
	}
}


void ConfigManager::findConfigFiles()
{
	configfiles.clear();
	for (std::vector<std::string>::iterator it = configsources.begin(); it != configsources.end(); ++it) {
		findConfigFiles(*it);
	}
}

void ConfigManager::findConfigFiles(const std::string &str)
{
	bfs::path p(str);
	//std::cout << "Find configfile in source: " << str << std::endl;
	
	if (!bfs::exists(p)) {
		// Doesn't exist, check if this is a directory with wildcard...
		bfs::path dir = p.parent_path();
		
		if (dir.string() == "") {
			// If the pattern is without a directory, for example "*.conf", use cwd
			dir = bfs::path(".");
		}

		if (!bfs::is_directory(dir)) {
			std::cout << "!!! Warning: invalid wildcard pattern - not a directory:" << str << std::endl;
			return;
		}
	
		// From http://stackoverflow.com/questions/3300419/file-name-matching-with-wildcard
		// Escape all regex special chars
		std::string wildcardPattern = p.filename().string();

		escapeRegex(wildcardPattern);
		// Convert chars '*?' back to their regex equivalents
		boost::replace_all(wildcardPattern, "\\?", ".");
		boost::replace_all(wildcardPattern, "\\*", ".*");
		boost::regex pattern(wildcardPattern, boost::regex::normal);
		
		std::vector<bfs::path> dirlist;
		std::copy(bfs::directory_iterator(dir), bfs::directory_iterator(), std::back_inserter(dirlist));
		std::sort(dirlist.begin(), dirlist.end());
		
		for (std::vector<bfs::path>::const_iterator it(dirlist.begin()), it_end(dirlist.end()) ; it != it_end; ++it) {
			// Skip directories to avoid recursive loading
			if (bfs::is_directory(*it))
				continue;
			if (regex_match(it->filename().string(), pattern)) {
				//std::cout << "Find config file - regex " << it->string() << std::endl;
				
				this->findConfigFiles(it->string());
			}
		}
	} else if (bfs::is_directory(p) ) {
		// Load all files from this directory
		std::vector<bfs::path> dirlist;
		std::copy(bfs::directory_iterator(p), bfs::directory_iterator(), std::back_inserter(dirlist));
		std::sort(dirlist.begin(), dirlist.end());
		
		for (std::vector<bfs::path>::const_iterator it(dirlist.begin()), it_end(dirlist.end()) ; it != it_end; ++it) {
			// Skip directories to avoid recursive loading
			if (bfs::is_directory(*it))
				continue;
			//std::cout << "Find config file - isdir " << it->string() << std::endl;

			this->findConfigFiles(it->string());
		}

	} else if (bfs::is_regular_file(p)) {
		// Add to config file list
		//std::cout << "Push " << str << std::endl;
		configfiles.push_back(str);
	}
}

unsigned int ConfigManager::getVersion() const
{
	return this->version;
}


void ConfigManager::escapeRegex(std::string &regex) const
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
