import json
# import aiopg
# import bcrypt
# import markdown
import os.path
# import psycopg2
import re
import tornado.escape
import tornado.httpserver
import tornado.ioloop
import tornado.locks
import tornado.options
import tornado.web
import unicodedata

# from tornado.options import define, options
# define("port", default=8000, help="run on the given port", type=int)
# define("db_host", default="127.0.0.1", help="blog database host")
# define("db_port", default=5432, help="blog database port")
# define("db_database", default="thssoj", help="blog database name")
# define("db_user", default="postgres", help="blog database user")
# define("db_password", default="zUY3Z2N2ul", help="blog database password")


class NoResultError(Exception):
    pass


# class DatabaseRowObject(tornado.util.ObjectDict):
#     def __init__(self, db, *args, **kw):
#         self.db = db
#         super(DatabaseRowObject, self).__init__(*args, **kw)
#
#     async def execute(self, stmt, *args):
#         """Execute a SQL statement.
#
#         Must be called with ``await self.execute(...)``
#         """
#         with (await self.application.db.cursor()) as cur:
#             await cur.execute(stmt, args)
#
#     async def save(self):
#         properties = []
#         for key, value in self:
#             if key != 'db' and key != 'id':
#                 properties.append(str(key) + ' = ' + str(value))
#                 print(key)
#         # self.execute('''UPDATE ''')


async def maybe_create_tables(db, filename):
    # try:
    #     with (await db.cursor()) as cur:
    #         await cur.execute("SELECT COUNT(*) FROM entries LIMIT 1")
    #         await cur.fetchone()
    #     print("in create")
    # except psycopg2.ProgrammingError:
        print('create tables')
        with open(filename) as f:
            schema = f.read()
        with (await db.cursor()) as cur:
            await cur.execute(schema)
    # with open('schema.sql') as f:
    #     schema = f.read()
    # with (await db.cursor()) as cur:
    #     await cur.execute(schema)

class Application(tornado.web.Application):
    def __init__(self, db, *args, **kw):
        self.db = db
        super(Application, self).__init__(*args, **kw)


class BaseHandler(tornado.web.RequestHandler):
    def __init__(self, *args, **kw):
        super(BaseHandler, self).__init__(*args, **kw)
        self.getargs()


    def row_to_obj(self, row, cur):
        """Convert a SQL row to an object supporting dict and attribute access."""
        # obj = tornado.util.ObjectDict()
        obj = tornado.util.ObjectDict()
        for val, desc in zip(row, cur.description):
            obj[desc.name] = val
        return obj

    async def execute(self, stmt, *args):
        """Execute a SQL statement.
        Must be called with ``await self.execute(...)``
        """
        print('execute: ', stmt, args)
        with (await self.application.db.cursor()) as cur:
            await cur.execute(stmt, args)

    async def query(self, stmt, *args):
        """Query for a list of results.
        Typical usage::
            results = await self.query(...)
        Or::
            for row in await self.query(...)
        """
        with (await self.application.db.cursor()) as cur:
            await cur.execute(stmt, args)
            res = [self.row_to_obj(row, cur)
                    for row in await cur.fetchall()]
            # for item in res:
            #     print(item)
            # for xx in res:
            #     xx.save()
            # res[0].name = 'ycdfwzy'
            # self.saveObject('users', res[0])
            return res
    async def queryone(self, stmt, *args):
        """Query for exactly one result.
        Raises NoResultError if there are no results, or ValueError if
        there are more than one.
        """
        results = await self.query(stmt, *args)
        if len(results) == 0:
            raise NoResultError()
        elif len(results) > 1:
            raise ValueError("Expected 1 result, got %d" % len(results))
        print(results[0])
        return results[0]

    async def prepare(self):
        pass
        # get_current_user cannot be a coroutine, so set
        # self.current_user in prepare instead.
        # user_id = self.get_secure_cookie("blogdemo_user")
        # print(user_id)
        # if user_id:
        #     self.current_user = await self.queryone("SELECT * FROM authors WHERE id = %s",
        #                                             int(user_id))

    async def any_author_exists(self):
        return bool(await self.query("SELECT * FROM authors LIMIT 1"))

    async def dropTable(self, si_table_name):
        await self.execute('''DROP TABLE IF EXISTS {table_name};'''.format(table_name = si_table_name))


    async def createTable(self, si_table_name, **kw):
        await self.execute('''CREATE TABLE {table_name} (\n{cols_info}\n );'''.format(table_name = si_table_name, cols_info = ',\n'.join(map(lambda tp : str(tp[0]) + ' ' + str(tp[1]), kw.items()))), None)

    async def saveObject(self, si_table_name, object):
        plst = []
        for key, value in object.items():
            if key != 'id':
                plst.append(str(key) + ' = ' + str(value))
            slst = ','.join(plst)
        await self.execute('''UPDATE {table_name} SET {prop} WHERE id = {oid}'''.format(table_name = si_table_name, prop = slst, oid = object['id']))

    async def all(self, si_table_name):
        return await self.query('''SELECT * FROM {table_name}'''.format(table_name = si_table_name))

    async def getObject(self, si_table_name, **kw):
        plst = []
        valuelist = []
        for key, value in kw.items():
            # if key != 'id':
            plst.append(str(key) + ' = %s')
            valuelist.append(value)
        slst = ','.join(plst)
        return await self.query('''SELECT * FROM {table_name} WHERE {conditions}'''.format(table_name = si_table_name, conditions = slst), *valuelist)

    async def createObject(self, si_table_name, **kw):
        propfmt = ['%s'] * len(kw)
        spropfmt = ','.join(propfmt)
        propkeys = []
        propvalues = []
        for key, value in kw.items():
            propkeys.append(str(key))
            propvalues.append(value)
        str_fmt = '''INSERT INTO {table_name} ({property_keys})\n VALUES ({property_fmt});'''.format(table_name = si_table_name, property_keys = ','.join(propkeys), property_fmt = spropfmt)
        print('fmt = ', str_fmt, propvalues)
        await self.execute(str_fmt, *propvalues)

    def getargs(self):
        print('getargs: ', self.request.body.decode() or '{}')
        self.args = json.loads(self.request.body.decode() or '{}')