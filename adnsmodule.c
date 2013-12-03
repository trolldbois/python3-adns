/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.
*/

#include <Python.h>
#include <adns.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static PyObject *ErrorObject;
static PyObject *NotReadyError;
static PyObject *LocalError;
static PyObject *RemoteError, *RemoteFailureError, *RemoteTempError,
    *RemoteConfigError;
static PyObject *QueryError;
static PyObject *PermanentError, *NXDomainError, *NoDataError;

/* -----------------------------------------------------*/

/* Declarations for objects of type ADNS_State*/

typedef struct {
    PyObject_HEAD
    adns_state state;
} ADNS_Stateobject;

static PyTypeObject ADNS_Statetype;

/* ----------------------------------------------------------------*/

/* Declarations for objects of type ADNS_Query*/

typedef struct {
    PyObject_HEAD
    ADNS_Stateobject *s;
    adns_query query;
    PyObject *answer;
    PyObject *exc_type;
    PyObject *exc_value;
    PyObject *exc_traceback;
} ADNS_Queryobject;

static PyTypeObject ADNS_Querytype;



/* ----------------------------------------------------------------*/

typedef struct {
    char *name;
    int value;
} _constant_class;

/* int PyModule_AddIntConstant(PyObject *module, const char *name, long value)*/

static PyModuleDef module_iflags = {
    PyModuleDef_HEAD_INIT,
    "iflags",
    "iflags Constants",
    -1,
    NULL, NULL, NULL, NULL, NULL
};
static _constant_class adns_iflags[] = {
    { "noenv", adns_if_noenv },
    { "noerrprint", adns_if_noerrprint },
    { "noserverwarn", adns_if_noserverwarn },
    { "debug", adns_if_debug },
    { "logpid", adns_if_logpid },
    { "noautosys", adns_if_noautosys },
    { "eintr", adns_if_eintr },
    { "nosigpipe", adns_if_nosigpipe },
    { "checkc_entex", adns_if_checkc_entex },
    { "checkc_freq", adns_if_checkc_freq },
    { NULL, 0 },
};

static PyModuleDef module_qflags = {
    PyModuleDef_HEAD_INIT,
    "qflags",
    "qflags Constants",
    -1,
    NULL, NULL, NULL, NULL, NULL
};
static _constant_class adns_qflags[] = {
    { "search", adns_qf_search },
    { "usevc", adns_qf_usevc },
    { "owner", adns_qf_owner },
    { "quoteok_query", adns_qf_quoteok_query },
    { "quoteok_cname", adns_qf_quoteok_cname },
    { "quoteok_anshost", adns_qf_quoteok_anshost },
    { "quotefail_cname", adns_qf_quotefail_cname },
    { "cname_loose", adns_qf_cname_loose },
    { "cname_forbid", adns_qf_cname_forbid },
    { "internalmask", adns__qf_internalmask },
    { NULL, 0 },
};

static PyModuleDef module_rr = {
    PyModuleDef_HEAD_INIT,
    "rr",
    "rr Constants",
    -1,
    NULL, NULL, NULL, NULL, NULL
};
static _constant_class adns_rr[] = {
    { "typemask", adns_rrt_typemask },
    { "deref", adns__qtf_deref },
    { "mail822", adns__qtf_mail822 },
    { "unknown", adns_r_unknown },
    { "none", adns_r_none },
    { "A", adns_r_a },
    { "NSraw", adns_r_ns_raw },
    { "NS", adns_r_ns },
    { "CNAME", adns_r_cname },
    { "SOAraw", adns_r_soa_raw },
    { "SOA", adns_r_soa },
    { "PTRraw", adns_r_ptr_raw },
    { "PTR", adns_r_ptr },
    { "HINFO", adns_r_hinfo },
    { "MXraw", adns_r_mx_raw },
    { "MX", adns_r_mx },
    { "TXT", adns_r_txt },
    { "RPraw", adns_r_rp_raw },
    { "RP", adns_r_rp },
    { "SRVraw", adns_r_srv_raw },
    { "SRV", adns_r_srv },
    { "ADDR", adns_r_addr },
    { NULL, 0 }
};

