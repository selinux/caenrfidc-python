/*
 * Copyright (C) 2011-2016 Rodolfo Giometti <giometti@cosino>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation version 2
 *  of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this package; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <Python.h>

#include <caenrfid.h>

/*
 * Private functions
 */

static char nibble2hex(char c)
{
        switch (c) {
        case 0 ... 9:
                return '0' + c;

        case 0xa ... 0xf:
                return 'a' + (c - 10);
        }

        printf("got invalid data!");
        return '\0';
}

char *bin2hex(uint8_t *data, size_t len)
{
        char *str;
        int i;

        str = malloc(len * 2 + 1);
        if (!str)
                return NULL;

        for (i = 0; i < len; i++) {
                str[i * 2] = nibble2hex(data[i] >> 4);
                str[i * 2 + 1] = nibble2hex(data[i] & 0x0f);
        }
        str[i * 2] = '\0';

        return str;
}

/*
 * Python oblects stuff
 */

static PyObject *caenrfid_error;

typedef struct {
	PyObject_HEAD
	struct caenrfid_handle handle;
} caenrfid_handle_obj;

static void caenrfid_handle_dealloc(caenrfid_handle_obj *h)
{
	PyObject_Del(h);
}

static PyTypeObject caenrfid_handle_type = {
	PyObject_HEAD_INIT(NULL)
	0,					/*ob_size*/
	"caenrfid.handler",			/*tp_name*/
	sizeof(caenrfid_handle_obj),		/*tp_size*/
	0,					/*tp_itemsize*/

	/* methods */
	(destructor) caenrfid_handle_dealloc,	/*tp_dealloc*/
	0,					/*tp_print*/
	0,					/*tp_getattr*/
	0,					/*tp_setattr*/
	0,					/*tp_compare*/
	0,					/*tp_repr*/
        0,				 	/*tp_as_number*/
};
#define is_caenrfid_handle_obj(v)	((v)->ob_type == &caenrfid_handle_type)

static caenrfid_handle_obj *new_caenrfid_handle_obj(void)
{
	caenrfid_handle_obj *h;

	h = PyObject_New(caenrfid_handle_obj, &caenrfid_handle_type);
	if (h == NULL)
		return NULL;

	/* No actual initialisation needed */
	return h;
}

static caenrfid_handle_obj *caenrfid_handle_coerce(PyObject *z)
{
	/* shortcut: 9 out of 10 times the type is already ok */
	if (is_caenrfid_handle_obj(z)) {
		Py_INCREF(z);
		return (caenrfid_handle_obj *) z;	/* coercion succeeded */
	}

	PyErr_SetString(PyExc_TypeError, "invalid type");
	return NULL;
}

/*
 * Methods
 */

static PyObject *caenrfidc_fw_release(PyObject *self, PyObject *args)
{
	PyObject *h_obj, *res_obj;
	caenrfid_handle_obj *h;
	char string[256];
	int ret;

	if (!PyArg_ParseTuple(args, "O", &h_obj))
		return NULL;

	h = caenrfid_handle_coerce(h_obj);
	if (h == NULL) {
		/* Py_XDECREF(h) */
		return NULL;
	}

	ret = caenrfid_get_fw_release(&h->handle, string, 256);
	if (ret < 0) {
		PyErr_SetString(caenrfid_error, "cannot get firmware release");
		Py_DECREF(h);
                return NULL;
        }

	res_obj = Py_BuildValue("s", string);
	Py_INCREF(res_obj);

	Py_DECREF(h);

	return res_obj;
}

