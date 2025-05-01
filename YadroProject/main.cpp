#include <iostream>
#include <fstream>
#include <string>

class Config {
public:
	Config() = default;
	Config(const std::string& configFile) {
		std::ifstream file(configFile);
		std::string line;
		int writeDelay = -1, readDelay = -1, rewindDelay = -1, shiftDelay = -1;
		while (std::getline(file, line)) {
			if (line.empty()) {
				throw std::runtime_error("Bad config: empty line");
			}
			else if (line[0] == '[') {
				if (line != "[tape_config]") {
					throw std::runtime_error("Bad config: bad name");
				}
			}
			else {
				size_t equal = line.find('=');
				if (equal == std::string::npos) {
					throw std::runtime_error("Bad config: bad line");
				}
				std::string parameter = line.substr(0, equal - 1);
				if (parameter == "write_delay") {
					writeDelay = std::stoi(line.substr(equal + 2));
				} else if (parameter == "read_delay") {
					readDelay = std::stoi(line.substr(equal + 2));
				} else if (parameter == "rewind_delay") {
					rewindDelay = std::stoi(line.substr(equal + 2));
				} else if (parameter == "shift_delay") {
					shiftDelay = std::stoi(line.substr(equal + 2));
				}
				else {
					throw std::runtime_error("Bad config: bad line");
				}
			}
		}
		if (writeDelay == -1 || readDelay == -1 || rewindDelay == -1 || shiftDelay == -1) {
			throw std::runtime_error("Bad config: not enough parameters");
		}
		write_delay_ = writeDelay;
		read_delay_ = readDelay;
		rewind_delay_ = rewindDelay;
		shift_delay_ = shiftDelay;
	}
	int GetWriteDelay() {
		return write_delay_;
	}
	int GetReadDelay() {
		return read_delay_;
	}
	int GetRewindDelay() {
		return rewind_delay_;
	}
	int GetShiftDelay() {
		return shift_delay_;
	}

private:
	int write_delay_ = 0;
	int read_delay_ = 0;
	int rewind_delay_ = 0;
	int shift_delay_ = 0;
};

int main(int argc, char* argv[]) {
	if (argc != 3) {
		std::cerr << "Bad input\n";
		return 1;
	}
	std::string inputFile = argv[1], ouputFile = argv[2], configFile = "config.ini";
	try {
		Config config(configFile);

	} catch (const std::exception& e) {
		std::cerr << e.what() << "\n";
		return 1;
	}

	return 0;
}