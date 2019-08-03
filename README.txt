ChatRobot部署说明(Ubuntu 18.04上使用uWSGI和Nginx为Flask应用程序提供服务)
第1步 - 从Ubuntu存储库安装组件
sudo apt update
sudo apt install python3-pip python3-dev build-essential libssl-dev libffi-dev python3-setuptools

第2步 - 创建Python虚拟环境
sudo apt install python3-venv
cd /home/lcf/workspace/ChatRobot/linux/ui
创建一个虚拟环境来存储Flask项目的Python需求
python3.6 -m venv venv_chatrobot
 在虚拟环境中安装应用程序之前，需要将其激活。 输入以下命令：
source venv_chatrobot/bin/activate

第3步 - 设置Flask应用程序
虚拟环境下，可以安装Flask和uWSGI，并开始设计应用程序。
首先，使用pip的本地实例安装wheel ，以确保我们的软件包即使丢失了轮存档也会安装：
pip install wheel（在激活虚拟环境时，都应该使用pip命令（而不是pip3 ））
安装Flask和uWSGI：
pip install uwsgi flask

第4步 - 创建systemd单元文件
sudo nano /etc/systemd/system/chatrobot.service
注意路径修正为相应工程路径
-------------------------------------------------
[Unit]
Description=uWSGI instance to serve chatrobot
After=network.target

[Service]
User=lcf
Group=www-data
WorkingDirectory=/home/lcf/workspace/ChatRobot/linux/ui/appserver
Environment="PATH=/home/lcf/workspace/ChatRobot/linux/ui/venv_chatrobot/bin;"
ExecStart=/home/lcf/workspace/ChatRobot/linux/ui/venv_chatrobot/bin/uwsgi --ini chatrobot.ini

[Install]
WantedBy=multi-user.target
------------------------------------------------------
sudo systemctl start chatrobot
sudo systemctl enable chatrobot

第5步 - 将Nginx配置为代理请求
sudo nano /etc/nginx/sites-available/chatrobot
注意路径修正为相应工程路径
------------------------------------------------------
server {
    listen 80;
    server_name 127.0.0.1;

    location / {
        include uwsgi_params;
        uwsgi_pass unix:/home/lcf/workspace/ChatRobot/linux/ui/appserver/chatrobot.sock;
    }
}
---------------------------------------------------------
将文件链接到sites-enabled目录：
sudo ln -s /etc/nginx/sites-available/chatrobot /etc/nginx/sites-enabled
sudo nginx -t
sudo systemctl restart nginx
sudo ufw allow 'Nginx Full'

第6步 - 环境变量设置
sudo sh -c "echo /home/lcf/workspace/ChatRobot/linux/service/lib/x86_64 > /etc/ld.so.conf.d/chatrobot_lib.conf"
sudo ldconfig

第7步 - 重启
sudo reboot

第8步 - 启动web ui demo
cd /home/lcf/workspace/ChatRobot/linux/ui/webserver/chatrobot_server
yarn serve



