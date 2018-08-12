#ifndef _BF2OBJECT_H_
#define _BF2OBJECT_H_

#include <vector>
#include <map>
#include "bf2mesh.h"

//other atrributes that may not have any functions here
struct BF2ObjectTemplateAtrributes
{
	bool hasMobilePhysics;
	bool hasCollisionPhysics;
	int physicsType;

	//constructor&destructor
	BF2ObjectTemplateAtrributes() {

	};
};

struct BF2ObjectTemplateDummy
{
	std::string name;
	float3 position;
	float3 rotation;

	//constructor&destructor
	BF2ObjectTemplateDummy() 
		:name(""), position({ 0,0,0 }), rotation({ 0,0,0 })
	{
	};
	BF2ObjectTemplateDummy(std::string name) 
		:name(name), position({0,0,0}), rotation({ 0,0,0 })
	{

	};
};

class BF2ObjectTemplate
{
private:


protected:
	std::vector<BF2ObjectTemplate> m_subObjects;
	bf2mesh* m_geometry;
	bf2col* m_collision;

public:
	//std::vector<BF2ObjectTemplate> m_subObjects;
	std::vector<BF2ObjectTemplateDummy> m_subObjectDummys;
	std::string m_name;
	std::string m_type;
	std::string m_geometryName;
	int m_geometryPart;
	std::string m_collisionName;
	int m_collisionPart;
	float3 m_position;
	float3 m_rotation;
	BF2ObjectTemplateAtrributes m_attrib;

	//constructor&destructor
	BF2ObjectTemplate();
	BF2ObjectTemplate(std::string type, std::string name);
	~BF2ObjectTemplate();

	//methods

	
};

#endif