static PyModuleDef module_status = {
    PyModuleDef_HEAD_INIT,
    "status",
    "status Constants",
    -1,
    NULL, NULL, NULL, NULL, NULL
};
static _constant_class adns_s[] = {
    { "ok", adns_s_ok },
    { "nomemory", adns_s_nomemory },
    { "unknownrrtype", adns_s_unknownrrtype },
    { "systemfail", adns_s_systemfail },
    { "max_localfail", adns_s_max_localfail },
    { "timeout", adns_s_timeout },
    { "allservfail", adns_s_allservfail },
    { "norecurse", adns_s_norecurse },
    { "invalidresponse", adns_s_invalidresponse },
    { "unknownformat", adns_s_unknownformat },
    { "max_remotefail", adns_s_max_remotefail },
    { "rcodeservfail", adns_s_rcodeservfail },
    { "rcodeformaterror", adns_s_rcodeformaterror },
    { "rcodenotimplemented", adns_s_rcodenotimplemented },
    { "rcoderefused", adns_s_rcoderefused },
    { "rcodeunknown", adns_s_rcodeunknown },
    { "max_tempfail", adns_s_max_tempfail },
    { "inconsistent", adns_s_inconsistent },
    { "prohibitedcname", adns_s_prohibitedcname },
    { "answerdomaininvalid", adns_s_answerdomaininvalid },
    { "answerdomaintoolong", adns_s_answerdomaintoolong },
    { "invaliddata", adns_s_invaliddata },
    { "max_misconfig", adns_s_max_misconfig },
    { "querydomainwrong", adns_s_querydomainwrong },
    { "querydomaininvalid", adns_s_querydomaininvalid },
    { "querydomaintoolong", adns_s_querydomaintoolong },
    { "max_misquery", adns_s_max_misquery },
    { "nxdomain", adns_s_nxdomain },
    { "nodata", adns_s_nodata },
    { "max_permfail", adns_s_max_permfail },
    { NULL, 0 }
};

static PyObject *
interpret_addr(
    adns_rr_addr *v
    )
{
    PyObject *o;
    o = Py_BuildValue("is", v->addr.inet.sin_family, 
              inet_ntoa(v->addr.inet.sin_addr)) ;
    return o;
}

static PyObject *
interpret_hostaddr(
    adns_rr_hostaddr *hostaddr
    )
{
    PyObject *o, *addrs;
    if (hostaddr->naddrs == -1) {
        addrs = Py_None;
        Py_INCREF(addrs);
    } else {
        int i;
        addrs = PyTuple_New(hostaddr->naddrs);
        for (i=0; i<hostaddr->naddrs; i++) {
            adns_rr_addr *v = hostaddr->addrs+i;
            PyTuple_SET_ITEM(addrs,i,interpret_addr(v));
        }
    }
    o = Py_BuildValue("siO", hostaddr->host, hostaddr->astatus,
              addrs);
    Py_DECREF(addrs);
    return o;
}

static PyObject *
interpret_answer(
    adns_answer *answer
    )
{
    PyObject *o, *rrs;
    int i;
    adns_rrtype t = answer->type & adns_rrt_typemask;
    adns_rrtype td = answer->type & adns__qtf_deref;

    rrs = PyTuple_New(answer->nrrs);
    if (!rrs) return NULL;
    for (i=0; i<answer->nrrs; i++) {
        PyObject *a = NULL;
        switch (t) {
        case adns_r_a:
            if (td) {
                a = interpret_addr((answer->rrs.addr+i));
            } else {
                struct in_addr *v = answer->rrs.inaddr+i;
                a = Py_BuildValue("s", inet_ntoa(*v));
            }
            break;
        case adns_r_hinfo:
            {
                adns_rr_intstrpair *v = \
                    answer->rrs.intstrpair+i;
                a = Py_BuildValue("s#s#", v->array[0].str,
                          v->array[0].i,
                          v->array[1].str,
                          v->array[1].i);
            }
            break;
        case adns_r_mx_raw:
            if (td) {
                adns_rr_inthostaddr *v = \
                    answer->rrs.inthostaddr+i;
                o = interpret_hostaddr(&v->ha);
                a = Py_BuildValue("iO", v->i, o);
                Py_DECREF(o);
            } else {
                adns_rr_intstr *v = answer->rrs.intstr+i;
                a = Py_BuildValue("is", v->i, v->str);
            }
            break;
        case adns_r_ptr_raw:
        case adns_r_cname:
            {
                char *(*v) = answer->rrs.str+i;
                a = PyBytes_FromString(*v);
            }
            break;
        case adns_r_txt:
            {
                PyObject *txt;
                int array_len = 0;
                int ai;
                adns_rr_intstr *(*s) = answer->rrs.manyistr+i;

                while ((*s)[array_len].i != -1)
                    array_len++;

                if (!(a = PyTuple_New(array_len))) break;
                for (ai = 0; ai < array_len; ai++)
                {
                    txt = PyBytes_FromStringAndSize((*s)[ai].str, (*s)[ai].i);
                    if (!txt) {
                        Py_DECREF(a);
                        a = NULL;
                        break;
                    }
                    PyTuple_SET_ITEM(a, ai, txt);
                }
            }
            break;
        case adns_r_ns_raw:
            if (td) {
                a = interpret_hostaddr(answer->rrs.hostaddr+i);
            } else {
                char *(*v) = answer->rrs.str+i;
                a = PyBytes_FromString(*v);
            }
            break;
        case adns_r_soa_raw:
            {
                adns_rr_soa *v = answer->rrs.soa+i;
                a = Py_BuildValue("sslllll", v->mname, v->rname,
                          v->serial, v->refresh, v->retry,
                          v->expire, v->minimum);
            }
            break;
        case adns_r_rp:
            {
                adns_rr_strpair *v = answer->rrs.strpair+i;
                a = Py_BuildValue("ss", v->array[0], v->array[1]);
            }
            break;
        case adns_r_srv_raw:
            if (td) {
                adns_rr_srvha *v = answer->rrs.srvha+i;
                o = interpret_hostaddr(&v->ha);
                a = Py_BuildValue("iiiO", v->priority, v->weight, v->port, o);
                Py_DECREF(o);
            } else {
                adns_rr_srvraw *v = answer->rrs.srvraw+i;
                a = Py_BuildValue("iiis", v->priority, v->weight, v->port,
                           v->host);
            }
            break;
        default:
            a = Py_None;
            Py_INCREF(a);
        }
        if (!a) {
            Py_DECREF(rrs);
            return NULL;
        }
        PyTuple_SET_ITEM(rrs, i, a);
    }
    o = Py_BuildValue("isiO", (int) answer->status, answer->cname,
              answer->expires, rrs);
    Py_DECREF(rrs);
    return o ;
}

