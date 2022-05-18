#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <config.h>
#include <cstring>

// Stuff to do with Config files
ConfigFile Config::readConfigFile(std::string file) {
	// Setup Vars and open file
	std::ifstream ifile(file);
	// If file could not be opened, bail with a default config
	if(!ifile.is_open()) {
		ConfigFile out;
		out.values["failed"] = "true";
		return out;
	}
	return readFromStream(ifile);
}

ConfigFile Config::readFromData(char* data) {
    std::istringstream ss(std::string(data, strlen(data)));
    return readFromStream(ss);
}

ConfigFile Config::readFromStream(std::istream& stream) {
    ConfigFile out;
    std::string line;
    // Loop over every line
    while(std::getline(stream, line)) {
        if(line == "") { continue; }
        // Convert line into a stream for easier handling
        std::istringstream is_line(line);
        std::string name;
        // Get setting name
        if(std::getline(is_line, name, '=')) {
            std::string value;
            // Get setting value
            if(std::getline(is_line, value)) {
                out.values[name] = value;
            }
        } else {
            std::cerr << "found invalid config line \"" << line <<"\"; attempting to ignore it. this might cause problems\n";
        }
    }
    return out;
}