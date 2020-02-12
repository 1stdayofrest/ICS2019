#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#define BAD_EXP -1111
/*1*1 +* * *  (a)*a (*a), NEG负数,DEREF指针解引用*/
enum {
  TK_NOTYPE = 256,
  NUM,
  REG,
  HEX,
  RBRACKET,
  ADD,
  MINUS,
  MULTIPLY,
  TK_EQ,
  DIVIDE,
  LBRACKET,
  AND,
  OR,
  NEQ,
  NEG,
  DEREF

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", TK_NOTYPE},          // spaces
    {"\\+", '+'},               // plus
    {"==", TK_EQ},              // equal
    {"0[xX][0-9a-fA-F]+", HEX}, // hex number
    {"[0-9]+", NUM},            // numbers
    {"\\-", MINUS},             // minus
    {"\\*", MULTIPLY},          // multiply
    {"\\/", DIVIDE},            // divide
    {"\\(", LBRACKET},          // left bracket
    {"\\)", RBRACKET},          // right bracket
    {"\\$e[abcd]x", REG},       // register
    {"\\$e[bs]p", REG},
    {"\\$e[sd]i", REG},
    {"\\$eip", REG},
    {"\\$pc", REG},
    {"&&", AND},    // and
    {"\\|\\|", OR}, // or
    {"!=", NEQ}     // not equal
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i++) {
    /* 这个函数把指定的正则表达式pattern编译成一种特定的数据格式compiled，
    这样可以使匹配更有效。函数regexec
    会使用这个数据在目标文本串中进行模式匹配。执行成功返回０。 　 参数说明：
    @regex_t 是一个结构体数据类型，用来存放编译后的正则表达式，它的成员re_nsub
    用来存储正则表达式中的子正则表达式的个数，子正则表达式就是用圆括号包起来的部分表达式。
    @pattern 是指向我们写好的正则表达式的指针。
    @cflags 有如下4个值或者是它们或运算(|)后的值：
        REG_EXTENDED 以功能更加强大的扩展正则表达式的方式进行匹配。
        REG_ICASE 匹配字母时忽略大小写。
        REG_NOSUB 不用存储匹配后的结果。
        REG_NEWLINE
    识别换行符，这样'$'就可以从行尾开始匹配，'^'就可以从行的开头开始匹配。
    */
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;
/*
首先要识别出其中的token, 进行这项工作的是make_token()函数.
它用position变量来指示当前处理到的位置,
并且按顺序尝试用不同的规则来匹配当前位置的字符串. 当一条规则匹配成功,
并且匹配出的子串正好是position所在位置的时候, 我们就成功地识别出一个token,
Log()宏会输出识别成功的信息.
你需要做的是将识别出的token信息记录下来(一个例外是空格串),
我们使用Token结构体来记录token的信息
@return: 是否被正则读取
*/
static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++) {
      /*
      当我们编译好正则表达式后，就可以用regexec 匹配我们的目标文本串了，
      如果在编译正则表达式的时候没有指定cflags的参数为REG_NEWLINE，
      则默认情况下是忽略换行符的，也就是把整个文本串当作一个字符串处理。
      执行成功返回０。
      regmatch_t 是一个结构体数据类型，在regex.h中定义：
      typedef struct
      {
      regoff_t rm_so;
      regoff_t rm_eo;
      } regmatch_t;
      成员rm_so 存放匹配文本串在目标串中的开始位置，rm_eo 存放结束位置。
      通常我们以数组的形式定义一组这样的结构。因为往往我们的正则表达式中还包含子正则表达式。
      数组0单元存放主正则表达式位置，后边的单元依次存放子正则表达式位置。
      参数说明：
      @compiled 是已经用regcomp函数编译好的正则表达式。
      @string 是目标文本串。
      @nmatch 是regmatch_t结构体数组的长度。
      @matchptr regmatch_t类型的结构体数组，存放匹配文本串的位置信息。
      @eflags 有两个值
          REG_NOTBOL
      按我的理解是如果指定了这个值，那么'^'就不会从我们的目标串开始匹配。
          总之我到现在还不是很明白这个参数的意义；
          REG_NOTEOL 和上边那个作用差不多，不过这个指定结束end of line。
      */
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 &&
          pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i,
            rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        /* nr_token 指示已经被识别出的token数目 */
        tokens[nr_token].type = rules[i].token_type;
        switch (rules[i].token_type) {
          // 因为空格串并不参加求值过程, 识别出来之后就可以将它们丢弃了
        case TK_NOTYPE:
          break;
        case NUM:

        case REG:

        case HEX:
          for (i = 0; i < substr_len; i++)
            tokens[nr_token].str[i] = substr_start[i];
          tokens[nr_token].str[i] = '\0';
          nr_token++;
          break;
        case ADD:

        case MINUS:
          //判断一下 可能是负数 也可能是减号

        case MULTIPLY:
          //判断一下 可能是引用

        case DIVIDE:

        case LBRACKET:

        case RBRACKET:
          tokens[nr_token].str[0] = substr_start[0];
          tokens[nr_token++].str[1] = '\0';
          break;
        case AND:

        case OR:

        case TK_EQ:

        case NEQ:
          tokens[nr_token].str[0] = substr_start[0];
          tokens[nr_token].str[1] = substr_start[1];
          tokens[nr_token++].str[2] = '\0';
          break;
        default:
          TODO();
        }
        //不用再尝试其他规则
        break;
      }
    }
    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
  return true;
}

