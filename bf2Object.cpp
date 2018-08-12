#include "bf2Object.h"

BF2ObjectTemplate::BF2ObjectTemplate()
	:m_type("unknown"), m_name(""), m_geometryName(""), m_geometryPart(-1), m_collisionName(""), m_collisionPart(-1), m_position({ 0,0,0 }), m_rotation({ 0,0,0 })
{
}

BF2ObjectTemplate::BF2ObjectTemplate(std::string type, std::string name)
	:m_type(type), m_name(name), m_geometryName(""), m_geometryPart(-1), m_collisionName(""), m_collisionPart(-1), m_position({0,0,0}), m_rotation({ 0,0,0 })
{
}

BF2ObjectTemplate::~BF2ObjectTemplate()
{
}
