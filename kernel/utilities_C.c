#include "../usrlib/syscall.h"
#include "../include/types.h"
#include "../include/const.h"
#include "types.h"
#include "const.h"
#include "external.h"
#include "prototype.h"

typedef struct _feature {
	tss_t *supplier;
	tss_t *waiter;
} feature_t;

static feature_t featureTable[sizeof(uint32_t)] = { {NULL, NULL} };
static tss_t *irqs[MAX_IRQ + 1] = { NULL };

/*int streq(char *s1, char *s2)
{
  char *cur1 = s1;
  char *cur2 = s2;
  for(;;) {
    if (*cur1 == '\0') {
      if (*cur2 == '\0') {
	return 1;
      }
      return 0;
    }
    if (*cur1 != *cur2) {
      return 0;
    }
    cur1++;
    cur2++;
  }
}*/

void paus()
{
  int i;
  for (i = 0; i < 0x0; i++) {
  }
}

void uptime()
{
  kputunsint(hours);
  kputstring(" hours ");
  kputunsint(minutes);
  kputstring(" minutes ");
  kputunsint(seconds);
  kputstring(" seconds.");
  kputchar(LF);
}

#define lastNeighBoursValid(p) FALSE

void enqueue(tss_t *process)
{
  if (process->misc & ENQUEUED) {
    return;
  }
  process->misc |= ENQUEUED;
  tss_t **cur;
  if (lastNeighBoursValid(process)) {
    if (process->previous != NULL) {
      process->previous->next = process;
    }
    if (process->next != NULL) {
      process->next->previous = process;
    }
  } else {
    cur = &(schedqueue.head);
    while ((*cur != NULL)&&((*cur)->schedticks > process->schedticks)) {
      cur = &((*cur)->next);
    }
    process->next = *cur;
    (*cur)->previous = process;
    *cur = process;
  }
  if (schedqueue.schedticks == 0) {
    schedqueue.schedticks = schedqueue.ticksleft = process->schedticks;
  }
  unsigned count1 = process->schedticks - ((schedqueue.schedticks -
    schedqueue.ticksleft) * process->schedticks / schedqueue.schedticks);
  unsigned count2 = process->ticksleft;
  process->ticksleft = count1 < count2 ? count1 : count2;
  schedqueue.schedticks += process->schedticks;
  schedqueue.ticksleft += process->ticksleft;
}

void showQ()
{
  tss_t *cur = NULL;
  kputstring("showing q"); kputchar(LF);
  for (cur = schedqueue.head; cur != NULL; cur = cur->next) {
    kputhex(cur); kputchar(' ');
  }
  kputchar(LF);
}

void chargeProc()
{
  if (curptr == &idlestate) {
    return;
  }
  curptr->ticksleft--;
  schedqueue.ticksleft--;
}

void dequeue(tss_t *process)
{
  if (!(process->misc & ENQUEUED)) {
    return;
  }
  process->misc &= (~(ENQUEUED));
  if (schedqueue.schedticks == process->schedticks) {
    schedqueue.schedticks = schedqueue.ticksleft = 0;
    schedqueue.head = NULL;
    pickNextProc();
    return;
  }
  if (schedqueue.head == NULL) {
    nextptr = &idlestate;
  } else if (curptr == process) {
    do {
      chargeProc();
      pickNextProc();
    } while (nextptr == process);
  } else {
  }
  if (process->previous != NULL) {
    process->previous->next = process->next;
  }
  if (process->next != NULL) {
    process->next->previous = process->previous;
  }
  if (schedqueue.head == process) {
    schedqueue.head = process->next;
  }
  schedqueue.schedticks -= process->schedticks;
  schedqueue.ticksleft -= process->ticksleft;
}