uint32_t eval(int p, int q);
uint32_t getnum(char str);
bool judge_exp();
bool check_parentheses(int p, int q);
int find_dominant_operator(int p, int q);
int priority(int i);

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  if (!judge_exp()){
    *success = false;
  }
  else{
    for (int i = 0; i < nr_token; i ++) {
      if ((tokens[i].type == MINUS) && (i == 0 || tokens[i - 1].type >= ADD )) {
        tokens[i].type = NEG;
      }
      if ((tokens[i].type == MULTIPLY) && (i == 0 || tokens[i - 1].type >= ADD )) {
        tokens[i].type = DEREF;
      }
    }
    return eval(0, nr_token - 1);
  }
  return 0;
}

/*求出 [p，q]之间的表达式的值*/
uint32_t eval(int p, int q) {
//  printf("in the eval p = %d, q = %d\n", p, q);
  if (p > q){
    return BAD_EXP;
  }
  else if (p == q){
    //tokens[p]是一个值 可能是num 可能是 REG 可能是 HEX
    if (tokens[p].type == NUM) {
      return atoi(tokens[p].str);
    }
      //eax, ecx, edx, ebx, esp, ebp, esi, edi;
    else if (tokens[p].type == REG) {
      if (strcmp(tokens[p].str, "$eax") == 0){
        //printf("eax = %u\n", cpu.eax);
        return cpu.eax;}
      else if (strcmp(tokens[p].str, "$ebx") == 0)  return cpu.ebx;
      else if (strcmp(tokens[p].str, "$ecx") == 0)  return cpu.ecx;
      else if (strcmp(tokens[p].str, "$edx") == 0)  return cpu.edx;
      else if (strcmp(tokens[p].str, "$ebp") == 0)  return cpu.ebp;
      else if (strcmp(tokens[p].str, "$esp") == 0)  return cpu.esp;
      else if (strcmp(tokens[p].str, "$esi") == 0)  return cpu.esi;
      else if (strcmp(tokens[p].str, "$edi") == 0)  return cpu.edi;
      else if (strcmp(tokens[p].str, "$pc") == 0)  return cpu.pc;
    }
    else if (tokens[p].type == HEX) {
      int cnt, i, len, sum = 0;
      len = strlen(tokens[p].str);
      cnt = 1;

      for (i = len-1; i >= 0; i--) {
        sum = sum + cnt * getnum(tokens[p].str[i]);
        cnt *= 16;
      }
      return sum;
    }
  }
    //p < q [p,q]这一段被括号合法包围
  else if (check_parentheses(p, q)){
    //去掉括号
    return eval(p + 1, q - 1);
  }
  else {
    //没有被括号包围
    //找到dominant运算符，起决定性作用的运算符
    int op = find_dominant_operator(p, q);
    //printf("op = %d\n", op);
    //p=0,q=1  -1
    //op = 0
    //(0,-1)
    //(1, 1)
    //计算运算符两边的表达式的值
    uint32_t val1;
    if (tokens[op].type != DEREF && tokens[op].type != NEG)
    {
      val1 = eval(p, op - 1);
    }
    uint32_t val2 = eval(op + 1, q);
    //printf("op = %d val1 = %u val2 = %u\n", op, val1, val2);

    switch (tokens[op].type) {
    case ADD:
      return val1 + val2;
    case MINUS:
      return val1 - val2;
    case MULTIPLY:
      return val1 * val2;
    case DIVIDE:
      return val1 / val2;
    case AND:
      return val1 && val2;
    case OR:
      return val1 || val2;
    case TK_EQ:
      return val1 == val2;
    case NEQ:
      return val1 != val2;
    case NEG:
      return 0 - val2;
    case DEREF:
      return vaddr_read(val2, 4);
    default:
      assert(0);
    }
  }
  return 1;
}
//判断tokens数组是不是一个合法的表达式，主要是看括号是否匹配，左括号数？=右括号数
bool judge_exp() {
  int i, cnt;

  cnt = 0;
  for (i = 0; i <= nr_token; i++) {
    if (tokens[i].type == LBRACKET)
      cnt++;
    else if (tokens[i].type == RBRACKET)
      cnt--;

    if (cnt < 0)
      return false;
  }

  return true;
}
/*
check_parentheses()函数用于判断表达式是否被一对匹配的括号包围着,
同时检查表达式的左右括号是否匹配, 如果不匹配, 这个表达式肯定是不符合语法的,
也就不需要继续进行求值了
范围[p,q]
*/
bool check_parentheses(int p, int q) {
  int i, bra = 0;

  for (i = p; i <= q; i++) {
    if (tokens[i].type == LBRACKET) {
      bra++;
    }
    if (tokens[i].type == RBRACKET) {
      bra--;
    }
    if(bra == 0 && i < q) {
      return false;
    }
  }

  return true;
}

