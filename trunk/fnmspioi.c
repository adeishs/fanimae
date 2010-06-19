/*
 * Fanimae MIREX 2010 Edition
 * Search (Pitch and IOI)
 *
 * Copyright 2010 by RMIT MIRT Project.
 * Copyright 2010 by Iman S. H. Suyoto.
 *
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>

#include "oakpark.h"

#define DEFAULT_NUM_OF_ANSWERS 10

/* answers are organized as min-heap */
struct answer {
    double score;
    char *title;
};

struct answers {
    unsigned short max_num_of_answers;
    unsigned short num_of_answers;
    struct answer *items;
};

/* create a list of answers */
struct answers *create_answers
                (unsigned short max_num_of_answers)
{
    struct answers *answers = malloc(sizeof *answers);
    unsigned short c = 0;

    if (!answers) {
        goto bail_out;
    }
    if (!(answers->items =
          malloc(max_num_of_answers *
                 sizeof *answers->items))) {
        free(answers);
        goto bail_out;
    }

    answers->max_num_of_answers = max_num_of_answers;
    answers->num_of_answers = 0;
    do {
        struct answer *a = answers->items + c;

        a->title = NULL;
        a->score = 0.0;
    } while (++c < max_num_of_answers);
bail_out:
    return answers;
}

void swap_answers(struct answer *a, struct answer *b)
{
    struct answer tmp = *a;

    *a = *b;
    *b = tmp;
}

/* insert an answer */
int insert_answer(struct answers *answers, char *title,
                  double score)
{
    unsigned short n = answers->num_of_answers;
    unsigned short curr = 0;
    struct answer *a = answers->items;
    struct answer *root = a;
    size_t title_len = strlen(title);

    /* if the heap is already full, replace the answer with
     * minimum score, i.e. the root, with the new answer, and
     * top-down min-heapify
     */
    if (n == answers->max_num_of_answers) {
        void *tmp = realloc(root->title, title_len + 1);

        if (!tmp) {
            return 0;
        }
        /* replace root */
        strcpy(root->title = tmp, title);
        root->score = score;

        /* top-down min-heapify */
        while (curr < n) {
            unsigned short left = curr * 2 + 1;
            unsigned short right = left + 1;
            unsigned short min = curr;

            if (left < n && a[left].score < a[min].score) {
                min = left;
            }
            if (right < n && a[right].score < a[min].score) {
                min = right;
            }
            if (min == curr) {
                return 1;
            }

            swap_answers(a + min, a + curr);
            curr = min;
        }
    } else {
        /* the heap isn't full, so put the new answer at the
         * tail
         */
        if (!(a[n].title = malloc(title_len + 1))) {
            return 0;
        }
        strcpy(a[n].title, title);
        a[n].score = score;

        /* bottom-up min-heapify to put the new answer at the
         * right place
         */
        curr = n;
        while (curr > 0) {
            unsigned short parent = curr / 2;

            if (a[parent].score <= score) {
                break;
            }

            swap_answers(a + parent, a + curr);
            curr = parent;
        };
        answers->num_of_answers++;
    }
    return 1;
}

/* answer comparison function for qsort() */
int cmp_answer(const void *a_v, const void *b_v)
{
    const struct answer *a = a_v;
    const struct answer *b = b_v;

    return a->score > b->score ? 1 :
           a->score < b->score ? -1 : 0;
}

/* sort answers in ascending score order */
void sort_answers(struct answers *answers)
{
    qsort(answers->items, answers->num_of_answers,
          sizeof *answers->items, cmp_answer);
}

/* destroy a list of answers */
void destroy_answers(struct answers *answers)
{
    if (answers) {
        free(answers->items);
        free(answers);
    }
}

/* validate and parse a sequence line */
int parse_seq(char *seq_line, char **title,
              char **pitch_seq, char **ioi_seq)
{
    const char *seq_line_prefix = "pi:";
    const char *sep = "***";
    const size_t seq_line_prefix_len = strlen(seq_line_prefix);

    if (!(strstr(seq_line, seq_line_prefix) == seq_line)) {
        return 0;
    }
    *title = seq_line + seq_line_prefix_len;

    if (!(*pitch_seq =
          oakpark_tokenize_str(*title, sep))) {
        return 0;
    }

    if (!(*ioi_seq = 
          oakpark_tokenize_str(*pitch_seq, sep))) {
        return 0;
    }
    return 1;
}

