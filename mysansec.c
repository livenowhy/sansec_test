/*
 * mysansec.c
 * sansec封装函数
 *  Created on: 2014年8月12日
 *      Author: love
 */


# include "mysansec.h"

/**
 * 功能：打开设备并创建会话
 * phDeviceHandle：设备句柄(二级指针)
 * phSessionHandle：会话句柄(二级指针)
 * 必须传入二级指针
 */
int open_device_and_session(void **phDeviceHandle, void **phSessionHandle) {
	int ret;
	if (SDR_OK != (ret = SDF_OpenDevice(phDeviceHandle))) {
		print_error_msg(ret, "打开设备失败");
		return ret;
	}

	if (SDR_OK != (ret = SDF_OpenSession(&phDeviceHandle, phSessionHandle))) {
		print_error_msg(ret, "打开会话失败");
		return ret;
	}
	return ret;
}

///**
// * 获取设备信息，传入会话句柄，传出设备信息
// */
//int get_device_info(void **phSessionHandle, void *pstDeviceInfo) {
//	int ret;
//	if (SDR_OK != (ret = SDF_GetDeviceInfo(&phSessionHandle, pstDeviceInfo))) {
//		print_error_msg(ret, "获取设备信息失败");
//		return ret;
//	}
//	return ret;
//}
//
//int GetDeviceInfoTest(SGD_HANDLE hSessionHandle)
//{
//	int rv;
//	unsigned char sFirmwareVersion[32] = { 0 };
//	unsigned int uiFirmwareVersionLen = 32;
//	unsigned char sLibraryVersion[16] = { 0 };
//	unsigned int uiLibraryVersionLen = 16;
//
//	DEVICEINFO stDeviceInfo;
//
//
//	printf("获取设备信息测试:\n");
//
//
////获取设备信息
//	rv = SDF_GetDeviceInfo(hSessionHandle, &stDeviceInfo);
//	if (rv != SDR_OK) {
//		printf("获取设备信息错误，错误码[0x%08x]\n", rv);
//	} else {
//		printf("获取设备信息成功。\n");
//		printf("\n");
//		printf("    |     项目      |   返回值  \n");
//		printf(
//				"   _|_______________|______________________________________________________\n");
//		printf("   1|   生产厂商    | %s\n", stDeviceInfo.IssuerName);
//		printf("   2|   设备型号    | %s\n", stDeviceInfo.DeviceName);
//		printf("   3|  设备序列号   | %s\n", stDeviceInfo.DeviceSerial);
//		printf("   4|   设备版本    | v%08x\n", stDeviceInfo.DeviceVersion);
//		printf("   5| 支持标准版本  | v%d\n", stDeviceInfo.StandardVersion);
//		printf("   6| 支持公钥算法  | %08x | %08x\n", stDeviceInfo.AsymAlgAbility[0],
//				stDeviceInfo.AsymAlgAbility[1]);
//		printf("   7| 支持对称算法  | %08x\n", stDeviceInfo.SymAlgAbility);
//		printf("   8| 支持杂凑算法  | %08x\n", stDeviceInfo.HashAlgAbility);
//		printf("   9| 用户存储空间  | %dKB\n", stDeviceInfo.BufferSize >> 10);
//	}
//
//	printf("\n");
//
////获取固件版本
//	rv = SDF_GetFirmwareVersion(hSessionHandle, sFirmwareVersion,
//			&uiFirmwareVersionLen);
//	if (rv != SDR_OK) {
//		printf("获取设备固件版本信息错误，错误码[0x%08x]\n", rv);
//	} else {
//		printf("设备固件版本：%s\n", sFirmwareVersion);
//	}
//
////获取软件库版本
//	rv = SDF_GetLibraryVersion(hSessionHandle, sLibraryVersion,
//			&uiLibraryVersionLen);
//	if (rv != SDR_OK) {
//		printf("获取软件库版本错误， 错误码[0x%08x]\n", rv);
//	} else {
//		printf("设备软件版本：%s\n", sLibraryVersion);
//	}
//
//	printf("\n");
//
//	return 1;
//}
//

/**
 * 传入设备结构体指针
 */
