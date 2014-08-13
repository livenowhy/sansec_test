/*
 * mysansec.h
 *
 *  Created on: 2014年8月12日
 *      Author: love
 */

#ifndef MYSANSEC_H_
#define MYSANSEC_H_


#include <stdio.h>

#include "swsds.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include <stdio.h>
#include <errno.h>






/**
 * 加解密操作的文件结构体
 */
typedef struct _File_Mutex
{
	char file_plaintext_old[64];   // 原始的明文文件
	char file_ciphertext[64];      // 加密之后的文件
	char file_plaintext_new[64];   // 由加密文件解密得到的明文文件
	pthread_mutex_t lock_plaintext_old;    // 需要加密的明文文件的访问锁
	pthread_mutex_t lock_ciphertext;       // 密码文件的访问锁
	pthread_mutex_t lock_plaintext_new;    // 解密得到的明文文件的访问锁

	pthread_cond_t cond_ciphertext;  // 暂时不用
}file_mutex;


typedef struct _List_File_Mutex    // 链表节点
{
	struct _File_Mutex file_info;  // 实际的
	struct _List_File_Mutex *next;
}list_file_mutex;

typedef struct _Head_File_Mutex   //  链表头节点
{
	int num_node;
	struct _List_File_Mutex *next;
}head_file_mutex;






#endif /* MYSANSEC_H_ */
