#include <python2.7/Python.h>
#include "guidepycli.h"
#include "guide.pb.h"
#include "guide.grpc.pb.h"
#include <cstdlib>
#include <memory>
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

PyMethodDef GuidePyCliMethods[] = {
  {"make_stub", guidepycli_make_stub, METH_VARARGS, ""},
  {"get_next_step", guidepycli_get_next_step, METH_VARARGS, ""},
  {NULL, NULL, 0, NULL},
};

PyMODINIT_FUNC initguidepycli() {
  Py_InitModule("guidepycli", GuidePyCliMethods);
}

PyAPI_FUNC(PyObject *) guidepycli_make_stub(PyObject *self, PyObject *args) {
  const char *server_addr = NULL;
  std::shared_ptr<grpc::ChannelCredentials> credentials;
  std::shared_ptr<grpc::Channel> channel;
  Guide::Stub *stub = NULL;

  if (!PyArg_ParseTuple(args, "s", &server_addr))
    return PyErr_Format(PyExc_RuntimeError, "");

  credentials = grpc::InsecureChannelCredentials();
  channel = grpc::CreateChannel(server_addr, credentials);
  stub = Guide::NewStub(channel).release();
  
  return PyCObject_FromVoidPtr(stub, std::free);
}

PyAPI_FUNC(PyObject *) guidepycli_get_next_step(PyObject *self, PyObject *args) {
  PyObject *py_object = NULL;
  Guide::Stub *stub = NULL;
  int car_id = 0;
  int seq = 0;
  int cur_row_idx = 0;
  int cur_col_idx = 0;
  int last_row_idx = 0;
  int last_col_idx = 0;
  int dst_row_idx = 0;
  int dst_col_idx = 0;
  CarState car_state;
  CarState_Position *cur_pos = NULL;
  CarState_Position *last_pos = NULL;
  CarState_Position *dst_pos = NULL;
  grpc::ClientContext context;
  grpc::Status status;
  Step step;

  if (!PyArg_ParseTuple(args, "Oiiiiiiii", &py_object, &car_id, &seq,
                        &cur_row_idx, &cur_col_idx, &last_row_idx, &last_col_idx, &dst_row_idx, &dst_col_idx))
    return PyErr_Format(PyExc_RuntimeError, "");
  if (PyCObject_Check(py_object))
    stub = (Guide::Stub *) ((PyCObject *) py_object)->cobject;
  else
    return PyErr_Format(PyExc_RuntimeError, "");

  cur_pos = new CarState_Position;
  last_pos = new CarState_Position;
  dst_pos = new CarState_Position;
  car_state.set_car_id(car_id);
  car_state.set_seq(seq);
  cur_pos->set_row_idx(cur_row_idx);
  cur_pos->set_col_idx(cur_col_idx);
  car_state.set_allocated_cur_pos(cur_pos);
  last_pos->set_row_idx(last_row_idx);
  last_pos->set_col_idx(last_col_idx);
  car_state.set_allocated_last_pos(last_pos);
  dst_pos->set_row_idx(dst_row_idx);
  dst_pos->set_col_idx(dst_col_idx);
  car_state.set_allocated_dst_pos(dst_pos);

  status = stub->GetNextStep(&context, car_state, &step);
  if (!status.ok())
    return PyErr_Format(PyExc_RuntimeError, "");
  
  return PyInt_FromLong(step.step_code());
}
