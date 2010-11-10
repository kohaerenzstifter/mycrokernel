#include "../usrlib/syscall.h"
#include "../usrlib/prototype.h"
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

typedef void (*syscallFunc_t)(void) ;

void syscall_exit(void);
void syscall_set_feature(void);
void syscall_receive(void);
void syscall_send_by_feature(void);
void syscall_request_irq(void);
void syscall_inb(void);
void syscall_claim_port(void);
void syscall_outb(void);
void syscall_inw(void);
void syscall_outw(void);
void syscall_vir2phys(void);

static syscallFunc_t syscalls[] = {
  &syscall_exit,
  &syscall_set_feature,
  &syscall_receive,
  &syscall_send_by_feature,
  &syscall_request_irq,
  &syscall_inb,
  &syscall_claim_port,
  &syscall_outb,
  &syscall_inw,
  &syscall_outw,
  &syscall_vir2phys
};

static feature_t featureTable[sizeof(uint32_t)] = { {NULL, NULL} };
static tss_t *irqs[MAX_IRQ + 1] = { NULL };

static tss_t *port_accessers[NUM_PORTS];

extern tss_t intrrpt0state;
extern tss_t intrrpt1state;
extern tss_t intrrpt2state;
extern tss_t intrrpt3state;
extern tss_t intrrpt4state;
extern tss_t intrrpt5state;
extern tss_t intrrpt6state;
extern tss_t intrrpt7state;
extern tss_t intrrpt8state;
extern tss_t intrrpt9state;
extern tss_t intrrpt10state;
extern tss_t intrrpt11state;
extern tss_t intrrpt12state;
extern tss_t intrrpt13state;
extern tss_t intrrpt14state;
extern tss_t intrrpt15state;

#define NO_SYSCALLS (sizeof(syscalls) / sizeof(syscalls[0]))
#define syscallstate intrrpt2state

void init_port_accessers()
{
  int i = 0;
  for (i = 0; i < NUM_PORTS; i++) {
    port_accessers[i] = NULL;
  }
}

#define kputstring_if(c) \
  { \
    if (c) { \
      kputstring("(conditional) " __FILE__); kputstring(":"); kputunsint(__LINE__); kputchar(LF); \
    } \
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

//TODO
#define lastNeighBoursValid(p) FALSE

void enqueue(tss_t *process)
{
  tss_t *previous = NULL;

  if (process->misc & ENQUEUED) {
    goto finish;
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
    process->previous = NULL;
    process->next = NULL;
    cur = &(schedqueue.head);
    
    while ((*cur != NULL)&&((*cur)->schedticks > process->schedticks)) {
      previous = *cur;
      cur = &((*cur)->next);
    }

    process->next = *cur;
    process->previous = previous;

    if ((*cur) != NULL) {
      (*cur)->previous = process;
    }
    *cur = process;
  }

  if (schedqueue.schedticks == 0) {
    process->ticksleft = process->schedticks;
  } else {
    unsigned count1 = process->schedticks - ((schedqueue.schedticks -
      schedqueue.ticksleft) * process->schedticks / schedqueue.schedticks);
    unsigned count2 = process->ticksleft;
    process->ticksleft = count1 < count2 ? count1 : count2;
  }
  schedqueue.schedticks += process->schedticks;
  schedqueue.ticksleft += process->ticksleft;

finish:
  return;
}

void zerify(char *address, uint32_t size)
{
  uint32_t i;
  for (i = 0; i < size; i++) {
    address[i] = 0;
  }
}

void show_queue()
{
  tss_t *cur = NULL;
  kputstring("showing q"); kputchar(LF);
  for (cur = schedqueue.head; cur != NULL; cur = cur->next) {
    kputstring("process "); kputstring(cur->procname); kputstring(" ");
  }
  kputchar(LF);
}

void charge_process()
{
  if (curptr == &idlestate) {
    return;
  }
  curptr->ticksleft--;
  schedqueue.ticksleft--;
}

#define vft_numerator(x) \
  (((x)->schedticks) - ((x)->ticksleft) + 1)

#define vtrr_is_eligible(p,q) \
  (((q)->ticksleft > (p)->ticksleft) || \
  (((vft_numerator(q) * schedqueue.schedticks) - \
  (vft_numerator(&schedqueue) * q->schedticks)) < \
  schedqueue.schedticks))

void pick_next_proc()
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
    if (nextptr->ticksleft == 0) {
      nextptr->ticksleft = nextptr->schedticks;
    }
    goto finish;
  }
  if (last->ticksleft == (last->schedticks - 1)) {
    nextptr = last->next;
    nextptr->ticksleft = nextptr->schedticks;
    goto finish;
  }
  if (vtrr_is_eligible(last, (last->next))) {
    nextptr = last->next;
    goto finish;
  }
  nextptr = schedqueue.head;
  if (nextptr->ticksleft == 0) {
    nextptr->ticksleft = nextptr->schedticks;
  }