static char adns_exception__doc__[] = \
"exception(s)\n\
\n\
Checks the status code of an answer and raises an exception if necessary.\n";

static PyObject*
adns_exception(
    PyObject *self,
    PyObject *args
    )
{
    PyObject *e, *m;
    adns_status status;
    if (!PyArg_ParseTuple(args, "i", &status))
        return NULL;
    switch (status) {
    case adns_s_ok:
        Py_INCREF(Py_None);
        return Py_None;
    case adns_s_nomemory:
        return PyErr_NoMemory();
    case adns_s_unknownrrtype:
    case adns_s_systemfail:
        e = LocalError;
        break;
    case adns_s_timeout:
    case adns_s_allservfail:
    case adns_s_norecurse:
    case adns_s_invalidresponse:
    case adns_s_unknownformat:
        e = RemoteFailureError;
        break;
    case adns_s_rcodeservfail:
    case adns_s_rcodeformaterror:
    case adns_s_rcodenotimplemented:
    case adns_s_rcoderefused:
    case adns_s_rcodeunknown:
        e = RemoteTempError;
        break;
    case adns_s_inconsistent:
    case adns_s_prohibitedcname:
    case adns_s_answerdomaininvalid:
    case adns_s_invaliddata:
        e = RemoteConfigError;
    case adns_s_querydomainwrong:
    case adns_s_querydomaininvalid:
    case adns_s_querydomaintoolong:
        e = QueryError;
        break;
    case adns_s_nxdomain:
        e = NXDomainError;
        break;
    case adns_s_nodata:
        e = NoDataError;
        break;
    default:
        e = ErrorObject;
    }
    if (!(m=Py_BuildValue("is", status,
                 adns_strerror((adns_status) status))))
        return NULL;
    PyErr_SetObject(e, m);
    Py_DECREF(m);
    return NULL;
}

/* ----------------------------------------------------------------*/

static char ADNS_State_synchronous__doc__[] = 
"s.synchronous(name,type[,flags]\n\
\n\
Perform a query on name synchronously for RR type, returning the answer.\n\
Answers returned as (status, cname, expires, rrs).\n\
rrs is an n-tuple, each element is a RR. Format varies by\n\
RR and query.\n"
;

static PyObject *
ADNS_State_synchronous(
    ADNS_Stateobject *self,
    PyObject *args
    )
{
    char *owner;
    adns_rrtype type = 0;
    adns_queryflags flags = 0;
    adns_answer *answer_r;
    int r;
    PyObject *o;
    if (!PyArg_ParseTuple(args, "si|i", &owner, &type, &flags))
        return NULL;
    Py_BEGIN_ALLOW_THREADS;
    r = adns_synchronous(self->state, owner, type, flags, &answer_r);
    Py_END_ALLOW_THREADS;
    if (r) {
        PyErr_SetString(ErrorObject, strerror(r));
        return NULL;
    }
    o = interpret_answer(answer_r);
    free(answer_r);
    return o;
}


static char ADNS_State_submit__doc__[] = 
"s.submit(name,type[,flags])\n\
\n\
Submit a query. Returns a ADNS_Query object.\n"
;

static ADNS_Queryobject *newADNS_Queryobject(ADNS_Stateobject *state);

static PyObject *
ADNS_State_submit(
    ADNS_Stateobject *self,
    PyObject *args
    )
{
    char *owner;
    adns_rrtype type = 0;
    adns_queryflags flags = 0;
    int r;
    ADNS_Queryobject *o;
    if (!PyArg_ParseTuple(args, "si|i", &owner, &type, &flags))
        return NULL;
    o = newADNS_Queryobject(self);
    Py_BEGIN_ALLOW_THREADS;
    r = adns_submit(self->state, owner, type, flags, o, &o->query);
    Py_END_ALLOW_THREADS;
    if (r) {
        PyErr_SetString(ErrorObject, strerror(r));
        return NULL;
    }
    return (PyObject *) o;
}