int print_device_info(DEVICEINFO *p_device_info)
{
	DEVICEINFO device_info = *p_device_info;

	printf("获取设备信息成功。\n");
	printf("\n");
	printf("    |     项目      |   返回值  \n");
	printf(
			"   _|_______________|______________________________________________________\n");
	printf("   1|   生产厂商    | %s\n", device_info.IssuerName);
	printf("   2|   设备型号    | %s\n", device_info.DeviceName);
	printf("   3|  设备序列号   | %s\n", device_info.DeviceSerial);
	printf("   4|   设备版本    | v%08x\n", device_info.DeviceVersion);
	printf("   5| 支持标准版本  | v%d\n", device_info.StandardVersion);
	printf("   6| 支持公钥算法  | %08x | %08x\n", device_info.AsymAlgAbility[0], device_info.AsymAlgAbility[1]);
	printf("   7| 支持对称算法  | %08x\n", device_info.SymAlgAbility);
	printf("   8| 支持杂凑算法  | %08x\n", device_info.HashAlgAbility);
	printf("   9| 用户存储空间  | %dKB\n", device_info.BufferSize >> 10);

	return 0;
}

/**
 * 功能：关闭设备和会话
 */
int close_devices_and_session(void *hDeviceHandle, void *hSessionHandle) {
	int ret;

	if (SDR_OK != (ret = SDF_CloseSession(hSessionHandle))) {
		print_error_msg(ret, "关闭会话失败");
		return ret;
	}

	printf("会话已经关闭\n");

	if (SDR_OK != (ret = SDF_CloseDevice(hDeviceHandle))) {
		print_error_msg(ret, "关闭设备失败");
		return ret;
	}
	printf("设备已经关闭\n");
}

/**
 * 产生 ECC 密钥对并输出,请求密码设备产生指定类型和模长的 ECC 密钥对
 * hSessionHandle[in] 与设备建立的会话句柄
 * algorithm_id[in] 指定算法标识
 * key_len [in] 指定密钥长度
 * p_public_key[out] ECC 公钥结构
 * p_private_key[out] ECC 私钥结构
 */
int generate_key_pair_ecc(void *hSessionHandle, unsigned int algorithm_id, unsigned int key_len,
		ECCrefPublicKey *p_public_key, ECCrefPrivateKey *p_private_key)
{
	int ret;
	ret = SDF_GenerateKeyPair_ECC(hSessionHandle, algorithm_id, key_len,
			p_public_key, p_private_key);
	if (SDR_OK != ret)
	{
		print_error_msg(ret, "产生 ECC 密钥对失败");
		return ret;
	}
	return ret;
}

/**
 * 保存公钥数据和私钥数据到文件
 *
 */
int save_key_pair_ecc(ECCrefPublicKey *p_public_key, ECCrefPrivateKey *p_private_key)
{
	int ret;

	size_t public_key_len = sizeof(ECCrefPublicKey);
	size_t private_key_len = sizeof(ECCrefPrivateKey);

	ret = FileWrite("prikey_ecc", "wb+", (unsigned char *)p_public_key, public_key_len);
	if(public_key_len != ret)
	{
		print_error_msg(ret, "保存公钥失败");
		return ret;
	}
	ret = FileWrite("pubkey_ecc", "wb+", (unsigned char *)p_private_key, private_key_len);
	if(private_key_len != ret)
	{
		print_error_msg(ret, "保存私钥失败");
		return ret;
	}

	printf("保存成功\n");
	return ret;
}


/**
 * 功能：获取随机数
 * hSessionHandle[in] 与设备建立的会话句柄
 * random_length[in] 欲获取的随机数长度
 * random_out_buffer[out] 缓冲区指针，用于存放获取的随机数
 */
int get_generate_random(void *hSessionHandle, unsigned int random_length, unsigned char *random_out_buffer)
{
	int ret;
	ret = SDF_GenerateRandom(hSessionHandle, random_length, random_out_buffer);
	if(SDR_OK != ret)
	{
		print_error_msg(ret, "获取随机数失败");
		return ret;
	}
	return ret;
}





/**
 * 读取外部文件的数据进行加密，加密数据输出保存到一个新的文件
 * hSessionHandle[in] 与设备建立的会话句柄
 * algorithm_id[in] 算法标识，指定使用的 ECC 算法
 * p_public_key[in] 外部 ECC 公钥结构
 * plaintext_filename:需要加密的明文数据文件
 * ciphertext_filename:加密之后的密文保存文件
 * 中英文测试通过
 */
