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