static char ADNS_State_submit_reverse__doc__[] = 
"s.submit_reverse(name,type[,flags])\n\
\n\
Submit a query. Returns a ADNS_Query object.\n\
flags must specify some kind of PTR query."
;


static PyObject *
ADNS_State_submit_reverse(
    ADNS_Stateobject *self,
    PyObject *args
    )
{
    char *owner;
    struct sockaddr_in addr;
    adns_rrtype type = 0;
    adns_queryflags flags = 0;
    int r;
    ADNS_Queryobject *o;
    if (!PyArg_ParseTuple(args, "si|i", &owner, &type, &flags))
        return NULL;
        r = inet_aton(owner, (struct in_addr *) &(addr.sin_addr));
        if (!r) {
                PyErr_SetString(ErrorObject, "invalid IP address");
                return NULL;
        }
    addr.sin_family = AF_INET;
    o = newADNS_Queryobject(self);
    Py_BEGIN_ALLOW_THREADS;
    r = adns_submit_reverse(self->state, (struct sockaddr *)&addr, type, flags, o, &o->query);
    Py_END_ALLOW_THREADS;
    if (r) {
        PyErr_SetString(ErrorObject, strerror(r));
        return NULL;
    }
    return (PyObject *) o;
}

static char ADNS_State_submit_reverse_any__doc__[] = 
"s.submit_reverse_any(name,zone,type[,flags])\n\
\n\
Submit a query. Returns a ADNS_Query object.\n\
zone is in-addr.arpa., etc.\n\
flags must specify some kind of PTR query."
;


static PyObject *
ADNS_State_submit_reverse_any(
    ADNS_Stateobject *self,
    PyObject *args
    )
{
    char *owner, *zone;
    struct sockaddr_in addr;
    adns_rrtype type = 0;
    adns_queryflags flags = 0;
    int r;
    ADNS_Queryobject *o;
    if (!PyArg_ParseTuple(args, "ssi|i", &owner, &zone, &type, &flags))
        return NULL;
        r = inet_aton(owner, &(addr.sin_addr));
        if (!r) {
                PyErr_SetString(ErrorObject, "invalid IP address");
                return NULL;
        }
    addr.sin_family = AF_INET;
    o = newADNS_Queryobject(self);
    Py_BEGIN_ALLOW_THREADS;
    r = adns_submit_reverse_any(self->state, (struct sockaddr *)&addr, zone, type, flags, o, &o->query);
    Py_END_ALLOW_THREADS;
    if (r) {
        PyErr_SetString(ErrorObject, strerror(r));
        return NULL;
    }
    return (PyObject *) o;
}


static char ADNS_State_allqueries__doc__[] = 
"s.allqueries()\n\
\n\
Returns a list of all pending queries.\n"
;


static PyObject *
ADNS_State_allqueries(
    ADNS_Stateobject *self,
    PyObject *args
    )
{
    ADNS_Queryobject *o;
    adns_query q;
    PyObject *l;

    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    if (!(l = PyList_New(0))) return NULL;
    for (adns_forallqueries_begin(self->state);
         (q = adns_forallqueries_next(self->state, (void *)&o));
        ) {
        if (PyList_Append(l, (PyObject *) o)) {
            Py_DECREF(l);
            return NULL;
        }
    }
    return l;
}



static char ADNS_State_select__doc__[] = 
"s.select(timeout=0)\n\
\n\
Like the select() system call, except it works only on\n\
pending queries, i.e. internally it does:\n\
1) adns_beforeselect\n\
2) select\n\
3) adns_afterselect\n"
;


static PyObject *
ADNS_State_select(
    ADNS_Stateobject *self,
    PyObject *args
    )
{
    fd_set rfds, wfds, efds;
    int r, maxfd=0;
    double ft = 0;
    struct timeval **tv_mod, tv_buf, now, timeout;
    struct timezone tz;

    if (!PyArg_ParseTuple(args, "|d", &ft))
        return NULL;
    timeout.tv_sec = (int) ft;
    timeout.tv_usec = (int) ((ft - timeout.tv_sec) * 1e6);
    tv_mod = NULL;
    if (gettimeofday(&now, &tz))
        return PyErr_SetFromErrno(ErrorObject);
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_ZERO(&efds);
    adns_beforeselect(self->state, &maxfd, &rfds, &wfds, &efds,
              tv_mod, &tv_buf, &now);
    Py_BEGIN_ALLOW_THREADS;
    r = select(maxfd, &rfds, &wfds, &efds, &timeout);
    Py_END_ALLOW_THREADS;
    if (r == -1) return PyErr_SetFromErrno(ErrorObject);
    if (gettimeofday(&now, &tz))
        return PyErr_SetFromErrno(ErrorObject);
    adns_afterselect(self->state, maxfd, &rfds, &wfds, &efds, &now);
    Py_INCREF(Py_None);
    return Py_None;
}