void showTSS(tss_t *tss)
{
  kputstring("backlink: "); kputhex(tss->backlink); kputchar(LF);
  kputstring("zeroes1: "); kputhex(tss->zeroes1); kputchar(' ');
  kputstring("esp0: "); kputhex(tss->esp0); kputchar(LF);
  kputstring("ss0: "); kputhex(tss->ss0); kputchar(' ');
  kputstring("zeroes2: "); kputhex(tss->zeroes2); kputchar(LF);
  kputstring("schedticks: "); kputhex(tss->schedticks); kputchar(' ');
  kputstring("state: "); kputhex(tss->state); kputchar(LF);
  kputstring("misc: "); kputhex(tss->misc); kputchar(' ');
  kputstring("zeroes3: "); kputhex(tss->zeroes3); kputchar(LF);
  kputstring("ticksleft: "); kputhex(tss->ticksleft); kputchar(' ');
  kputstring("pid: "); kputhex(tss->pid); kputchar(LF);
  kputstring("zeroes4: "); kputhex(tss->zeroes4); kputchar(' ');
  kputstring("cr3_reg: "); kputhex(tss->cr3_reg); kputchar(LF);
  kputstring("eip_reg: "); kputhex(tss->eip_reg); kputchar(' ');
  kputstring("eflag_reg: "); kputhex(tss->eflag_reg); kputchar(LF);
  kputstring("eax_reg: "); kputhex(tss->eax_reg); kputchar(' ');
  kputstring("ecx_reg: "); kputhex(tss->ecx_reg); kputchar(LF);
  kputstring("edx_reg: "); kputhex(tss->edx_reg); kputchar(' ');
  kputstring("ebx_reg: "); kputhex(tss->ebx_reg); kputchar(LF);
  kputstring("esp_reg: "); kputhex(tss->esp_reg); kputchar(' ');
  kputstring("ebp_reg: "); kputhex(tss->ebp_reg); kputchar(LF);
  kputstring("esi_reg: "); kputhex(tss->esi_reg); kputchar(' ');
  kputstring("edi_reg: "); kputhex(tss->edi_reg); kputchar(LF);
  kputstring("es_reg: "); kputhex(tss->es_reg); kputchar(' ');
  kputstring("zeroes5: "); kputhex(tss->zeroes5); kputchar(LF);
  kputstring("cs_reg: "); kputhex(tss->cs_reg); kputchar(' ');
  kputstring("zeroes6: "); kputhex(tss->zeroes6); kputchar(LF);
  kputstring("ss_reg: "); kputhex(tss->ss_reg); kputchar(' ');
  kputstring("zeroes7: "); kputhex(tss->zeroes7); kputchar(LF);
  kputstring("ds_reg: "); kputhex(tss->ds_reg); kputchar(' ');
  kputstring("zeroes8: "); kputhex(tss->zeroes8); kputchar(LF);
  kputstring("fs_reg: "); kputhex(tss->fs_reg); kputchar(' ');
  kputstring("zeroes9: "); kputhex(tss->zeroes9); kputchar(LF);
  kputstring("gs_reg: "); kputhex(tss->gs_reg); kputchar(' ');
  kputstring("zeroes10: "); kputhex(tss->zeroes10); kputchar(LF);
  kputstring("ldt_sel: "); kputhex(tss->ldt_sel); kputchar(' ');
  kputstring("zeroes11: "); kputhex(tss->zeroes11); kputchar(LF);
  kputstring("debug: "); kputhex(tss->debug); kputchar(' ');
  //kputstring("features: "); kputhex(tss->features); kputchar(LF);
  kputstring("iomap_base: "); kputhex(tss->iomap_base); kputchar(' ');
  kputstring("next: "); kputhex((uint32_t) tss->next); kputchar(LF);
  kputstring("idx_tss: "); kputhex(tss->idx_tss); kputchar(' ');
  kputstring("callMask: "); kputhex(tss->callMask); kputchar(LF);
  kputstring("previous: "); kputhex((uint32_t) tss->previous); kputchar(' ');
  kputstring("firstSender: "); kputhex((uint32_t) tss->firstSender); kputchar(LF);
  kputstring("nextSender: "); kputhex((uint32_t) tss->nextSender); kputchar(' ');
  kputstring("receivingFrom: "); kputhex((uint32_t) tss->receivingFrom); kputchar(LF);
}

#define vftNumerator(x) \
  (((x)->schedticks) - ((x)->ticksleft) + 1)