int find_dominant_operator(int p, int q) {
  int i = 0, j, cnt;
  int op = 0, opp, pos = -1;
  for (i = p; i <= q; i++){
    if (tokens[i].type == NUM || tokens[i].type == REG || tokens[i].type == HEX)
      continue;
    else if (tokens[i].type == LBRACKET) {
      cnt = 0;
      for (j = i + 1; j <= q; j++) {
        if (tokens[j].type == RBRACKET) {
          cnt++;
          i += cnt;
          break;
        }
        else
          cnt++;
      }
    }
    else {
      opp = priority(i);
      if (opp >= op) {
        pos = i;
        op = opp;
      }
    }
  }
//  printf("op = %d, pos = %d\n",  op, pos);
  return pos;
}

int priority(int i) {
  if (tokens[i].type == ADD || tokens[i].type == MINUS) return 4;
  else if (tokens[i].type == MULTIPLY || tokens[i].type == DIVIDE) return 3;
  else if (tokens[i].type == OR) return 12;
  else if (tokens[i].type == AND) return 11;
  else if (tokens[i].type == NEQ || tokens[i].type == TK_EQ) return 7;
  return 0;
}

uint32_t getnum(char str)
{
  if (str >= '0' && str <= '9')
    return str - '0';
  else if (str >= 'a' && str <= 'f')
    return str - 'a' + 10;
  else if (str >= 'A' && str <= 'F')
    return str - 'A' + 10;
  return 0;
}