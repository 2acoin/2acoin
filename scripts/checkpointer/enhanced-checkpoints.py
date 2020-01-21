#!/usr/bin/python
# -*- coding: utf-8 -*-
# Copyright (c) 2018, The TurtleCoin Developers
# Copyright (c) 2018, 2ACoin Developers
# 
# Please see the included LICENSE file for more information.

import sys
import json
import requests
import datetime

topbuffer = 1


def lastknownblock():
    try:
        with open(sys.argv[1], 'r') as cp:
            (block, hash) = list(cp)[-1].split(',')
            cp.close()
            return int(block)
    except:
        return 0


def height():
    base_url = 'http://localhost:17910/getheight'
    resp = requests.get(base_url).json()
    if 'height' not in resp:
        print ('Unexpected response, make sure 2ACoind is running',
               resp)
        sys.exit(-1)
    else:
        return resp['height']


def rpc(method, params={}):
    base_url = 'http://localhost:17910/json_rpc'
    payload = {
        'jsonrpc': '2.0',
        'id': 'block_info',
        'method': method,
        'params': params,
        }
    resp = requests.post(base_url, data=json.dumps(payload)).json()
    if 'result' not in resp:
        print ('Unexpected response, make sure 2ACoind is running with block explorer enabled'
               , resp)
        sys.exit(-1)
    else:
        if len('result') < 1:
           return resp
        else:
           return resp['result']


def get_height():
    resp = rpc('getblockcount')
    return resp['count']


def get_block_info(from_height):
    resp = rpc('f_blocks_list_json', {'height': from_height})
    return resp['blocks']

def get_block_hash(height):
   resp = rpc('on_getblockhash', [height])
   return resp

stop_height = lastknownblock() + 1

current_height = height() - topbuffer
process_height = 1
start_time = datetime.datetime.now()
with open(sys.argv[1], 'a') as f:
   while process_height < current_height:
       try:
           hash = get_block_hash(process_height+1)
           check = (process_height % 1000)
           if check == 0:
               print '%s,%s' % (process_height, hash)
           f.write('%s,%s\n' % (process_height,hash))
           process_height = process_height + 1
           if process_height > current_height:
               break
       except:
           print "Whoops... let's try that again"

#   if process_height > 1:
#       f.write('\n')
end_time = datetime.datetime.now()
print '#############################################'
print 'Current Height = %s' % current_height
print 'Start Time     = %s' % start_time.strftime("%c")
print 'End Time       = %s' % end_time.strftime("%c")
print '#############################################'

