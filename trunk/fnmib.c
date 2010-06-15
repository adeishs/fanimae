/*
 * RMIT MIRT Project
 * Fanimae Index Builder MIREX 2005 Edition
 *
 * Copyright 2004--2005 by RMIT MIRT Project.
 *
 * Developer:
 * Iman S. H. Suyoto
 *
 * Filename: fnmib.c.
 *
 * Requires oakpark v0.2b.
 */

#include "fanimae.h"
#include "oakpark.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <limits.h>
#include <assert.h>

#define DEVELOPERS \
    "Iman S. H. Suyoto\n"
#define ARGI_IDX_FN 1
#define ARGI_SEQ_FN 2
#define MIN_ARGC 3
#define SIZE_T_MAX_ ((size_t)-1)

/* build status */
typedef enum bld_stat {
    BLD_STAT_OK,
    /* error reading sequence file */
    BLD_STAT_ERR_READ_SEQ,
    /* error writing to pitch pointer-to-inverted list
       file */
    BLD_STAT_ERR_WRITE_P_ILP,
    /* error writing to pitch inverted list file */
    BLD_STAT_ERR_WRITE_P_IL,
    /* error writing to duration pointer-to-inverted
       list file */
    BLD_STAT_ERR_WRITE_DL,
    /* error initializing in-memory index structure */
    BLD_STAT_ERR_INIT_IDX_STRUCT,
    /* erronous sequence file */
    BLD_STAT_ERR_SEQ,
    /* error adding to index */
    BLD_STAT_ERR_ADD_IDX
} bld_stat_t;

#define STR(x) #x
#define XSTR(x) STR(x)
#define close_file(fp) \
    if (fp) {\
        fclose(fp); \
        fp = NULL; \
    }
#define IN_LOC "in " __FILE__ ":" XSTR(__LINE__) "\n"

/*
 * function: show_usage
 * param: char *: program name
 * return: none
 * purpose: show program usage
 */
static void show_usage(char *argv0)
{
    fprintf(stderr,
            "RMIT MIRT Project\n"
            "Fanimae " FANIMAE_VERSION "\n"
            "Index Builder\n\n"
            "Developers:\n" DEVELOPERS "\n"
            "based on research by:\n"
            RESEARCHERS "\n\n"
            "To build an index:\n"
            "%s idxfn seqfn\n\n"
            "If idxfn.* exist, they will be overwritten\n\n",
            argv0);
}

/*
 * function: postingcmp
 * (for use by qsort() and bsearch())
 */
static int postingcmp(const void *x, const void *y)
{
    const doc_num_t *p1 = x;
    const doc_num_t *p2 = y;

    return (*p1 > *p2) ?
           1 : (*p1 < *p2) ?
               -1 : 0;
}

/*
 * function: write_uint
 * parameter: FILE *: output (binary) stream
 *           unsigned long: integer to output
 *           size_t: compression flag:
 *                   0 = compress (variable-nibble enconding)
 *                   > 0 = number of bytes of output integer
 * return: number of nibbles written on success
 *         negative value on failure
 * purpose: outputs an integer to a stream
 */
static int write_uint
           (FILE *ostream, unsigned long x, size_t nob)
{
    int result = 0;
    int is_finished = 0;
    int p;
    unsigned char ob = 0;  /* output byte */

    assert(!!ostream);
    if (nob > 0) {
        size_t n;

        for (n = 0; (n < nob) && !is_finished; ++n) {
            result = (fputc(x & 0xff, ostream) == EOF) ?
                     -1 : result + 2;
            if (result < 0) {
                is_finished = 1;
            } else {
                    x >>= 8;
            }
        }
    } else {
        if (x == 0) {
            p = fputc(0, ostream);
            return (p == EOF) ? -1 : 1;
        }
        p = 0;
        while (!is_finished) {
            if (x == 0) {
                is_finished = 1;
            } else {
                ob = x & 0x07;  /* 0000 ?... */
                p = 1;
                if (x >>= 3) {  /* any more bits? */
                    /* ?... 1... */
                    ob |= 0x08 | ((x & 0x07) << 4u);
                    ++p;
                    if (x >>= 3) {  /* any more bits? */
                        ob |= 0x80;  /* 1... 1... */
                    }
                }
                result = (fputc(ob, ostream) == EOF) ?
                         -1 : result + p;
                if (result < 0) {
                    is_finished = 1;
                }
            }
        }
    }
    return result;
}

