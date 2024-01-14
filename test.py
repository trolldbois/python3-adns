#!/usr/bin/env python

"""Test functions."""

import adns

import socket
import ipaddress
import unittest


def get_ip(hostname):
    return socket.gethostbyname_ex(hostname)[2][0]


class TestADNS(unittest.TestCase):

    def setUp(self):
        self.resolver = adns.init()

    def test_synchronous(self):
        # sync
        # Results are generally returned as a 4-tuple: status, CNAME, expires, answer
        host = "google-public-dns-a.google.com."
        result = self.resolver.synchronous(host, adns.rr.A)
        self.assertEqual(result[0], 0)
        ip1 = ipaddress.ip_address(result[3][0])
        ip2 = ipaddress.ip_address(get_ip(host))
        self.assertEqual(ip1, ip2)

    # def submit(self, qname, rr, flags=0, callback=None, extra=None):

    # def submit_reverse(self, qname, rr, flags=0, callback=None, extra=None):

    # def submit_reverse_any(self, qname, rr, flags=0, callback=None, extra=None):

    # def cancel(self, query):

    # def run(self, timeout=0):

    # def finished(self):

    # def finish(self):

    # def run_max(self, max):

    # def globalsystemfailure(self):


import DNSBL


class TestDNSBL(unittest.TestCase):
    def test_dnsbl(self):
        blacklists = [
            DNSBL.DNSBL('ORDB', 'relays.ordb.org.',
                        'http://ordb.org/lookup?addr=%s',
                        {'127.0.0.2': 'ORDB'}),
            DNSBL.DNSBL('DEVNULL', 'dev.null.dk.',
                        'http://fabel.dk/relay/test/index.epl?ip=%s&send=Check',
                        {'127.0.0.2': 'DEVNULL'}),
        ]

        s = DNSBL.DNSBLQueryEngine(blacklists=blacklists)
        for i in ['8.8.8.8', '182.0.1.1']:
            s.submit_dnsbl(i)
        s.finish()
        listed = s.dnsbl_results
        self.assertEqual(len(listed), 2)
        # for k, v in list(listed.items()):
        #     hits = []
        #     for l, url in v: hits.append(l)
        #     if len(listed) > 1:
        #         print("%s: %s" % (k, ','.join(hits)))
        #     else:
        #         print(','.join(hits))


if __name__ == '__main__':
    unittest.main()
    # TestDNSBL().test_dnsbl()
