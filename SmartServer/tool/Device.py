# -*- coding: utf-8 -*-
import socket

class Device(object):
    MASTER = "MASTER"
    SLAVE = "SLAVE"
    def __init__(self, typ: str, name: str, ID: str, addr: tuple):
        self.typ = typ
        self.name = name
        self.ID = ID
        self.addr = addr
        self.relation = None
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    def send(self, msg: str):
        self.socket.sendto(msg.encode(), self.addr)

    def update_addr(self, addr: tuple):
        self.addr = addr

    def add_relation(self, device):
        self.relation = device.ID

    def discard_relation(self):
        self.relation = None

    def get_relation(self) -> str:
        """ 获取相关设备的ID """
        return self.relation

    def __str__(self):
        return ("[%s] [%s] [%s] (%s:%s)" % (self.typ, self.ID, self.name, str(self.addr[0]), str(self.addr[1])))


class DeviceList(object):
    def __init__(self):
        self.masterList = {}
        self.slaveList = {}

    def check_device_exists(self, ID: str):
        if ID is None:
            return False
        return ID in self.masterList.keys() or ID in self.slaveList.keys()

    def add_device(self, device: Device):
        if device.typ == Device.MASTER:
            self.masterList[device.ID] = device
        elif device.typ == Device.SLAVE:
            self.slaveList[device.ID] = device

    def remove_device(self, ID):
        if ID in self.masterList.keys():
            self.masterList.pop(ID)
        elif ID in self.slaveList.keys():
            self.slaveList.pop(ID)

    def get_master_number(self):
        return len(self.masterList)

    def get_slave_name(self) -> str:
        return ';'.join(["%s,%s" % (device.ID, device.name) for device in self.slaveList.values()])

    def get_device(self, ID: str) -> Device:
        if self.check_device_exists(ID) is True:
            if ID in self.masterList.keys():
                return self.masterList[ID]
            else:
                return self.slaveList[ID]
        else:
            return None