/*
 * function: ulpow
 * parameter: unsigned long: x
 *            unsigned long: y
 * return: x raised to the power of y
 * purpose: calculates the value of x raised to the power of y
 */
static unsigned long ulpow(unsigned long x, unsigned long y)
{
    unsigned long result = 1;
    unsigned long c;

    for (c = 0; c < y; c++) {
        result *= x;
    }
    return result;
}

/*
 * function: get_entry_num
 * parameter: ng_idx_t *: index
 *            char *: n-gram
 *            size_t *: pointer to result placeholder
 * return: 0 on failure
 *         1 on success
 * purpose: find the entry number of the n-gram in index
 */
static int get_entry_num(char * ng, size_t * num)
{
    num_of_grams_t n;
    const num_of_symbols_t nos = P_DM12_ALPHABET_SIZE;
    const num_of_grams_t nog = NUM_OF_GRAMS;
    const char *symbols = P_DM12_ALPHABET;

    assert(!!ng && !!num);
    assert(strlen(ng) >= nog);
    *num = 0;
    for (n = 0; n < nog; n++) {
        ptrdiff_t d;
        char *p;

        /* find ng[n] in symbol list */
        p = strchr(symbols, ng[n]);
        if (p) {  /* if found */
            d = p - symbols;
            *num = nos * *num + d;
        } else {  /* if not found */
            return 0;    /* bail out */
        }
    }
    return 1;
}

/*
 * function: ng_init
 * parameter: nog: number of grams
 *            nos: number of symbols
 *            char *: symbols
 * return: NULL on failure
 *         pointer to index entries on success
 * purpose: initializes an n-gram index
 */
static ng_idx_t *ng_init(void)
{
    size_t num_of_entries = 1;
    size_t c;
    ng_idx_t * result = NULL;
    const num_of_symbols_t nos = P_DM12_ALPHABET_SIZE;
    const num_of_grams_t nog = NUM_OF_GRAMS;

    assert((MAX_DOC_NUM_T > 0) &&
           (MAX_NUM_OF_GRAMS_T > 0) &&
           (MAX_NUM_OF_SYMBOLS_T > 0));
    /* number of entries is
     * (number of symbols) ^ (number of grams)
     */
    num_of_entries = ulpow(nos, nog);
    if (num_of_entries >= SIZE_T_MAX_) {
        return NULL;
    }

    /* allocate memory for index */
    result = malloc(sizeof *result);
    if (result) {
        /* allocate memory for entries */
        result->entries =
        malloc(num_of_entries * sizeof *(result->entries));
        if (result->entries) {
            for (c = 0; c < num_of_entries; c++) {
                result->entries[c].num_of_docs = 0;
                result->entries[c].doc_nums = NULL;
        }
    } else {
        free(result);
        result = NULL;
    }
}
    return result;
}

/*
 * function: ng_destroy
 * parameter: ng_idx_entry_t *: pointer to index entries
 * purpose: destroys an n-gram index
 */
static void ng_destroy(ng_idx_t *idx)
{
    size_t c;
    const num_of_symbols_t nos = P_DM12_ALPHABET_SIZE;
    const num_of_grams_t nog = NUM_OF_GRAMS;

    if (!idx) {
        return;
    }

    for (c = 0; c < ulpow(nos, nog); c++) {
        free(idx->entries[c].doc_nums);
    }
    free(idx->entries);
    free(idx);
}

/*
 * function: ng_put
 * parameter: idx: pointer to index entries
 *            item: item to be put
 *            doc_num: document number
 * return: 1 on success
 *         0 on failure
 * purpose: puts an item to index
 */
