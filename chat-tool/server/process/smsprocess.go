package process2

import (
	"encoding/json"
	"fmt"
	"go-work/chat-tool/conmon/massage"
	"go-work/chat-tool/server/untils"
	"net"
)

type SmsProcess struct {
}

//群发送给其他客户端

func (this *SmsProcess) SendGroupMes(mes *massage.LoginType) {
	var sms massage.CurUserMes
	err := json.Unmarshal([]byte(mes.Date), &sms)
	if err != nil {
		fmt.Println("Unmarshal SendGroupMes err=", err)
		return
	}
	data, err := json.Marshal(mes)
	if err != nil {
		fmt.Println("marshal SendGroupMes err=", err)
		return
	}
	usermgr := userMgr.GetAllUserMgr()
	for id, up := range usermgr {
		if id == sms.UserId {
			continue
		}
		this.SendMesOne(up.Conn, data)
	}
}

//给指定连接发送消息

func (this *SmsProcess) SendMesOne(conn net.Conn, data []byte) {
	tf := &untils.Transfor{
		Conn: conn,
	}
	err := tf.WritePkg(data)
	if err != nil {
		fmt.Println("WritePkg SendMesOne err=", err)
		return
	}
}

//处理1v1连接发送消息

func (this *SmsProcess) SendOneMes(mes *massage.LoginType) {
	var sms massage.SmsOne
	err := json.Unmarshal([]byte(mes.Date), &sms)
	if err != nil {
		fmt.Println("Unmarshal SendGroupMes err=", err)
		return
	}
	data, err := json.Marshal(mes)
	if err != nil {
		fmt.Println("marshal SendGroupMes err=", err)
		return
	}
	usermgr := userMgr.GetAllUserMgr()
	for id, up := range usermgr {
		if id == sms.UserId {
			this.SendMesOne(up.Conn, data)
		}
		//this.SendMesOne(up.Conn, data)
	}
}
