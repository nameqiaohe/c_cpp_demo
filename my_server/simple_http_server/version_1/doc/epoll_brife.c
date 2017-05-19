/*####################################################
# File Name: epoll_brife.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-19 23:39:26
# Last Modified: 2017-05-20 00:09:18
####################################################*/

参考：http://blog.csdn.net/xiajun07061225/article/details/9250579

1. int epoll_create(int size);
	创建一个epoll的句柄。自从linux2.6.8之后，size参数是被忽略的。需要注意的是，当创建好epoll句柄后，它就是会占用一个fd值，在linux下如果查看/proc/进程id/fd/，是能够看到这个fd的，所以在使用完epoll后，必须调用close()关闭，否则可能导致fd被耗尽。

2、int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
	epoll的事件注册函数，它不同于select()是在监听事件时告诉内核要监听什么类型的事件，而是在这里先注册要监听的事件类型。
	第一个参数是epoll_create()的返回值。
	第二个参数表示动作，用三个宏来表示：
		EPOLL_CTL_ADD：注册新的fd到epfd中；
		EPOLL_CTL_MOD：修改已经注册的fd的监听事件；
		EPOLL_CTL_DEL：从epfd中删除一个fd；
 
	第三个参数是需要监听的fd。
	第四个参数是告诉内核需要监听什么事，struct epoll_event结构如下：
		//保存触发事件的某个文件描述符相关的数据（与具体使用方式有关）
		typedef union epoll_data {  
		    void *ptr;  
		    int fd;  
		    __uint32_t u32;  
		    __uint64_t u64;  
		} epoll_data_t;  
		//感兴趣的事件和被触发的事件  
		struct epoll_event {  
			__uint32_t events; /* Epoll events */  
			epoll_data_t data; /* User data variable */  
		};  

	events可以是以下几个宏的集合：
		EPOLLIN ：表示对应的文件描述符可以读（包括对端SOCKET正常关闭）；
		EPOLLOUT：表示对应的文件描述符可以写；
		EPOLLPRI：表示对应的文件描述符有紧急的数据可读（这里应该表示有带外数据到来）；
		EPOLLERR：表示对应的文件描述符发生错误；
		EPOLLHUP：表示对应的文件描述符被挂断；
		EPOLLET： 将EPOLL设为边缘触发(Edge Triggered)模式，这是相对于水平触发(Level Triggered)来说的。
		EPOLLONESHOT：只监听一次事件，当监听完这次事件之后，如果还需要继续监听这个socket的话，需要再次把这个socket加入到EPOLL队列里

3、int epoll_wait(int epfd, struct epoll_event * events, int maxevents, int timeout);
	收集在epoll监控的事件中已经发生的事件。
	参数：
		参数events是分配好的epoll_event结构体数组，epoll将会把发生的事件赋值到events数组中
			（events不可以是空指针，内核只负责把数据复制到这个events数组中，不会去帮助我们在用户态中分配内存）。
		
		参数maxevents告之内核这个events有多大，这个 maxevents的值不能大于创建epoll_create()时的size
		参数timeout是超时时间（毫秒，0会立即返回，-1将不确定，也有说法说是永久阻塞）。
	返回值：
		如果函数调用成功，返回对应I/O上已准备好的文件描述符数目，如返回0表示已超时。
