#!/usr/bin/env python
# -*- coding: utf-8 -*-
# by vellhe 2017/7/9
from flask import Flask, abort, request, jsonify
import chatrobot_restful_api as chatrobot
app = Flask(__name__)

@app.route('/start/', methods=['GET'])
def start():
    chatrobot.start("127,0,0,1", 2222, "/home/lcf/workspace/ChatRobot/linux/ui")
    return jsonify({'result':"success"});

@app.route('/create/', methods=['GET'])
def create():
    chatrobot.createGroup();
    return jsonify({'result':"success"});

@app.route('/list/', methods=['GET'])
def list():
    return chatrobot.list();
if __name__ == "__main__":
    # 将host设置为0.0.0.0，则外网用户也可以访问到这个服务
    app.run(host="127.0.0.1", port=8384, debug=True)
    

