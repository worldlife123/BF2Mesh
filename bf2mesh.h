//original code by ByteHazard
//https://github.com/ByteHazard/BF2Mesh

#ifndef _BF2MESH_H_
#define _BF2MESH_H_

#include <stdio.h>
#include <string>
//#include "bf2Object.h"

enum BF2MeshType{
	BF2MESHTYPE_BUNDLEDMESH,
	BF2MESHTYPE_STATICMESH,
	BF2MESHTYPE_SKINNEDMESH,
	BF2MESHTYPE_BUNDLEDMESH_BFP4F,
};

enum VarType {
	TYPE_FLOAT1 = 0,
	TYPE_FLOAT2 = 1,
	TYPE_FLOAT3 = 2,
	TYPE_D3DCOLOR=4,
};

enum VertexUsage {
	USAGE_POSITION = 0,
	USAGE_BLENDWEIGHT = 1,
	USAGE_BLENDINDICES = 2,
	USAGE_NORMAL = 3,
	USAGE_UV1 = 5,
	USAGE_TANGENT = 6,
	USAGE_UV2 = 261,
	USAGE_UV3 = 517,
	USAGE_UV4 = 773,
	USAGE_UV5 = 1029,
};

// four dimensional floating point vector(quat)
struct float4 // 16 bytes
{
	float x;
	float y;
	float z;
	float w;
};

// three dimensional floating point vector
struct float3 // 12 bytes
{
	float x;
	float y;
	float z;
};

// two dimensional floating point vector(for uv)
struct float2 // 8 bytes
{
	float x;
	float y;
};

struct color4 //4 bytes
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};


// bounding box
struct aabb // 24 bytes
{
	float3 min;
	float3 max;
};


// 4x4 transformation matrix
struct matrix4 // 64 bytes
{
	float m[4][4];
};


// bf2 mesh file header
struct bf2head             // 20 bytes
{
	unsigned int u1;          // 0
	unsigned int version;     // 10 for most bundledmesh, 6 for some bundledmesh, 11 for staticmesh
	unsigned int u3;          // 0
	unsigned int u4;          // 0
	unsigned int u5;          // 0
};


// vertex attribute table entry
struct bf2attrib           // 8 bytes
{
	unsigned short flag;      // some sort of boolean flag (if true the below field are to be ignored?)
	unsigned short offset;    // offset from vertex data start
	unsigned short vartype;   // attribute type (vec2, vec3 etc)
	unsigned short usage;     // usage ID (vertex, texcoord etc)

	// Note: "usage" field correspond to the definition in DX SDK "Include\d3d9types.h"
	//       Looks like DICE extended these for additional UV channels, these constants
	//       are much larger to avoid conflict with core DX enums.
	/*
	   D3DDECLUSAGE_POSITION = 0,
	   D3DDECLUSAGE_BLENDWEIGHT,   // 1
	   D3DDECLUSAGE_BLENDINDICES,  // 2
	   D3DDECLUSAGE_NORMAL,        // 3
	   D3DDECLUSAGE_PSIZE,         // 4
	   D3DDECLUSAGE_TEXCOORD,      // 5
	   D3DDECLUSAGE_TANGENT,       // 6
	   D3DDECLUSAGE_BINORMAL,      // 7
	   D3DDECLUSAGE_TESSFACTOR,    // 8
	   D3DDECLUSAGE_POSITIONT,     // 9
	   D3DDECLUSAGE_COLOR,         // 10
	   D3DDECLUSAGE_FOG,           // 11
	   D3DDECLUSAGE_DEPTH,         // 12
	   D3DDECLUSAGE_SAMPLE,        // 13
	*/
};

//newly added for better data representaion
struct bf2vertices //size depends on bf2atrrib
{
	float3* position;
	float* blendWeight;
	color4* blendIndices;
	float3* normal;
	float3* tangent;
	float2* uv1;
	float2* uv2;
	float2* uv3;
	float2* uv4;
	float2* uv5;

	// constructor/destrutor
	bf2vertices() {
		position = NULL;
		blendWeight = NULL;
		blendIndices = NULL;
		normal = NULL;
		tangent = NULL;
		uv1 = NULL;
		uv2 = NULL;
		uv3 = NULL;
		uv4 = NULL;
		uv5 = NULL;
	};
	~bf2vertices() {
		if (position) delete[] position;
		if (blendWeight) delete[] blendWeight;
		if (blendIndices) delete[] blendIndices;
		if (normal) delete[] normal;
		if (tangent) delete[] tangent;
		if (uv1) delete[] uv1;
		if (uv2) delete[] uv2;
		if (uv3) delete[] uv3;
		if (uv4) delete[] uv4;
		if (uv5) delete[] uv5;
	};