#define vtrrIsEligible(p,q) \
  (((q)->ticksleft > (p)->ticksleft) || \
  (((vftNumerator(q) * schedqueue.schedticks) - \
  (vftNumerator(&schedqueue) * q->schedticks)) < \
  schedqueue.schedticks))

void pickNextProc()
{
  if (schedqueue.head == NULL) {
    nextptr = &idlestate;
    goto finish;
  }
  tss_t *last = nextptr == NULL ? curptr : nextptr;
  if (last == NULL) {
    nextptr = schedqueue.head;
    goto finish;
  }
  if (last == &idlestate) {
    nextptr = schedqueue.head;
    goto finish;
  }
  if (last->next == NULL) {
	nextptr = schedqueue.head;
	if (schedqueue.head->ticksleft == 0) {
	  schedqueue.head->ticksleft = schedqueue.head->schedticks;
	}
	goto finish;
  }
  if (last->ticksleft == (last->schedticks - 1)) {
	nextptr = last->next;
	nextptr->ticksleft = nextptr->schedticks;
	goto finish;
  }
  if (vtrrIsEligible(last, (last->next))) {
	nextptr = last->next;
	goto finish;
  }
  nextptr = schedqueue.head;
finish:
//  showTSS(nextptr);
  return;
}

unsigned isPrimeNumber(unsigned what)
{
  int i;
  for (i = 2; i < what; i++) {
    if ((what % i) == 0) {
      return 1;
    }
  }
  return 0;
}
/*
void doTask1()
{
  char buf;
  int i;
  //for (i = 0; i < 10000; i++);
  disableInterrupts();
  kputstring("t1:hola");kputchar(LF);
  enableInterrupts();
  call_syscall_setFeature(1);
  disableInterrupts();
  kputstring("t1:set feature");kputchar(LF);
  enableInterrupts();
  for (i = 0; i < 100000; i++);
  kputstring("t1:will receive");kputchar(LF);
  call_syscall_receive(ANYPROC, &buf, 1);
  disableInterrupts();
  kputstring("t1:received: "); kputunsint(buf); kputchar(LF);
  enableInterrupts();
  for(;;) {
    halt();
  }
}

void doTask2()
{
  err_t error = OK;
  char buf = 78;
  tss_t *proc;
  int i;
  disableInterrupts();
  kputstring("t2:hola");kputchar(LF);
  enableInterrupts();
  for (i = 0; i < 100000; i++);
  kputstring("t2:will send by feature");kputchar(LF);
  call_syscall_send_by_feature(1,&buf,1,FALSE, &error);
  disableInterrupts();
  kputstring("t2:send!");kputchar(LF);
  enableInterrupts();
  for(;;) {
    halt();
  }
}*/

void writer()
{
  err_t error = OK;
  int i;
  for(;;) {
    call_syscall_send_by_feature(25,"Hallo Welt der Micokernel-Programmierung",40,TRUE, &error);
    if (error != OK) {
      kputstring("hat leider nicht geklappt!");kputchar(LF);
      error = OK;
    }
  }
}

unsigned validateDataArea(tss_t *process, void *mem, unsigned size)
{
  unsigned index = process->ds_reg & 0xfff8;
  //TODO: verify this!
  if (getLimit(index) < ((uint32_t) mem + size)) {
    return -1;
  }
  return 0;
}

tss_t *getSender(tss_t *desiredSender, tss_t *receiver)
{
	tss_t *result = NULL;
	if (receiver->firstSender == NULL) {
		result = NULL;
		goto finish;
	}
	if (desiredSender == ANYPROC) {
		result = receiver->firstSender;
		goto finish;
	}
	tss_t *cur = receiver->firstSender;
	for(;;) {
		if (cur == NULL) {
			result = NULL;
			goto finish;
		}
		if (cur == desiredSender) {
			result = desiredSender;
			goto finish;
		}
		cur = cur->nextSender;
	}
finish:
	return result;
}

boolean_t receivingFrom(tss_t *receiver, tss_t *sender)
{
	boolean_t result = FALSE;
	if (!(receiver->state & RECEIVING)) {
	  goto finish;
	}
	if (receiver->receivingFrom == ANYPROC) {
	  result = TRUE;
	  goto finish;
	}
	if ((receiver->receivingFrom) == sender) {
	  result = TRUE;
	  goto finish;
	}
finish:
	return result;
}

