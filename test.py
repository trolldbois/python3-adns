#!/usr/bin/env python

"""Test functions."""
import time

import adns

import socket
import ipaddress
import unittest

import ADNS


def get_ip(hostname):
    return socket.gethostbyname_ex(hostname)[2][0]


class Test_adns(unittest.TestCase):
    """
    queries returns status, CNAME, expires, answer
    """

    def setUp(self):
        self.resolver = adns.init()

    def test_aaaa(self):
        fqdn = "one.one.one.one"
        result = self.resolver.synchronous(fqdn, adns.rr.AAAA)
        ip_tuple = result[3]
        self.assertIn('2606:4700:4700::1001', ip_tuple)
        self.assertIn('2606:4700:4700::1111', ip_tuple)

    def test_a(self):
        fqdn = "one.one.one.one"
        result = self.resolver.synchronous(fqdn, adns.rr.A)
        ip_tuple = result[3]
        self.assertIn('1.1.1.1', ip_tuple)
        self.assertIn('1.0.0.1', ip_tuple)

    def test_ptr(self):
        fqdn = b"one.one.one.one"
        ptr = '1.1.1.1.in-addr.arpa.'
        result = self.resolver.synchronous(ptr, adns.rr.PTR)
        fqdn_tuple = result[3]
        self.assertIn(fqdn, fqdn_tuple)

    def test_synchronous(self):
        # sync
        # Results are generally returned as a 4-tuple: status, CNAME, expires, answer
        host = "google-public-dns-a.google.com."
        result = self.resolver.synchronous(host, adns.rr.A)
        self.assertEqual(result[0], 0)
        ip1 = ipaddress.ip_address(result[3][0])
        ip2 = ipaddress.ip_address(get_ip(host))
        self.assertEqual(ip1, ip2)


class Test_ADNS(unittest.TestCase):
    """
    queries returns status, CNAME, expires, answer
    """

    def setUp(self):
        self.resolver = ADNS.QueryEngine()

    # def submit(self, qname, rr, flags=0, callback=None, extra=None):

    # def submit_reverse(self, qname, rr, flags=0, callback=None, extra=None):

    # def submit_reverse_any(self, qname, rr, flags=0, callback=None, extra=None):

    # def cancel(self, query):

    # def test_run(self, timeout=0):

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
