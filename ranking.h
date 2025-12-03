#ifndef RANKING_H
#define RANKING_H

#include "types.h"

RankingNode* load_rankings();
void save_ranking(Game* game);
void free_rankings(RankingNode* head);

#endif  // RANKING_H
