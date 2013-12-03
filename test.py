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
    self.assertEqual(result[0],0)
    ip1 = ipaddress.ip_address(result[3][0])
    ip2 = ipaddress.ip_address(get_ip(host))
    self.assertEqual(ip1,ip2)
  
  #def submit(self, qname, rr, flags=0, callback=None, extra=None):

  #def submit_reverse(self, qname, rr, flags=0, callback=None, extra=None):

  #def submit_reverse_any(self, qname, rr, flags=0, callback=None, extra=None):

  #def cancel(self, query):

  #def run(self, timeout=0):

  #def finished(self):

  #def finish(self):

  #def run_max(self, max):

  #def globalsystemfailure(self):





if __name__ == '__main__':
  unittest.main()
