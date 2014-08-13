/*
 * util.c
 * 测试程序需要使用的工具函数
 *  Created on: 2014年8月12日
 *      Author: love
 */

# include "mysansec.h"


extern void *hSessionHandle;
extern void *hSessionHandle;
extern unsigned int algorithm_id;  // 指定算法标识
extern unsigned int key_len;
extern ECCrefPublicKey public_key;
extern ECCrefPrivateKey private_key;

/**
 * Purpose: 初始化 _File_Mutex 文件信息结构体
 * file_st[in]: 创建的文件信息结构体，使用传入的plaintext_old, file_ciphertext, plaintext_new信息来初始化它
 * plaintext_old[in]:   原始明文文件名
 * file_ciphertext[in]: 原始明文加密之后的密文保存文件名
 * plaintext_new[in]:   密文解密得到的明文数据保存文件名
 * Return: 1 成功执行(返回值无意义)
 */
int init_file_mutex(struct _File_Mutex *file_st, char *plaintext_old, char *file_ciphertext, char *plaintext_new)
{
	strcpy(&(file_st->file_plaintext_old), plaintext_old);
	strcpy(&(file_st->file_ciphertext), file_ciphertext);
	strcpy(&(file_st->file_plaintext_new), plaintext_new);

	pthread_mutex_init(&(file_st->lock_plaintext_old), NULL);
	pthread_mutex_init(&(file_st->lock_ciphertext), NULL);

	pthread_mutex_lock(&(file_st->lock_ciphertext)); //  给密码文件上锁，防止还没有输出密码文件时，解码线程开始运行.暂时这样处理，但是这样处理有些问题，需要采用pthread_cond_t来处理

	pthread_mutex_init(&(file_st->lock_plaintext_new), NULL);

	pthread_cond_init(&(file_st->cond_ciphertext), NULL);  // 暂时不用

	return 0;
}


/**
 * 初始化文件链表的头节点, num_node是代表链表中含有执行文件信息结构体节点的个数。(即之后要开启加解密线程的个数)
 * num_node：由argv[2] 传入
 */
struct _Head_File_Mutex *init_head_file_mutex(int num_node)
{
	struct _Head_File_Mutex * head  = NULL;
	if( NULL == (head = (struct _Head_File_Mutex *)malloc(sizeof(struct _Head_File_Mutex))))
		return NULL;
	head->num_node = num_node;
	head->next = NULL;
	return head;
}

/**
 * 根据i的值来自动造生成i个原始明文文件,并且响应生成i个密文和i个解密文件的文件名字
 * 例如第100个加解密线程  使用的文件名依次是：100_old.txt, 100_cip.txt, 100_new.txt
 *
 * 生成一个文件信息节点，并返回节点指针
 */
struct _List_File_Mutex *make_test_file_and_init_list_file_mutex_node_and_ret(int i)
{
	struct _List_File_Mutex *node = NULL; // 文件节点
	if(NULL == (node = (struct _List_File_Mutex *)malloc(sizeof(struct _List_File_Mutex))))
		return NULL;

	node->next = NULL;
	//node->file_info

	char temp_old[64];
	char temp_cip[64];
	char temp_new[64];


	char command[512];

	sprintf(temp_old, "%d%s", i, "_old.txt"); // 原始明文文件名

	sprintf(command, "cp -r %s %s", "text.txt", temp_old);
	int ret;
	if(-1 == (ret = system(command)))
	{
		printf("原始明文拷贝失败 \n");
		return NULL;
	}

	sprintf(temp_cip, "%d%s", i, "_cip.txt"); // 加密的密文文件名
	sprintf(temp_new, "%d%s", i, "_new.txt"); // 解密密文得到的明文文件名


	init_file_mutex(&(node->file_info), temp_old, temp_cip, temp_new);

	return node;
}

/**
 * 头插法添加节点
 *
 */
int add_node_to_list(struct _Head_File_Mutex *head, struct _List_File_Mutex *node)
{
	node->next = head->next;
	head->next = node;
	return 0;
}

/**
 * 销毁链表
 */
int destroy_list(struct _Head_File_Mutex *head)
{
	struct _List_File_Mutex *index = head->next;
	struct _List_File_Mutex *temp = head->next;
	while(NULL != temp)
	{
		free(temp);
		temp = index->next;
		index = temp;
	}
	free(head);
	return 0;
}

/***
 * 加密线程的调用函数
 */
