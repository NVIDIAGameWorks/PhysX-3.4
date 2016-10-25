#ifndef FILESCANNER_H
#define FILESCANNER_H

#include <stdio.h>

#define EOL 10
#define MAX_BONDS 5

#ifdef __BCPLUSPLUS__
#define STRCMPI(s, t) (strcmpi(s, t))
#elif defined (__WXWINDOWS25__) && defined (__WXMAC__) && defined (__WXMAC_XCODE__)
#define STRCMPI(s, t) (strcasecmp(s, t))
#else
#define STRCMPI(s, t) (_stricmp(s, t))
#endif

#ifdef __BCPLUSPLUS__
#define STRNCMPI(s, t, m) (strncmpi(s, t, m))
#elif defined (__WXWINDOWS25__) && defined (__WXMAC__) && defined (__WXMAC_XCODE__)
#define STRNCMPI(s, t, m) (strncasecmp(s, t, m))
#else
#define STRNCMPI(s, t, m) (_strnicmp(s, t, m))
#endif


// ------------------------------------------------------------------------------------
class FileScanner {
// ------------------------------------------------------------------------------------
public:
	FileScanner(); 
	~FileScanner();

	bool open(const char *filename);
	void close();

	void getSymbol(char *sym);
	bool checkSymbol(const char *expectedSym, bool caseSensitive = false);
	bool getIntSymbol(int &i);
	bool getFloatSymbol(float &r);

	bool getBinaryData(void *buffer, int size);  // must start after newline

	int  getLineNr() const { return lineNr; }
	int  getPositionNr() const { return (int)(pos - line); }

	bool endReached() const { if (feof(f)) return true; else return false; }

	void setErrorMessage(char *msg);

	char errorMessage[256];

	// ------------------------------------------------------
	static bool equalSymbol(const char *sym, const char *expectedSym, bool caseSensitive = false);
	static const int maxLineLen = 1024;

private:
	void getNextLine();
	FILE *f;
	int  lineNr;
	char line[maxLineLen+1];
	char *pos;
};

#endif