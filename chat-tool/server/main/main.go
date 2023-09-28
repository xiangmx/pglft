package main

import (
	"fmt"
	"net"
)

func main() {
	InitPool("localhost:6379", 16, 0, 100)
	InitUserDao()
	fmt.Println("正在监听....")
	listen, err := net.Listen("tcp", "localhost:8889")
	if err != nil {
		fmt.Println("监听失败,err=", err)
		return
	}
	defer listen.Close()
	for {
		fmt.Println("等待服务器进行连接....")
		conn, err := listen.Accept()
		//defer conn.Close()
		if err != nil {
			fmt.Println("连接失败：err=", err)
			return
		}
		fmt.Println("连接成功，conn=", conn)
		go prosess(conn)
	}
}

func prosess(conn net.Conn) {
	defer conn.Close()
	proc := &Processor{
		Conn: conn,
	}
	err := proc.Process2()
	if err != nil {
		fmt.Println("客户端和服务器通讯协程出现错误 ,err=", err)
		return
	}
}
