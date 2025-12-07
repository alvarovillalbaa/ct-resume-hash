#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdint.h>

#include "ct_resume_hash.h"

static PyObject *py_ct_resume_hash_once(PyObject *self, PyObject *args) {
    const char *input = NULL;
    Py_ssize_t input_len = 0;

    if (!PyArg_ParseTuple(args, "s#", &input, &input_len)) {
        return NULL;
    }

    uint8_t out[CT_RESUME_HASH_LEN];
    if (ct_resume_hash_once((const uint8_t *)input, (size_t)input_len, out) != 0) {
        PyErr_SetString(PyExc_RuntimeError, "ct_resume_hash_once failed");
        return NULL;
    }

    return PyBytes_FromStringAndSize((const char *)out, CT_RESUME_HASH_LEN);
}

static PyMethodDef Methods[] = {
    {"hash_once", py_ct_resume_hash_once, METH_VARARGS, "Hash resume text"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "ct_resume_hash._native",
    NULL,
    -1,
    Methods,
};

PyMODINIT_FUNC PyInit__native(void) {
    return PyModule_Create(&moduledef);
}

