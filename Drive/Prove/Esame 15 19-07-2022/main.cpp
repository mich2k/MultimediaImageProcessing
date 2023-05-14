#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cstdint>
#include <cassert>
#include <string>
#include <unordered_map>
#include <iomanip>
#include <sstream>

void syntax() {
	std::cout << "\n\tSintassi del programma fitdump:"
				<< "\n\tfitdump <input file.FIT>\n";
	exit(EXIT_FAILURE);
}

void error(const std::string& message) {
	std::cout << "\n\tERRORE: " << message << "\n";
	exit(EXIT_FAILURE);
}

struct fit_header {

	uint8_t headerSize;
	uint8_t protocolVersion;
	uint16_t profileVersion;
	uint32_t dataSize;
	uint8_t dataType[4];
};

struct field {
	uint8_t number, size, baseType;
};

struct fieldData {
	size_t num, value;
};

struct definition_message {
	uint8_t reserved;
	uint8_t architecture;
	uint16_t globalMessageNumber;
	uint8_t numFields;
	std::vector<field> fieldDefinition;
};


void printSpeed(uint16_t val, std::ostream& os) {
	double convertedVal = (val * 3.6) / 1000;
	os << "avg_speed = " << std::setprecision(4) << convertedVal << " km/h\n";
}

void readDefinitionMessage(std::istream& is, definition_message& msg, uint32_t& totalBytes) {
	is.read(reinterpret_cast<char*>(&msg.reserved), 1); ++totalBytes;
	is.read(reinterpret_cast<char*>(&msg.architecture), 1); ++totalBytes;
	is.read(reinterpret_cast<char*>(&msg.globalMessageNumber), 2); totalBytes += 2;
	is.read(reinterpret_cast<char*>(&msg.numFields), 1); ++totalBytes;
	for (uint8_t i = 0; i < msg.numFields; ++i) {
		field f;
		is.read(reinterpret_cast<char*>(&f.number), 1);
		is.read(reinterpret_cast<char*>(&f.size), 1);
		is.read(reinterpret_cast<char*>(&f.baseType), 1);
		msg.fieldDefinition.push_back(f);
		totalBytes += 3;
	}
}

std::vector<fieldData> readMessageFromDefinition(std::istream& is, definition_message def_msg, uint32_t& totalBytes) {
	std::vector<fieldData> fields;
	for (auto& definition : def_msg.fieldDefinition) {
		fieldData couple; couple.num = 0; couple.value = 0;
		if (definition.size > 8) {
			uint8_t flush;
			for (uint8_t i = 0; i < definition.size; ++i) { is.read(reinterpret_cast<char*>(&flush), 1); }
		}
		else is.read(reinterpret_cast<char*>(&couple.value), definition.size);
		totalBytes += definition.size;
		//fields[definition.number] = val;
		couple.num = definition.number;
		fields.push_back(couple);
	}
	return fields;
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

void updateCRC(std::istream& is, uint32_t nBytes, uint16_t& crc) {
	is.seekg((uint32_t)is.tellg() - nBytes);
	for (uint32_t i = 0; i < nBytes; ++i) {
		uint8_t byte;
		is.read(reinterpret_cast<char*>(&byte), 1);
		FitCRC_Get16(crc, byte);
	}
}

void fit_dump(std::istream& is) {

	// 1. Lettura dell'header
	fit_header header;
	uint16_t crc = 0;
	is.read(reinterpret_cast<char*>(&header), sizeof(fit_header));
	updateCRC(is, header.headerSize - 2, crc);
	uint16_t headerCRC;
	is.read(reinterpret_cast<char*>(&headerCRC), 2);
	std::stringstream os;
	if (crc == headerCRC) { os << "Header CRC ok\n"; }
	else { //std::cout << "Header CRC non ok"; 
		exit(EXIT_FAILURE);
	}
	// 2. Lettura del record dei dati
	bool outputTime = false;
	std::unordered_map<uint8_t, definition_message> definitionList;
	for (uint32_t i = 0; i < header.dataSize; ++i) {
		uint8_t recordHeader;
		is.read(reinterpret_cast<char*>(&recordHeader), 1);
		uint8_t top4bits = (recordHeader & 0b11110000) >> 4;
		uint8_t last4bits = (recordHeader & 0b00001111);
		if (top4bits == 0) { // Data message
			definition_message def_msg = definitionList[last4bits]; // Leggo la definizione mediante il numero locale
			std::vector<fieldData> dataFields = readMessageFromDefinition(is, def_msg, i);
			if (!outputTime) {
				for (auto& couple : dataFields) {
					if (couple.num == 4) {
						os << "time_created = " << std::to_string(couple.value) << "\n";
						outputTime = true;
						break;
					}
				}
			}
			if (def_msg.globalMessageNumber == 19) {
				for (auto& couple : dataFields) {
					if (couple.num == 13) {
						printSpeed((uint16_t)couple.value, os);
						break;
					}
				}
			}
		}
		if (top4bits == 4) { // Definition message
			definition_message msg;
			readDefinitionMessage(is, msg, i);
			definitionList[last4bits] = msg;
		}
	}
	std::cout << os.str();
	crc = 0;
	updateCRC(is, header.dataSize, crc);
	// 3. Check CRC
	uint16_t dataCRC;
	is.read(reinterpret_cast<char*>(&dataCRC), 2);
	if (dataCRC != crc) { //std::cout << "File CRC non ok\n"; 
		exit(EXIT_FAILURE); }
	else { 
		std::cout << "File CRC ok\n";
	}
}

int main(int argc, char **argv)
{
	// Gestione della linea di comando
	if (argc != 2) {
		syntax();
	}
	std::string input_filename = argv[1];
	std::ifstream is(input_filename, std::ios::binary);
	if (!is) { error("Impossibile aprire il file di input " + input_filename); }

	fit_dump(is);

	return EXIT_SUCCESS;
}
