#define NULL ((void*)0)
typedef unsigned long size_t;  // Customize by platform.
typedef long intptr_t; typedef unsigned long uintptr_t;
typedef long scalar_t__;  // Either arithmetic or pointer type.
/* By default, we understand bool (as a convenience). */
typedef int bool;
#define false 0
#define true 1

/* Forward declarations */
typedef  struct TYPE_10__   TYPE_5__ ;
typedef  struct TYPE_9__   TYPE_4__ ;
typedef  struct TYPE_8__   TYPE_3__ ;
typedef  struct TYPE_7__   TYPE_2__ ;
typedef  struct TYPE_6__   TYPE_1__ ;

/* Type definitions */
struct TYPE_9__ {int* data_ptr; } ;
struct TYPE_8__ {int status; scalar_t__ target_id; } ;
union ccb {TYPE_4__ ataio; TYPE_3__ ccb_h; } ;
typedef  int uint8_t ;
struct siis_channel {int /*<<< orphan*/  numhslots; TYPE_5__** hold; scalar_t__ recoverycmd; } ;
struct ata_res {int status; int error; int lba_low; int lba_mid; int lba_high; int device; int lba_low_exp; int lba_mid_exp; int lba_high_exp; int sector_count; int sector_count_exp; } ;
typedef  int /*<<< orphan*/  device_t ;
struct TYPE_7__ {scalar_t__ target_id; int status; } ;
struct TYPE_6__ {struct ata_res res; } ;
struct TYPE_10__ {TYPE_2__ ccb_h; TYPE_1__ ataio; } ;

/* Variables and functions */
 int CAM_REQUEUE_REQ ; 
 int CAM_REQ_CMP ; 
 int CAM_STATUS_MASK ; 
 int /*<<< orphan*/  M_SIIS ; 
 int SIIS_MAX_SLOTS ; 
 struct siis_channel* device_get_softc (int /*<<< orphan*/ ) ; 
 int /*<<< orphan*/  device_printf (int /*<<< orphan*/ ,char*) ; 
 int /*<<< orphan*/  free (int*,int /*<<< orphan*/ ) ; 
 int /*<<< orphan*/  xpt_done (TYPE_5__*) ; 
 int /*<<< orphan*/  xpt_free_ccb (union ccb*) ; 

__attribute__((used)) static void
siis_process_read_log(device_t dev, union ccb *ccb)
{
	struct siis_channel *ch = device_get_softc(dev);
	uint8_t *data;
	struct ata_res *res;
	int i;

	ch->recoverycmd = 0;
	data = ccb->ataio.data_ptr;
	if ((ccb->ccb_h.status & CAM_STATUS_MASK) == CAM_REQ_CMP &&
	    (data[0] & 0x80) == 0) {
		for (i = 0; i < SIIS_MAX_SLOTS; i++) {
			if (!ch->hold[i])
				continue;
			if (ch->hold[i]->ccb_h.target_id != ccb->ccb_h.target_id)
				continue;
			if ((data[0] & 0x1F) == i) {
				res = &ch->hold[i]->ataio.res;
				res->status = data[2];
				res->error = data[3];
				res->lba_low = data[4];
				res->lba_mid = data[5];
				res->lba_high = data[6];
				res->device = data[7];
				res->lba_low_exp = data[8];
				res->lba_mid_exp = data[9];
				res->lba_high_exp = data[10];
				res->sector_count = data[12];
				res->sector_count_exp = data[13];
			} else {
				ch->hold[i]->ccb_h.status &= ~CAM_STATUS_MASK;
				ch->hold[i]->ccb_h.status |= CAM_REQUEUE_REQ;
			}
			xpt_done(ch->hold[i]);
			ch->hold[i] = NULL;
			ch->numhslots--;
		}
	} else {
		if ((ccb->ccb_h.status & CAM_STATUS_MASK) != CAM_REQ_CMP)
			device_printf(dev, "Error while READ LOG EXT\n");
		else if ((data[0] & 0x80) == 0) {
			device_printf(dev, "Non-queued command error in READ LOG EXT\n");
		}
		for (i = 0; i < SIIS_MAX_SLOTS; i++) {
			if (!ch->hold[i])
				continue;
			if (ch->hold[i]->ccb_h.target_id != ccb->ccb_h.target_id)
				continue;
			xpt_done(ch->hold[i]);
			ch->hold[i] = NULL;
			ch->numhslots--;
		}
	}
	free(ccb->ataio.data_ptr, M_SIIS);
	xpt_free_ccb(ccb);
}