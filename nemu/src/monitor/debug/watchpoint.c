#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32
/* 代码中定义了监视点结构的池wp_pool,
还有两个链表head和free_, 其中head用于组织使用中的监视点结构,
*/
static WP wp_pool[NR_WP] = {};
/*free_用于组织空闲的监视点结构*/
static WP *head = NULL, *free_ = NULL;
/*init_wp_pool()函数会对两个链表进行了初始化.*/
void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
    wp_pool[i].exp[0] = '\0';
    wp_pool[i].value = -1;
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
/*
new_wp()从free_链表中返回一个空闲的监视点结构
*/
WP* new_wp() {
  if (free_ == NULL) {
    assert(0);
  }

  WP *wp = free_;
  free_ = free_->next;
  wp->next = NULL;

  return wp;
}
/*
free_wp()将wp归还到free_链表中,
这两个函数会作为监视点池的接口被其它函数调用.
*/
void free_wp(WP *wp) {
  wp->exp[0] = '\0';
  wp->value = -1;
  wp->next = free_;
  free_ = wp;
}

void insert_wp(char *args) {
  bool flag = true;
  uint32_t val = expr(args, &flag);

  if (!flag) {
    printf("You input an invalid expression, failed to create watchpoint!");
    return ;
  }

  WP *wp = new_wp();
  wp->value = val;
  strcpy(wp->exp, args);

  if (head == NULL) {
    wp->NO = 1;
    head = wp;
  }
  else {
    WP *wwp;
    wwp = head;
    while (wwp->next != NULL) {
      wwp = wwp->next;
    }
    wp->NO = wwp->NO + 1;
    wwp->next = wp;
  }

  return ;
}

void delete_wp(int no) {

  if (head == NULL) {
    printf("There is no watchpoint to delete!");
    return ;
  }

  WP *wp;
  if (head->NO == no) {
    wp = head;
    head = head->next;
    free_wp(wp);
  }
  else {
    wp = head;
    while (wp->next != NULL && wp->next->NO != no) {
      wp = wp->next;
    }
    if (wp == NULL) {
      printf("Failed to find the NO.%d watchpoint!", no);
    }
    else {
      WP *del_wp;
      del_wp = wp->next;
      wp->next = del_wp->next;
      free_wp(del_wp);
      printf("NO.%d  watchpoint has been deleted!\n", no);
    }
  }

  return ;
}

void display_wp() {
  if (head == NULL) {
    printf("There is no watchpoint!\n");
    return ;
  }

  WP *wp;
  printf("NO      expression        value\n");
  wp = head;
  while (wp != NULL) {
    printf("%-5d   %-15s   %-16u\n", wp->NO, wp->exp, wp->value);
    wp = wp->next;
  }
}

/*
  每当cpu_exec()执行完一条指令, 就对所有待监视的表达式进行求值
  (你之前已经实现了表达式求值的功能了), 比较它们的值有没有发生变化,
  若发生了变化, 程序就因触发了监视点而暂停下来,
  你需要将nemu_state.state变量设置为NEMU_STOP来达到暂停的效果.
  最后输出一句话提示用户触发了监视点, 并返回到ui_mainloop()循环中等待用户的命令
*/
int * haschanged() {
  WP *wp = head;
  bool flag = true;
  uint32_t val;
  static int no[NR_WP];
  int i = 0;

  while (wp != NULL) {
    val = expr(wp->exp, &flag);
    if (val != wp->value) {
      wp->value = val;
      no[i++] = wp->NO;
    }
    wp = wp->next;
  }
  no[i] = -1;

  return no;
}
