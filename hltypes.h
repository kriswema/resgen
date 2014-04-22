#ifndef HLTYPES_H
#define HLTYPES_H

#include <cstdint>

struct modelheader_s
{
    char id[4]; // Orriginal header is int, but this is easier
    uint32_t version;

    char name[64];
    uint32_t length;

    float eyeposition[3];
    float min[3];
    float max[3];

    float bbmin[3];
    float bbmax[3];

    uint32_t flags;

    uint32_t numbones;
    uint32_t boneindex;

    uint32_t numbonecontrollers;
    uint32_t bonecontrollerindex;

    uint32_t numhitboxes;
    uint32_t hitboxindex;

    uint32_t numseq;
    uint32_t seqindex;

    uint32_t numseqgroups;
    uint32_t seqgroupindex;

    uint32_t numtextures; // Number of textures
    uint32_t textureindex; // Index of texture location - 0 means no textures present
    uint32_t texturedataindex;

    // incomplete - cut off for space saving
};

struct wadheader_s
{
    char identification[4]; // Should be WAD2 or WAD3
    int32_t numlumps; // Number of lumps
    int32_t infotableofs; // Offset of lump data
};

struct wadlumpinfo_s
{
    uint32_t filepos;
    uint32_t disksize;
    uint32_t size;
    char type;
    char compression;
    char pad1, pad2;
    char name[16]; // texture name, null terminated
};

struct pakheader_s
{
    uint32_t pakid; // 'PACK'
    int32_t diroffset;
    uint32_t dirsize;
};

struct fileinfo_s
{
    char name[56];
    uint32_t fileoffset;
    uint32_t filelen;
};

struct texdata_s
{
    char name[16];
    uint32_t width, height;
    uint32_t offsets[4];
};

struct lumpinfo_s
{
    int32_t fileofs;
    uint32_t filelen;
};

struct bsp_header
{
    uint32_t version;
    lumpinfo_s ent_header;
    lumpinfo_s _ignore; // Don't care about this
    lumpinfo_s tex_header;
    // incomplete - cut off for space saving
};

#endif
