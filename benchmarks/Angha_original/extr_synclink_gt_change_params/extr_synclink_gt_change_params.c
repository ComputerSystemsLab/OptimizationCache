#define NULL ((void*)0)
typedef unsigned long size_t;  // Customize by platform.
typedef long intptr_t; typedef unsigned long uintptr_t;
typedef long scalar_t__;  // Either arithmetic or pointer type.
/* By default, we understand bool (as a convenience). */
typedef int bool;
#define false 0
#define true 1

/* Forward declarations */
typedef  struct TYPE_13__   TYPE_4__ ;
typedef  struct TYPE_12__   TYPE_3__ ;
typedef  struct TYPE_11__   TYPE_2__ ;
typedef  struct TYPE_10__   TYPE_1__ ;

/* Type definitions */
struct TYPE_12__ {TYPE_4__* tty; int /*<<< orphan*/  flags; } ;
struct TYPE_11__ {int data_bits; int stop_bits; int data_rate; int /*<<< orphan*/  parity; } ;
struct slgt_info {int timeout; int read_status_mask; int ignore_status_mask; TYPE_3__ port; TYPE_2__ params; int /*<<< orphan*/  signals; int /*<<< orphan*/  device_name; } ;
struct TYPE_13__ {TYPE_1__* termios; } ;
struct TYPE_10__ {unsigned int c_cflag; } ;

/* Variables and functions */
 int /*<<< orphan*/  ASYNC_CHECK_CD ; 
 int /*<<< orphan*/  ASYNC_CTS_FLOW ; 
 int /*<<< orphan*/  ASYNC_PARITY_EVEN ; 
 int /*<<< orphan*/  ASYNC_PARITY_NONE ; 
 int /*<<< orphan*/  ASYNC_PARITY_ODD ; 
 unsigned int CBAUD ; 
 unsigned int CLOCAL ; 
 unsigned int CRTSCTS ; 
#define  CS5 131 
#define  CS6 130 
#define  CS7 129 
#define  CS8 128 
 unsigned int CSIZE ; 
 unsigned int CSTOPB ; 
 int /*<<< orphan*/  DBGINFO (char*) ; 
 int HZ ; 
 int IRQ_RXOVER ; 
 scalar_t__ I_BRKINT (TYPE_4__*) ; 
 scalar_t__ I_IGNBRK (TYPE_4__*) ; 
 scalar_t__ I_IGNPAR (TYPE_4__*) ; 
 scalar_t__ I_INPCK (TYPE_4__*) ; 
 scalar_t__ I_PARMRK (TYPE_4__*) ; 
 int MASK_BREAK ; 
 int MASK_FRAMING ; 
 int MASK_OVERRUN ; 
 int MASK_PARITY ; 
 unsigned int PARENB ; 
 unsigned int PARODD ; 
 int /*<<< orphan*/  SerialSignal_DTR ; 
 int /*<<< orphan*/  SerialSignal_RTS ; 
 int /*<<< orphan*/  program_hw (struct slgt_info*) ; 
 int tty_get_baud_rate (TYPE_4__*) ; 

__attribute__((used)) static void change_params(struct slgt_info *info)
{
	unsigned cflag;
	int bits_per_char;

	if (!info->port.tty || !info->port.tty->termios)
		return;
	DBGINFO(("%s change_params\n", info->device_name));

	cflag = info->port.tty->termios->c_cflag;

	/* if B0 rate (hangup) specified then negate DTR and RTS */
	/* otherwise assert DTR and RTS */
 	if (cflag & CBAUD)
		info->signals |= SerialSignal_RTS + SerialSignal_DTR;
	else
		info->signals &= ~(SerialSignal_RTS + SerialSignal_DTR);

	/* byte size and parity */

	switch (cflag & CSIZE) {
	case CS5: info->params.data_bits = 5; break;
	case CS6: info->params.data_bits = 6; break;
	case CS7: info->params.data_bits = 7; break;
	case CS8: info->params.data_bits = 8; break;
	default:  info->params.data_bits = 7; break;
	}

	info->params.stop_bits = (cflag & CSTOPB) ? 2 : 1;

	if (cflag & PARENB)
		info->params.parity = (cflag & PARODD) ? ASYNC_PARITY_ODD : ASYNC_PARITY_EVEN;
	else
		info->params.parity = ASYNC_PARITY_NONE;

	/* calculate number of jiffies to transmit a full
	 * FIFO (32 bytes) at specified data rate
	 */
	bits_per_char = info->params.data_bits +
			info->params.stop_bits + 1;

	info->params.data_rate = tty_get_baud_rate(info->port.tty);

	if (info->params.data_rate) {
		info->timeout = (32*HZ*bits_per_char) /
				info->params.data_rate;
	}
	info->timeout += HZ/50;		/* Add .02 seconds of slop */

	if (cflag & CRTSCTS)
		info->port.flags |= ASYNC_CTS_FLOW;
	else
		info->port.flags &= ~ASYNC_CTS_FLOW;

	if (cflag & CLOCAL)
		info->port.flags &= ~ASYNC_CHECK_CD;
	else
		info->port.flags |= ASYNC_CHECK_CD;

	/* process tty input control flags */

	info->read_status_mask = IRQ_RXOVER;
	if (I_INPCK(info->port.tty))
		info->read_status_mask |= MASK_PARITY | MASK_FRAMING;
 	if (I_BRKINT(info->port.tty) || I_PARMRK(info->port.tty))
 		info->read_status_mask |= MASK_BREAK;
	if (I_IGNPAR(info->port.tty))
		info->ignore_status_mask |= MASK_PARITY | MASK_FRAMING;
	if (I_IGNBRK(info->port.tty)) {
		info->ignore_status_mask |= MASK_BREAK;
		/* If ignoring parity and break indicators, ignore
		 * overruns too.  (For real raw support).
		 */
		if (I_IGNPAR(info->port.tty))
			info->ignore_status_mask |= MASK_OVERRUN;
	}

	program_hw(info);
}