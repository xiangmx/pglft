package main

import (
	"github.com/garyburd/redigo/redis"
	"go-work/chat-tool/server/model"
	"time"
)

var pool *redis.Pool

func InitPool(address string, maxIdle int, maxActive int, idleTimeout time.Duration) {
	pool = &redis.Pool{
		MaxIdle:     maxIdle,     //最大空闲数
		MaxActive:   maxActive,   //和数据库的最大连接数，0表示没有限制
		IdleTimeout: idleTimeout, //最大空闲时间
		Dial: func() (redis.Conn, error) {
			return redis.Dial("tcp", address)
		},
	}
}

func InitUserDao() {
	model.MyUserDao = model.NewUserDao(pool)
}
