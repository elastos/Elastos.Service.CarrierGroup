from ctypes import cdll, string_at,create_string_buffer
carrierManager = cdll.LoadLibrary("./libcarrierManager.so")

def start(ip, port, data_dir):
    hostname = bytes(ip, encoding='utf-8')
    data_dir = bytes(data_dir,  encoding='utf-8')
    print(hostname, port, data_dir)
    carrierManager.start(hostname,port,data_dir)

def createGroup():
    print("createGroup in");
    carrierManager.createGroup()
    print("createGroup out");

def list():
    data_out = create_string_buffer(1024*512)
    carrierManager.list(data_out)
    print("list:",string_at(data_out, -1).decode('utf-8'));
    return string_at(data_out, -1).decode('utf-8')


