#define NULL ((void*)0)
typedef unsigned long size_t;  // Customize by platform.
typedef long intptr_t; typedef unsigned long uintptr_t;
typedef long scalar_t__;  // Either arithmetic or pointer type.
/* By default, we understand bool (as a convenience). */
typedef int bool;
#define false 0
#define true 1

/* Forward declarations */
typedef  struct TYPE_8__   TYPE_4__ ;
typedef  struct TYPE_7__   TYPE_3__ ;
typedef  struct TYPE_6__   TYPE_2__ ;
typedef  struct TYPE_5__   TYPE_1__ ;

/* Type definitions */
struct super_block {int dummy; } ;
struct TYPE_7__ {TYPE_2__* external; TYPE_1__* internal; } ;
struct bplus_header {int n_free_nodes; int n_used_nodes; int first_free; TYPE_3__ u; scalar_t__ internal; } ;
struct fnode {struct bplus_header btree; } ;
struct buffer_head {int dummy; } ;
struct anode {struct bplus_header btree; } ;
typedef  unsigned int secno ;
typedef  unsigned int anode_secno ;
struct TYPE_8__ {scalar_t__ sb_chk; } ;
struct TYPE_6__ {unsigned int file_secno; unsigned int length; unsigned int disk_secno; } ;
struct TYPE_5__ {unsigned int file_secno; unsigned int down; } ;

/* Variables and functions */
 int /*<<< orphan*/  brelse (struct buffer_head*) ; 
 int /*<<< orphan*/  hpfs_ea_remove (struct super_block*,unsigned int,int,int /*<<< orphan*/ ) ; 
 int /*<<< orphan*/  hpfs_error (struct super_block*,char*,unsigned int,...) ; 
 int /*<<< orphan*/  hpfs_free_sectors (struct super_block*,unsigned int,unsigned int) ; 
 struct anode* hpfs_map_anode (struct super_block*,unsigned int,struct buffer_head**) ; 
 struct fnode* hpfs_map_fnode (struct super_block*,unsigned int,struct buffer_head**) ; 
 int /*<<< orphan*/  hpfs_remove_btree (struct super_block*,struct bplus_header*) ; 
 TYPE_4__* hpfs_sb (struct super_block*) ; 
 scalar_t__ hpfs_stop_cycles (struct super_block*,unsigned int,int*,int*,char*) ; 
 int /*<<< orphan*/  mark_buffer_dirty (struct buffer_head*) ; 

void hpfs_truncate_btree(struct super_block *s, secno f, int fno, unsigned secs)
{
	struct fnode *fnode;
	struct anode *anode;
	struct buffer_head *bh;
	struct bplus_header *btree;
	anode_secno node = f;
	int i, j, nodes;
	int c1, c2 = 0;
	if (fno) {
		if (!(fnode = hpfs_map_fnode(s, f, &bh))) return;
		btree = &fnode->btree;
	} else {
		if (!(anode = hpfs_map_anode(s, f, &bh))) return;
		btree = &anode->btree;
	}
	if (!secs) {
		hpfs_remove_btree(s, btree);
		if (fno) {
			btree->n_free_nodes = 8;
			btree->n_used_nodes = 0;
			btree->first_free = 8;
			btree->internal = 0;
			mark_buffer_dirty(bh);
		} else hpfs_free_sectors(s, f, 1);
		brelse(bh);
		return;
	}
	while (btree->internal) {
		nodes = btree->n_used_nodes + btree->n_free_nodes;
		for (i = 0; i < btree->n_used_nodes; i++)
			if (btree->u.internal[i].file_secno >= secs) goto f;
		brelse(bh);
		hpfs_error(s, "internal btree %08x doesn't end with -1", node);
		return;
		f:
		for (j = i + 1; j < btree->n_used_nodes; j++)
			hpfs_ea_remove(s, btree->u.internal[j].down, 1, 0);
		btree->n_used_nodes = i + 1;
		btree->n_free_nodes = nodes - btree->n_used_nodes;
		btree->first_free = 8 + 8 * btree->n_used_nodes;
		mark_buffer_dirty(bh);
		if (btree->u.internal[i].file_secno == secs) {
			brelse(bh);
			return;
		}
		node = btree->u.internal[i].down;
		brelse(bh);
		if (hpfs_sb(s)->sb_chk)
			if (hpfs_stop_cycles(s, node, &c1, &c2, "hpfs_truncate_btree"))
				return;
		if (!(anode = hpfs_map_anode(s, node, &bh))) return;
		btree = &anode->btree;
	}	
	nodes = btree->n_used_nodes + btree->n_free_nodes;
	for (i = 0; i < btree->n_used_nodes; i++)
		if (btree->u.external[i].file_secno + btree->u.external[i].length >= secs) goto ff;
	brelse(bh);
	return;
	ff:
	if (secs <= btree->u.external[i].file_secno) {
		hpfs_error(s, "there is an allocation error in file %08x, sector %08x", f, secs);
		if (i) i--;
	}
	else if (btree->u.external[i].file_secno + btree->u.external[i].length > secs) {
		hpfs_free_sectors(s, btree->u.external[i].disk_secno + secs -
			btree->u.external[i].file_secno, btree->u.external[i].length
			- secs + btree->u.external[i].file_secno); /* I hope gcc optimizes this :-) */
		btree->u.external[i].length = secs - btree->u.external[i].file_secno;
	}
	for (j = i + 1; j < btree->n_used_nodes; j++)
		hpfs_free_sectors(s, btree->u.external[j].disk_secno, btree->u.external[j].length);
	btree->n_used_nodes = i + 1;
	btree->n_free_nodes = nodes - btree->n_used_nodes;
	btree->first_free = 8 + 12 * btree->n_used_nodes;
	mark_buffer_dirty(bh);
	brelse(bh);
}