#!/usr/bin/python
# -*- coding: utf8 -*-

import os
import json
import urllib2
import requests


class Chat(object):
    key = "19b2db1fc83c800ae1e9ee75fc0b9964"   			#apikey
    apiaddress = "http://www.tuling123.com/openapi/api"  	#api地址
    userid = 'wangbo2008@vip.qq.com'

    def init(self):
        os.system("clear")		#清屏
        print "欢迎使用!  输入exit或者quit退出"
        print "-------------------------------"

    def get(self):#发送消息
        print "you： ",		
        scand = raw_input()	#输入对话内容
        if scand == 'exit' or scand == "quit":#当用户输入exit或者quit时候退出
            print "it: 再见"
            return				#程序退出
        self.send(scand)		

    def send(self, scand):#接受消息
        url = self.apiaddress + '?key=' + self.key + '&' + 'info=' + scand +'&userid='+self.userid
        
#请求URL示例:http://www.tuling123.com/openapi/api?key=APIKEY&info= 今 天 天 气 怎 么 样&loc=北京市中关村&userid=12345678
        
        recv = urllib2.urlopen(url).read()#获得html页面信息
        re_dict = json.loads(recv)	#把Json格式字符串解码转换成Python对象
        text = re_dict['text']
        print 'it： ', text
        self.get()
        
      

if __name__ == "__main__":
    chat = Chat()
    chat.init()
    chat.get()
