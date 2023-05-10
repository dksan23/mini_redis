#include "hash_table.h"

static void h_init(htab *htab, size_t n)
{
    assert(n > 0 && ((n-1) & n == 0));
    htab->tab = (hnode**) calloc(sizeof(hnode *), n);
    htab->size = 0;
    htab->mask = n-1;
}

static void h_insert(htab* htab, hnode* node)
{
    if(!htab->tab)
    {
        return;
    }
    size_t pos = node->hcode & htab->mask;
    hnode* next = htab->tab[pos];
    node->next = next;
    htab->tab[pos] = node;
    htab->size++;
}
// hashtable look up subroutine.
// Pay attention to the return value. It returns the address of
// the parent pointer that owns the target node,
// which can be used to delete the target node.
static hnode **h_lookup(htab *htab, hnode* key, bool (*cmp) (hnode* , hnode* ))
{
    if(!htab->tab)
    {
        return nullptr;
    }
    size_t pos = htab->mask & key->hcode;
    hnode ** from = &htab->tab[pos];
    while(*from)
    {
        if(cmp(*from, key))
        {
            return from;
        }
    }

    return nullptr;
}

static hnode* h_detach(htab *htab, hnode **from)
{
    if(!htab->tab)
    {
        return nullptr;
    }
    hnode *node = *from;
    *from = (*from)->next;
    htab->size--;
    return node;
}

static void hmap_resize(HMap* hmap)
{
    if(hmap->ht1.tab == nullptr)
    {
        return;
    }

    uint16_t nwork = 0;
    while(nwork < RESIZING_BATCH && hmap->ht1.size > 0)
    {
        hnode** from = &hmap->ht1.tab[hmap->resizing_pos];
        if(!*from)
        {
            hmap->resizing_pos++;
            continue;
        }

        h_insert(&hmap->ht2, h_detach(&hmap->ht1, from));
    }

    if(hmap->ht1.size == 0)
    {
        free(hmap->ht1.tab);
        hmap->ht1 = htab{};
    }
}

hnode* hm_lookup(HMap* hmap, hnode* node, bool (*cmp) (hnode* , hnode* ))
{
    if(!hmap->ht1.tab && !hmap->ht2.tab)
    {
        return nullptr;
    }

    hnode **from = h_lookup(&hmap->ht1, node, cmp);
    if(!from)
    {
        from = h_lookup(&hmap->ht2, node, cmp);
    }

    return !from ? nullptr : *from;
}

static void hm_insert(HMap *hmap, hnode* node)
{
    if(hmap->ht1.tab == nullptr)
    {
        h_init(&hmap->ht1, 4);
    }

    h_insert(&hmap->ht1, node);

    if(!hmap->ht2.tab)
    {
        size_t load_factor = hmap->ht1.size / (hmap->ht1.mask + 1);
        if(load_factor >= MAX_LOAD_FACTOR)
        {
            hmap_start_resize(hmap);
        }
    }

    hmap_resize(hmap);
}

static void hmap_start_resize(HMap* hmap)
{
    assert(hmap->ht2.tab == nullptr);
    hmap->ht2 = hmap->ht1;
    h_init(&hmap->ht2, (hmap->ht1.mask + 1) * 2);
    hmap->resizing_pos = 0;
}

static hnode* hm_pop(HMap* hmap, hnode* node, bool (*cmp) (hnode* , hnode* ))
{
    hnode** from = h_lookup(&hmap->ht1, node, cmp);
    if(from)
    {
        return h_detach(&hmap->ht1, from);
    }

    from = h_lookup(&hmap->ht2, node, cmp);
    if(from)
    {
        return h_detach(&hmap->ht2, from);
    }

    return from ? *from : nullptr;
}