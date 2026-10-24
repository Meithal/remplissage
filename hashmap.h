
struct yt_hashmap {
    struct { int32_t complete_key; void*the_thing;} bucket[256];
};

/// calcule le hash d'une chaine de caracteres sur 32 bits, les 2 bits de poids fort ne peuvent jamais etre touchés
/// car on s'en sert pour detecter les hash supprimmés
int32_t compute_hash(char* key);

int add_object(uint32_t key, void * thing, struct yt_hashmap* map);
int get_object(uint32_t key, void ** thing, struct yt_hashmap* map);
int delete_object(uint32_t key, struct yt_hashmap*map);
