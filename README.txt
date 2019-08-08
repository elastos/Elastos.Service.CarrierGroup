ChatRobot部署说明(Ubuntu 18.04上使用uWSGI和Nginx为Flask应用程序提供服务)
注意将xxxxxxx替换成有效的用户名

第1步 - 从Ubuntu存储库安装组件
sudo apt update
sudo apt install python3-pip python3-dev build-essential libssl-dev libffi-dev python3-setuptools
sudo apt install nginx

第2步 - 创建Python虚拟环境
sudo apt install python3-venv
cd /home/xxxxxxx/workspace/ChatRobot/linux/ui
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
pip install flask_cors
deactivate
第4步 - 修改chatrobot 配置文件
1.创建数据存放根目录
mkdir /home/xxxxxxx/workspace/testData
2.修改配置文件内容：data_dir---各服务的数据存放根目录、socket_ip/socket_port----各服务与manager通信的ip与端口号
/home/xxxxxxx/workspace/ChatRobot/linux/ui/appserver/chatrobot_config.ini
--------------------------------------------------
[chatrobot]
data_dir = /home/xxxxxxx/workspace/testData
socket_ip = 127.0.0.1
socket_port = 2222
--------------------------------------------------
第5步 - 创建systemd单元文件
sudo nano /etc/systemd/system/chatrobot.service
注意路径修正为相应工程路径
-------------------------------------------------
[Unit]
Description=uWSGI instance to serve chatrobot
After=network.target

[Service]
User=xxxxxxx
Group=www-data
WorkingDirectory=/home/xxxxxxx/workspace/ChatRobot/linux/ui/appserver
Environment="PATH=/home/xxxxxxx/workspace/ChatRobot/linux/ui/venv_chatrobot/bin;"
ExecStart=/home/xxxxxxx/workspace/ChatRobot/linux/ui/venv_chatrobot/bin/uwsgi --ini chatrobot.ini

[Install]
WantedBy=multi-user.target
------------------------------------------------------
sudo systemctl start chatrobot
sudo systemctl enable chatrobot

第6步 - 将Nginx配置为代理请求
sudo nano /etc/nginx/sites-available/chatrobot
注意路径修正为相应工程路径
------------------------------------------------------
server {
    listen 80;
    server_name 127.0.0.1;

    location / {
        include uwsgi_params;
        uwsgi_pass unix:/home/xxxxxxx/workspace/ChatRobot/linux/ui/appserver/chatrobot.sock;
    }
}
---------------------------------------------------------
将文件链接到sites-enabled目录：
sudo ln -s /etc/nginx/sites-available/chatrobot /etc/nginx/sites-enabled
sudo nginx -t
sudo systemctl restart nginx
sudo ufw allow 'Nginx Full'

第7步 - 环境变量设置
sudo sh -c "echo /home/xxxxxxx/workspace/ChatRobot/linux/service/lib/x86_64 > /etc/ld.so.conf.d/chatrobot_lib.conf"
sudo ldconfig

第8步 - 重启
sudo reboot

**********启动web ui demo*******
安装必要软件库
sudo apt install curl
----------------------------------------------------
按照以下步骤在 Ubuntu 16.04/18.04 系统上安装 Yarn：
步骤1.添加GPG密钥

curl -sS https://dl.yarnpkg.com/debian/pubkey.gpg | sudo apt-key add -

步骤2.添加Yarn存储库

echo "deb https://dl.yarnpkg.com/debian/ stable main" | sudo tee /etc/apt/sources.list.d/yarn.list

步骤3.更新包列表并安装Yarn

sudo apt update
sudo apt install yarn

如果您的系统上尚未安装 Node.js，则上面的命令将安装它。 那些使用 nvm 的人可以跳过 Node.js 安装：

sudo apt install --no-install-recommends yarn

步骤4.检查Yarn的版本

要验证 Yarn 是否已成功安装，请运行以下命令以打印 Yarn 版本号：

yarn --version
--------------------------------------------------------------
安装Vue
yarn global add @vue/cli
yarn add babel-plugin-import --dev
sudo apt install npm
cd /home/xxxxxxx/workspace/ChatRobot/linux/ui/webserver
npm i --save ant-design-vue
cd /home/xxxxxxx/workspace/ChatRobot/linux/ui/webserver/chatrobot_server
npm install
npm install --save @chenfengyuan/vue-qrcode

yarn serve

