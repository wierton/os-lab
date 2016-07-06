#include "common.h"
#include "shell/comarg.h"
#include "font/font.h"
#include "x86/x86.h"
#include "device/fs.h"
#include "math.h"
#include "proc/proc.h"

int user_mode = 0;
int head = 0, pos = 0, history_st = 0;
char history[10][100];
char cur_path[100] = "/";
static int history_head = 0;

int set_backst(int val);

char *full_path(char *name)
{
	static char fullpath[100];
	if(name && name[0] == '/')
		strcpy(fullpath, name);
	else
		strcpy(fullpath, cur_path);
	if(name == NULL)
		return fullpath;
	if(name[0] == ' ')
		name = strtok(name, ' ');
	if(strcmp(name, ".") != 0 && name[0] != '/')
		strcat(fullpath, name);
	return fullpath;
}

int cmd_echo(char *args)
{
	if(args == NULL)
		return 0;
	char *str = args;
	if(args[0] == ' ')
		args = strtok(args, ' ');
	str = strtok(args, '>');

	if(str == args)
		printk("%s\n", args);
	else
	{
		int off = 0;
		uint32_t inode = creat(full_path(str));
		if(inode == INVALID_INODENO)
			return 0;
		
		INODE *pinode = open_inode(inode);
		if(*(str - 2) == 0)
			off = pinode->filesz;
		fs_write(pinode, off, strlen(args), args);
		pinode->filesz = off + strlen(args);
		close_inode(pinode);
	}
	return 0;
}

int cmd_ls(char *args)
{
	int i;
	TrapFrame tf;
	tf.ebx = (uint32_t)full_path(NULL);
	tf.ecx = 0;
	tf.edx = 0;

	if(args == NULL)
	{
		list(&tf);
		return 0;
	}
	char *oldargs = args;
	char *newargs = strtok(args, ' ');
	for(i = 0; i < 4; i++)
	{
		if(strcmp(oldargs, "-l") == 0)
			tf.ecx = 1;
		else if(strcmp(oldargs, "-a") == 0)
			tf.edx = 1;
		else
		{
			int len = strlen(oldargs);
			if(strcmp(oldargs, ".") != 0 && oldargs[len - 1] != '/')
			{
				oldargs[len] = '/';
				oldargs[len + 1] = '\0';
			}
			tf.ebx = (uint32_t)full_path(oldargs);
		}
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
	if(args && args[0] == ' ')
		args = strtok(args, ' ');
	str = strtok(args, '>');

	if(str == args)
		re = 0;
	else
		re = 1;

	int off = 0;
	INODE *ptarinode = NULL;
	if(re)
	{
		uint32_t tarinode = creat(full_path(str));
		if(tarinode == INVALID_INODENO)
			return 0;
		ptarinode = open_inode(tarinode);
		if(*(str - 2) == 0)
			off = ptarinode->filesz;
	}
	uint32_t inode = opendir(full_path(args));
	if(inode == INVALID_INODENO)
		return 0;
	char tbuf[4096];
	INODE *pinode = open_inode(inode);
	for(i = 0; i < pinode->filesz; i += 4096)
	{
		int sz = min(pinode->filesz - i, 4096);
		fs_read(pinode, i, sz, tbuf);
		if(!re)
			for(j = 0; j < sz; j++)
				write_char(tbuf[j]);
		else
			fs_write(ptarinode, off + i, sz, tbuf);
	}
	if(re)
	{
		ptarinode->filesz = off + pinode->filesz;
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
	creat(full_path(args));
	return 0;
}

int cmd_cd(char *args)
{
	if(args == NULL)
		return 0;
	int len = strlen(args);
	char tpath[100] = {0};
	if(strcmp(args, ".") == 0)
		return 0;
	if(strcmp(args, "..") == 0 && strcmp(cur_path, "/") != 0)
	{
		int i;
		len = strlen(cur_path);
		cur_path[len - 1] = 0;
		for(i = len - 1; i > 0; i--)
			if(cur_path[i] != '/')
				cur_path[i] = '\0';
			else
				break;
		set_backst(strlen(cur_path) + 1);
		return 0;
	}
	if(args[len - 1] != '/')
	{
		args[len] = '/';
		args[len + 1] = '\0';
	}
	if(args[0] != '/')
		strcpy(tpath, cur_path);
	strcat(tpath, args);
	if(opendir(tpath) != INVALID_INODENO)
	{
		strcpy(cur_path, tpath);
		set_backst(strlen(cur_path) + 1);
	}
	return 0;
}

int cmd_exec(char *args)
{
	if(args == NULL)
		return 0;
	if(args[0] == '.')
		args += 2;
	if(opendir(full_path(args)) == INVALID_INODENO)
	{
		printk("'%s' command not found\n", args);
		return 0;
	}
	ProcAttr pa = {3};
	HANDLE hProc = create_proc(full_path(args), &pa);
	extern int forbid_switch;
	user_mode = 1;
	forbid_switch = 0;
	enter_proc(hProc);
	return 0;
}

int cmd_ps(char *args)
{
	void show_queue(int id);
	show_queue(-1);
	show_queue(0);
	show_queue(1);
	return 0;
}

int cmd_help(char *args);

typedef struct {
	char *cmd;
	int (* func)(char *args);
	char *helpinfo;
} Command_list;

Command_list command_list[] = {
	{"help", cmd_help, "help."},
	{"echo", cmd_echo, "type string on screen."},
	{"ls", cmd_ls, "list the file and directory."},
	{"cat", cmd_cat, "display file."},
	{"touch", cmd_touch, "create file."},
	{"cd", cmd_cd, "change directory."},
	{"ps", cmd_ps, "process list."},
	{"./", cmd_exec, "execute program."}
};

#define NR_CMD (sizeof(command_list) / sizeof(command_list[0]))

int cmd_help(char *args)
{
	int i;
	for(i = 0; i < NR_CMD; i++)
	{
		printk("%s\t%s\n", command_list[i].cmd, command_list[i].helpinfo);
	}
	return 0;
}

void command_helper(char *his)
{
	int i;
	char tmp[50];
	strcpy(tmp, his);
	char *args = strtok(tmp, ' ');

	if(args == tmp)
		args = NULL;
	for(i = 0; i < NR_CMD; i++)
	{
		if(strcmp(tmp, command_list[i].cmd) == 0)
		{
			command_list[i].func(args);
			return;
		}
	}
	cmd_exec(tmp);
}

void update_buf(int ch)
{
	if(ch == '\0')
		return;

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
		if(pos > 1)
		{
			head = (head + 1) % 10;
			if(head == history_st)
				history_st = (history_st + 1) % 10;
			history_head = head;
			pos = 0;
			/**/
			command_helper(history[(head + 9) % 10]);
		}
		history_head = head;
		pos = 0;
		printk("%s>", cur_path);
		return;
	}
	history[head][pos ++] = ch;
}

void call_history(int ch)
{
	if(ch == KEY_UP && history_head != history_st)
	{
		printk("\r                                        ");
		history_head = (history_head + 9) % 10;
		if(history[history_head][0] != '\0')
			printk("\r%s>%s", cur_path, history[history_head]);
		strcpy(history[head], history[history_head]);
		pos = strlen(history[head]);
	}
	if(ch == KEY_DOWN && history_head != head)
	{
		printk("\r                                        ");
		history_head = (history_head + 1) % 10;
		if(history[history_head][0] != '\0')
			printk("\r%s>%s", cur_path, history[history_head]);
		strcpy(history[head], history[history_head]);
		pos = strlen(history[head]);
	}
}
