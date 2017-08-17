# block/nonblock

------
阻塞模式、非阻塞模式的区别

------

参考 [浅谈TCP/IP网络编程中socket的行为](http://www.cnblogs.com/promise6522/archive/2012/03/03/2377935.html)
### 1. 为什么会阻塞？
```C
#include <unistd.h>
ssize_t write(int fd, const void *buf, size_t count);

首先，write成功返回，只是buf中的数据被复制到了kernel中的TCP发送缓冲区。
至于数据什么时候被发往网络，什么时候被对方主机接收，什么时候被对方进程读取，系统调用层面不会给予任何保证和通知
```
```
当kernel的该socket的发送缓冲区已满时，write操作就会阻塞。

对于每个socket，拥有自己的send buffer和receive buffer。

已经发送到网络的数据依然需要暂存在send buffer中，只有收到对方的ack后，kernel才从buffer中清除这一部分数据，
为后续发送数据腾出空间。

接收端将收到的数据暂存在receive buffer中，自动进行确认。但如果socket所在的进程不及时将数据从receive buffer中取出，
最终导致receive buffer填满，由于TCP的滑动窗口和拥塞控制，接收端会阻止发送端向其发送数据。

这些控制皆发生在TCP/IP栈中，对应用程序是透明的，应用程序继续发送数据，最终导致send buffer填满，write调用阻塞。
```
```
一般来说，由于接收端进程从socket读数据的速度跟不上发送端进程向socket写数据的速度，最终导致发送端write调用阻塞。

而read调用的行为相对容易理解，从socket的receive buffer中拷贝数据到应用程序的buffer中。read调用阻塞，通常是发送端的数据没有到达。
```
