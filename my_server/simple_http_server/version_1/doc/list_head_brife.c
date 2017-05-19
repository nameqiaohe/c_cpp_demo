/*####################################################
# File Name: list_head_brife.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-18 17:24:14
# Last Modified: 2017-05-18 17:37:16
####################################################*/

#define container_of(ptr, type, member) ({ \
		    const typeof(((type *)0)->member) *__mptr = (ptr); \
		    (type *)((char *)__mptr - offsetof(type, member)); \
		})
作用：返回包含某成员的结构的入口地址
( 先将 member域的地址存储到 __mptr，然后计算出 member域相对于 type结构的偏移地址，再用 member域的地址 减去 偏移地址，结果就是 type结构的首地址 )


分解如下：
	1、const typeof( ((type *)0)->member ) *__mptr = (ptr);
	解释：
		(1) sizeof()是返回变量类型的的大小，typeof()作用是返回变量的类型。
		(2) ((type *)0) 
			把0强制转化为指向type类型的指针
		(3) typeof( ((type *)0)->member )
			返回type结构中member成员的类型

		(4) __mptr是一个指向与member的类型相同的指针，并把ptr赋值给__mptr

	2、#define offsetof(TYPE, MEMBER) ((size_t) & ((TYPE *)0)->MEMBER )
	作用：求结构体中一个成员在该结构体中的偏移量
	解释：
		( (TYPE *)0 ) 将零转型为TYPE类型指针;
		((TYPE *)0)->MEMBER 访问结构中的数据成员;
		&( ( (TYPE *)0 )->MEMBER )取出数据成员的地址;
		(size_t)(&(((TYPE*)0)->MEMBER))结果转换类型；
		巧妙之处在于将0转换成(TYPE*)，结构以内存空间首地址0作为起始地址，则成员地址自然为偏移地址。
