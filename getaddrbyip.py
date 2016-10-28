# -*- coding:utf-8 -*-
# author:allenȨ
import sys
import urllib2
import json

def get_ip_information(ip):
    url='http://api.map.baidu.com/highacciploc/v1?qcip='+ip+'&qterm=pc&ak=cEOFb5BF15kviUF26y6RpFoPINsqiPFm&coord=bd09ll&extensions=3'
    poiss=''
    request = urllib2.Request(url)
    page = urllib2.urlopen(request, timeout=10)
    data_json = page.read()
    data_dic = json.loads(data_json)
    if(data_dic.has_key("content")):
        content=data_dic["content"]
        address_component=content["address_component"]
        formatted_address=content["formatted_address"]
        print "��IP��ַ�ľ���λ��Ϊ��"
        print address_component["country"]
        print formatted_address
        if (content.has_key("pois")):
            print "��IP��ַ����POI��Ϣ���£�"
            pois = content["pois"]
            for index in range(len(pois)):
                pois_name = pois[index]["name"]
                pois_address = pois[index]["address"]
                print pois_name, pois_address
    else:
        print 'IP��ַ��λʧ�ܣ�����'
if __name__ == '__main__':
    get_ip_information('116.231.64.199')
