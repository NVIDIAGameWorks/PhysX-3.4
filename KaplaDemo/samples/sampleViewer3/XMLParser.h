#ifndef XML_PARSER_H
#define XML_PARSER_H

#include <stdio.h>
#include <string>
#include <vector>
#include "FileScanner.h"

#define XML_LINE_MAX 1024


// -------------------------------------------------------------------------
struct XMLVariable {
	std::string name;
	std::string value;
};

// -------------------------------------------------------------------------
class XMLParser {
public: 
	// singleton pattern
	static XMLParser* getInstance();
	static void destroyInstance();

	bool open(std::string filename);
	void close();
	bool endOfFile();
	bool readNextTag();

	std::string tagName;
	std::vector<XMLVariable> vars;
	bool closed;
	bool endTag;

	FileScanner &getFileScanner() { return fs; }

	// ezm
	void ezmParseSemantic(int &count);
	void ezmReadSemanticEntry(int nr);

	std::vector<std::string> ezmAttrs;
	std::vector<std::string> ezmTypes;
	std::vector<float> ezmFloats;
	std::vector<int> ezmInts;

private:
	XMLParser();
	~XMLParser();

	void clearTag();
	FileScanner fs;
};

#include <stdio.h>

#define FILE_SCANNER_LINE_MAX 1024



#endif