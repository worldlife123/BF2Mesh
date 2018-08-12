
//#include <process.h>
//#include <vld.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "bf2mesh.h"
#include "bf2conParser.h"


// main
int main(int argnum, char *arg[])
{
	 printf("---------------------------------------------\n");
	 printf("MeshMan v0.1\n");
	 printf("By Remdul (www.bytehazard.com)\n");
	 printf("For Forgotten Hope Mod (www.fhmod.org)\n");
	 printf("---------------------------------------------\n");
 
	 if (argnum != 2) {
	  printf("Error: Invalid argument.\n");
	  printf("Usage: meshman <filename>\n");
	  return 2;
	 }
 
	 const char *fname = arg[1];
 
	 printf("Input: %s\n", fname);
 
	 // determine file extension
	 const char *ext = strrchr(fname,'.');
	 if (ext == NULL) {
	  printf("Input file name extension missing.\n");
	  return 3;
	 }
	 ext++;

	 int ret=-1;
	 if (StrMatch(ext, "con") || StrMatch(ext, "inc")) {
		 BF2ConParser conParser = BF2ConParser(std::string(fname));
		 conParser.loadFiles();
		 ret = 0;
	 }
	 else if (StrMatch(ext, "CollisionMesh")) {
		 printf("Using CollisionMesh profile...\n");
		 bf2col colmesh;
		 ret = colmesh.Load(fname);
	 }
	 else if (StrMatch(ext, "ske")) {
		 printf("Using Skeleton profile...\n");
		 bf2ske skeleton;
		 ret = skeleton.Load(fname);
	 }
	 else if (StrMatch(ext, "baf")) {
		 printf("Using Animation profile...\n");
		 bf2baf anim;
		 ret = anim.Load(fname);
	 }
	 else
	 {
		 printf("Using Standard profile...\n");
		 bf2mesh mesh;
		 ret = mesh.Load(fname, ext);
	 }

	 if (ret != 0) {
		 printf("Error occurred while parsing file.\n");
		 return ret;
	 }
 
	 // success
	 printf("---------------------------------------------\n");
	 return 0;
}