	//functions
	bool Read(FILE* fp, bf2attrib attribList[], int attribNum, int vertnum);
};

// bone structure
struct bf2bone       // 68 bytes
{
	unsigned int id;    //  4 bytes
	matrix4 transform;  // 64 bytes
};


// rig structure
struct bf2rig
{
	int bonenum;
	bf2bone *bone;

	// constructor/destrutor
	bf2rig() {
		bone = NULL;
	};
	~bf2rig() {
		if (bone) delete[] bone;
	};

	// functions
	bool Read(FILE *fp, int version);
};


// material (aka drawcall)
struct bf2mat
{
	BF2MeshType type;

	unsigned int alphamode;             // 0=opaque, 1=blend, 2=alphatest
	std::string fxfile;                 // shader filename string
	std::string technique;              // technique name

	// texture map filenames
	int mapnum;                         // number of texture map filenames
	std::string *map;                   // map filename array

	// geometry info
	unsigned int vstart;                // vertex start offset
	unsigned int istart;                // index start offset
	unsigned int inum;                  // number of indices
	unsigned int vnum;                  // number of vertices

	// misc
	unsigned int u4;                    // always 1?
	unsigned short u5;                  // always 0x34E9?
	unsigned short u6;                  // most often 18/19
	aabb bounds;                        // per-material bounding box (StaticMesh only)

	// constructor/destructor
	bf2mat() {
		map = NULL;
		type = BF2MESHTYPE_BUNDLEDMESH;
	};
	bf2mat(BF2MeshType type) {
		map = NULL;
		this->type=type;
	};
	~bf2mat() {
		if (map) delete[] map;
	};

	// functions
	bool Read(FILE *fp, int version);
};


// lod, holds mainly a collection of materials
struct bf2lod
{
	BF2MeshType type;

	// bounding box
	float3 min;
	float3 max;
	float3 pivot; // not sure this is really a pivot (only on version<=6)

	// skinning matrices (SkinnedMesh only)
	int rignum;              // this usually corresponds to meshnum (but what was meshnum again??)
	bf2rig *rig;             // array of rigs

	// nodes (staticmesh and bundledmesh only)
	int nodenum;
	matrix4 *node;

	// material/drawcalls
	int matnum;              // number of materials
	bf2mat *mat;             // material array

	// constructor/destructor
	bf2lod() {
		rig = NULL;
		node = NULL;
		mat = NULL;
		type = BF2MESHTYPE_BUNDLEDMESH;
	};
	bf2lod(BF2MeshType type) {
		rig = NULL;
		node = NULL;
		mat = NULL;
		this->type = type;
	};
	~bf2lod() {
		if (rig) delete[] rig;
		if (node) delete[] node;
		if (mat) delete[] mat;
	};

	// functions
	bool ReadNodeData(FILE *fp, int version);
	bool ReadMatData(FILE *fp, int version);
};


// geom, holds a collection of LODs
struct bf2geom
{
	int lodnum;              // number of LODs
	bf2lod *lod;             // array of LODs
	BF2MeshType type;

	// constructor/destructor
	bf2geom() {
		lod = NULL;
		type = BF2MESHTYPE_BUNDLEDMESH;
	};
	bf2geom(BF2MeshType type) {
		lod = NULL;
		this->type = type;
	};
	~bf2geom() {
		if (lod) delete[] lod;
	}

	// functions
	bool Read(FILE *fp, int version);
};


// BF2 mesh file structure
struct bf2mesh
{
	// header
	bf2head head;

	// unknown
	unsigned char u1; // always 0?

	// geoms
	int geomnum;                      // numer of geoms
	bf2geom *geom;                    // geom array

	// vertex attribute table
	int vertattribnum;                // number of vertex attribute table entries
	bf2attrib *vertattrib;            // array of vertex attribute table entries

	// vertices
	int vertformat;                   // always 4?  (e.g. GL_FLOAT)
	int vertstride;                   // vertex stride
	int vertnum;                      // number of vertices in buffer
	bf2vertices *vert;                      // vertex array

	// indices
	int indexnum;                     // number of indices
	unsigned short *index;            // index array

	// unknown
	int u2;                           // always 8?

	// constructor/destructor
	bf2mesh() {
		geom = NULL;
		vertattrib = NULL;
		vert = NULL;
		index = NULL;
	};
	~bf2mesh() {
		if (geom) delete[] geom;
		if (vertattrib) delete[] vertattrib;
		if (vert) delete vert;
		if (index) delete[] index;
	};

	// functions
	int Load(const char *filename, const char *ext);

	// internal/hacks
	//bool isSkinnedMesh;
	//bool isBundledMesh;
	BF2MeshType type;
	bool isBFP4F;
};


