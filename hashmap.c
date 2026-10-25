#include <stdint.h>
#include <stdio.h>

#include "hashmap.h"


union yt_key hash_compute(char* key)
{
    /// http://www.cse.yorku.ca/~oz/hash.html
    /// On utilise deux hashes pour "nullifier"? les possibilités de collision
    /// djb2 a été rapporté par dan bernstein dans c.comp.lang
    /// sdbm est utilisé pour la base de données eponyme basée sur ndbm
    uint32_t djb2 = 5381;
    uint32_t sdbm = 0;
    for (int i = 0; key[i]; i++) {
        djb2 = ((djb2 << 5) + djb2) + key[i];
        sdbm = key[i] + (sdbm << 6) + (sdbm << 16) - sdbm;
    }
    /// on efface les deux premiers bits pour y mettre des meta data
    djb2 &= (1 << 30) - 1;
    
    return (union yt_key) {.str = {.djb2 = djb2, .sdbm = sdbm}};
}

/** Ajoute un objet dans la hash map, retourne -1 si pleine, retourne l'offset entre 0 et 256 sinon */
int hash_add(union yt_key key, void * thing, struct yt_hashmap* map)
{
    uint8_t primary = key.tot & 255;
    
    uint8_t begin = primary;
    while (map->bucket[primary].complete_key.tot != 0) {
        if(map->bucket[primary].complete_key.tot & (1ull << 63))
            /// si entrée supprimée, le bit de poids le + fort est à 1, donc on prend cette place
            break;
        primary ++; /// `uint_8` fait le modulo pour nous
        if(primary == begin) return -1; /// hashmap pleine
    }
    if (map->bucket[primary].complete_key.tot == 0) {
        map->bucket[primary].complete_key.tot = key.tot;
        map->bucket[primary].the_thing = thing;
    }
    return primary;
}

int hash_get(union yt_key key, void ** thing, struct yt_hashmap* map)
{
    uint8_t primary = key.tot & 255;
    
    uint8_t begin = primary;
    while(map->bucket[primary].complete_key.tot != key.tot) {
        
        primary++;
        if(map->bucket[primary].complete_key.tot == 0)
            return -1; /// non trouvé
        if(primary == begin)
            return -2; /// non trouvé apres avoir bouclé
        ///
    }
    *thing = map->bucket[primary].the_thing;
    return primary;
}

/// supprime un objet de la hash map
/// si l'objet a été malloc, c'est a vous de le free() en allant le `get_object()` d'abord
int hash_delete(union yt_key key, struct yt_hashmap*map)
{
    uint8_t primary = key.tot & 255;

    uint8_t begin = primary;
    while(map->bucket[primary].complete_key.tot != key.tot) {
        
        primary++;
        if(map->bucket[primary].complete_key.tot == 0)
            return -1; /// non trouvé
        if(primary == begin)
            return -2; /// non trouvé apres avoir bouclé
    }
    /// laisse un marqueur afin de pouvoir atteindre les objets decalés auparavent
    map->bucket[primary].complete_key.tot = (1ull << 63);
    
    return primary;
}


/// affiche les bits d'un nombre
int pbits(uint64_t num, int size)
{
    for(int i = size-1; i >=0 ; i--) {
        putchar('0' + (!!(num & (1llu << i))));
        if(i%8 == 0) putchar(' ');
    }
    //putchar('\n');
    
    return 0;
}


int testing()
{
    pbits(hash_compute("0").tot, 64); printf(" %llu \"%s\" ", hash_compute("0").tot, "0"); pbits('0', 8);puts("");
    pbits(hash_compute("").tot, 64); printf(" %llu \"%s\"\n", hash_compute("").tot, "");
    pbits(hash_compute("10").tot, 64); printf(" %llu \"%s\"\n", hash_compute("10").tot, "10");
    pbits(hash_compute("01").tot, 64); printf(" %llu \"%s\"\n", hash_compute("01").tot, "01");
    pbits(hash_compute("abc").tot, 64); printf(" %llu \"%s\"\n", hash_compute("abc").tot, "abc");
    pbits(hash_compute("bac").tot, 64); printf(" %llu \"%s\"\n", hash_compute("bac").tot, "bac");
    pbits(hash_compute("vertex_applat").tot, 64); printf(" %llu \"%s\"\n", hash_compute("vertex_applat").tot, "vertex_applat");
    pbits(hash_compute("fragment_applat").tot, 64); printf(" %llu \"%s\"\n", hash_compute("fragment_applat").tot, "fragment_applat");
    
    struct yt_hashmap map = {0};
    pbits(hash_add(hash_compute("0"), NULL, &map), 8);puts("");
    
    hash_add(hash_compute("toto"), "toto object", &map);
    hash_add(hash_compute("tata"), "tata object", &map);
    hash_add(hash_compute("titi"), "titi object", &map);
    
    void*o1, *o2, *o3;
    hash_get(hash_compute("toto"), &o1, &map);
    hash_get(hash_compute("tata"), &o2, &map);
    hash_get(hash_compute("titi"), &o3, &map);
    
    puts(o1);
    puts(o2);
    puts(o3);
    o2 = NULL;
    
    hash_delete(hash_compute("tata"), &map);
    hash_get(hash_compute("tata"), &o2, &map);
    puts(o2);
    
    return 0;
}

#if TESTING
int main() {
    return testing();
}
#endif
