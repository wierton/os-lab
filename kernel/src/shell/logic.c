#include "common.h"
#include "shell/comarg.h"
#include "font/font.h"
#include "x86/x86.h"
#include "device/fs.h"
#include "math.h"

int head = 0, pos = 0;
char history[10][100];
char cur_path[100] = "/";

char *full_path(char *name)
{
	static char fullpath[100];
	if(name[0] == '/')
		return name;
	strcpy(fullpath, cur_path);
	strcat(fullpath, name);
	return fullpath;
}

int cmd_echo(char *args)
{
	if(args == NULL)
		return 0;
	char *str = args;
	while(*str && *str != '\\') {str ++;}

	if(*str == 0)
		printk("%s\n", args);
	else
	{
		*str = 0;
		str ++;
		uint32_t inode = creat(str);
		if(inode == INVALID_INODENO)
			return 0;
		
		INODE *pinode = open_inode(inode);
		fs_write(pinode, 0, strlen(args), args);
		pinode->filesz = strlen(args);
		close_inode(pinode);
	}
	return 0;
}

int cmd_ls(char *args)
{
	int i;
	TrapFrame tf;
	tf.ebx = 0;
	tf.ecx = 0;
	tf.edx = 0;
	char *oldargs = args;
	char *newargs = strtok(args, ' ');
	for(i = 0; i < 10; i++)
	{
		if(strcmp(oldargs, "-l") == 0)
			tf.ecx = 1;
		if(strcmp(oldargs, "-a") == 0)
			tf.edx = 1;
		else
			tf.ebx = (uint32_t)oldargs;
		oldargs = newargs;
		newargs = strtok(oldargs, ' ');
		if(oldargs == NULL)
			break;
	}
	list(&tf);
	return 0;
}

int cmd_cat(char *args)
{
	int i, j, re = 0;
	void write_char(char);
	char *str = args;
	while(*str && *str != '\\') {str ++;}

	if(*str == 0)
		re = 0;
	else
	{
		re = 1;
		*str = 0;
		str ++;
	}

	INODE *ptarinode;
	if(re)
	{
		uint32_t tarinode = creat(str);
		if(tarinode == INVALID_INODENO)
			return 0;
		ptarinode = open_inode(tarinode);
	}
	uint32_t inode = opendir(args);
	if(inode == INVALID_INODENO)
		return 0;
	char tbuf[4096];
	INODE *pinode = open_inode(inode);
	for(i = 0; i < pinode->filesz; i += 4096)
	{
		int sz = min(pinode->filesz - i, 4096);
		fs_read(pinode, 0, sz, tbuf);
		if(!re)
			for(j = 0; j < sz; j++)
				write_char(tbuf[j]);
		else
			fs_write(ptarinode, i, sz, tbuf);
	}
	if(re)
	{
		ptarinode->filesz = pinode->filesz;
		close_inode(ptarinode);
	}
	close_inode(pinode);
	return 0;
}

int cmd_touch(char *args)
{
	strtok(args, ' ');
	if(strcmp(args, "-c") == 0)
		return 0;
	creat(args);
	return 0;
}

int cmd_cd(char *args)
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
	{"cat", cmd_cat, "display file."},
	{"touch", cmd_touch, "create file."}
	{"cd", cmd_cd, "change directory."}
};

#define NR_CMD (sizeof(command_list) / sizeof(command_list[0]))

void command_helper()
{
	int i;
	char *args = strtok(history[head], ' ');
	for(i = 0; i < NR_CMD; i++)
	{
		if(strcmp(history[head], command_list[i].cmd) == 0)
		{
			command_list[i].func(args);
			break;
		}
	}
}

void update_buf(char ch)
{
	if(pos >= 100)
	{
		printk("error:%s, %s, line:%d, %d", __FILE__, __func__, __LINE__, pos);
		return;
	}
	if(pos > 0 && ch == '\b')
	{
		pos --;
		history[head][pos] = '\0';
		return;
	}
	if(ch == '\n')
	{
		history[head][pos ++] = '\0';
		command_helper();
		head = (head + 1) % 10;
		pos = 0;
		printk("/>");
		return;
	}
	history[head][pos ++] = ch;
}
