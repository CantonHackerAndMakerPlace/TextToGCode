#define _CRT_SECURE_NO_WARNINGS 1

#include <string>
#include <iostream>
#include <vector>
#include <fstream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h" /* http://nothings.org/stb/stb_image_write.h */

#define STB_TRUETYPE_IMPLEMENTATION 
#include "stb_truetype.h" /* http://nothings.org/stb/stb_truetype.h */

#include "dirent.h"

#include "SerialPort.h"

void writeText(const char* font, const char* file, const char* word) {

	long size;
	unsigned char* fontBuffer;

	/* load font file */ {
		FILE* fontFile = fopen(font, "rb");
		fseek(fontFile, 0, SEEK_END);
		size = ftell(fontFile); /* how long is the file ? */
		fseek(fontFile, 0, SEEK_SET); /* reset */

		fontBuffer = (unsigned char*)malloc(size);

		fread(fontBuffer, size, 1, fontFile);
		fclose(fontFile);
	}

	/* prepare font */
	stbtt_fontinfo info;
	if (!stbtt_InitFont(&info, fontBuffer, 0)) {
		printf("failed\n");
		return;
	}

	int l_h = 64; /* line height */
	int b_w = strlen(word) * 64; /* bitmap width */
	int b_h = 64; /* bitmap height */

				  /* create a bitmap for the phrase */
	unsigned char* bitmap = (unsigned char*)malloc(b_w * b_h);

	/* Make it black*/
	for (int i = 0; i < b_h * b_w;i++) {
		bitmap[i] = 0;
	}

	/* calculate font scaling */
	float scale = stbtt_ScaleForPixelHeight(&info, l_h);

	int x = 0;

	int ascent, descent, lineGap;
	stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

	ascent *= scale;
	descent *= scale;

	int i;
	for (i = 0; i < strlen(word); ++i) {
		/* get bounding box for character (may be offset to account for chars that dip above or below the line */
		int c_x1, c_y1, c_x2, c_y2;
		stbtt_GetCodepointBitmapBox(&info, word[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);

		/* compute y (different characters have different heights */
		int y = ascent + c_y1;

		/* render character (stride and offset is important here) */
		int byteOffset = x + (y  * b_w);
		stbtt_MakeCodepointBitmap(&info, bitmap + byteOffset, c_x2 - c_x1, c_y2 - c_y1, b_w, scale, scale, word[i]);

		/* how wide is this character */
		int ax;
		stbtt_GetCodepointHMetrics(&info, word[i], &ax, 0);
		x += ax * scale;

		/* add kerning */
		int kern;
		kern = stbtt_GetCodepointKernAdvance(&info, word[i], word[i + 1]);
		x += kern * scale;
	}


	std::string filename(file);
	filename += ".bmp";
	/* save out a 1 channel image */
	stbi_write_bmp(filename.c_str(), b_w, b_h, 1, bitmap);

	free(fontBuffer);
	free(bitmap);
}

void getFonts(std::vector<std::string>& fonts) {
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir("fonts")) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			std::string name = "fonts/";
			switch (ent->d_type)
			{
			case DT_REG:
				name += ent->d_name;
				fonts.push_back(name);
				break;
			default:
				break;
			}
		}
		closedir(dir);
	}
	else {
		/* could not open directory */
		perror("Unalbe to open directory fonts.");
	}
}

void doTrace(const std::string& name) {
	std::string potfile = "ext\\trace.exe -s ";
	potfile += name;
	potfile += ".bmp -o ";
	potfile += name;
	potfile += ".svg";
	system(potfile.c_str());
}

void doGcode(const std::string& name) {
	std::string gcode = "ext\\gcode.exe ";
	gcode += name;
	gcode += ".svg -o ";
	gcode += name;
	gcode += ".gcode";
	system(gcode.c_str());
}

void sendGcode(const std::string& name, const std::string& port) {
	

	//String for incoming data
	char incomingData[MAX_DATA_LENGTH];

	SerialPort cnc(port.c_str());
	if (cnc.isConnected()) std::cout << "Connection Established" << std::endl;
	else std::cout << "ERROR, check port name";
	std::ifstream file(name + ".gcode");
	std::string line;

	while (cnc.isConnected() && std::getline(file, line)) {
		cnc.writeSerialPort(line.c_str(), line.size()); //Write on line of gcode
		//Check if data has been read or not
		int read_result = cnc.readSerialPort(incomingData, MAX_DATA_LENGTH);

		if (strcmp(incomingData, "OK") == 0) {
			puts(incomingData);
		}
		Sleep(10);
	}

}

int main(int argc, const char * argv[]) {
	

	std::vector<std::string> fonts;
	
	getFonts(fonts);

	char str[100];
	std::cout << "Input text:";
	std::cin.getline(str, 100);

	char str2[100];
	std::cout << "Filename:";
	std::cin.getline(str2, 100);

	std::string fontname = "";
	fontname += fonts[rand() % fonts.size()];

	writeText(fontname.c_str(), str2, str); //Create a file

	doTrace(std::string(str2)); //Convert bmp to svg

	doGcode(std::string(str2)); //Convert svg to gcode

	std::vector<std::string> ports;
	SerialPort::GetPortNames(ports); //Enumerate ports.

	if (ports.size() != 1) {
		std::cout << "Problem with ports. Spesify one.";
		std::cin.get();
		return EXIT_FAILURE;
	}

	sendGcode(str2, ports[0]);

	return EXIT_SUCCESS;
}
