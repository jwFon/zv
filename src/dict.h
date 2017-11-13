#ifndef KV_DICT_H
#define KV_DICT_H
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <functional>
#define HASHTABLE_INIT_SIZE 128

template<typename K>
struct MurMurHash
{
	MurMurHash(uint32_t add_seed = 3) : dict_hash_function_seed(5381) {
		dict_hash_function_seed += add_seed;
	}
	unsigned int operator()(const K& key);
	unsigned int hash(const void *key, int len);
	uint32_t dict_hash_function_seed;
};

template<typename K>
unsigned int MurMurHash<K>::operator()(const K& key)
{
	std::cout << &key << " " << sizeof(K) << std::endl;
	const unsigned char* p =  (const unsigned char*)&key;
	return hash(&key, sizeof(K));
}

/* MurmurHash2, by Austin Appleby
 * Note - This code makes a few assumptions about how your machine behaves -
 * 1. We can read a 4-byte value from any address without crashing
 * 2. sizeof(int) == 4
 *
 * And it has a few limitations -
 *
 * 1. It will not work incrementally.
 * 2. It will not produce the same results on little-endian and big-endian
 *    machines.
 */
template<typename K>
unsigned int MurMurHash<K>::hash(const void *key, int len)
{
	/* 'm' and 'r' are mixing constants generated offline.
	 They're not really 'magic', they just happen to work well.  */
	uint32_t seed = dict_hash_function_seed;
	const uint32_t m = 0x5bd1e995;
	const int r = 24;

	/* Initialize the hash to a 'random' value */
	uint32_t h = seed ^ len;

	/* Mix 4 bytes at a time into the hash */
	const unsigned char *data = (const unsigned char *)key;

	while(len >= 4) {
	    uint32_t k = *(uint32_t*)data;

	    k *= m;
	    k ^= k >> r;
	    k *= m;

	    h *= m;
	    h ^= k;

	    data += 4;
	    len -= 4;
	}

	/* Handle the last few bytes of the input array  */
	switch(len) {
		case 3: h ^= data[2] << 16;
		case 2: h ^= data[1] << 8;
		case 1: h ^= data[0]; h *= m;
	};

	/* Do a few final mixes of the hash to ensure the last few
	 * bytes are well-incorporated. */
	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return (unsigned int)h;
}


template<typename K, typename V>
struct DictEntry
{
	K key;
	V value;
	DictEntry<K, V> *next;

	DictEntry<K, V>() : next(NULL) {
		char* ptr =  reinterpret_cast<char*>(&value);
		for (int i = 0; i < sizeof(V); i++) {
			ptr[i] = 0;
		}
	};
};

template<typename K, typename V, typename HashFunc>
struct HashTable
{
	DictEntry<K, V> **table;
	unsigned int size;
	unsigned int sizemask;
	unsigned int used;
	HashFunc hash_func;

	HashTable<K, V, HashFunc>(unsigned int size = HASHTABLE_INIT_SIZE) :
		table(NULL), size(size), sizemask(size-1), used(0)  {
		table = (DictEntry<K, V> **)malloc(sizeof(DictEntry<K, V> *) * size);
		for (int i = 0; i < size; i++) {
			table[i] = NULL;
		}
	}
	

	unsigned int getHashIndex(const K& key) {
		return hash_func(key) & sizemask;
	}
	~HashTable() {
		for (int i = 0 ; i < size; i++) {
			DictEntry<K, V> *entry = table[i];
			while (entry) {
				DictEntry<K, V> *tmp = entry->next;
				delete entry;
				entry = tmp;
			}
		}
		free(table);
	}
};


template<typename K, typename V, typename Hash = std::hash<K>>
struct Dict 
{
	HashTable<K, V, Hash>** ht;
	int rehashId;

	Dict<K, V, Hash>() : rehashId(-1) {
		ht = (HashTable<K, V, Hash>**) malloc(sizeof(HashTable<K, V, Hash>*) * 2);
		ht[0] = new HashTable<K, V, Hash>();
		ht[1] = NULL;
	}
	

	V& operator[](const K& k);
	bool del(const K& k);

	~Dict() {
		for (int i = 0; i < 2; i++) {
			if (ht[i]) {
				delete ht[i];
			}
		}
		free(ht);
	}
	void rehash(int cnt);
};

template<typename K, typename V, typename Hash>
void Dict<K, V, Hash>::rehash(int cnt)
{
	while (cnt-- && ht[0]->used > 0) {
		while(true) {
			DictEntry<K, V> *entry = ht[0]->table[rehashId];
			if (entry == NULL) {
				rehashId++;
				continue;
			}
			while (entry) {
				auto* tmp = entry->next;
				unsigned int new_index = ht[1]->getHashIndex(entry->key);
				entry->next = ht[1]->table[new_index];
				ht[1]->table[new_index] = entry;
				entry = tmp;
				ht[0]->used--;
				ht[1]->used++;
			}
			ht[0]->table[rehashId] = NULL;
			rehashId++;
			break;
		}
	}
	if (ht[0]->used == 0) {
		delete ht[0];
		ht[0] = ht[1];
		ht[1] = NULL;
		rehashId = -1;
	}
}

template<typename K, typename V, typename Hash>
V& Dict<K, V, Hash>::operator[](const K& k)
{
	if (ht[0]->used >=  (size_t)ht[0]->size * 0.8) {
		rehashId = 0;
		ht[1] = new HashTable<K, V, Hash>(ht[0]->size * 2);
	}

	if (rehashId != -1) {
		rehash(1);
	}

	unsigned int index, i;
	DictEntry<K, V> *entry;
	for (i = 0; i < 2; i++) {
		index = ht[i]->getHashIndex(k);
		entry =  ht[i]->table[index];

		while (entry) {
			if (entry->key == k) {
				return entry->value;
			}
			entry = entry->next;
		}
		if (rehashId == -1) {
			break;
		}
	}
	i = rehashId == -1 ? 0 : 1;
	entry = new DictEntry<K, V>;
	entry->key = k;
	entry->next = ht[i]->table[index];
	ht[i]->table[index] = entry;
	ht[i]->used++;
	return entry->value;
}
template<typename K, typename V, typename Hash>
bool Dict<K, V, Hash>::del(const K& k)
{
	if (rehashId != -1) {
		rehash(1);
	}

	unsigned int index, i;
	for (i = 0; i < 2; i++) {
		index = ht[i]->getHashIndex(k);
		DictEntry<K, V> *entry =  ht[i]->table[index], *pre_entry = NULL;
		while (entry) {
			if (entry->key == k) {
				if (pre_entry) {
					pre_entry->next = entry->next;
				} else {
					ht[i]->table[index] = entry->next;;
				}
				ht[i]->used--;
				delete entry;
				return true;	
			}
			pre_entry = (pre_entry == NULL) ? entry : pre_entry->next;
			entry = entry->next;
		}
		if (rehashId == -1) {
			break;
		}
	}

	return false;
}
#endif

