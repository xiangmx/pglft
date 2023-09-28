package main

import (
	"fmt"
	"go-work/chat-tool/client/process"
	"os"
)

func main() {
	var key int
	//var count bool = true
	var UserId int
	var UserPwd string
	var UserName string
	for {
		fmt.Println("================欢迎登录多人聊天系统================")
		fmt.Println("\t\t\t1.登录多人聊天室")
		fmt.Println("\t\t\t2.注册用户")
		fmt.Println("\t\t\t3.退出系统")
		fmt.Println("\t\t\t请选择（1-3）")
		fmt.Scanln(&key)
		switch key {
		case 1:
			fmt.Println("登录界面")
			fmt.Println("请输入用户名：")
			fmt.Scanln(&UserId)
			fmt.Println("请输入密码：")
			fmt.Scanln(&UserPwd)
			up := &process.Userprocess{}
			up.SendMessage(UserId, UserPwd)
		case 2:
			fmt.Println("注册用户")
			fmt.Println("请输入用户名：")
			fmt.Scanln(&UserId)
			fmt.Println("请输入密码：")
			fmt.Scanln(&UserPwd)
			fmt.Println("请输入用户昵称：")
			fmt.Scanln(&UserName)
			up := &process.Userprocess{}
			up.RegisMes(UserId, UserPwd, UserName)
		case 3:
			fmt.Println("你已退出系统.....")
			os.Exit(0)
		default:
			fmt.Println("输入错误，请重新输入：")
		}
	}
}

//func loginsys(id *string, pwd *string) {
//	fmt.Println("请输入用户名：")
//	fmt.Scanln(&id)
//	fmt.Println("请输入密码：")
//	fmt.Scanln(&pwd)
//}
