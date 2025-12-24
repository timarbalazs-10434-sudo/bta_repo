/*
 * Copyright (c) 2001-2023 Python Software Foundation.
 * Licensed under the Python Software Foundation License.
 */

#ifndef Py_CPYTHON_FILEUTILS_H
#  error "this header file must not be included directly"
#endif

// Used by _testcapi which must not use the internal C API
PyAPI_FUNC(FILE*) _Py_fopen_obj(
    PyObject *path,
    const char *mode);
