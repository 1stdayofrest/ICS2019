int init_monitor(int, char *[]);
void ui_mainloop(int);

#include <stdio.h>
/*
 * 我们已经知道, NEMU是一个用来执行客户程序的程序,
 * 但客户程序一开始并不存在于客户计算机中.
 * 我们需要将客户程序读入到客户计算机中,
 * 这件事是monitor来负责的. 于是NEMU在开始运行的时候,
 * 首先会调用init_monitor()函数
 * (在nemu/src/monitor/monitor.c中定义)
 * 来进行一些和monitor相关的初始化工作.
 * */
int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
  int is_batch_mode = init_monitor(argc, argv);
//  printf("hello windows remote");
  /* Receive commands from user. */
  ui_mainloop(is_batch_mode);

  return 0;
}
