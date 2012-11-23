/*
 * Copyright (c) 2012, BohuTANG <overred.shuttler at gmail dot com>
 * All rights reserved.
 * Code is licensed with GPL. See COPYING.GPL file.
 *
 *  Fractional cascading: To speed up SST level's search
 */

#include "fc.h"
#include "xmalloc.h"
#include "debug.h"

int _count2idx(int level) 
{
	int k = 0;
	int idx = 0;

	for (k = 0; k < level; k++)
		idx += (1<<k) * (L0_SIZE/ITEM_SIZE/GAP);

	return idx;
}

int _idx2count(int idx)
{
	return idx * GAP;
}

struct fc *fc_new(int sst_count)
{
	struct fc *fc = xcalloc(1, sizeof(struct fc));

	fc->sst_count = sst_count;
	fc->fcs = xcalloc(1 + sst_count / GAP, ITEM_SIZE);

	return fc;
}

void fc_build(struct fc *fc, struct cola_item *items, int count, int level)
{
	int i;
	int idx = _count2idx(level);

	for (i = 0; i < count; i += GAP) {
		__DEBUG("--- idx#%d,building....key:%s", idx, items[i].data);
		memcpy(&fc->fcs[idx++], &items[i], ITEM_SIZE);
	}
}

int fc_search(struct fc *fc, const char *key, int level)
{
	int cmp;
	int i;
	int start = 0;

	for (i = 0; i < level; i++)
		start += (1<<i) * (L0_SIZE / ITEM_SIZE);

	for (i = start; i < fc->sst_count; i++) {
		if (fc->fcs[i].data== NULL) 
			return 0;

		cmp = strcmp(key, fc->fcs[i].data);
		if (cmp < 0) 
			goto RET;
	}

RET:
	return _idx2count(i - 1);
}

void fc_free(struct fc *fc)
{
	free(fc->fcs);
	free(fc);
}
