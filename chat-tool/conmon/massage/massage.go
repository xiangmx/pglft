package massage

const (
	LoginMessageType  = "LodinMessage"
	LoginNumType      = "LoginNum"
	RegisLoginMesType = "RegisLoginMes "
	RegisErrorType    = "RegisError"
	UserStatusMesType = "UserStatusMes"
	CurUserMesType    = "CurUserMes"
	SmsOneType        = "SmsOne"
)

const (
	UserOnline = iota
	UserOffline
)

type LoginType struct {
	Type string `json:"type"` //发送数据的类型
	Date string `json:"date"` //发送数据的内容
}

type LodinMessage struct {
	UserId   int    `json:"userId"`   //用户名
	UserPwd  string `json:"userPwd"`  //用户密码
	UserName string `json:"userName"` //用户序列号
}

type LoginNum struct {
	Code    int    `json:"code"` //返回状态码，500表示用户未注册，200表示登录成功
	UsersId []int  //保存在线的用户id
	Error   string `json:"error"` //返回错误信息
}

type RegisLoginMes struct {
	User User `json:"user"`
}
type RegisError struct {
	Code  int    `json:"code"`  //返回状态码，500表示用户未注册，200表示注册成功
	Error string `json:"error"` //返回错误信息
}
type UserStatusMes struct {
	UserId     int
	UserStatus int
}

type CurUserMes struct {
	Content string `json:"content"`
	User
}

type SmsOne struct {
	Content string `json:"content"`
	User
	MyId int `json:"myId"`
}
