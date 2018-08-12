
#define _CRT_SECURE_NO_DEPRECATE
#include <assert.h>
#include <vector>
#include <set>
#include "BF2mesh.h"

        // loads mesh from file
        int bf2mesh::Load(const char *filename, const char *ext)
        {
            assert(filename != NULL && ext != NULL);
            // decide type
            if (strcmp(ext, "bundledmesh") == 0) type = BF2MESHTYPE_BUNDLEDMESH;
            else if (strcmp(ext, "skinnedmesh") == 0) type = BF2MESHTYPE_SKINNEDMESH;
            else if (strcmp(ext, "staticmesh") == 0) type = BF2MESHTYPE_STATICMESH;
            else return 1;

            // open file
            FILE *fp = fopen(filename, "rb");
            if (!fp) {
                printf("File \"%s\" not found.\n", filename);
                return 1;
            }

            // header
            fread(&head, sizeof(bf2head), 1, fp);
            printf("head start at %i\n", ftell(fp));
            printf(" u1: %i\n", head.u1);
            printf(" version: %i\n", head.version);
            printf(" u3: %i\n", head.u5);
            printf(" u4: %i\n", head.u5);
            printf(" u5: %i\n", head.u5);
            printf("head end at %i\n", ftell(fp));
            printf("\n");

            // unknown (1 byte)
            // stupid little byte that misaligns the entire file!
            fread(&u1, 1, 1, fp);
            printf("u1: %i\n", u1);
            printf("\n");
            // for BFP4F, the value is "1", so perhaps this is a version number as well
            if (u1 == 1) {
                isBFP4F = true;
                if (type == BF2MESHTYPE_BUNDLEDMESH) type = BF2MESHTYPE_BUNDLEDMESH_BFP4F;
            }
            // --- geom table ---------------------------------------------------------------------------

            printf("geom table start at %i\n", ftell(fp));

            // geomnum (4 bytes)
            fread(&geomnum, 4, 1, fp);
            printf(" geomnum: %i\n", geomnum);

            // geom table (4 bytes * groupnum)
            geom = new bf2geom[geomnum];
            for (int i = 0; i < geomnum; i++)
            {
                geom[i].type = type;
                geom[i].Read(fp, head.version);
            }

            printf("geom table end at %i\n", ftell(fp));
            printf("\n");

            // --- vertex attribute table -------------------------------------------------------------------------------

            printf("attrib block at %i\n", ftell(fp));

            // vertattribnum (4 bytes)
            fread(&vertattribnum, sizeof(vertattribnum), 1, fp);
            printf(" vertattribnum: %i\n", vertattribnum);

            // vertex attributes
            vertattrib = new bf2attrib[vertattribnum];
            fread(vertattrib, sizeof(bf2attrib)*vertattribnum, 1, fp);
            for (int i = 0; i < vertattribnum; i++)
            {
                printf(" attrib[%i]: %i %i %i %i\n", i, vertattrib[i].flag,
                    vertattrib[i].offset,
                    vertattrib[i].vartype,
                    vertattrib[i].usage);
            }

            printf("attrib block end at %i\n", ftell(fp));
            printf("\n");

            // --- vertices -----------------------------------------------------------------------------

            printf("vertex block start at %i\n", ftell(fp));

            fread(&vertformat, 4, 1, fp);
            fread(&vertstride, 4, 1, fp);
            fread(&vertnum, 4, 1, fp);
            printf(" vertformat: %i\n", vertformat);
            printf(" vertstride: %i\n", vertstride);
            printf(" vertnum: %i\n", vertnum);

            assert(vertnum > 0 && vertformat == 4);

            //vert = new int[vertnum * (vertstride / vertformat)];
            //fread(vert, vertnum*vertstride, 1, fp);
            /*for (int i = 0; i < vertnum; i++)
            {
                printf("vertex %i's 27: %i\n", i, vert[i*(vertstride / vertformat) + (24 / vertformat)] >> 24);
                printf("vertex %i's 26: %i\n", i, (vert[i*(vertstride / vertformat) + (24 / vertformat)] & 0x00FF0000) >> 16);
                printf("vertex %i's 25: %i\n", i, (vert[i*(vertstride / vertformat) + (24 / vertformat)] & 0x0000FF00) >> 8);
                printf("vertex %i's 24: %i\n", i, vert[i*(vertstride / vertformat) + (24 / vertformat)] & 0x000000FF);
            }*/
            vert = new bf2vertices;
            if (!(vert->Read(fp, vertattrib, vertattribnum, vertnum))) return 1;

            printf("vertex block end at %i\n", ftell(fp));
            printf("\n");

            // --- indices ------------------------------------------------------------------------------

            printf("index block start at %i\n", ftell(fp));

            fread(&indexnum, 4, 1, fp);
            printf(" indexnum: %i\n", indexnum);
            index = new unsigned short[indexnum];
            fread(index, 2 * indexnum, 1, fp);

            printf("index block end at %i\n", ftell(fp));
            printf("\n");

            // --- rigs -------------------------------------------------------------------------------

            // unknown (4 bytes)
            if (type != BF2MESHTYPE_SKINNEDMESH) {
                fread(&u2, 4, 1, fp); // always 8?
                printf("u2: %i\n", u2);
                printf("\n");
            }

            // rigs/nodes
            printf("nodes chunk start at %i\n", ftell(fp));
            for (int i = 0; i < geomnum; i++)
            {
                if (i > 0) printf("\n");
                printf(" geom %i start\n", i);
                for (int j = 0; j < geom[i].lodnum; j++)
                {
                    printf("  lod %i start\n", j);
                    geom[i].lod[j].ReadNodeData(fp, head.version);
                    printf("  lod %i end\n", j);
                }
                printf(" geom %i end\n", i);
            }
            printf("nodes chunk end at %i\n", ftell(fp));
            printf("\n");

            // --- geoms ------------------------------------------------------------------------------

            for (int i = 0; i < geomnum; i++)
            {
                printf("geom %i start at %i\n", i, ftell(fp));
                //geom[i].ReadMatData( fp, head.version );

                for (int j = 0; j < geom[i].lodnum; j++)
                {
                    printf(" lod %i start\n", j);
                    geom[i].lod[j].ReadMatData(fp, head.version);
                    printf(" lod %i end\n", j);
                }

                printf("geom %i block end at %i\n", i, ftell(fp));
                printf("\n");
            }

            // --- end of file -------------------------------------------------------------------------

            printf("done reading %i\n", ftell(fp));
            fseek(fp, 0, SEEK_END);
            printf("file size is %i\n", ftell(fp));
            printf("\n");

            // close file
            fclose(fp);
            fp = NULL;

            // success!
            return 0;
        }

        // reads rig
        bool bf2rig::Read(FILE *fp, int version)
        {

            // bonenum (4 bytes)
            fread(&bonenum, 4, 1, fp);
            printf("   bonenum: %i\n", bonenum);
            assert(bonenum >= 0);
            assert(bonenum < 99);

            // bones (68 bytes * bonenum)
            if (bonenum > 0) {
                bone = new bf2bone[bonenum];
                fread(bone, sizeof(bf2bone)*bonenum, 1, fp);

                for (int i = 0; i < bonenum; i++)
                {
                    printf("   boneid[%i]: %i\n", i, bone[i].id);
                }
            }

            // success
            return true;
        }


        // reads lod node table
        bool bf2lod::ReadNodeData(FILE *fp, int version)
        {

            // bounds (24 bytes)
            fread(&min, 12, 1, fp);
            fread(&max, 12, 1, fp);

            // unknown (12 bytes)
            if (version <= 6) { // version 4 and 6
                fread(&pivot, 12, 1, fp);
            }

            // skinnedmesh has different rigs
            if (type==BF2MESHTYPE_SKINNEDMESH) {

                // rignum (4 bytes)
                fread(&rignum, 4, 1, fp);
                printf("  rignum: %i\n", rignum);

                // read rigs
                if (rignum > 0) {
                    rig = new bf2rig[rignum];
                    for (int i = 0; i < rignum; i++)
                    {
                        printf("  rig block %i start at %i\n", i, ftell(fp));

                        rig[i].Read(fp, version);

                        printf("  rig block %i end at %i \n", i, ftell(fp));
                    }
                }

            }
            else {

                // nodenum (4 bytes)
                fread(&nodenum, 4, 1, fp);
                printf("   nodenum: %i\n", nodenum);

                // node matrices (64 bytes * nodenum) (BFP4F variant)
                if (type != BF2MESHTYPE_BUNDLEDMESH) {
                    printf("   node data\n");
                    if (nodenum > 0) {
                        node = new matrix4[nodenum];
                        //fread(node, sizeof(matrix4)*nodenum, 1, fp);
                        for (int i = 0; i < nodenum; i++)
                        {
                            fread(node+i, sizeof(matrix4), 1, fp);
                            if (type == BF2MESHTYPE_BUNDLEDMESH_BFP4F)
                            {
                                printf("   node string: %s\n", BF2ReadString(fp).c_str());
                            }
                        }      
                    }
                }

            }

            // success
            return true;
        }


        // reads lod material chunk
        bool bf2mat::Read(FILE *fp, int version)
        {

            // alpha flag (4 bytes)
            if (type != BF2MESHTYPE_SKINNEDMESH) {
                fread(&alphamode, 4, 1, fp);
                printf("   alphamode: %i\n", alphamode);
            }

            // fx filename
            fxfile = BF2ReadString(fp);
            printf("   fxfile: %s\n", fxfile.c_str());

            // material name
            technique = BF2ReadString(fp);
            printf("   matname: %s\n", technique.c_str());

            // mapnum (4 bytes)
            fread(&mapnum, 4, 1, fp);
            printf("   mapnum: %i\n", mapnum);
            assert(mapnum >= 0);
            assert(mapnum < 99);

            // mapnames
            if (mapnum > 0) {
                map = new std::string[mapnum];
                for (int i = 0; i < mapnum; i++)
                {
                    map[i] = BF2ReadString(fp);
                    printf("    map %i: %s\n", i, map[i].c_str());
                }
            }

            // geometry info
            fread(&vstart, 4, 1, fp);
            fread(&istart, 4, 1, fp);
            fread(&inum, 4, 1, fp);
            fread(&vnum, 4, 1, fp);
            printf("   vstart: %i\n", vstart);
            printf("   istart: %i\n", istart);
            printf("   inum: %i\n", inum);
            printf("   vnum: %i\n", vnum);

            // unknown
            fread(&u4, 4, 1, fp);
            fread(&u5, 2, 1, fp);
            fread(&u6, 2, 1, fp);

            // bounds
            if (type != BF2MESHTYPE_SKINNEDMESH) {
                if (version == 11) {
                    fread(&bounds, sizeof(aabb), 1, fp);
                }
            }

            // success
            return true;
        }


        // reads geom lod chunk
        bool bf2lod::ReadMatData(FILE *fp, int version)
        {
            // matnum (4 bytes)
            fread(&matnum, 4, 1, fp);
            printf("  matnum: %i\n", matnum);

            assert(matnum >= 0);
            assert(matnum < 99);

            // materials (? bytes)
            if (matnum > 0) {
                mat = new bf2mat[matnum];
                for (int i = 0; i < matnum; i++)
                {
                    mat[i].type = type;
                    printf("  mat %i start at %i\n", i, ftell(fp));
                    if (!mat[i].Read(fp, version)) return false;
                    printf("  mat %i end at %i\n", i, ftell(fp));
                }
            }

            // success
            return true;
        }


        // reads geom from file
        bool bf2geom::Read(FILE *fp, int version)
        {
            // lodnum (4 bytes)
            fread(&lodnum, 4, 1, fp);
            printf("  lodnum: %i\n", lodnum);

            assert(lodnum >= 0);
            assert(lodnum < 99);

            // allocate lods
            if (lodnum > 0) {
                lod = new bf2lod[lodnum];
                for (int i = 0; i < lodnum; i++)
                {
                    lod[i].type = type;
                }
            }

            // success
            return true;
        }

        bool bf2vertices::Read(FILE * fp, bf2attrib attribList[], int attribNum, int vertnum)
        {
            //read attribute list and initialize
            uintptr_t* offsetPtrs = new uintptr_t[attribNum-1];//pointer that used by fread to store data
            uintptr_t* increments = new uintptr_t[attribNum-1];//size of the pointer's data
            for (int i = 0; i < attribNum-1; i++) // the last attribute has non-zero flag
            {
                assert(attribList[i].flag==0); //check flag
                //TODO: check attribList[i].offset
                switch (attribList[i].usage)
                {
                case USAGE_POSITION:
                    assert(attribList[i].vartype == TYPE_FLOAT3);
                    position = new float3[vertnum];
                    offsetPtrs[i] = (uintptr_t)position;
                    increments[i] = sizeof(float3);
                    break;
                case USAGE_BLENDWEIGHT:
                    assert(attribList[i].vartype == TYPE_FLOAT1);
                    blendWeight = new float[vertnum];
                    offsetPtrs[i] = (uintptr_t)blendWeight;
                    increments[i] = sizeof(float);
                    break;
                case USAGE_BLENDINDICES:
                    assert(attribList[i].vartype == TYPE_D3DCOLOR);
                    blendIndices = new color4[vertnum];
                    offsetPtrs[i] = (uintptr_t)blendIndices;
                    increments[i] = sizeof(color4);
                    break;
                case USAGE_NORMAL:
                    assert(attribList[i].vartype == TYPE_FLOAT3);
                    normal = new float3[vertnum];
                    offsetPtrs[i] = (uintptr_t)normal;
                    increments[i] = sizeof(float3);
                    break;
                case USAGE_UV1:
                    assert(attribList[i].vartype == TYPE_FLOAT2);
                    uv1 = new float2[vertnum];
                    offsetPtrs[i] = (uintptr_t)uv1;
                    increments[i] = sizeof(float2);
                    break;
                case USAGE_TANGENT:
                    assert(attribList[i].vartype == TYPE_FLOAT3);
                    tangent = new float3[vertnum];
                    offsetPtrs[i] = (uintptr_t)tangent;
                    increments[i] = sizeof(float3);
                    break;
                case USAGE_UV2:
                    assert(attribList[i].vartype == TYPE_FLOAT2);
                    uv2 = new float2[vertnum]; 
                    offsetPtrs[i] = (uintptr_t)uv2;
                    increments[i] = sizeof(float2);
                    break;
                case USAGE_UV3:
                    assert(attribList[i].vartype == TYPE_FLOAT2);
                    uv3 = new float2[vertnum];
                    offsetPtrs[i] = (uintptr_t)uv3;
                    increments[i] = sizeof(float2);
                    break;
                case USAGE_UV4:
                    assert(attribList[i].vartype == TYPE_FLOAT2);
                    uv4 = new float2[vertnum];
                    offsetPtrs[i] = (uintptr_t)uv4;
                    increments[i] = sizeof(float2);
                    break;
                case USAGE_UV5:
                    assert(attribList[i].vartype == TYPE_FLOAT2);
                    uv5 = new float2[vertnum];
                    offsetPtrs[i] = (uintptr_t)uv5;
                    increments[i] = sizeof(float2);
                    break;
                default:
                    printf("Unknown vertex usage %d", attribList[i].usage);
                    return false;
                }
            }

            //read vertices
            for (int i = 0; i < vertnum; i++)
            {
                for (int j = 0; j < attribNum-1; j++)
                {
                    void* ptr = (void*)(offsetPtrs[j] + (increments[j]*i)); //the offset
                    fread(ptr, increments[j], 1, fp);
                }
            }

            return true;
        }

        int bf2col::Load(const char * filename, float impscale)
        {
            assert(filename != NULL);

            // open file
            FILE *fp = fopen(filename, "rb");
            if (!fp) {
                printf("File \"%s\" not found.\n", filename);
                return 1;
            }

            // --- head (8 bytes) ---------------------------------------------------------------------------
            printf("head start at %i\n", ftell(fp));
            fread(&head, 4, 1, fp);
            fread(&version, 4, 1, fp);
            printf(" head: %i\n", head);
            printf(" version: %i\n", version);
            printf("head end at %i\n", ftell(fp));

            // --- geoms ---------------------------------------------------------------------------

            printf("geom chunk start at %i\n", ftell(fp));

            // geomnum (4 bytes)
            fread(&geomNum, 4, 1, fp);
            printf(" geomNum: %i\n", geomNum);

            // geom chunk
            geoms = new bf2colGeom[geomNum];
            //find max material index
            for (int i = 0; i < geomNum; i++)
            {
                if(!geoms[i].Read(fp, version, impscale)) return 1;//may fail
                for (int j = 0; j < geoms[i].geomSubNum; j++)
                {
                    for (int k = 0; k < geoms[i].geomSubs[j].geomColNum; k++)
                    {
                        if (maxMatIdx < geoms[i].geomSubs[j].geomCols[k].maxMatIdx) maxMatIdx = geoms[i].geomSubs[j].geomCols[k].maxMatIdx;
                    }
                }
            }
            

            printf("geom chunk end at %i\n", ftell(fp));
            printf("\n");

            // --- end of file -------------------------------------------------------------------------

            printf("done reading %i\n", ftell(fp));
            fseek(fp, 0, SEEK_END);
            printf("file size is %i\n", ftell(fp));
            printf("\n");

            // close file
            fclose(fp);
            fp = NULL;

            // success!
            return 0;
        }

        bool bf2colGeom::Read(FILE * fp, int version, float impscale)
        {
            fread(&geomSubNum, 4, 1, fp);
            if (geomSubNum < 0) return false;
            printf(" geomSubNum: %i\n", geomSubNum);
            geomSubs = new bf2GeomSub[geomSubNum];
            for (int i = 0; i < geomSubNum; i++)
            {
                if (!geomSubs[i].Read(fp, version, impscale)) return false;
            }
            return true;
        }

        bool bf2GeomSub::Read(FILE * fp, int version, float impscale)
        {
            // --- cols ---------------------------------------------------------------------------

            printf("col chunk start at %i\n", ftell(fp));

            // geomColNum (4 bytes)
            fread(&geomColNum, 4, 1, fp);
            printf(" colNum: %i\n", geomColNum);

            if (geomColNum < 0) return false;

            // col chunk
            geomCols = new bf2GeomCol[geomColNum];
            for (int i = 0; i < geomColNum; i++)
            {
                if (!geomCols[i].Read(fp, version, impscale)) return false;
                //set coltype manually if version<9
                if (version < 9) geomCols->coltype = i;
            }

            printf("col chunk end at %i\n", ftell(fp));
            printf("\n");

            //success
            return true;
        }

        bool bf2GeomCol::Read(FILE * fp, int version, float impscale)
        {
            if (version >= 9)
            {
                fread(&coltype, 4, 1, fp);
            }
            // --- faces ------------------------------------------------------------------------------

            printf("face block start at %i\n", ftell(fp));

            fread(&faceNum, 4, 1, fp);
            printf(" faceNum: %i\n", faceNum);
            idxList = new unsigned short[4 * faceNum];
            fread(idxList, sizeof(unsigned short) * 4 * faceNum, 1, fp);
            for (int i = 3; i < 4 * faceNum; i += 4)
            {
                if (idxList[i] > maxMatIdx) maxMatIdx = idxList[i];
            }

            printf("face block end at %i\n", ftell(fp));
            printf("\n");

            // --- verts ------------------------------------------------------------------------------

            printf("vert block start at %i\n", ftell(fp));

            fread(&vertNum, 4, 1, fp);
            printf(" vertNum: %i\n", vertNum);
            vertList = new float3[vertNum];
            
            fread(vertList, sizeof(float3) * vertNum, 1, fp);
            for (int i = 0; i < vertNum; i++)
            {
                vertList[i].x *= impscale;
                vertList[i].y *= impscale;
                vertList[i].z *= impscale;
            }

            printf("vert block end at %i\n", ftell(fp));
            printf("\n");

            // --- unknowns ------------------------------------------------------------------------------
            uList = new unsigned short[vertNum];
            fread(uList, sizeof(unsigned short) * vertNum, 1, fp);
            fread(&uBB1, sizeof(aabb), 1, fp);
            fread(&ub, 1, 1, fp);
            fread(&uBB2, sizeof(aabb), 1, fp);
            fread(&ynum, 4, 1, fp);
            printf(" ynum: %i\n", ynum);
            yList = new unsigned int[4*ynum];
            fread(yList, sizeof(unsigned int) * 4 * ynum, 1, fp);
            fread(&znum, 4, 1, fp);
            printf(" znum: %i\n", znum);
            zList = new unsigned short[znum];
            fread(zList, sizeof(unsigned short) * znum, 1, fp);
            
            if (version >= 10)
            {
                fread(&anum, 4, 1, fp);
                printf(" anum: %i\n", anum);
                aList = new unsigned int[anum];
                fread(aList, sizeof(unsigned int) * anum, 1, fp);
            }

            return true;
        }

        bool bf2skeBone::Read(FILE * fp, int version, float scale)
        {
            name = readName(fp);
            fread(&motherIdx, 2, 1, fp);
            printf("bone: %s, motherIdx=%d\n", name.c_str(), motherIdx);
            fread(&rotation, sizeof(float4), 1, fp);
            fread(&position, sizeof(float3), 1, fp);
            if (name.substr(0, 4) == "mesh") //kit nodes
            {
                rotation = float4{ 0,0,0,1 };
                position = float3{ 0,0,0 };
            }
            printf("pos: %f,%f,%f\n", position.x, position.y, position.z);
            printf("rot: %f,%f,%f,%f\n", rotation.x, rotation.y, rotation.z, rotation.w);
            //success
            return true;
        }

        std::string bf2skeBone::readName(FILE * fp)
        {
            unsigned short num;
            fread(&num, 2, 1, fp);

            assert(num >= 0);
            assert(num < 99);

            if (num == 0) return "";

            char *str = new char[num];
            fread(str, num, 1, fp);

            std::string tmp(str, num);

            delete[] str;

            return tmp;
        }

        int bf2ske::Load(const char * filename, float scale)
        {
            assert(filename != NULL);

            // open file
            FILE *fp = fopen(filename, "rb");
            if (!fp) {
                printf("File \"%s\" not found.\n", filename);
                return 1;
            }

            // --- head (4 bytes) ---------------------------------------------------------------------------
            printf("head start at %i\n", ftell(fp));
            fread(&version, 4, 1, fp);
            printf(" version: %i\n", version);
            printf("head end at %i\n", ftell(fp));

            // --- bones ---------------------------------------------------------------------------
            printf("bones chunk start at %i\n", ftell(fp));

            // geomnum (4 bytes)
            fread(&boneNum, 4, 1, fp);
            //assert(boneNum <= 80);//in bf2, bone number cannot exceed 80(may exceed 80 in later games)
            printf(" boneNum: %i\n", boneNum);

            // geom chunk
            bones = new bf2skeBone[boneNum];
            //find max material index
            for (int i = 0; i < boneNum; i++)
            {
                if (!bones[i].Read(fp, version, scale)) return 1;//may fail
            }

            printf("bones chunk end at %i\n", ftell(fp));
            printf("\n");

            // --- end of file -------------------------------------------------------------------------

            printf("done reading %i\n", ftell(fp));
            fseek(fp, 0, SEEK_END);
            printf("file size is %i\n", ftell(fp));
            printf("\n");

            // close file
            fclose(fp);
            fp = NULL;

            // success!
            return 0;
        }

        int bf2baf::Load(const char * filename, float scale)
        {
            assert(filename != NULL);

            // open file
            FILE *fp = fopen(filename, "rb");
            if (!fp) {
                printf("File \"%s\" not found.\n", filename);
                return 1;
            }

            // --- head (4 bytes) ---------------------------------------------------------------------------
            printf("head start at %i\n", ftell(fp));
            fread(&version, 4, 1, fp);
            printf(" version: %i\n", version);
            printf("head end at %i\n", ftell(fp));

            // --- bones ---------------------------------------------------------------------------
            // boneNum (2 bytes)
            fread(&boneNum, 2, 1, fp);
            //assert(boneNum <= 80);//in bf2, bone number cannot exceed 80(may exceed 80 in later games)
            printf(" boneNum: %i\n", boneNum);

            boneIds = new short[boneNum];
            fread(boneIds, 2*boneNum, 1, fp);

            // frameNum (4 bytes)
            fread(&frameNum, 4, 1, fp);
            printf(" frameNum: %i\n", frameNum);

            // precision (1 byte)
            fread(&filePrecision, 1, 1, fp);
            printf(" filePrecision: %i\n", filePrecision);

            bones = new bf2animbone[boneNum];
            for (int i = 0; i < boneNum; i++)
            {
                printf("bone %i start at %i\n",i ,ftell(fp));
                bones[i].frameNum = frameNum;
                bones[i].filePrecision = filePrecision;
                printf("bones %i id:%i \n", i, boneIds[i]);
                if (!bones[i].Read(fp, version, scale)) return 1;//may fail
                printf("bone %i end at %i\n", i, ftell(fp));
            }

            // --- end of file -------------------------------------------------------------------------

            printf("done reading %i\n", ftell(fp));
            fseek(fp, 0, SEEK_END);
            printf("file size is %i\n", ftell(fp));
            printf("\n");

            // close file
            fclose(fp);
            fp = NULL;

            // success!
            return 0;
        }

        bool bf2animbone::Read(FILE * fp, int version, float scale)
        {
            fread(&dataLenTotal, 2, 1, fp);
            printf(" dataLenTotal: %i\n", dataLenTotal);

            if (frameNum <= 0) {
                printf(" invalid frameNum %i\n", frameNum);
                return false;
            }

            frames = new bf2frame[frameNum];

            for (int i = 0; i < 7; i++)
            {
                short dataLeft;
                fread(&dataLeft, 2, 1, fp);
                int curFrame = 0;
                while (dataLeft > 0)
                {
                    int8_t tmpByte;
                    fread(&tmpByte, 1, 1, fp);
                    bool rleCompression = (tmpByte & 0b10000000) >> 7;
                    int8_t numFrames = (tmpByte & 0b01111111);
                    printf(" numFrames: %i\n", numFrames);
                    int8_t nextHeader;
                    fread(&nextHeader, 1, 1, fp);
                    short tmpVal;
                    if (rleCompression) fread(&tmpVal, 2, 1, fp);

                    //read numFrames times
                    for (int j = 0; j < numFrames; j++)
                    {
                        if (!rleCompression) fread(&tmpVal, 2, 1, fp);
                        switch (i)
                        {
                        //0-3 is rotation part
                        case 0: frames[curFrame].rot.x = Convert16bitToFloat(tmpVal, 15); break;
                        case 1: frames[curFrame].rot.y = Convert16bitToFloat(tmpVal, 15); break;
                        case 2: frames[curFrame].rot.z = Convert16bitToFloat(tmpVal, 15); break;
                        case 3: frames[curFrame].rot.w = Convert16bitToFloat(tmpVal, 15); break;
                        //4-6 is position part
                        case 4: frames[curFrame].pos.x = Convert16bitToFloat(tmpVal, filePrecision); break;
                        case 5: frames[curFrame].pos.y = Convert16bitToFloat(tmpVal, filePrecision); break;
                        case 6: frames[curFrame].pos.z = Convert16bitToFloat(tmpVal, filePrecision); break;
                            
                        }
                        curFrame++;
                    }

                    //decrement
                    dataLeft -= nextHeader;
                }
            }

            return true;
        }

        // reads string from file
        std::string BF2ReadString(FILE *fp)
        {
            unsigned int num;
            fread(&num, 4, 1, fp);

            assert(num >= 0);
            assert(num < 999);

            if (num == 0) return "";

            char *str = new char[num];
            fread(str, num, 1, fp);

            std::string tmp(str, num);

            delete[] str;

            return tmp;
        }

        float Convert16bitToFloat(int16_t tmpInt16, int8_t precision)
        {
            float flt16_mult = 32767.f / (2 << (15 - precision));
            float tmpVal = (float)tmpInt16;
            if (tmpInt16 > 32767) tmpVal -= 65535;
            return tmpVal / flt16_mult;
        }

        float Read16bitFloat(FILE * fp, int8_t precision)
        {
            int16_t tmpInt;
            fread(&tmpInt, 2, 1, fp);
            return Convert16bitToFloat(tmpInt, precision);
        }

        float3 Read16bitFloat3(FILE * fp, int8_t precision)
        {
            float3 tmpVal;
            tmpVal.x = Read16bitFloat(fp, precision);
            tmpVal.y = Read16bitFloat(fp, precision);
            tmpVal.z = Read16bitFloat(fp, precision);
            return tmpVal;
        }

        float4 Read16bitFloat4(FILE * fp, int8_t precision)
        {
            float4 tmpVal;
            tmpVal.x = Read16bitFloat(fp, precision);
            tmpVal.y = Read16bitFloat(fp, precision);
            tmpVal.z = Read16bitFloat(fp, precision);
            tmpVal.w = Read16bitFloat(fp, precision);
            return tmpVal;
        }
