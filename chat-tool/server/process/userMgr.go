package process2

import (
	"fmt"
)

var (
	userMgr *UserMgr
)

type UserMgr struct {
	onlineusers map[int]*UserProcess
}

//初始化UserMgr

func init() {
	userMgr = &UserMgr{
		onlineusers: make(map[int]*UserProcess, 1024),
	}
}

//增加一个UserMgr

func (this *UserMgr) AddUserMgr(up *UserProcess) {
	this.onlineusers[up.UserId] = up
}

//减少一个UserMgr

func (this *UserMgr) DelUserMgr(userid int) {
	delete(this.onlineusers, userid)
}

//返回所有的UserMgr

func (this *UserMgr) GetAllUserMgr() map[int]*UserProcess {
	return this.onlineusers
}

//根据id返回对应的值

func (this *UserMgr) GetUserById(userid int) (UP *UserProcess, err error) {
	UP, ok := this.onlineusers[userid]
	if !ok {
		err = fmt.Errorf("用户%d不存在", userid)
		return
	}
	return
}
