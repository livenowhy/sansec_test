/*
 * log.c
 *
 *  Created on: 2014年8月13日
 *      Author: love
 *      程序运行log写入函数
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

#define MAX_LOG_MSG_DATA 512  //一条log信息的字符数
typedef struct _Log_Msg
{
	// 标记该log节点是否已经被填充了log信息
	char tag;  //  T代表该节点已经添加信息、F代表该节点为添加log信息
	int buf_len;  // 实际含有的信息字节
	char buf[MAX_LOG_MSG_DATA];   //   log信息
	struct _Log_Msg *next;  // 下一个信息节点
}log_msg;

typedef struct _Log_Msg_List
{
	int log_msg_num; // 该log链表含有多少个可以使用log节点信息
	struct _Log_Msg *next; // 第一个含有log的信息节点
}log_msg_list;

typedef struct _Log_Msg_List_Index
{
	int log_msg_num;  //当前添加到的位置
	pthread_mutex_t lock_list_index; // 正在添加log信息时，给该节点上锁
	struct _Log_Msg *next; // 第一个含有log的信息节点
}log_msg_list_index;

typedef struct _Log_Info
{
	struct _Log_Msg_List *log_list_head ;  // 链表头
	struct _Log_Msg_List_Index *log_list_write_index; // 正在操作读取节点信息并保存至文件的节点
	struct _Log_Msg_List_Index *log_list_add_index; // 指向正在添加信息节点的位置
}log_info;


/**
 * Return: NULL失败;
 */
struct _Log_Msg_List *init_point_node(int num)
{
	struct _Log_Msg_List *temp = NULL;
	if(NULL == (temp = (struct _Log_Msg_List *)malloc(sizeof(struct _Log_Msg_List))))
		return NULL;
	temp->next = NULL;
	return temp;
}

struct _Log_Msg_List_Index *init_point_index_node(int num)
{
	struct _Log_Msg_List_Index *temp = NULL;
	if(NULL == (temp = (struct _Log_Msg_List_Index *)malloc(sizeof(struct _Log_Msg_List_Index))))
		return NULL;
	pthread_mutex_init(&(temp->lock_list_index), NULL);
	temp->next = NULL;

	return temp;
}

struct _Log_Msg *init_log_msg(void)
{
	struct _Log_Msg *temp = NULL;
	temp = (struct _Log_Msg*)malloc(sizeof(struct _Log_Msg));
	if(NULL == temp)
		return temp;
	temp->next = NULL;
	temp->buf_len = 0;
	temp->tag = 'F';
	return temp;
}

int add_msg_to_list(struct _Log_Info *p_log_info_st, struct _Log_Msg *node)
{
	node->next = p_log_info_st->log_list_head->next;
	p_log_info_st->log_list_head->next = node;
	return 0;
}

//int init_log_list(int num, struct _Log_Msg_List **p_list_head, struct _Log_Msg_List_Index **p_write_index,struct _Log_Msg_List_Index **p_add_index)  // num --> log链表初始信息节点数
int init_log_list(int num, struct _Log_Info *p_log_info_st)  // num --> log链表初始信息节点数
{
	p_log_info_st->log_list_head = init_point_node(50);  //头节点出入的参数是该链表总共含有log信息节点的个数
	p_log_info_st->log_list_write_index = init_point_index_node(0); //  正在保存信息节点的位置
	p_log_info_st->log_list_add_index = init_point_index_node(0); //  正在添加信息的位置
	printf("%s,%d\n", __FUNCTION__, __LINE__);

	int j;
	struct _Log_Msg *node = NULL;
	for(j = 0; j < num; j++)
	{
		node = init_log_msg();
		add_msg_to_list(p_log_info_st, node);
	}

	p_log_info_st->log_list_write_index->next = p_log_info_st->log_list_head;
	p_log_info_st->log_list_add_index->next = p_log_info_st->log_list_head;

	if((NULL == p_log_info_st->log_list_head) || (NULL == p_log_info_st->log_list_write_index) ||(NULL == p_log_info_st->log_list_add_index))
	{
		printf("log链表构指向节点构造失败\n");
		return 0;
	}
	printf("%s,%d\n", __FUNCTION__, __LINE__);
	return 0;
}