static int ng_put
           (ng_idx_t *idx, char *item, doc_num_t doc_num)
{
    int result;
    size_t entry_num;

    assert(!!idx && !!item);
    result = get_entry_num(item, &entry_num);
    if (result) {
        ng_idx_entry_t * entry = &(idx->entries[entry_num]);
        doc_num_t d;
        int is_exist;

        is_exist = 0;
        /* find if document number already exists in
         * postings
         */
        for (d = 0; (d < entry->num_of_docs) && !is_exist;
             d++) {
            if (entry->doc_nums[d] == doc_num) {
                return 1;
            }
        }
    /* is maximum number of docs not reached? */
        if (entry->num_of_docs < SIZE_T_MAX_) {
            doc_num_t * tmp;

            tmp = realloc(entry->doc_nums,
                          ++(entry->num_of_docs) *
                          sizeof *(entry->doc_nums));
            if (tmp) {
                entry->doc_nums = tmp;
                entry->doc_nums[entry->num_of_docs - 1] =
                doc_num;
            } else {
                --(entry->num_of_docs);
                result = 0;
            }
        } else {
            result = 0;
        }
    }
    return result;
}

#define P_CHECK \
    if (p < 0) { \
        return p; \
    } else {\
        result += p; \
    }

/*
 * function: ng_save
 * parameter: ng_idx_t *: pointer to index
 *            FILE *: output (binary) stream for n-grams
 *            FILE *: output (binary) stream for inverted list
 * return: number of bytes written on success
 *         negative value on failure
 * purpose: builds an n-gram index and outputs it to a stream
 */
static int ng_save(ng_idx_t *idx, FILE *ng_fp, FILE *il_fp)
{
    size_t ec;
    const num_of_symbols_t nos = P_DM12_ALPHABET_SIZE;
    const num_of_grams_t nog = NUM_OF_GRAMS;
    const size_t num_of_entries = ulpow(nos, nog);
    int p;
    int result = 0;
    /* position is safely assumed to be at least 32 bits. Only
     * 32 bits are used.
     */
    long pos;

    assert(!!idx && !!ng_fp && !!il_fp);
    /* iterate through all possible entries. N-grams are not
     * stored
     */
    for (ec = 0; ec < num_of_entries; ec++) {
        ng_idx_entry_t * entry = &(idx->entries[ec]);
        doc_num_t dc;

        pos = ftell(il_fp);
        if (pos < 0) {
            return -1;
        }
        assert(pos >= 0);
        p = write_uint(ng_fp, (pos & 0xffffffff), POS_SIZE);
        P_CHECK;
        /* write the number of documents containing the
         * n-gram
         */
        p = write_uint(il_fp, entry->num_of_docs, 0);
        P_CHECK;
        if (entry->num_of_docs > 1) {
            /* sort document numbers */
            qsort(entry->doc_nums, entry->num_of_docs,
                        sizeof *(entry->doc_nums), postingcmp);
        }
        /* iterate through all postings in the document */
        for (dc = 0; dc < entry->num_of_docs; dc++) {
            p = write_uint(il_fp, entry->doc_nums[dc], 0);
            P_CHECK;
        }
    }
    return (result + 1) / 2;
}

/*
 * function: index_sequence
 * parameter: idx: pointer to index entries
 *            seq: sequence to be indexed
 *            song_num: song number
 * return: 0 on failure
 *         1 on success
 * purpose: indexes a sequence
 */
static int index_sequence
           (ng_idx_t *idx, char *seq, doc_num_t song_num)
{
    int result = 1;
    size_t seq_len;
    size_t sc;
    char ngram[NUM_OF_GRAMS + 1];

    assert(!!idx && !!seq);
    seq_len = strlen(seq);
    if (seq_len < NUM_OF_GRAMS) {
        return 1;
    }
    ngram[NUM_OF_GRAMS] = '\0';
    for (sc = 0;
         result && (sc < seq_len - NUM_OF_GRAMS);
         ++sc) {
        memcpy(ngram, &(seq[sc]), NUM_OF_GRAMS);
        result = ng_put(idx, ngram, song_num);
    }
    return result;
}

/*
 * function: build_index
 * parameter: seq_fp: sequence file pointer
 *            p_ilp_fp: pointers to inverted list file pointer
 *            p_il_fp: inverted list file pointer
 *            dl_fp: document name lookup file pointer
 * return: build status
 * purpose: build sequences in seq_fp
 */
