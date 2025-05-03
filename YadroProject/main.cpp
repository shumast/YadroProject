#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <thread>
#include <chrono>
#include <vector>
#include <algorithm>

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
		std::ios::openmode mode = std::ios::binary | std::ios::in | std::ios::out | std::ios::ate;
		file_.open(filename, mode);
		if (!file_.is_open()) {
			throw std::runtime_error("Failed to open file " + filename);
		}
		pos_ = 0;
		size_ = file_.tellg() / sizeof(int);
		write_delay_ = config.GetWriteDelay();
		read_delay_ = config.GetReadDelay();
		rewind_delay_ = config.GetRewindDelay();
		shift_delay_ = config.GetShiftDelay();
		memory_limit_ = config.GetMemoryLimit() / sizeof(int);
	}
	void shiftLeft() {
		makeDelay(shift_delay_);
		if (pos_ == 0) {
			throw std::runtime_error("Can't move left, begin of file");
		}
		pos_--;
	}
	void shiftRight() {
		makeDelay(shift_delay_);
		if (pos_ == size_) {
			throw std::runtime_error("Can't move right, end of file");
		}
		pos_++;
	}
	void rewind() {
		makeDelay(pos_ * rewind_delay_);
		pos_ = 0;
	}
	int read() {
		makeDelay(read_delay_);
		int ans = 0;
		file_.seekg(pos_ * sizeof(int), std::ios::beg);
		file_.read(reinterpret_cast<char*>(&ans), sizeof(int));
		return ans;
	}
	void write(int val) {
		makeDelay(write_delay_);
		file_.seekg(pos_ * sizeof(int), std::ios::beg);
		file_.write(reinterpret_cast<char*>(&val), sizeof(int));
		if (pos_ == size_) {
			size_++;
		}
	}
	void clear(const std::string& filename) {
		file_.close();
		std::ios::openmode mode = std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc;
		file_.open(filename, mode);
		if (!file_.is_open()) {
			throw std::runtime_error("Failed to open file " + filename);
		}
		pos_ = 0;
		size_ = 0;
	}
	long long size() const {
		return size_;
	}
	long long memotyLimit() const {
		return memory_limit_;
	}
	~Tape() {
		if (file_.is_open()) {
			file_.close();
		}
	}

private:
	long long write_delay_ = 0;
	long long read_delay_ = 0;
	long long rewind_delay_ = 0;
	long long shift_delay_ = 0;
	long long memory_limit_ = 0;
	long long size_ = 0;
	long long pos_ = 0;
	std::fstream file_;
	void makeDelay(long long delay) {
		auto start = std::chrono::high_resolution_clock::now();
		while (true) {
			auto elapsed = std::chrono::high_resolution_clock::now() - start;
			if (std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count() >= delay) {
				break;
			}
		}
	}
};

