package process

import (
	"encoding/json"
	"fmt"
	"go-work/chat-tool/client/utils"
	"go-work/chat-tool/conmon/massage"
	"net"
)

type Userprocess struct {
}

func (this *Userprocess) SendMessage(UserId int, UserPwd string) {
	conn, err := net.Dial("tcp", "localhost:8889")
	if err != nil {
		fmt.Println("net.Dial err=", err)
		return
	}
	defer conn.Close()
	//fmt.Println("连接服务器成功")
	var logintype massage.LoginType
	logintype.Type = massage.LoginMessageType
	var loginmes massage.LodinMessage
	loginmes.UserId = UserId
	loginmes.UserPwd = UserPwd
	use, err := json.Marshal(loginmes)
	if err != nil {
		fmt.Println("json.Marshal(loginmes) err=", err)
		return
	}
	logintype.Date = string(use)
	fmt.Println(logintype.Date)
	types, err := json.Marshal(logintype)
	if err != nil {
		fmt.Println("json.Marshal(logintype) err=", err)
		return
	}
	transfor := &untils.Transfor{
		Conn: conn,
	}
	err = transfor.WritePkg(types)
	if err != nil {
		fmt.Println("发送包出现错误,err=", err)
		return
	}
	mes, err := transfor.Readpkg()
	if err != nil {
		fmt.Println("readpkg err=", err)
		return
	}
	var mesnum massage.LoginNum
	err = json.Unmarshal([]byte(mes.Date), &mesnum)
	if err != nil {
		fmt.Println("json.Unmarshal err=", err)
		return
	}
	if mesnum.Code == 200 {
		go ProcessserverMes(conn)
		curUser.Conn = conn
		curUser.UserId = UserId
		//curUser.UserName = username
		curUser.UserStatus = massage.UserOnline
		for _, v := range mesnum.UsersId {
			if v == UserId {
				continue
			}
			use := &massage.User{
				UserId:     v,
				UserStatus: massage.UserOnline,
			}
			onlineusers[v] = use
		}
		LoginList()
	} else {
		fmt.Println(mesnum.Error)
	}
	fmt.Println("关闭连接")
}

//发送注册用户信息

func (this *Userprocess) RegisMes(userId int, userPwd string, username string) {
	conn, err := net.Dial("tcp", "localhost:8889")
	if err != nil {
		fmt.Println("net.Dial err=", err)
		return
	}
	defer conn.Close()

	var regisMes massage.LoginType
	regisMes.Type = massage.RegisLoginMesType
	var remes massage.RegisLoginMes
	remes.User.UserId = userId
	remes.User.UserPwd = userPwd
	remes.User.UserName = username

	use, err := json.Marshal(remes)
	if err != nil {
		fmt.Println("json.Marshal err=", err)
		return
	}
	regisMes.Date = string(use)
	fmt.Println(regisMes.Date)
	types, err := json.Marshal(regisMes)
	if err != nil {
		fmt.Println("json.Marshal err=", err)
		return
	}

	transfor := &untils.Transfor{
		Conn: conn,
	}
	err = transfor.WritePkg(types)
	if err != nil {
		fmt.Println("发送包出现错误,err=", err)
		return
	}

	mes, err := transfor.Readpkg()
	if err != nil {
		fmt.Println("readpkg err=", err)
		return
	}
	var mesnum massage.RegisError
	err = json.Unmarshal([]byte(mes.Date), &mesnum)
	if err != nil {
		fmt.Println("json.Unmarshal err=", err)
		return
	}

	if mesnum.Code == 200 {
		fmt.Println("注册成功")
		//curUser.Conn = conn
		//curUser.UserId = userId
		//curUser.UserName = username
		//curUser.UserStatus = massage.UserOnline
		//LoginList()
	} else {
		fmt.Println(mesnum.Error)
	}
}
