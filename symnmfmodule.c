#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <math.h>
#include <stdlib.h>
#include "symnmf.h"

// Convert Python list[list[float]] to struct vector*
struct vector* pylist_to_vector_list(PyObject *py_list) {
    if (!PyList_Check(py_list)) return NULL;

    Py_ssize_t n = PyList_Size(py_list);
    struct vector *head = NULL, *prev = NULL;

    for (Py_ssize_t i = 0; i < n; ++i) {
        PyObject *py_vec = PyList_GetItem(py_list, i);
        if (!PyList_Check(py_vec)) return NULL;

        struct cord *cord_head = NULL, *cord_prev = NULL;
        Py_ssize_t dim = PyList_Size(py_vec);

        for (Py_ssize_t j = 0; j < dim; ++j) {
            PyObject *item = PyList_GetItem(py_vec, j);
            double val = PyFloat_AsDouble(item);

            struct cord *new_cord = malloc(sizeof(struct cord));
            new_cord->value = val;
            new_cord->next = NULL;

            if (!cord_head)
                cord_head = new_cord;
            else
                cord_prev->next = new_cord;

            cord_prev = new_cord;
        }

        struct vector *new_vec = malloc(sizeof(struct vector));
        new_vec->cords = cord_head;
        new_vec->next = NULL;

        if (!head)
            head = new_vec;
        else
            prev->next = new_vec;

        prev = new_vec;
    }

    return head;
}


// Main C API wrapper
static PyObject* sym_capi(PyObject *self, PyObject *args) {
    PyObject *py_vectors;
    int n;

    if (!PyArg_ParseTuple(args, "Oi", &py_vectors, &n))
        return NULL;

    struct vector *vec = pylist_to_vector_list(py_vectors);
    double** A = sym(vec,n);
    // Free memory
    Free_vector(vec);
    // build Python list of lists from A
    PyObject *result = PyList_New(n);
    for (int i = 0; i < n; i++) {
        PyObject *row = PyList_New(n);
        for (int j = 0; j < n; j++) {
            PyObject *val = PyFloat_FromDouble(A[i][j]);
            PyList_SET_ITEM(row, j, val);
        }
        PyList_SET_ITEM(result, i, row);
    }

    // free C matrix
    Free_Matrix(A, n);
    return result;
}

static PyObject* ddg_capi(PyObject *self, PyObject *args) {
    PyObject *py_vectors;
    int n;

    if (!PyArg_ParseTuple(args, "Oi", &py_vectors, &n))
        return NULL;

    struct vector *vec = pylist_to_vector_list(py_vectors);
    double** A = sym(vec,n);
    double** D = ddg(A,n);
    // Free memory
    Free_vector(vec);
    // build Python list of lists from A
    PyObject *result = PyList_New(n);
    for (int i = 0; i < n; i++) {
        PyObject *row = PyList_New(n);
        for (int j = 0; j < n; j++) {
            PyObject *val = PyFloat_FromDouble(D[i][j]);
            PyList_SET_ITEM(row, j, val);
        }
        PyList_SET_ITEM(result, i, row);
    }

    // free C matrix
    Free_Matrix(A, n);
    Free_Matrix(D, n);
    return result;
}

static PyObject* norm_capi(PyObject *self, PyObject *args) {
    PyObject *py_vectors;
    int n;

    if (!PyArg_ParseTuple(args, "Oi", &py_vectors, &n))
        return NULL;

    struct vector *vec = pylist_to_vector_list(py_vectors);
    double** A = sym(vec,n);
    double** D = ddg(A,n);
    double** W = norm(A,D,n);
    // Free memory
    Free_vector(vec);
    // build Python list of lists from A
    PyObject *result = PyList_New(n);
    for (int i = 0; i < n; i++) {
        PyObject *row = PyList_New(n);
        for (int j = 0; j < n; j++) {
            PyObject *val = PyFloat_FromDouble(W[i][j]);
            PyList_SET_ITEM(row, j, val);
        }
        PyList_SET_ITEM(result, i, row);
    }

    // free C matrix
    Free_Matrix(A, n);
    Free_Matrix(D, n);
    Free_Matrix(W, n);
    return result;
}

static PyMethodDef capiMethods[] = {
    {"sym",  (PyCFunction)sym_capi,  METH_VARARGS, "create sym matrix"},
    {"ddg",  (PyCFunction)ddg_capi,  METH_VARARGS, "create ddg matrix"},
    {"norm", (PyCFunction)norm_capi, METH_VARARGS, "create norm matrix"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef symnmfmodule = {
    PyModuleDef_HEAD_INIT,
    "symnmf_capi", /* name of module */
    NULL, /* module documentation, may be NULL */
    -1,  /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
    capiMethods /* the PyMethodDef array from before containing the methods of the extension */
};

PyMODINIT_FUNC PyInit_symnmf_capi(void)
{
    PyObject *m;
    m = PyModule_Create(&symnmfmodule);
    if (!m) {
        return NULL;
    }
    return m;
}