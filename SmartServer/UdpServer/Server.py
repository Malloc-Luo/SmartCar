# -*- coding: utf-8 -*-
import socket
import threading
from tool.Device import Device, DeviceList
import re, time


class UdpServer(object):
    Local = "0.0.0.0"
    MasterPort = 19500
    SlavePort = 19501
    MaxMaster = 3

    def __init__(self):
        # 一个用来绑定master，一个slave
        self.master_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.slave_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        # ip port
        self.master_socket.bind((self.Local, self.MasterPort))
        self.slave_socket.bind((self.Local, self.SlavePort))
        # 设备列表
        self.device_list = DeviceList()
        # 离线检查列表
        self.offlineTable = {}

    def start_wait(self):
        # 全局锁
        self.globalLock = threading.Lock()
        self.master_thread = threading.Thread(target=self.recv_master, args=(self.globalLock, ))
        self.slave_thread = threading.Thread(target=self.recv_slave, args=(self.globalLock, ))
        self.check_thread = threading.Thread(target=self.check_offline_thread, args=(self.globalLock, ))
        # 开启线程调度
        self.master_thread.start()
        self.slave_thread.start()
        self.check_thread.start()
        # 阻塞
        self.master_thread.join()
        self.slave_thread.join()
        self.check_thread.join()

    def recv_master(self, lock: threading.Lock):
        """ 接收master发来的数据，整个流程如下：
        1. master主动发来 requests，格式 master+id+name+requests
        2. master发送请求连接信号 connect，格式 master+id+name+connect+slaveid
        3. master发送控制信号，格式 master+id+name+control+data
        """
        while True:
            data, addr = self.master_socket.recvfrom(1024)
            data = data.decode()
            buff = data.split('+')
            print(buff)
            typ, ID, name, cmd = buff[0], buff[1], buff[2], buff[3]
            self.offlineTable[ID] = 0
            # 判断是否已经连接过，如果没有连接过就加入连接
            if typ.lower() == "master" and self.device_list.check_device_exists(ID) is False:
                # 小于最大连接数则加入列表，否则跳过
                if self.device_list.get_master_number() < self.MaxMaster:
                    device = Device(Device.MASTER, name, ID, addr)
                    self.device_list.add_device(device)
                    print("[add device] ", device)
                else:
                    continue
            device = self.device_list.get_device(ID)
            # 如果是连接请求
            if cmd.lower() == 'requests':
                slaves = self.device_list.get_slave_name()
                # device.send(self.format_msg2_master(slaves))
                self.master_socket.sendto(self.format_msg2_master(slaves).encode(), addr)
                print('<requests from> ', device)
                print('<send back> ', slaves)
            # 如果是连接请求，则查看请求ID是否存在，若存在则查看是否已经建立连接
            # 若已经建立连接则返回 "Fail"，否则返回 "OK"
            elif cmd.lower() == 'connect':
                slaveId = buff[4]
                slaveDevice = self.device_list.get_device(slaveId)
                print("<connect from> ", device, " <to> ", slaveDevice)
                # 目标设备存在
                if slaveDevice is not None:
                    # 目标设备没有绑定master或者已经绑定的master就是发起请求的设备
                    if slaveDevice.get_relation() is None or slaveDevice.get_relation() == ID:
                        slaveDevice.add_relation(device)
                        device.add_relation(slaveDevice)
                        self.master_socket.sendto(self.format_msg2_master('OK').encode(), addr)
                else:
                    self.master_socket.sendto(self.format_msg2_master('Fail').encode(), addr)
            # 如果是控制的请求，接到控制请求后转发给客户端
            elif cmd.lower() == 'control':
                # 控制数据的格式：
                # 几个量：模式,速度vx,速度vy,速度vr
                # 例如：1,15,26,18
                slaveDevice = self.device_list.get_device(device.get_relation())
                isOffline = self.is_device_offline(slaveDevice)
                if isOffline is False:
                    # 如果没有离线就往小车发送控制数据
                    lock.acquire()
                    self.slave_socket.sendto(self.format_msg2_slave(buff[4]).encode(), slaveDevice.addr)
                    lock.release()
                else:
                    self.master_socket.sendto(self.format_msg2_master('Fail').encode(), addr)
            elif cmd.lower() == 'disconnect':
                # 断开连接
                if self.device_list.get_device(device.get_relation()) is not None:
                    slaveDevice = self.device_list.get_device(device.get_relation())
                    slaveDevice.discard_relation()
                device.discard_relation()
                self.master_socket.sendto(self.format_msg2_master('OK').encode(), addr)
            # 更新地址，动态分配的有时候会变
            device.update_addr(addr)


    def recv_slave(self, lock: threading.Lock):
        """ 监听slave发来的消息，时间紧迫就没有那么多要求了~
        发送的信息只有一种格式：slave+id+name+data
        往slave发送的信息：+data
        """
        while True:
            data, addr = self.slave_socket.recvfrom(1024)
            _buff = data.decode().split('+')
            if self.check_slave_msg(data.decode()) is False or len(_buff) < 4:
                continue
            _typ, _Id, _name, _data = _buff
            self.offlineTable[_Id] = 0
            # 如果设备已经注册了则转发数据，如果没有注册则进行注册
            if _typ.lower() == 'slave' and self.device_list.check_device_exists(_Id) is False:
                device = Device(Device.SLAVE, _name, _Id, addr)
                self.device_list.add_device(device)
                print('[add device] ', device)
            # 获取这个device
            device = self.device_list.get_device(_Id)
            if device is not None:
                masterDevice = self.device_list.get_device(device.get_relation())
                # 说明有关联的设备
                if masterDevice is not None:
                    self.master_socket.sendto(self.format_msg2_master(_data).encode(), masterDevice.addr)
                device.update_addr(addr)
            print(_buff)

    def check_offline_thread(self, lock: threading.Lock):
        """ 离线检查的线程，每收到一次就清零，否则+1
        直到大于15s认为设备离线
        """
        while True:
            time.sleep(0.01)
            for ID in self.offlineTable.keys():
                lock.acquire()
                if self.offlineTable[ID] != -1:
                    self.offlineTable[ID] += 1
                if self.offlineTable[ID] > 1500:
                    self.offlineTable[ID] = -1
                    device = self.device_list.get_device(ID)
                    print("<device offline> ", device)
                    self.device_list.remove_device(ID)
                lock.release()

    def is_device_offline(self, device: Device) -> bool:
        """ 检查指定ID的设备是否离线，如果离线返回True
        否则返回False
        """
        if device is None:
            return True
        elif device.ID not in self.offlineTable.keys():
            return True
        return self.offlineTable[device.ID] == -1

    def check_msg(self, msg: str) -> bool:
        return re.match(r'master|slave\+[\da-zA-Z]{5,10}\+[0-9a-zA-Z\s]+\+[a-zA-Z]+', msg) is not None

    def check_slave_msg(self, msg: str) -> bool:
        return re.match(r'slave\+[\da-zA-Z]{5,10}\+[0-9a-zA-Z\s]+\+[\d\,]+', msg) is not None

    def format_msg2_master(self, msg: str) -> str:
        return 'server+' + msg

    def format_msg2_slave(self, msg: str) -> str:
        return msg + '+' * (30 - len(msg))