static PyObject *caenrfidc_inventory(PyObject *self, PyObject *args)
{
	char *source;
	PyObject *h_obj;
	caenrfid_handle_obj *h;
	struct caenrfid_tag *tag;
	size_t size;
	char *str;
	PyObject *tag_list, *tag_item;
	int i, ret;

	if (!PyArg_ParseTuple(args, "Os", &h_obj, &source))
		return NULL;

	h = caenrfid_handle_coerce(h_obj);
	if (h == NULL) {
		/* Py_XDECREF(h) */
		return NULL;
	}

	ret = caenrfid_complex_inventory(&h->handle, source,
			(unsigned char *) "", 0, 0, 1, &tag, &size);
	if (ret < 0) {
		PyErr_SetString(caenrfid_error, "inventory error");
		Py_DECREF(h);
                return NULL;
        }

        /* Report results */
	tag_list = PyList_New(0);
	Py_INCREF(tag_list);
        for (i = 0; i < size; i++) {
                str = bin2hex(tag[i].id, tag[i].len);
                if (!str) {
                        fprintf(stderr, "cannot allocate memory!\n");
                        exit(EXIT_FAILURE);
                }

		tag_item = Py_BuildValue("{s:s,s:s,s:s,s:H,s:H}",
				"tag", str, "source", source,
				"readpoint", tag[i].readpoint,
				"type", tag[i].type, "rssi", tag[i].rssi);
  		Py_INCREF(tag_item);

  		PyList_Append(tag_list, tag_item);
                free(str);
        }

	/* Free inventory data */
	free(tag);

	Py_DECREF(h);

	return tag_list;
}

static PyObject *caenrfidc_add_readpoint(PyObject *self, PyObject *args)
{
	PyObject *h_obj;
	caenrfid_handle_obj *h;
	char *source, *antenna;
	int ret;

	if (!PyArg_ParseTuple(args, "Oss", &h_obj, &source, &antenna))
		return NULL;

	h = caenrfid_handle_coerce(h_obj);
	if (h == NULL) {
		/* Py_XDECREF(h) */
		return NULL;
	}

	ret = caenrfid_add_readpoint(&h->handle, source, antenna);
	if (ret < 0) {
		PyErr_SetString(caenrfid_error,
				"cannot add antenna to readpoint");
		Py_DECREF(h);
                return NULL;
        }

	return Py_None;
}

static PyObject *caenrfidc_remove_readpoint(PyObject *self, PyObject *args)
{
	PyObject *h_obj;
	caenrfid_handle_obj *h;
	char *source, *antenna;
	int ret;

	if (!PyArg_ParseTuple(args, "Oss", &h_obj, &source, &antenna))
		return NULL;

	h = caenrfid_handle_coerce(h_obj);
	if (h == NULL) {
		/* Py_XDECREF(h) */
		return NULL;
	}

	ret = caenrfid_remove_readpoint(&h->handle, source, antenna);
	if (ret < 0) {
		PyErr_SetString(caenrfid_error,
				"cannot remove antenna to readpoint");
		Py_DECREF(h);
                return NULL;
        }

	return Py_None;
}

static PyObject *caenrfidc_check_readpoint(PyObject *self, PyObject *args)
{
	PyObject *h_obj;
	caenrfid_handle_obj *h;
	char *source, *antenna;
	uint16_t val;
	int ret;

	if (!PyArg_ParseTuple(args, "Oss", &h_obj, &source, &antenna))
		return NULL;

	h = caenrfid_handle_coerce(h_obj);
	if (h == NULL) {
		/* Py_XDECREF(h) */
		return NULL;
	}

	ret = caenrfid_check_readpoint(&h->handle, source, antenna, &val);
	if (ret < 0) {
		PyErr_SetString(caenrfid_error,
				"cannot inquiry readpoint");
		Py_DECREF(h);
                return NULL;
        }

	return val ? Py_True : Py_False;
}