void *encrypt_func(void *argv)
{
	struct _File_Mutex *temp_file = argv;

	pthread_mutex_lock(&(temp_file->lock_plaintext_old)); //取得明文文件的访问权限(加锁或阻塞)

	struct timeval encrypt_begin_time, encrypt_stop_time;
	gettimeofday(&encrypt_begin_time, NULL);

	external_file_data_encrypt_ecc(hSessionHandle, algorithm_id, &public_key, temp_file->file_plaintext_old, temp_file->file_ciphertext);

	gettimeofday(&encrypt_stop_time, NULL);
	printf("%s 文件加密用时: time ---> %ld \n",temp_file->file_plaintext_old, encrypt_stop_time.tv_usec - encrypt_begin_time.tv_usec);
	pthread_mutex_unlock(&(temp_file->lock_ciphertext));   // 给密码文件解锁，使得解密进程开始运行
}


/**
 * 解密线程的执行调用函数
 */
void *decrypt_func(void *argv)
{
	struct _File_Mutex *temp_file = argv;
	pthread_mutex_lock(&(temp_file->lock_ciphertext)); //给密码文件上锁
	struct timeval decrypt_begin_time, decrypt_stop_time;
	gettimeofday(&decrypt_begin_time, NULL);
	external_file_data_decrypt_ecc(hSessionHandle, algorithm_id, &private_key, temp_file->file_plaintext_new, temp_file->file_ciphertext);
	gettimeofday(&decrypt_stop_time, NULL);
	printf("%s 文件解密用时: time ---> %ld \n",temp_file->file_ciphertext, decrypt_stop_time.tv_usec - decrypt_begin_time.tv_usec);
	pthread_mutex_unlock(&(temp_file->lock_plaintext_old));
}

/**
 * 为每一个文件信息节点创建加密线程
 */
int create_encrypt_pthread(struct _Head_File_Mutex *head, pthread_t *pid_encrypt)
{

	struct _List_File_Mutex *temp = NULL;
	temp = head->next;
	struct _List_File_Mutex *index = NULL;
	index = head->next;
	int pid_num = 0;
	while(NULL != index)
	{
		pthread_create(&pid_encrypt[pid_num], NULL, encrypt_func, &(index->file_info)); // 加密
	//	pthread_join(pid_encrypt[pid_num],NULL);   // 等待线程结束
		pthread_join(pid_encrypt[pid_num],NULL);

		index = temp->next;
		temp = index;
		pid_num++;
	}
	return 0;
}

/**
 *  为每一个文件节点创建解密线程
 */
int create_decrypt_pthread(struct _Head_File_Mutex *head, pthread_t *pid_decrypt)
{
	struct _List_File_Mutex *temp = head->next;
	struct _List_File_Mutex *index = head->next;
	int pid_num = 0;
	while (NULL != index)
	{
		pthread_create(&pid_decrypt[pid_num], NULL, decrypt_func, &(index->file_info)); // 解密
		pthread_join(pid_decrypt[pid_num],NULL);   // 不可少的代码行
		index = temp->next;
		temp = index;
		pid_num++;
	}
	return 0;
}


/**
 * 标准错误码定义
 * 根据错误码输出错误信息，并且打印调用函数传入的msg信息
 */
