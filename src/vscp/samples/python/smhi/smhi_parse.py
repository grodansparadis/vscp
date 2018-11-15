#! /usr/bin/python

# https://forum.telldus.com/viewtopic.php?t=4135

import urllib2
import json
import argparse
from time import strftime, localtime
from datetime import datetime, timedelta

debug = True

parser = argparse.ArgumentParser(description='Read temperature from SMHI')
parser.add_argument('hours', type=int)
parser.add_argument('latitude', type=float)
parser.add_argument('longitude', type=float)
args = parser.parse_args()
if debug:
    print args.hours, args.longitude, args.latitude

url = 'http://opendata-download-metfcst.smhi.se/api/category/pmp3g/version/2/geotype/point/lon/' + str(args.longitude) + '/lat/' +  str(args.latitude) + '/data.json'
#
#url = 'http://opendata-download-metfcst.smhi.se/api/category/pmp2g/version/1/geopoint/lat/%27+str%28args.latitude%29+%27/lon/%27+str%28args.longitude%29+%27/data.json'
#url='http://opendata-download-metfcst.smhi.se/api/category/pmp1g/version/1/geopoint/lat/'+str(args.latitude)+'/lon/'+str(args.longitude)+'/data.json'
if debug:
    print url  
j = urllib2.urlopen(url)
j_obj = json.load(j)

output = 'temperature'
for predition_time in j_obj['timeSeries']:
    for i in range(0, args.hours):
        if predition_time['validTime'] == (datetime.now() + timedelta(hours=i)).strftime("%Y-%m-%dT%H:00:00Z"):
            #output += ","+ str(predition_time['parameters'][1]['values'][0])
            for parameter in predition_time['parameters']:
                if parameter['name'] == 't':
                    output += "," + str(parameter['values'][0])
print output
