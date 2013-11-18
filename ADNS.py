"""High-level interface to adns."""

import adns

class Error(Exception): pass

class QueryEngine:

    callback_submit = None
    callback_submit_reverse = None
    callback_submit_reverse_any = None
    
    def __init__(self, s=None):
        self._s = s or adns.init(adns.iflags.noautosys)
        self._queries = {}

    def synchronous(self, qname, rr, flags=0):
        return self._s.synchronous(qname, rr, flags)

    def submit(self, qname, rr, flags=0, callback=None, extra=None):
        callback = callback or self.callback_submit
        if not callback: raise Error("callback required")
        q = self._s.submit(qname, rr, flags)
        self._queries[q] = qname, rr, flags, callback, extra

    def submit_reverse(self, qname, rr, flags=0, callback=None, extra=None):
        callback = callback or self.callback_submit_reverse
        if not callback: raise Error("callback required")
        q = self._s.submit_reverse(qname, rr, flags)
        self._queries[q] = qname, rr, flags, callback, extra

    def submit_reverse_any(self, qname, rr, flags=0,
                           callback=None, extra=None):
        callback = callback or self.callback_submit_reverse_any
        if not callback: raise Error("callback required")
        q = self._s.submit_reverse_any(qname, rr, flags)
        self._queries[q] = qname, rr, flags, callback, extra

    def cancel(self, query):
        query.cancel()
        try: del self._queries[query]
        except KeyError: pass
        
    def run(self, timeout=0):
        for q in self._s.completed(timeout):
            answer = q.check()
            qname, rr, flags, callback, extra = self._queries[q]
            del self._queries[q]
            callback(*(answer, qname, rr, flags, extra))

    def finished(self):
        return not len(self._queries)

    def finish(self):
        while not self.finished():
            self.run(1)

    def run_max(self, max):
        from time import time
        quittime = time()+max
        while not self.finished() and time()<=quittime:
            self.run(1)

    def globalsystemfailure(self):
        self._s.globalsystemfailure()
        self._queries.clear()

init = QueryEngine
