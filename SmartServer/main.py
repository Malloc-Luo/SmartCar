# -*- coding: utf-8 -*-
from UdpServer.Server import UdpServer


if __name__ == '__main__':
    print("Start Server")
    server = UdpServer()
    server.start_wait()
    print("Stop Server")