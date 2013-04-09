/***************************************************************************
 * Project: SawMill
 * Authors:
 *          Bart Meuris <bart . meuris @ gmail . com>
 *
 * License: (to be discussed LGPL version 2.0?)
 *
 ***************************************************************************/
#include <string>
#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <boost/program_options.hpp>

#include "command.pb.h"
#include "logevent.pb.h"

#include "config.h"
#include "sawmill.h"
#include "version.h"

namespace po = boost::program_options;

using namespace sawmill;


SawMill::SawMill()
	: initialized(false), configset()
{
}

void SawMill::run(void)
{
	this->config().check();
}

bool SawMill::ready()
{
	// Check/load the config if needed
	this->config().check();
	// Return if it succeeded
	return this->config().isLoaded();
}

void SawMill::showVersion(std::ostream &out)
{
	get_version(out);
}


int main(int argc, char* argv[])
{
	SawMill mill;

	// Parse commandline arguments
	po::options_description desc("Generic options");
	desc.add_options()
		("help,h", "Show this help message")
		("version,v", "Show the version")
		("foreground,f", "Run in foreground")
		("config,c", po::value< std::vector<std::string> >(), "Specify a config file to use")
	;
	po::variables_map vm;

	try {
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);
	} catch (po::error &e) {
		std::cerr << "Commandline error: " << e.what() << std::endl;
		std::cout << desc << std::endl;
		return 1;
	}
	if ( vm.count("version")) {
		get_version(std::cout, true);
		return 0;
	}

	if ( vm.count("help")) {
		get_version(std::cout);
		std::cout << desc << std::endl;
		return 0;
	}
	
	// Use boost::asio:::signal_set to handle signals/ctrl-c/...
	
	
	// Add configuration options - multiple allowed
	if (vm.count("config") > 0) {
		mill.config().addConfigSource( vm["config"].as< std::vector< std::string> >() );
	}
	// Check configuration and state
	if ( !mill.ready()) {
		mill.showVersion(std::cout);
		std::cout << desc << std::endl;
		return 1;
	}

	// Run in background unless choosen otherwise
	if ( ! vm.count("foreground") ) {
		// Daemonize
		daemon(1, 0);
	}
	

	mill.run();

	return 0;
}
