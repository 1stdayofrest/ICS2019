#include "monitor/expr.h"
#include "monitor/monitor.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <readline/history.h>
#include <readline/readline.h>
#include <stdlib.h>

void cpu_exec(uint64_t);
/* 打印寄存器状态
 */
void isa_reg_display();
void init_regex();
uint32_t expr(char *e, bool *success);
//string -> int
int trans(char *e) {
  int len, num, i, j;
  len = strlen(e);
  num = 0;
  j = 1;

  for (i = len-1; i > 1; i--) {
    num += (e[i]-'0')*j;
    j *= 16;
  }
  // printf("num = %d\n", num);

  return num;
}

/* We use the `readline' library to provide more flexibility to read from stdin.
 */
static char *rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) { return -1; }

static int cmd_help(char *args);

static int cmd_si(char *args){
  if (args == NULL)
  {
    cpu_exec(1);
    return 0;
  }

  int a = atoi(args);
  cpu_exec(a);
  return 0;
}

static int cmd_info(char *args){
  if (args == NULL) {
    printf("Please input the info r or info w\n");
    return -1;
  }
  if (strcmp(args, "r") == 0)
  {
    printf("eax:  0x%-10x    %-10d\n", cpu.eax, cpu.eax);
    printf("edx:  0x%-10x    %-10d\n", cpu.edx, cpu.edx);
    printf("ecx:  0x%-10x    %-10d\n", cpu.ecx, cpu.ecx);
    printf("ebx:  0x%-10x    %-10d\n", cpu.ebx, cpu.ebx);
    printf("ebp:  0x%-10x    %-10d\n", cpu.ebp, cpu.ebp);
    printf("esi:  0x%-10x    %-10d\n", cpu.esi, cpu.esi);
    printf("esp:  0x%-10x    %-10d\n", cpu.esp, cpu.esp);
    printf("edi:  0x%-10x    %-10d\n", cpu.edi, cpu.edi);
    printf("pc:  0x%-10x    %-10d\n", cpu.pc, cpu.pc);
  }
  else if (strcmp(args, "w") == 0) {
    display_wp();
  }
  else {
    printf("The info command need a parameter 'r' or 'w'\n");
  }
  return 0;
}
//扫描内存
static int cmd_x(char *args){

  if (args == NULL)
  {
    printf("Input invalid command!\n");
  }else
  {
    int n, addr, i;
    char* exp;
    //解析扫描长度
    n = atoi(strtok(NULL," "));
    //解析解析扫描的表达式的值，默认16位,并将它转换成16进制数
    //比如 "0x100000"
    //TODO:怎么将字符串的16进制转换成整数
    //调用vaddr_read阅读内存地址对应的值是什么
    exp = strtok(NULL," ");
    //把16进制转换位10进制
    addr = trans(exp);
    //printf("%d\n",addr);
    for (i = 0; i < n; i++) {
      printf("0x%x\n", vaddr_read(addr, 4));
      // printf("%c\n", vaddr_read(addr, 4));
      addr += 4;
    }
  }
  return 0;
}
static int cmd_p(char *args){
  if (args == NULL) {
    printf("Input invalid command! Please input the expression.\n");
  }
  else {
    //init_regex();

    bool success = true;
    //printf("args = %s\n", args);
    int result = expr(args, &success);

    if (success) {
      printf("result = %d\n", result);
    }
    else {
      printf("Invalid expression!\n");
    }
  }
  return 0;
}
static int cmd_w(char *args){
  if (args == NULL) {
    printf("Input invalid command! Please input the expression.\n");
  }
  else {
    insert_wp(args);
  }
  return 0;
}
static int cmd_d(char *args){
  if (args == NULL) {
    printf("Input invalid command! Please input the NO.\n");
  }
  else {
    int no = atoi(args);
    delete_wp(no);
  }
  return 0;
}

static struct {
  char *name;
  char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display informations about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},
    { "si", "Let the program execute n steps", cmd_si },
    { "info", "Display the register status and the watchpoint information", cmd_info},
    { "x", "Caculate the value of expression and display the content of the address", cmd_x},
    { "p","Calculate an expression", cmd_p},
    { "w", "Create a watchpoint", cmd_w},
    { "d", "Delete a watchpoint", cmd_d},
    /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  } else {
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL;) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) {
      continue;
    }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) {
          return;
        }
        break;
      }
    }

    if (i == NR_CMD) {
      printf("Unknown command '%s'\n", cmd);
    }
  }
}
