package process

import (
	"fmt"
	"go-work/chat-tool/client/model"
	"go-work/chat-tool/conmon/massage"
)

var onlineusers map[int]*massage.User = make(map[int]*massage.User, 10)

var curUser model.CurUser

//显示在线用户

func OutPutOnlineUsers() {
	fmt.Println("你的在线好友有：")
	for i, _ := range onlineusers {
		fmt.Println(i)
	}

}

//处理返回的UserStatusMes消息

func ProcessUserStatusMes(usestu massage.UserStatusMes) {
	user, ok := onlineusers[usestu.UserId]
	if !ok {
		user = &massage.User{
			UserId:     usestu.UserId,
			UserStatus: massage.UserOnline,
		}
	}
	user.UserStatus = usestu.UserStatus
	onlineusers[usestu.UserId] = user
	OutPutOnlineUsers()
}