void removeFromSenderList(tss_t *sender, tss_t *receiver)
{
  tss_t *previous = NULL;
  tss_t *cur = receiver->firstSender;
  for(;;) {
    if (cur == sender) {
      break;
    }
    previous = cur;
    cur = cur->nextSender;
  }
  if (previous == NULL) {
    receiver->firstSender = sender->nextSender;
  } else {
    previous->nextSender = sender->nextSender;
  }
  sender->nextSender = NULL;
  unsetStateFlag(sender, SENDING);
}

void markAsReceivingFrom(tss_t *receiver, tss_t *sender)
{
  receiver->receivingFrom = sender;
  setStateFlag(receiver, RECEIVING);
}

void clearReceivingFrom(tss_t *receiver)
{
  receiver->receivingFrom = NULL;
  unsetStateFlag(receiver, RECEIVING);
}

void addToSenderList(tss_t *sender, tss_t *receiver)
{
  tss_t *previous = NULL;
  tss_t *cur = receiver->firstSender;
  while (cur != NULL) {
    previous = cur;
    cur = cur->nextSender;
  }
  if (previous != NULL) {
    previous->nextSender = sender;
  } else {
    receiver->firstSender = sender;
  }
  sender->nextSender = NULL;
  setStateFlag(sender, SENDING);
}

void add_waiting_for_feature(tss_t *process, uint32_t feature)
{
	tss_t **cur = NULL;
	uint32_t mask = 1;
	uint32_t idx = 0;
	while ((mask & feature) != 1) {
	  idx++;
	  mask <<= 1;
	}
	cur = &(featureTable[idx].waiter);
	while ((*cur) != NULL) {
	    cur = &((*cur)->nextSender);
	}
	(*cur) = process;
	process->nextSender = NULL;
}

int check_feature(uint32_t feature)
{
  uint8_t found = 0;
	uint32_t mask = 1;
	while (mask > 0) {
	    if ((feature & mask) != 0) {
	      found++;
	    }
	    if (found > 1) {
		break;
	    }
	    mask <<= 1;
	}

  return (found == 1) ? 0 : -1;
}

void set_feature(tss_t *context, tss_t *process, uint32_t feature)
{
	uint32_t mask = 1;
	uint32_t idx = 0;
	
	if (check_feature(feature) != 0) {
	      err = INVALIDFEATURE;
	      set_error(context, process);
	      goto finish;
	}
	
	while ((mask & feature) != 1) {
	  idx++;
	  mask <<= 1;
	}
	if (featureTable[idx].supplier == NULL) {
		featureTable[idx].supplier = process;
		process->firstSender = featureTable[idx].waiter;
		featureTable[idx].waiter = NULL;
	}
finish:
	return;
}

tss_t *getProcessByFeature(uint32_t feature)
{
	uint32_t mask = 1;
	uint32_t idx = 0;
	while ((mask & feature) != 1) {
	  idx++;
	  mask <<= 1;
	}
	return featureTable[idx].supplier;
}

void request_irq(tss_t *context, tss_t *process, uint32_t irq)
{
  if (irq > MAX_IRQ) {
    err = INVALIDIRQ;
    set_error(context, process);
    goto finish;
  }
  if (irqs[irq] != NULL) {
    err = IRQINUSE;
    set_error(context, process);
    goto finish;
  }
  irqs[irq] = process;
finish:
  return;
}
void release_irqs_taken(tss_t *process)
{
  int i;
  for (i = 0; i < MAX_IRQ; i++) {
    if (irqs[i] == process) {
      irqs[i] = NULL;
    }
  }
}

void unblock_waiting_senders(tss_t *context, tss_t *process)
{
  while (process->firstSender != NULL) {
    tss_t *cur = process->firstSender;
    err = PROCESSDIED;
    set_error(context, cur);
    unsetStateFlag(cur,SENDING);
    process->firstSender = cur->nextSender;
    cur->nextSender = NULL;
  }
}

//this may unblock the process
void do_hard_int(uint32_t number)
{
  //TODO
}