static char ADNS_State_completed__doc__[] = 
"s.allqueries()\n\
\n\
Returns a list of all completed queries.\n"
;


static PyObject *
ADNS_State_completed(
    ADNS_Stateobject *self,
    PyObject *args
    )
{
    adns_answer *answer_r;
    int r;
    ADNS_Queryobject *o, *o2;
    adns_query q;
    PyObject *l;

    if (!(l = ADNS_State_select(self, args))) return NULL;
    Py_DECREF(l);
    if (!(l = PyList_New(0))) return NULL;
    for (adns_forallqueries_begin(self->state);
         (q = adns_forallqueries_next(self->state, (void *)&o));
        ) {
        r = adns_check(self->state, &q, &answer_r, (void *) &o2);
        if (r) {
            if (r == EWOULDBLOCK)
                continue;
            else {
                PyErr_SetString(ErrorObject, strerror(r));
                PyErr_Fetch(&(o->exc_type),
                        &(o->exc_value),
                        &(o->exc_traceback));
                continue;
            }
        }
        assert(o == o2);
        o->answer = interpret_answer(answer_r);
        free(answer_r);
        o->query = NULL;
        if (PyList_Append(l, (PyObject *) o)) {
            Py_DECREF(l);
            return NULL;
        }
    }
    return l;
}



static char ADNS_State_globalsystemfailure__doc__[] = 
""
;

static PyObject *
ADNS_State_globalsystemfailure(
    ADNS_Stateobject *self,
    PyObject *args
    )
{
    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    adns_globalsystemfailure(self->state);
    Py_INCREF(Py_None);
    return Py_None;
}


static struct PyMethodDef ADNS_State_methods[] = {
 {"synchronous",    (PyCFunction)ADNS_State_synchronous,    METH_VARARGS,    ADNS_State_synchronous__doc__},
 {"submit",    (PyCFunction)ADNS_State_submit,    METH_VARARGS,    ADNS_State_submit__doc__},
 {"submit_reverse",    (PyCFunction)ADNS_State_submit_reverse,    METH_VARARGS,    ADNS_State_submit_reverse__doc__},
 {"submit_reverse_any",    (PyCFunction)ADNS_State_submit_reverse_any,    METH_VARARGS,    ADNS_State_submit_reverse_any__doc__},
 {"allqueries",    (PyCFunction)ADNS_State_allqueries,    METH_VARARGS,    ADNS_State_allqueries__doc__},
 {"completed",    (PyCFunction)ADNS_State_completed,    METH_VARARGS,    ADNS_State_completed__doc__},
 {"select",    (PyCFunction)ADNS_State_select,    METH_VARARGS,    ADNS_State_select__doc__},
 {"globalsystemfailure",    (PyCFunction)ADNS_State_globalsystemfailure,    METH_VARARGS,    ADNS_State_globalsystemfailure__doc__},
 
    {NULL,        NULL}        /* sentinel*/
};

/* ----------*/

/*
static PyObject *
ADNS_State_getattr(
    ADNS_Stateobject *self,
    char *name
    )
{
    // XXXX Add your own getattr code here
    // return Py_FindMethod(ADNS_State_methods, (PyObject *)self, name);
    // TODO add methods
    PyErr_Format(PyExc_AttributeError,
                 "'%.50s' object has no attribute '%.400s'",
                 Py_TYPE(self)->tp_name, name);
    return NULL;    
}
*/

static ADNS_Stateobject *
newADNS_Stateobject(void)
{
    ADNS_Stateobject *self;
    
    self = PyObject_New(ADNS_Stateobject, &ADNS_Statetype);
    if (self == NULL)
        return NULL;
    self->state = NULL;
    return self;
}


static void
ADNS_State_dealloc(ADNS_Stateobject *self)
{
    Py_BEGIN_ALLOW_THREADS;
    adns_finish(self->state);
    Py_END_ALLOW_THREADS;
    Py_INCREF(Py_None);
    PyObject_Del(self);
}

static char ADNS_Statetype__doc__[] = 
"Contains state information for adns session."
;

