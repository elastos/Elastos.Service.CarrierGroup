from ctypes import cdll, c_char_p
carrierManager = cdll.LoadLibrary("./libcarrierManager.so")

def start(ip, port, data_dir):
    hostname = bytes(ip, encoding='utf-8')
    data_dir = bytes(data_dir,  encoding='utf-8')
    carrierManager.start(hostname,port,data_dir)
def createGroup():
    carrierManager.createGroup()

def list():
    result = carrierManager.list()
    return result