int print_error_msg(int ret, char *msg)
{
	if (NULL != msg)
		printf("%s \n", msg);
	switch (ret) {
	case SDR_UNKNOWERR:
		printf("未知错误\n");
		break;

	case SDR_NOTSUPPORT:
		printf("不支持\n");
		break;

	case SDR_COMMFAIL:
		printf("通信错误\n");
		break;

	case SDR_HARDFAIL:
		printf("硬件错误\n");
		break;

	case SDR_OPENDEVICE:
		printf("打开设备错误\n");
		break;

	case SDR_OPENSESSION:
		printf("打开会话句柄错误\n");
		break;

	case SDR_PARDENY:
		printf("权限不满足\n");
		break;

	case SDR_KEYNOTEXIST:
		printf("密钥不存在\n");
		break;

	case SDR_ALGNOTSUPPORT:
		printf("不支持的算法\n");
		break;

	case SDR_ALGMODNOTSUPPORT:
		printf("不支持的算法模式\n");
		break;

	case SDR_PKOPERR:
		printf("公钥运算错误\n");
		break;

	case SDR_SKOPERR:
		printf("私钥运算错误\n");
		break;

	case SDR_SIGNERR:
		printf("签名错误\n");
		break;

	case SDR_VERIFYERR:
		printf("验证错误\n");
		break;

	case SDR_SYMOPERR:
		printf("对称运算错误\n");
		break;

	case SDR_STEPERR:
		printf("步骤错误\n");
		break;

	case SDR_FILESIZEERR:
		printf("文件大小错误或输入数据长度非法\n");
		break;

	case SDR_FILENOEXIST:
		printf("文件不存在\n");
		break;

	case SDR_FILEOFSERR:
		printf("文件操作偏移量错误\n");
		break;

	case SDR_KEYTYPEERR:
		printf("密钥类型错误\n");
		break;

	case SDR_KEYERR:
		printf("密钥错误\n");
		break;

		/*扩展错误码*/
	case SWR_BASE:
		printf("自定义错误码基础值\n");
		break;

	case SWR_INVALID_USER:
		printf("无效的用户名\n");
		break;

	case SWR_INVALID_AUTHENCODE:
		printf("无效的授权码\n");
		break;

	case SWR_PROTOCOL_VER_ERR:
		printf("不支持的协议版本\n");
		break;

	case SWR_INVALID_COMMAND:
		printf("错误的命令字\n");
		break;

	case SWR_INVALID_PARAMETERS:
		printf("参数错误或错误的数据包格式\n");
		break;

	case SWR_FILE_ALREADY_EXIST:
		printf("已存在同名文件\n");
		break;

	case SWR_SYNCH_ERR:
		printf("多卡同步错误\n");
		break;

	case SWR_SYNCH_LOGIN_ERR:
		printf("多卡同步后登录错误\n");
		break;

	case SWR_SOCKET_TIMEOUT:
		printf("超时错误\n");
		break;

	case SWR_CONNECT_ERR:
		printf("连接服务器错误\n");
		break;

	case SWR_SET_SOCKOPT_ERR:
		printf("设置Socket参数错误\n");
		break;

	case SWR_SOCKET_SEND_ERR:
		printf("发送LOGINRequest错误\n");
		break;

	case SWR_SOCKET_RECV_ERR:
		printf("发送LOGINRequest错误\n");
		break;

	case SWR_SOCKET_RECV_0:
		printf("发送LOGINRequest错误\n");
		break;

	case SWR_SEM_TIMEOUT:
		printf("超时错误\n");
		break;

	case SWR_NO_AVAILABLE_HSM:
		printf("没有可用的加密机\n");
		break;

	case SWR_NO_AVAILABLE_CSM:
		printf("加密机内没有可用的加密模块\n");
		break;

	case SWR_CONFIG_ERR:
		printf("配置文件错误\n");
		break;

		/*密码卡错误码*/
	case SWR_CARD_BASE:
		printf("密码卡错误码\n");
		break;

	case SWR_CARD_UNKNOWERR:
		printf("未知错误\n");
		break;

	case SWR_CARD_NOTSUPPORT:
		printf("不支持的接口调用\n");
		break;

	case SWR_CARD_COMMFAIL:
		printf("与设备通信失败\n");
		break;

	case SWR_CARD_HARDFAIL:
		printf("运算模块无响应\n");
		break;

	case SWR_CARD_OPENDEVICE:
		printf("打开设备失败\n");
		break;

	case SWR_CARD_OPENSESSION:
		printf("创建会话失败\n");
		break;

	case SWR_CARD_PARDENY:
		printf("无私钥使用权限\n");
		break;

	case SWR_CARD_KEYNOTEXIST:
		printf("不存在的密钥调用\n");
		break;

	case SWR_CARD_ALGNOTSUPPORT:
		printf("不支持的算法调用\n");
		break;

	case SWR_CARD_ALGMODNOTSUPPORT:
		printf("不支持的算法调用\n");
		break;

	case SWR_CARD_PKOPERR:
		printf("公钥运算失败\n");
		break;

	case SWR_CARD_SKOPERR:
		printf("私钥运算失败\n");
		break;

	case SWR_CARD_SIGNERR:
		printf("签名运算失败\n");
		break;

	case SWR_CARD_VERIFYERR:
		printf("验证签名失败\n");
		break;

	case SWR_CARD_SYMOPERR:
		printf("对称算法运算失败\n");
		break;

	case SWR_CARD_STEPERR:
		printf("多步运算步骤错误\n");
		break;

	case SWR_CARD_FILESIZEERR:
		printf("文件长度超出限制\n");
		break;

	case SWR_CARD_FILENOEXIST:
		printf("指定的文件不存在\n");
		break;

	case SWR_CARD_FILEOFSERR:
		printf("文件起始位置错误\n");
		break;

	case SWR_CARD_KEYTYPEERR:
		printf("密钥类型错误\n");
		break;

	case SWR_CARD_KEYERR:
		printf("密钥错误\n");
		break;

	case SWR_CARD_BUFFER_TOO_SMALL:
		printf("接收参数的缓存区太小\n");
		break;

	case SWR_CARD_DATA_PAD:
		printf("数据没有按正确格式填充，或解密得到的脱密数据不符合填充格式\n");
		break;

	case SWR_CARD_DATA_SIZE:
		printf("明文或密文长度不符合相应的算法要求\n");
		break;

	case SWR_CARD_CRYPTO_NOT_INIT:
		printf("该错误表明没有为相应的算法调用初始化函数\n");
		break;

		//01/03/09版密码卡权限管理错误码
	case SWR_CARD_MANAGEMENT_DENY:
		printf("管理权限不满足\n");
		break;

	case SWR_CARD_OPERATION_DENY:
		printf("操作权限不满足\n");
		break;

	case SWR_CARD_DEVICE_STATUS_ERR:
		printf("当前设备状态不满足现有操作\n");
		break;

	case SWR_CARD_LOGIN_ERR:
		printf("登录失败\n");
		break;

	case SWR_CARD_USERID_ERR:
		printf("用户ID数目/号码错误\n");
		break;

	case SWR_CARD_PARAMENT_ERR:
		printf("参数错误\n");
		break;

		//05/06版密码卡权限管理错误码
	case SWR_CARD_MANAGEMENT_DENY_05:
		printf("管理权限不满足\n");
		break;

	case SWR_CARD_OPERATION_DENY_05:
		printf("操作权限不满足\n");
		break;

	case SWR_CARD_DEVICE_STATUS_ERR_05:
		printf("当前设备状态不满足现有操作\n");
		break;

	case SWR_CARD_LOGIN_ERR_05:
		printf("登录失败\n");
		break;

	case SWR_CARD_USERID_ERR_05:
		printf("用户ID数目/号码错误\n");
		break;

	case SWR_CARD_PARAMENT_ERR_05:
		printf("参数错误\n");
		break;

		/*读卡器错误*/
	case SWR_CARD_READER_BASE:
		printf("读卡器类型错误\n");
		break;

	case SWR_CARD_READER_PIN_ERROR:
		printf("口令错误\n");
		break;

	case SWR_CARD_READER_NO_CARD:
		printf("IC未插入\n");
		break;

	case SWR_CARD_READER_CARD_INSERT:
		printf("IC插入方向错误或不到位\n");
		break;

	case SWR_CARD_READER_CARD_INSERT_TYPE:
		printf("IC类型错误\n");
		break;
	default:
		printf("未知错误码--------\n");
		break;
	}
	printf("错误码 ----> %x,%d",ret, ret);
	return ret;
}


