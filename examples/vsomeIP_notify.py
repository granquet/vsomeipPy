import vsomeip
import binascii
import time

SVC_ID = 1
INST_ID = 1
MTD_ID = 1

def cb(val):
    print("got response!: {:s}".format(binascii.hexlify(val)))
    return False

vsomeip.create("test notify")

vsomeip.register_message(SVC_ID,INST_ID,MTD_ID,cb)
vsomeip.start()
time.sleep(5)

vsomeip.send_service(SVC_ID,INST_ID,MTD_ID,bytearray([42,0x60]))
print("Sent service!?")

input()
