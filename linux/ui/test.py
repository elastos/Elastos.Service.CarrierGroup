import ctypes    
ll = ctypes.cdll.LoadLibrary   
lib = ll("./libtestManager.so")
hostname = bytes("127.0.0.1", encoding='utf-8')
data_dir = bytes("/home/lcf/workspace/ChatRobotLinux/testPython",  encoding='utf-8')
lib.start(hostname,2222,data_dir)
lib.createGroup()
