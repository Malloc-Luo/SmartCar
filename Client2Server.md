### Client到Server的通信规则

##### 1. `InitRequest`

*   Client：发送 `master` + `id` + `request` 请求连接（`id`为一个`uint16_t`型数据），例如一个客户端ID为`12345`，则发送的请求信号为：`master 12345 request`；
*   Server：收到请求格式信号，新建一个线程用于