static bld_stat_t build_index
                  (FILE *seq_fp,
                   FILE *p_ilp_fp, FILE *p_il_fp,
                   FILE *dl_fp)
{
    bld_stat_t result = BLD_STAT_OK;
    char *buf;
    size_t buf_len;
    ng_idx_t *p_idx;
    doc_num_t song_num = 0;
    void *tmp = NULL;
    char *ilp_buf = NULL;
    char *il_buf = NULL;
    static const char *pitch_prefix = "p:";
    const size_t pitch_prefix_len = strlen(pitch_prefix);
    static const char *seq_prefix = "***";
    const size_t seq_prefix_len = strlen(seq_prefix);

    assert(!!seq_fp &&
           !!p_ilp_fp && !!p_il_fp &&
           !!dl_fp);
    fprintf(stderr, "Initializing index structure...\n");
    p_idx = ng_init();
    if (!p_idx) {
        result = BLD_STAT_ERR_INIT_IDX_STRUCT;
    }
    if (result != BLD_STAT_OK) {
        goto BAILOUT;
    }
    fprintf(stderr, "Indexing...");
    fflush(stderr);
    while ((result == BLD_STAT_OK) &&
           (buf = oakpark_get_line(seq_fp, &buf_len))) {
        char *title = NULL;
        char *p_seq = NULL;

        if (strncmp(buf, pitch_prefix, pitch_prefix_len) !=
            0) {
            free(buf);
            continue;
        }
        /* get rid of the ending '\n' */
        title = buf + pitch_prefix_len;
        buf[--buf_len] = '\0';

        p_seq = strstr(title, seq_prefix);
        if (p_seq) {
            *p_seq = '\0'; 
            p_seq += seq_prefix_len;
            if (!index_sequence(p_idx, p_seq, song_num)) {
                result = BLD_STAT_ERR_ADD_IDX;
                fprintf
                (stderr, "\nError when inserting song %s\n",
                 title);
            }
        }
        fprintf(dl_fp, "%s\n", title);
        if ((song_num++ % 100) == 0) {
            fprintf(stderr, "#");
            fflush(stderr);
        }
        free(buf);
    }
    if (result != BLD_STAT_OK) {
        fprintf(stderr, "FAILED\n");
        return result;
    }
    fprintf(stderr, "\nDONE!\n"
                    "Writing pitch index to file... ");

    /* allocate space as large as possible for output
     * buffer to write index files. This will hopefully speed
     * up writing to the index files in practice.
     */
    if ((buf_len = SIZE_T_MAX_) > (1LU << 24)) {
        buf_len = 1LU << 24;
    }
    do {
        if (!(tmp = realloc(ilp_buf, buf_len))) {
            buf_len >>= 1;
        }
    } while (buf_len > BUFSIZ && !tmp);
    if (tmp) {
        ilp_buf = tmp;
        setvbuf(p_ilp_fp, ilp_buf, _IOFBF, buf_len);
    }

    if ((buf_len = SIZE_T_MAX_) > (1LU << 24)) {
        buf_len = 1LU << 24;
    }
    do {
        if (!(tmp = realloc(il_buf, buf_len))) {
            buf_len >>= 1;
        }
    } while (buf_len > BUFSIZ && !tmp);
    if (tmp) {
        il_buf = tmp;
        setvbuf(p_il_fp, il_buf, _IOFBF, buf_len);
    }

    fflush(stderr);
    if (!ng_save(p_idx, p_ilp_fp, p_il_fp)) {
        fprintf(stderr, "FAILED\n");
        result = BLD_STAT_ERR_WRITE_P_ILP;
        goto BAILOUT;
    }
    fprintf(stderr, "DONE!\n"
                    "Destroying in-memory pitch index... ");
    fflush(stderr);
    fprintf(stderr, "DONE!\n");
BAILOUT:
    fflush(NULL);
    setvbuf(p_ilp_fp, NULL, _IOFBF, BUFSIZ);
    setvbuf(p_il_fp, NULL, _IOFBF, BUFSIZ);
    free(il_buf);
    free(ilp_buf);
    ng_destroy(p_idx);
    return result;
}

/* main function */

