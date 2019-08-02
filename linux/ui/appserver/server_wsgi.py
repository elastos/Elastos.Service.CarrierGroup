#!/usr/bin/env python
# -*- coding: utf-8 -*-
# by vellhe 2017/7/9
from flask import Flask

def create_app(chatrobot, data_dir):
    app = Flask(__name__)
    chatrobot.start("127.0.0.1", 2222, data_dir)
    return app
