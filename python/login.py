#! /usr/bin/env python
#coding=utf-8
db = {}
def login():
    prompt = '用户名: '
    while True:
        name = raw_input(prompt)
        if db.has_key(name):
            prompt = ': 用户名被注册，请重试'
            continue
        else:
            break
    pwd = raw_input('密码： ')
    db[name] = pwd
   
def download():
    name = raw_input('用户名: ')
    pwd = raw_input('密码: ')
    passwd = db.get(name)
    if passwd == pwd:
        print '欢迎您', name
    else:
        print '登陆失败!'
       
def showmenu():
    promt = """
    		欢迎您
×××××××××××××××××××××××××××××××××
	    (N)新用户注册
	    (E)登陆
	    (Q)退出
      请输入选择: """
    done = False
    while not done:
        chosen = False
        while not chosen:
            try:
                choice = raw_input(promt).strip()[0].lower()
            except(EOFError, KeyboardInterrupt):
                choice = 'q'
            print '\n你选择了: [%s]' % choice
            if choice not in 'neq':
                print '输入错误，请重新输入!'
            else:
                chosen = True
        if choice == 'q':done = True
        if choice == 'n':login()
        if choice == 'e':download()
       
if __name__ == '__main__':
    showmenu()