int external_file_data_encrypt_ecc(void *hSessionHandle, unsigned int algorithm_id, ECCrefPublicKey *p_public_key, char *plaintext_filename, char *ciphertext_filename)
{


	FILE *plaintext_fp, *ciphertext_fp;
	extern int errno;

	if ((plaintext_fp = fopen(plaintext_filename, "rb")) == NULL) //以只读二进制方式打开明文数据文件
	{
		printf("%s --> %s \n", plaintext_filename, strerror(errno));
		return 0;
	}

	if ((ciphertext_fp = fopen(ciphertext_filename, "wb")) == NULL) //只写打开或新建一个二进制文件,只允许写数据。密文
	{
		printf("%s --> %s \n", ciphertext_filename, strerror(errno));
		return 0;
	}

	unsigned char input_data[ECCref_MAX_LEN]; //用于临时存放从明文文件中取出来需要加密的数据
	ECCCipher output_enc_data; // 加密之后的数据
	int input_actual_len; //需要进行加密数据的真实长度

	int ret;

	int enc_data_len = sizeof(ECCCipher); //密文长度

	while((!feof(plaintext_fp)))  // 明文数据没有加密完，一直循环
	{
		if ((input_actual_len = fread(input_data, 1, ECCref_MAX_LEN, plaintext_fp)) <= 0)  //读取数据
			break;

		ret = SDF_ExternalEncrypt_ECC(hSessionHandle, algorithm_id, p_public_key, input_data, input_actual_len, &output_enc_data);
		if (SDR_OK != ret)
		{
			print_error_msg(ret, "加密失败");
			return ret;
		}

		if((ret = fwrite(&output_enc_data, 1, enc_data_len, ciphertext_fp)) <= 0)
		{
			printf("%s --> %s \n", ciphertext_filename, strerror(errno));
			break;

		}
		if(input_actual_len != ECCref_MAX_LEN)
			break;
	}

	fclose(plaintext_fp);
	fclose(ciphertext_fp);

	return 0;
}


/**
 * 读取外部密文文件的数据进行解密，解密数据保存另外一个文件
 *
 * 使用外部  ECC 私钥进行解密运算
 * hSessionHandle[in]: 与设备建立的会话句柄
 * algorithm_id[in]: 算法标识，指定使用的 ECC 算法
 * p_private_key[in]: 外部 ECC 私钥结构指针
 * plaintext_filename: 存放解密之后的数据文件
 * ciphertext_filename: 保存加密的密文数据的文件
 * 中英文测试通过
 */
int external_file_data_decrypt_ecc(void *hSessionHandle, unsigned int algorithm_id, ECCrefPrivateKey *p_private_key,
		char *plaintext_filename, char *ciphertext_filename)
{


	FILE *plaintext_fp, *ciphertext_fp;
	extern int errno;

	// 只写打开或新建一个二进制文件,只允许写数据。解密得到的明文
	if ((plaintext_fp = fopen(plaintext_filename, "wb")) == NULL)
	{
		printf("%s --> %s \n", plaintext_filename, strerror(errno));
		return 0;
	}

	if ((ciphertext_fp = fopen(ciphertext_filename, "rb")) == NULL) //以只读二进制方式打开密文数据文件
	{
		printf("%s --> %s \n", ciphertext_filename, strerror(errno));
		return 0;
	}

	unsigned char output_data[ECCref_MAX_LEN]; //用于临时存放从密文解密出来的明文数据
	ECCCipher input_enc_data; // 存放读取到的需要解密的密文数据
	int output_actual_len; //    解密之后得到的解密数据的真实长度

	int ret;

	int enc_data_len = sizeof(ECCCipher); //密文长度

	while((!feof(ciphertext_fp)))  // 明文数据没有加密完，一直循环
	{
		if ((output_actual_len = fread(&input_enc_data, 1, enc_data_len, ciphertext_fp)) <= 0)  //读取数据
			break;

		ret = SDF_ExternalDecrypt_ECC(hSessionHandle, algorithm_id, p_private_key, &input_enc_data,
				&output_data, &output_actual_len); // 解密
		if(SDR_OK != ret)
		{
			print_error_msg(ret, "解密成功");
			return ret;
		}

		if((ret = fwrite(&output_data, 1, output_actual_len, plaintext_fp)) <= 0)
		{
			printf("%s --> %s \n", plaintext_filename, strerror(errno));
			break;
		}
		if(output_actual_len != ECCref_MAX_LEN)
			break;
	}

	fclose(plaintext_fp);
	fclose(ciphertext_fp);

	return 0;
}



