#!/usr/bin/env python
import urllib2
i = 0
url = 'http://localhost:8080/index.html'
while i < 100:
    urllib2.urlopen(url);
