#ifndef HLTYPES_H
#define HLTYPES_H

struct modelheader_s
{
    char id[4]; // Orriginal header is int, but this is easier
    int version;

    char name[64];
    int length;

    float eyeposition[3];
    float min[3];
    float max[3];

    float bbmin[3];
    float bbmax[3];

    int flags;

    int numbones;
    int boneindex;

    int numbonecontrollers;
    int bonecontrollerindex;

    int numhitboxes;
    int hitboxindex;

    int numseq;
    int seqindex;

    int numseqgroups;
    int seqgroupindex;

    int numtextures; // Number of textures
    int textureindex; // Index of texture location - 0 means no textures present
    int texturedataindex;

    // incomplete - cut off for space saving
};

struct wadheader_s
{
    char identification[4]; // Should be WAD2 or WAD3
    int numlumps; // Number of lumps
    int infotableofs; // Offset of lump data
};

struct wadlumpinfo_s
{
    int filepos;
    int disksize;
    int size;
    char type;
    char compression;
    char pad1, pad2;
    char name[16]; // texture name, null terminated
};

struct pakheader_s
{
    int pakid;
    int diroffset;
    size_t dirsize;
};

struct fileinfo_s
{
    char name[56];
    int fileoffset;
    size_t filelen;
};

struct texdata_s
{
    char name[16];
    unsigned width, height;
    unsigned offsets[4];
};

struct lumpinfo_s
{
    int fileofs;
    size_t filelen;
};

struct bsp_header
{
    int version;
    lumpinfo_s ent_header;
    lumpinfo_s dnt_care01; // don't care
    lumpinfo_s tex_header;
    // incomplete - cut off for space saving
};

#endif
