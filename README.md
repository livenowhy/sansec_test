#### 三未信安加密卡测试程序

  `1` 采用SM2加解密算法，测试硬件加解密和软解密效率
  `2` 如果硬件坏了或者无法连接，则自动调用软解密
  `3`
  `4`

####  测试原理及流程

  `1` 首先生成一个含有文件信息的链表，链表除了头节点外每一个节点指向一个还有需要操作的文件信息的结构体

    每一个_List_File_Mutex节点的数据域指向一个文件信息结构体，加解密操作是对这个文件结构体里面的文件进行的。一个线程对其明文文件进行加密操作，一个线程解密有加密线程加密之后的数据。
  `2` 
  `3` 



####  特殊文件

  每个加密卡项目都需要三个文件
	swsds.h    // 加密卡头文件,尽量不要修改该头文件
	swsds.ini  // 配置文件
	swsds.lib  // 库文件


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