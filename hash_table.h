#include<stdint.h>
#include<stdlib.h>
#include<assert.h>
#include<string>

#define RESIZING_BATCH 128
#define MAX_LOAD_FACTOR 8

#define container_of(ptr, type, member) ({ \
const typeof( ((type *)0)->member ) *__mptr = (ptr); \
(type *)( (char *)__mptr - offsetof(type, member) );})


typedef struct hnode
{
    hnode* next = nullptr;
    uint64_t hcode = 0;
}Hnode;

typedef struct htab
{
    hnode** tab = nullptr;
    uint64_t mask = 0;
    uint64_t size = 0;
}Htab;

/* Resizing HashMap*/  //should a eviciton strategy based approach

struct HMap
{
    htab ht1;
    htab ht2;
    size_t resizing_pos = 0;
};

struct Entry {
    struct hnode node;
    std::string key;
    std::string val;
};

static struct {
    HMap db;
} g_data;