//int add_log_msg_to_list(struct _Log_Msg_List_Index *add_index, char *msg, int msg_len) // 向链表的节点添加信息
int add_log_msg_to_list(struct _Log_Info *p_log_info_st, char *msg, int msg_len) // 向链表的节点添加信息
{
	pthread_mutex_lock(&(p_log_info_st->log_list_add_index->lock_list_index));

	if(NULL == p_log_info_st->log_list_add_index->next->next)
	{
		printf("NULL == add_index->next->next \n");

	}
	else
	{
		p_log_info_st->log_list_add_index->log_msg_num++;

		p_log_info_st->log_list_add_index->next->tag = 'T';


		p_log_info_st->log_list_add_index->next->buf_len = msg_len;
		printf("%s,%d\n", __FUNCTION__, __LINE__);
		strcpy(p_log_info_st->log_list_add_index->next->buf, msg);
		p_log_info_st->log_list_add_index->next = p_log_info_st->log_list_add_index->next->next;
		pthread_mutex_unlock(&(p_log_info_st->log_list_add_index->lock_list_index));

	}


	return 0;
}

int write_log_msg_to_file(unsigned char *path, struct _Log_Info *log_info_st)
{


	pthread_mutex_lock(&(log_info_st->log_list_write_index->lock_list_index));
	printf("%s,%d\n", __FUNCTION__, __LINE__);

	FILE *fp = NULL;
	extern int errno;
	if(NULL == (fp = fopen(path, "a+")))
	{
		printf("创建log文件失败\n");
		printf("%s \n", strerror(errno));

	}

	while('T' == log_info_st->log_list_write_index->next->tag)
	{
		fwrite(log_info_st->log_list_write_index->next->buf, 1, log_info_st->log_list_write_index->next->buf_len, fp);

		log_info_st->log_list_write_index->next->tag = 'F';
		memset(log_info_st->log_list_write_index->next->buf, 0, MAX_LOG_MSG_DATA);
		log_info_st->log_list_write_index->next = log_info_st->log_list_write_index->next->next;
		log_info_st->log_list_write_index->log_msg_num++;
	}
	fflush(fp);
	fclose(fp);
	pthread_mutex_unlock(&(log_info_st->log_list_write_index->lock_list_index));


		// size_t fwrite(const void* buffer, size_t size, size_t count, FILE* stream);

	return 0;
}

FILE *init_log_file(unsigned char *path)
{
	FILE *fp = NULL;
	extern int errno;
	if(NULL == (fp = fopen(path, "a+")))
	{
		printf("创建log文件失败\n");
		printf("%s \n", strerror(errno));

	}
	return fp;
}

int main(int argc, char **argv)
{
	struct _Log_Msg_List *log_list_head = NULL;  // 链表头
	struct _Log_Msg_List_Index *log_list_write_index = NULL; // 正在操作读取节点信息并保存至文件的节点
	struct _Log_Msg_List_Index *log_list_add_index = NULL; // 指向正在添加信息节点的位置

	struct _Log_Info log_info_st = {"NULL","NULL", "NULL"};

	init_log_list(50, &log_info_st);  // num --> log链表初始信息节点数



	int i;
	char temp_buf[8] = {'a', 'v', '3', '5', 'e', 't', '5', '\n'};
	//int add_log_msg_to_list(struct _Log_Msg_List_Index *add_index, char *msg, int msg_len)
	for(i = 0; i < 20; i++)
	{
		add_log_msg_to_list(&log_info_st, temp_buf, 8);
	}


	write_log_msg_to_file("loglog.txt", &log_info_st);
	printf("%s,%d\n", __FUNCTION__, __LINE__);



	return 0;
}
