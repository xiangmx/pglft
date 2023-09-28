package main

import (
	"fmt"
	"go-work/chat-tool/conmon/massage"
	"go-work/chat-tool/server/process"
	"go-work/chat-tool/server/untils"
	"io"
	"net"
)

type Processor struct {
	Conn net.Conn
}

//服务器处理各种类型的消息进行调度

func (this *Processor) ServerprocessMes(mes *massage.LoginType) (err error) {
	//fmt.Println("mes=", mes.Date)
	switch mes.Type {
	case massage.LoginMessageType:
		//处理登录
		userprocess := &process2.UserProcess{
			Conn: this.Conn,
		}
		err = userprocess.ServerprocessLogin(mes)
		if err != nil {
			fmt.Println("处理登录失败,err=", err)
			return
		}
	case massage.RegisLoginMesType:
		//处理注册
		userprocess := &process2.UserProcess{
			Conn: this.Conn,
		}
		err = userprocess.ServerProcessRegis(mes)
		if err != nil {
			fmt.Println("处理注册失败,err=", err)
			return
		}
	case massage.CurUserMesType:
		//处理群发消息
		smspro := &process2.SmsProcess{}
		smspro.SendGroupMes(mes)
	case massage.SmsOneType:
		smspro := &process2.SmsProcess{}
		smspro.SendOneMes(mes)
	}
	return
}

func (this *Processor) Process2() (err error) {
	for {
		transfor := &untils.Transfor{
			Conn: this.Conn,
		}
		mes, err := transfor.Readpkg()
		if err != nil {
			if err == io.EOF {
				fmt.Println("客户端退出，服务端也退出....")
				return err
			} else {
				fmt.Println("readpkg err=", err)
				return err
			}
		}
		messer := &Processor{
			Conn: this.Conn,
		}
		err = messer.ServerprocessMes(&mes)
		if err != nil {
			return err
		}
		fmt.Println("mes=", mes)
	}
}
