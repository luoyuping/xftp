xftp --ftp服务器
=====

###服务器的要求
+ 守护进程，以后台的形式运行                                                       Y
+ 可配置   调试模式下，读取xftp.conf,正式发布版的配置文件/etc/xftp                 Y
+ 运行时，向 /var/run/  写入 一个PID 文件 ，避免，重复启动                         Y
+ 日志文件的输出，方便调试和记录服务器状况                                         Y
+ 以非root 用户运行                                                                N


###服务器代码细节
+ 线程池 处理并发
+ 状态转换机来处理单个用户的网络通讯，逻辑流畅，清晰
+ 方便调试，代码支持，调试模式，和非调试模式，具体的做法是，根据编译器的选项，'-D FTP_DEBUG',代码中宏判断'#ifdef FTP_DEBUG.....#else......#endif'
 

### 缺点
+ 目前的支持的命令有限，准备支持的命令如下

+ 支持的用户，目前只有anonymous,但用户验证接口还在，只是匹配了这个用户，所以权限管理这块，没有过多的关注
 
+ IO 这块是

+ 测试的客户端是linxu的ftp客户端，没有测试win的ftp客户端，但又针对换行的跨平台性的处理，应该对win的用户可以支持，只要客户端的命令的样式严格按照ftp官方的协议设计
  
