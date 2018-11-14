import subprocess
import time
import json
import requests
import tornado
import tornado.escape
import tornado.httpserver
import tornado.httpclient
import tornado.ioloop
import tornado.locks
import tornado.options
import tornado.web
import tornado.websocket
from urllib.parse import urlencode, unquote
from tornado.options import define, options
from base import BaseHandler

define("port", default=12345, help="run on the given port", type=int)

class traditionalJudger(tornado.web.RequestHandler):
	def post(self):
		data = self.request.body
		judger = subprocess.Popen( ['./tradiJudger', \
									'--tl=%d' % data['TIME_LIMIT'],
									'--ml=%d' % data['MEMORY_LIMIT'],\
									'--ol=%d' % data['OUTPUT_LIMIT'],\
									'--in-pre=%s' % data['INPRE'],\
									'--in-suf=%s' % data['INSUF'],\
									'--out-pre=%s' % data['OUTPRE'],\
									'--out-suf=%s' % data['OUTSUF'],\
									'--Lang=%s' % data['Language'],\
									'--data-dir=%s' % data['DATA_DIR'],\
									'--checker=%s' % data['CHECKER'],\
									'--n-tests=%d' % data['NTESTS'],\
									'--source-name=%s' % data['SOURCE_FILE'],\
									'--source-dir=%s' % data['SOURCE_DIR']
									], stdout=subprocess.PIPE)


class Application(tornado.web.Application):
	def __init__(self):
		handlers = [
			(r"/traditionaljudger", traditionalJudger),
		]
		settings = dict(
			ui_modules = {},
			debug = True,
		)
		super(Application, self).__init__(handlers, **settings)

if __name__ == "__main__":
	judger = subprocess.Popen( ['./tradiJudger', \
								'--tl=1', '--ml=256', '--ol=64',\
								'--in-pre=test', '--in-suf=in',\
								'--out-pre=test', '--out-suf=out',\
								'--Lang=C++',\
								'--data-dir=/home/ycdfwzy/myworkspace/tinyjudger/test/',\
								'--checker=ncmp',  '--n-tests=2', '--source-name=test',\
								'--source-dir=/home/ycdfwzy/myworkspace/tinyjudger/test/'
								], stdout=subprocess.PIPE)
	time.sleep(1)
	judger.kill()
	# app = Application()
	# app.listen(options.port)
	# tornado.ioloop.IOLoop.current().start()