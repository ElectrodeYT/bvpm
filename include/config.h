#ifndef CONFIG_HPP
#define CONFIG_HPP
// Things for config files
#include <string>
#include <map>
#include <fstream>

class ConfigFile {
public:
	// Map of values
    std::map<std::string, std::string> values;
};

class Config {
public:
	static ConfigFile readConfigFile(std::string file);
    static ConfigFile readFromData(char* data);
    static ConfigFile readFromStream(std::istream& stream);
};
#endif
