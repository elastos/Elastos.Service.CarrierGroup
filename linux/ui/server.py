#!/usr/bin/env python
# -*- coding: utf-8 -*-
# by vellhe 2017/7/9
from flask import Flask
def init():
    print("init in")

def create_app():
    app = Flask(__name__)
    init();
    return app
