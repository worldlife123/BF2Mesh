#include <fstream>  
#include <string>  
#include <iostream>
#include <assert.h>
#include "bf2conParser.h"

#define RETURN_OK 0 
#define RETURN_INVALID_ARGV 1

#define CHECK_ARGS(num) if (args.size() != num) { std::cout << "Error: Invalid arguments for command: " << cmd << std::endl;return;}
#define CHECK_ARGV(num) if (argv.size() != num) return RETURN_INVALID_ARGV;

// case-insensitive string compare
bool StrMatch(const char *a, const char *b)
{
	assert(a != NULL);
	assert(b != NULL);
	const size_t lena = strlen(a);
	const size_t lenb = strlen(b);
	if (lena != lenb) return false;
	for (size_t i = 0; i<lena; i++)
	{
		if (tolower(a[i]) != tolower(b[i])) return false;
	}
	return true;
}

BF2ConParser::BF2ConParser()
	: m_os(std::cout), m_activeObjectTemplate(""), m_currentDir("")
{
	m_conFunctionList =	{
		{ "ObjectTemplate.create", &(BF2ConParser::ObjectTemplate_create) }//,

	};
}

BF2ConParser::BF2ConParser(std::string filename)
	: m_os(std::cout), m_activeObjectTemplate(""), m_currentDir("") //TODO: implement cd
{
	std::ifstream in(filename);

	std::string line;

	if (in) // 有该文件  
	{
		while (getline(in, line)) // line中不包括每行的换行符  
		{
			process(line);
		}
	}
	else // 没有该文件  
	{
		//cout << "no such file" << endl;
	}

}

BF2ConParser::~BF2ConParser()
{
}

void BF2ConParser::process(std::string command)
{
	//empty
	if (command.length() < 1) return;
	//rem
	if (command.length() >= 3 && command.substr(0, 3) == "rem") return;
	//get command
	std::vector<std::string> args;
	size_t startIdx = 0;
	size_t tokenIdx = command.find(' ');
	//TODO: process " token to enable strings with space
	while (tokenIdx != command.npos)
	{
		if (tokenIdx==startIdx) //multiple space
		{
			startIdx++;
			continue;
		}
		args.push_back(command.substr(startIdx, tokenIdx-startIdx));
		startIdx = tokenIdx+1;
		tokenIdx = command.find(' ', startIdx);
	}
	args.push_back(command.substr(startIdx));
	//send command
	//invoke(args);
	try
	{
		std::vector<std::string> argv(args.begin()+1,args.end());
		auto ret = m_conFunctionList[args[0]](argv);
	}
	catch(...)
	{
		m_os << "Unknown command:" << args[0] << "!";
	}
	
}

