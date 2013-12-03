import os, sys, string
import ADNS

class DNSBL:

    """A class for defining various DNS-based blacklists."""

    def __init__(self, name, zone, URL='', results=None):
        """Create a DNS blacklist name, based on the given zone.
        If presently, URL is a template that produces a link
        back to information for a given address. results
        should map returned addresses to list codes."""

        self.name = name
        self.zone = zone
        self.URL = URL
        self.results = {}
        if results:
            for result, name in list(results.items()): self.result(result, name)

    def result(self, result, name):
        """Add a possible result set."""
        self.results[result] = name

    def getURL(self, ip):
        """Return a URL to information on the list of ip on this
        blacklist."""
        return self.URL % ip
    

class DNSBLQueryEngine(ADNS.QueryEngine):

    def __init__(self, s=None, blacklists=None):
        ADNS.QueryEngine.__init__(self, s)
        self.blacklists = {}
        self.dnsbl_results = {}
        if blacklists:
            for l in blacklists: self.blacklist(l)
            
    def blacklist(self, dnsbl):
        """Add a DNSBL."""
        self.blacklists[dnsbl.name] = dnsbl
        
    def submit_dnsbl(self, qname):
        from adns import rr
        for l, d in list(self.blacklists.items()):
            self.dnsbl_results[qname] = []
            self.submit_reverse_any(qname, d.zone, rr.A,
                                    callback=self.dnsbl_callback,
                                    extra=l)

    def dnsbl_callback(self, answer, qname, rr, flags, l):
        if not answer[0]:
            for addr in answer[3]:
                self.dnsbl_results[qname].append( (
                    self.blacklists[l].results.get(addr, "%s-%s"%(l,addr)),
                    self.blacklists[l].getURL(qname)) )

if __name__ == "__main__":
    blacklists = [
        DNSBL('ORDB', 'relays.ordb.org.',
              'http://ordb.org/lookup?addr=%s',
              {'127.0.0.2': 'ORDB'}),
        DNSBL('DEVNULL', 'dev.null.dk.',
              'http://fabel.dk/relay/test/index.epl?ip=%s&send=Check',
              { '127.0.0.2': 'DEVNULL' }),
        ]

    s = DNSBLQueryEngine(blacklists=blacklists)
    for i in sys.argv[1:]:
        s.submit_dnsbl(i)
    s.finish()
    listed = s.dnsbl_results
    for k, v in list(listed.items()):
        hits = []
        for l, url in v: hits.append(l)
        if len(listed) > 1:
            print("%s: %s" % (k, ','.join(hits)))
        else:
            print(','.join(hits))
            
