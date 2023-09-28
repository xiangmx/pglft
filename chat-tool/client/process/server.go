package process

import (
	"encoding/json"
	"fmt"
	"go-work/chat-tool/client/utils"
	"go-work/chat-tool/conmon/massage"
	"net"
	"os"
)

func LoginList() {
	for {
		fmt.Println("==================登录成功==================")
		fmt.Println("\t\t1.显示在线用户列表")
		fmt.Println("\t\t2.发送消息")
		fmt.Println("\t\t3.私聊消息")
		fmt.Println("\t\t4.退出系统")
		fmt.Println("\t\t请选择(1-4)")
		var key int
		fmt.Scanln(&key)
		var id int
		var content string
		sms := &SmsProcess{}
		switch key {
		case 1:
			OutPutOnlineUsers()
		case 2:
			fmt.Println("你要发送什么消息呢：")
			fmt.Scanln(&content)
			sms.SendGroupMes(content)
		case 3:
			fmt.Println("请输入你要私聊的id:")
			fmt.Scanln(&id)
			fmt.Println("你要发送什么消息呢：")
			fmt.Scanln(&content)
			sms.SendOneMes(content, id)
		case 4:
			fmt.Println("你已退出系统.....")
			os.Exit(0)
		default:
			fmt.Println("你的输入有误，请重新输入 ")
		}
	}
}

//处理服务端发来的消息

func ProcessserverMes(conn net.Conn) {
	cp := &untils.Transfor{
		Conn: conn,
	}
	for {
		fmt.Println("客户端正在等待服务器发送的消息.....")
		mes, err := cp.Readpkg()
		if err != nil {
			fmt.Println("cp read err=", err)
			return
		}
		switch mes.Type {
		case massage.UserStatusMesType:
			var ud massage.UserStatusMes
			err = json.Unmarshal([]byte(mes.Date), &ud)
			if err != nil {
				fmt.Println("json.Unmarshal ud err=", err)
				return
			}
			ProcessUserStatusMes(ud)
		case massage.CurUserMesType:
			var sms massage.CurUserMes
			err = json.Unmarshal([]byte(mes.Date), &sms)
			if err != nil {
				fmt.Println("json.Unmarshal sms err=", err)
				return
			}
			OutPutGroupMes(&sms)
		case massage.SmsOneType:
			var sms massage.SmsOne
			err = json.Unmarshal([]byte(mes.Date), &sms)
			if err != nil {
				fmt.Println("json.Unmarshal sms err=", err)
				return
			}
			OutPutOneMes(&sms)
		default:
			fmt.Println("服务器返回未知消息...")
		}
	}
}
