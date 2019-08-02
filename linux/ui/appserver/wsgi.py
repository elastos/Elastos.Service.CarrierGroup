#!/usr/bin/env python
# -*- coding: utf-8 -*-
# by vellhe 2017/7/9
from flask import Flask, abort, request, jsonify,make_response
from flask_cors import CORS
from server_wsgi import create_app
import chatrobot_restful_api as chatrobot

app = create_app(chatrobot, "/home/lcf/workspace/testData/")
CORS(app, resource={r"/*":{"orgins":"*"}}, supports_credentials=True)

@app.route('/test', methods=['GET'])
def test():
    print("test in")
    chatrobot.createGroupTest();
    return jsonify({'result':"success"});

@app.route('/create', methods=['GET'])
def create():
    print("create in")
    chatrobot.createGroup();
    return jsonify({'result':"success"});

@app.route('/groups', methods=['GET'])
def list_groups():
    data = chatrobot.list();
    print("groups:", data);
    return jsonify({'code':0, 'data':data});

if __name__ == "__main__":
    print("start in************************************8")
    app.run(host="0.0.0.0")
