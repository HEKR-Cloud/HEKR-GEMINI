( GEMINI for Arduino )  Limited Memory Scheme System
============================

lmss被设计用于运行在内存受限的环境下，例如内存资源极其紧缺的单片机
系统，本例首先以Arduino Uno开发板作为首选测试运行环境。

代码结构
------

```
arduino --- Arduino转接
common.cpp --- 主程序模板
global.cpp --- 全局定义
inoint.cpp --- 中断处理
vm.cpp --- 虚拟机，包含evaluation和parsing接受到的string形式的字符串
Arduino-Makefile
debug.cpp --- 调试信息输出
init.cpp --- 初始化
lmss.h --- 多合一头文件，包含所有所需的内容
ref.cpp --- 引用结构
args.cpp --- 参数传递

inobind.cpp --- Arduino标准绑定
parser.cpp -- IO和语法分析辅助
stdbind.cpp -- 标准绑定
```

使用指南
------

使用所谓Arduino语言，本质是C++，方法是在基本的框架上中填写代码，框架为：
```
void setup()
{
}

void loop()
{
}
```

setup为启动初始化的初始化函数。loop为运行过程中反复执行的函数。

而使用lmss时，首先要导入lmss的所有定义，使用：
```
#include "lmss.h"
```

接着在void中初始化必要的数据：
```
void setup()
{
    /* Initialize */
    lmss_init();
    /* Setup Stdlib */
    lmss_stdlib_binds();
    /* Setup lmss Arduino Binds */
    lmss_ino_binds();
    /* Install Interrupt Handler */
    lmss_ino_interrupt_init();

}
```

完成以上操作后，准备操作就已经完成，此时可以开始执行Scheme代码，执行代码使用vm对象的eval方法。
以一个典型的初始化串口的代码为例：

```
    const char *s = ""
        "(serial-begin 9600)"
        "(pin-mode led-builtin output)"
        ;
    u8 len = strlen(s);
    vm->eval(s, len);
```

首先定义一个存放了代码的字符串，然后将其传递给vm对象的eval方法。


绑定Native函数
------------
原生的Arduino代码是类似C/C++的原始代码，直接操作硬件需要使用C/C++编写的函数，而将这些函数绑定在lmss中方便用Scheme代码调用是可能的，方法是使用vm对象的bind方法。以上文的serial-begin函数为例：打开串口通信的函数是Serial.begin(baud)，但是该函数是C/C++的Native函数，所以需要另外写一个函数封装它以实现Scheme和C/C++的通信。

```
/* serial-begin() */
void proc_serial_begin(lmssArgs *args)
{
    s16 baud = args->get_int();
    Serial.begin(baud);
}
```

每个这样用于封装Native函数的“转接”函数签名都类似，没有返回值，同时有一个lmssArgs *类型的参数。
该参数指向的类有一些方法用于通信，包括获取传递进来的参数，比如获取传入的整数型参数使用get_int方法。
其它类似的方法还有：
- get_int
- get_bool
- get_proc
等等，在args.cpp里有详细实现。

将运算结果返回给lmss的Scheme语言则可以使用ret_int, ret_bool, ret_proc等方法。

编写“转接函数”完毕后，要让lmss的虚拟机看到，则需要使用vm对象的bind_proc方法，该方法包括三个参数：绑定的变量名文本，绑定的变量名长度，绑定的函数指针，例如：
```
vm->bind_proc("serial-begin", 12, &proc_serial_begin);
```

这样就可以实现绑定。


绑定中断
-------

中断处理程序是一种特殊的程序，当注册后会在被触发时自动跳转到并且执行，Scheme作为运行在C之上的另一层系统，中断触发时会暂停Scheme系统的运行，此时无法对Scheme系统做任何安全的操作，于是采用另一种策略，即中断到来时记录中断，一旦可以安全访问虚拟机的时候主动检查是否存在被触发的中断并且进行处理。

```
/* attach_interrupt(num:int, isr:proc, mode:int) */
void proc_attach_int(lmssArgs *args)
{
    u8 num = args->get_int();
    u8 isr = args->get_proc();
    u8 mode = args->get_int();
    void (*func)(void) = NULL;

    /* Native bind */
    switch (num)
    {
        case 0: func = native_isr_transporter_0; break;
        case 1: func = native_isr_transporter_1; break;
        default: return;
    }
    attachInterrupt(num, func, mode);
    interrupt_vector->attach(num, isr);
}
```

空闲时间处理中断可放在loop中：

```C
/* Initialize interrupt 
 * Use in loop() */
void lmss_ino_interrupt_process(void)
{
    Obj obj;

    for (;;)
    {
        obj = interrupt_vector->get_interrupt_event();
        if (obj.type() == OBJ_NIL) break;

        vm->eval_blank_lambda_proc(obj);
    }
}
```

以我们要注册一个中断为例，注册在0号中断，触发中断的方式为change，并且显示一串字符：

```C
 "(attach-int 0 (lambda () (display \"interrupted\")) change)"
```

数据编码
------

因为内存极端受限，所以所有的对象都编码为一个8 bit的整数。8 bit只能表示256种不同的状态。编码的方式记录在ref.cpp中：

```
/*
 * +----------------------------------+
 * |  8 bits    | Category            |
 * +----------------------------------+
 * |  0 ~ 89    | numbers             |
 * |  90        | not match           |
 * |  91        | error               |
 * |  92        | emptylist           |
 * |  93        | nil                 |
 * |  94        | #f                  |
 * |  95        | #t                  |
 * |            |                     |
 * |   96 ~ 127 | 'big' numbers (32)  |
 * |  128 ~ 159 | primitive procs (32)|
 * |  160 ~ 175 | lambda proc (16)    |
 * |  176 ~ 223 | pairs (48)          |
 * |  224 ~ 255 | sym+str (32)        |
 * +----------------------------------+
 */
 ```
 
其中大整数、函数、pair、字符串和符号为引用类型，这样它们就可以用8 bit的整数的一个数值进行表示。这些引用类型的定义都在ref.cpp中，可以根据实际项目需求进行调整。

