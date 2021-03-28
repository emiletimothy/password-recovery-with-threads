#define _GNU_SOURCE
#include <assert.h>
#include <crypt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dictionary_words.h"
#include "thread_pool.h"

const char HASH_START[] = "$6$";
const size_t SALT_LENGTH = 20;
const size_t HASH_LENGTH = 106;
const size_t NUM_THREADS = 16;
static size_t hash_count = 0;
static char **hashes = NULL;

static inline bool hashes_match(const char *password, const char *hash) {
    char salt[SALT_LENGTH + 1];
    memcpy(salt, hash, sizeof(char[SALT_LENGTH]));
    salt[SALT_LENGTH] = '\0';
    struct crypt_data data;
    memset(&data, 0, sizeof(data));
    char *hashed = crypt_r(password, salt, &data);
    return strcmp(&hashed[SALT_LENGTH], &hash[SALT_LENGTH]) == 0;
}

void brute_force_modified_words(void *index_ptr) {
    char *dictionary_word = (char *) index_ptr;
    size_t word_length = strlen(dictionary_word);
    char word[word_length + 2];
    for (size_t j = 0; j <= word_length; j++) {
        memcpy(word, dictionary_word, j * sizeof(char));
        memcpy(&word[j + 1], &dictionary_word[j], (word_length - j + 1) * sizeof(char));
        for (char k = '0'; k <= '9'; k++) {
            word[j] = k;
            // fprintf(stderr, "%s\n", word);
            for (size_t i = 0; i < hash_count; i++) {
                if (hashes_match(word, hashes[i])) {
                    printf("%s\n", word);
                }
            }
        }
    }
}

int main(void) {
    // Read in the hashes from the standard input
    char *line = NULL;
    size_t line_capacity = 0;
    while (getline(&line, &line_capacity, stdin) > 0 && line[0] != '\n') {
        hashes = realloc(hashes, sizeof(char * [hash_count + 1]));
        assert(hashes != NULL);
        assert(strlen(line) == HASH_LENGTH + 1 &&
               strncmp(line, HASH_START, strlen(HASH_START)) == 0 &&
               line[HASH_LENGTH] == '\n');
        char *hash = malloc(sizeof(char[HASH_LENGTH + 1]));
        assert(hash != NULL);
        memcpy(hash, line, sizeof(char[HASH_LENGTH]));
        hash[HASH_LENGTH] = '\0';
        hashes[hash_count++] = hash;
    }
    free(line);

    // Uses the threadpool to recover the passwords from the hashes.
    // Assumes the provided dictionary.h includes all the
    // possible root words.
    thread_pool_t *thread_pool = thread_pool_init(NUM_THREADS);
    for (size_t index = 0; index < NUM_DICTIONARY_WORDS; index++) {
        thread_pool_add_work(thread_pool, brute_force_modified_words,
                             (void *) DICTIONARY[index]);
    }
    thread_pool_finish(thread_pool);
    return 0;
}
