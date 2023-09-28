package model

import (
	"encoding/json"
	"fmt"
	"github.com/garyburd/redigo/redis"
	"go-work/chat-tool/conmon/massage"
)

var MyUserDao *UserDao

type UserDao struct {
	pool *redis.Pool
}

// 用工厂模式创建一个UserDao实例
func NewUserDao(Pool *redis.Pool) (userdao *UserDao) {
	userdao = &UserDao{
		pool: Pool,
	}
	return
}

//根据用户输入id，返回一个user实例

func (this *UserDao) GetUserById(conn redis.Conn, id int) (user *User, err error) {
	res, err := redis.String(conn.Do("HGet", "users", id))
	if err != nil {
		if err == redis.ErrNil { //表示在哈希中没有找到对应id
			err = ERROR_USER_NOTEXISTS
		}
		return
	}
	//需要把res反序列化成User实例
	user = &User{}
	err = json.Unmarshal([]byte(res), user)
	if err != nil {
		fmt.Println("json.Unmarshal err=", err)
		return
	}
	return
}

//登录验证

func (this *UserDao) LoginVerify(userid int, userpwd string) (user *User, err error) {
	conn := this.pool.Get()
	defer conn.Close()
	user, err = this.GetUserById(conn, userid)
	if err != nil {
		return
	}
	if userpwd != user.UserPwd {
		err = ERROR_USER_PWD
		return
	}
	return
}

//注册信息

func (ud *UserDao) RegisRedis(user *massage.User) (err error) {
	conn := ud.pool.Get()
	defer conn.Close()
	_, err = ud.GetUserById(conn, user.UserId)
	if err == nil { //如果获取到一个用户，说明已经注册过了
		err = ERROR_USER_EXISTS
		return
	}
	date, err := json.Marshal(user)
	if err != nil {
		fmt.Println("json.Marshal(user) err")
		return
	}
	_, err = conn.Do("HSet", "users", user.UserId, string(date))
	if err != nil {
		fmt.Println("注册用户信息失败")
		return
	}
	return
}
