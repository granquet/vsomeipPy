import vsomeip
import binascii
import time

SVC_ID = 1
INST_ID = 1
MTD_ID = 1

def cb(val):
    print("Hello cb!: {:s}".format(binascii.hexlify(val)))
    return bytearray([1,2,3])

vsomeip.create("test")
vsomeip.register_message(SVC_ID,INST_ID,MTD_ID,cb)
vsomeip.offer_service(SVC_ID,INST_ID)

vsomeip.start()
print("====== READY =====")

input()
