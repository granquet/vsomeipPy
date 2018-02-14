#include <Python.h>

#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <thread>
#include <vsomeip/vsomeip.hpp>

std::shared_ptr<vsomeip::application> app;
std::map<int, std::map<int, std::map<int, PyObject *>>> cb_map;

static void print_request(const std::shared_ptr<vsomeip::message> &_request) {
  std::shared_ptr<vsomeip::payload> its_payload = _request->get_payload();
  vsomeip::length_t l = its_payload->get_length();

  // Get payload
  std::stringstream ss;
  for (vsomeip::length_t i = 0; i < l; i++) {
    ss << std::setw(2) << std::setfill('0') << std::hex
       << (int)*(its_payload->get_data() + i) << " ";
  }

  std::cout << "SERVICE: Received message with Client/Session [" << std::setw(4)
            << std::setfill('0') << std::hex << _request->get_client() << "/"
            << std::setw(4) << std::setfill('0') << std::hex
            << _request->get_session() << "] " << ss.str() << std::endl;
}

static PyObject *payload_pack(std::shared_ptr<vsomeip::payload> pl) {

  return PyByteArray_FromStringAndSize((const char *)pl->get_data(),
                                       pl->get_length());
}

std::shared_ptr<vsomeip::payload> payload_unpack(PyObject *obj) {

  std::shared_ptr<vsomeip::payload> pl;
  char *raw_data = PyByteArray_AsString(obj);
  Py_ssize_t raw_size = PyByteArray_Size(obj);

  std::vector<vsomeip::byte_t> pl_data(raw_size);
  for (int i = 0; i < raw_size; i++)
    pl_data[i] = raw_data[i];
  pl = vsomeip::runtime::get()->create_payload();
  pl->set_data(pl_data);

  return pl;
}

PyObject *test_pack_unpack(PyObject *self, PyObject *args) {

  PyObject *bytes;
  if (!PyArg_ParseTuple(args, "O", &bytes)) {
    return NULL;
  }
  std::shared_ptr<vsomeip::payload> pl = payload_unpack(bytes);

  vsomeip::length_t l = pl->get_length();

  // Get payload
  std::stringstream ss;
  for (vsomeip::length_t i = 0; i < l; i++) {
    ss << std::setw(2) << std::setfill('0') << std::hex
       << (int)*(pl->get_data() + i) << " ";
  }

  std::cout << "PL: " << ss.str() << std::endl;

  PyObject *obj = payload_pack(pl);

  return Py_BuildValue("O", obj);
}

void message_dispatcher(const std::shared_ptr<vsomeip::message> &_msg) {

  std::shared_ptr<vsomeip::payload> its_payload = _msg->get_payload();

  print_request(_msg);
  PyObject *cb =
      cb_map[_msg->get_service()][_msg->get_instance()][_msg->get_method()];

  PyObject *arglist;
  PyObject *bytes;
  bytes = payload_pack(its_payload);
  arglist = Py_BuildValue("(O)", bytes);

  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();

  PyObject *result = PyObject_CallObject(cb, arglist);

  PyGILState_Release(gstate);

  if (PyObject_TypeCheck(result, &PyByteArray_Type)) {
    std::shared_ptr<vsomeip::message> its_response =
        vsomeip::runtime::get()->create_response(_msg);

    std::shared_ptr<vsomeip::payload> payload = payload_unpack(result);
    its_response->set_payload(payload);
    app->send(its_response, true);
  }
}

static PyObject *vsomeip_create_app(PyObject *self, PyObject *args) {
  const char *name;
  int sts = 0;

  if (!PyArg_ParseTuple(args, "s", &name))
    return NULL;

  app = vsomeip::runtime::get()->create_application(name);
  app->init();

  return Py_BuildValue("i", sts);
}

static PyObject *vsomeip_reg_message(PyObject *self, PyObject *args) {

  int svc_id, inst_id, mtd;
  int sts = 0;
  PyObject *pycbobj;

  if (!PyArg_ParseTuple(args, "iiiO", &svc_id, &inst_id, &mtd, &pycbobj))
    PyErr_SetString(PyExc_TypeError, "Invalid args!");

  // make sure last argument is a function
  if (!PyCallable_Check(pycbobj)) {
    PyErr_SetString(PyExc_TypeError, "Need a callable object!");
  }

  Py_XINCREF(pycbobj);

  cb_map[svc_id][inst_id][mtd] = pycbobj;

  app->register_message_handler(svc_id, inst_id, mtd, message_dispatcher);

  return Py_BuildValue("i", sts);
}

static PyObject *vsomeip_notify(PyObject *self, PyObject *args) {

  int svc_id, inst_id, evt;
  PyObject *val;
  int sts = 0;

  if (!PyArg_ParseTuple(args, "iiiO", &svc_id, &inst_id, &evt, &val))
    PyErr_SetString(PyExc_TypeError, "=Invalid args!");

  //std::shared_ptr<vsomeip::payload> payload;
  //payload = vsomeip::runtime::get()->create_payload();
  //std::vector<vsomeip::byte_t> payload_data;
  //payload_data.push_back(val);
  //payload->set_data(payload_data);

std::shared_ptr<vsomeip::payload> payload = payload_unpack(val);

  app->notify(svc_id, inst_id, evt, payload, false);

  return Py_BuildValue("i", sts);
}

