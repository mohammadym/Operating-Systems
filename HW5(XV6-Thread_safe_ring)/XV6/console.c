// Console input and output.
// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "stdbool.h"

static void consputc(int);

static int panicked = 0;

static struct
{
  struct spinlock lock;
  int locking;
} cons;

// history
#define MAX_CMD 11
#define CMD_LEN 100

static struct history
{
  char commands[MAX_CMD][CMD_LEN];
  short head;
  short tail;
  short curr;
  short size;
  bool bottom;
} hist;

typedef struct history history_t;
void hist_init(history_t *h);
void hist_advance(history_t *h);
void hist_add(history_t *h, char *cmd);
char *hist_getprev(history_t *h);
char *hist_getnext(history_t *h);
char *hist_lastcmd(history_t *h);
// end history

static void
printint(int xx, int base, int sign)
{
  static char digits[] = "0123456789abcdef";
  char buf[16];
  int i;
  uint x;

  if (sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do
  {
    buf[i++] = digits[x % base];
  } while ((x /= base) != 0);

  if (sign)
    buf[i++] = '-';

  while (--i >= 0)
    consputc(buf[i]);
}
//PAGEBREAK: 50

// Print to the console. only understands %d, %x, %p, %s.
void cprintf(char *fmt, ...)
{
  int i, c, locking;
  uint *argp;
  char *s;

  locking = cons.locking;
  if (locking)
    acquire(&cons.lock);

  if (fmt == 0)
    panic("null fmt");

  argp = (uint *)(void *)(&fmt + 1);
  for (i = 0; (c = fmt[i] & 0xff) != 0; i++)
  {
    if (c != '%')
    {
      consputc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if (c == 0)
      break;
    switch (c)
    {
    case 'd':
      printint(*argp++, 10, 1);
      break;
    case 'x':
    case 'p':
      printint(*argp++, 16, 0);
      break;
    case 's':
      if ((s = (char *)*argp++) == 0)
        s = "(null)";
      for (; *s; s++)
        consputc(*s);
      break;
    case '%':
      consputc('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      consputc('%');
      consputc(c);
      break;
    }
  }

  if (locking)
    release(&cons.lock);
}

void panic(char *s)
{
  int i;
  uint pcs[10];

  cli();
  cons.locking = 0;
  // use lapiccpunum so that we can call panic from mycpu()
  cprintf("lapicid %d: panic: ", lapicid());
  cprintf(s);
  cprintf("\n");
  getcallerpcs(&s, pcs);
  for (i = 0; i < 10; i++)
    cprintf(" %p", pcs[i]);
  panicked = 1; // freeze other CPU
  for (;;)
    ;
}

//PAGEBREAK: 50
#define BACKSPACE 0x100
#define KEY_UP 0x101
#define KEY_DN 0x102
#define CRTPORT 0x3d4

static ushort *crt = (ushort *)P2V(0xb8000); // CGA memory

static void
cgaputc(int c)
{
  int pos;

  // Cursor position: col + 80*row.
  outb(CRTPORT, 14);
  pos = inb(CRTPORT + 1) << 8;
  outb(CRTPORT, 15);
  pos |= inb(CRTPORT + 1);

  if (c == '\n')
    pos += 80 - pos % 80;
  else if (c == BACKSPACE)
  {
    if (pos > 0)
      --pos;
  }
  else
    crt[pos++] = (c & 0xff) | 0x0700; // black on white

  if (pos < 0 || pos > 25 * 80)
    panic("pos under/overflow");

  if ((pos / 80) >= 24)
  { // Scroll up.
    memmove(crt, crt + 80, sizeof(crt[0]) * 23 * 80);
    pos -= 80;
    memset(crt + pos, 0, sizeof(crt[0]) * (24 * 80 - pos));
  }

  outb(CRTPORT, 14);
  outb(CRTPORT + 1, pos >> 8);
  outb(CRTPORT, 15);
  outb(CRTPORT + 1, pos);
  crt[pos] = ' ' | 0x0700;
}

void consputc(int c)
{
  if (panicked)
  {
    cli();
    for (;;)
      ;
  }

  // if (c == KEY_DN || c == KEY_UP)
  // {
  //   return;
  // }

  if (c == BACKSPACE)
  {
    uartputc('\b');
    uartputc(' ');
    uartputc('\b');
  }
  else
    uartputc(c);
  cgaputc(c);
}

#define INPUT_BUF 128
struct
{
  char buf[INPUT_BUF];
  uint r; // Read index
  uint w; // Write index
  uint e; // Edit index
} input;

#define C(x) ((x) - '@') // Control-x

void helper(char c)
{
  if (c != 0 && input.e - input.r < INPUT_BUF)
  {
    c = (c == '\r') ? '\n' : c;
    input.buf[input.e++ % INPUT_BUF] = c;
    consputc(c);
    if (c == '\n' || c == C('D') || input.e == input.r + INPUT_BUF)
    {
      input.w = input.e;
      wakeup(&input.r);
    }
  }
}

void kill_line()
{
  while (input.e != input.w &&
         input.buf[(input.e - 1) % INPUT_BUF] != '\n')
  {
    input.e--;
    consputc(BACKSPACE);
  }
}

void cmd_write(char *cmd)
{
  cmd--;
  while (*(++cmd) != 0)
  {
    input.buf[input.e++ % INPUT_BUF] = *cmd;
    consputc(*cmd);
  }
}

// call this when you encounter ESC + 91

void handle_history_keys(int (*getc)(void))
{
  int c = getc();
  char *cmd = NULL;
  // if it's not arrow up down or page up or down just do the normal thing with them
  if (c != 65 && c != 66 && c != 53 && c != 54)
  {
    helper(27);
    helper(91);
    helper(c);
    return;
  }

  // at this point c is either pageup or pagedown or arrow up or down
  // we assume we got 27(ESC) or 91([)
  if (c == 53 || c == 65) // pageup or arrow up
  {
    cmd = hist_getprev(&hist);
    if (cmd == NULL)
      return;
    kill_line();
    cmd_write(cmd);
  }
  else // pagedown or arrow down
  {
    cmd = hist_getnext(&hist);
    if (cmd == NULL)
    {
      if (input.e != input.w)
        kill_line();
      return;
    }
    kill_line();
    cmd_write(cmd);
  }

  //now we gotta check if its pagekey because pagekeys has 126(~) at the end
  if (c == 53 || c == 54)
  {
    getc(); // 126
  }
}

void consoleintr(int (*getc)(void))
{
  int c, doprocdump = 0;
  char *cmd = NULL;
  char *lastcmd;
  short lastcmd_len;
  acquire(&cons.lock);

  while ((c = getc()) >= 0)
  {
    switch (c)
    {
    case C('P'): // Process listing.
      // procdump() locks cons.lock indirectly; invoke later
      doprocdump = 1;
      break;
    case C('U'): // Kill line.
      while (input.e != input.w &&
             input.buf[(input.e - 1) % INPUT_BUF] != '\n')
      {
        input.e--;
        consputc(BACKSPACE);
      }
      break;
    case C('H'):
    case '\x7f': // Backspace
      if (input.e != input.w)
      {
        input.e--;
        consputc(BACKSPACE);
      }
      break;
    case 0xE2: // ARROW UP - for graphical mode
    case 0xE6: // PAGE  UP - for graphical mode
      cmd = hist_getprev(&hist);
      if (cmd == NULL)
        break;
      kill_line();
      cmd_write(cmd);
      break;
    case 0xE3: // ARROW DOWN - for graphical mode
    case 0xE7: // PAGE  DOWN - for graphical mode
      cmd = hist_getnext(&hist);
      if (cmd == NULL)
      {
        if (input.e != input.w)
          kill_line();
        break;
      }
      kill_line();
      cmd_write(cmd);
      break;

    case '\x1b': //27 or ESC
      c = getc();

      if (c != 91)
      {
        helper(27);
        helper(91);
        break;
      }

      // when we reach here it means we pressed arrowkey or pagekeys
      handle_history_keys(getc);
      break;
    default:

      if (c != 0 && input.e - input.r < INPUT_BUF)
      {
        c = (c == '\r') ? '\n' : c;
        input.buf[input.e++ % INPUT_BUF] = c;
        consputc(c);
        if (c == '\n' || c == C('D') || input.e == input.r + INPUT_BUF || c == 3)
        {
          if (input.e - input.w != 1)
          {
            lastcmd = hist_lastcmd(&hist);
            lastcmd_len = strlen(&input.buf[input.w]) - 1;
            if (strncmp(&input.buf[input.w], lastcmd, lastcmd_len) != 0)
              hist_add(&hist, &input.buf[input.w]);
          }

          input.w = input.e;
          wakeup(&input.r);
        }
      }
      break;
    }
  }
  release(&cons.lock);
  if (doprocdump)
  {
    procdump(); // now call procdump() wo. cons.lock held
  }
}

int consoleread(struct inode *ip, char *dst, int n)
{
  uint target;
  int c;

  iunlock(ip);
  target = n;
  acquire(&cons.lock);
  while (n > 0)
  {
    while (input.r == input.w)
    {
      if (myproc()->killed)
      {
        release(&cons.lock);
        ilock(ip);
        return -1;
      }
      sleep(&input.r, &cons.lock);
    }
    c = input.buf[input.r++ % INPUT_BUF];
    if (c == C('D'))
    { // EOF
      if (n < target)
      {
        // Save ^D for next time, to make sure
        // caller gets a 0-byte result.
        input.r--;
      }
      break;
    }

    *dst++ = c;
    --n;
    if (c == '\n')
      break;
  }
  release(&cons.lock);
  ilock(ip);

  return target - n;
}

int consolewrite(struct inode *ip, char *buf, int n)
{
  int i;
  iunlock(ip);
  acquire(&cons.lock);
  for (i = 0; i < n; i++)
    consputc(buf[i] & 0xff);
  release(&cons.lock);
  ilock(ip);

  return n;
}

void consoleinit(void)
{
  initlock(&cons.lock, "console");

  devsw[CONSOLE].write = consolewrite;
  devsw[CONSOLE].read = consoleread;
  cons.locking = 1;

  ioapicenable(IRQ_KBD, 0);

  // init history
  hist_init(&hist);
}

// History

void hist_init(history_t *h)
{
  h->curr = 0;
  h->head = 0;
  h->size = 0;
  h->tail = -1;
  h->bottom = true;

  for (short i = 0; i < MAX_CMD; i++)
  {
    memset(h->commands[i], '\0', CMD_LEN);
  }
}

void hist_advance(history_t *h)
{

  if (h->tail == -1)
  {
    h->head++;
    h->tail++;
  }
  else if (h->head > h->tail && (h->head + 1) % MAX_CMD == 0)
  {
    h->head = 0;
    h->tail++;
  }
  else if (h->head < h->tail && (h->tail + 1) % MAX_CMD == 0)
  {
    h->head++;
    h->tail = 0;
  }
  else if (h->size == MAX_CMD)
  {
    h->head++;
    h->tail++;
  }
  else
    h->head++;

  h->curr = h->head;
}

void hist_add(history_t *h, char *cmd)
{
  memset(h->commands[h->head], '\0', CMD_LEN);
  safestrcpy(h->commands[h->head], cmd, strlen(cmd) % (CMD_LEN + 1));
  h->bottom = false;

  if (h->size != MAX_CMD)
    ++h->size;

  hist_advance(h);
}

char *hist_getprev(history_t *h)
{
  // if current is less than tail by one we have the oldest history
  if (h->curr == h->head)
    h->curr = h->curr == 0 ? MAX_CMD - 1 : h->curr - 1;

  if (h->bottom)
    return NULL;

  char *cmd = h->commands[h->curr];
  if (h->curr == h->tail)
    h->bottom = true;
  else
    h->curr = h->curr == 0 ? MAX_CMD - 1 : h->curr - 1;

  return cmd;
}

char *hist_getnext(history_t *h)
{
  if (h->bottom == true)
  {
    h->curr = (h->curr + 1) % MAX_CMD;
    h->bottom = false;
  }

  if (h->curr == h->head)
    return NULL;

  char *cmd = h->commands[h->curr];
  h->curr = (h->curr + 1) % MAX_CMD;
  return cmd;
}

char *hist_lastcmd(history_t *h)
{
  return h->commands[h->head - 1 < 0 ? MAX_CMD - 1 : h->head - 1];
}