static PyTypeObject ADNS_Statetype = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "adns.State",                   /*tp_name*/
    sizeof(ADNS_Stateobject),       /*tp_basicsize*/
    0,                              /*tp_itemsize*/
    (destructor)ADNS_State_dealloc, /*tp_dealloc*/
    0,                              /*tp_print*/
    0,                              /*tp_getattr*/
    0,                              /*tp_setattr*/
    0,                              /*reserved*/
    0,                              /*tp_repr*/
    0,                              /*tp_as_number*/
    0,                              /*tp_as_sequence*/
    0,                              /*tp_as_mapping*/
    0,                              /*tp_hash*/
    0,                              /*tp_call*/
    0,                              /*tp_str*/
    0,                              /*tp_getattro*/
    0,                              /*tp_setattro*/
    0,                              /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    ADNS_Statetype__doc__,          /*tp_doc*/
    0,                              /*tp_traverse*/
    0,                              /*tp_clear*/
    0,                              /*tp_richcompare*/
    0,                              /*tp_weaklistoffset*/
    0,                              /*tp_iter*/
    0,                              /*tp_iternext*/
    ADNS_State_methods,             /*tp_methods*/
    0,                              /*tp_members*/
    0,                              /*tp_getset*/
    0,                              /*tp_base*/
    0,                              /*tp_dict*/
    0,                              /*tp_descr_get*/
    0,                              /*tp_descr_set*/
    0,                              /*tp_dictoffset*/
    0,                              /*tp_init*/
    0,                              /*tp_alloc*/
    0,                              /*tp_new*/
};

/* End of code for ADNS_State objects*/
/* --------------------------------------------------------*/


static char ADNS_Query_check__doc__[] = 
"answer = q.check()\n\
\n\
Check for an answer. Return value same as for\n\
s.synchronous.\n"
;

static PyObject *
ADNS_Query_check(
    ADNS_Queryobject *self,
    PyObject *args
    )
{
    adns_answer *answer_r;
    int r;
    PyObject *o2=(PyObject *)self;

    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    if (self->exc_type) {
        PyErr_Restore(self->exc_type, self->exc_value, self->exc_traceback);
        self->exc_type = self->exc_value = self->exc_traceback = NULL;
        return NULL;
    }
    if (self->answer) goto ret_answer;
    if (!(self->query)) {
        PyErr_SetString(ErrorObject, "query invalidated");
        return NULL;
    }
    r = adns_check(self->s->state, &self->query, &answer_r, (void *) &o2);
    if (r) {
        if (r == EWOULDBLOCK)
            PyErr_SetString(NotReadyError, strerror(r));
        else {
            PyErr_SetString(ErrorObject, strerror(r));
            self->query = NULL;
        }
        return NULL;
    }
    assert(o2 == (PyObject *) self);
    self->answer = interpret_answer(answer_r);
    self->query = NULL;
    free(answer_r);
  ret_answer:
    Py_INCREF(self->answer);
    return self->answer;
}


static char ADNS_Query_wait__doc__[] = 
"answer=q.wait()\n\
\n\
Wait for an answer. Return value same as for\n\
s.synchronous.\n"
;

static PyObject *
ADNS_Query_wait(
    ADNS_Queryobject *self,
    PyObject *args
    )
{
    adns_answer *answer_r;
    int r;
    PyObject *o2=(PyObject *)self;

    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    if (self->exc_type) {
        PyErr_Restore(self->exc_type, self->exc_value, self->exc_traceback);
        self->exc_type = self->exc_value = self->exc_traceback = NULL;
        return NULL;
    }
    if (self->answer) goto ret_answer;
    if (!(self->query)) {
        PyErr_SetString(ErrorObject, "query invalidated");
        return NULL;
    }
    Py_BEGIN_ALLOW_THREADS;
    r = adns_wait(self->s->state, &self->query, &answer_r, (void *) &o2);
    Py_END_ALLOW_THREADS;
    if (r) {
        if (r == EWOULDBLOCK)
            PyErr_SetString(NotReadyError, strerror(r));
        else {
            PyErr_SetString(ErrorObject, strerror(r));
            self->query = NULL;
        }
        return NULL;
    }
    assert(o2 == (PyObject *) self);
    self->answer = interpret_answer(answer_r);
    self->query = NULL;
    free(answer_r);
  ret_answer:
    Py_INCREF(self->answer);
    return self->answer;
}


static char ADNS_Query_cancel__doc__[] = 
"q.cancel()\n\
\n\
Cancel a pending query.\n"
;

static PyObject *
ADNS_Query_cancel(
    ADNS_Queryobject *self,
    PyObject *args
    )
{
    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    if (!(self->query)) {
        PyErr_SetString(ErrorObject, "query invalidated");
        return NULL;
    }
    Py_BEGIN_ALLOW_THREADS;
    adns_cancel(self->query);
    Py_END_ALLOW_THREADS;
    Py_INCREF(Py_None);
    self->query = NULL;
    return Py_None;
}


static struct PyMethodDef ADNS_Query_methods[] = {
    {"check", (PyCFunction)ADNS_Query_check, METH_VARARGS, ADNS_Query_check__doc__},
    {"wait",  (PyCFunction)ADNS_Query_wait,  METH_VARARGS, ADNS_Query_wait__doc__},
    {"cancel",(PyCFunction)ADNS_Query_cancel,METH_VARARGS, ADNS_Query_cancel__doc__},
    {NULL,    NULL}        /* sentinel*/
};

