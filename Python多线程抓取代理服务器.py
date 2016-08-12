#coding: utf-8

import urllib2
import re
import time
import threading
import MySQLdb

rawProxyList = []
checkedProxyList = []

#抓取代理网站
targets = []
for i in xrange(1, 23):
    target = r"http://www.proxy.com.ru/list_%d.html" % i
    targets.append(target)
    #print target + "\n"

#抓取代理服务器正则
p = re.compile(r'''<tr><b><td>(\d+)</td><td>(.+?)</td><td>(\d+)</td><td>(.+?)</td><td>(.+?)</td></b></tr>''')

#获取代理的类

class ProxyGet(threading.Thread):
    def __init__(self, target):
        threading.Thread.__init__(self)
        self.target = target


    def getProxy(self):
        req = urllib2.Request(self.target)
        respnse = urllib2.urlopen(req)
        result = respnse.read()
        matches = p.findall(result)
        #print matches
        for row in matches:
            ip = row[1]
            port = row[2]
            addr = row[4].decode("cp936").encode("utf-8")
            proxy = [ip, port, addr]
            #print proxy
            rawProxyList.append(proxy)


    def run(self):
        self.getProxy()

#核对代理是否有效的类
class ProxyCheck(threading.Thread):
    def __init__(self,proxyList):
        threading.Thread.__init__(self)
        self.proxyList = proxyList
        self.timeout = 5
        self.testUrl = "http://www.baidu.com/"
        self.testStr = "030173"

    def checkProxy(self):
        cookies = urllib2.HTTPCookieProcessor()
        for proxy in self.proxyList:
            proxyHandler = urllib2.ProxyHandler({"http": r'http://%s:%s' %(proxy[0], proxy[1])})
            #print r'http://%s:%s' %(proxy[0],proxy[1])
            opener = urllib2.build_opener(cookies, proxyHandler)
            opener.addheaders = [('User-agent', 'Mozilla/5.0 (Windows NT 6.2; WOW64; rv:22.0) Gecko/20100101 Firefox/22.0')]
            #urllib2.install_opener(opener)
            t1 = time.time()

            try:
                #req = urllib2.urlopen("http://www.baidu.com", timeout=self.timeout)
                req = opener.open(self.testUrl, timeout=self.timeout)
                #print "urlopen is ok...."
                result = req.read()
                #print "read html...."
                timeused = time.time() - t1
                pos = result.find(self.testStr)
                #print "pos is %s" %pos

                if pos >= 1:
                    checkedProxyList.append((proxy[0], proxy[1], proxy[2], timeused))
                    print "ok ip: %s %s %s %s" %(proxy[0],proxy[1],proxy[2],timeused)
                else:
                     continue
            except Exception, e:
                #print e.message
                continue

    def run(self):
        self.checkProxy()


if __name__ == "__main__":
    getThreads = []
    checkThreads = []

#对每个目标网站开启一个线程负责抓取代理
for i in range(len(targets)):
    t = ProxyGet(targets[i])
    getThreads.append(t)

for i in range(len(getThreads)):
    getThreads[i].start()

for i in range(len(getThreads)):
    getThreads[i].join()

print '.'*10 + "总共抓取了%s个代理" % len(rawProxyList) + '.'*10

#开启20个线程负责校验，将抓取到的代理分成20份，每个线程校验一份
for i in range(20):
    t = ProxyCheck(rawProxyList[((len(rawProxyList)+19)/20) * i:((len(rawProxyList)+19)/20) * (i+1)])
    checkThreads.append(t)

for i in range(len(checkThreads)):
    checkThreads[i].start()

for i in range(len(checkThreads)):
    checkThreads[i].join()

print '.'*10 + "总共抓取了%s个代理" % len(checkedProxyList) + '.'*10

#插入数据库，四个字段ip, port, speed, addr
def db_insert(insert_list):
    try:
        conn = MySQLdb.connect(host="127.0.0.1", user="root", passwd="root", db="cdma", charset='utf8')
        cursor = conn.cursor()
        cursor.execute('delete from proxy')
        cursor.execute('alter table proxy AUTO_INCREMENT=1')
        cursor.executemany("INSERT INTO proxy(ip,port,speed,address) VALUES(%s, %s, %s,%s)", insert_list)
        conn.commit()
        cursor.close()
        conn.close()

    except MySQLdb.Error, e:
        print "Mysql Error %d: %s" %(e.args[0], e.args[1])

#代理排序持久化
proxy_ok = []
for proxy in sorted(checkedProxyList, cmp=lambda x, y: cmp(x[3], y[3])):
    if proxy[3] < 8:
        #print "checked proxy is: %s:%s\t%s\t%s" %(proxy[0],proxy[1],proxy[2],proxy[3])
        proxy_ok.append((proxy[0], proxy[1], proxy[3], proxy[2]))

db_insert(proxy_ok)
