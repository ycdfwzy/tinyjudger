# import tornado.web
# import tornado.httpclient

# @tornado.web.asynchronous
# def test_push_data2():
#     client = tornado.httpclient.AsyncHTTPClient()

# 	data = {"data_test":"1"}
#     data_send = urllib.urlencode(data)

# 	url = "http://127.0.0.1:12345"

# 	response = client.fetch(url, method='POST', body=data_send, callback = test_push_data2_resp)

# def test_push_data2_resp(resp):
# 	print(resp)

# test_push_data2()
import requests
import json
from urllib.parse import urlencode

data = dict()
data['TIME_LIMIT'] = 1
data['MEMORY_LIMIT'] = 256
data['OUTPUT_LIMIT'] = 64
data['INPRE'] = 'test'
data['INSUF'] = 'in'
data['OUTPRE'] = 'test'
data['OUTSUF'] = 'out'
data['Language'] = 'C++'
data['DATA_DIR'] = '/home/ycdfwzy/myworkspace/tinyjudger/test/'
data['CHECKER'] = 'ncmp'
data['NTESTS'] = 2
data['SOURCE_FILE'] = 'test'
data['SOURCE_DIR'] = '/home/ycdfwzy/myworkspace/tinyjudger/test/'

r = requests.post('http://localhost:12345/traditionaljudger', data = json.dumps(data))
print(json.dumps(r.text))