/* ----------*/

/*
static PyObject *
ADNS_Query_getattr(
    ADNS_Queryobject *self,
    char *name
    )
{
    // XXXX Add your own getattr code here
    // return Py_FindMethod(ADNS_Query_methods, (PyObject *)self, name);
    PyErr_Format(PyExc_AttributeError,
                 "'%.50s' object has no attribute '%.400s'",
                 Py_TYPE(self)->tp_name, name);
    return NULL;    
}
*/

static ADNS_Queryobject *
newADNS_Queryobject(ADNS_Stateobject *state)
{
    ADNS_Queryobject *self;
    
    self = PyObject_New(ADNS_Queryobject, &ADNS_Querytype);
    if (self == NULL)
        return NULL;
    Py_INCREF(state);
    self->s = state;
    self->query = NULL;
    self->answer = NULL;
    self->exc_type = NULL;
    self->exc_value = NULL;
    self->exc_traceback = NULL;
    return self;
}


static void
ADNS_Query_dealloc(ADNS_Queryobject *self)
{
    Py_DECREF(self->s);
    Py_XDECREF(self->answer);
    Py_XDECREF(self->exc_type);
    Py_XDECREF(self->exc_value);
    Py_XDECREF(self->exc_traceback);
    PyObject_Del(self);
}

static char ADNS_Querytype__doc__[] = 
"A query currently being processed by adns."
;

static PyTypeObject ADNS_Querytype = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "adns.Query",                   /*tp_name*/
    sizeof(ADNS_Queryobject),       /*tp_basicsize*/
    0,                              /*tp_itemsize*/
    (destructor)ADNS_Query_dealloc, /*tp_dealloc*/
    0,                              /*tp_print*/
    0,                              /*tp_getattr*/
    0,                              /*tp_setattr*/
    0,                              /*reserved*/
    0,                              /*tp_repr*/
    0,                              /*tp_as_number*/
    0,                              /*tp_as_sequence*/
    0,                              /*tp_as_mapping*/
    0,                              /*tp_hash*/
    0,                              /*tp_call*/
    0,                              /*tp_str*/
    0,                              /*tp_getattro*/
    0,                              /*tp_setattro*/
    0,                              /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    ADNS_Querytype__doc__,          /*tp_doc*/
    0,                              /*tp_traverse*/
    0,                              /*tp_clear*/
    0,                              /*tp_richcompare*/
    0,                              /*tp_weaklistoffset*/
    0,                              /*tp_iter*/
    0,                              /*tp_iternext*/
    ADNS_Query_methods,             /*tp_methods*/
    0,                              /*tp_members*/
    0,                              /*tp_getset*/
    0,                              /*tp_base*/
    0,                              /*tp_dict*/
    0,                              /*tp_descr_get*/
    0,                              /*tp_descr_set*/
    0,                              /*tp_dictoffset*/
    0,                              /*tp_init*/
    0,                              /*tp_alloc*/
    0,                              /*tp_new*/
};

/* End of code for ADNS_Query objects*/
/* --------------------------------------------------------*/


static char adns_init__doc__[] =
"s=adns.init([initflags,debugfileobj=stderr,configtext=''])\n\
\n\
Initialize an ADNS_State object, which contains state information\n\
used internally by adns."
;

int
_file_converter(
    PyObject *obj,
    FILE **f
    )
{
    /*  *f = PyFile_AsFile(obj);*/
    /* int PyObject_AsFileDescriptor(PyObject *p)*/
    *f = fdopen(PyObject_AsFileDescriptor(obj),"a+");
    return (*f != NULL);
}

static PyObject *
adns__init(
    PyObject *self,    /* Not used*/
    PyObject *args,
    PyObject *kwargs
    )
{
    static char *kwlist[] = { "flags", "diagfile", "configtext" };
    adns_initflags flags = 0;
    int status;
    FILE *diagfile = NULL;
    char *configtext = NULL;
    ADNS_Stateobject *s;

    /* file is debug file
    s=adns.init([initflags,debugfileobj=stderr,configtext=''])
    
    "|iO&s",
    | Indicates that the remaining arguments in the Python argument list are optional.
    i (int) [int]    Convert a Python integer to a plain C int.
    O& (object) [converter, anything]  Convert a Python object to a C variable through a converter function.
    s (string or Unicode) [const char *]  Convert a Python string or Unicode object to a C pointer to a character string
   */
    if (!PyArg_ParseTupleAndKeywords(
        args, kwargs, "|iO&s", kwlist,
        &flags, _file_converter, &diagfile, &configtext))
        return NULL;
    if (!(s = newADNS_Stateobject())) return NULL;
    if (configtext)
        status = adns_init_strcfg(&s->state, flags,
                      diagfile, configtext);
    else
        status = adns_init(&s->state, flags, diagfile);
    if (status) {
        PyErr_SetFromErrno(ErrorObject);
        ADNS_State_dealloc(s);
        return NULL;
    }
    return (PyObject *) s;
}

