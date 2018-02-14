import vsomeip
import binascii
import time

#SD
INSTANCE_ID = 0x1

#SoAD
LAMP_METHOD_ID = 0x0140
LAMP_SERVICE_ID = 0x0065

EVENT_LAMP_CHANGE_ID = 0x8141
EVENT_GROUP_LAMP_CHANGE_ID = 0x0

LAMP_FRONT_RIGHT=1
LAMP_FRONT_LEFT=2
LAMP_REAR_RIGHT=3
LAMP_REAR_LEFT=4   

INDICATOR_SERVICE_ID = 42
INDICATORS_ON_EVENT_ID = 0x8003
INDICATORS_OFF_EVENT_ID = 0x8004

# receiving 1 byte containing 4 bits representing the lamp status
# 1 : lamp is working // 0: dead lamp
def cb_Lamp_status(val):
    print("LAMP STATUS:\n \tFR: {}\tFL: {}\n\tRR: {}\tRL: {}\n".format( \
    "ok" if (val[0]==0) else "KO", \
    "ok" if (val[1]==0) else "KO", \
    "ok" if (val[2]==0) else "KO", \
    "ok" if (val[3]==0) else "KO" ))
    print("EVENT: got : {:s}".format(binascii.hexlify(val)))
    return False

def cb_add(val):
    print("  RESULT ADD got response!: {:s}".format(binascii.hexlify(val)))
    return False

def cb_warning(val):
    print("    RESULT WARNING got response!: {:s}".format(binascii.hexlify(val)))
    return False

def cb_evt(val):
    print("EVENT: got : {:s}".format(binascii.hexlify(val)))
    return False

def indicators_on_cb(val):
    print("indicators_on_cb")

def indicators_off_cb(val):
    print("indicators_off_cb")

vsomeip.create("test_benoit")
#vsomeip.register_message(LAMP_SERVICE_ID, INSTANCE_ID, LAMP_METHOD_ID, cb_warning)
#vsomeip.register_message(LAMP_SERVICE_ID, INSTANCE_ID, EVENT_LAMP_CHANGE_ID, cb_Lamp_status)
vsomeip.start()

#SoAD
MATH_ADD_METHOD_ID = 0x0140
MATH_SERVICE_ID = 0x0164

EVENT_MATH_CHANGE_ID = 0x8141
EVENT_GROUP_MATH_CHANGE_ID = 0x0

#vsomeip.register_message(MATH_SERVICE_ID, INSTANCE_ID, MATH_ADD_METHOD_ID, cb_add)
#vsomeip.register_message(MATH_SERVICE_ID, INSTANCE_ID, EVENT_MATH_CHANGE_ID, cb_evt)
vsomeip.start()
time.sleep(5)

#vsomeip.request_service(LAMP_SERVICE_ID, INSTANCE_ID)
#vsomeip.request_event(LAMP_SERVICE_ID, INSTANCE_ID, EVENT_LAMP_CHANGE_ID, EVENT_GROUP_LAMP_CHANGE_ID)
#vsomeip.request_service(MATH_SERVICE_ID, INSTANCE_ID)
#vsomeip.request_event(MATH_SERVICE_ID, INSTANCE_ID, EVENT_MATH_CHANGE_ID, EVENT_GROUP_MATH_CHANGE_ID)
time.sleep(2)


time.sleep(2)

print("Offer indicator events")
#vsomeip.register_message(INDICATOR_SERVICE_ID, INSTANCE_ID, INDICATOR_ID, indicator_cb)
vsomeip.offer_service(INDICATOR_SERVICE_ID, INSTANCE_ID)
vsomeip.offer_event(INDICATOR_SERVICE_ID, INSTANCE_ID, INDICATORS_ON_EVENT_ID, 0x3)
vsomeip.offer_event(INDICATOR_SERVICE_ID, INSTANCE_ID, INDICATORS_OFF_EVENT_ID, 0x3)
print("   => DONE")

#vsomeip.send_service(MATH_SERVICE_ID, INSTANCE_ID, MATH_ADD_METHOD_ID, bytearray([0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42]))
time.sleep(2)
#vsomeip.send_service(LAMP_SERVICE_ID, INSTANCE_ID, LAMP_METHOD_ID, bytearray([0x42]))
#time.sleep(2)

# BUG! when bytearray contains 0's ... no payload is sent!?
v = 2

u=0
while True:
    print("toggling")
    u=u+1 & 0xFF
    time.sleep(3)
    print("Offer and notify event ON")
    #vsomeip.offer_event(INDICATOR_SERVICE_ID, INSTANCE_ID, INDICATORS_ON_EVENT_ID)
    #vsomeip.offer_event(INDICATOR_SERVICE_ID, INSTANCE_ID, INDICATORS_OFF_EVENT_ID)
    vsomeip.notify(INDICATOR_SERVICE_ID, INSTANCE_ID, INDICATORS_ON_EVENT_ID,  bytearray([0x64,0x64,0x64,0x64,0x64,0x64,0x64,0x64,0x16,0x16,0x18,0x18,u]))
    print("    => DONE")

#vsomeip.send_service(MATH_SERVICE_ID, INSTANCE_ID, MATH_ADD_METHOD_ID, bytearray([0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42]))
    time.sleep(2)
    vsomeip.notify(INDICATOR_SERVICE_ID, INSTANCE_ID, INDICATORS_OFF_EVENT_ID, bytearray([0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, u]))
#vsomeip.send_service(LAMP_SERVICE_ID, INSTANCE_ID, LAMP_METHOD_ID, bytearray([v]))
#v = ~ v & 0x2 | 0x1