struct bf2GeomCol {
	int coltype;            // exist in version>=9

	//face
	int faceNum;
	unsigned short* idxList;// face list of indices of vertices of length 4*facenum (v1,v2,v3,matIdx)
	//unsigned short* matList;// material indices of faces of length facenum
	unsigned short maxMatIdx;//helper

	//vert
	int vertNum;
	float3* vertList;       // vert list of length vertnum

	// something unknown
	unsigned short* uList;  // unknown list of length vertnum
	aabb uBB1;
	unsigned char ub;       // unknown 1 byte
	aabb uBB2;
	int ynum;
	unsigned int* yList;    // unknown list of length 4*ynum
	int znum;
	unsigned short* zList;  // unknown list of length znum
	int anum;               // anum and aList only exist in version>=10
	unsigned int* aList;  // unknown list of length anum

	// constructor/destructor
	bf2GeomCol() {
		idxList = NULL;
		//matList = NULL;
		maxMatIdx = 0;
		vertList = NULL;
		uList = NULL;
		yList = NULL;
		zList = NULL;
		aList = NULL;
	};
	~bf2GeomCol() {
		if (idxList) delete[] idxList;
		//if (matList) delete[] matList;
		if (vertList) delete[] vertList;
		if (uList) delete[] uList;
		if (yList) delete[] yList;
		if (zList) delete[] zList;
		if (aList) delete[] aList;
	}

	// functions
	bool Read(FILE *fp, int version, float impscale = 10.0);
};

struct bf2GeomSub {
	int geomColNum;
	bf2GeomCol* geomCols;

	// constructor/destructor
	bf2GeomSub() {
		geomCols = NULL;
	};
	~bf2GeomSub() {
		if (geomCols) delete[] geomCols;
	}

	// functions
	bool Read(FILE *fp, int version, float impscale = 10.0);
};

struct bf2colGeom {
	int geomSubNum;
	bf2GeomSub* geomSubs;

	// constructor/destructor
	bf2colGeom() {
		geomSubs = NULL;
	};
	~bf2colGeom() {
		if (geomSubs) delete[] geomSubs;
	}

	// functions
	bool Read(FILE *fp, int version, float impscale = 10.0);
};

struct bf2col {
	//flags
	int head;
	int version;

	//geom
	int geomNum;
	bf2colGeom* geoms;

	// helper
	unsigned short maxMatIdx;

	// constructor/destructor
	bf2col() {
		geoms = NULL;
		maxMatIdx = 0;
	};
	~bf2col() {
		if (geoms) delete[] geoms;
	}

	// functions
	int Load(const char *filename, float impscale = 10.0);
};

struct bf2skeBone {
	std::string name;
	float4 rotation;
	float3 position;
	short motherIdx; //-1 stands for root/camera bone

	// helper functions
	std::string readName(FILE* fp);

	// functions
	bool Read(FILE *fp, int version, float scale = 10.0);
};

struct bf2ske {
	//flags
	int version;

	//bones
	int boneNum;
	bf2skeBone* bones;

	// constructor/destructor
	bf2ske() {
		bones = NULL;
	};
	~bf2ske() {
		if (bones) delete[] bones;
	}

	// functions
	int Load(const char *filename, float scale = 10.0);
};

struct bf2frame {
	float4 rot;
	float3 pos;            
};

struct bf2animbone {
	int dataLenTotal;
	int frameNum;
	int8_t filePrecision;
	bf2frame* frames;

	// constructor/destructor
	bf2animbone() {
		frames = NULL;
		frameNum = 0;
		filePrecision = 15;
	};
	~bf2animbone() {
		if (frames) delete[] frames;
	}

	// functions
	bool Read(FILE *fp, int version, float scale = 10.0);
};

struct bf2baf {
	//flags
	int version;//always 4

	//bones
	short boneNum;
	//bf2skeBone* bones;
	short* boneIds;

	//frames
	int frameNum;
	bf2animbone* bones;
	int8_t filePrecision;//1 byte

	// constructor/destructor
	bf2baf() {
		boneIds = NULL;
		bones = NULL;
	};
	~bf2baf() {
		if (boneIds) delete[] boneIds;
		if (bones) delete[] bones;
	}

	// functions
	int Load(const char *filename, float scale = 10.0);
};

// importer helper functions
std::string BF2ReadString(FILE *fp);
float Convert16bitToFloat(int16_t tmpInt16, int8_t precision);
float Read16bitFloat(FILE *fp, int8_t precision);
float3 Read16bitFloat3(FILE *fp, int8_t precision);
float4 Read16bitFloat4(FILE *fp, int8_t precision);

#endif

