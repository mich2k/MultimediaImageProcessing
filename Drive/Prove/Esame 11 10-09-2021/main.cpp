#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <string>
#include <functional>
#include <exception>
#include <cctype>
#include <sstream>
#include "image_operations.h"
#include "types.h"
#include "ppm.h"
#include "image.h"

using namespace std;

#include "ppm.h"

struct  object {
	string typeLabel;
};

struct canvas : object {
	uint16_t width;
	uint16_t height;
	vec3b background;
	canvas() { typeLabel = "canvas"; }
};

struct figure : object {
	image<vec3b> imgData;
	vector<string> features_;
	size_t x, y, width, height;
	void printFeaturesInline() {
		cout << this->typeLabel << " : ";
		for (auto& feature : features_)
			cout << feature << ",";
		cout << "\n";
	}
};

void syntax() {
	cout << "\n\tSintassi del programma ubj2ppm:"
		<< "\n\tubj2ppm <inputFilename> <outputFilename>\n";
	exit(EXIT_FAILURE);
}

int error(const string& errorMessage) {
	cout << "\n\tERRORE: " + errorMessage + "\n";
	return EXIT_FAILURE;
}

void flushOBJsOnstdout(vector<figure>& objs) {
	for (auto& obj : objs) {
		if(!obj.typeLabel.empty())
			obj.printFeaturesInline();
	}
}

uint16_t LEtoBE(uint8_t lessSignificant, uint8_t mostSignificant) {
	stringstream stream;
	stream.write(reinterpret_cast<char*>(&mostSignificant), 1);
	stream.write(reinterpret_cast<char*>(&lessSignificant), 1);
	uint16_t valBE;
	stream.read(reinterpret_cast<char*>(&valBE), 2);
	return valBE;
}

image<vec3b> outputCanvas(canvas toBeCreated) {
	image<vec3b> img(toBeCreated.width, toBeCreated.height);
	for (auto& tripletta : img)
		tripletta = toBeCreated.background;
	writeP6("canvas.ppm", img);
	return img;
}

void setStreamToData(istream& is, vector<figure> objs) {
	string line;
	is.seekg(0);
	size_t nImages = 0;
	for (auto& elem : objs) {
		if (elem.typeLabel == "image")
			++nImages;
	}
	size_t counter = 0;
	while (getline(is, line, '[')) {
		if (line.find("data") != string::npos) {
			if (counter == nImages)
				break;
			++counter;
		}
	}
	size_t pos = is.tellg();
	pos = pos + 5;
	is.seekg(pos);
	if (is.peek() == 'z')
		++pos;
	is.seekg(pos);
}

void outputImages(vector<figure>& objs) {
	size_t counter = 1;
	for (auto& figura : objs) {
		if (figura.typeLabel == "image") {
			string filename = "image" + to_string(counter);
			filename.append(".ppm");
			writeP6(filename, figura.imgData);
			++counter;
			break; // Perché a quanto pare non bisogna produrre più di un'immagine. Mah
		}
	}
}

figure getImageData(istream& is, vector<figure> objs, string &elementContent) {

	figure image;
	image.typeLabel = "image"; image.features_.push_back("x"); image.features_.push_back("y"); image.features_.push_back("width"); image.features_.push_back("height"); image.features_.push_back("data");
	size_t imageLabelPos = elementContent.find("image"); // Individuo la posizione dell'immagine
	elementContent = elementContent.substr(imageLabelPos, elementContent.length() - imageLabelPos);  // Trim per ripulire il contenuto da ciò che precede.
	size_t xPos = elementContent.find("x"); // x
	image.x = (uint8_t)elementContent[xPos + 2];
	size_t yPos = elementContent.find("y"); // y
	image.y = (uint8_t)elementContent[yPos + 2];
	string label = "width"; // width
	image.width = (uint8_t)elementContent[elementContent.find(label) + label.length() + 1];
	label = "height"; // height
	image.height = (uint8_t)elementContent[elementContent.find(label) + label.length() + 1];
	image.imgData.resize(image.width, image.height);
	size_t dataPos = elementContent.find("[") + 5; // data
	// Qui la situazione si complica: incappando nel carattere '}' come dato, si rischia di interrompere il flusso di dati.
	// Riapro lo stream e mi setto in prossimità della parentesi '['
	setStreamToData(is, objs);
	for (size_t y = 0; y < image.imgData.height(); ++y) { 
		for (size_t x = 0; x < image.imgData.width(); ++x) {
			is.read(reinterpret_cast<char*>(&image.imgData(x, y)[0]), 1);
			is.read(reinterpret_cast<char*>(&image.imgData(x, y)[1]), 1);
			is.read(reinterpret_cast<char*>(&image.imgData(x, y)[2]), 1);
		}
	}
	if (is.peek() != '}') {
		setStreamToData(is, objs);
		// Scarto il primo pixel:
		is.read(reinterpret_cast<char*>(&image.imgData(0, 0)[0]), 1);
		is.read(reinterpret_cast<char*>(&image.imgData(0, 0)[1]), 1);
		is.read(reinterpret_cast<char*>(&image.imgData(0, 0)[2]), 1);
		for (size_t y = 0; y < image.imgData.height(); ++y) {
			for (size_t x = 0; x < image.imgData.width(); ++x) {
				is.read(reinterpret_cast<char*>(&image.imgData(x, y)[0]), 1);
				is.read(reinterpret_cast<char*>(&image.imgData(x, y)[1]), 1);
				is.read(reinterpret_cast<char*>(&image.imgData(x, y)[2]), 1);
			}
		}
	}
	return image;
}

