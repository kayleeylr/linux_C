/*************************************************************       
    FileName : makefile  
    FileFunc : Linux编译链接源程序,生成目标文件,库或者可执行代码     
    Version  : V0.1       
    Author   : Sunrier       
    Date     : 2012-06-14  
    Descp    : Linux下makefile模板      
*************************************************************/   
#makefile开始 

#定义宏
OBJS = test config

#all objects
all:$(OBJS)

#where are include files kept
INCLUDE = .
#.表示当前目录下

#Options -O for release and -g for development
#-w:不生成任何警告信息
#-Wall:输出所有的警告信息
#-O:在编译时进行优化(注:O为大写字母)
#-O0:不进行优化处理
#-O或-O1:优化生成代码,缺省值
#-O2:进一步优化
#-O3:比-O2更进一步优化,包括inline函数
#-g:表示编译debug版本,生成调试信息(注:使用GNU的gdb必须加入这个选项)
#-ansi:只支持ANSI标准的C语法,这一选项将禁止GNU C的某些特色,例如asm或typeof关键词	
CFLAGS = -g -Wall -ansi 
#CFLAGS = -O -Wall -ansi

#which compiler
CC = gcc $(CFLAGS)

MYSQLLIB= -lmysqlclient -lz -m
MYSQLPATH = -I/usr/include/mysql -L/usr/lib/mysql 

TCC = gcc $(CFLAGS) -I$(SYSINCLPATH) -I$(APPINCLPATH)  -L$(APPLIBPATH) -L$(SYSLIBPATH)

CLEANFILES =  *.o 

#Set system path
#主路径
MAINPATH=$(HOME)
#系统库头文件
SYSINCLPATH=$(MAINPATH)/include
#系统库程序
SYSLIBPATH=$(MAINPATH)/lib
#执行文件
BINPATH=$(MAINPATH)/bin
#源程序路径
SRCPATH=$(MAINPATH)/src
#应用程序路径
APPINCLPATH=$(MAINPATH)/src
#应用程序库
APPLIBPATH=$(MAINPATH)/src

#Set user lib path
#lib head file path
LIBINCLPATH = .

#lib source file path
LIBSRCPATH = .

#.SUFFIXES:	先清除默认的后缀
#.SUFFIXES: .ec .sqc 定义自己的后缀

#ar打包
#-c create的意思 
#-r replace的意思,表示当插入的模块名已经在库中存在,则替换同名的模块.
#如果若干模块中有一个模块在库中不存在,ar显示一个错误消息,并不替换其他同名模块.
#默认的情况下,新的成员增加在库的结尾处,可以使用其他任选项来改变增加的位置。
AR = ar -rc

#$@ 扩展成当前规则的目的文件名
#$< 扩展成依靠列表中的第一个依靠文件
#$^ 扩展成整个依靠的列表(除掉了里面所有重 复的文件名)
#前面加@不回显执行的命令在标准输出上
#libtool.a: $(LIBSRCPATH)/
libtool.a: $(LIBSRCPATH)/ $(LIBINCLPATH)/*.h
	@-$(AR) $@ $(LIBSRCPATH)/
	@rm -rf $(LIBSRCPATH)/*.o

#.c.o:
#	@$(TCC) -c $<
	
	
#%.o: %.c
#@$(TCC) -c $< -o $@


#Manager:$(OBJS)
#	@$(CC)  -o Manager $(OBJS)
#gcc test.o config.o -o Manager
test.o:test.c config.h
	@$(CC) -I$(INCLUDE) $(CFLAGS) -c test.c -o test.o

#test:test.c test.h
#	@$(TCC) -o $(BINPATH)/$@ $? 	
config.o:config.c config.h
	@$(CC) -I$(INCLUDE) $(CFLAGS) -c config.c -o config.o
clean:  
    @ls | grep -v ^makefile$$ | grep -v [.]c$$ | grep -v [.]h$$ | grep -v [.]sql$$ | xargs rm -rf
#	@cd $(LIBSRCPATH) && \   
#	@rm -rf $(LIBSRCPATH)/$(CLEANFILES)    
#makefile结束



