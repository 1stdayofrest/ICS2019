#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"
/* 结构体中只定义了两个成员: NO表示监视点的序号, next就不用多说了吧.
为了实现监视点的功能, 你需要根据你对监视点工作原理的理解在结构体
中增加必要的成员. 同时我们使用"池"的数据结构来管理监视点结构体,
框架代码中已经给出了一部分相关的代码
(在nemu/src/monitor/debug/watchpoint.c中)
 */
typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char exp[100];//表达式
  uint32_t value;//值
} WP;
void display_wp();
void insert_wp(char *args);
void delete_wp(int no);
int * haschanged();
#endif
