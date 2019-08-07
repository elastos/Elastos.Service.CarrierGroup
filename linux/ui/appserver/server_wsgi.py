#!/usr/bin/env python
# -*- coding: utf-8 -*-
# by vellhe 2017/7/9
from flask import Flask
import configparser

def create_app(chatrobot):
    app = Flask(__name__)

    conf = configparser.ConfigParser()
    conf.read("chatrobot_config.ini")
    data_dir = conf.get("chatrobot", "data_dir") # 获取指定section 的option值
    ip = conf.get("chatrobot", "socket_ip")
    port = conf.get("chatrobot", "socket_port")
    chatrobot.start(ip, port, data_dir)
    return app
