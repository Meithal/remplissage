#include <stdint.h>
#include <stdio.h>

#include "hashmap.h"


int32_t compute_hash(char* key)
{
    int32_t sum = 0;
    uint8_t kick1 = 0b1010'1010;
    uint8_t kick2 = 0b0101'0101;
    uint8_t kick3 = 0b1111'1111;
    uint8_t kick4 = 0b0000'0000;
    for (int i = 0; key[i]; i++) {
        kick1 ^= key[i]; kick1 <<= 2; kick1 |= 0b10;
        kick2 ^= key[i]; kick2 <<= 2; kick2 |= 0b10;
        kick3 ^= key[i]; kick3 <<= 2; kick3 |= 0b10;
        kick4 ^= key[i]; kick4 <<= 2; kick4 |= 0b10;
    }
    sum = ((kick1 & 0b0011'1111) << 24) | (kick2 << 16) | (kick3 << 8) | kick4;
    
    return sum;
}

/** Ajoute un objet dans la hash map, retourne -1 si pleine, retourne l'offset entre 0 et 256 sinon */
int add_object(uint32_t key, void * thing, struct yt_hashmap* map)
{
    uint8_t primary =
        ((0b11 << 6) & key)
    + (((0b11 << (4 + 8)) & key) >> 8)
    + (((0b11 << (2 + 16)) & key) >> 16)
    + (((0b11 << (0 + 24)) & key) >> 24);
    
    uint8_t begin = primary;
    while (map->bucket[primary].complete_key != 0) {
        if(map->bucket[primary].complete_key & (1 << 31))
            /// si entrée supprimée, le bit de poids le + fort est à 1, donc on prend cette place
            break;
        primary ++; /// `uint_8` fait le modulo pour nous
        if(primary == begin) return -1; /// hashmap pleine
    }
    if (map->bucket[primary].complete_key == 0) {
        map->bucket[primary].complete_key = key;
        map->bucket[primary].the_thing = thing;
    }
    return primary;
}

int get_object(uint32_t key, void ** thing, struct yt_hashmap* map)
{
    uint8_t primary =
        ((0b11 << 6) & key)
    + (((0b11 << (4 + 8)) & key) >> 8)
    + (((0b11 << (2 + 16)) & key) >> 16)
    + (((0b11 << (0 + 24)) & key) >> 24);
    
    uint8_t begin = primary;
    while(map->bucket[primary].complete_key != key) {
        
        primary++;
        if(map->bucket[primary].complete_key == 0)
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
int delete_object(uint32_t key, struct yt_hashmap*map)
{
    uint8_t primary =
        ((0b11 << 6) & key)
    + (((0b11 << (4 + 8)) & key) >> 8)
    + (((0b11 << (2 + 16)) & key) >> 16)
    + (((0b11 << (0 + 24)) & key) >> 24);

    uint8_t begin = primary;
    while(map->bucket[primary].complete_key != key) {
        
        primary++;
        if(map->bucket[primary].complete_key == 0)
            return -1; /// non trouvé
        if(primary == begin)
            return -2; /// non trouvé apres avoir bouclé
    }
    /// laisse un marqueur afin de pouvoir atteindre les objets decalés auparavent
    map->bucket[primary].complete_key = (1 << 31);
    
    return primary;
}


/// affiche les bits d'un nombre
int pbits(uint32_t num, int size)
{
    for(int i = size-1; i >=0 ; i--) {
        putchar('0' + (!!(num & (1 << i))));
        if(i%8 == 0) putchar(' ');
    }
    //putchar('\n');
    
    return 0;
}


int main()
{
    pbits(compute_hash("0"), 32); printf(" %u \"%s\" ", compute_hash("0"), "0"); pbits('0', 8);puts("");
    pbits(compute_hash(""), 32); printf(" %u \"%s\"\n", compute_hash(""), "");
    pbits(compute_hash("10"), 32); printf(" %u \"%s\"\n", compute_hash("10"), "10");
    pbits(compute_hash("01"), 32); printf(" %u \"%s\"\n", compute_hash("01"), "01");
    pbits(compute_hash("abc"), 32); printf(" %u \"%s\"\n", compute_hash("abc"), "abc");
    pbits(compute_hash("bac"), 32); printf(" %u \"%s\"\n", compute_hash("bac"), "bac");
    
    struct yt_hashmap map = {0};
    pbits(add_object(compute_hash("0"), NULL, &map), 8);puts("");
    
    add_object(compute_hash("toto"), "toto object", &map);
    add_object(compute_hash("tata"), "tata object", &map);
    add_object(compute_hash("titi"), "titi object", &map);
    
    void*o1, *o2, *o3;
    get_object(compute_hash("toto"), &o1, &map);
    get_object(compute_hash("tata"), &o2, &map);
    get_object(compute_hash("titi"), &o3, &map);
    
    puts(o1);
    puts(o2);
    puts(o3);
    o2 = NULL;
    
    delete_object(compute_hash("tata"), &map);
    get_object(compute_hash("tata"), &o2, &map);
    puts(o2);
}