finish:
  return;
}

void dequeue(tss_t *process)
{
  if (!(process->misc & ENQUEUED)) {
    goto finish;
  }
  process->misc &= (~(ENQUEUED));
  if (schedqueue.schedticks == process->schedticks) {
    schedqueue.schedticks = schedqueue.ticksleft = 0;
    schedqueue.head = NULL;
    goto finish;
  }
  if (curptr == process) {
    do {
      charge_process();
      pick_next_proc();
      curptr = nextptr;
    } while (nextptr == process);
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

finish:
  return;
}

void show_tss(tss_t *tss)
{
  kputstring("backlink: "); kputhex(tss->backlink); kputchar(LF);
  kputstring("interrupts: "); kputhex(tss->interrupts); kputchar(' ');
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
  kputstring("iomap_base: "); kputhex(tss->iomap_base); kputchar(' ');
  kputstring("next: "); kputhex((uint32_t) tss->next); kputchar(LF);
  kputstring("idx_tss: "); kputhex(tss->idx_tss); kputchar(' ');
  kputstring("callMask: "); kputhex(tss->callMask); kputchar(LF);
  kputstring("previous: "); kputhex((uint32_t) tss->previous); kputchar(' ');
  kputstring("firstSender: "); kputhex((uint32_t) tss->firstSender); kputchar(LF);
  kputstring("nextSender: "); kputhex((uint32_t) tss->nextSender); kputchar(' ');
  kputstring("receivingFrom: "); kputhex((uint32_t) tss->receivingFrom); kputchar(LF);
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

void writer()
{
  int i;
  err_t err = OK;
  err_t *error = &err;

  for(;;) {
    for (i = 0; i < 100000; i++);
    call_syscall_send_by_feature(1,"Hallo Welt der Microkernel-Programmierung",41, FALSE, error);
    if (error != OK) {
      disable_interrupts();
      //kputstring("hat leider nicht geklappt: "); kputhex(error); kputchar(LF);
      enable_interrupts();
      error = OK;
    } else {
      disable_interrupts();
     // kputstring("juchhu, hat geklappt"); kputchar(LF);
      enable_interrupts();
    }
  }
}

unsigned validate_data_area(tss_t *process, void *mem, unsigned size)
{
  uint32_t index = process->ds_reg & 0xfff8;
  //TODO: verify this!
  if (get_limit(index) < ((uint32_t) mem + size)) {
    return -1;
  }
  return 0;
}

tss_t *get_sender(tss_t *desiredSender, tss_t *receiver)
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

boolean_t receiving_from(tss_t *receiver, tss_t *sender)
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

void remove_from_senders_list(tss_t *sender, tss_t *receiver)
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

void mark_as_receiving_from(tss_t *receiver, tss_t *sender)
{
  receiver->receivingFrom = sender;
  setStateFlag(receiver, RECEIVING);
}

void clear_receiving_from(tss_t *receiver)
{
  receiver->receivingFrom = NULL;
  unsetStateFlag(receiver, RECEIVING);
}

void add_to_senders_list(tss_t *sender, tss_t *receiver)
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
  while ((mask & feature) != mask) {
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

int set_feature(tss_t *process, uint32_t feature)
{
  uint32_t mask = 1;
  uint32_t idx = 0;
  int result = 1;
  while ((mask & feature) != mask) {
    idx++;
    mask <<= 1;
  }
  if (featureTable[idx].supplier != NULL) {
    goto finish;
  }
  if (featureTable[idx].supplier == NULL) {
    featureTable[idx].supplier = process;
    process->firstSender = featureTable[idx].waiter;
    featureTable[idx].waiter = NULL;
    result = 0;
  }
finish:
  return result;
}

tss_t *get_process_by_feature(uint32_t feature)
{
  uint32_t mask = 1;
  uint32_t idx = 0;
  while ((mask & feature) != mask) {
    idx++;
    mask <<= 1;
  }
  return featureTable[idx].supplier;
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
    set_err(cur, PROCESSDIED);
    unsetStateFlag(cur,SENDING);
    process->firstSender = cur->nextSender;
    cur->nextSender = NULL;
  }
}

void syscall_exit(void)
{
  curptr->state = EMPTY;
  if (curptr->cs_reg != 0) {
    free_gdt_idx(curptr->cs_reg & 0xfffc);
  }
  if (curptr->ds_reg != 0) {
    free_gdt_idx(curptr->ds_reg & 0xfffc);
  }
  if (curptr->es_reg != 0) {
    free_gdt_idx(curptr->es_reg & 0xfffc);
  }
  if (curptr->fs_reg != 0) {
    free_gdt_idx(curptr->fs_reg & 0xfffc);
  }
  if (curptr->gs_reg != 0) {
    free_gdt_idx(curptr->gs_reg & 0xfffc);
  }
  if (curptr->ss_reg != 0) {
    free_gdt_idx(curptr->ss_reg & 0xfffc);
  }
  free_pid(curptr->pid);
  release_irqs_taken(curptr);
  unblock_waiting_senders(&syscallstate, curptr);
  dequeue(curptr);
}

void syscall_send(void)
{
  tss_t *receiver = (tss_t *) curptr->ebx_reg;

  uint32_t bytes = curptr->ecx_reg;
  void *addr = (void *) curptr->edx_reg;
  if (validate_data_area(curptr,addr,bytes) != 0) {
    set_err(curptr, INVALIDBUFFER);
    goto finish;
  }
  if (receiving_from(receiver,curptr)) {
    bytes = exchange_data(curptr,receiver);
    curptr->eax_reg = bytes;
    receiver->eax_reg = bytes;
    clear_receiving_from(receiver);
    set_err(curptr, OK);
    set_err(receiver, OK);
    goto finish;
  }
  add_to_senders_list(curptr,receiver);
finish:
  return;
}


void syscall_send_by_feature(void)
{
  tss_t *receiver = NULL;
  uint32_t feature = curptr->ebx_reg;

  if (check_feature(feature) != 0) {
    set_err(curptr, INVALIDFEATURE);
    goto finish;
  }
  receiver = get_process_by_feature(feature);
  if (receiver == curptr) {
    set_err(curptr, CIRCULARSEND);
    goto finish;
  }
  if (receiver != NULL) {
    curptr->ebx_reg = (uint32_t) receiver;
    syscall_send();
    goto finish;
  }
  if (curptr->edi_reg == TRUE) {
    add_waiting_for_feature(curptr, feature);
    dequeue(curptr);
    goto finish;
  }
  set_err(curptr, WOULDBLOCK);
  goto finish;
  
finish:
  return;
}

void syscall_receive(void)
{
  tss_t *sender = (tss_t *) curptr->ebx_reg;
  tss_t *desired_sender = (tss_t *) curptr->ebx_reg;
  uint32_t bytes = curptr->ecx_reg;
  void *addr = (void *) curptr->edx_reg;

  if (validate_data_area(curptr,addr,bytes) != 0) {
    set_err(curptr, INVALIDBUFFER);
    goto finish;
  }
  
  if (curptr->interrupts != 0) {
    set_err(curptr, curptr->interrupts);
    curptr->interrupts = 0;
    curptr->eax_reg = 0;
    goto finish;
  }

  if ((sender = get_sender(desired_sender, curptr)) != NULL) {
    bytes = exchange_data(sender,curptr);
    curptr->eax_reg = bytes;
    sender->eax_reg = bytes;
    remove_from_senders_list(sender,curptr);
    set_err(sender, OK);
    set_err(curptr, OK);
    goto finish;
  }

  mark_as_receiving_from(curptr, desired_sender);

finish:
  return;
}

void syscall_set_feature(void)
{
  uint32_t feature = curptr->ebx_reg;
  if (check_feature(feature) != 0) {
    set_err(curptr, INVALIDFEATURE);
    goto finish;
  }
  if (set_feature(curptr, feature) != 0) {
    set_err(curptr, FEATUREBUSY);
    goto finish;
  }
  set_err(curptr, OK);
finish:
  return;
}

#define has_port_access(process, port) (port_accessers[port] == process)

void syscall_inb(void)
{
  uint32_t port = curptr->ebx_reg & 0xffff;

  if (!has_port_access(curptr, port)) {
    set_err(curptr, NOPERMS);
    goto finish;
  }
  curptr->eax_reg = inb(port);
  set_err(curptr, OK);
finish:
  return;
}

void syscall_inw(void)
{
  uint32_t port = curptr->ebx_reg & 0xffff;

  if (!has_port_access(curptr, port)) {
    set_err(curptr, NOPERMS);
    goto finish;
  }
  curptr->eax_reg = inw(port);
  set_err(curptr, OK);
finish:
  return;
}

void syscall_outb(void)
{
  uint32_t port = curptr->ebx_reg & 0xffff;
  uint32_t value = curptr->ecx_reg & 0xff;

  if (!has_port_access(curptr, port)) {
    set_err(curptr, NOPERMS);
    goto finish;
  }
  outb(port, value);
  set_err(curptr, OK);
finish:
  return;
}

void syscall_outw(void)
{
  uint32_t port = curptr->ebx_reg & 0xffff;
  uint32_t value = curptr->ecx_reg & 0xffff;

  if (!has_port_access(curptr, port)) {
    set_err(curptr, NOPERMS);
    goto finish;
  }
  outw(port, value);
  set_err(curptr, OK);
finish:
  return;
}

static uint32_t vir2phys(tss_t *process, uint32_t virtual)
{
  uint32_t index = process->ds_reg & 0xfff8;
  uint32_t base = get_base(index);
  return (base + virtual);
}

void syscall_vir2phys(void)
{
  if (validate_data_area(curptr, (void *) curptr->ebx_reg, 0) != 0) {
    set_err(curptr, INVALIDADDRESS);
    goto finish;
  }
  curptr->eax_reg = vir2phys(curptr, curptr->ebx_reg);

finish:
  return;
}

void syscall_claim_port(void)
{
  uint32_t port = curptr->ebx_reg & 0xffff;

  if ((port_accessers[port] != NULL)&&(port_accessers[port] != curptr)) {
    set_err(curptr, UNAVAILABLE);
    goto finish;
  }
  port_accessers[port] = curptr;
finish:
  return;
}

void syscall_request_irq(void)
{
  
  uint32_t irq = curptr->ebx_reg;
  if ((irq == 0)||(irq == 2)||(irq > MAX_IRQ)) {
    set_err(curptr, INVALIDIRQ);
    goto finish;
  }
  if ((irqs[irq] != NULL)&&(irqs[irq] != curptr)) {
    set_err(curptr, IRQINUSE);
    goto finish;
  }
  irqs[irq] = curptr;
  set_err(curptr, OK);
finish:
  return;
}

void clockIsr()
{
  seconds++;
  if (seconds == 60) {
    seconds = 0;
    minutes++;
    if (minutes == 60) {
      minutes = 0;
      hours++;
    }
  }
}

void syscallIsr()
{
  if (curptr->eax_reg > NO_SYSCALLS - 1) {
    set_err(curptr, UNKNOWNSYSCALL);
    goto finish;
  }
  syscalls[curptr->eax_reg]();
finish:
  return;
}

tss_t *interrupt_states[16] = {
  &intrrpt0state,
  &intrrpt1state,
  &intrrpt2state,
  &intrrpt3state,
  &intrrpt4state,
  &intrrpt5state,
  &intrrpt6state,
  &intrrpt7state,
  &intrrpt8state,
  &intrrpt9state,
  &intrrpt0state,
  &intrrpt11state,
  &intrrpt12state,
  &intrrpt13state,
  &intrrpt14state,
  &intrrpt15state
};

void notify_irq(uint32_t number)
{
  uint16_t mask = (1 << number);

  if (irqs[number]->state & RECEIVING) {
    set_err(irqs[number], mask);
    clear_receiving_from(irqs[number]);
    irqs[number]->eax_reg = 0;
    goto finish;
  }
  irqs[number]->interrupts |= mask;

finish:
  return;
}

//this may unblock the process
void do_hard_int(uint32_t number)
{
  if (number == 0) {
    clockIsr();
    goto finish;
  }
  if (number == 2) {
    syscallIsr();
    goto finish;
  }
  //kputstring("INTERRUPT: "); kputchar(LF); kputunsint(number); halt();
  if (irqs[number] == NULL) {
    goto finish;
  }
  notify_irq(number);
finish:
  if (nextptr == NULL) {
    charge_process();
    pick_next_proc();
  }
  return;
}

void do_exception(uint32_t number, uint32_t error)
{
  kputstring("exception: "); kputhex(number); kputchar(LF);
  kputstring("error: "); kputhex(error); kputchar(LF);
  kputstring("flag: "); kputunsint(flag); kputchar(LF);
  kputstring("curptr->procname: "); kputstring(curptr->procname); kputchar(LF);
  kputstring("curptr->eip: "); kputunsint(curptr->eip_reg); kputchar(LF);
  kputstring("curptr->eip: (hex)"); kputhex(curptr->eip_reg); kputchar(LF);
  kputstring("curptr->esp: "); kputunsint(curptr->esp_reg); kputchar(LF);
  kputstring("curptr->esp: (hex)"); kputhex(curptr->esp_reg); kputchar(LF);
  show_queue();
  halt();
}

static boolean_t memAreaValid(uint32_t base, uint32_t limit, boolean_t video)
{
  uint32_t lowerBound = video ? 0xc0000 : 0xa0000;
  boolean_t result = FALSE;
  if ((base > lowerBound)&&(base < 0x100000)) {
    //this region cannot be used
    goto finish;
  }
  if ((base <= lowerBound)&&((base + limit) > lowerBound)) {
    goto finish;
  }
  //TODO: add further checks here...
  result = TRUE;
finish:
  return result;
}

tss_t *create_process(uint32_t privilege, uint32_t schedticks, uint32_t start,
  uint32_t csegmBase, uint32_t dsegmBase, uint32_t esegmBase,
  uint32_t fsegmBase, uint32_t gsegmBase, uint32_t ssegmBase,
  uint32_t csegmLimit, uint32_t dsegmLimit, uint32_t esegmLimit,
  uint32_t fsegmLimit, uint32_t gsegmLimit, uint32_t ssegmLimit,
  char *name)
{
  uint32_t limit = 0;
  tss_t *result = NULL;
  uint32_t idx = 0;
  uint32_t tssIdx = INVALID_GDT_IDX;
  uint32_t csegmIdx = INVALID_GDT_IDX;
  uint32_t dsegmIdx = INVALID_GDT_IDX;
  uint32_t esegmIdx = INVALID_GDT_IDX;
  uint32_t fsegmIdx = INVALID_GDT_IDX;
  uint32_t gsegmIdx = INVALID_GDT_IDX;
  uint32_t ssegmIdx = INVALID_GDT_IDX;
  tss_t *process = NULL;

  for (;;) {
    process = pproctab[idx];
    if (process == pproctab[MAX_NO_PROCS]) {
      goto finish;
    }
    if (process->state == EMPTY) {
      break;
    }
    idx++;
  }
  if (csegmBase != NO_BASE) {
    if (!memAreaValid(csegmBase, csegmLimit, FALSE)) {
      goto finish;
    }
    csegmIdx = get_free_gdt_idx();
    if (csegmIdx == INVALID_GDT_IDX) {
      goto finish;
    }
    if (csegmLimit > 0xfffff) {
      limit = csegmLimit;
      limit &= 0x3ff;
      if (limit == 0) {
	limit = (csegmLimit >> 12);
      } else {
	limit = (csegmLimit >> 12) + 1;
      }
      write_segment_desc(csegmIdx, limit, csegmBase, TRUE, privilege, CODE, PAGE_GRANULARITY);
    } else {
      write_segment_desc(csegmIdx, csegmLimit, csegmBase, TRUE, privilege, CODE, BYTE_GRANULARITY);      
    }
  }
  if (dsegmBase != NO_BASE) {
    if (!memAreaValid(dsegmBase, dsegmLimit, FALSE)) {
      goto finish;
    }
    dsegmIdx = get_free_gdt_idx();
    if (dsegmIdx == INVALID_GDT_IDX) {
      goto finish;
    }
    if (dsegmLimit > 0xfffff) {
      limit = dsegmLimit;
      limit &= 0x3ff;
      if (limit == 0) {
	limit = (dsegmLimit >> 12);
      } else {
	limit = (dsegmLimit >> 12) + 1;
      }
      write_segment_desc(dsegmIdx, limit, dsegmBase, TRUE, privilege, DATA, PAGE_GRANULARITY);
    } else {
      write_segment_desc(dsegmIdx, dsegmLimit, dsegmBase, TRUE, privilege, DATA, BYTE_GRANULARITY);      
    }
  }
  if (esegmBase != NO_BASE) {
    if (!memAreaValid(esegmBase, esegmLimit, FALSE)) {
      goto finish;
    }
    esegmIdx = get_free_gdt_idx();
    if (esegmIdx == INVALID_GDT_IDX) {
      goto finish;
    }
    if (esegmLimit > 0xfffff) {
      limit = esegmLimit;
      limit &= 0x3ff;
      if (limit == 0) {
	limit = (esegmLimit >> 12);
      } else {
	limit = (esegmLimit >> 12) + 1;
      }
      write_segment_desc(esegmIdx, limit, esegmBase, TRUE, privilege, DATA, PAGE_GRANULARITY);
    } else {
      write_segment_desc(esegmIdx, esegmLimit, esegmBase, TRUE, privilege, DATA, BYTE_GRANULARITY);      
    }
  }
  if (fsegmBase != NO_BASE) {
    if (!memAreaValid(fsegmBase, fsegmLimit, FALSE)) {
      goto finish;
    }
    fsegmIdx = get_free_gdt_idx();
    if (fsegmIdx == INVALID_GDT_IDX) {
      goto finish;
    }
    if (fsegmLimit > 0xfffff) {
      limit = fsegmLimit;
      limit &= 0x3ff;
      if (limit == 0) {
	limit = (fsegmLimit >> 12);
      } else {
	limit = (fsegmLimit >> 12) + 1;
      }
      write_segment_desc(fsegmIdx, limit, fsegmBase, TRUE, privilege, DATA, PAGE_GRANULARITY);
    } else {
      write_segment_desc(fsegmIdx, fsegmLimit, fsegmBase, TRUE, privilege, DATA, BYTE_GRANULARITY);      
    }
  }
  if (gsegmBase != NO_BASE) {
    if (!memAreaValid(gsegmBase, gsegmLimit, TRUE)) {
      goto finish;
    }
    gsegmIdx = get_free_gdt_idx();
    if (gsegmIdx == INVALID_GDT_IDX) {
      goto finish;
    }
    if (gsegmLimit > 0xfffff) {
      limit = gsegmLimit;
      limit &= 0x3ff;
      if (limit == 0) {
	limit = (gsegmLimit >> 12);
      } else {
	limit = (gsegmLimit >> 12) + 1;
      }
      write_segment_desc(gsegmIdx, limit, gsegmBase, TRUE, privilege, DATA, PAGE_GRANULARITY);
    } else {
      write_segment_desc(gsegmIdx, gsegmLimit, gsegmBase, TRUE, privilege, DATA, BYTE_GRANULARITY);      
    }
  }
  if (ssegmBase != NO_BASE) {
    if (!memAreaValid(ssegmBase, ssegmLimit, FALSE)) {
      goto finish;
    }
    ssegmIdx = get_free_gdt_idx();
    if (ssegmIdx == INVALID_GDT_IDX) {
      goto finish;
    }
    if (ssegmLimit > 0xfffff) {
      limit = ssegmLimit;
      limit &= 0x3ff;
      if (limit == 0) {
	limit = (ssegmLimit >> 12);
      } else {
	limit = (ssegmLimit >> 12) + 1;
      }
      write_segment_desc(ssegmIdx, limit, ssegmBase, TRUE, privilege, DATA, PAGE_GRANULARITY);
    } else {
      write_segment_desc(ssegmIdx, ssegmLimit, ssegmBase, TRUE, privilege, DATA, BYTE_GRANULARITY);      
    }
  }
  tssIdx = get_free_gdt_idx();
  if (tssIdx == INVALID_GDT_IDX) {
    goto finish;
  }
  write_tss_desc(tssIdx, sizeof(tss_t), KERNEL_BASE + (uint32_t ) process, TRUE, privilege, TRUE, BYTE_GRANULARITY);

  process->idx_tss = tssIdx;
  process->schedticks = schedticks;
  process->ticksleft = schedticks;
  process->esp_reg = ssegmLimit;
  process->ebp_reg = ssegmLimit;
  process->eip_reg = start;
  //TODO: what do these flags mean?
  process->eflag_reg = 0x200;
  process->state = 0;
  process->callMask = 0;
  process->next = NULL;
  process->previous = NULL;
  process->firstSender = NULL;
  process->nextSender = NULL;
  process->receivingFrom = NULL;
  process->iomap_base = sizeof(tss_t);
  strncpy(process->procname, name, sizeof(process->procname));

  process->cs_reg = csegmIdx + privilege;
  process->ds_reg = dsegmIdx + privilege;
  process->es_reg = esegmIdx + privilege;
  process->fs_reg = fsegmIdx + privilege;
  process->gs_reg = gsegmIdx + privilege;
  process->ss_reg = ssegmIdx + privilege;
  result = process;
  csegmIdx = INVALID_GDT_IDX;
  dsegmIdx = INVALID_GDT_IDX;
  esegmIdx = INVALID_GDT_IDX;
  fsegmIdx = INVALID_GDT_IDX;
  gsegmIdx = INVALID_GDT_IDX;
  ssegmIdx = INVALID_GDT_IDX;
  tssIdx = INVALID_GDT_IDX;
finish:
  if (csegmIdx != INVALID_GDT_IDX) {
    free_gdt_idx(csegmIdx);
  }
  if (dsegmIdx != INVALID_GDT_IDX) {
    free_gdt_idx(dsegmIdx);
  }
  if (esegmIdx != INVALID_GDT_IDX) {
    free_gdt_idx(esegmIdx);
  }
  if (fsegmIdx != INVALID_GDT_IDX) {
    free_gdt_idx(fsegmIdx);
  }
  if (gsegmIdx != INVALID_GDT_IDX) {
    free_gdt_idx(gsegmIdx);
  }
  if (ssegmIdx != INVALID_GDT_IDX) {
    free_gdt_idx(ssegmIdx);
  }
  if (tssIdx != INVALID_GDT_IDX) {
    free_gdt_idx(tssIdx);
  }
  return result;
}