void BF2ConParser::invoke(std::vector<std::string> args)
{
	if (args.size() == 0) return;
	std::string cmd = args[0];
	//invoke commands
	//std::cout << "command: " << cmd << std::endl;
	if (StrMatch(cmd.c_str(), "ObjectTemplate.create")) {
		CHECK_ARGS(3);
		BF2ObjectTemplate newTemp(args[1], args[2]);
		auto it_objTemp = m_objTemplates.find(args[2]);
		if ( m_objTemplates.end()!=it_objTemp) std::cout << "Warning: Template " << args[2] << "already exist!" << std::endl;
		m_objTemplates[args[2]]=newTemp;
		m_activeObjectTemplate = args[2]; 
		//m_activeObjectTemplateDummy = "";
	}
	else if (StrMatch(cmd.c_str(), "GeometryTemplate.create")) {
		CHECK_ARGS(3);
		m_filesToLoad.push_back(m_currentDir + "\\meshes\\" + args[2] + "." + args[1]);
	}
	else if (StrMatch(cmd.c_str(), "CollisionManager.createTemplate")) {
		CHECK_ARGS(2);
		m_filesToLoad.push_back(m_currentDir + "\\meshes\\" + args[1] + ".collisionmesh");
	}
	else if (StrMatch(cmd.c_str(), "ObjectTemplate.geometry")) {
		CHECK_ARGS(2);
		m_objTemplates[m_activeObjectTemplate].m_geometryName = args[1];
	}
	else if (StrMatch(cmd.c_str(), "ObjectTemplate.collisionMesh")) {
		CHECK_ARGS(2);
		m_objTemplates[m_activeObjectTemplate].m_collisionName = args[1];
	}
	else if (StrMatch(cmd.c_str(), "ObjectTemplate.geometryPart")) {
		CHECK_ARGS(2);
		m_objTemplates[m_activeObjectTemplate].m_geometryPart = std::stoi(args[1]);
	}
	else if (StrMatch(cmd.c_str(), "ObjectTemplate.collisionPart")) {
		CHECK_ARGS(2);
		m_objTemplates[m_activeObjectTemplate].m_collisionPart = std::stoi(args[1]);
	}
	else if (StrMatch(cmd.c_str(), "ObjectTemplate.addTemplate")) {
		CHECK_ARGS(2);
		m_objTemplates[m_activeObjectTemplate].m_subObjectDummys.push_back(BF2ObjectTemplateDummy(args[1]));
	}
	else if (StrMatch(cmd.c_str(), "ObjectTemplate.setPosition")) {
		CHECK_ARGS(2);
		float3 position = {0,0,0};
		size_t startIdx = 0;
		size_t tokenIdx = args[1].find('/');
		if (tokenIdx == args[1].npos) {
			std::cout << "Error: Invalid arguments for command: " << cmd << std::endl; return;
			return;
		}
		position.x = std::stof(args[1].substr(startIdx, tokenIdx - startIdx));
		startIdx = tokenIdx + 1;
		tokenIdx = args[1].find('/', startIdx);
		if (tokenIdx == args[1].npos) {
			std::cout << "Error: Invalid arguments for command: " << cmd << std::endl; return;
			return;
		}
		position.y = std::stof(args[1].substr(startIdx, tokenIdx - startIdx));
		startIdx = tokenIdx + 1;
		tokenIdx = args[1].find('/', startIdx);
		if (tokenIdx == args[1].npos) {
			std::cout << "Error: Invalid arguments for command: " << cmd << std::endl; return;
			return;
		}
		position.z = std::stof(args[1].substr(startIdx, tokenIdx - startIdx));
		m_objTemplates[m_activeObjectTemplate].m_subObjectDummys.end()->position = position;
	}
	else if (StrMatch(cmd.c_str(), "ObjectTemplate.setRotation")) {
		CHECK_ARGS(2);
		float3 rotation = { 0,0,0 };
		size_t startIdx = 0;
		size_t tokenIdx = args[1].find('/');
		if (tokenIdx == args[1].npos) {
			std::cout << "Error: Invalid arguments for command: " << cmd << std::endl; return;
			return;
		}
		rotation.x = std::stof(args[1].substr(startIdx, tokenIdx - startIdx));
		startIdx = tokenIdx + 1;
		tokenIdx = args[1].find('/', startIdx);
		if (tokenIdx == args[1].npos) {
			std::cout << "Error: Invalid arguments for command: " << cmd << std::endl; return;
			return;
		}
		rotation.y = std::stof(args[1].substr(startIdx, tokenIdx - startIdx));
		startIdx = tokenIdx + 1;
		tokenIdx = args[1].find('/', startIdx);
		if (tokenIdx == args[1].npos) {
			std::cout << "Error: Invalid arguments for command: " << cmd << std::endl; return;
			return;
		}
		rotation.z = std::stof(args[1].substr(startIdx, tokenIdx - startIdx));
		m_objTemplates[m_activeObjectTemplate].m_subObjectDummys.end()->rotation = rotation;
	}
	else {
		//unknown command
		std::cout << "Unknown command: " << cmd << std::endl;
	}
}

void BF2ConParser::loadFiles()
{
	if (m_filesToLoad.size() == 0) std::cout << "Info: No files to load!" << std::endl;
	for (std::string file : m_filesToLoad)
	{
		//TODO: LOAD FILES
		std::cout << "Info: Loading " << file;
	}
}

std::map<std::string, int> BF2ConParser::myMap = {
	{ "ObjectTemplate.create", 2 },

};

/*
std::map<std::string, int (*)(std::vector<std::string>)> BF2ConParser::m_conFunctionList = 
{
	{"ObjectTemplate.create", &(BF2ConParser::ObjectTemplate_create)}//,
	
};
*/

int BF2ConParser::ObjectTemplate_create(std::vector<std::string> argv)
{
	CHECK_ARGV(2);
	BF2ObjectTemplate newTemp(argv[0], argv[1]);
	auto it_objTemp = m_objTemplates.find(argv[1]);
	if (m_objTemplates.end()!=it_objTemp) m_os << "Warning: Template " << argv[1] << "already exist!" << std::endl;
	m_objTemplates[argv[1]]=newTemp;
	m_activeObjectTemplate = argv[1]; 
}
