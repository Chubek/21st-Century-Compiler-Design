/* Chapter 76: CPython multi-phase extension, Limited API floor 3.11.
   Requires a conventional GIL-enabled CPython build; see README.md. */
#define Py_LIMITED_API 0x030B0000
#include <Python.h>
#include "bookffi.h"
#include <limits.h>
#include <stdlib.h>

typedef struct { BbBuffer *buffer; } Owner;
typedef struct { unsigned long calls; } State;
static const char capsule_name[] = "bookffi.buffer.v1";

static PyObject *status_error(int status)
{
    if (status == BB_NOMEM) return PyErr_NoMemory();
    PyErr_SetString(status == BB_RANGE ? PyExc_OverflowError : PyExc_ValueError,
                    "native boundary rejected the request");
    return NULL;
}

static int integer32(PyObject *object, int32_t *out)
{
    long long value;
    if (!PyLong_CheckExact(object)) {
        PyErr_SetString(PyExc_TypeError, "expected an exact integer");
        return 0;
    }
    value = PyLong_AsLongLong(object);
    if (value == -1 && PyErr_Occurred()) return 0;
    if (value < INT32_MIN || value > INT32_MAX) {
        PyErr_SetString(PyExc_OverflowError, "integer outside int32 range");
        return 0;
    }
    *out = (int32_t)value;
    return 1;
}

static PyObject *add(PyObject *module, PyObject *args)
{
    PyObject *a, *b;
    int32_t x, y, sum;
    int status;
    State *state = PyModule_GetState(module);
    if (!PyArg_ParseTuple(args, "OO:add", &a, &b) ||
        !integer32(a, &x) || !integer32(b, &y)) return NULL;
    status = bb_add_via_ffi(x, y, &sum);
    if (status != BB_OK) return status_error(status);
    if (state->calls != ULONG_MAX) ++state->calls;
    return PyLong_FromLong(sum);
}

static void destroy(PyObject *capsule)
{
    Owner *owner = PyCapsule_GetPointer(capsule, capsule_name);
    /* Only this module creates named capsules with this destructor. */
    if (owner) { bb_free(owner->buffer); free(owner); }
}

static PyObject *create(PyObject *module, PyObject *args)
{
    Py_buffer view;
    Owner *owner;
    PyObject *capsule;
    int status;
    (void)module;
    if (!PyArg_ParseTuple(args, "y*:create", &view)) return NULL;
    owner = calloc(1, sizeof(*owner));
    if (!owner) { PyBuffer_Release(&view); return PyErr_NoMemory(); }
    status = bb_new(view.buf, (size_t)view.len, &owner->buffer);
    PyBuffer_Release(&view);
    if (status != BB_OK) { free(owner); return status_error(status); }
    capsule = PyCapsule_New(owner, capsule_name, destroy);
    if (!capsule) { bb_free(owner->buffer); free(owner); }
    return capsule;
}

static PyObject *bytes_copy(PyObject *module, PyObject *capsule)
{
    Owner *owner = PyCapsule_GetPointer(capsule, capsule_name);
    (void)module;
    if (!owner) return NULL;
    if (!owner->buffer) { PyErr_SetString(PyExc_ValueError, "closed buffer"); return NULL; }
    return PyBytes_FromStringAndSize((const char *)bb_data(owner->buffer),
                                     (Py_ssize_t)bb_size(owner->buffer));
}

static PyObject *close_buffer(PyObject *module, PyObject *capsule)
{
    Owner *owner = PyCapsule_GetPointer(capsule, capsule_name);
    (void)module;
    if (!owner) return NULL;
    bb_free(owner->buffer);
    owner->buffer = NULL;
    Py_RETURN_NONE;
}

static int callback(void *context, int32_t index, int32_t *out)
{
    PyObject *result = PyObject_CallFunction(context, "i", (int)index);
    int ok;
    if (!result) return BB_CALLBACK;
    ok = integer32(result, out);
    Py_DECREF(result);
    return ok ? BB_OK : BB_CALLBACK;
}

static PyObject *visit(PyObject *module, PyObject *args)
{
    PyObject *count, *function;
    int32_t n;
    int64_t sum;
    int status;
    (void)module;
    if (!PyArg_ParseTuple(args, "OO:visit", &count, &function) ||
        !integer32(count, &n)) return NULL;
    if (!PyCallable_Check(function)) {
        PyErr_SetString(PyExc_TypeError, "callback must be callable"); return NULL;
    }
    /* args roots function; synchronous callbacks stay on this attached thread. */
    status = bb_visit(n, callback, function, &sum);
    if (status == BB_CALLBACK) return NULL; /* Preserve the callback exception. */
    if (status != BB_OK) return status_error(status);
    return PyLong_FromLongLong(sum);
}

static PyObject *calls(PyObject *module, PyObject *unused)
{
    State *state = PyModule_GetState(module);
    (void)unused;
    return PyLong_FromUnsignedLong(state->calls);
}

static PyMethodDef methods[] = {
    {"add", add, METH_VARARGS, "Checked int32 addition through libffi."},
    {"create", create, METH_VARARGS, "Copy a contiguous byte buffer into native ownership."},
    {"bytes", bytes_copy, METH_O, "Copy native bytes into Python."},
    {"close", close_buffer, METH_O, "Idempotently close the native payload."},
    {"visit", visit, METH_VARARGS, "Invoke a synchronous callback at each index."},
    {"calls", calls, METH_NOARGS, "Successful addition count for this module instance."},
    {NULL, NULL, 0, NULL}
};
static PyModuleDef_Slot slots[] = { {0, NULL} };
static struct PyModuleDef definition = {
    PyModuleDef_HEAD_INIT, "bookffi", NULL, sizeof(State), methods, slots,
    NULL, NULL, NULL
};
PyMODINIT_FUNC PyInit_bookffi(void) { return PyModuleDef_Init(&definition); }
