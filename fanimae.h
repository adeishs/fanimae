#ifndef H__FANIMAE_
#define H__FANIMAE_

#include <stddef.h>

#define FANIMAE_VERSION "MIREX 2005 Edition"
#define RESEARCHERS \
        "Alexandra L. Uitdenbogerd\n" \
        "Justin Zobel\n"

#define P_INVLISTPTR_SUFFIX ".fipp"
#define P_INVLISTPTR_SUFFIX_LEN strlen(P_INVLISTPTR_SUFFIX)
#define P_INVLIST_SUFFIX ".filp"
#define P_INVLIST_SUFFIX_LEN strlen(P_INVLIST_SUFFIX)
#define DOCLOOKUP_SUFFIX ".fdl"
#define DOCLOOKUP_SUFFIX_LEN strlen(DOCLOOKUP_SUFFIX)
#define P_DM12_ALPHABET "abcdefghijklmnopqrstuvwxy"
#define P_DM12_ALPHABET_SIZE strlen(P_DM12_ALPHABET)
#define NUM_OF_GRAMS 5
#define POS_SIZE 4

typedef size_t doc_num_t;
typedef unsigned char num_of_grams_t;
typedef unsigned char num_of_symbols_t;

#define MAX_DOC_NUM_T (doc_num_t)-1
#define MAX_NUM_OF_GRAMS_T (num_of_grams_t)-1
#define MAX_NUM_OF_SYMBOLS_T (num_of_symbols_t)-1

struct ng_idx_entry
{ doc_num_t num_of_docs;
  doc_num_t * doc_nums;
  };

typedef struct ng_idx_entry ng_idx_entry_t;

struct ng_idx
{ struct ng_idx_entry * entries;
  };

typedef struct ng_idx ng_idx_t;

#endif
