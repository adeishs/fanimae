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
    double score_sum;
    double pitch_score;
    double ioi_score;
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
        a->score_sum =
        a->pitch_score =
        a->ioi_score = 0.0;
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
void insert_answer(struct answers *answers, char *title,
                   double pitch_score, double ioi_score)
{
    unsigned short n = answers->num_of_answers;
    unsigned short curr = 0;
    struct answer *a = answers->items;
    struct answer *root = a;
    double score_sum = pitch_score + ioi_score;

    /* if the heap is already full, replace the answer with
     * minimum score_sum, i.e. the root, with the new answer, and
     * top-down min-heapify
     */
    if (n == answers->max_num_of_answers) {
        /* replace root */
        root->title = title;
        root->pitch_score = pitch_score;
        root->ioi_score = ioi_score;
        root->score_sum = score_sum;

        /* top-down min-heapify */
        while (curr < n) {
            unsigned short left = curr * 2 + 1;
            unsigned short right = left + 1;
            unsigned short min = curr;

            if (left < n && a[left].score_sum < a[min].score_sum) {
                min = left;
            }
            if (right < n && a[right].score_sum < a[min].score_sum) {
                min = right;
            }
            if (min == curr) {
                return;
            }

            swap_answers(a + min, a + curr);
            curr = min;
        }
    } else {
        /* the heap isn't full, so put the new answer at the
         * tail
         */
        a[n].title = title;
        a[n].pitch_score = pitch_score;
        a[n].ioi_score = ioi_score;
        a[n].score_sum = score_sum;

        /* bottom-up min-heapify to put the new answer at the
         * right place
         */
        curr = n;
        while (curr > 0) {
            unsigned short parent = curr / 2;

            if (a[parent].score_sum <= score_sum) {
                break;
            }

            swap_answers(a + parent, a + curr);
            curr = parent;
        };
        answers->num_of_answers++;
    }
}

/* answer comparison function for qsort() */
int cmp_answer(const void *a_v, const void *b_v)
{
    const struct answer *a = a_v;
    const struct answer *b = b_v;

    return a->score_sum > b->score_sum ? 1 :
           a->score_sum < b->score_sum ? -1 : 0;
}

/* sort answers in ascending score_sum order */
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
    char *query_title = NULL;

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
    while (query = oakpark_get_line(coll_fp, &query_len)) {
    }

    /* present answer */
    query_title = "title";  /* FIXME: it's just a stub */
    printf("%s", query_title);
    output_answers(answers);
    /* no error, so exit with success status */
    result = EXIT_SUCCESS;
bail_out:
    if (coll_fp) {
        fclose(coll_fp);
    }
    destroy_answers(answers);
    return result;
}