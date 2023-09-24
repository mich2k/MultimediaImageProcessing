#include <fstream>
#include <string>
#include <vector>


uint8_t get_high_field(uint8_t& bit)
{
	uint8_t high = 0;
	for (int i = 7; i > 3; --i) {
		high = (high << 1) | ((bit >> i) & 1);
	}
	return high;
}

uint8_t get_low_field(uint8_t& bit) {

	uint8_t low = 0;
	for (int i = 3; i >= 0; --i) {
		low = (low << 1) | ((bit >> i) & 1);
	}

	return low;
}


bool decompress_lz4(std::string filein, std::string fileout)
{
	if (filein.empty() || fileout.empty()) {
		return false;
	}

	std::ifstream is(filein, std::ios::binary);
	std::ofstream os(fileout, std::ios::binary);
	if (!is || !os) {
		return false;
	}
	char  magic_number[4], check[4] = { 3,33,76,24 };
	int32_t length, costant, block_lenght;
	uint8_t token, high = 0, low = 0, more_byte, byte = 0;
	uint16_t offset = 0;
	int literal_length, minmatch = 0;
	is.read(&(magic_number[0]), 4);
	for (int i = 0; i < 4; ++i) {
		if (magic_number[i] != check[i]) {
			return false;
		}
	}
	is.read(reinterpret_cast<char*>(&length), 4);
	is.read(reinterpret_cast<char*>(&costant), 4);
	is.read(reinterpret_cast<char*>(&block_lenght), 4);
	if (costant != 1291845632) {
		return false;
	}
	std::vector<uint8_t> literals;
	int i = 0;
	int index;
	while (i < (block_lenght - 6)) {

		token = is.get();
		++i;
		high = get_high_field(token);
		low = get_low_field(token);
		more_byte = 0;
		literal_length = 0;

		if (high == 0) //NO literals
		{
			is.read(reinterpret_cast<char*>(&offset), 2);
			i += 2;
			if (offset != 0) {
				return false;
			}
		} else {

			if (high == 15) //altri due
			{
				byte = is.get();
				++i;
				while (byte == 255) {
					more_byte += byte;
					byte = is.get();
					++i;
				}
				more_byte += byte + 4;
				byte = is.get();
				++i;
				byte = ((byte & 0b1111'0000) >> 4);
				while (byte == 255) {
					more_byte += byte;
					byte = is.get();
					++i;
				}
				more_byte += byte + 4;


				literal_length = high + more_byte;
			} else {

				literal_length = high;
			}

			for (int j = 0; j < literal_length; ++j) {
				literals.push_back(is.get());
				++i;
			}

		}
		is.read(reinterpret_cast<char*>(&offset), 2);
		i += 2;
		if (offset <= 0 || offset > 65535) {
			return false;
		}

		if (low == 0) {
			minmatch = 4;
		} else if (low == 15) {
			minmatch = 19;
			byte = is.get();
			++i;
			while (byte == 255) {
				minmatch += byte;
				byte = is.get();
				++i;
			}
			minmatch += byte + 4; // e x a m p l e 1 8

		}


		for (int j = 0; j < offset; ++j) {
			os.put(literals[j]);
		}

		for (int k = 0; k < (minmatch / offset); ++k) {
			for (int j = offset - 1; j >= 0; --j) {
				index = (int)literals.size() - (j + 1);
				os.put(literals[index]);

			}
		}

	}


	//ultimi byte 
	for (int j = offset - 1; j > 5; --j) {
		index = (int)literals.size() - (j + 1);
		os.put(literals[index]);

	}

	token = is.get();

	high = (token & 0b1111'0000) >> 4;
	if (high != 5) {
		return false;
	}

	for (int i = 0; i < high; ++i) {
		os.put(is.get());
	}


	return true;
}



int main(int argc, char** argv)
{

	if (argc != 3) {
		return 1;
	}

	decompress_lz4(argv[1], argv[2]);

}