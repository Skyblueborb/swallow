#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"

typedef struct {
    int score;
    char username[MAX_USERNAME_LENGTH];
} SortEntry;

static int compare_scores(const void* a, const void* b) {
    const SortEntry* entryA = (SortEntry*)a;
    const SortEntry* entryB = (SortEntry*)b;
    return (entryB->score - entryA->score);
}

const RankingNode* load_rankings() {
    FILE* file = fopen("ranking.txt", "r");
    if (!file) {
        return NULL;
    }

    const RankingNode* head = NULL;
    RankingNode* tail = NULL;

    int score = 0;
    char name[MAX_USERNAME_LENGTH] = {0};

    while (fscanf(file, "%d %49s", &score, name) == 2) {
        RankingNode* new_node = (RankingNode*)malloc(sizeof(RankingNode));
        if (!new_node) {
            break;
        }

        new_node->score = score;
        strncpy(new_node->username, name, MAX_USERNAME_LENGTH - 1);
        new_node->username[MAX_USERNAME_LENGTH - 1] = '\0';
        new_node->next = NULL;

        if (head == NULL) {
            head = new_node;
            tail = new_node;
        } else {
            tail->next = new_node;
            tail = new_node;
        }
    }

    fclose(file);
    return head;
}

void free_rankings(RankingNode* head) {
    RankingNode* current = head;
    while (current != NULL) {
        RankingNode* next = current->next;
        free(current);
        current = next;
    }
}

void save_ranking(Game* game) {
    SortEntry* entries = NULL;
    int count = 0;

    FILE* read_file = fopen("ranking.txt", "r");
    if (read_file) {
        int temp_score = 0;
        char temp_name[MAX_USERNAME_LENGTH] = {0};
        while (fscanf(read_file, "%d %49s", &temp_score, temp_name) == 2) {
            entries = (SortEntry*)realloc(entries, sizeof(SortEntry) * (count + 1));
            entries[count].score = temp_score;
            strcpy(entries[count].username, temp_name);
            count++;
        }
        fclose(read_file);
    }

    entries = (SortEntry*)realloc(entries, sizeof(SortEntry) * (count + 1));
    entries[count].score = game->score;
    strncpy(entries[count].username, game->username, MAX_USERNAME_LENGTH - 1);
    entries[count].username[MAX_USERNAME_LENGTH - 1] = '\0';
    count++;

    qsort(entries, count, sizeof(SortEntry), compare_scores);

    FILE* write_file = fopen("ranking.txt", "w");
    if (!write_file) {
        if (entries) {
            free(entries);
        }
        return;
    }

    for (int i = 0; i < count; i++) {
        fprintf(write_file, "%d %s\n", entries[i].score, entries[i].username);
    }

    fclose(write_file);
    if (entries) {
        free(entries);
    }
}
