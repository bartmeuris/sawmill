#include <string>
#include <iostream>
#include <boost/program_options.hpp>

#include "command.pb.h"
#include "logevent.pb.h"

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
	// Parse commandline arguments
	po::options_description desc("Generic options");
	desc.add_options()
		("help,h", "Show this help message")
		("config,c", po::value<std::string>(), "Specify a config file to use")
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
	if ( (argc <= 1) || (vm.count("help")) ) {
		std::cout << desc << std::endl;
		return 1;
	}

	std::cout << "Test" << std::endl;
	return 0;
}