/* List of methods defined in the module*/

static struct PyMethodDef adns_methods[] = {
    {"init", (PyCFunction)adns__init, METH_VARARGS|METH_KEYWORDS, adns_init__doc__},
    {"exception",(PyCFunction)adns_exception, METH_VARARGS, adns_exception__doc__},
 
    {NULL,     (PyCFunction)NULL, 0, NULL}        /* sentinel*/
};



static char adns_module_documentation[] = "adns python3 module";

static PyObject *
_new_exception(
    PyObject *dict,
    char *name,
    PyObject *base
    )
{
    PyObject *v;
    char longname[256];
    
    sprintf(longname, "adns.%s", name);
    if ((v = PyErr_NewException(longname, base, NULL)) == NULL)
        return NULL;
    if (PyDict_SetItemString(dict, name, v)) return NULL;
    return v;
}



static int
_new_constant_class(
    PyObject *module,
    PyModuleDef *moduledef,
    _constant_class *table
    )
{
    int i;
    PyObject * m = PyModule_Create(moduledef);
    if (m == NULL)
        return -1;
    /* int PyModule_AddIntConstant(PyObject *module, const char *name, long value)*/
    for (i = 0; table[i].name; i++) {
        PyModule_AddIntConstant(m,table[i].name,(long)table[i].value);
        /*printf("created %s %ld\n",table[i].name,(long)table[i].value);*/
    }
    PyModule_AddObject(module, moduledef->m_name, m);
    return 0;
}

static struct PyModuleDef module_adns = {
    PyModuleDef_HEAD_INIT,
    "adns",     /* m_name*/
    adns_module_documentation,  /* m_doc*/
    -1,                  /* m_size*/
    adns_methods,    /* m_methods*/
    NULL,                /* m_reload*/
    NULL,                /* m_traverse*/
    NULL,                /* m_clear*/
    NULL,                /* m_free*/
};

/* Initialization function for the module */

PyMODINIT_FUNC
PyInit_adns(void)
{
    PyObject *m, *d;

    /* Create the module and add the functions*/
    m = PyModule_Create(&module_adns);
    if (m == NULL)
        return NULL;
    /*printf("created module adns %s %s\n",__DATE__, __TIME__);*/

    /* Initialize object type properly*/
    /* maybe not a good idea, because of init values in newADNS_Queryobject*/
    /* ADNS_Querytype.tp_new = newADNS_Queryobject; // tp_new is complicated*/
    /* no need to create instances from py 
    ADNS_Querytype.tp_new = PyType_GenericNew;*/
    if (PyType_Ready(&ADNS_Querytype) < 0)
        return NULL;
    Py_INCREF(&ADNS_Querytype);
    PyModule_AddObject(m, "Query", (PyObject *)&ADNS_Querytype);
    
    /* ADNS_Statetype.tp_new = newADNS_Stateobject;*/
    /*    
    ADNS_Statetype.tp_new = PyType_GenericNew;*/
    if (PyType_Ready(&ADNS_Statetype) < 0)
        return NULL;
    Py_INCREF(&ADNS_Statetype);
    PyModule_AddObject(m, "State", (PyObject *)&ADNS_Statetype);

    /* Add some symbolic constants to the module*/
    d = PyModule_GetDict(m);
    /**  Py_INCREF + PyModule_AddObject( ??? - not using the object dict ?*/
    ErrorObject = _new_exception(d, "Error", PyExc_ConnectionError);
    NotReadyError = _new_exception(d, "NotReady", ErrorObject);
    LocalError = _new_exception(d, "LocalError", ErrorObject);
    RemoteError = _new_exception(d, "RemoteError", ErrorObject);
    RemoteFailureError = _new_exception(d, "RemoteFailureError", RemoteError);
    RemoteTempError = _new_exception(d, "RemoteTempError", RemoteError);
    RemoteConfigError = _new_exception(d, "RemoteConfigError", RemoteError);
    QueryError = _new_exception(d, "QueryError", ErrorObject);
    PermanentError = _new_exception(d, "PermanentError", ErrorObject);
    NXDomainError = _new_exception(d, "NXDomain", PermanentError);
    NoDataError = _new_exception(d, "NoData", PermanentError);

    /* XXXX Add constants here*/
    /**  Py_INCREF + PyModule_AddObject( ??? - not using the object dict ?*/
    _new_constant_class(m, &module_iflags, adns_iflags);
    _new_constant_class(m, &module_qflags, adns_qflags);
    _new_constant_class(m, &module_rr, adns_rr);
    _new_constant_class(m, &module_status, adns_s);
    /* Check for errors*/
    if (PyErr_Occurred())
        Py_FatalError("can't initialize module adns");

    return m;
}