string getWord(string& elementContent, size_t &i) {
	size_t init = i;
	string word;
	while (isalpha(elementContent[i]) || elementContent[i] == '-') {
		++i;
	}
	word.clear();
	if (init != i) 
		word = elementContent.substr(init, i - init);
	return word;
}

figure getFigureFeatures(string& elementContent) {
	figure currentFigure;
	if (elementContent.find("width") == string::npos)
		return currentFigure;
	for (size_t i = 0; i < elementContent.length(); ++i) {
		if (elementContent[i] == 'i') {
			i = i + 2; // Salto l'elemento d'indentazione
			string feature = getWord(elementContent, i);
			// Controlli:
			if (!feature.empty() && feature != "elements") {
				if (feature[feature.length() - 1] == 'i')
					feature.pop_back();
				if (currentFigure.typeLabel.empty()) currentFigure.typeLabel = feature; // Se non ho ancora l'etichetta iniziale, la inserisco
				else currentFigure.features_.push_back(feature); // Altrimenti inserisco una feature normale
			}
		}
	}
	return currentFigure;
}

void pasteImages(image<vec3b> &img, vector<figure>& objs) {
	for (auto& element : objs) {
		if (element.typeLabel == "image")
			paste(img, element.imgData, element.x, element.y);
	}
}

vector<figure> processUBJContent(istream& is) {
	vector<figure> objs;
	string elementConent;
	while (getline(is, elementConent, '}')) {
		if (elementConent.find("image") != string::npos)
			objs.push_back(getImageData(is, objs, elementConent));
		else objs.push_back(getFigureFeatures(elementConent));
		//if (elementConent.length() < 10) break;
	}
	return objs;
}

canvas canvasReader(std::istream& is, unsigned& w, unsigned& h) {
	string canvasContent;
	canvas canvasRead;
	getline(is, canvasContent, '}');
	string label = "width"; // w
	size_t pos = canvasContent.find(label);
	canvasRead.width = canvasContent[pos + label.length() + 1];
	if (canvasContent[pos + label.length() + 2] != 'i') {
		canvasRead.width = LEtoBE(canvasContent[pos + label.length() + 1], canvasContent[pos + label.length() + 2]);
	}
	label = "height"; // h
	pos = canvasContent.find(label);
	canvasRead.height = canvasContent[pos + label.length() + 1];
	if (canvasContent[pos + label.length() + 2] != 'i') {
		canvasRead.height = LEtoBE(canvasContent[pos + label.length() + 1], canvasContent[pos + label.length() + 2]);
	}
	label = "background[$U#i"; // background
	pos = canvasContent.find(label);
	canvasRead.background[0] = canvasContent[pos + label.length() + 1];
	canvasRead.background[1] = canvasContent[pos + label.length() + 2];
	canvasRead.background[2] = canvasContent[pos + label.length() + 3];
	w = canvasRead.width;
	h = canvasRead.height;
	return canvasRead;
}

int convert(const string& sInput, const string& sOutput) {

	// Apro i due file:
	ifstream is(sInput, std::ios::binary);
	if (!is) { return error("Impossibile aprire il file di input " + sInput); }
	ofstream os(sOutput, std::ios::binary);
	if (!os) { return error("Impossibile aprire il file di output " + sOutput); }

	// Dal file UBJ devo estrarre le informazioni e creare il canvas
	unsigned w, h;
	canvas canvasRead = canvasReader(is, w, h);
	image<vec3b> canvasImage = outputCanvas(canvasRead);
	image<vec3b> img(w, h); // Immagine finale da ritornare
	// Per accedere ai pixel di img posso usare img(x,y) oppure img.begin() e img.end()
	paste(img, canvasImage, 0, 0);
	// Dal file UBJ devo estrarre le informazioni sulle immagini da incollare su img 
	vector<figure> objs = processUBJContent(is);
	// Output su linea di comando
	flushOBJsOnstdout(objs);
	// Output in formato PPM
	outputImages(objs);
	// Incollo le immagini sul canvas
	pasteImages(img, objs);
	if (!writeP6(sOutput, img))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {

	// Linea di comando
	if (argc != 3) {
		syntax();
	}
	string sInput = argv[1];
	string sOutput = argv[2];

	return convert(sInput, sOutput);
}
