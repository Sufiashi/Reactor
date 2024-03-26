# Reactor

2024/2/17
    用epoll模型完成了基本的网络通信服务端.
    1.InetAddress类封装了socket的地址协议类，可以用成员函数访问网络的ip,port和类的地址协议对象
        可以用ip和port创建类对象；用地址协议类创建类对象；用地址协议类设置类对象
    2.Socket类封装了socket的句柄，可以用成员函数设置句柄的属性，还可以用来listen,bind,accept，返回句柄fd
        用句柄fd创建Socket类

2024/2/19
    创建了Epoll类和Channel类。
    1.Epoll类封装了epoll的基本用法，成员变量包括创建的epoll句柄和存放事件的数组，成员函数有将一个fd或者Channel类添加到epoll句柄的红黑树中，还有运行epoll_wait的成员函数
    2.Channel类封装了处理fd事件的一系列方法，成员变量有fd_，以及该fd_的各种属性，是否在红黑树中，在哪一颗红黑树中，是否是监听的fd，需要监听的事件，和已经监听到的事件。成员函数有返回个成员变量的值，和设置fd_的相关属性，如设置边缘触发和监听读事件，以及处理返回事件。

2024/2/20
    在Channel类中添加了回调函数，添加了Tcpserver类，Acceptor类，Connection类和Eventloop类
    1.添加回调函数onmessage用于处理连接上的客户端的相关事件，newconnection处理服务端监听到的连接事件。回调函数用函数setreadcallback来设置，在创建Channel对象时，根据它是客户端还是服务端设置回调函数，方便之后调用
    2.EventLoop类是一个事件循环类，封装了epoll的一系列操作，成员变量是一个Epoll对象，用来控制事件，成员函数run用来循环执行事件
    3.TcpServer类是Tcp网络服务类，每一个服务类对象，有一个事件循环对象和Acceptor对象，成员函数start条用事件循环对象的run来循环执行事件
    4.Acceptor类，将Channel进行了封装，用于服务端使用
    5.Connection类，封装了Channel，客户端使用

2024/2/23
    添加ThreadPool类，为了发挥多CPU的性能，采用多线程的Reactor模型。
    1.多线程模型：运行多个事件循环，主事件循环运行在主线程中，从事件循环运行在线程池中。主线程负责创建客户端连接，然后把connn分配给线程池。一个Reactor负责多个Connection，每个Connection的工作内容包括IO和计算（处理业务），IO不会阻塞事件循环，但计算会阻塞事件循环。可以在一个Reactor中创建工作线程，工作线程负责计算，Reactor负责IO。
    2.线程资源管理：有时候不知道什么时候析构，就使用智能指针。当client断开连接之后，如果使用普通指针，那么connection对象就被析构了，conn就成了野指针，如果使用智能指针，connection对象不会析构，但但是断开连接后继续调用conn是没有意义的，因此在connection类中加入了disconnect_成员变量，用于判断connection是否还在连接，如果断开连接，就要将该channel从事件循环的红黑树中移除，因此在EventLoop类中加入了remove函数用于删除channel。将属于各个类的成员变量改成智能指针，传入的用常智能指针引用.Acceptor的channel使用栈内存，而connection的channel使用堆内存。
    3.异步唤醒事件循环。connection类中的send将在工作线程中访问发送缓冲区，writecallback会在IO线程中访问发送缓冲区，同时访问会发生冲突。为了防止冲突，可以用锁，但是每个connection都有一个发送缓冲区，如果用锁的话，占用的资源太大了。可以将发送的任务交给IO线程，IO线程会阻塞在epoll_wait中，可以用eventfd去唤醒IO线程。通知线程的方法：条件变量，信号量，socket，管道，eventfd。事件循环阻塞在epoll_wait，条件变量，信号量都有自己的等待函数，不适合用于通知事件循环。socket，管道，eventfd都是fd，都可加入epoll，如果要通知事件循环，往socket，管道，eventfd中写入数据即可。

    指针使用技巧：
    1.如果资源的生命周期难以确定，则使用shard_ptr管理
    2.类自己拥有的资源用unique_ptr来管理，在类被销毁的时候，将会自动释放
    3.不属于自己，但会使用的资源，采用unique_ptr&或shared_ptr来管理会很麻烦，不易阅读，依旧采用裸指针

2024/3/24
    1.清理空闲的Connection。空闲的Connection占用资源，还容易被攻击。定时器用于粗合理定时任务例如清理空闲的tcp连接。可以用信号和计时器完成清理空闲Connection。
    2.添加了时间戳类，用于计时使用，时间一到闹钟就会响，主事件的闹钟响应事件和从事件的闹钟响应事件是不一样的。 从事件会关闭超时的connection。主线程和从线程都会调用connnection，因此需要加锁保护。
    3.