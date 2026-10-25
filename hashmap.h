
struct yt_hashmap {
    struct {
        union yt_key {
            struct {uint32_t djb2; uint32_t sdbm;} str;
            uint64_t tot;
        } complete_key;
        void*the_thing;
    } bucket[256];
};

/// calcule le hash d'une chaine de caracteres sur 32 bits, les 2 bits de poids fort ne peuvent jamais etre touchés
/// car on s'en sert pour detecter les hash supprimmés
union yt_key hash_compute(char* key);

int hash_add(union yt_key key, void * thing, struct yt_hashmap* map);
int hash_get(union yt_key key, void ** thing, struct yt_hashmap* map);
int hash_delete(union yt_key key, struct yt_hashmap*map);