/* calculate similarity */
int calc_sim(double *sim_score,
             const char *answer_pitch_seq,
             const char *query_pitch_seq,
             const char *answer_ioi_seq,
             const char *query_ioi_seq)
{
    *sim_score = 0;
    return 1;
}

/* query the collection */
int query_coll(FILE *coll_fp, struct answers *answers,
               char *query_pitch_seq, char *query_ioi_seq)
{
    int result = 0;
    char *coll_line = NULL;
    size_t coll_line_len = 0;

    if (!(coll_fp && answers &&
          query_pitch_seq && query_ioi_seq)) {
        goto bail_out;
    }

    while ((coll_line =
            oakpark_get_line(coll_fp, &coll_line_len)) !=
           NULL) {
        char *answer_title = NULL;
        char *answer_pitch_seq = NULL;
        char *answer_ioi_seq = NULL;
        double sim_score = 0;

        /* validate and parse collection answer */
        if (!parse_seq(coll_line, &answer_title,
                       &answer_pitch_seq, &answer_ioi_seq)) {
            break;
        }

        if (!calc_sim(&sim_score,
                      answer_pitch_seq, query_pitch_seq,
                      answer_ioi_seq, query_ioi_seq)) {
            break;
        }

        if (!insert_answer(answers, answer_title, sim_score)) {
            break;
        }

        free(coll_line);
    }
    if (coll_line) {
        free(coll_line);
    }

    result = 1;
bail_out:
    return result;
}

/* output answers */
void output_answers(struct answers *answers)
{
    struct answer *curr = answers->items +
                          answers->num_of_answers;

    while (curr != answers->items) {
        printf(" %s", (--curr)->title);
    }
    printf("\n");
}

/* program entry point */
int main(int argc, char **argv)
{
    int result = EXIT_FAILURE;
    unsigned short num_of_answers = 0;
    const char *num_of_answers_s =
                getenv("FNM_NUM_OF_ANSWERS");
    unsigned long n =
                  num_of_answers_s ?
                  strtoul(num_of_answers_s, NULL, 10) :
                  DEFAULT_NUM_OF_ANSWERS;
    struct answers *answers = NULL;
    FILE *coll_fp = NULL;
    char *coll_fn = NULL;
    char *query = NULL;
    size_t query_len = 0;

    /* validate command line argument */
    if (argc != 2) {
        fprintf(stderr, "Usage:\n" \
                        "%s coll-seq\n\n", argv[0]);
        goto bail_out;
    }
    coll_fn = argv[1];

    /* open collection sequence file */
    errno = 0;
    if (!(coll_fp = fopen(coll_fn, "r"))) {
        fprintf(stderr, "Can't open ");
        perror(coll_fn);
        goto bail_out;
    }

    /* allocate memory to rank answers */
    if (n == 0) {
        n = DEFAULT_NUM_OF_ANSWERS;
    }
    num_of_answers = (n > USHRT_MAX) ? USHRT_MAX : n;
    if (!(answers = create_answers(num_of_answers))) {
        fprintf(stderr,
                "Can't allocate memory for answers in %s:%d",
                __FILE__, __LINE__);
        goto bail_out;
    }

    /* query the collection */
    while ((query = oakpark_get_line(stdin, &query_len)) !=
           NULL) {
        char *pitch_seq = NULL;
        char *ioi_seq = NULL;
        char *query_title = NULL;

        /* validate and parse query */
        if (!parse_seq(query, &query_title,
                       &pitch_seq, &ioi_seq)) {
            break;
        }

        /* query the collection */
        rewind(coll_fp);
        if (!(query_coll(coll_fp, answers,
                         pitch_seq, ioi_seq))) {
            break;
        }

        /* present answers */
        printf("%s", query_title);
        output_answers(answers);

        /* clean up */
        free(query);
    }
    if (query) {
        free(query);
    }

    /* no error, so exit with success status */
    result = EXIT_SUCCESS;
bail_out:
    if (coll_fp) {
        fclose(coll_fp);
    }
    destroy_answers(answers);
    return result;
}
