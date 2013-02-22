/****************************************************************************
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
#include <boost/program_options.hpp>

#include "command.pb.h"
#include "logevent.pb.h"

#include "config.h"
#include "sawmill.h"

namespace po = boost::program_options;

using namespace sawmill;


SawMill::SawMill()
	: initialized(false), configset()
{
}

void SawMill::run(void)
{
	this->config().load();
}

bool SawMill::ready()
{
	if (this->config().sourceCount() == 0)
		return false;
	return true;
}

void SawMill::showVersion(std::ostream &out)
{
	out << APP_NAME << " version " << VER_MAJOR << "." << VER_MINOR;
#ifdef VER_BUILD
	out << "." << VER_BUILD;
#endif
	out << " (built on ";
	out << OS_NAME;
#ifdef BUILD_MACHINE_NAME
	out << ":" <<  BUILD_MACHINE_NAME;
#endif
#ifdef BUILD_DATETIME
	out << ":"<< BUILD_DATETIME;
#endif
	out << ")";
	out << std::endl;
}


int main(int argc, char* argv[])
{
	SawMill mill;

	// Parse commandline arguments
	po::options_description desc("Generic options");
	desc.add_options()
		("help,h", "Show this help message")
		("version,v", "Show the version")
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
		mill.showVersion(std::cout);
	}
	if ( vm.count("help")) {
		mill.showVersion(std::cout);
		std::cout << desc << std::endl;
		return 1;
	}

	// (Double) fork if necessary
	
	// Use boost::asio:::signal_set to handle signals/ctrl-c/...
	
	
	// Add configuration options - multiple allowed
	if (vm.count("config") > 0) {
		mill.config().addConfigSource( vm["config"].as< std::vector< std::string> >() );
	}
	if ( !mill.ready()) {
		mill.showVersion(std::cout);
		std::cout << desc << std::endl;
		return 1;
	}
	mill.run();

	return 0;
}
