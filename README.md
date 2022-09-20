# WINSOCKTEST 

## Blocking Client & Server
[Microsoft Winsock2 tutorial](https://learn.microsoft.com/en-us/windows/win32/winsock/getting-started-with-winsock)

### Usage
1. Run server.exe
2. Run client by command line and give the server IP
```sh
$ client 127.0.0.1
```
3. Press any key in console to send a message from the client to the server once.
4. Two programs automatically close gracefully.

### Remark
Graceful disconnect in [Microsoft document](https://learn.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-shutdown):
> Another method to wait for notification that the remote end has sent all its data and initiated a graceful disconnect uses overlapped receive calls follows :
> 1. Call shutdown with how=SD_SEND.
> 2. Call recv or WSARecv until the function completes with success and indicates zero bytes were received. If SOCKET_ERROR is returned, then the graceful disconnect is not possible.
> 3. Call closesocket.

## Timeout Client & Server
Implement receiving with timeout by `setsockopt()`.

A [tutorial site](https://tangentsoft.net/wskfaq/newbie.html) says,
> You can adjust the send() and recv() timeouts with the SO_SNDTIMEO and SO_RCVTIMEO setsockopt() options.

In each loop of the client, the client first reads the receiving buffer with a timeout of 3 s. If any thing is read from buffer, client will show the message in the console. Then, the user can choose whether to send a message or to disconnect by entering (y/n/b).

In each loop of the server, the server first reads the receiving buffer with a timeout of 3 s. If a timeout happens, the server sends a meassge, ex "TIMEOUT: 10". The server also echos messages back to the client.

### Usage
1. Run ContinuousServer.exe
2. Run client by command line and give the server IP
```sh
$ ContinuousClient 127.0.0.1
```
3. Enter (y/n/b) in the client console.
4. If enter b, two programs will automatically close gracefully.

### Remark
Receive buffer will store all the data, if not read out. And all buffered data of different packet can be read out at the same time.

## Nonblocking Server
Implement a non-blocking sever by `select()`.

This server is modified from [this blog](https://blog.csdn.net/sbfksmq/article/details/51051585).

### Usage
1. Run SelectServer.exe
2. Run client by command line and give the server IP
```sh
$ ContinuousClient 127.0.0.1
```
3. Enter (y/n/b) in the client console.
4. If enter b, two programs will automatically close gracefully.

In each loop of the server, the server non-blockingly checks the socket is readable, writable, and having errors. If any message is received, the server echos the message back to the client.

### Remark
1. This server accepts only one client.
2. Don't know how to get errors of a set of client who are selected to be checked for errors.
3. `select()` can also be used in blocking, blocking with timeout, and non-blocking modes, by setting the timeout argument. See [this](https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-select).

## Other References
[Simple Explanation for networking and some mechanisms of sockets](https://www.madwizard.org/programming/tutorials/netcpp/1)
