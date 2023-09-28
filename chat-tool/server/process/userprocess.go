package process2

import (
	"encoding/json"
	"fmt"
	"go-work/chat-tool/conmon/massage"
	"go-work/chat-tool/server/model"
	"go-work/chat-tool/server/untils"
	"net"
)

type UserProcess struct {
	Conn   net.Conn
	UserId int
}

func (this *UserProcess) NotifyothersOnlineUser() {
	for id, up := range userMgr.onlineusers {
		if id == this.UserId {
			continue
		}
		up.NotifyMeOnline(this.UserId)
	}
}

//通知其他用户我上线了

func (this *UserProcess) NotifyMeOnline(userid int) {
	var sd massage.LoginType
	sd.Type = massage.UserStatusMesType
	var md massage.UserStatusMes
	md.UserId = userid
	md.UserStatus = massage.UserOnline
	data, err := json.Marshal(md)
	if err != nil {
		fmt.Println("json.Marshal md err=", err)
		return
	}
	sd.Date = string(data)
	sdata, err := json.Marshal(sd)
	if err != nil {
		fmt.Println("json.Marshal sd err=", err)
		return
	}
	tf := &untils.Transfor{
		Conn: this.Conn,
	}
	tf.WritePkg(sdata)
}

// 处理用户登录

func (this *UserProcess) ServerprocessLogin(mes *massage.LoginType) (err error) {
	var loginmes massage.LodinMessage
	err = json.Unmarshal([]byte(mes.Date), &loginmes)
	if err != nil {
		fmt.Println("json.Unmarshal err=", err)
		return
	}
	var nummes massage.LoginType
	nummes.Type = massage.LoginNumType
	var loginNum massage.LoginNum
	user, err := model.MyUserDao.LoginVerify(loginmes.UserId, loginmes.UserPwd)
	if err != nil {
		if err == model.ERROR_USER_NOTEXISTS {
			loginNum.Code = 500
			loginNum.Error = err.Error()
		} else if err == model.ERROR_USER_PWD {
			loginNum.Code = 403
			loginNum.Error = err.Error()
		} else {
			loginNum.Code = 505
			loginNum.Error = "服务器内部错误...."
		}
	} else {
		loginNum.Code = 200
		this.UserId = loginmes.UserId
		userMgr.AddUserMgr(this)
		this.NotifyothersOnlineUser()
		for id, _ := range userMgr.GetAllUserMgr() {
			loginNum.UsersId = append(loginNum.UsersId, id)
		}
		fmt.Println(user, "登录成功")
	}
	num, err := json.Marshal(loginNum)
	if err != nil {
		fmt.Println("json.Marshal(loginNum) err=", err)
		return
	}
	nummes.Date = string(num)
	sevmes, err := json.Marshal(nummes)
	if err != nil {
		fmt.Println("json.Marshal(nummes) err=", err)
		return
	}
	transfor := untils.Transfor{
		Conn: this.Conn,
	}
	err = transfor.WritePkg(sevmes)
	return
}

//处理用户注册

func (this *UserProcess) ServerProcessRegis(mes *massage.LoginType) (err error) {
	var rmes massage.RegisLoginMes
	err = json.Unmarshal([]byte(mes.Date), &rmes)
	if err != nil {
		fmt.Println("json.Unmarshal err=", err)
		return
	}

	var rerror massage.LoginType
	rerror.Type = massage.RegisErrorType
	var regisNum massage.RegisError
	err = model.MyUserDao.RegisRedis(&rmes.User)
	if err != nil {
		if err == model.ERROR_USER_EXISTS {
			regisNum.Code = 100
			regisNum.Error = "用户已经存在"
		} else {
			regisNum.Code = 503
			regisNum.Error = "注册出现未知错误"
		}
	} else {
		regisNum.Code = 200
		fmt.Println("注册成功")
	}
	num, err := json.Marshal(regisNum)
	if err != nil {
		fmt.Println("json.Marshal(regisNum) err=", err)
		return
	}
	rerror.Date = string(num)
	sevmes, err := json.Marshal(rerror)
	if err != nil {
		fmt.Println("json.Marshal err=", err)
		return
	}
	transfor := untils.Transfor{
		Conn: this.Conn,
	}
	err = transfor.WritePkg(sevmes)
	return
}