static PyObject *caenrfidc_get_power(PyObject *self, PyObject *args)
{
	PyObject *h_obj, *res_obj;
	caenrfid_handle_obj *h;
	uint32_t pow;
	int ret;

	if (!PyArg_ParseTuple(args, "O", &h_obj))
		return NULL;

	h = caenrfid_handle_coerce(h_obj);
	if (h == NULL) {
		/* Py_XDECREF(h) */
		return NULL;
	}

	ret = caenrfid_get_power(&h->handle, &pow);
	if (ret < 0) {
		PyErr_SetString(caenrfid_error, "cannot get power");
		Py_DECREF(h);
                return NULL;
        }

	res_obj = Py_BuildValue("I", pow);
	Py_INCREF(res_obj);

	Py_DECREF(h);

	return res_obj;
}

static PyObject *caenrfidc_set_power(PyObject *self, PyObject *args)
{
	PyObject *h_obj;
	caenrfid_handle_obj *h;
	uint32_t pow;
	int ret;

	if (!PyArg_ParseTuple(args, "OI", &h_obj, &pow))
		return NULL;

	h = caenrfid_handle_coerce(h_obj);
	if (h == NULL) {
		/* Py_XDECREF(h) */
		return NULL;
	}

	ret = caenrfid_set_power(&h->handle, pow);
	if (ret < 0) {
		PyErr_SetString(caenrfid_error, "cannot set power");
		Py_DECREF(h);
                return NULL;
        }

	return Py_None;
}

static PyObject *caenrfidc_open(PyObject *self, PyObject *args)
{
	char *addr;
	caenrfid_handle_obj *h;
	int ret;

	if (!PyArg_ParseTuple(args, "s", &addr))
		return NULL;

	h = new_caenrfid_handle_obj();
	if (h == NULL)
		return NULL;

	/* Start a new connection with the CAENRFIDD server */
        ret = caenrfid_open(CAENRFID_PORT_TCP, addr, &h->handle);
        if (ret < 0) {
		PyErr_SetString(caenrfid_error, "cannot connect");
		return NULL;
        }

	return (PyObject *) h;
}

static PyObject *caenrfidc_close(PyObject *self, PyObject *args)
{
	PyObject *h_obj;
	caenrfid_handle_obj *h;
	int ret;

	if (!PyArg_ParseTuple(args, "O", &h_obj))
		return NULL;

	h = caenrfid_handle_coerce(h_obj);
	if (h == NULL) {
		/* Py_XDECREF(h) */
		return NULL;
	}

	ret = caenrfid_close(&h->handle);
	if (ret < 0) {
		PyErr_SetString(caenrfid_error, "error on closing connection");
                return NULL;
        }

	Py_DECREF(h);

	return Py_None;
}

static PyMethodDef caenrfidc_handle_methods[] = {
    {"fw_release", caenrfidc_fw_release, METH_VARARGS,
		"Get server's firmware release."},
    {"inventory", caenrfidc_inventory, METH_VARARGS,
		"Do an inventory."},
    {"add_readpoint", caenrfidc_add_readpoint, METH_VARARGS,
		"Add an antenna into a specified read point."},
    {"remove_readpoint", caenrfidc_remove_readpoint, METH_VARARGS,
		"Remove an antenna from a specified read point."},
    {"check_readpoint", caenrfidc_check_readpoint, METH_VARARGS,
		"Check for an antenna into a specified read point."},
    {"get_power", caenrfidc_get_power, METH_VARARGS,
		"Get antennae power."},
    {"set_power", caenrfidc_set_power, METH_VARARGS,
		"Set antennae power."},
    {"open", caenrfidc_open, METH_VARARGS,
		"Open a new connection."},
    {"close", caenrfidc_close, METH_VARARGS,
		"Close an existing connection."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

/*
 * Module stuff
 */

PyMODINIT_FUNC initcaenrfidc(void)
{
	PyObject *m;

	caenrfid_handle_type.ob_type = &PyType_Type;
	m = Py_InitModule("caenrfidc", caenrfidc_handle_methods);
	if (m == NULL)
		return;

	caenrfid_error = PyErr_NewException("caenrfid.error", NULL, NULL);
	Py_INCREF(caenrfid_error);
	PyModule_AddObject(m, "error", caenrfid_error);
}
