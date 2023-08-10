#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cstdint>
#include <cassert>
#include <string>
#include <array>
#include <vector>

using namespace std;

template <typename T>
istream& raw_read(istream& in,T& v) {
	return in.read(reinterpret_cast<char*>(&v), sizeof(T));
}

typedef array<uint8_t, 3> field;

struct header {
	uint8_t header_len_ = 0;
	uint8_t protocol_version_ = 0;
	uint16_t profile_version_lsb_ = 0;
	uint32_t data_size_lsb_ = 0;
	string data_type_;
	uint16_t crc_lsb_ = 0;
};


struct recordHeader {
	uint8_t architecture_;
	uint16_t gmn_;
	uint8_t numFields_;
	vector<field> fields_;
	bool isDefinitionMessage_;
};

class fit_decoder {
private:

	header h;
	istream& in_;
	vector<recordHeader> records_;


	void readHeader() {
		uint8_t v = 4;
		raw_read(in_, h.header_len_);
		raw_read(in_, h.protocol_version_);
		raw_read(in_, h.profile_version_lsb_);
		raw_read(in_, h.data_size_lsb_);
		while (v-- > 0)
			h.data_type_.push_back(in_.get());
		raw_read(in_, h.crc_lsb_);
	}



	void readRecord() {
		recordHeader record;
		uint8_t record_header = 0;
		raw_read(in_, record_header);
		record.isDefinitionMessage_ = record_header >> 4 == 0x04 ? true : false;
		in_.ignore(1);	// brucio il reserved (1 byte)
		raw_read(in_, record.architecture_);
		uint16_t gmn = 0;
		if (record.architecture_ == 0) {
			raw_read(in_, record.gmn_);
		} else { // big endian
			uint16_t t = 0;
			raw_read(in_, t);
			unsigned mask = (1 >> 8);
			mask--;
			record.gmn_ = (t >> 8) & mask;
			record.gmn_ <<= 8;
			record.gmn_ |= t & mask;
		}
		raw_read(in_, record.numFields_);

		for (size_t i = 0; i < record.numFields_; i++) {
			field this_field = { 0,0,0 };
			raw_read(in_, this_field[0]);
			raw_read(in_, this_field[1]);
			raw_read(in_, this_field[2]);
			record.fields_.push_back(this_field);
		}

		records_.push_back(record);

	}

	void FitCRC_Get16(uint16_t& crc, uint8_t byte)
	{
		static const uint16_t crc_table[16] =
		{
			0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
			0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400
		};
		uint16_t tmp;
		// compute checksum of lower four bits of byte
		tmp = crc_table[crc & 0xF];
		crc = (crc >> 4) & 0x0FFF;
		crc = crc ^ tmp ^ crc_table[byte & 0xF];
		// now compute checksum of upper four bits of byte
		tmp = crc_table[crc & 0xF];
		crc = (crc >> 4) & 0x0FFF;
		crc = crc ^ tmp ^ crc_table[(byte >> 4) & 0xF];
	}

public:

	fit_decoder(istream& in) : in_(in) {
		readHeader();
	};


	bool checkCRC() {
		uint8_t t = 0;
		uint16_t crc = 0;

		FitCRC_Get16(crc, h.header_len_);
		FitCRC_Get16(crc, h.protocol_version_);

		// non necessaria la maschera, bastava l'assegnamento o |=
		unsigned mask = (1 >> 8);
		mask--;
		t = (h.profile_version_lsb_ & mask);
		FitCRC_Get16(crc, t);

		t = (h.profile_version_lsb_ >> 8);
		FitCRC_Get16(crc, t);


		t = h.data_size_lsb_;
		FitCRC_Get16(crc, t);
		t = h.data_size_lsb_ >> 8;
		FitCRC_Get16(crc, t);
		t = h.data_size_lsb_ >> 16;
		FitCRC_Get16(crc, t);
		t = h.data_size_lsb_ >> 24;
		FitCRC_Get16(crc, t);


		FitCRC_Get16(crc, h.data_type_[0]);
		FitCRC_Get16(crc, h.data_type_[1]);
		FitCRC_Get16(crc, h.data_type_[2]);
		FitCRC_Get16(crc, h.data_type_[3]);

		return h.crc_lsb_ == crc;
	}




	void readRecords() {
		for (uint32_t r = 0; r < h.data_size_lsb_; ++r) {
			uint8_t dataSegmentToIgnore = 0;
			readRecord();

			if (r == 1) {
				in_.ignore(1);// header
				for (int i = 0; i < 3; i++) {
					uint8_t field_size = records_[r].fields_[i][1];
					in_.ignore(field_size);
				}
				uint32_t time_created = 0;
				raw_read(in_, time_created);
				cout << "time_created = " << time_created << endl;

			} else {
				if (records_[r].gmn_ == 19) { // lap
					field avg_speed = records_[r].fields_[13];
					cout << "avg_speed =" << avg_speed[0] * 1e6<< " hm/h" << endl;
				}
			}
		}
	}

};

int main(int argc, char **argv){
	if (argc != 2)
		return EXIT_FAILURE;
	ifstream in(argv[1]);
	if (!in)
		return EXIT_FAILURE;

	fit_decoder f(in);

	bool ret = f.checkCRC();
	if (!ret)
		return EXIT_FAILURE;

	cout << "Header CRC OK!";
	f.readRecords();

	return EXIT_SUCCESS;
}