/**
 * 功能：以二进制的形式打印sourceData中的内容
 * itemName打印的名称，dataLength为sourceData指针指向数据的长度，rowCount一行打印十六进制的个数
 * 当rowCount = dataLength时只打印一行；rowCount = 1时，打印dataLength行
 */
int PrintData(char *itemName, unsigned char *sourceData, unsigned int dataLength, unsigned int rowCount)
{
	int i, j;

	if ((sourceData == NULL) || (rowCount == 0) || (dataLength == 0))
		return -1;

	if (itemName != NULL)
		printf("%s[%d]:\n", itemName, dataLength);

	for (i = 0; i < (int) (dataLength / rowCount); i++) // 打印前i = (int) (dataLength / rowCount)行
	{
		printf("%08x  ", i * rowCount);   //  行标，第几行
		for (j = 0; j < (int) rowCount; j++) {
			printf("%02x ", *(sourceData + i * rowCount + j));
		}
		printf("\n");
	}
	if (!(dataLength % rowCount))
		return 0;

	printf("%08x  ", (dataLength / rowCount) * rowCount); //  行标，第几行
	for (j = 0; j < (int) (dataLength % rowCount); j++) // 打印剩下不足一行的数据，个数为j = (int)(dataLength % rowCount)
	{
		printf("%02x ", *(sourceData + (dataLength / rowCount) * rowCount + j));
	}
	printf("\n");
	return 0;
}

/**
 * 保存数据到文件
 * filename:保存的文件名
 * mode:保存时用的文件格式
 * buffer:需要保存数据的指针
 * size:buffer的数据长度，必须传入，buffer可能没有数据结束标志
 */
int FileWrite(char *filename, char *mode, unsigned char *buffer, size_t size)
{
	FILE *fp;
	int rw, rwed;

	if ((fp = fopen(filename, mode)) == NULL)
		return 0;

	rwed = 0; // 控制每次写入文件的字符个数,此处rwed = 0进行控制一次就把所有数据写入到文件。
	while (size > rwed)
	{
		if ((rw = fwrite(buffer + rwed, 1, size - rwed, fp)) <= 0)
		{
			break;
		}
		rwed += rw;
	}
	fclose(fp);
	return rwed;
}

int FileRead(char *filename, char *mode, unsigned char *buffer, size_t size)
{
	FILE *fp;
	int rw, rwed;

	if ((fp = fopen(filename, mode)) == NULL)
		return 0;

	rwed = 0;
	while ((!feof(fp)) && (size > rwed)) {
		if ((rw = fread(buffer + rwed, 1, size - rwed, fp)) <= 0) {
			break;
		}
		rwed += rw;
	}
	fclose(fp);
	return rwed;
}

