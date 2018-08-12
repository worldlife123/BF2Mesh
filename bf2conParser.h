#ifndef _BF2CONPARSER_H_
#define _BF2CONPARSER_H_

#include <vector>
#include <map>
#include "bf2Object.h"

typedef int CONFUNC(std::vector<std::string>);

bool StrMatch(const char *a, const char *b);

class BF2ConParser
{
public:
	//constructor&destructor
	BF2ConParser();
	BF2ConParser(std::string filename);
	~BF2ConParser();
	void process(std::string command);
	void invoke(std::vector<std::string> args);	
	void loadFiles();

private:
	std::ostream& m_os;
	std::string m_currentDir;
	std::vector<std::string> m_filesToLoad;
	//std::vector<BF2ObjectTemplate> m_objTemplates;
	std::map<std::string, BF2ObjectTemplate> m_objTemplates;
	std::string m_activeObjectTemplate;
	//std::string m_activeObjectTemplateDummy;
	
	//con function implementations
	std::map<std::string, int (*)(std::vector<std::string>)> m_conFunctionList;
	int ObjectTemplate_create(std::vector<std::string> argv);
};

#endif