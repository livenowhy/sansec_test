#### 三未信安加密卡测试程序

  `1` 采用SM2加解密算法，测试硬件加解密和软解密效率
  `2` 如果硬件坏了或者无法连接，则自动调用软解密
  `3` 
  `4`

####  测试原理及流程

  `1` 首先生成一个含有文件信息的链表，链表除了头节点外每一个节点指向一个还有需要操作的文件信息的结构体

    每一个_List_File_Mutex节点的数据域指向一个文件信息结构体，
    加解密操作是对这个文件结构体里面的文件进行的。
    一个线程对其明文文件进行加密操作，一个线程解密有加密线程加密之后的数据。
  `2` 
  `3` 

####  测试日志写入控制

  `1` 先生成日志信息链表，主进程中有一个专一线程对该链表进行遍历。
  `2`
  `3`
  `4`



####  特殊文件

  每个加密卡项目都需要三个文件
	swsds.h    // 加密卡头文件,尽量不要修改该头文件
	swsds.ini  // 配置文件
	swsds.lib  // 库文件

####  软算法
  编译、加载演示版驱动程序，演示版驱动程序针对SM2、SM3算法调用模拟硬件死掉，接口层第一次调用返回超时错误，后续调用自动调用软算法。可以通过硬件、软件算法性能指标以及CPU占用率观察算法调用是否由软件实现。

  第一次错误码是01010200，表示超时错误，认为密码卡死掉。

  0x02000002错误码表示密码卡死掉，接口内部针对SM2、SM3自动切换为软算法，但是类似获取设备信息等访问硬件的操作，直接返回错误。

  死掉的情况下调用软算法，类似会话句柄也会分配空间，用来存储类似哈希中间结果

  软算法的话，并发应该性能更高些，可以充分利用多核等硬件资源

  接口库中设置了硬件操作超时功能，应该是10秒，如果10秒不返回即认为硬件死掉返回特定错误码，后续的算法调用会自动切换为软算法





####  Create a new repository on the command line

	touch README.md
	git init
	git add README.md
	git commit -m "first commit"
	git remote add origin git@github.com:livenowhy/sansec_test.git
	git push -u origin master

####  Push an existing repository from the command line

	git remote add origin git@github.com:livenowhy/sansec_test.git
	git push -u origin master