static PyObject *vsomeip_offer_event(PyObject *self, PyObject *args) {
  int svc_id, inst_id, evt_id, grp;
  int sts = 0;

  if (!PyArg_ParseTuple(args, "iiii", &svc_id, &inst_id, &evt_id, &grp))
    PyErr_SetString(PyExc_TypeError, "Invalid args!");

  std::set<vsomeip::eventgroup_t> its_groups;
  its_groups.insert(grp);

  app->offer_event(svc_id, inst_id, evt_id, its_groups, true);

  return Py_BuildValue("i", sts);
}

static PyObject *vsomeip_request_event(PyObject *self, PyObject *args) {
  int svc_id, inst_id, evt_id, grp;
  int sts = 0;

  if (!PyArg_ParseTuple(args, "iiii", &svc_id, &inst_id, &evt_id, &grp))
    PyErr_SetString(PyExc_TypeError, "Invalid args!");

  std::set<vsomeip::eventgroup_t> its_groups;
  its_groups.insert(grp);

  app->request_event(svc_id, inst_id, evt_id, its_groups, true);
  app->subscribe(svc_id, inst_id, grp);

  return Py_BuildValue("i", sts);
}

static PyObject *vsomeip_offer_service(PyObject *self, PyObject *args) {
  int svc_id, inst_id;
  int sts = 0;

  if (!PyArg_ParseTuple(args, "ii", &svc_id, &inst_id))
    PyErr_SetString(PyExc_TypeError, "Invalid args!");

  app->offer_service(svc_id, inst_id);

  return Py_BuildValue("i", sts);
}

static PyObject *vsomeip_request_service(PyObject *self, PyObject *args) {
  int svc_id, inst_id;
  int sts = 0;

  if (!PyArg_ParseTuple(args, "ii", &svc_id, &inst_id))
    PyErr_SetString(PyExc_TypeError, "Invalid args!");

  app->request_service(svc_id, inst_id);

  return Py_BuildValue("i", sts);
}

static PyObject *vsomeip_stop_offer_service(PyObject *self, PyObject *args) {
  int svc_id, inst_id;
  int sts = 0;

  if (!PyArg_ParseTuple(args, "ii", &svc_id, &inst_id))
    PyErr_SetString(PyExc_TypeError, "Invalid args!");

  app->stop_offer_service(svc_id, inst_id);

  return Py_BuildValue("i", sts);
}

static void start() { app->start(); }

static PyObject *vsomeip_send_service(PyObject *self, PyObject *args) {
  int svc_id, inst_id, mtd;
  PyObject *val;
  int sts = 0;

  if (!PyArg_ParseTuple(args, "iiiO", &svc_id, &inst_id, &mtd, &val))
    PyErr_SetString(PyExc_TypeError, "== Invalid args!");

  std::shared_ptr<vsomeip::message> rq =
      vsomeip::runtime::get()->create_request();
  // Set the hello world service as target of the request
  rq->set_service(svc_id);
  rq->set_instance(inst_id);
  rq->set_method(mtd);

  // Create a payload which will be sent to the service
  std::shared_ptr<vsomeip::payload> payload = payload_unpack(val);

  rq->set_payload(payload);
  // Send the request to the service. Response will be delivered to the
  // registered message handler
  app->send(rq, true);

  return Py_BuildValue("i", sts);
}

static PyObject *vsomeip_start(PyObject *self, PyObject *args) {
  int sts = 0;

  std::thread vsomeip_main(&start);

  vsomeip_main.detach();

  return Py_BuildValue("i", sts);
}

static PyMethodDef VsomeIpMethods[] = {
    {"start", vsomeip_start, METH_VARARGS, "Start vsomeip application."},
    {"create", vsomeip_create_app, METH_VARARGS,
     "Create a vsomeip application."},
    {"register_message", vsomeip_reg_message, METH_VARARGS,
     "register message."},
    {"offer_service", vsomeip_offer_service, METH_VARARGS,
     "start offering service."},
    {"request_service", vsomeip_request_service, METH_VARARGS,
     "request service."},
    {"send_service", vsomeip_send_service, METH_VARARGS, "send over service."},
    {"stop_offer_service", vsomeip_stop_offer_service, METH_VARARGS,
     "start offering service."},
    {"offer_event", vsomeip_offer_event, METH_VARARGS, "start offering event."},
    {"request_event", vsomeip_request_event, METH_VARARGS, "start requesting event."},
    {"notify", vsomeip_notify, METH_VARARGS, "notify."},
    {"test", test_pack_unpack, METH_VARARGS, "notify."},

    {NULL, NULL, 0, NULL} /* Sentinel */
};

static struct PyModuleDef vsomeipPyDef = {
    PyModuleDef_HEAD_INIT,
    "vsomeip",
    NULL, //doc
    -1, //the module keeps state in global variables
    VsomeIpMethods
};

PyMODINIT_FUNC PyInit_vsomeip(void) {
  auto module = PyModule_Create(&vsomeipPyDef);
  PyEval_InitThreads();
  return module;
}

int main(int argc, char *argv[]) {
  /* Decode and pass argv[0] to the Python interpreter */
  wchar_t *program = Py_DecodeLocale(argv[0], NULL);
  if (program == NULL) {
    std::cerr << "Fatal error: cannot decode argv[0]" << std::endl;
    return -1;
  }
  Py_SetProgramName(program);

  /* Initialize the Python interpreter.  Required. */
  Py_Initialize();

  return 0;
}
