/*
 * Copyright (c) 2001-2023 Python Software Foundation.
 * Licensed under the Python Software Foundation License.
 */

#ifndef Py_INTERNAL_SLICEOBJECT_H
#define Py_INTERNAL_SLICEOBJECT_H
#ifdef __cplusplus
extern "C" {
#endif

#ifndef Py_BUILD_CORE
#  error "this header requires Py_BUILD_CORE define"
#endif


/* runtime lifecycle */

extern void _PySlice_Fini(PyInterpreterState *);


#ifdef __cplusplus
}
#endif
#endif /* !Py_INTERNAL_SLICEOBJECT_H */
