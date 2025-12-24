/*
 * Copyright (c) 2001-2023 Python Software Foundation.
 * Licensed under the Python Software Foundation License.
 */

// Implementation of PEP 585: support list[int] etc.
#ifndef Py_GENERICALIASOBJECT_H
#define Py_GENERICALIASOBJECT_H
#ifdef __cplusplus
extern "C" {
#endif

PyAPI_FUNC(PyObject *) Py_GenericAlias(PyObject *, PyObject *);
PyAPI_DATA(PyTypeObject) Py_GenericAliasType;

#ifdef __cplusplus
}
#endif
#endif /* !Py_GENERICALIASOBJECT_H */