int main(int argc, char **argv)
{
    int result = EXIT_SUCCESS;

    if (argc >= MIN_ARGC) {  /* are there enough arguments? */
        char *idx_fn = argv[ARGI_IDX_FN];
        size_t idx_fn_len = strlen(idx_fn);
        /* pitch inverted list pointer filename */
        char *p_ilp_fn = NULL;
        char *p_il_fn = NULL;  /* pitch inverted list filename */
        /* duration inverted list pointer filename */
        char *dl_fn = NULL;  /* document lookup filename */
        FILE *p_ilp_fp = NULL;
        FILE *p_il_fp = NULL;
        FILE *dl_fp = NULL;
        char *seq_fn = argv[ARGI_SEQ_FN];
        FILE *seq_fp = NULL;
        bld_stat_t build_status;

        /* allocate spaces for filenames */
        if ((p_ilp_fn =
             malloc(idx_fn_len + P_INVLISTPTR_SUFFIX_LEN + 1))
            &&
            (p_il_fn =
             malloc(idx_fn_len + P_INVLIST_SUFFIX_LEN + 1)) &&
            (dl_fn =
             malloc(idx_fn_len + DOCLOOKUP_SUFFIX_LEN + 1))) {
            sprintf(p_ilp_fn,
                    "%s" P_INVLISTPTR_SUFFIX, idx_fn);
            sprintf(p_il_fn,
                    "%s" P_INVLIST_SUFFIX, idx_fn);
            sprintf(dl_fn,
                    "%s" DOCLOOKUP_SUFFIX, idx_fn);
        } else {
            fprintf(stderr, "Memory allocation error " IN_LOC);
            goto BAIL_OUT;
        }
        if (!(p_ilp_fp = fopen(p_ilp_fn, "wb"))) {
            fprintf(stderr, "Failed opening %s " IN_LOC,
                    p_ilp_fn);
            goto BAIL_OUT;
        }
        if (!(p_il_fp = fopen(p_il_fn, "wb"))) {
            fprintf(stderr, "Failed opening %s " IN_LOC,
                    p_il_fn);
            goto BAIL_OUT;
        }
        if (!(dl_fp = fopen(dl_fn, "w"))) {
            fprintf(stderr, "Failed opening %s " IN_LOC,
                    dl_fn);
            goto BAIL_OUT;
        }
        if (!(seq_fp = fopen(seq_fn, "r"))) {
            fprintf(stderr, "Failed opening %s " IN_LOC,
                    seq_fn);
            goto BAIL_OUT;
        }

        fprintf(stderr, "Indexing %s...\n", seq_fn);
        fflush(stderr);
        build_status = build_index
                       (seq_fp, p_ilp_fp, p_il_fp, dl_fp);
        switch (build_status) {
            case BLD_STAT_OK:
                fprintf(stderr, " DONE!\n");
                break;
            case BLD_STAT_ERR_READ_SEQ:
                fprintf(stderr, "Error reading %s " IN_LOC,
                        seq_fn);
                break;
            case BLD_STAT_ERR_WRITE_P_ILP:
                fprintf(stderr, "Error writing %s " IN_LOC,
                        p_ilp_fn);
                break;
            case BLD_STAT_ERR_WRITE_P_IL:
                fprintf(stderr, "Error writing %s " IN_LOC,
                        p_il_fn);
                break;
            case BLD_STAT_ERR_WRITE_DL:
                fprintf(stderr, "Error writing %s " IN_LOC,
                        dl_fn);
                break;
            case BLD_STAT_ERR_INIT_IDX_STRUCT:
                fprintf(stderr, "Error initializing index "
                                "structure " IN_LOC);
                break;
            default:
                fprintf(stderr, "\nInvalid build_status: %d "
                                IN_LOC, build_status);
        }
        close_file(seq_fp);
        if (build_status != BLD_STAT_OK) {
            result = EXIT_FAILURE;
        }
BAIL_OUT:
        close_file(seq_fp);
        close_file(dl_fp);
        close_file(p_il_fp);
        close_file(p_ilp_fp);

        /* release spaces used by filenames */
        free(dl_fn);
        free(p_il_fn);
        free(p_ilp_fn);
    } else {
        show_usage(argv[0]);
    }
    return result;
}
