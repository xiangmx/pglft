package process

import (
	"encoding/json"
	"fmt"
	untils "go-work/chat-tool/client/utils"
	"go-work/chat-tool/conmon/massage"
)

type SmsProcess struct {
}

//发送群消息

func (this *SmsProcess) SendGroupMes(content string) (err error) {
	var logmes massage.LoginType
	logmes.Type = massage.CurUserMesType
	var curmes massage.CurUserMes
	curmes.Content = content
	curmes.UserId = curUser.UserId
	curmes.UserStatus = curUser.UserStatus
	curmes.UserName = curUser.UserName
	data, err := json.Marshal(curmes)
	if err != nil {
		fmt.Println("json.Marshal curmes err=", err)
		return
	}
	logmes.Date = string(data)
	data, err = json.Marshal(logmes)
	if err != nil {
		fmt.Println("json.Marshal logmes err=", err)
		return
	}
	tf := &untils.Transfor{
		Conn: curUser.Conn,
	}
	err = tf.WritePkg(data)
	if err != nil {
		fmt.Println("WritePkg SendGroupMes err=", err)
		return
	}
	return
}

//处理接受的群消息

func OutPutGroupMes(sms *massage.CurUserMes) {
	intfout := fmt.Sprintf("用户id:\t%d 对大家说：\t%s", sms.UserId, sms.Content)
	fmt.Println(intfout)
	fmt.Printf("\n\n")
}

// 发送1v1消息

func (this *SmsProcess) SendOneMes(content string, id int) (err error) {
	var logmes massage.LoginType
	logmes.Type = massage.SmsOneType
	var curmes massage.SmsOne
	curmes.Content = content
	curmes.MyId = curUser.UserId
	curmes.UserId = id
	data, err := json.Marshal(curmes)
	if err != nil {
		fmt.Println("json.Marshal curmes err=", err)
		return
	}
	logmes.Date = string(data)
	data, err = json.Marshal(logmes)
	if err != nil {
		fmt.Println("json.Marshal logmes err=", err)
		return
	}
	tf := &untils.Transfor{
		Conn: curUser.Conn,
	}
	err = tf.WritePkg(data)
	if err != nil {
		fmt.Println("WritePkg SendGroupMes err=", err)
		return
	}
	return
}

//接收1v1消息

func OutPutOneMes(sms *massage.SmsOne) {
	intfout := fmt.Sprintf("用户id:\t%d 对你说：\t%s", sms.MyId, sms.Content)
	fmt.Println(intfout)
	fmt.Printf("\n\n")
}
