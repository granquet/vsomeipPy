import vsomeip
import binascii
import time

#SD
INSTANCE_ID = 0x1

#SoAD
MATH_ADD_METHOD_ID = 0x0140
MATH_SERVICE_ID = 0x0064

EVENT_MATH_CHANGE_ID = 0x8141
EVENT_GROUP_MATH_CHANGE_ID = 0x0

def cb(val):
    print("Hello cb!: {:s}".format(binascii.hexlify(val)))
    return False

def cb_evt(val):
    print("EVENT: got : {:s}".format(binascii.hexlify(val)))
    return False

vsomeip.create("test_benoit")
vsomeip.register_message(MATH_SERVICE_ID, INSTANCE_ID, MATH_ADD_METHOD_ID, cb)
vsomeip.register_message(MATH_SERVICE_ID, INSTANCE_ID, EVENT_MATH_CHANGE_ID, cb_evt)
vsomeip.start()
time.sleep(5)
vsomeip.request_event(MATH_SERVICE_ID, INSTANCE_ID, EVENT_MATH_CHANGE_ID, EVENT_GROUP_MATH_CHANGE_ID)
vsomeip.request_service(MATH_SERVICE_ID, INSTANCE_ID)
time.sleep(2)

vsomeip.send_service(MATH_SERVICE_ID, INSTANCE_ID, MATH_ADD_METHOD_ID, bytearray([0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42]))

input()
