#include "common.h"
#include "shell/comarg.h"
#include "font/font.h"

int head = 0, pos = 0;
char history[10][100];

int cmd_echo(char *args)
{
	printk("%s\n", args);
	return 0;
}

int cmd_ls(char *args)
{
	return 0;
}

int cmd_cat(char *args)
{
	return 0;
}

int cmd_touch(char *args)
{
	return 0;
}

typedef struct {
	char *cmd;
	int (* func)(char *args);
	char *helpinfo;
} Command_list;

Command_list command_list[] = {
	{"echo", cmd_echo, "type string on screen."},
	{"ls", cmd_ls, "list the file and directory."},
	{"cat", cmd_echo, "display file."},
	{"touch", cmd_touch, "create file."}
};

#define NR_CMD (sizeof(command_list) / sizeof(command_list[0]))

void command_helper()
{
	int i;
	for(i = 0; i < NR_CMD; i++)
	{}
}

void update_buf(char ch)
{
	if(ch == '\n')
	{
		history[head][pos ++] = '\0';
		command_helper();
		pos = 0;
	}
	history[head][pos ++] = ch;
}