void makeTapeSort(Tape& inputTape, Tape& outputTape, Config& config) {
	const std::string tmp1 = "tmp/tmp1.bin";
	const std::string tmp2 = "tmp/tmp2.bin";
	const std::string tmp3 = "tmp/tmp3.bin";
	const std::string tmp4 = "tmp/tmp4.bin";
	Tape tmpTape1(tmp1, config);
	Tape tmpTape2(tmp2, config);
	Tape tmpTape3(tmp3, config);
	Tape tmpTape4(tmp4, config);
	tmpTape1.clear(tmp1);
	tmpTape2.clear(tmp2);
	tmpTape3.clear(tmp3);
	tmpTape4.clear(tmp4);
	std::vector<int> buffer;
	long long i = 0, szBlock = inputTape.memotyLimit(), cnt = 1, count = 0;
	while (i < inputTape.size()) {
		for (long long j = 0; j < szBlock; j++) {
			if (i == inputTape.size()) {
				break;
			}
			buffer.push_back(inputTape.read());
			inputTape.shiftRight();
			i++;
		}
		std::sort(buffer.begin(), buffer.end());
		if (cnt & 1) {
			for (long long j = 0; j < buffer.size(); j++) {
				tmpTape1.write(buffer[j]);
				tmpTape1.shiftRight();
			}
		}
		else {
			for (long long j = 0; j < buffer.size(); j++) {
				tmpTape2.write(buffer[j]);
				tmpTape2.shiftRight();
			}
		}
		cnt++;
		buffer.clear();
	}
	while (szBlock < inputTape.size()) {
		cnt = 1;
		count++;
		if (count & 1) {
			tmpTape1.rewind();
			tmpTape2.rewind();
			tmpTape3.clear(tmp3);
			tmpTape4.clear(tmp4);
			long long i = 0, j = 0;
			while (i + j < inputTape.size()) {
				int val1 = 0, val2 = 0;
				bool act1 = false, act2 = false;
				while (i < std::min(cnt * szBlock, tmpTape1.size()) || j < std::min(cnt * szBlock, tmpTape2.size())) {
					if (i < std::min(cnt * szBlock, tmpTape1.size()) && j < std::min(cnt * szBlock, tmpTape2.size())) {
						if (!act1) {
							val1 = tmpTape1.read();
							act1 = true;
						}
						if (!act2) {
							val2 = tmpTape2.read();
							act2 = true;
						}
						if (val1 < val2) {
							i++;
							act1 = false;
							tmpTape1.shiftRight();
							if (cnt & 1) {
								tmpTape3.write(val1);
								tmpTape3.shiftRight();
							}
							else {
								tmpTape4.write(val1);
								tmpTape4.shiftRight();
							}
						}
						else {
							j++;
							act2 = false;
							tmpTape2.shiftRight();
							if (cnt & 1) {
								tmpTape3.write(val2);
								tmpTape3.shiftRight();
							}
							else {
								tmpTape4.write(val2);
								tmpTape4.shiftRight();
							}
						}
					}
					else if (i < std::min(cnt * szBlock, tmpTape1.size())) {
						if (!act1) {
							val1 = tmpTape1.read();
							act1 = true;
						}
						i++;
						act1 = false;
						tmpTape1.shiftRight();
						if (cnt & 1) {
							tmpTape3.write(val1);
							tmpTape3.shiftRight();
						}
						else {
							tmpTape4.write(val1);
							tmpTape4.shiftRight();
						}
					}
					else if (j < std::min(cnt * szBlock, tmpTape2.size())) {
						if (!act2) {
							val2 = tmpTape2.read();
							act2 = true;
						}
						j++;
						act2 = false;
						tmpTape2.shiftRight();
						if (cnt & 1) {
							tmpTape3.write(val2);
							tmpTape3.shiftRight();
						}
						else {
							tmpTape4.write(val2);
							tmpTape4.shiftRight();
						}
					}
				}
				cnt++;
			}
		}
		else {
			tmpTape1.clear(tmp1);
			tmpTape2.clear(tmp2);
			tmpTape3.rewind();
			tmpTape4.rewind();
			long long i = 0, j = 0;
			while (i + j < inputTape.size()) {
				int val1 = 0, val2 = 0;
				bool act1 = false, act2 = false;
				while (i < std::min(cnt * szBlock, tmpTape3.size()) || j < std::min(cnt * szBlock, tmpTape4.size())) {
					if (i < std::min(cnt * szBlock, tmpTape3.size()) && j < std::min(cnt * szBlock, tmpTape4.size())) {
						if (!act1) {
							val1 = tmpTape3.read();
							act1 = true;
						}
						if (!act2) {
							val2 = tmpTape4.read();
							act2 = true;
						}
						if (val1 < val2) {
							i++;
							act1 = false;
							tmpTape3.shiftRight();
							if (cnt & 1) {
								tmpTape1.write(val1);
								tmpTape1.shiftRight();
							}
							else {
								tmpTape2.write(val1);
								tmpTape2.shiftRight();
							}
						}
						else {
							j++;
							act2 = false;
							tmpTape4.shiftRight();
							if (cnt & 1) {
								tmpTape1.write(val2);
								tmpTape1.shiftRight();
							}
							else {
								tmpTape2.write(val2);
								tmpTape2.shiftRight();
							}
						}
					}
					else if (i < std::min(cnt * szBlock, tmpTape3.size())) {
						if (!act1) {
							val1 = tmpTape3.read();
							act1 = true;
						}
						i++;
						act1 = false;
						tmpTape3.shiftRight();
						if (cnt & 1) {
							tmpTape1.write(val1);
							tmpTape1.shiftRight();
						}
						else {
							tmpTape2.write(val1);
							tmpTape2.shiftRight();
						}
					}
					else if (j < std::min(cnt * szBlock, tmpTape4.size())) {
						if (!act2) {
							val2 = tmpTape4.read();
							act2 = true;
						}
						j++;
						act2 = false;
						tmpTape4.shiftRight();
						if (cnt & 1) {
							tmpTape1.write(val2);
							tmpTape1.shiftRight();
						}
						else {
							tmpTape2.write(val2);
							tmpTape2.shiftRight();
						}
					}
				}
				cnt++;
			}
		}
		szBlock *= 2;
	}
	if (count & 1) {
		tmpTape3.rewind();
		for (long long i = 0; i < inputTape.size(); i++) {
			outputTape.write(tmpTape3.read());
			tmpTape3.shiftRight();
			outputTape.shiftRight();
		}
	}
	else {
		tmpTape1.rewind();
		for (long long i = 0; i < inputTape.size(); i++) {
			outputTape.write(tmpTape1.read());
			tmpTape1.shiftRight();
			outputTape.shiftRight();
		}
	}
}

void makeFile() {
	std::ofstream file("input.bin", std::ios::binary);
	int n = 100'000;
	std::vector<int> data(n);
	for (int i = 0; i < n; i++) {
		data[i] = rand();
	}
	for (int val : data) {
		file.write(reinterpret_cast<const char*>(&val), sizeof(int));
	}
	file.close();
}

void makeTestTape(Tape& tape) {
	for (long long i = 0; i < tape.size(); i++) {
		tape.shiftRight();
	}
	for (long long i = 0; i < tape.size(); i++) {
		tape.shiftLeft();
	}
}

int main(int argc, char* argv[]) {
	makeFile();

	if (argc != 3) {
		std::cerr << "Bad input\n";
		return 1;
	}
	std::string inputFile = argv[1], outputFile = argv[2], configFile = "config.ini";
	try {
		Config config(configFile);
		Tape inputTape(inputFile, config);
		Tape outputTape(outputFile, config);
		
		//makeTestTape(inputTape);

		makeTapeSort(inputTape, outputTape, config);
	} catch (const std::exception& e) {
		std::cerr << e.what() << "\n";
		return 1;
	}

	return 0;
}