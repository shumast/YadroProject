#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <thread>
#include <chrono>

class Config {
public:
	Config() = default;
	Config(const std::string& configFile) {
		std::ifstream file(configFile);
		std::string line;
		long long writeDelay = -1, readDelay = -1, rewindDelay = -1, shiftDelay = -1, memoryLimit = -1;
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
					writeDelay = std::stoll(line.substr(equal + 2));
				}
				else if (parameter == "read_delay") {
					readDelay = std::stoll(line.substr(equal + 2));
				}
				else if (parameter == "rewind_delay") {
					rewindDelay = std::stoll(line.substr(equal + 2));
				}
				else if (parameter == "shift_delay") {
					shiftDelay = std::stoll(line.substr(equal + 2));
				}
				else if (parameter == "memory_limit") {
					memoryLimit = std::stoll(line.substr(equal + 2));
				}
				else {
					throw std::runtime_error("Bad config: bad line");
				}
			}
		}
		if (writeDelay == -1 || readDelay == -1 || rewindDelay == -1 || shiftDelay == -1 || memoryLimit == -1) {
			throw std::runtime_error("Bad config: not enough parameters");
		}
		write_delay_ = writeDelay;
		read_delay_ = readDelay;
		rewind_delay_ = rewindDelay;
		shift_delay_ = shiftDelay;
		memory_limit_ = memoryLimit;
	}
	long long GetWriteDelay() const {
		return write_delay_;
	}
	long long GetReadDelay() const {
		return read_delay_;
	}
	long long GetRewindDelay() const {
		return rewind_delay_;
	}
	long long GetShiftDelay() const {
		return shift_delay_;
	}
	long long GetMemoryLimit() const {
		return memory_limit_;
	}

private:
	long long write_delay_ = 0;
	long long read_delay_ = 0;
	long long rewind_delay_ = 0;
	long long shift_delay_ = 0;
	long long memory_limit_ = 0;
};

class Tape {
public:
	Tape(const std::string& filename, const Config& config) {
		write_delay_ = config.GetWriteDelay();
		read_delay_ = config.GetReadDelay();
		rewind_delay_ = config.GetRewindDelay();
		shift_delay_ = config.GetShiftDelay();
		memory_limit_ = config.GetMemoryLimit();
	}


private:
	long long write_delay_ = 0;
	long long read_delay_ = 0;
	long long rewind_delay_ = 0;
	long long shift_delay_ = 0;
	long long memory_limit_ = 0;
	long long size_ = 0;
	std::fstream file_;
	void makeDelay(long long delay) {
		std::this_thread::sleep_for(std::chrono::nanoseconds(delay));
	}
};



int main(int argc, char* argv[]) {
	if (argc != 3) {
		std::cerr << "Bad input\n";
		return 1;
	}
	std::string inputFile = argv[1], outputFile = argv[2], configFile = "config.ini";
	try {
		Config config(configFile);
		Tape inputTape(inputFile, config);
		Tape outputTape(outputFile, config);

	} catch (const std::exception& e) {
		std::cerr << e.what() << "\n";
		return 1;
	}

	return 0;
}